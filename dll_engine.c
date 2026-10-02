#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define MAX_STR 256
#define BUFFER_SIZE 65536  /* 64 KB buffer for binary file uploads */

typedef struct Song {
    char title[MAX_STR];
    char artist[MAX_STR];
    int duration;
    char filename[MAX_STR];
    struct Song *next;
    struct Song *prev;
} Song;

typedef struct {
    Song *head;
    Song *tail;
    Song *current;
    int size;
    int loop_mode;
} Playlist;

/* Parse MP3 binary header to calculate audio duration in seconds */
int getMP3Duration(const char *filename) {
    if (!filename || strlen(filename) == 0) return 180;
    FILE *fp = fopen(filename, "rb");
    if (!fp) return 180;

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (filesize <= 0) { fclose(fp); return 180; }

    unsigned char buf[10];
    int id3Size = 0;
    if (fread(buf, 1, 10, fp) == 10) {
        if (memcmp(buf, "ID3", 3) == 0) {
            id3Size = ((buf[6] & 0x7F) << 21) |
                      ((buf[7] & 0x7F) << 14) |
                      ((buf[8] & 0x7F) << 7)  |
                       (buf[9] & 0x7F);
            id3Size += 10;
        }
    }

    long audioSize = filesize - id3Size;
    if (audioSize <= 0) { fclose(fp); return 180; }

    fseek(fp, id3Size, SEEK_SET);

    const int bitrates[] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
    int detectedBitrate = 128;

    unsigned char header[4];
    long bytesRead = 0;
    while (bytesRead < 8192 && fread(header, 1, 4, fp) == 4) {
        if (header[0] == 0xFF && (header[1] & 0xE0) == 0xE0) {
            int bitrateIdx = (header[2] >> 4) & 0x0F;
            if (bitrateIdx > 0 && bitrateIdx < 15) {
                detectedBitrate = bitrates[bitrateIdx];
                break;
            }
        }
        fseek(fp, -3, SEEK_CUR);
        bytesRead++;
    }

    fclose(fp);

    int durationSec = (int)((audioSize * 8) / (detectedBitrate * 1000));
    return durationSec > 0 ? durationSec : 180;
}

Playlist* initPlaylist(void) {
    Playlist *pl = (Playlist*)malloc(sizeof(Playlist));
    pl->head = pl->tail = pl->current = NULL;
    pl->size = pl->loop_mode = 0;
    return pl;
}

void addSong(Playlist *pl, const char *title, const char *artist, int duration, const char *filename) {
    Song *newSong = (Song*)malloc(sizeof(Song));
    if (!newSong) return;

    strncpy(newSong->title, title, MAX_STR - 1);
    newSong->title[MAX_STR - 1] = '\0';
    strncpy(newSong->artist, artist, MAX_STR - 1);
    newSong->artist[MAX_STR - 1] = '\0';
    newSong->duration = duration;

    if (filename && strlen(filename) > 0) {
        strncpy(newSong->filename, filename, MAX_STR - 1);
        newSong->filename[MAX_STR - 1] = '\0';
    } else {
        strcpy(newSong->filename, "");
    }

    newSong->next = NULL;
    newSong->prev = pl->tail;

    if (pl->tail) pl->tail->next = newSong;
    pl->tail = newSong;

    if (!pl->head) {
        pl->head = newSong;
        pl->current = newSong;
    }
    pl->size++;
}

void deleteCurrentSong(Playlist *pl) {
    if (!pl->current) return;
    Song *toDelete = pl->current;

    if (toDelete->prev) toDelete->prev->next = toDelete->next;
    else pl->head = toDelete->next;

    if (toDelete->next) {
        toDelete->next->prev = toDelete->prev;
        pl->current = toDelete->next;
    } else {
        pl->tail = toDelete->prev;
        pl->current = toDelete->prev;
    }

    free(toDelete);
    pl->size--;
}

void nextSong(Playlist *pl) {
    if (!pl->current) return;
    if (pl->current->next) {
        pl->current = pl->current->next;
    } else if (pl->head) {
        pl->current = pl->head; /* Wrap around to head */
    }
}

void prevSong(Playlist *pl) {
    if (!pl->current) return;
    if (pl->current->prev) {
        pl->current = pl->current->prev;
    } else if (pl->tail) {
        pl->current = pl->tail; /* Wrap around to tail */
    }
}

