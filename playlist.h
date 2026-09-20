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

#define MAX_STR 256

typedef struct Song {
    char title[MAX_STR];
    char artist[MAX_STR];
    int duration; // in seconds
    char filename[MAX_STR]; // MP3 File Name or Web URL
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
void addSong(Playlist *pl, const char *title, const char *artist, int duration, const char *filename);
void deleteCurrentSong(Playlist *pl);
void nextSong(Playlist *pl);
void prevSong(Playlist *pl);
void saveToFile(const Playlist *pl, const char *filename);
void loadFromFile(Playlist *pl, const char *filename);
void freePlaylist(Playlist *pl);

// Data Structures: Search & Sort
int linearSearch(const Playlist *pl, const char *title);
int binarySearch(Playlist *pl, const char *title);
void bubbleSort(Playlist *pl, int sortBy); // 0 = Name, 1 = Time
void mergeSort(Playlist *pl, int sortBy);  // 0 = Name, 1 = Time

#endif