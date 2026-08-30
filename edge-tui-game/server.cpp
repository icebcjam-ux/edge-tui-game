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

#include "balancer_module.hpp"
#include "cluster_module.hpp"

BalancerModule g_balancer;
ClusterModule  g_cluster;

#define PORT 8888
#define MAP_WIDTH 60
#define MAP_HEIGHT 20
#define NODE_COUNT 5  // 擴充至 5 個節點

// ANSI 顏色控制碼
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

auto g_server_start_time = std::chrono::steady_clock::now();

struct NodeStats {
    uint32_t laps = 0;
    uint32_t total_packets = 0;
    float current_speed = 0.0f;
    std::chrono::steady_clock::time_point first_seen;
    std::chrono::steady_clock::time_point last_seen;
    bool active = false;
};

NodeStats node_stats[NODE_COUNT];

void draw_tui(const GameFrame& frame) {
    auto current_time = std::chrono::steady_clock::now();

    std::cout << "\033[H";
    std::cout << "==================== 異質極速陣列 TUI Dashboard ====================\n";

    char map_grid[MAP_HEIGHT][MAP_WIDTH];
    for (int r = 0; r < MAP_HEIGHT; ++r) {
        for (int c = 0; c < MAP_WIDTH; ++c) {
            if (r == 0 || r == MAP_HEIGHT - 1 || c == 0 || c == MAP_WIDTH - 1)
                map_grid[r][c] = '#';
            else
                map_grid[r][c] = '.';
        }
    }

    // 車輛圖示：Pi5(P), Pi3B(A), Pi3B+(B), Zero2W(Z), ESP32(E)
    char icons[NODE_COUNT] = {'P', 'A', 'B', 'Z', 'E'};
    for (int i = 0; i < NODE_COUNT; ++i) {
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
            else if (ch == 'E') std::cout << MAGENTA << ch << RESET;
            else std::cout << ch;
        }
        std::cout << "\n";
    }

    std::cout << "------------------------------------------------------------\n";
    std::cout << "Frame Seq: " << frame.frame_seq << "\n";

    const char* node_names[NODE_COUNT] = {
        "[0] Pi 5    (P)", 
        "[1] Pi 3B   (A)", 
        "[2] Pi 3B+  (B)", 
        "[3] Zero 2W (Z)",
        "[4] ESP32   (E)"
    };
    const char* colors[NODE_COUNT] = {BLUE, GREEN, YELLOW, RED, MAGENTA};

    for (int i = 0; i < NODE_COUNT; ++i) {
        long node_uptime_sec = 0;
        if (node_stats[i].active) {
            node_uptime_sec = std::chrono::duration_cast<std::chrono::seconds>(current_time - node_stats[i].first_seen).count();
        }

        int n_h = node_uptime_sec / 3600;
        int n_m = (node_uptime_sec % 3600) / 60;
        int n_s = node_uptime_sec % 60;

        std::cout << colors[i] << node_names[i] << RESET
                  << " X: " << std::setw(2) << (int)frame.vehicles[i].pos_x
                  << " | " << CYAN << "Laps: " << RESET << std::setw(3) << node_stats[i].laps
                  << " | " << CYAN << "Spd: " << RESET << std::fixed << std::setprecision(1) << std::setw(4) << node_stats[i].current_speed << " step/s"
                  << " | " << CYAN << "Node Time: " << RESET
                  << std::setfill('0') << std::setw(2) << n_h << ":"
                  << std::setfill('0') << std::setw(2) << n_m << ":"
                  << std::setfill('0') << std::setw(2) << n_s << std::setfill(' ')
                  << "\n";
    }

    auto total_elapsed_sec = std::chrono::duration_cast<std::chrono::seconds>(current_time - g_server_start_time).count();
    int g_h = total_elapsed_sec / 3600;
    int g_m = (total_elapsed_sec % 3600) / 60;
    int g_s = total_elapsed_sec % 60;

    std::cout << "------------------------------------------------------------\n";
    std::cout << CYAN << "Total Uptime : " << RESET 
              << std::setfill('0') << std::setw(2) << g_h << ":"
              << std::setfill('0') << std::setw(2) << g_m << ":"
              << std::setfill('0') << std::setw(2) << g_s << std::setfill(' ')
              << " | " << CYAN << "Total Frame Loops: " << RESET << frame.frame_seq << "\n";
}

