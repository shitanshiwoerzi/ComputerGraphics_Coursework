#pragma once
#include "mathLib.h"
#include "player.h"

//class FPSCamera {
//public:
//	Player* player;           // 绑定的主角
//	mathLib::Vec3 offset;     // 摄像机相对主角的偏移
//	mathLib::Vec3 position;      // Camera position
//	mathLib::Vec3 front;         // Front vector
//	mathLib::Vec3 up;            // Up vector
//	mathLib::Vec3 right;         // Right vector
//	mathLib::Vec3 worldUp;       // World up vector
//
//	float yaw;          // Yaw angle (horizontal rotation)
//	float pitch;        // Pitch angle (vertical rotation)
//	float movementSpeed;
//	float mouseSensitivity;
//
//	// Constructor
//	FPSCamera(/*const mathLib::Vec3& startPos = mathLib::Vec3(0.0f, 0.0f, 0.0f),*/
//		Player* _player, const mathLib::Vec3& _offset,
//		const mathLib::Vec3& _worldUp = mathLib::Vec3(0.0f, 1.0f, 0.0f),
//		float _startYaw = -90.0f, float _startPitch = 0.0f,
//		float _speed = 5.0f, float _sensitivity = 0.2f)
//		: player(_player), offset(_offset), worldUp(_worldUp), yaw(_startYaw), pitch(_startPitch),
//		movementSpeed(_speed), mouseSensitivity(_sensitivity) {
//		position = player->position + offset;
//		updateCameraVectors();
//	}
//
//	// 更新摄像机位置
//	void update() {
//		position = player->position + offset;
//	}
//
//	// Process keyboard input
//	void processKeyboard(Window& canvas, float deltaTime) {
//		float velocity = movementSpeed * deltaTime;
//
//		if (canvas.keys['W']) position += front * velocity;
//		if (canvas.keys['S']) position -= front * velocity;
//		if (canvas.keys['D']) position -= right * velocity;
//		if (canvas.keys['A']) position += right * velocity;
//	}
//
//	// Process mouse input
//	void processMouse(Window& canvas) {
//		float deltaX, deltaY;
//		canvas.getMouseMovement(deltaX, deltaY);
//
//		deltaX *= mouseSensitivity;
//		deltaY *= mouseSensitivity;
//
//		yaw += deltaX;
//		pitch += deltaY;
//
//		// Constrain pitch
//		if (pitch > 89.0f) pitch = 89.0f;
//		if (pitch < -89.0f) pitch = -89.0f;
//
//		updateCameraVectors();
//	}
//
//	// Generate the view matrix
//	mathLib::Matrix getViewMatrix() {
//		mathLib::Vec3 target = position + front;
//		return mathLib::lookAt(position, target, up);
//	}
//
//private:
//	// Recalculate front, right, and up vectors based on yaw and pitch
//	void updateCameraVectors() {
//		mathLib::Vec3 newFront;
//		newFront.x = cosf(radians(yaw)) * cosf(radians(pitch));
//		newFront.y = sinf(radians(pitch));
//		newFront.z = sinf(radians(yaw)) * cosf(radians(pitch));
//		front = newFront.normalize();
//		right = worldUp.cross(front).normalize();
//		up = front.cross(right).normalize();
//	}
//
//	// Convert degrees to radians
//	float radians(float degrees) const {
//		return degrees * static_cast<float>(M_PI) / 180.0f;
//	}
//};

class TPSCamera {
public:
	Player* player;
	mathLib::Vec3 position;   // camera position
	mathLib::Vec3 target;     // camera targer
	mathLib::Vec3 up;         // camera up vector

	float distance;           // distance from camera to player
	float yaw;                // Yaw angle (horizontal rotation)
	float pitch;              // Pitch angle(vertical rotation)
	float mouseSensitivity;

	// Constructor
	TPSCamera(Player* _player, float _distance = 5.0f, float _yaw = -90.0f, float _pitch = 20.0f, float _sensitivity = 0.2f)
		: player(_player), distance(_distance), yaw(_yaw), pitch(_pitch), mouseSensitivity(_sensitivity) {
		up = mathLib::Vec3(0.0f, 1.0f, 0.0f);
		updateCameraPosition();
	}

	// update
	void updateCameraPosition() {
		// Calculate camera position based on player position and offset
		float offsetX = cosf(radians(yaw)) * cosf(radians(pitch)) * distance;
		float offsetY = sinf(radians(pitch)) * distance;
		float offsetZ = sinf(radians(yaw)) * cosf(radians(pitch)) * distance;

		position = player->position - mathLib::Vec3(offsetX, offsetY, offsetZ);
		target = player->position; // camera target point is always the player

		// make sure the camera is above the plane
		if (position.y < player->position.y + 1.0f) { // 1.0f is the minimum height of the camera from the ground
			position.y = player->position.y + 1.0f;
		}
	}

	// mouse movement process
	void processMouse(Window& canvas) {
		float deltaX, deltaY;
		canvas.getMouseMovement(deltaX, deltaY);

		deltaX *= mouseSensitivity;
		deltaY *= mouseSensitivity;

		yaw += deltaX;
		pitch += deltaY;

		// constrain
		if (pitch > -5.0f) pitch = -5.0f; 
		if (pitch < -89.0f) pitch = -89.0f; // prevent the camera from looking at the ground

		updateCameraPosition();
	}

	// get View Matrix
	mathLib::Matrix getViewMatrix() {
		return mathLib::lookAt(position, target, up);
	}

private:
	float radians(float degrees) const {
		return degrees * static_cast<float>(M_PI) / 180.0f;
	}
};

static float getGroundHeight(const mathLib::Vec3& position) {
	return 0.0f;
}

static void handleInput(Player& player, TPSCamera& camera, Window& canvas, float deltaTime, AABB& obstacle) {
	// Forward and right direction of the player
	mathLib::Vec3 forward = camera.target - camera.position;
	forward.y = 0;
	forward = forward.normalize();

	mathLib::Vec3 right = mathLib::Vec3(0.0f, 1.0f, 0.0f).cross(forward).normalize();

	// attack logic
	if (canvas.keys['J'] && !player.isAttacking) {
		player.isAttacking = true;
		player.attackAnimationTime = 0.0f; // reset animation time
		player.updateAnimation("attack", deltaTime); // switch to attack animation
	}

	// player move
	if (!player.isAttacking) {
		mathLib::Vec3 moveDirection(0.0f, 0.0f, 0.0f);
		if (canvas.keys['W'])
			moveDirection += forward;
		if (canvas.keys['S'])
			moveDirection -= forward;
		if (canvas.keys['D'])
			moveDirection -= right;
		if (canvas.keys['A'])
			moveDirection += right;

		if (!(moveDirection.x == 0 && moveDirection.y == 0 && moveDirection.z == 0)) {
			moveDirection = moveDirection.normalize();
		}

		player.update(moveDirection, forward, obstacle, deltaTime);
	}
	else {
		player.attackAnimationTime += deltaTime;

		// check if the attack animation has finished playing
		if (player.attackAnimationTime >= player.attackDuration) {
			player.isAttacking = false; // animation done
			player.updateAnimation("Idle", deltaTime); // Switch to idle animation
		}
		else {
			player.updateAnimation("attack", deltaTime); // play attack animation
		}
	}

	camera.processMouse(canvas);

	// constrain player's position
	float groundHeight = getGroundHeight(player.position);
	player.stayOnGround(groundHeight);
}
