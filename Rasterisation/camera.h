#pragma once
#include "mathLib.h"

class camera {
	mathLib::Vec3 pos;
	float yaw;
	float pitch;
	float speed;
	float sensitivity;
public:
	camera(float _x, float _y, float _z, float _yaw, float _pitch)
		:pos(_x, _y, _z), yaw(_yaw), pitch(_pitch), speed(2.5f), sensitivity(0.1f) {}

	mathLib::Matrix getViewMatrix() {
		mathLib::Vec3 from(cosf(yaw) * cosf(pitch), sinf(pitch), sin(yaw) * cos(pitch));
		from = from.normalize();
		mathLib::Vec3 to = pos + from;
		mathLib::Vec3 up(0.0f, 1.0f, 0.0f);
		return mathLib::lookAt(from, to, up);
	}
};