#ifndef BALANCER_MODULE_HPP
#define BALANCER_MODULE_HPP

#include <iostream>
#include <string>
#include <cstdint>

class BalancerModule {
private:
    bool enabled = false;

public:
    void setup_interactive_menu() {
        std::cout << "\n========================================" << std::endl;
        std::cout << " ⚖️  [動態平衡模組] 補償機制設定" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << " [1] 關閉平衡 (展現真實硬體極限速度)" << std::endl;
        std::cout << " [2] 開啟平衡 (自動加權補償慢速節點)" << std::endl;
        std::cout << "請選擇 (預設 1): ";

        std::string input;
        std::getline(std::cin, input);
        enabled = (!input.empty() && input == "2");
    }

    float calculate_step(uint8_t node_id, float throttle) const {
        float step = throttle * 0.5f;
        if (!enabled) return step;

        // 依據 Node 晶片給予平衡權重
        switch (node_id) {
            case 1:  return step * 1.8f; // Pi 3B
            case 2:  return step * 2.2f; // Pi 3B+
            case 3:  return step * 2.0f; // Zero 2W
            default: return step;        // Pi 5 / ESP32
        }
    }

    bool is_enabled() const { return enabled; }
};

#endif