#ifndef CLUSTER_MODULE_HPP
#define CLUSTER_MODULE_HPP

#include <chrono>
#include <cstdint>

struct NodeMember {
    uint8_t id;
    bool active = false;
    uint32_t laps = 0;
    uint32_t total_packets = 0;
    float current_speed = 0.0f;
    std::chrono::steady_clock::time_point first_seen;
    std::chrono::steady_clock::time_point last_seen;
};

class ClusterModule {
private:
    static constexpr int MAX_NODES = 5;
    NodeMember members[MAX_NODES];

public:
    ClusterModule() {
        for (int i = 0; i < MAX_NODES; ++i) {
            members[i].id = i;
        }
    }

    void register_packet(uint8_t id, float move_step) {
        if (id >= MAX_NODES) return;

        auto now = std::chrono::steady_clock::now();
        if (!members[id].active) {
            members[id].active = true;
            members[id].first_seen = now;
        }
        members[id].last_seen = now;
        members[id].total_packets++;
        members[id].current_speed = move_step * 60.0f;
    }

    NodeMember& get_member(uint8_t id) { return members[id]; }
    int get_max_nodes() const { return MAX_NODES; }
};

#endif