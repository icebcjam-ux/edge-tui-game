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
#define CYAN    "\033[36m"

// 全域整體啟動時間
auto g_server_start_time = std::chrono::steady_clock::now();

// 節點個別統計資料結構
struct NodeStats {
    uint32_t laps = 0;              // 跑了幾圈
    uint32_t total_packets = 0;     // 累積收包數（計算跑多久與活躍度）
    float current_speed = 0.0f;     // 即時速度 (x/sec)
    std::chrono::steady_clock::time_point first_seen; // 首次上線時間
    std::chrono::steady_clock::time_point last_seen;  // 上次收到封包時間
    bool active = false;
};

NodeStats node_stats[4];

void draw_tui(const GameFrame& frame) {
    auto current_time = std::chrono::steady_clock::now();

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

    // 顯示全場數據面板與各節點個別數據
    std::cout << "------------------------------------------------------------\n";
    std::cout << "Frame Seq: " << frame.frame_seq << "\n";

    const char* node_names[4] = {"[0] Pi 5    (P)", "[1] Pi 3B   (A)", "[2] Pi 3B+  (B)", "[3] Zero 2W (Z)"};
    const char* colors[4]     = {BLUE, GREEN, YELLOW, RED};

    for (int i = 0; i < 4; ++i) {
        // 計算該節點運行時間 (Node Uptime)
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

    // 整體系統統計資訊
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

    for (int i = 0; i < 4; ++i) {
        frame.vehicles[i].pos_x = 1.0f;
        frame.vehicles[i].pos_y = i * 4 + 3;
    }

    socklen_t addr_len = sizeof(client_addr);
    VehicleInput input_pkt;

    g_server_start_time = std::chrono::steady_clock::now();

    while (true) {
        auto fps_start_time = std::chrono::high_resolution_clock::now();

        // 收取 UDP 封包並更新數據
        while (true) {
            ssize_t bytes = recvfrom(sockfd, &input_pkt, sizeof(input_pkt), MSG_DONTWAIT,
                                     (struct sockaddr*)&client_addr, &addr_len);
            if (bytes <= 0) break;

            uint8_t id = input_pkt.vehicle_id;
            if (id < 4) {
                auto now = std::chrono::steady_clock::now();

                // 首次上線記錄
                if (!node_stats[id].active) {
                    node_stats[id].active = true;
                    node_stats[id].first_seen = now;
                }
                node_stats[id].last_seen = now;
                node_stats[id].total_packets++;

                // 計算移動與圈數 (Lap Count)
                float move_step = input_pkt.throttle * 0.5f;
                node_stats[id].current_speed = move_step * 60.0f; // 估算每秒移動距離

                frame.vehicles[id].pos_x += move_step;

                // 觸發衝線過關 -> 圈數 +1
                if (frame.vehicles[id].pos_x >= (float)(MAP_WIDTH - 2)) {
                    frame.vehicles[id].pos_x = 1.0f;
                    node_stats[id].laps++;
                }
            }
        }

        frame.frame_seq++;
        draw_tui(frame);

        // 60 FPS 幀率控制
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