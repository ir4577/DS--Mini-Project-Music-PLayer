#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "playlist.h"

Playlist* initPlaylist(void) {
    Playlist *pl = (Playlist*)malloc(sizeof(Playlist));
    pl->head = NULL;
    pl->tail = NULL;
    pl->current = NULL;
    pl->size = 0;
    pl->loop_mode = 0;
    return pl;
}

void addSong(Playlist *pl, const char *title, const char *artist, int duration, const char *filename) {
    Song *newSong = (Song*)malloc(sizeof(Song));
    strncpy(newSong->title, title, MAX_STR - 1);
    strncpy(newSong->artist, artist, MAX_STR - 1);
    newSong->duration = duration;
    
    if (filename && strlen(filename) > 0) {
        strncpy(newSong->filename, filename, MAX_STR - 1);
    } else {
        strcpy(newSong->filename, "");
    }

    newSong->next = NULL;
    newSong->prev = pl->tail;

    if (pl->tail) {
        pl->tail->next = newSong;
    }
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

    if (toDelete->prev) {
        toDelete->prev->next = toDelete->next;
    } else {
        pl->head = toDelete->next;
    }

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
    } else if (pl->loop_mode && pl->head) {
        pl->current = pl->head;
    }
}

void prevSong(Playlist *pl) {
    if (!pl->current) return;
    if (pl->current->prev) {
        pl->current = pl->current->prev;
    } else if (pl->loop_mode && pl->tail) {
        pl->current = pl->tail;
    }
}

void saveToFile(const Playlist *pl, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return;

    Song *curr = pl->head;
    while (curr) {
        fprintf(fp, "%s;%s;%d;%s\n", curr->title, curr->artist, curr->duration, curr->filename);
        curr = curr->next;
    }
    fclose(fp);
}

void loadFromFile(Playlist *pl, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return;

    char line[MAX_STR * 4];
    while (fgets(line, sizeof(line), fp)) {
        char title[MAX_STR] = "", artist[MAX_STR] = "", fn[MAX_STR] = "";
        int duration = 180;

        char *token = strtok(line, ";");
        if (token) strncpy(title, token, MAX_STR - 1);

        token = strtok(NULL, ";");
        if (token) strncpy(artist, token, MAX_STR - 1);

        token = strtok(NULL, ";");
        if (token) duration = atoi(token);

        token = strtok(NULL, ";\n");
        if (token) strncpy(fn, token, MAX_STR - 1);

        if (strlen(title) > 0) {
            addSong(pl, title, artist, duration, fn);
        }
    }
    fclose(fp);
}

void freePlaylist(Playlist *pl) {
    Song *curr = pl->head;
    while (curr) {
        Song *next = curr->next;
        free(curr);
        curr = next;
    }
    free(pl);
}