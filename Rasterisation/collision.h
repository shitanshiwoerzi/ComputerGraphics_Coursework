#pragma once
#include "mathLib.h"

class Sphere;
class Ray;

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

	bool intersects(const AABB& other) const {
		return (min.x <= other.max.x && max.x >= other.min.x &&
			min.y <= other.max.y && max.y >= other.min.y &&
			min.z <= other.max.z && max.z >= other.min.z);
	}

	bool intersects(const Sphere& sphere);

	bool intersects(Ray& ray, float& tmin, float& tmax);
};

class OOBB
{
public:
	mathLib::Vec3 center;       // 中心点
	mathLib::Vec3 halfExtents;  // 各轴上的半尺寸
	mathLib::Vec3 axes[3];      // OOBB 的三个局部坐标轴（需正交归一）

	OOBB() {}

	OOBB(mathLib::Vec3& _center, mathLib::Vec3& _halfExtents, mathLib::Vec3& _xAxis, mathLib::Vec3& _yAxis, mathLib::Vec3& _zAxis)
		: center(_center), halfExtents(_halfExtents) {
		axes[0] = _xAxis.normalize();
		axes[1] = _yAxis.normalize();
		axes[2] = _zAxis.normalize();
	}

	bool intersects(Ray& ray, float& tmin, float& tmax);

	bool intersects(AABB& aabb) {
		// Get the axes to test (OOBB axes + AABB axes)
		mathLib::Vec3 testAxes[15];
		std::copy(std::begin(axes), std::end(axes), testAxes);
		testAxes[3] = mathLib::Vec3(1, 0, 0); // AABB's X-axis
		testAxes[4] = mathLib::Vec3(0, 1, 0); // AABB's Y-axis
		testAxes[5] = mathLib::Vec3(0, 0, 1); // AABB's Z-axis

		int index = 6;
		for (int i = 0; i < 3; ++i) {
			for (int j = 3; j < 6; ++j) {
				testAxes[index++] = cross(axes[i], testAxes[j]);
			}
		}

		// Test all axes
		for (int i = 0; i < 15; ++i) {
			if (!overlapOnAxis(*this, aabb, testAxes[i])) {
				return false;
			}
		}
		return true;
	}

private:
	static bool overlapOnAxis(OOBB& oobb, AABB& aabb, mathLib::Vec3& axis) {
		float oobbProjection = 0.0f;
		for (int i = 0; i < 3; ++i) {
			oobbProjection += oobb.halfExtents[i] * std::abs(dot(oobb.axes[i], axis));
		}

		float aabbProjection =
			(aabb.max.x - aabb.min.x) * std::abs(dot(mathLib::Vec3(1, 0, 0), axis)) +
			(aabb.max.y - aabb.min.y) * std::abs(dot(mathLib::Vec3(0, 1, 0), axis)) +
			(aabb.max.z - aabb.min.z) * std::abs(dot(mathLib::Vec3(0, 0, 1), axis));

		float centerDistance = std::abs(dot(oobb.center - (aabb.min + aabb.max) * 0.5f, axis));
		return centerDistance <= (oobbProjection + aabbProjection);
	}
};

class Sphere
{
public:
	mathLib::Vec3 centre;
	float radius;

	Sphere() : centre(mathLib::Vec3(0, 0, 0)), radius(1.0f) {}
	Sphere(const mathLib::Vec3& _centre, float _radius) : centre(_centre), radius(_radius) {}

	// 检查点是否在球体内
	bool contains(mathLib::Vec3& point) {
		return (point - centre).getLengthSquare() <= radius * radius;
	}

	// 检查与另一个球体是否相交
	bool intersects(Sphere& other) {
		float distanceSquared = (other.centre - centre).getLengthSquare();
		float radiusSum = radius + other.radius;
		return distanceSquared <= radiusSum * radiusSum;
	}

	// 检查与 AABB 是否相交
	bool intersects(const AABB& aabb) {
		float distSquared = 0.0f;
		for (int i = 0; i < 3; ++i) {
			float v = centre[i];
			if (v < aabb.min[i]) distSquared += (aabb.min[i] - v) * (aabb.min[i] - v);
			if (v > aabb.max[i]) distSquared += (v - aabb.max[i]) * (v - aabb.max[i]);
		}
		return distSquared <= radius * radius;
	}

	// 检查与射线是否相交，并返回最近的交点
	bool intersects(Ray& ray, float& t);
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

	// 判断点是否在射线上（允许一定误差）
	bool isPointOnRay(mathLib::Vec3& point, float epsilon = 1e-6f) {
		mathLib::Vec3 v = point - o; // 点到射线起点的向量
		float t = dot(v, dir);       // 计算点到射线的投影长度
		if (t < 0) return false;    // 如果 t 为负，点在射线起点的反方向
		return (v - dir * t).getLengthSquare() <= epsilon * epsilon; // 检查点是否在射线方向
	}

	// 检测与平面的相交，返回交点和参数 t
	bool intersectsPlane(mathLib::Vec3& planePoint, mathLib::Vec3& planeNormal, float& t) {
		float denom = dot(planeNormal, dir);
		if (std::abs(denom) < 1e-6f) {
			return false; // 射线与平面平行
		}
		t = dot(planePoint - o, planeNormal) / denom;
		return t >= 0.0f; // 只有 t >= 0 时，交点在射线方向上
	}

