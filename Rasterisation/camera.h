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
	Player* player;           // 绑定的主角
	mathLib::Vec3 position;   // 相机位置
	mathLib::Vec3 target;     // 相机目标点
	mathLib::Vec3 up;         // 相机的上方向

	float distance;           // 相机到主角的距离
	float yaw;                // Yaw angle (水平旋转)
	float pitch;              // Pitch angle (垂直旋转)
	float mouseSensitivity;

	// Constructor
	TPSCamera(Player* _player, float _distance = 5.0f, float _yaw = -90.0f, float _pitch = 20.0f, float _sensitivity = 0.2f)
		: player(_player), distance(_distance), yaw(_yaw), pitch(_pitch), mouseSensitivity(_sensitivity) {
		up = mathLib::Vec3(0.0f, 1.0f, 0.0f);
		updateCameraPosition();
	}

	// 更新相机位置
	void updateCameraPosition() {
		// 计算相机位置基于主角位置和偏移
		float offsetX = cosf(radians(yaw)) * cosf(radians(pitch)) * distance;
		float offsetY = sinf(radians(pitch)) * distance;
		float offsetZ = sinf(radians(yaw)) * cosf(radians(pitch)) * distance;

		position = player->position - mathLib::Vec3(offsetX, offsetY, offsetZ);
		target = player->position; // 相机目标点始终是主角

		// 确保相机在平面以上
		if (position.y < player->position.y + 1.0f) { // 1.0f 是相机与地面的最小高度
			position.y = player->position.y + 1.0f;
		}
	}

	// 处理鼠标输入
	void processMouse(Window& canvas) {
		float deltaX, deltaY;
		canvas.getMouseMovement(deltaX, deltaY);

		deltaX *= mouseSensitivity;
		deltaY *= mouseSensitivity;

		yaw += deltaX;
		pitch += deltaY;

		// 限制 pitch 的范围
		if (pitch > -5.0f) pitch = -5.0f;
		if (pitch < -89.0f) pitch = -89.0f; // 限制为 -30 度，防止相机看向地面

		updateCameraPosition();
	}

	// 获取 View Matrix
	mathLib::Matrix getViewMatrix() {
		return mathLib::lookAt(position, target, up);
	}

private:
	float radians(float degrees) const {
		return degrees * static_cast<float>(M_PI) / 180.0f;
	}
};

static float getGroundHeight(const mathLib::Vec3& position) {
	return 0.0f; // 地面高度恒定为 0
}

static void handleInput(Player& player, TPSCamera& camera, Window& canvas, float deltaTime, AABB& obstacle) {
	// 主角的前向和右向（由相机计算）
	mathLib::Vec3 forward = camera.target - camera.position;
	forward.y = 0; // 忽略垂直方向
	forward = forward.normalize();

	mathLib::Vec3 right = mathLib::Vec3(0.0f, 1.0f, 0.0f).cross(forward).normalize();

	// 移动主角
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
		player.move(moveDirection, deltaTime, obstacle);
	}

	// 更新主角的动画状态
	if (moveDirection.getLengthSquare() > 0.0f) {
		player.updateAnimation("walking", deltaTime); // 行走动画
	}
	else {
		player.updateAnimation("idle", deltaTime);    // 待机动画
	}

	// 更新主角碰撞盒
	player.updateBoundingBox();

	// 处理相机鼠标输入
	camera.processMouse(canvas);

	// 更新主角位置（贴地行走）
	float groundHeight = getGroundHeight(player.position);
	player.stayOnGround(groundHeight);
}

//static void handleInput(Player& player, FPSCamera& camera, Window& canvas, float deltaTime) {
//	// 主角的前向和右向
//	mathLib::Vec3 forward = camera.front;
//	mathLib::Vec3 right = camera.right;
//
//	mathLib::Vec3 moveDirection(0.0f, 0.0f, 0.0f);
//	if (canvas.keys['W']) 
//		moveDirection += forward;
//	if (canvas.keys['S']) 
//		moveDirection -= forward;
//	if (canvas.keys['A']) 
//		moveDirection += right;
//	if (canvas.keys['D']) 
//		moveDirection -= right;
//
//	// 移动主角
//	if (!(moveDirection.x == 0 && moveDirection.y == 0 && moveDirection.z == 0)) {
//		moveDirection = moveDirection.normalize();
//		player.move(moveDirection, deltaTime);
//	}
//
//	// 调整视角
//	camera.processMouse(canvas);
//
//	// 更新主角位置（模拟贴地行走）
//	float groundHeight = getGroundHeight(player.position); // 获取地面高度
//	player.stayOnGround(groundHeight);
//}
