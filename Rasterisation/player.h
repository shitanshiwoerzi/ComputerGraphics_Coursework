#pragma once
#include "mathLib.h"

class Player {
public:
    mathLib::Vec3 position;
    mathLib::Vec3 velocity;    // 主角速度
    float speed;               // movement speed
    animatedModel* model;
    AABB boundingBox;

    Player(const mathLib::Vec3& startPos, float moveSpeed, animatedModel* _model)
        : position(startPos), velocity(0.0f, 0.0f, 0.0f), speed(moveSpeed), model(_model) {
        updateBoundingBox();
    }

    // 更新主角的碰撞盒
    void updateBoundingBox() {
        mathLib::Vec3 halfSize(0.5f, 1.0f, 0.5f); // 假设主角的尺寸
        boundingBox.min = position - halfSize;
        boundingBox.max = position + halfSize;
    }

    bool checkCollision(cube& object) {
        return this->boundingBox.intersects(object.boundingBox);
    }

    void handleCollision(cube& object) {
        if (checkCollision(object)) {
            // 简单的碰撞响应：停止主角移动
            this->velocity = mathLib::Vec3(0.0f, 0.0f, 0.0f);
        }
    }

    // 更新主角位置
    void move(mathLib::Vec3& direction, float deltaTime) {
        velocity = direction * speed;
        position += velocity * deltaTime;
        updateBoundingBox();
    }

    // 更新主角模型的动画状态
    void updateAnimation(const std::string& animationName, float deltaTime) {
        if (model) {
            model->instance.update(animationName, deltaTime);
        }
    }

    // 更新主角模型的世界矩阵
    void updateModel() {
        if (model) {
            model->calculateBoundingBox(); // 更新包围盒
            model->bounds.min += position; // 平移包围盒
            model->bounds.max += position;
        }
    }

    // 渲染主角模型
    void draw(DxCore* core, Shader* shader, textureManager textures, sampler sam, mathLib::Matrix& vp) {
        if (model) {
            mathLib::Matrix worldMatrix = mathLib::Matrix::scaling(mathLib::Vec3(0.01f, 0.01f, 0.01f)) * mathLib::Matrix::translation(position);
            model->draw(core, shader, textures, sam, worldMatrix, vp);
        }
    }

    // 与地形或物体碰撞检测（简化为地面高度）
    void stayOnGround(float groundHeight) {
        position.y = groundHeight;
    }
};