void swapSongData(Song *a, Song *b) {
    char temp_title[MAX_STR], temp_artist[MAX_STR], temp_fn[MAX_STR];
    int temp_dur = a->duration;

    strcpy(temp_title, a->title); strcpy(temp_artist, a->artist); strcpy(temp_fn, a->filename);
    strcpy(a->title, b->title);   strcpy(a->artist, b->artist);   strcpy(a->filename, b->filename);
    a->duration = b->duration;
    strcpy(b->title, temp_title); strcpy(b->artist, temp_artist); strcpy(b->filename, temp_fn);
    b->duration = temp_dur;
}

void sortByName(Playlist *pl) {
    if (!pl->head || pl->size < 2) return;
    int swapped; Song *ptr1, *lptr = NULL;
    do {
        swapped = 0; ptr1 = pl->head;
        while (ptr1->next != lptr) {
            if (stricmp(ptr1->title, ptr1->next->title) > 0) {
                swapSongData(ptr1, ptr1->next);
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);
}

void sortByDuration(Playlist *pl) {
    if (!pl->head || pl->size < 2) return;
    int swapped; Song *ptr1, *lptr = NULL;
    do {
        swapped = 0; ptr1 = pl->head;
        while (ptr1->next != lptr) {
            if (ptr1->duration > ptr1->next->duration) {
                swapSongData(ptr1, ptr1->next);
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);
}

void buildJSONResponse(Playlist *pl, char *output, size_t max_len) {
    snprintf(output, max_len, "{\"songs\":[");
    Song *curr = pl->head;
    while (curr) {
        char item[MAX_STR * 4];
        int is_curr = (curr == pl->current) ? 1 : 0;
        snprintf(item, sizeof(item),
                 "{\"title\":\"%s\",\"artist\":\"%s\",\"duration\":%d,\"filename\":\"%s\",\"is_current\":%s}%s",
                 curr->title, curr->artist, curr->duration, curr->filename,
                 is_curr ? "true" : "false", curr->next ? "," : "");
        strncat(output, item, max_len - strlen(output) - 1);
        curr = curr->next;
    }
    strncat(output, "]}", max_len - strlen(output) - 1);
}

void urlDecode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a' && a <= 'f') a -= 'a' - 'A';
            if (a >= 'A' && a <= 'F') a -= 'A' - 10; else a -= '0';
            if (b >= 'a' && b <= 'f') b -= 'a' - 'A';
            if (b >= 'A' && b <= 'F') b -= 'A' - 10; else b -= '0';
            *dst++ = 16 * a + b; src += 3;
        } else if (*src == '+') { *dst++ = ' '; src++; }
        else { *dst++ = *src++; }
    }
    *dst = '\0';
}

void getHeaderValue(const char *buffer, const char *headerName, char *outVal) {
    char search[128];
    snprintf(search, sizeof(search), "%s:", headerName);
    char *loc = strstr(buffer, search);
    if (loc) {
        loc += strlen(search);
        while (*loc == ' ') loc++;
        char *end = strstr(loc, "\r\n");
        if (end) {
            int len = end - loc;
            if (len >= MAX_STR) len = MAX_STR - 1;
            strncpy(outVal, loc, len);
            outVal[len] = '\0';
            return;
        }
    }
    strcpy(outVal, "");
}

void getQueryParam(const char *buffer, const char *paramName, char *outVal) {
    char search[128];
    snprintf(search, sizeof(search), "%s=", paramName);
    char *loc = strstr(buffer, search);
    if (loc) {
        loc += strlen(search);
        int i = 0;
        while (loc[i] != '\0' && loc[i] != '&' && loc[i] != ' ' && loc[i] != '\r' && loc[i] != '\n' && i < MAX_STR - 1) {
            outVal[i] = loc[i];
            i++;
        }
        outVal[i] = '\0';
        return;
    }
    strcpy(outVal, "");
}

void handleClient(SOCKET clientSocket, Playlist *pl) {
    char *buffer = (char*)malloc(BUFFER_SIZE);
    if (!buffer) return;
    memset(buffer, 0, BUFFER_SIZE);
    char httpHeader[BUFFER_SIZE];
    snprintf(httpHeader, sizeof(httpHeader), 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Cache-Control: no-cache, no-store, must-revalidate\r\n"
        "Content-Length: %d\r\n\r\n%s", 
        (int)strlen(jsonBuffer), jsonBuffer);
    send(clientSocket, httpHeader, strlen(httpHeader), 0);
    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesRead <= 0) { free(buffer); closesocket(clientSocket); return; }

    /* CORS PREFLIGHT OPTIONS REQUEST HANDLER */
    if (strncmp(buffer, "OPTIONS", 7) == 0) {
        const char *corsHeader = 
            "HTTP/1.1 204 No Content\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type, X-Song-Title, X-Song-Artist, X-Song-Filename, X-Song-Duration\r\n"
            "Content-Length: 0\r\n\r\n";
        send(clientSocket, corsHeader, strlen(corsHeader), 0);
    }
    /* TEXT NODE ADDITION ENDPOINT */
    else if (strncmp(buffer, "GET /api/add", 12) == 0) {
        char rawTitle[MAX_STR] = "Untitled", rawArtist[MAX_STR] = "Unknown", rawFn[MAX_STR] = "";
        char title[MAX_STR], artist[MAX_STR], filename[MAX_STR];
        int duration = 0;

        getQueryParam(buffer, "title", rawTitle);
        getQueryParam(buffer, "artist", rawArtist);
        getQueryParam(buffer, "filename", rawFn);

        char durStr[32] = {0};
        getQueryParam(buffer, "duration", durStr);
        if (strlen(durStr) > 0) duration = atoi(durStr);

        urlDecode(title, rawTitle);
        urlDecode(artist, rawArtist);
        urlDecode(filename, rawFn);

        if (strlen(title) == 0) strcpy(title, "New Track");
        if (strlen(artist) == 0) strcpy(artist, "Unknown Artist");
        
        if (duration <= 0 && strlen(filename) > 0) {
            duration = getMP3Duration(filename);
        } else if (duration <= 0) {
            duration = 180;
        }

        addSong(pl, title, artist, duration, filename);

        char jsonBuffer[BUFFER_SIZE / 2] = {0};
        buildJSONResponse(pl, jsonBuffer, sizeof(jsonBuffer));
        char httpHeader[BUFFER_SIZE];
        snprintf(httpHeader, sizeof(httpHeader), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: %zu\r\n\r\n%s", strlen(jsonBuffer), jsonBuffer);
        send(clientSocket, httpHeader, strlen(httpHeader), 0);
    }
    /* MP3 FILE UPLOAD ENDPOINT */
    else if (strncmp(buffer, "POST /api/upload", 16) == 0) {
        char rawTitle[MAX_STR] = "Uploaded Song", rawArtist[MAX_STR] = "Artist", rawFn[MAX_STR] = "song.mp3";
        char title[MAX_STR], artist[MAX_STR], filename[MAX_STR];
        int duration = 0;

        getHeaderValue(buffer, "X-Song-Title", rawTitle);
        getHeaderValue(buffer, "X-Song-Artist", rawArtist);
        getHeaderValue(buffer, "X-Song-Filename", rawFn);
        
        char durStr[32] = {0};
        getHeaderValue(buffer, "X-Song-Duration", durStr);
        if (strlen(durStr) > 0) duration = atoi(durStr);

        urlDecode(title, rawTitle);
        urlDecode(artist, rawArtist);
        urlDecode(filename, rawFn);

        if (strlen(filename) == 0) strcpy(filename, "uploaded.mp3");

        char contentLengthStr[32] = {0};
        getHeaderValue(buffer, "Content-Length", contentLengthStr);
        int contentLength = atoi(contentLengthStr);

        char *bodyStart = strstr(buffer, "\r\n\r\n");
        if (bodyStart) {
            bodyStart += 4;
            int initialBodyBytes = bytesRead - (bodyStart - buffer);

            FILE *fp = fopen(filename, "wb");
            if (fp) {
                if (initialBodyBytes > 0) {
                    fwrite(bodyStart, 1, initialBodyBytes, fp);
                }
                int totalWritten = initialBodyBytes;
                while (totalWritten < contentLength) {
                    int chunk = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                    if (chunk <= 0) break;
                    fwrite(buffer, 1, chunk, fp);
                    totalWritten += chunk;
                }
                fclose(fp);
            }
        }

        if (duration <= 0) {
            duration = getMP3Duration(filename);
        }

        addSong(pl, title, artist, duration, filename);

        char jsonBuffer[BUFFER_SIZE / 2] = {0};
        buildJSONResponse(pl, jsonBuffer, sizeof(jsonBuffer));
        char httpHeader[BUFFER_SIZE];
        snprintf(httpHeader, sizeof(httpHeader), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: %zu\r\n\r\n%s", strlen(jsonBuffer), jsonBuffer);
        send(clientSocket, httpHeader, strlen(httpHeader), 0);
    }
    /* JSON STATE ENDPOINTS */
    else if (strncmp(buffer, "GET /api/data", 13) == 0) {
        char jsonBuffer[BUFFER_SIZE / 2] = {0};
        buildJSONResponse(pl, jsonBuffer, sizeof(jsonBuffer));
        char httpHeader[BUFFER_SIZE];
        snprintf(httpHeader, sizeof(httpHeader), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: %zu\r\n\r\n%s", strlen(jsonBuffer), jsonBuffer);
        send(clientSocket, httpHeader, strlen(httpHeader), 0);
    }
    else if (strncmp(buffer, "GET /api/next", 13) == 0) {
        nextSong(pl);
        char jsonBuffer[BUFFER_SIZE / 2] = {0};
        buildJSONResponse(pl, jsonBuffer, sizeof(jsonBuffer));
        char httpHeader[BUFFER_SIZE];
        snprintf(httpHeader, sizeof(httpHeader), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: %zu\r\n\r\n%s", strlen(jsonBuffer), jsonBuffer);
        send(clientSocket, httpHeader, strlen(httpHeader), 0);
    }
    else if (strncmp(buffer, "GET /api/prev", 13) == 0) {
        prevSong(pl);
        char jsonBuffer[BUFFER_SIZE / 2] = {0};
        buildJSONResponse(pl, jsonBuffer, sizeof(jsonBuffer));
        char httpHeader[BUFFER_SIZE];
        snprintf(httpHeader, sizeof(httpHeader), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: %zu\r\n\r\n%s", strlen(jsonBuffer), jsonBuffer);
        send(clientSocket, httpHeader, strlen(httpHeader), 0);
    }
    else if (strncmp(buffer, "GET /api/delete", 15) == 0) {
        deleteCurrentSong(pl);
        char jsonBuffer[BUFFER_SIZE / 2] = {0};
        buildJSONResponse(pl, jsonBuffer, sizeof(jsonBuffer));
        char httpHeader[BUFFER_SIZE];
        snprintf(httpHeader, sizeof(httpHeader), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: %zu\r\n\r\n%s", strlen(jsonBuffer), jsonBuffer);
        send(clientSocket, httpHeader, strlen(httpHeader), 0);
    }
    else if (strncmp(buffer, "GET /api/sort_name", 18) == 0) {
        sortByName(pl);
        char jsonBuffer[BUFFER_SIZE / 2] = {0};
        buildJSONResponse(pl, jsonBuffer, sizeof(jsonBuffer));
        char httpHeader[BUFFER_SIZE];
        snprintf(httpHeader, sizeof(httpHeader), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: %zu\r\n\r\n%s", strlen(jsonBuffer), jsonBuffer);
        send(clientSocket, httpHeader, strlen(httpHeader), 0);
    }
    else if (strncmp(buffer, "GET /api/sort_time", 18) == 0) {
        sortByDuration(pl);
        char jsonBuffer[BUFFER_SIZE / 2] = {0};
        buildJSONResponse(pl, jsonBuffer, sizeof(jsonBuffer));
        char httpHeader[BUFFER_SIZE];
        snprintf(httpHeader, sizeof(httpHeader), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: %zu\r\n\r\n%s", strlen(jsonBuffer), jsonBuffer);
        send(clientSocket, httpHeader, strlen(httpHeader), 0);
    }
    /* MP3 & HTML STATIC FILE SERVING */
    else {
        char reqPath[MAX_STR] = "index.html";
        sscanf(buffer, "GET /%s", reqPath);

        if (strcmp(reqPath, "HTTP/1.1") == 0 || strcmp(reqPath, "") == 0) {
            strcpy(reqPath, "index.html");
        }

        char decodedPath[MAX_STR];
        urlDecode(decodedPath, reqPath);

        FILE *f = fopen(decodedPath, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);

            const char *mimeType = "text/html";
            if (strstr(decodedPath, ".mp3")) mimeType = "audio/mpeg";

            char header[256];
            snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: %ld\r\n\r\n", mimeType, fsize);
            send(clientSocket, header, strlen(header), 0);

            char fileBuf[8192];
            size_t n;
            while ((n = fread(fileBuf, 1, sizeof(fileBuf), f)) > 0) {
                send(clientSocket, fileBuf, n, 0);
            }
            fclose(f);
        } else {
            const char *notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found";
            send(clientSocket, notFound, strlen(notFound), 0);
        }
    }

    free(buffer);
    closesocket(clientSocket);
}

int main(void) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    Playlist *pl = initPlaylist();

    printf("DLL Music Studio C Engine running on http://localhost:8080\n");

    while (1) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket != INVALID_SOCKET) {
            handleClient(clientSocket, pl);
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}