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

int main(int argc, char* argv[]) {
    // 預設為 Node 3 (Zero 2W)，若有輸入參數則帶入指定 ID
    uint8_t my_id = (argc > 1) ? static_cast<uint8_t>(std::stoi(argv[1])) : 3;
    int node_id = static_cast<int>(my_id);

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

    std::cout << "[Bot Agent Node " << (int)my_id << "] 啟動，準備發送數據..." << std::endl;

    int packet_count = 0;
    int weights_kg = 0; // 當前掛了幾公斤鉛塊

    while (true) {
        packet_count++;

        // 只有 Pi 5 (Node 0) 會觸發「每發送 100 個封包，就自動加 1 KG 鉛塊」
        if (node_id == 0 && packet_count % 100 == 0) {
            weights_kg += 1;
            std::cout << "\n[Pi 5 健身房] 教練又幫你加了 1 KG 鉛塊！目前總重：" << weights_kg << " KG\n" << std::endl;
        }

        auto start = std::chrono::high_resolution_clock::now();

        // 1. 執行比賽專用的動態鉛塊負載運算
        if (weights_kg > 0) {
            do_handicap_work(weights_kg);
        }

        // 2. 打包油門指令發送給 Server
        sendto(sockfd, &my_input, sizeof(my_input), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> elapsed = end - start;

        std::cout << "[Node " << node_id << "] 算力耗時: " << elapsed.count() << " ms (負重: " << weights_kg << " kg)" << std::endl;

        // 控制發送頻率 (約 60Hz)
        usleep(16000);
    }

    close(sockfd);
    return 0;
}