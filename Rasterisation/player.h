#pragma once
#include "mathLib.h"

class Player {
public:
	mathLib::Vec3 position;
	mathLib::Vec3 velocity;    // player's velocity(with direction)
	float speed;               // movement speed
	mathLib::Quaternion rotation; // player's orientation
	animatedModel* model;
	AABB boundingBox;
	std::string currentAnimation;
	bool isAttacking = false; // Whether or not the attack animation is playing
	float attackAnimationTime = 0.0f; // Current attack animation play time
	float attackDuration = 1.0f; // Total duration of the attack animation


	Player(const mathLib::Vec3& startPos, float moveSpeed, animatedModel* _model)
		: position(startPos), velocity(0.0f, 0.0f, 0.0f), speed(moveSpeed), model(_model) {
		rotation = mathLib::Quaternion::fromAxisAngle(mathLib::Vec3(1, 0, 0), M_PI);
	}

	void update(mathLib::Vec3 direction, mathLib::Vec3 forward, AABB& obstacle, float deltaTime) {
		// Update the animation status
		if (direction.getLengthSquare() > 0.0f) {
			updateRotation(direction);
			updateAnimation("Run", deltaTime); // Switch to running animation
			move(direction, deltaTime, obstacle);
		}
		else {
			updateAnimation("Idle", deltaTime);    // switch to idle
		}
	}

	// update player position
	void move(mathLib::Vec3& direction, float deltaTime, AABB& obstacle) {
		velocity = direction * speed;

		if (direction.getLengthSquare() > 0.0f) {
			mathLib::Vec3 newPosition = position + velocity * deltaTime;

			// update boundingBox
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

					// check collision
			if (newBoundingBox.intersects(obstacle)) {
				// compute normal
				mathLib::Vec3 collisionNormal = calculateCollisionNormal(obstacle);

				// Adjustment of speed direction (sliding direction)
				velocity = velocity - collisionNormal * velocity.dot(collisionNormal);

				// Update Location
				newPosition = position + velocity * deltaTime;

				// get new boundingBox
				newBoundingBox.min += (newPosition - position);
				newBoundingBox.max += (newPosition - position);

				// check again
				if (!newBoundingBox.intersects(obstacle)) {
					position = newPosition;
					updateBoundingBox();
				}
			}
			else {
				// if no collision, move
				position = newPosition;
				updateBoundingBox();
			}
		}
	}

	void updateRotation(mathLib::Vec3& moveDirection) {
		if (moveDirection.getLengthSquare() > 0.0f) {
			float angle = atan2(-moveDirection.x, -moveDirection.z);
			rotation = mathLib::Quaternion::fromAxisAngle(mathLib::Vec3(0, 1, 0), angle);
			rotation = rotation * mathLib::Quaternion::fromAxisAngle(mathLib::Vec3(1, 0, 0), M_PI); // 修正 X 轴翻转
		}
	}

	// Update the animation state of the player
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
		// get original boundingBox
		mathLib::Vec3 localMin = model->bounds.min;
		mathLib::Vec3 localMax = model->bounds.max;

		// 8 vertices
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

		// init AABB
		mathLib::Vec3 transformedMin(FLT_MAX, FLT_MAX, FLT_MAX);
		mathLib::Vec3 transformedMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		mathLib::Matrix scaling = mathLib::Matrix::scaling(mathLib::Vec3(0.3f, 0.3f, 0.3f));
		mathLib::Matrix translation = mathLib::Matrix::translation(position);
		mathLib::Matrix rotationMatrix = rotation.toMatrix();
		mathLib::Matrix worldMatrix = scaling * rotationMatrix * translation;

		// transform each vertex
		for (auto& corner : corners) {
			mathLib::Vec3 transformedCorner = worldMatrix.mulPoint(corner);

			transformedMin.x = min(transformedMin.x, transformedCorner.x);
			transformedMin.y = min(transformedMin.y, transformedCorner.y);
			transformedMin.z = min(transformedMin.z, transformedCorner.z);

			transformedMax.x = max(transformedMax.x, transformedCorner.x);
			transformedMax.y = max(transformedMax.y, transformedCorner.y);
			transformedMax.z = max(transformedMax.z, transformedCorner.z);
		}

		// update boundingBox
		boundingBox.min = transformedMin;
		boundingBox.max = transformedMax;
	}

	// render player
	void draw(DxCore* core, Shader* shader, textureManager textures, sampler sam, mathLib::Matrix& vp) {
		if (model) {
			// compute matrix
			mathLib::Matrix scaling = mathLib::Matrix::scaling(mathLib::Vec3(0.3f, 0.3f, 0.3f));
			mathLib::Matrix translation = mathLib::Matrix::translation(position);
			mathLib::Matrix rotationMatrix = rotation.toMatrix();
			mathLib::Matrix worldMatrix = scaling * rotationMatrix * translation;

			model->draw(core, shader, textures, sam, worldMatrix, vp);
		}
	}

	void stayOnGround(float groundHeight) {
		position.y = groundHeight;
	}

	// compute normal(AABB)
	mathLib::Vec3 calculateCollisionNormal(AABB& obstacle) {
		// Assuming that the player's AABB boundary overlaps an obstacle, calculate the nearest contact surface normal
		mathLib::Vec3 normal;

		float dx1 = obstacle.max.x - boundingBox.min.x;
		float dx2 = boundingBox.max.x - obstacle.min.x;
		float dy1 = obstacle.max.y - boundingBox.min.y;
		float dy2 = boundingBox.max.y - obstacle.min.y;
		float dz1 = obstacle.max.z - boundingBox.min.z;
		float dz2 = boundingBox.max.z - obstacle.min.z;

		float minDist = min(min(dx1, dx2), min(dy1, dy2), min(dz1, dz2));
		if (minDist == dx1) normal = mathLib::Vec3(-1, 0, 0); // left
		else if (minDist == dx2) normal = mathLib::Vec3(1, 0, 0); // right
		else if (minDist == dy1) normal = mathLib::Vec3(0, -1, 0); // bottom
		else if (minDist == dy2) normal = mathLib::Vec3(0, 1, 0); // up
		else if (minDist == dz1) normal = mathLib::Vec3(0, 0, -1); // front
		else if (minDist == dz2) normal = mathLib::Vec3(0, 0, 1); // back
		return normal;
	}
};