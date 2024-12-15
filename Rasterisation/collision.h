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

class Sphere
{
public:
	mathLib::Vec3 centre;
	float radius;

	Sphere() : centre(mathLib::Vec3(0, 0, 0)), radius(1.0f) {}
	Sphere(const mathLib::Vec3& _centre, float _radius) : centre(_centre), radius(_radius) {}

	// Is the checkpoint inside the sphere
	bool contains(mathLib::Vec3& point) {
		return (point - centre).getLengthSquare() <= radius * radius;
	}

	// Check for intersection with another sphere
	bool intersects(Sphere& other) {
		float distanceSquared = (other.centre - centre).getLengthSquare();
		float radiusSum = radius + other.radius;
		return distanceSquared <= radiusSum * radiusSum;
	}

	// Check for intersection with AABB
	bool intersects(const AABB& aabb) {
		float distSquared = 0.0f;
		for (int i = 0; i < 3; ++i) {
			float v = centre[i];
			if (v < aabb.min[i]) distSquared += (aabb.min[i] - v) * (aabb.min[i] - v);
			if (v > aabb.max[i]) distSquared += (v - aabb.max[i]) * (v - aabb.max[i]);
		}
		return distSquared <= radius * radius;
	}

	// Checks for intersection with a ray and returns the nearest intersection point
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

	// Determine if the point is on the ray (allow for some error)
	bool isPointOnRay(mathLib::Vec3& point, float epsilon = 1e-6f) {
		mathLib::Vec3 v = point - o; // Vector from the point to the start of the ray
		float t = dot(v, dir);       // Calculate the length of the projection from the point to the ray
		if (t < 0) return false;    // If t is negative, the point is in the opposite direction of the start of the ray
		return (v - dir * t).getLengthSquare() <= epsilon * epsilon; // Check if the point is in the ray direction
	}

	// Detects the intersection with the plane, returns the intersection point and the parameter t
	bool intersectsPlane(mathLib::Vec3& planePoint, mathLib::Vec3& planeNormal, float& t) {
		float denom = dot(planeNormal, dir);
		if (std::abs(denom) < 1e-6f) {
			return false; // The rays are parallel to the plane
		}
		t = dot(planePoint - o, planeNormal) / denom;
		return t >= 0.0f; // The intersection is in the direction of the ray only if t >= 0.
	}

	// Detects the intersection with the sphere and returns the intersection parameter t
	bool intersectsSphere(const mathLib::Vec3& sphereCenter, float sphereRadius, float& t) {
		mathLib::Vec3 oc = o - sphereCenter;

		// quadratic coefficient
		float a = dot(dir, dir);
		float b = 2.0f * dot(oc, dir);
		float c = dot(oc, oc) - sphereRadius * sphereRadius;

		// discriminant 
		float discriminant = b * b - 4 * a * c;
		if (discriminant < 0) {
			return false; // no intersection
		}

		// nearest point of intersection
		float sqrtDisc = std::sqrt(discriminant);
		float t0 = (-b - sqrtDisc) / (2.0f * a);
		float t1 = (-b + sqrtDisc) / (2.0f * a);

		t = (t0 > 0) ? t0 : t1;
		return t > 0;
	}

	// Detects the intersection with AABB and returns the parameters tmin and tmax.
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
	float distSquared = 0.0f;// Initialize the square of the shortest distance from the center of the sphere to the AABB.
	for (int i = 0; i < 3; ++i) {// Iterate over the x, y, and z axes.
		float v = sphere.centre[i];  // Get the coordinates of the center of the sphere in the current axis
		// Sphere center outside the range of the AABB's current axis
		if (v < min[i]) distSquared += (min[i] - v) * (min[i] - v);
		if (v > max[i]) distSquared += (v - max[i]) * (v - max[i]);
	}
	std::cout << "Distance Squared: " << distSquared << "\n";
	std::cout << "Sphere Radius Squared: " << sphere.radius * sphere.radius << "\n";
	//  Compare the square of the shortest distance from the center of the sphere to AABB to the square of the radius of the sphere.
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

bool Sphere::intersects(Ray& ray, float& t) {
	mathLib::Vec3 oc = ray.o - centre;

	float a = dot(ray.dir, ray.dir);
	float b = 2.0f * dot(oc, ray.dir);
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - 4 * a * c;

	if (discriminant < 0) {
		return false;
	}

	// Find the two points of intersection
	float sqrtDisc = std::sqrt(discriminant);
	float t0 = (-b - sqrtDisc) / (2.0f * a);
	float t1 = (-b + sqrtDisc) / (2.0f * a);

	// Returns the nearest orthogonal point
	t = (t0 > 0) ? t0 : t1;
	return t > 0;
}