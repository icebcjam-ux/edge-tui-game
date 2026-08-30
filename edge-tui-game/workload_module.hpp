#ifndef WORKLOAD_MODULE_HPP
#define WORKLOAD_MODULE_HPP

#include <iostream>
#include <cmath>
#include <string>

enum class WorkloadMode {
    BENCHMARK = 1, // 裸跑純算力
    DYNAMIC = 2,   // 動態加壓
    CUSTOM = 3     // 客製化固定負載
};

class WorkloadModule {
private:
    WorkloadMode mode = WorkloadMode::BENCHMARK;
    int current_weight_kg = 0;
    int packet_counter = 0;

public:
    void setup_interactive_menu() {
        std::cout << "\n========================================" << std::endl;
        std::cout << " 🛠️  [載重模組] 負載模式設定" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << " [1] 裸跑 Benchmark (純粹硬體算力)" << std::endl;
        std::cout << " [2] 動態加壓 (每 100 封包 +1 KG)" << std::endl;
        std::cout << " [3] 自訂固定負載 (KG)" << std::endl;
        std::cout << "請選擇 (預設 1): ";

        std::string input;
        std::getline(std::cin, input);
        int opt = input.empty() ? 1 : std::stoi(input);

        if (opt == 2) mode = WorkloadMode::DYNAMIC;
        else if (opt == 3) {
            mode = WorkloadMode::CUSTOM;
            std::cout << "輸入設定重量 (KG): ";
            std::getline(std::cin, input);
            current_weight_kg = input.empty() ? 5 : std::stoi(input);
        } else {
            mode = WorkloadMode::BENCHMARK;
        }
    }

    void execute_workload() {
        packet_counter++;
        if (mode == WorkloadMode::DYNAMIC && packet_counter % 100 == 0) {
            current_weight_kg++;
        }

        if (current_weight_kg <= 0) return;

        volatile double dummy = 0.0;
        int iterations = current_weight_kg * 100000;
        for (int i = 0; i < iterations; ++i) {
            dummy += std::sin(i) * std::cos(i) * std::atan(i);
        }
    }

    int get_weight() const { return current_weight_kg; }
};

#endif