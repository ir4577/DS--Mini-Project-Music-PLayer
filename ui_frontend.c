#include <stdio.h>
#include <stdlib.h>
#include "playlist.h"

// ANSI Escape Codes for Styling
#define COLOR_RESET   "\033[0m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_MAGENTA "\033[1;35m"

void clearScreen(void) {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void renderUI(const Playlist *pl) {
    clearScreen();
    printf("%s===================================================%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s    ♪ MUSIC PLAYLIST MANAGER (DLL ENGINE) ♪        %s\n", COLOR_MAGENTA, COLOR_RESET);
    printf("%s===================================================%s\n", COLOR_CYAN, COLOR_RESET);

    // Dynamic Header Info
    printf(" Total Tracks: %s%d%s | Loop Mode: %s%s%s\n", 
           COLOR_YELLOW, pl->size, COLOR_RESET,
           pl->loop_mode ? COLOR_GREEN : COLOR_RED, 
           pl->loop_mode ? "ON" : "OFF", COLOR_RESET);
    printf("---------------------------------------------------\n");

    if (pl->size == 0) {
        printf("\n  %s[ Playlist is empty. Add tracks or load library! ]%s\n\n", COLOR_RED, COLOR_RESET);
        printf("%s===================================================%s\n", COLOR_CYAN, COLOR_RESET);
        return;
    }

    // Now Playing Hero Widget
    printf("%s NOW PLAYING:%s %s%s%s by %s%s%s [%dm %ds]\n", 
           COLOR_GREEN, COLOR_RESET,
           COLOR_YELLOW, pl->current->title, COLOR_RESET,
           COLOR_CYAN, pl->current->artist, COLOR_RESET,
           pl->current->duration / 60, pl->current->duration % 60);
    printf("---------------------------------------------------\n");

    // Track Queue Rendering
    Song *curr = pl->head;
    int idx = 1;
    while (curr) {
        if (curr == pl->current) {
            printf(" %s-> %2d. %-20s | %-15s [*PLAYING*]%s\n", 
                   COLOR_GREEN, idx, curr->title, curr->artist, COLOR_RESET);
        } else {
            printf("    %2d. %-20s | %-15s\n", idx, curr->title, curr->artist);
        }
        curr = curr->next;
        idx++;
    }
    printf("%s===================================================%s\n", COLOR_CYAN, COLOR_RESET);
}

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}