// ... 前面 include 與結構體保持不變 ...

int main() {
    // ==========================================
    //  🎮 Server 啟動選單：動態平衡機制選擇
    // ==========================================
    // 1. 啟動選單 (在畫面清空前執行)
    g_balancer.setup_interactive_menu();
    bool enable_speed_balancing = false;

    std::cout << "\n========================================" << std::endl;
    std::cout << " 🖥️  Edge TUI Game Server 啟動中..." << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "請選擇速度與動態平衡模式：" << std::endl;
    std::cout << " [1] 純粹真實速度 (關閉平衡，完全展現晶片肌肉硬實力)" << std::endl;
    std::cout << " [2] 動態平衡模式 (開啟平衡，自動補償弱節點速度)" << std::endl;
    std::cout << "請輸入選項 (1-2, 預設為 1): ";

    std::string user_input;
    std::getline(std::cin, user_input);
    if (!user_input.empty() && user_input == "2") {
        enable_speed_balancing = true;
        std::cout << "\n>>> 已啟用：動態平衡模式\n" << std::endl;
    } else {
        enable_speed_balancing = false;
        std::cout << "\n>>> 已啟用：純粹真實速度模式 (無動態補償)\n" << std::endl;
    }

    // ==========================================
    //  UDP Socket 初始化與監聽
    // ==========================================
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return 1;

    sockaddr_in server_addr{}, client_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) return 1;

    
    std::cout << "\033[2J\033[?25l"; // 清空螢幕並隱藏游標

    GameFrame frame{};
    frame.frame_seq = 0;

    for (int i = 0; i < NODE_COUNT; ++i) {
        frame.vehicles[i].pos_x = 1.0f;
        frame.vehicles[i].pos_y = i + 3 + 2;
    }

    socklen_t addr_len = sizeof(client_addr);
    VehicleInput input_pkt;

    g_server_start_time = std::chrono::steady_clock::now();

    while (true) {
        auto fps_start_time = std::chrono::high_resolution_clock::now();
        // 1. 非阻塞式收取所有已到達的 UDP 封包
        socklen_t addr_len = sizeof(client_addr);
        VehicleInput input_pkt{};
        
        while (true) {
            ssize_t bytes = recvfrom(sockfd, &input_pkt, sizeof(input_pkt), MSG_DONTWAIT,
                                     (struct sockaddr*)&client_addr, &addr_len);
            if (bytes <= 0) break;

            uint8_t id = input_pkt.vehicle_id;
            if (id < NODE_COUNT) {
                auto now = std::chrono::steady_clock::now();

                if (!node_stats[id].active) {
                    node_stats[id].active = true;
                    node_stats[id].first_seen = now;
                }
                node_stats[id].last_seen = now;
                node_stats[id].total_packets++;

                // 計算單次移動步階
                float move_step = input_pkt.throttle * 0.5f;

                // 若開啟動態平衡，針對較慢的節點給予步階加權補償
                if (enable_speed_balancing) {
                    if (id == 1) move_step *= 1.8f;      // Pi 3B 補償
                    else if (id == 2) move_step *= 2.2f; // Pi 3B+ 補償
                    else if (id == 3) move_step *= 2.0f; // Zero 2W 補償
                }

                node_stats[id].current_speed = move_step * 60.0f;

                frame.vehicles[id].pos_x += move_step;

                if (frame.vehicles[id].pos_x >= (float)(MAP_WIDTH - 2)) {
                    frame.vehicles[id].pos_x = 1.0f;
                    node_stats[id].laps++;
                }
            }
        }

        frame.frame_seq++;
        draw_tui(frame);

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> elapsed = end_time - fps_start_time;
        if (elapsed.count() < 16.6f) {
            std::this_thread::sleep_for(std::chrono::milliseconds((int)(16.6f - elapsed.count())));
        }
    }

    std::cout << "\033[?25h";
    close(sockfd);
    return 0;
}