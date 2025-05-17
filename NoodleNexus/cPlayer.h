#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include "sMesh.h"
#include <glm/common.hpp>
#include "GLCommon.h"
#include "cPhysics.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "sharedThings.h"
#include <vector>
#include "cMasterInteractable.h"
#include "Animation/Model.h"
#include "Animation/Animator.h"
#include "Animation/Animation.h"
#include "cCharacter.h"
#include "PhysXWraper/cPhysXWraper.h"
#include "include/PhysX/PxPhysicsAPI.h"
#include "cMasterZombie.h"

using namespace physx;

class cMasterInteractable;

//enum ePlayerState
//{
//	IDLE,
//	WALK
//	//STRAFE_RIGHT,
//	//STRAFE_LEFT,
//	//AIM
//};


enum eDirection
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	DEFAULT
};



struct sBulletInfo
{
	sMesh* pBulletMesh;
	cPhysics::sPhysInfo* pBulletPhyscis;
	PxRigidDynamic* pPhysXActor;
	double spawnTime;
};

class cMasterZombie;


class cPlayer
{
public:
	cPlayer();
	~cPlayer();
	PxController* controller;
	//PxCapsuleController* pPlayerController; // PhysX player controller
	glm::vec3 moveDirection;
	PxControllerManager* pPlayerControllerManager; // Manager for controllers

	double playerDeltaTime;
	//Core Functionalities
	//void PlayerBeginPlay(sMesh* PlayerMesh);
	void PlayerBeginPlay(Model* PlayerModel, Animator* Animator, GLuint program);
	void PlayerUpdate(float deltaTime);

	Model* pPlayerSkeletalMesh;
	Animator* pPlayerAnimator;
	PxRigidDynamic* gPlayerColliderBody;

	sMesh* pGunMesh;

	GLuint programToUse;

	std::vector<sAnimationNames*> vecPlayerAnims;

	//Animator* pPlayerAnimator;
	//Animation* pCurrentAnimation;
	//ePlayerState CurrentState;

	//Movement Stuff
	void AddMovementInput(bool forward, bool backward, bool left, bool right);
	glm::vec3 getForwardVector();
	glm::vec3 getRightVector();

	glm::vec3 getPlayerForward();

	glm::vec3 getPlayerRight();

	float playerMoveSpeed = 0.025f;
	float rotationRate = 2.5f;
	bool orientRotationToMovement;

	bool bIsMoving = false;
	bool bIsHit = false;
	float hitTime = 0.0f;
	int currentAmmo = 30;
	
	void SetPlayerAnimation();

	//CameraStuff
	void UpdateSpringArm();
	void UpdateCameraRotation(double mouseDeltaX, double mouseDeltaY);

	const float cameraCollisionRadius = 0.3f;

	bool IsCameraPositionValid(const glm::vec3& newCameraPos, float radius);

	glm::vec3 CameraLocation;
	glm::vec3 CameraRotation;

	float CameraYaw;
	float CameraPitch;
	float MouseSensitivity = 0.01f;


	//Spring Arm System
	glm::vec3 SpringArmOffsetUnAimed = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 SpringArmOffsetAimed = glm::vec3(0.0f, 1.5f, 0.0f);

	float SpringArmLengthUnAimed = 3.0f;
	float SpringArmLengthAimed = 1.5f;

	float FovUnAimed = 0.6f;
	float FovAimed = 0.4f;

	//Defaul Variables
	float SpringArmLength;
	glm::vec3 SpringArmOffset;
	
	float MinCameraPitch = -45.0f; // Lower limit
	float MaxCameraPitch = 45.0f;  // Upper limit

	//CheckCameraCollisions
	//float LineTrace(glm::vec3 direction, float maxDistance);

	//Bullet Stuff
	//void SpawnBullet();
	//void UpdateBulletPositions();
	//void CreateBulletSphere();
	void fireBullet();
	//void DestroyExpiredBullets();
	//void CheckBulletCollisions();
	//bool CheckCollision(sMesh* bullet, Model* pZombieSkeletalMesh);
	//bool CheckPhysXCollision(PxRigidActor* bulletActor, PxController* zombieController);
	//void HandleBulletHit(cMasterZombie* enemy);

	float bulletSpeed = 50.0f;
	float fireRate = 0.3f; // Fire a bullet every 0.3 seconds
	double lastFireTime = 0.0; // Tracks the last shot time
	std::vector<sBulletInfo> BulletDataVector;

	void ADS();

	//Interaction Stuff
	void AddInteraction(cMasterInteractable* interactable);
	void RemoveInteraction(cMasterInteractable* interactable);
	void GetBestInteraction();
	void SetBestInteraction(cMasterInteractable* interactable);
	bool ShouldUpdateInteraction();
	void UpdateInteraction();
	//Animation* GetAnimationByName(std::string name);
	void PlayAnimation(Animation* animation, float animationSpeed = 1.0f);

	std::vector<cMasterInteractable*> Interactables;
	cMasterInteractable* pSelectedInteraction;

	//Health Stuff
	float Health = 100.0f;
	Animation* currentAnimation;


	void AffectHealth(float damage);

	std::vector<sMesh*> pickedKeys;
private:

	void AnimateSpringArmLength();
	void AnimateSprinArmOffset();
	void AnimateFov();

	void ResetGame();

};