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
		return mathLib::lookAt(to, from, up);
	}

    void processKeyboard(bool* keys, float deltaTime) {
        float velocity = speed * deltaTime;
        mathLib::Vec3 front;
        front.x = cos(yaw) * cos(pitch);
        front.y = sin(pitch);
        front.z = sin(yaw) * cos(pitch);
        front = front.normalize();

        mathLib::Vec3 right = mathLib::Vec3(0.0f, 1.0f, 0.0f).cross(front).normalize();

        if (keys['W'])
            pos = pos + front * velocity;
        if (keys['S'])
            pos = pos - front * velocity;
        if (keys['A'])
            pos = pos - right * velocity;
        if (keys['D'])
            pos = pos + right * velocity;
    }

    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (constrainPitch) {
            if (pitch > 1.5f)
                pitch = 1.5f;
            if (pitch < -1.5f)
                pitch = -1.5f;
        }
    }
};