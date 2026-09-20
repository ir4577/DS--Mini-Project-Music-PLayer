#include <stdio.h>
#include <string.h>
#include "playlist.h"

void handleSearchMenu(Playlist *pl) {
    char searchTitle[MAX_STR];
    int choice = 0, index = -1;

    printf("\n=== SEARCH MENU ===\n");
    printf("Enter Song Title: ");
    fgets(searchTitle, MAX_STR, stdin);
    searchTitle[strcspn(searchTitle, "\n")] = 0;

    printf("1. Linear Search  O(N)\n");
    printf("2. Binary Search  O(log N)\n");
    printf("Choice: ");
    if (scanf("%d", &choice) != 1) { clearInputBuffer(); return; }
    clearInputBuffer();

    if (choice == 1) index = linearSearch(pl, searchTitle);
    else if (choice == 2) index = binarySearch(pl, searchTitle);

    if (index != -1) printf("\n[SUCCESS] Track found at Position #%d!\n", index);
    else printf("\n[NOT FOUND] Track '%s' not found.\n", searchTitle);
    
    printf("Press ENTER to continue...");
    getchar();
}

void handleSortMenu(Playlist *pl) {
    int choice = 0;

    printf("\n=== SORT MENU ===\n");
    printf("1. Bubble Sort O(N^2)\n");
    printf("2. Merge Sort  O(N log N)\n");
    printf("Choice: ");
    if (scanf("%d", &choice) != 1) { clearInputBuffer(); return; }
    clearInputBuffer();

    if (choice == 1) bubbleSort(pl);
    else if (choice == 2) mergeSort(pl);

    printf("\n[SUCCESS] Playlist sorted!\n");
    printf("Press ENTER to continue...");
    getchar();
}

int main(void) {
    Playlist *myPlaylist = initPlaylist();
    loadFromFile(myPlaylist, "library.txt");

    if (myPlaylist->size == 0) {
        addSong(myPlaylist, "Hotel California", "Eagles", 390);
        addSong(myPlaylist, "Bohemian Rhapsody", "Queen", 354);
        addSong(myPlaylist, "Starboy", "The Weeknd", 230);
        addSong(myPlaylist, "Africa", "Toto", 295);
    }

    int choice = 0;
    char title[MAX_STR], artist[MAX_STR];
    int duration;

    while (1) {
        renderUI(myPlaylist);
        printf("\nControls:\n");
        printf(" 1. Next Track      2. Prev Track      3. Add Song\n");
        printf(" 4. Delete Current  5. Search Playlist 6. Sort Playlist\n");
        printf(" 7. Toggle Loop     8. Save Library    9. Exit\n");
        printf("\nSelect Action: ");

        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1: nextSong(myPlaylist); break;
            case 2: prevSong(myPlaylist); break;
            case 3:
                printf("\nSong Title: ");
                fgets(title, MAX_STR, stdin);
                title[strcspn(title, "\n")] = 0;

                printf("Artist Name: ");
                fgets(artist, MAX_STR, stdin);
                artist[strcspn(artist, "\n")] = 0;

                printf("Duration (seconds): ");
                scanf("%d", &duration);
                clearInputBuffer();

                addSong(myPlaylist, title, artist, duration);
                break;
            case 4: deleteCurrentSong(myPlaylist); break;
            case 5: handleSearchMenu(myPlaylist); break;
            case 6: handleSortMenu(myPlaylist); break;
            case 7: myPlaylist->loop_mode = !myPlaylist->loop_mode; break;
            case 8: saveToFile(myPlaylist, "library.txt"); break;
            case 9:
                saveToFile(myPlaylist, "library.txt");
                freePlaylist(myPlaylist);
                return 0;
            default: break;
        }
    }
}