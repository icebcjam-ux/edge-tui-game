// GAME/server.cpp
#include <iostream>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "protocol.h"

#define PORT 8888
#define MAP_WIDTH 60
#define MAP_HEIGHT 20

// ANSI 顏色控制碼
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"

// --- 全域變數（解決 Scope 區域問題）---
auto g_start_time = std::chrono::steady_clock::now();
std::string g_start_time_str = "";

// 取得當前系統時間字串 (HH:MM:SS)
std::string get_start_time_str() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    struct tm* timeinfo = localtime(&now_time);
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
    return std::string(buffer);
}

void draw_tui(const GameFrame& frame) {
    // 1. 計算總運行時間 (秒)
    auto current_time = std::chrono::steady_clock::now();
    auto elapsed_sec = std::chrono::duration_cast<std::chrono::seconds>(current_time - g_start_time).count();

    int hours = elapsed_sec / 3600;
    int minutes = (elapsed_sec % 3600) / 60;
    int seconds = elapsed_sec % 60;

    // 游標位移至 Terminal 左上角
    std::cout << "\033[H";
    std::cout << "==================== 樹莓派分散式點陣競速場 ====================\n";

    // 初始化二維地圖陣列
    char map_grid[MAP_HEIGHT][MAP_WIDTH];
    for (int r = 0; r < MAP_HEIGHT; ++r) {
        for (int c = 0; c < MAP_WIDTH; ++c) {
            if (r == 0 || r == MAP_HEIGHT - 1 || c == 0 || c == MAP_WIDTH - 1)
                map_grid[r][c] = '#';
            else
                map_grid[r][c] = '.';
        }
    }

    // 將車輛位置映射至二維網格
    char icons[4] = {'P', 'A', 'B', 'Z'};
    for (int i = 0; i < 4; ++i) {
        int gx = (int)frame.vehicles[i].pos_x;
        int gy = (int)frame.vehicles[i].pos_y;

        if (gx >= 1 && gx < MAP_WIDTH - 1 && gy >= 1 && gy < MAP_HEIGHT - 1) {
            map_grid[gy][gx] = icons[i];
        }
    }

    // 繪製地圖
    for (int r = 0; r < MAP_HEIGHT; ++r) {
        for (int c = 0; c < MAP_WIDTH; ++c) {
            char ch = map_grid[r][c];
            if (ch == 'P') std::cout << BLUE << ch << RESET;
            else if (ch == 'A') std::cout << GREEN << ch << RESET;
            else if (ch == 'B') std::cout << YELLOW << ch << RESET;
            else if (ch == 'Z') std::cout << RED << ch << RESET;
            else std::cout << ch;
        }
        std::cout << "\n";
    }

    // 顯示全場數據面板
    std::cout << "------------------------------------------------------------\n";
    std::cout << "Frame Seq: " << frame.frame_seq << "\n";
    std::cout << BLUE   << "[0] Pi 5    (P) " << RESET << "X: " << (int)frame.vehicles[0].pos_x << " Y: " << (int)frame.vehicles[0].pos_y << "\n";
    std::cout << GREEN  << "[1] Pi 3B   (A) " << RESET << "X: " << (int)frame.vehicles[1].pos_x << " Y: " << (int)frame.vehicles[1].pos_y << "\n";
    std::cout << YELLOW << "[2] Pi 3B+  (B) " << RESET << "X: " << (int)frame.vehicles[2].pos_x << " Y: " << (int)frame.vehicles[2].pos_y << "\n";
    std::cout << RED    << "[3] Zero 2W (Z) " << RESET << "X: " << (int)frame.vehicles[3].pos_x << " Y: " << (int)frame.vehicles[3].pos_y << "\n";

    // 2. 格式化輸出到框選區域
    std::cout << "\033[36mStart Time :\033[0m " << g_start_time_str << "\n";
    std::cout << "\033[36mUptime     :\033[0m "
              << std::setfill('0') << std::setw(2) << hours << ":"
              << std::setfill('0') << std::setw(2) << minutes << ":"
              << std::setfill('0') << std::setw(2) << seconds << "\n";
    std::cout << "\033[36mLoop Count :\033[0m " << frame.frame_seq << " loops\n";
}

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return 1;

    sockaddr_in server_addr{}, client_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) return 1;

    // 清空畫面並隱藏游標
    std::cout << "\033[2J\033[?25l";

    GameFrame frame{};
    frame.frame_seq = 0;

    // 給予初始位置
    for (int i = 0; i < 4; ++i) {
        frame.vehicles[i].pos_x = 1.0f;
        frame.vehicles[i].pos_y = i * 4 + 3;
    }

    socklen_t addr_len = sizeof(client_addr);
    VehicleInput input_pkt;

    // 初始化啟動時間點與時間字串
    g_start_time = std::chrono::steady_clock::now();
    g_start_time_str = get_start_time_str();

    while (true) {
        auto fps_start_time = std::chrono::high_resolution_clock::now();

        // 滿空嘗試接收所有堆積的 UDP 封包
        while (true) {
            ssize_t bytes = recvfrom(sockfd, &input_pkt, sizeof(input_pkt), MSG_DONTWAIT,
                                     (struct sockaddr*)&client_addr, &addr_len);
            if (bytes <= 0) break;

            uint8_t id = input_pkt.vehicle_id;
            if (id < 4) {
                frame.vehicles[id].pos_x += input_pkt.throttle * 0.5f;

                if (frame.vehicles[id].pos_x >= (float)(MAP_WIDTH - 2)) {
                    frame.vehicles[id].pos_x = 1.0f;
                }
            }
        }

        frame.frame_seq++;
        draw_tui(frame);

        // 60 FPS 渲染控制
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> elapsed = end_time - fps_start_time;
        if (elapsed.count() < 16.6f) {
            std::this_thread::sleep_for(std::chrono::milliseconds((int)(16.6f - elapsed.count())));
        }
    }

    std::cout << "\033[?25h"; // 恢復游標
    close(sockfd);
    return 0;
}