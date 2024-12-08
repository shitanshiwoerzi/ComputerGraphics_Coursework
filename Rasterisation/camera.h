#pragma once
#include "mathLib.h"
#include "player.h"

class FPSCamera {
public:
	Player* player;           // 绑定的主角
	mathLib::Vec3 offset;     // 摄像机相对主角的偏移
	mathLib::Vec3 position;      // Camera position
	mathLib::Vec3 front;         // Front vector
	mathLib::Vec3 up;            // Up vector
	mathLib::Vec3 right;         // Right vector
	mathLib::Vec3 worldUp;       // World up vector

	float yaw;          // Yaw angle (horizontal rotation)
	float pitch;        // Pitch angle (vertical rotation)
	float movementSpeed;
	float mouseSensitivity;

	// Constructor
	FPSCamera(/*const mathLib::Vec3& startPos = mathLib::Vec3(0.0f, 0.0f, 0.0f),*/
		Player* _player, const mathLib::Vec3& _offset,
		const mathLib::Vec3& _worldUp = mathLib::Vec3(0.0f, 1.0f, 0.0f),
		float _startYaw = -90.0f, float _startPitch = 0.0f,
		float _speed = 5.0f, float _sensitivity = 0.2f)
		: player(_player), offset(_offset), worldUp(_worldUp), yaw(_startYaw), pitch(_startPitch),
		movementSpeed(_speed), mouseSensitivity(_sensitivity) {
		position = player->position + offset;
		updateCameraVectors();
	}

	// 更新摄像机位置
	void update() {
		position = player->position + offset;
	}

	// Process keyboard input
	void processKeyboard(Window& canvas, float deltaTime) {
		float velocity = movementSpeed * deltaTime;

		if (canvas.keys['W']) position += front * velocity;
		if (canvas.keys['S']) position -= front * velocity;
		if (canvas.keys['D']) position -= right * velocity;
		if (canvas.keys['A']) position += right * velocity;
	}

	// Process mouse input
	void processMouse(Window& canvas) {
		float deltaX, deltaY;
		canvas.getMouseMovement(deltaX, deltaY);

		deltaX *= mouseSensitivity;
		deltaY *= mouseSensitivity;

		yaw += deltaX;
		pitch += deltaY;

		// Constrain pitch
		if (pitch > 89.0f) pitch = 89.0f;
		if (pitch < -89.0f) pitch = -89.0f;

		updateCameraVectors();
	}

	// Generate the view matrix
	mathLib::Matrix getViewMatrix() {
		mathLib::Vec3 target = position + front;
		return mathLib::lookAt(position, target, up);
	}

private:
	// Recalculate front, right, and up vectors based on yaw and pitch
	void updateCameraVectors() {
		mathLib::Vec3 newFront;
		newFront.x = cosf(radians(yaw)) * cosf(radians(pitch));
		newFront.y = sinf(radians(pitch));
		newFront.z = sinf(radians(yaw)) * cosf(radians(pitch));
		front = newFront.normalize();
		right = worldUp.cross(front).normalize();
		up = front.cross(right).normalize();
	}

	// Convert degrees to radians
	float radians(float degrees) const {
		return degrees * static_cast<float>(M_PI) / 180.0f;
	}
};

static void handleInput(Player& player, FPSCamera& camera, Window& canvas, float deltaTime) {
	// 主角的前向和右向
	mathLib::Vec3 forward = camera.front;
	mathLib::Vec3 right = camera.right;

	mathLib::Vec3 moveDirection(0.0f, 0.0f, 0.0f);
	if (canvas.keys['W']) 
		moveDirection += forward;
	if (canvas.keys['S']) 
		moveDirection -= forward;
	if (canvas.keys['A']) 
		moveDirection += right;
	if (canvas.keys['D']) 
		moveDirection -= right;

	// 移动主角
	if (!(moveDirection.x == 0 && moveDirection.y == 0 && moveDirection.z == 0)) {
		moveDirection = moveDirection.normalize();
		player.move(moveDirection, deltaTime);
	}

	// 调整视角
	camera.processMouse(canvas);

	// 更新主角位置（模拟贴地行走）
	float groundHeight = getGroundHeight(player.position); // 获取地面高度
	player.stayOnGround(groundHeight);
}

static float getGroundHeight(const mathLib::Vec3& position) {
	return 0.0f; // 地面高度恒定为 0
}