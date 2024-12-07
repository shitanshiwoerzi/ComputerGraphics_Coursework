#pragma once
#include "mathLib.h"

class Player {
public:
    mathLib::Vec3 position;
    mathLib::Vec3 velocity;    // 主角速度
    float speed;               // movement speed

    Player(const mathLib::Vec3& startPos, float moveSpeed)
        : position(startPos), velocity(0.0f, 0.0f, 0.0f), speed(moveSpeed) {}

    // 更新主角位置
    void move(mathLib::Vec3& direction, float deltaTime) {
        position += direction * speed * deltaTime;
    }

    // 与地形或物体碰撞检测（简化为地面高度）
    void stayOnGround(float groundHeight) {
        position.y = groundHeight;
    }
};