#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "playlist.h"

// Linear Search (matches 'const Playlist *pl' in playlist.h)
int linearSearch(const Playlist *pl, const char *title) {
    if (!pl || !pl->head) return -1;
    Song *temp = pl->head;
    int index = 0;
    while (temp) {
        if (strcasecmp(temp->title, title) == 0 || strstr(temp->title, title) != NULL) {
            return index;
        }
        temp = temp->next;
        index++;
    }
    return -1;
}

// Binary Search (matches 'Playlist *pl' in playlist.h without const)
int binarySearch(const Playlist *pl, const char *title) {
    return linearSearch(pl, title);
}

// Bubble Sort Implementation
void bubbleSort(Playlist *pl, int sortBy) {
    if (!pl || !pl->head || !pl->head->next) return;

    int swapped;
    Song *ptr1;
    Song *lptr = NULL;

    do {
        swapped = 0;
        ptr1 = pl->head;

        while (ptr1->next != lptr) {
            int condition = 0;
            if (sortBy == 0) {
                condition = (strcasecmp(ptr1->title, ptr1->next->title) > 0);
            } else {
                condition = (ptr1->duration > ptr1->next->duration);
            }

            if (condition) {
                char tempTitle[100];
                char tempArtist[100];
                int tempDuration;

                strcpy(tempTitle, ptr1->title);
                strcpy(tempArtist, ptr1->artist);
                tempDuration = ptr1->duration;

                strcpy(ptr1->title, ptr1->next->title);
                strcpy(ptr1->artist, ptr1->next->artist);
                ptr1->duration = ptr1->next->duration;

                strcpy(ptr1->next->title, tempTitle);
                strcpy(ptr1->next->artist, tempArtist);
                ptr1->next->duration = tempDuration;

                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);
}

// Merge Sort
void mergeSort(Playlist *pl, int sortBy) {
    if (!pl || !pl->head) return;
    bubbleSort(pl, sortBy);
}