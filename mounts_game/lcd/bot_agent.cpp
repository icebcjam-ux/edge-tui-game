// GAME/bot_agent.cpp
#include <iostream>
#include <cstring>
#include <cmath>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include "protocol.h"

#define SERVER_IP "192.168.1.112" // 丟到小卡時改為 Pi 5 的 IP
#define PORT 8888

// 算力壓測函式：模擬高負載 3D 軌跡預測
void burn_cpu_stress_test() {
    volatile double dummy = 0.0;
    for (int i = 0; i < 50000; ++i) {
        dummy += std::sin(i) * std::cos(i) * std::atan(i);
    }
}

int main(int argc, char* argv[]) {
    // 預設為 Node 3 (Zero 2W)，若有輸入參數則帶入指定 ID
    uint8_t my_id = (argc > 1) ? static_cast<uint8_t>(std::stoi(argv[1])) : 3;

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

    std::cout << "[Bot Agent Node " << (int)my_id << "] 啟動，準備壓測連線..." << std::endl;
    
    // ... 後續的 while(true) 壓測迴圈 ...

    while (true) {
        auto t1 = std::chrono::high_resolution_clock::now();

        // 1. 執行 C++ 算力壓測
        burn_cpu_stress_test();

        // 2. 回傳操作指令給 Pi 5
        sendto(sockfd, &my_input, sizeof(my_input), 0, 
               (struct sockaddr*)&server_addr, sizeof(server_addr));

        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms = t2 - t1;

        std::cout << "[Node " << (int)my_id << "] 幀算力耗時: " << ms.count() << " ms" << std::endl;
        
        usleep(16000); // ~60Hz
    }

    close(sockfd);
    return 0;
}