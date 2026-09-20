# 🎵 Music Playlist Manager (Doubly Linked List & Embedded C Web Engine)

A full-stack, low-level system project that pairs a custom C **Doubly Linked List (DLL)** audio engine with an embedded **WinSock TCP HTTP Web Server**. 

Instead of relying on high-level frameworks or command-line text prompts, this engine exposes a REST-like JSON API over `localhost:8080` to drive a modern Spotify-style Web Dashboard with live Heap pointer visualization and native MP3 audio streaming.

---

## 🌟 Key Features

* **Dynamic Memory Management:** Head-and-tail doubly linked list ($O(1)$ insertion, traversal, and node deletion) with dynamic Heap allocation (`malloc`/`free`).
* **Embedded C HTTP Server:** Raw TCP socket server written using Windows Sockets (`winsock2.h`) that parses HTTP GET/POST requests and serves static files and streaming binary MP3 audio.
* **REST JSON API:** Low-level C functions serialize list state into structured JSON strings on the fly for frontend consumption.
* **Live Heap Pointer Visualizer:** Real-time web visualization of node addresses and pointer linkages (`NULL ← [Prev | Data | Next] ⇄ [Prev | Data | Next] → NULL`).
* **Algorithmic Sorting & Search:** Flexible sorting engine supporting multi-key reordering (by **Track Title** or **Duration**) via Bubble Sort and Merge Sort.
* **Native Audio Playback:** HTML5 Audio player streaming local `.mp3` files or direct web links.
* **Persistent Library Storage:** Automatic synchronization to disk (`library.txt`) for list state persistence across server restarts.

---

## 🏗️ System Architecture

```text
┌──────────────────────────────────────────────────────────┐
│              Browser Dashboard (index.html)              │
│   (HTML5 Audio Player • Visual Pointer Map • REST Client) │
└─────────────────────────────┬────────────────────────────┘
                              │
                      HTTP / REST (JSON)
                              │
┌─────────────────────────────▼────────────────────────────┐
│             Embedded C TCP Server (web_server.c)         │
│         (WinSock2 • HTTP Request Parser • File Streamer) │
└─────────────────────────────┬────────────────────────────┘
                              │
                 Direct Pointer Manipulation
                              │
┌─────────────────────────────▼────────────────────────────┐
│              Doubly Linked List Engine (C)               │
│   ┌──────────┐      ┌──────────┐      ┌──────────┐       │
│   │  Node 1  │ ⇄ ⇄ │  Node 2  │ ⇄ ⇄ │  Node 3  │       │
│   └──────────┘      └──────────┘      └──────────┘       │
└──────────────────────────────────────────────────────────┘

## To run the code
1. clone the repository: git clone [https://github.com/ir4577/DS--Mini-Project-Music-PLayer.git](https://github.com/ir4577/DS--Mini-Project-Music-PLayer.git)
cd DS--Mini-Project-Music-PLayer
2. compile the code: gcc main.c dll_engine.c ui_frontend.c -o player.exe
3. launch the web on loacal host: .\player.exe
