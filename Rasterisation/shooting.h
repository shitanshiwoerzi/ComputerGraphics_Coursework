#pragma once
#include "mathLib.h"
#include <vector>
#include "mesh.h"
#include "collision.h"
#include "window.h"
#include "camera.h"
#include "GamesEngineeringBase.h"

class Bullet {
public:
	mathLib::Vec3 position;
	mathLib::Vec3 direction;
	float speed;
	bool isActive;
	float lifeTime; // Bullet lifetime in seconds

	Bullet(mathLib::Vec3& startPos, mathLib::Vec3& dir, float spd, float maxLifeTime = 3.0f)
		: position(startPos), direction(dir.normalize()), speed(spd), isActive(true), lifeTime(maxLifeTime) {}

	void update(float deltaTime) {
		if (!isActive) return;

		// Update position
		position += direction * speed * deltaTime;

		// Decrease lifetime
		lifeTime -= deltaTime;
		if (lifeTime <= 0.0f) {
			isActive = false;
		}
	}
};


class Enemy {
public:
	AABB bounds; // Axis-aligned bounding box for collision
	int health;
	bool isAlive;

	Enemy(const mathLib::Vec3& minPos, const mathLib::Vec3& maxPos, int hp)
		: bounds(), health(hp), isAlive(true) {
		bounds.min = minPos;
		bounds.max = maxPos;
	}

	void takeDamage(int damage) {
		if (!isAlive) return;
		health -= damage;
		if (health <= 0) {
			isAlive = false;
			// Play death animation or trigger effects
		}
	}
};

class ShootingSystem {
public:
	std::vector<Bullet> bullets;
	std::vector<Enemy> enemies;
	animatedModel* weapon;
	float fireCooldown; // Time between shots
	float cooldownTimer;
	int damage;

	ShootingSystem(animatedModel* _weapon, float cooldown = 0.5f, int dmg = 25)
		: weapon(_weapon), fireCooldown(cooldown), cooldownTimer(0.0f), damage(dmg) {}

	void shoot(mathLib::Vec3& startPosition, mathLib::Vec3& direction) {
		if (cooldownTimer > 0.0f) return;
		cooldownTimer = fireCooldown;

		// Play weapon shooting animation
		weapon->instance.update("Armature|08 Fire", 0.0f);

		// Create a bullet
		bullets.emplace_back(startPosition, direction, 50.0f);
	}

	void update(float deltaTime) {
		// Update cooldown timer
		if (cooldownTimer > 0.0f) cooldownTimer -= deltaTime;

		// Update bullets
		for (auto& bullet : bullets) {
			bullet.update(deltaTime);
			if (bullet.isActive) {
				checkCollisions(bullet);
			}
		}

		// Remove inactive bullets
		bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
			[](Bullet& b) { return !b.isActive; }),
			bullets.end());
	}

	void checkCollisions(Bullet& bullet) {
		for (auto& enemy : enemies) {
			if (!enemy.isAlive) continue;

			if (enemy.bounds.intersects(Sphere(bullet.position, 0.2f))) { // Bullet radius
				bullet.isActive = false;
				enemy.takeDamage(damage);
			}
		}
	}

	void addEnemy(const Enemy& enemy) {
		enemies.push_back(enemy);
	}
};

static void handleShooting(Window& canvas, FPSCamera& camera, ShootingSystem& shootingSystem, float deltaTime) {
	if (canvas.mouseButtons[0]) {
		// Get shooting direction from camera
		mathLib::Vec3 shootDirection = camera.front;
		mathLib::Vec3 shootPosition = camera.player->position + mathLib::Vec3(0.0f, 1.8f, 0.0f);

		shootingSystem.shoot(shootPosition, shootDirection);
	}

	shootingSystem.update(deltaTime);
}