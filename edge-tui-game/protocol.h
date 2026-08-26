// protocol.h
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint> // 放最上面，引入 uint8_t, uint32_t

// 強制 1 位元組對齊，避免跨平台 Padding 差異
#pragma pack(push, 1)

// 賽車/飛機 傳給 Pi 5 的控制指令 (Client -> Server)
struct VehicleInput {
    uint8_t vehicle_id;      // 0: Pi5, 1: 3B, 2: 3B+, 3: Zero2W
    float   throttle;        // 油門 (0.0 ~ 1.0)
    float   steering_angle;  // 轉向角 / 俯仰角 (弧度)
    bool    boost;           // 氮氣加速 / 後燃器開啟
};

// Pi 5 廣播給全場的動態狀態 (Server -> Client)
struct VehicleState {
    uint8_t id;
    float   pos_x, pos_y, pos_z; // 3D 座標
    float   velocity_x, velocity_y; // 速度向量
    float   heading;             // 朝向
    uint8_t health_or_lap;       // 血量或圈數
};

struct GameFrame {
    uint32_t     frame_seq;       // 幀序號
    uint8_t      active_vehicles;
    VehicleState vehicles[4];
};

#pragma pack(pop)

#endif