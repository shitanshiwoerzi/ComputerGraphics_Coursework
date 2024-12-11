#pragma once
#include "mathLib.h"

class Player {
public:
	mathLib::Vec3 position;
	mathLib::Vec3 velocity;    // 主角速度
	float speed;               // movement speed
	mathLib::Quaternion rotation; // 主角朝向（旋转）
	animatedModel* model;
	AABB boundingBox;

	Player(const mathLib::Vec3& startPos, float moveSpeed, animatedModel* _model)
		: position(startPos), velocity(0.0f, 0.0f, 0.0f), speed(moveSpeed), model(_model) {
	}

	// 更新主角位置
	void move(mathLib::Vec3& direction, float deltaTime, AABB& obstacle) {
		velocity = direction * speed;

		if (direction.getLengthSquare() > 0.0f) {
			mathLib::Vec3 newPosition = position + velocity * deltaTime;

			// 更新碰撞盒
			AABB newBoundingBox = boundingBox;
			newBoundingBox.min += (newPosition - position);
			newBoundingBox.max += (newPosition - position);

			// 检测碰撞
			bool hasCollision = false;
			if (newBoundingBox.intersects(obstacle))
				hasCollision = true;

			// 如果没有碰撞，更新位置
			if (!hasCollision) {
				position = newPosition;
				updateBoundingBox();
			}

		}
		// 更新朝向
		rotation = mathLib::Quaternion::fromAxisAngle(mathLib::Vec3(0, 1, 0), atan2(direction.x, direction.z));
	}

	// 更新主角模型的动画状态
	void updateAnimation(const std::string& animationName, float deltaTime) {
		if (model) {
			model->instance.update(animationName, deltaTime);
		}
	}

	//// 更新主角模型的碰撞盒位置
	//void updateBoundingBox() {
	//	if (model) {
	//		boundingBox = model->bounds; // 获取模型的包围盒
	//		model->bounds.min += position; // 平移包围盒
	//		model->bounds.max += position;
	//	}
	//}

	void updateBoundingBox() {
		// 获取模型的原始碰撞盒
		mathLib::Vec3 localMin = model->bounds.min;
		mathLib::Vec3 localMax = model->bounds.max;

		// 原始碰撞盒的八个顶点
		mathLib::Vec3 corners[8] = {
			mathLib::Vec3(localMin.x, localMin.y, localMin.z),
			mathLib::Vec3(localMin.x, localMin.y, localMax.z),
			mathLib::Vec3(localMin.x, localMax.y, localMin.z),
			mathLib::Vec3(localMin.x, localMax.y, localMax.z),
			mathLib::Vec3(localMax.x, localMin.y, localMin.z),
			mathLib::Vec3(localMax.x, localMin.y, localMax.z),
			mathLib::Vec3(localMax.x, localMax.y, localMin.z),
			mathLib::Vec3(localMax.x, localMax.y, localMax.z),
		};

		// 初始化新的 AABB
		mathLib::Vec3 transformedMin(FLT_MAX, FLT_MAX, FLT_MAX);
		mathLib::Vec3 transformedMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		mathLib::Matrix scaling = mathLib::Matrix::scaling(mathLib::Vec3(0.01f, 0.01f, 0.01f));
		mathLib::Matrix translation = mathLib::Matrix::translation(position);
		mathLib::Matrix rotationMatrix = rotation.toMatrix().rotateY(180);
		mathLib::Matrix worldMatrix = rotationMatrix * scaling * translation;

		// 变换每个顶点，并更新 AABB
		for (auto& corner : corners) {
			mathLib::Vec3 transformedCorner = worldMatrix.mulPoint(corner);

			transformedMin.x = min(transformedMin.x, transformedCorner.x);
			transformedMin.y = min(transformedMin.y, transformedCorner.y);
			transformedMin.z = min(transformedMin.z, transformedCorner.z);

			transformedMax.x = max(transformedMax.x, transformedCorner.x);
			transformedMax.y = max(transformedMax.y, transformedCorner.y);
			transformedMax.z = max(transformedMax.z, transformedCorner.z);
		}

		// 更新碰撞盒
		boundingBox.min = transformedMin;
		boundingBox.max = transformedMax;
	}

	// 渲染主角模型
	void draw(DxCore* core, Shader* shader, textureManager textures, sampler sam, mathLib::Matrix& vp) {
		if (model) {
			// 计算世界矩阵，包含缩放,平移和旋转
			mathLib::Matrix scaling = mathLib::Matrix::scaling(mathLib::Vec3(0.01f, 0.01f, 0.01f));
			mathLib::Matrix translation = mathLib::Matrix::translation(position);
			mathLib::Matrix rotationMatrix = rotation.toMatrix().rotateY(180);
			mathLib::Matrix worldMatrix = rotationMatrix * scaling * translation;

			model->draw(core, shader, textures, sam, worldMatrix, vp);
		}
	}

	// 与地形或物体碰撞检测（简化为地面高度）
	void stayOnGround(float groundHeight) {
		position.y = groundHeight;
	}
};