	// 检测与三角形的相交，返回交点参数 t 和重心坐标 u, v
	bool intersectsTriangle(mathLib::Vec3& v0, mathLib::Vec3& v1, mathLib::Vec3& v2, float& t, float& u, float& v) {
		// Möller-Trumbore 算法
		mathLib::Vec3 edge1 = v1 - v0;
		mathLib::Vec3 edge2 = v2 - v0;
		mathLib::Vec3 h = cross(dir, edge2);
		float a = dot(edge1, h);

		if (std::abs(a) < 1e-6f) return false; // 射线与三角形平面平行

		float f = 1.0f / a;
		mathLib::Vec3 s = o - v0;
		u = f * dot(s, h);
		if (u < 0.0f || u > 1.0f) return false;

		mathLib::Vec3 q = cross(s, edge1);
		v = f * dot(dir, q);
		if (v < 0.0f || u + v > 1.0f) return false;

		t = f * dot(edge2, q);
		return t >= 0.0f; // 交点必须在射线方向
	}

	// 检测与球体的相交，返回交点参数 t
	bool intersectsSphere(const mathLib::Vec3& sphereCenter, float sphereRadius, float& t) {
		mathLib::Vec3 oc = o - sphereCenter;

		// 二次方程系数
		float a = dot(dir, dir);
		float b = 2.0f * dot(oc, dir);
		float c = dot(oc, oc) - sphereRadius * sphereRadius;

		// 判别式
		float discriminant = b * b - 4 * a * c;
		if (discriminant < 0) {
			return false; // 没有交点
		}

		// 最近的交点
		float sqrtDisc = std::sqrt(discriminant);
		float t0 = (-b - sqrtDisc) / (2.0f * a);
		float t1 = (-b + sqrtDisc) / (2.0f * a);

		t = (t0 > 0) ? t0 : t1;
		return t > 0;
	}

	// 检测与 AABB 的相交，返回参数 tmin 和 tmax
	bool intersectsAABB(const AABB& box, float& tmin, float& tmax) {
		tmin = 0.0f;
		tmax = FLT_MAX;

		for (int i = 0; i < 3; ++i) {
			float t1 = (box.min[i] - o[i]) * invdir[i];
			float t2 = (box.max[i] - o[i]) * invdir[i];

			if (t1 > t2) std::swap(t1, t2);

			tmin = max(tmin, t1);
			tmax = min(tmax, t2);

			if (tmin > tmax) return false;
		}
		return true;
	}
};

bool AABB::intersects(const Sphere& sphere) {
	float distSquared = 0.0f;// 初始化球体中心到 AABB 的最短距离平方
	for (int i = 0; i < 3; ++i) {// 遍历 x, y, z 三个坐标轴
		float v = sphere.centre[i];  // 获取球体中心在当前轴的坐标
		// 球体中心在 AABB 当前轴范围之外的情况
		if (v < min[i]) distSquared += (min[i] - v) * (min[i] - v);
		if (v > max[i]) distSquared += (v - max[i]) * (v - max[i]);
	}
	std::cout << "Distance Squared: " << distSquared << "\n";
	std::cout << "Sphere Radius Squared: " << sphere.radius * sphere.radius << "\n";
	// 比较球体中心到 AABB 的最短距离平方与球体半径平方
	return distSquared <= sphere.radius * sphere.radius;
}

bool AABB::intersects(Ray& ray, float& tmin, float& tmax) {
	tmin = 0.0f;
	tmax = FLT_MAX;

	for (int i = 0; i < 3; ++i) {
		float t1 = (min[i] - ray.o[i]) * ray.invdir[i];
		float t2 = (max[i] - ray.o[i]) * ray.invdir[i];

		if (t1 > t2) std::swap(t1, t2);

		tmin = max(tmin, t1);
		tmax = min(tmax, t2);

		if (tmin > tmax) return false;
	}
	return true;
}

bool OOBB::intersects(Ray& ray, float& tmin, float& tmax) {
	tmin = 0.0f;
	tmax = FLT_MAX;

	mathLib::Vec3 p = center - ray.o;

	for (int i = 0; i < 3; ++i) {
		float e = dot(p, axes[i]);
		float f = dot(ray.dir, axes[i]);

		if (std::abs(f) > 0.0001f) {
			float t1 = (e - halfExtents[i]) / f;
			float t2 = (e + halfExtents[i]) / f;

			if (t1 > t2) std::swap(t1, t2);

			tmin = max(tmin, t1);
			tmax = min(tmax, t2);

			if (tmin > tmax) return false;
		}
		else if (-e - halfExtents[i] > 0.0f || -e + halfExtents[i] < 0.0f) {
			return false;
		}
	}
	return true;
}

bool Sphere::intersects(Ray& ray, float& t) {
	mathLib::Vec3 oc = ray.o - centre;

	// 求解射线方程：t^2 * (dir·dir) + 2t * (oc·dir) + (oc·oc - r^2) = 0
	float a = dot(ray.dir, ray.dir);
	float b = 2.0f * dot(oc, ray.dir);
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - 4 * a * c;

	if (discriminant < 0) {
		return false; // 无解，射线未与球体相交
	}

	// 求两个交点
	float sqrtDisc = std::sqrt(discriminant);
	float t0 = (-b - sqrtDisc) / (2.0f * a);
	float t1 = (-b + sqrtDisc) / (2.0f * a);

	// 返回最近的正交点
	t = (t0 > 0) ? t0 : t1;
	return t > 0;
}