#include <iostream>
#include <cstring>
#include <cmath>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include "protocol.h"

#define SERVER_IP "192.168.1.112" // 請確認為 Pi 5 的 IP
#define PORT 8888

// 健身房鉛塊負載運算 (模擬 CPU 算力負載)
void do_handicap_work(int weights_kg) {
    volatile double dummy = 0.0;
    // 根據公斤數 (weights_kg) 決定計算次數
    for (int i = 0; i < weights_kg * 100000; ++i) {
        dummy += std::sin(i) * std::cos(i) * std::atan(i);
    }
}

// ... 前面 include 與 do_handicap_work 保持不變 ...

int main(int argc, char* argv[]) {
    uint8_t my_id = (argc > 1) ? static_cast<uint8_t>(std::stoi(argv[1])) : 3;
    int node_id = static_cast<int>(my_id);

    // ==========================================
    //  🎮 遊戲開場白：難度 / 負載模式詢問機制
    // ==========================================
    int selected_mode = 0;
    int fixed_weight = 0;

    std::cout << "\n========================================" << std::endl;
    std::cout << " 🚀 邊緣運算節點 Agent 啟動 (Node ID: " << node_id << ")" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "請選擇運算負載模式 (Difficulty Mode)：" << std::endl;
    std::cout << " [1] 輕鬆模式 (裸跑純算力 Benchmark)" << std::endl;
    std::cout << " [2] 動態加壓 (每 100 封包增加 1 KG)" << std::endl;
    std::cout << " [3] 客製化固定負載 (自訂 KG)" << std::endl;
    std::cout << "請輸入選項 (1-3, 預設為 1): ";

    std::string user_input;
    std::getline(std::cin, user_input);
    if (!user_input.empty()) {
        try {
            selected_mode = std::stoi(user_input);
        } catch (...) {
            selected_mode = 1;
        }
    } else {
        selected_mode = 1;
    }

    int weights_kg = 0;
    if (selected_mode == 3) {
        std::cout << "請輸入固定負載重量 (KG): ";
        std::getline(std::cin, user_input);
        try {
            weights_kg = std::stoi(user_input);
        } catch (...) {
            weights_kg = 5;
        }
    }

    std::cout << "\n>>> 模式已確認！開始傳送封包至 Server...\n" << std::endl;

    // ==========================================
    //  UDP Socket 建立與初始化
    // ==========================================
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    VehicleInput my_input{};
    my_input.vehicle_id = my_id;
    my_input.throttle = 1.0f;
    my_input.steering_angle = 0.5f;
    my_input.boost = false;

    int packet_count = 0;

    while (true) {
        packet_count++;

        // 模式 2：才執行每 100 個封包動態加重 1 KG
        if (selected_mode == 2 && packet_count % 100 == 0) {
            weights_kg += 1;
            std::cout << "\n[健身房效應] 重量增加 1 KG！目前總重：" << weights_kg << " KG\n" << std::endl;
        }

        auto start = std::chrono::high_resolution_clock::now();

        // 1. 執行負載運算 (若重量為 0 則不額外加壓)
        if (weights_kg > 0) {
            do_handicap_work(weights_kg);
        }

        // 2. 打包傳送指令給 Server
        sendto(sockfd, &my_input, sizeof(my_input), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> elapsed = end - start;

        std::cout << "[Node " << node_id << "] 算力耗時: " << elapsed.count() 
                  << " ms (負載: " << weights_kg << " kg)" << std::endl;

        usleep(16000); // 模擬 ~60Hz 發送
    }

    close(sockfd);
    return 0;
}