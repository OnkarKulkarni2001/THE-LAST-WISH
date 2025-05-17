#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Animation.h"
#include "Bone.h"

// Forward declarations for Windows types
typedef void* HANDLE;
struct _RTL_CRITICAL_SECTION;
typedef _RTL_CRITICAL_SECTION CRITICAL_SECTION;

typedef unsigned long DWORD;
typedef void* LPVOID;

#ifndef WINAPI
#define WINAPI __stdcall
#endif


struct ThreadData {
	Animator* animator;
	const AssimpNodeData* node;
	glm::mat4 parentTransform;
};

DWORD WINAPI ThreadFunction(LPVOID lpParam);

class Animator
{
public:
	Animator(Animation* animation);

	Animator();

	~Animator();

	void UpdateAnimation(float dt);
	
	CRITICAL_SECTION* boneMatrixCriticalSection;

	void StartBoneTransform();

	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
	std::vector<glm::mat4> GetFinalBoneMatrices();
	void SetCurrentAnimation(Animation* currentAnimation);
	bool IsAnimationCompleted(Animation* animation);
	float GetDeltaTime();
	float GetAnimationCurrentTime();
	//float GetCurrentTime();
	void ResetCurrentTime();
	void SpeedUpAnimation(float value);
	float GetCurrentAnimationSpeed();
	Animation* GetCurrentAnimation();

	void CalculateBoneTransformMultiThreaded(const AssimpNodeData* node, glm::mat4 parentTransform);

private:
	std::vector<glm::mat4> finalBoneMatrices;
	Animation* currentAnimation;
	float currentTime;
	float deltaTime;
	float animationSpeed = 1.0f;
	bool bAmplifySpeed = false;
};