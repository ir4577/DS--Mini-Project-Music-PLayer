#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "playlist.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 8192

Playlist *myPlaylist;

void serveFile(SOCKET clientSocket, const char *fileName, const char *contentType) {
    FILE *fp = fopen(fileName, "rb");
    if (!fp) {
        const char *notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        send(clientSocket, notFound, (int)strlen(notFound), 0);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *fileBuffer = (char *)malloc(fileSize);
    if (fileBuffer) {
        fread(fileBuffer, 1, fileSize, fp);
        fclose(fp);

        char header[512];
        sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\nAccess-Control-Allow-Origin: *\r\n\r\n", contentType, fileSize);
        
        send(clientSocket, header, (int)strlen(header), 0);
        send(clientSocket, fileBuffer, fileSize, 0);
        free(fileBuffer);
    } else {
        fclose(fp);
    }
}

void sendPlaylistJSON(SOCKET clientSocket) {
    char body[BUFFER_SIZE];
    int offset = 0;

    offset += sprintf(body + offset, "{\"size\":%d,\"loop_mode\":%d,\"songs\":[", 
                       myPlaylist->size, myPlaylist->loop_mode);

    Song *curr = myPlaylist->head;
    int idx = 0;
    while (curr) {
        int is_current = (curr == myPlaylist->current) ? 1 : 0;
        offset += sprintf(body + offset, 
            "%s{\"id\":%d,\"title\":\"%s\",\"artist\":\"%s\",\"duration\":%d,\"filename\":\"%s\",\"is_current\":%d}",
            (idx == 0) ? "" : ",", idx, curr->title, curr->artist, curr->duration, curr->filename, is_current);
        curr = curr->next;
        idx++;
    }

    offset += sprintf(body + offset, "]}");

    char header[512];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %d\r\nAccess-Control-Allow-Origin: *\r\n\r\n", offset);
    
    send(clientSocket, header, (int)strlen(header), 0);
    send(clientSocket, body, offset, 0);
}

int main(void) {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];

    myPlaylist = initPlaylist();
    loadFromFile(myPlaylist, "library.txt");
    if (myPlaylist->size == 0) {
        // Sample public royalty-free MP3 URLs pre-loaded for testing audio playback out of the box
        addSong(myPlaylist, "SoundHelix Track 1", "SoundHelix", 372, "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-1.mp3");
        addSong(myPlaylist, "SoundHelix Track 2", "SoundHelix", 423, "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-2.mp3");
        addSong(myPlaylist, "SoundHelix Track 3", "SoundHelix", 344, "https://www.soundhelix.com/examples/mp3/SoundHelix-Song-3.mp3");
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock initialization failed.\n");
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    printf("========================================================\n");
    printf(" ♪ SERVER RUNNING! Open in browser: http://localhost:%d\n", PORT);
    printf("========================================================\n");

    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) continue;

        memset(buffer, 0, BUFFER_SIZE);
        recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

        if (strstr(buffer, "GET /api/data") != NULL) {
            sendPlaylistJSON(clientSocket);
        } else if (strstr(buffer, "GET /api/next") != NULL) {
            nextSong(myPlaylist);
            sendPlaylistJSON(clientSocket);
        } else if (strstr(buffer, "GET /api/prev") != NULL) {
            prevSong(myPlaylist);
            sendPlaylistJSON(clientSocket);
        } else if (strstr(buffer, "GET /api/delete") != NULL) {
            deleteCurrentSong(myPlaylist);
            sendPlaylistJSON(clientSocket);
        } else if (strstr(buffer, "GET /api/sort_name") != NULL) {
            bubbleSort(myPlaylist, 0); // 0 = Sort by Name
            sendPlaylistJSON(clientSocket);
        } else if (strstr(buffer, "GET /api/sort_time") != NULL) {
            bubbleSort(myPlaylist, 1); // 1 = Sort by Duration
            sendPlaylistJSON(clientSocket);
        } else if (strstr(buffer, "POST /api/add") != NULL) {
            char *body = strstr(buffer, "\r\n\r\n");
            if (body) {
                body += 4;
                char title[MAX_STR] = "New Track", artist[MAX_STR] = "Unknown", filename[MAX_STR] = "";
                int duration = 180;
                sscanf(body, "title=%[^&]&artist=%[^&]&duration=%d&filename=%s", title, artist, &duration, filename);
                addSong(myPlaylist, title, artist, duration, filename);
            }
            sendPlaylistJSON(clientSocket);
        } else if (strstr(buffer, ".mp3") != NULL) {
            char reqFile[MAX_STR] = "";
            sscanf(buffer, "GET /%s HTTP/1.1", reqFile);
            serveFile(clientSocket, reqFile, "audio/mpeg");
        } else {
            serveFile(clientSocket, "index.html", "text/html");
        }

        closesocket(clientSocket);
    }

    WSACleanup();
    return 0;
}