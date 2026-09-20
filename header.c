#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #define strcasecmp _stricmp
#else
  #include <strings.h>
#endif

#define MAX_STR 100

typedef struct Song {
    char title[MAX_STR];
    char artist[MAX_STR];
    int duration;
    struct Song *prev;
    struct Song *next;
} Song;

typedef struct Playlist {
    Song *head;
    Song *tail;
    Song *current;
    int size;
    int loop_mode;
} Playlist;

// Core Engine API
Playlist* initPlaylist(void);
void addSong(Playlist *pl, const char *title, const char *artist, int duration);
void deleteCurrentSong(Playlist *pl);
void nextSong(Playlist *pl);
void prevSong(Playlist *pl);
void saveToFile(const Playlist *pl, const char *filename);
void loadFromFile(Playlist *pl, const char *filename);
void freePlaylist(Playlist *pl);

// UI Engine API
void renderUI(const Playlist *pl);
void clearInputBuffer(void);

// Data Structures: Search Algorithms
int linearSearch(const Playlist *pl, const char *title);
int binarySearch(Playlist *pl, const char *title);

// Data Structures: Sort Algorithms
void bubbleSort(Playlist *pl);
void mergeSort(Playlist *pl);

#endif