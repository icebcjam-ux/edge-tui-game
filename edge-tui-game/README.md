# Edge TUI Game

一個基於 UDP Socket 與 C++ 打造的分散式邊緣運算 TUI 點陣地圖遊戲。

## 系統架構

* **Server (`server.cpp`)**: 運行於主控端（Raspberry Pi 5），負責監聽 UDP 8888 埠並渲染 Terminal TUI 地圖。
* **Agent (`bot_agent.cpp`)**: 運行於各邊緣節點（Pi 3B / Pi 3B+ / Zero 2W），負責運算並傳送節點數據。

## Quick Start

### 1. 編譯專案

```bash
g++ -O3 server.cpp -o server
g++ -O3 bot_agent.cpp -o bot_agent