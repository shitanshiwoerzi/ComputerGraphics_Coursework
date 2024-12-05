#pragma once
#include "mathLib.h"

class AABB
{
public:
	mathLib::Vec3 max;
	mathLib::Vec3 min;
	AABB() { reset(); }
	void reset()
	{
		max = mathLib::Vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		min = mathLib::Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	}
	void extend(const mathLib::Vec3& p)
	{
		max = Max(max, p);
		min = Min(min, p);
	}
};

class Sphere
{
public:
	mathLib::Vec3 centre;
	float radius;
};

class Ray
{
public:
	mathLib::Vec3 o;
	mathLib::Vec3 dir;
	mathLib::Vec3 invdir;
	Ray() {}
	Ray(const mathLib::Vec3 _o, const mathLib::Vec3 _dir) {
		init(_o, _dir);
	}
	void init(const mathLib::Vec3 _o, const mathLib::Vec3 _dir) {
		o = _o;
		dir = _dir;
		invdir = mathLib::Vec3(1.0f, 1.0f, 1.0f) / dir;
	}
	mathLib::Vec3 at(const float t) {
		return (o + (dir * t));
	}
};