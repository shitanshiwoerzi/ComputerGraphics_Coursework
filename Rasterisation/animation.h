#pragma once
#include "mathLib.h"
#include <vector>
#include <map>

struct Bone
{
	std::string name;
	mathLib::Matrix offset;
	int parentIndex;
};

struct Skeleton
{
	std::vector<Bone> bones;
	mathLib::Matrix globalInverse;
};

struct AnimationFrame
{
	std::vector<mathLib::Vec3> positions;
	std::vector<mathLib::Quaternion> rotations;
	std::vector<mathLib::Vec3> scales;
};

class AnimationSequence
{
public:
	std::vector<AnimationFrame> frames;
	float ticksPerSecond;

	mathLib::Vec3 interpolate(mathLib::Vec3 p1, mathLib::Vec3 p2, float t) {
		return ((p1 * (1.0f - t)) + (p2 * t));
	}
	mathLib::Quaternion interpolate(mathLib::Quaternion q1, mathLib::Quaternion q2, float t) {
		return mathLib::Quaternion::slerp(q1, q2, t);
	}
	float duration() {
		return ((float)frames.size() / ticksPerSecond);
	}

	void calcFrame(float t, int& frame, float& interpolationFact)
	{
		interpolationFact = t * ticksPerSecond;
		frame = (int)floorf(interpolationFact);
		interpolationFact = interpolationFact - (float)frame;
		frame = min(frame, frames.size() - 1);
	}

	int nextFrame(int frame)
	{
		return min(frame + 1, frames.size() - 1);
	}

	mathLib::Matrix interpolateBoneToGlobal(mathLib::Matrix* matrices, int baseFrame, float interpolationFact, Skeleton* skeleton, int boneIndex) {
		int nextFrameIndex = nextFrame(baseFrame);
		mathLib::Matrix scale = mathLib::Matrix::scaling(interpolate(frames[baseFrame].scales[boneIndex], frames[nextFrameIndex].scales[boneIndex], interpolationFact));
		mathLib::Matrix rotation = interpolate(frames[baseFrame].rotations[boneIndex], frames[nextFrameIndex].rotations[boneIndex], interpolationFact).toMatrix();
		mathLib::Matrix translation = mathLib::Matrix::translation(interpolate(frames[baseFrame].positions[boneIndex], frames[nextFrameIndex].positions[boneIndex], interpolationFact));
		mathLib::Matrix local = scale * rotation * translation;
		//mathLib::Matrix local = translation * rotation * scale;
		if (skeleton->bones[boneIndex].parentIndex > -1) {
			mathLib::Matrix global = local * matrices[skeleton->bones[boneIndex].parentIndex];
			//mathLib::Matrix global = matrices[skeleton->bones[boneIndex].parentIndex] * local;
			return global;
		}
		return local;
	}
};

class Animation
{
public:
	std::map<std::string, AnimationSequence> animations;
	Skeleton skeleton;
	void calcFrame(std::string name, float t, int& frame, float& interpolationFact) {
		animations[name].calcFrame(t, frame, interpolationFact);
	}
	mathLib::Matrix interpolateBoneToGlobal(std::string name, mathLib::Matrix* matrices, int baseFrame, float interpolationFact, int boneIndex) {
		return animations[name].interpolateBoneToGlobal(matrices, baseFrame, interpolationFact, &skeleton, boneIndex);
	}

	void calcFinalTransforms(mathLib::Matrix* matrices)
	{
		for (int i = 0; i < skeleton.bones.size(); i++)
		{
			matrices[i] = skeleton.bones[i].offset * matrices[i] * skeleton.globalInverse;
			//matrices[i] = matrices[i] * skeleton.bones[i].offset * skeleton.globalInverse;
		}
	}

};

class AnimationInstance
{
public:
	Animation* animation;
	std::string currentAnimation;
	float t;
	mathLib::Matrix matrices[256];

	void resetAnimationTime()
	{
		t = 0;
	}
	bool animationFinished()
	{
		if (t > animation->animations[currentAnimation].duration())
		{
			return true;
		}
		return false;
	}

	void update(std::string name, float dt) {
		if (name == currentAnimation) {
			t += dt;
		}
		else {
			currentAnimation = name;  t = 0;
		}
		if (animationFinished() == true) { resetAnimationTime(); }
		int frame = 0;
		float interpolationFact = 0;
		animation->calcFrame(name, t, frame, interpolationFact);
		for (int i = 0; i < animation->skeleton.bones.size(); i++)
		{
			matrices[i] = animation->interpolateBoneToGlobal(name, matrices, frame, interpolationFact, i);
		}
		animation->calcFinalTransforms(matrices);
	}
};