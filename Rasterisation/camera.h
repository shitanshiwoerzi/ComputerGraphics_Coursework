#pragma once
#include "mathLib.h"

class FPSCamera {
public:
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
	FPSCamera(const mathLib::Vec3& startPos = mathLib::Vec3(0.0f, 0.0f, 0.0f),
		const mathLib::Vec3& worldUp = mathLib::Vec3(0.0f, 1.0f, 0.0f),
		float startYaw = -90.0f, float startPitch = 0.0f,
		float speed = 5.0f, float sensitivity = 0.2f)
		: position(startPos), worldUp(worldUp), yaw(startYaw), pitch(startPitch),
		movementSpeed(speed), mouseSensitivity(sensitivity) {
		updateCameraVectors();
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

//class camera {
//	mathLib::Vec3 pos; // camera postion
//	mathLib::Quaternion orientation;
//	float speed; // movement speed
//	float sensitivity; // mouse sensitivity
//	float yaw;   // 当前偏航角
//	float pitch; // 当前俯仰角
//	const float maxPitch = 89.0f;
//public:
//	camera(float _x, float _y, float _z, float _yaw = 0.0f, float _pitch = 0.0f)
//		:pos(_x, _y, _z), speed(2.5f), sensitivity(0.001f), yaw(_yaw), pitch(_pitch) {
//	}
//
//	mathLib::Matrix getViewMatrix() {
//		mathLib::Vec3 vz(0.0f, 0.0f, -1.0f);
//		mathLib::Vec3 vy(0.0f, -1.0f, 0.0f);
//		mathLib::Vec3 front = orientation.toMatrix().mulPoint(vz);
//		mathLib::Vec3 up = orientation.toMatrix().mulPoint(vy);
//		mathLib::Vec3 to = pos + front;
//		return mathLib::lookAt(pos, to, up);
//	}
//
//	void processKeyboard(bool* keys, float deltaTime) {
//		float velocity = speed * deltaTime;
//		mathLib::Vec3 vz(0.0f, 0.0f, -1.0f);
//		mathLib::Vec3 vx(1.0f, 0.0f, 0.0f);
//		mathLib::Vec3 front = orientation.toMatrix().mulPoint(vz);
//		mathLib::Vec3 right = orientation.toMatrix().mulPoint(vx);
//
//		if (keys['W'])
//			pos += front * velocity;
//		if (keys['S'])
//			pos -= front * velocity;
//		if (keys['A'])
//			pos -= right * velocity;
//		if (keys['D'])
//			pos += right * velocity;
//	}
//
//	void processMouseMovement(float xoffset, float yoffset) {
//		xoffset *= sensitivity;
//		yoffset *= sensitivity;
//
//		yaw += xoffset * sensitivity;
//		pitch += yoffset * sensitivity;
//
//		// 限制俯仰角
//		if (pitch > maxPitch) pitch = maxPitch;
//		if (pitch < -maxPitch) pitch = -maxPitch;
//
//		mathLib::Quaternion qYaw = mathLib::Quaternion::fromAxisAngle(mathLib::Vec3(0.0f, -1.0f, 0.0f), xoffset);
//		mathLib::Quaternion qPitch = mathLib::Quaternion::fromAxisAngle(mathLib::Vec3(0.0f, 0.0f, 1.0f), yoffset);
//
//		orientation = qYaw * orientation * qPitch;
//		orientation.normalize();
//	}
//
//private:
//	void updateOrientation() {
//
//	}
//};