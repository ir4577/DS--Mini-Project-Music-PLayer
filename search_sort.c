#include "playlist.h"

// Payload Data Swap Helper
static void swapSongData(Song *a, Song *b) {
    char tempTitle[MAX_STR], tempArtist[MAX_STR], tempFn[MAX_STR];
    int tempDur;

    strcpy(tempTitle, a->title);
    strcpy(a->title, b->title);
    strcpy(b->title, tempTitle);

    strcpy(tempArtist, a->artist);
    strcpy(a->artist, b->artist);
    strcpy(b->artist, tempArtist);

    strcpy(tempFn, a->filename);
    strcpy(a->filename, b->filename);
    strcpy(b->filename, tempFn);

    tempDur = a->duration;
    a->duration = b->duration;
    b->duration = tempDur;
}

// Comparison Helper: sortBy = 0 (Name), sortBy = 1 (Time)
static int compareSongs(Song *a, Song *b, int sortBy) {
    if (sortBy == 1) { 
        return a->duration - b->duration; // Shortest to Longest
    }
    return strcasecmp(a->title, b->title); // Alphabetical A-Z
}

void bubbleSort(Playlist *pl, int sortBy) {
    if (!pl || !pl->head) return;
    int swapped;
    Song *ptr1;
    Song *lptr = NULL;

    do {
        swapped = 0;
        ptr1 = pl->head;
        while (ptr1->next != lptr) {
            if (compareSongs(ptr1, ptr1->next, sortBy) > 0) {
                swapSongData(ptr1, ptr1->next);
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);

    // Save updated sorted order to file
    saveToFile(pl, "library.txt");
}