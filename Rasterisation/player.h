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
	std::string currentAnimation;
	bool isAttacking = false; // 是否正在播放攻击动画
	float attackAnimationTime = 0.0f; // 当前攻击动画播放时间
	float attackDuration = 1.0f; // 攻击动画的总持续时间


	Player(const mathLib::Vec3& startPos, float moveSpeed, animatedModel* _model)
		: position(startPos), velocity(0.0f, 0.0f, 0.0f), speed(moveSpeed), model(_model) {
		rotation = mathLib::Quaternion::fromAxisAngle(mathLib::Vec3(1, 0, 0), M_PI);
	}

	void update(mathLib::Vec3 direction, mathLib::Vec3 forward, AABB& obstacle, float deltaTime) {
		// 更新主角的动画状态
		if (direction.getLengthSquare() > 0.0f) {
			updateRotation(direction); // 更新朝向
			updateAnimation("Run", deltaTime); // 切换到运行动画
			move(direction, deltaTime, obstacle);
		}
		else {
			updateAnimation("Idle", deltaTime);    // 待机动画
		}
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

			//// 检测碰撞
			//bool hasCollision = false;
			//if (newBoundingBox.intersects(obstacle))
			//	hasCollision = true;

			//// 如果没有碰撞，更新位置
			//if (!hasCollision) {
			//	position = newPosition;
			//	updateBoundingBox();
			//}

					// 检测碰撞
			if (newBoundingBox.intersects(obstacle)) {
				// 计算碰撞法线
				mathLib::Vec3 collisionNormal = calculateCollisionNormal(obstacle);

				// 调整速度方向（滑动方向）
				velocity = velocity - collisionNormal * velocity.dot(collisionNormal);

				// 更新位置
				newPosition = position + velocity * deltaTime;

				// 再次检查新位置的碰撞盒
				newBoundingBox.min += (newPosition - position);
				newBoundingBox.max += (newPosition - position);

				// 确保新的位置不再与障碍物碰撞
				if (!newBoundingBox.intersects(obstacle)) {
					position = newPosition;
					updateBoundingBox();
				}
			}
			else {
				// 如果没有碰撞，正常更新位置
				position = newPosition;
				updateBoundingBox();
			}
		}
	}

	// 更新朝向
	void updateRotation(mathLib::Vec3& moveDirection) {
		if (moveDirection.getLengthSquare() > 0.0f) {
			float angle = atan2(-moveDirection.x, -moveDirection.z);
			rotation = mathLib::Quaternion::fromAxisAngle(mathLib::Vec3(0, 1, 0), angle);
			rotation = rotation * mathLib::Quaternion::fromAxisAngle(mathLib::Vec3(1, 0, 0), M_PI); // 修正 X 轴翻转
		}
	}

	// 更新主角模型的动画状态
	void updateAnimation(const std::string& animationName, float deltaTime) {
		//if (model) {
		//	model->instance.update(animationName, deltaTime);
		//}
		if (model && currentAnimation != animationName) {
			currentAnimation = animationName;
			model->instance.update(animationName, deltaTime);
		}
		else if (model) {
			model->instance.update(animationName, deltaTime);
		}
	}

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

		mathLib::Matrix scaling = mathLib::Matrix::scaling(mathLib::Vec3(0.3f, 0.3f, 0.3f));
		mathLib::Matrix translation = mathLib::Matrix::translation(position);
		mathLib::Matrix rotationMatrix = rotation.toMatrix();
		mathLib::Matrix worldMatrix = scaling * rotationMatrix * translation;

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
			mathLib::Matrix scaling = mathLib::Matrix::scaling(mathLib::Vec3(0.3f, 0.3f, 0.3f));
			mathLib::Matrix translation = mathLib::Matrix::translation(position);
			mathLib::Matrix rotationMatrix = rotation.toMatrix();
			mathLib::Matrix worldMatrix = scaling * rotationMatrix * translation;

			model->draw(core, shader, textures, sam, worldMatrix, vp);
		}
	}

	// 与地形或物体碰撞检测（简化为地面高度）
	void stayOnGround(float groundHeight) {
		position.y = groundHeight;
	}

	// 计算碰撞法线（针对 AABB）
	mathLib::Vec3 calculateCollisionNormal(AABB& obstacle) {
		// 假设玩家的 AABB 边界与障碍物重叠，计算最近的接触面法线
		mathLib::Vec3 normal;

		float dx1 = obstacle.max.x - boundingBox.min.x;
		float dx2 = boundingBox.max.x - obstacle.min.x;
		float dy1 = obstacle.max.y - boundingBox.min.y;
		float dy2 = boundingBox.max.y - obstacle.min.y;
		float dz1 = obstacle.max.z - boundingBox.min.z;
		float dz2 = boundingBox.max.z - obstacle.min.z;

		float minDist = min(min(dx1, dx2), min(dy1, dy2), min(dz1, dz2));
		if (minDist == dx1) normal = mathLib::Vec3(-1, 0, 0); // 碰撞在左侧
		else if (minDist == dx2) normal = mathLib::Vec3(1, 0, 0); // 碰撞在右侧
		else if (minDist == dy1) normal = mathLib::Vec3(0, -1, 0); // 碰撞在底部
		else if (minDist == dy2) normal = mathLib::Vec3(0, 1, 0); // 碰撞在顶部
		else if (minDist == dz1) normal = mathLib::Vec3(0, 0, -1); // 碰撞在前方
		else if (minDist == dz2) normal = mathLib::Vec3(0, 0, 1); // 碰撞在后方
		return normal;
	}
};