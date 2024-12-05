#pragma once
#include "mathLib.h"

class camera {
	mathLib::Vec3 pos;
	mathLib::Quaternion orientation;
	float speed;
	float sensitivity;
public:
	camera(float _x, float _y, float _z, float _yaw, float _pitch)
		:pos(_x, _y, _z), speed(2.5f), sensitivity(0.001f) {}

	mathLib::Matrix getViewMatrix() {
		mathLib::Vec3 vz(0.0f, 0.0f, -1.0f);
		mathLib::Vec3 vy(0.0f, -1.0f, 0.0f);
		mathLib::Vec3 front = orientation.toMatrix().mulPoint(vz);
		mathLib::Vec3 up = orientation.toMatrix().mulPoint(vy);
		mathLib::Vec3 to = pos + front;
		return mathLib::lookAt(pos, to, up);
	}

	void processKeyboard(bool* keys, float deltaTime) {
		float velocity = speed * deltaTime;
		mathLib::Vec3 vz(0.0f, 0.0f, -1.0f);
		mathLib::Vec3 vx(1.0f, 0.0f, 0.0f);
		mathLib::Vec3 front = orientation.toMatrix().mulPoint(vz);
		mathLib::Vec3 right = orientation.toMatrix().mulPoint(vx);

		if (keys['W'])
			pos += front * velocity;
		if (keys['S'])
			pos -= front * velocity;
		if (keys['A'])
			pos -= right * velocity;
		if (keys['D'])
			pos += right * velocity;
	}

	void processMouseMovement(float xoffset, float yoffset) {
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		mathLib::Quaternion qYaw = mathLib::Quaternion::fromAxisAngle(mathLib::Vec3(0.0f, -1.0f, 0.0f), xoffset);
		mathLib::Quaternion qPitch = mathLib::Quaternion::fromAxisAngle(mathLib::Vec3(0.0f, 0.0f, 1.0f), yoffset);

		orientation = qYaw * orientation * qPitch;
		orientation.normalize();
	}
};