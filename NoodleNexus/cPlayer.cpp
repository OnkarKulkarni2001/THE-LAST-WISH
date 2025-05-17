#include "cPlayer.h"
#include <iostream>
#include <chrono>
#include <glm/gtx/euler_angles.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>  // Include this header for glm::rotation
#include <algorithm>
#include "cTimelineManager.h"
#include <glm/gtx/vector_angle.hpp>
#include "cParticleEmitter_2.h"
#include "cBullet.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "cSoundManager.h"
#include "sharedThings.h"

using namespace physx;

extern GLuint program;
extern float frontDistance;
extern float backDistance;
extern float rightDistance;
extern float leftDistance;
extern float botDistance;
extern float collisionThreshold;
extern float downDistance;

extern cBasicFlyCamera* ::g_pFlyCamera;
extern bool bUseThreading;
glm::vec3 rightHandOffset = glm::vec3(1.0f);
glm::vec3 gunRot = glm::vec3(0.0f);
// Example usage of glm::rotation
glm::vec3 fromDirection = glm::vec3(0.0f, 0.0f, -1.0f);  //

extern bool bIsAiming;
extern float FoV;
extern std::vector<sMesh*> g_vecMeshesToDraw;
extern cPhysics* g_pPhysicEngine;
extern cBasicFlyCamera* g_pFlyCamera;
extern std::vector<cMasterZombie*> AllZombies;
extern cSoundManager* g_pSoundManager;

extern cTimelineManager* pTimelineManager;

extern PxRigidDynamic* gPlayerBody;
extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern cPhysXWraper* g_pPhysX;
extern cParticleEmitter_2* g_pParticles;

extern cPlayer* pPlayerReference;

PxControllerManager* pControllerManager; // Manager for controllers

void createPlayerDynamic(PxVec3 playerPosition, PxF32 radius, PxF32 halfHeight);
Animation* GetAnimationByName(std::string name, std::vector<sAnimationNames*> vecAnimNames);

cPlayer::cPlayer()
{
	pControllerManager = PxCreateControllerManager(*gScene);
	PxCapsuleControllerDesc desc;
	desc.height = 1.8f;
	desc.radius = 0.4f;
	desc.position = PxExtendedVec3(35.0f, 4.0f, 5.0f); // Starting position
	desc.material = gMaterial; // Static friction, dynamic friction, restitution
	desc.slopeLimit = cosf(PxPi / 3); // 60 degrees slope limit
	desc.contactOffset = 0.1f;
	desc.stepOffset = 0.5f;
	desc.upDirection = PxVec3(0, 1, 0); // Y-Up world
	desc.reportCallback = nullptr; // Optional: custom hit report callback
	controller = pControllerManager->createController(desc);
	pPlayerControllerManager = pControllerManager;
}

cPlayer::~cPlayer()
{
	pPlayerControllerManager->release();
	pControllerManager = nullptr;
}

void cPlayer::PlayerBeginPlay(Model* PlayerModel, Animator* Animator, GLuint program)
{
	pPlayerSkeletalMesh = PlayerModel;
	pPlayerAnimator = Animator;
	CameraYaw = ::g_pFlyCamera->m_Yaw_Y_axis_rotation;
	CameraPitch = ::g_pFlyCamera->m_Pitch_X_axis_rotation;
	orientRotationToMovement = true;
	gPlayerColliderBody = gPlayerBody;

	programToUse = program;

	SpringArmOffset = SpringArmOffsetUnAimed;
	SpringArmLength = SpringArmLengthUnAimed;


	pGunMesh = new sMesh();
	pGunMesh->positionXYZ = glm::vec3(0.0f);
	pGunMesh->modelFileName = "assets/models/Gun.ply";
	pGunMesh->rotationEulerXYZ = glm::vec3(0.0f);
	pGunMesh->blendRatio[0] = 1.0f;
	pGunMesh->hasNormalMap = false;
	pGunMesh->uniformScale = 1.0f;
	pGunMesh->uniqueFriendlyName = "Gun";
	pGunMesh->textures[0] = "tt_pistol_Albedo.bmp";

	//g_vecMeshesToDraw.push_back(pGunMesh);
}

void cPlayer::AddMovementInput(bool moveForward, bool moveBackward, bool moveLeft, bool moveRight)
{
	if (bIsAiming)
		return;

	if (Health <= 0.0f) return;

	moveDirection = glm::vec3(0.0f);
	bIsMoving = moveForward || moveBackward || moveLeft || moveRight;
	// Combine movement vectors based on input
	if (moveForward)
		moveDirection += getForwardVector();
	if (moveBackward)
		moveDirection -= getForwardVector();
	if (moveLeft)
		moveDirection -= getRightVector();
	if (moveRight)
		moveDirection += getRightVector();

	// Normalize to avoid diagonal speed boost
	if (glm::length(moveDirection) > 0.0f)
		moveDirection = glm::normalize(moveDirection);
	else
		return; // No input, skip rest

	// Apply movement
	//pPlayerSkeletalMesh->positionXYZ.x += moveDirection.x * playerMoveSpeed;
	//pPlayerSkeletalMesh->positionXYZ.z += moveDirection.z * playerMoveSpeed;
	PxControllerFilters filters;

	controller->move(PxVec3(moveDirection.x * playerMoveSpeed, 0.0f, moveDirection.z * playerMoveSpeed), 0.001f, playerDeltaTime, filters);
	//pPlayerSkeletalMesh->positionXYZ = glm::vec3(controller->getPosition().x, controller->getPosition().y, controller->getPosition().z);
	pPlayerSkeletalMesh->positionXYZ = glm::vec3(controller->getPosition().x, pPlayerSkeletalMesh->positionXYZ.y, controller->getPosition().z);

	// Optional: animation trigger
	// pPlayerAnimator->SetAnimation("Walk");

	// Apply orientation if enabled
	if (orientRotationToMovement)
	{
		// Calculate yaw (Y axis rotation) based on movement direction
		glm::vec3 forward = glm::normalize(moveDirection);
		float targetYaw = glm::degrees(atan2(forward.x, forward.z));

		// Wrap yaw between -180 and 180
		if (targetYaw > 180.0f) targetYaw -= 360.0f;
		if (targetYaw < -180.0f) targetYaw += 360.0f;

		float currentYaw = pPlayerSkeletalMesh->rotationEulerXYZ.y;
		float deltaYaw = targetYaw - currentYaw;

		if (deltaYaw > 180.0f) deltaYaw -= 360.0f;
		if (deltaYaw < -180.0f) deltaYaw += 360.0f;

		// Smooth rotation
		if (std::abs(deltaYaw) > rotationRate)
			pPlayerSkeletalMesh->rotationEulerXYZ.y += rotationRate * glm::sign(deltaYaw);
		else
			pPlayerSkeletalMesh->rotationEulerXYZ.y = targetYaw;
	}
	else
	{
		// Instantly face right vector if no orientation smoothing
		glm::vec3 forward = glm::normalize(getRightVector());
		float yaw = atan2(forward.x, forward.z);
		float targetYaw = glm::degrees(yaw);

		if (targetYaw > 180.0f) targetYaw -= 360.0f;
		if (targetYaw < -180.0f) targetYaw += 360.0f;

		pPlayerSkeletalMesh->rotationEulerXYZ.y = targetYaw;
	}
}

glm::vec3 getCameraLookDirection();

void cPlayer::fireBullet()
{
	//// Get the current time in seconds
	double currentTime = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now().time_since_epoch()).count()) / 1000.0;
	
	if (currentAmmo > 0) {
		g_pParticles->SetSourceLocation(glm::vec3(this->pPlayerSkeletalMesh->positionXYZ.x, 2.5f, this->pPlayerSkeletalMesh->positionXYZ.z) +
			glm::vec3(this->getPlayerForward().x, 0.0f, this->getPlayerForward().z)/* * 1.5f*/);

		::g_pParticles->SetInitalVelocity(
			this->getPlayerForward() * 0.2f + glm::vec3(-0.2f, -0.2f, -0.2f),        // Min
			this->getPlayerForward() * 0.9f + glm::vec3(0.2f, 0.8f, 0.2f));       // Max


		::g_pParticles->CreateParticlesImmediately(50);

	}


	// Check if enough time has passed since the last shot
	if (currentTime - lastFireTime >= fireRate && currentAmmo > 0)
	{
		glm::vec3 fireDirection = getCameraLookDirection(); // <-- New direction
		glm::vec3 bulletStart = glm::vec3(this->pPlayerSkeletalMesh->positionXYZ) + fireDirection * 1.5f;

		sMesh* pBulletMesh = new sMesh();

		pBulletMesh->positionXYZ = glm::vec3(this->pPlayerSkeletalMesh->positionXYZ.x, 1.5f, this->pPlayerSkeletalMesh->positionXYZ.z) +
			glm::vec3(this->getPlayerForward().x, 0.0f, this->getPlayerForward().z) * 1.5f;
		cBullet* pBullet = new cBullet(pBulletMesh, fireDirection);


		pBulletMesh->modelFileName = "assets/LevelModels/Bullet.ply";
		// Spawn higher and farther forward
		pBulletMesh->rotationEulerXYZ = glm::vec3(0.0f, 0.0f, 0.0f); // Simplify rotation
		pBulletMesh->objectColourRGBA = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // Bright red for visibility
		pBulletMesh->bOverrideObjectColour = true;
		pBulletMesh->bDoNotLight = false;
		pBulletMesh->uniformScale = 0.01f; // Larger scale
		pBulletMesh->uniqueFriendlyName = "Bullet";
		pBulletMesh->bIsVisible = true;
		::g_vecMeshesToDraw.push_back(pBulletMesh);

		currentAmmo--;

		g_pSoundManager->PlaySoundEffect("PistolFire");
		lastFireTime = currentTime; // Update last fire time
	}
	else if (currentAmmo <= 0)
	{
		g_pSoundManager->PlaySoundEffect("GunJam");
	}
	
}

glm::vec3 cPlayer::getForwardVector()
{
	return glm::normalize(glm::vec3(
		cos(glm::radians(CameraYaw)) * cos(glm::radians(CameraPitch)),
		sin(glm::radians(CameraPitch)),
		sin(glm::radians(CameraYaw)) * cos(glm::radians(CameraPitch))));
}

glm::vec3 cPlayer::getRightVector()
{
	//// Choose up vector based on Z comparison (optional logic)
	//glm::vec3 up = (CameraLocation.z < pPlayerSkeletalMesh->positionXYZ.z)
	//	? glm::vec3(0.0f, 1.0f, 0.0f)
	//	: glm::vec3(0.0f, -1.0f, 0.0f);
	return glm::normalize(glm::cross(getForwardVector(), glm::vec3(0.0f, 1.0f, 0.0f)));
}

glm::vec3 cPlayer::getPlayerForward()
{
	float yawRadians = glm::radians(pPlayerSkeletalMesh->rotationEulerXYZ.y);
	glm::vec3 playerForward = glm::vec3(sin(yawRadians), 0.0f, cos(yawRadians));
	return glm::normalize(playerForward);
}

glm::vec3 cPlayer::getPlayerRight()
{
	return glm::normalize(glm::cross(getPlayerForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
}

void cPlayer::PlayerUpdate(float deltaTime)
{
	playerDeltaTime = deltaTime;	

	glm::mat4 handWorldMatrix = pPlayerSkeletalMesh->GetBoneTransform(pPlayerAnimator, "mixamorig9_RightHand");
	glm::quat handRotation = glm::quat_cast(handWorldMatrix);

	glm::vec3 adjustedGunRot = glm::vec3(gunRot.x - 90.0f, gunRot.y, gunRot.z + 180.0f);

	if (bIsAiming)
	{
		adjustedGunRot.x += 90.0f;   // Make gun horizontal
		adjustedGunRot.z += 180.0f;  // Flip upright (or adjust to 0 if it feels wrong)
	}

	glm::quat gunLocalRotation = glm::quat(glm::radians(adjustedGunRot));
	glm::quat finalGunRotation = gunLocalRotation * handRotation;

	glm::vec3 finalGunRotationEulerRadians = glm::eulerAngles(finalGunRotation);
	pGunMesh->rotationEulerXYZ = glm::degrees(finalGunRotationEulerRadians);

	glm::vec3 rightHandPos = pPlayerSkeletalMesh->GetBonePosition(pPlayerAnimator, "mixamorig9_RightHand");
	glm::vec3 rotatedOffset = handRotation * rightHandOffset;
	pGunMesh->positionXYZ = rightHandPos + rotatedOffset;




	// 1. Get the hand's world matrix
	//glm::mat4 handWorldMatrix = pPlayerSkeletalMesh->GetBoneTransform(pPlayerAnimator, "mixamorig9_RightHand");

	// 2. Extract world-space axes from the hand's matrix
	glm::vec3 handRight = glm::normalize(glm::vec3(handWorldMatrix[0]));
	glm::vec3 handUp = glm::normalize(glm::vec3(handWorldMatrix[1]));
	glm::vec3 handForward = glm::normalize(glm::vec3(handWorldMatrix[2]));

	// 3. Optional: flip any axes here if gun is facing wrong
	// handForward *= -1.0f;
	// handUp *= -1.0f;

	// 4. Build a new rotation matrix based on those axes
	glm::mat4 gunRotationMatrix = glm::mat4(1.0f);
	gunRotationMatrix[0] = glm::vec4(handRight, 0.0f);
	gunRotationMatrix[1] = glm::vec4(handUp, 0.0f);
	gunRotationMatrix[2] = glm::vec4(handForward, 0.0f);

	// 5. Apply positional offset in gun's local space
	//glm::vec3 rotatedOffset = handRight * rightHandOffset.x
	//	+ handUp * rightHandOffset.y
	//	+ handForward * rightHandOffset.z;

	// 6. Final gun transform
	glm::vec3 handPos = glm::vec3(handWorldMatrix[3]);

	pGunMesh->positionXYZ = handPos + rotatedOffset;

	// 7. Apply rotation
	//glm::quat finalGunRotation = glm::quat_cast(gunRotationMatrix);
	pGunMesh->rotationEulerXYZ = glm::degrees(glm::eulerAngles(finalGunRotation));














	if (Health <= 0)
	{
		Health = 0;
		PlayAnimation(GetAnimationByName("Die", vecPlayerAnims));
		GetAnimationByName("Die", vecPlayerAnims)->SetLooping(GetAnimationByName("Die", vecPlayerAnims), false);
		return;
	}

	if (bIsHit) {
		PlayAnimation(GetAnimationByName("Hit", vecPlayerAnims));
		//GetAnimationByName("Hit", vecPlayerAnims)->SetLooping(GetAnimationByName("Hit", vecPlayerAnims), true);
		bIsHit = false;
		return;
	}
	//if (hitTime > GetAnimationByName("Die", vecPlayerAnims)->GetTicksPerSecond() * playerDeltaTime) {
	//	bIsHit = false;
	//}
	//This is Permentant
	SetPlayerAnimation();

	//gPlayerBody->setLinearVelocity(PxVec3(getPlayerForward().x, getPlayerForward().y, getPlayerForward().z));

	if (bIsAiming)
	{
		//FoV = FovAimed;
		orientRotationToMovement = true;
		//orientRotationToMovement = false;
	}
	else
	{
		//FoV = FovUnAimed;
		orientRotationToMovement = true;
	}

	//Interaction System
	UpdateInteraction();
}

void cPlayer::SetPlayerAnimation()
{
	if (bIsMoving)
	{
		PlayAnimation(GetAnimationByName("Walk", vecPlayerAnims));
		//pPlayerAnimator->SetAnimation("Walk");
	}
	else if (bIsAiming)
	{
		PlayAnimation(GetAnimationByName("Aim", vecPlayerAnims));
		//pPlayerAnimator->SetAnimation("Aim");
	}
	else
	{
		PlayAnimation(GetAnimationByName("Idle", vecPlayerAnims));
		//pPlayerAnimator->SetAnimation("Idle");
	}
}

void cPlayer::UpdateCameraRotation(double mouseDeltaX, double mouseDeltaY)
{
	if (!bIsAiming)
	{
		CameraYaw -= mouseDeltaX * MouseSensitivity;  // Invert X-axis (yaw)
		CameraPitch -= mouseDeltaY * MouseSensitivity;  // Invert Y-axis (pitch)
	}
	else
	{
		////// Rotate player based on mouse X only
		pPlayerSkeletalMesh->rotationEulerXYZ.y += mouseDeltaX * MouseSensitivity;

		CameraYaw -= mouseDeltaX * MouseSensitivity;  // Invert X-axis (yaw)
		CameraPitch -= mouseDeltaY * MouseSensitivity;  // Invert Y-axis (pitch)
		CameraPitch = glm::clamp(CameraPitch, -20.0f, 20.0f);// Calculate camera rotation: use same direction as player
	}
}

//void cPlayer::UpdateSpringArm()
//{
//	if (!bIsAiming)
//	{
//		// Calculate base position and direction
//		glm::vec3 basePosition = pPlayerSkeletalMesh->positionXYZ + SpringArmOffset;
//		CameraLocation = basePosition - (getForwardVector() * SpringArmLength);
//
//		// Update camera position
//		::g_pFlyCamera->setEyeLocation(CameraLocation);
//	}
//	else
//	{
//		glm::vec3 basePosition = pPlayerSkeletalMesh->positionXYZ + SpringArmOffset;
//
//		CameraLocation = basePosition - getPlayerForward() * SpringArmLength + getPlayerRight() * 0.5f;
//		::g_pFlyCamera->setEyeLocation(CameraLocation);
//	}
//}
float LineTrace(glm::vec3 cameraDirection, GLuint program, float maxDistance);

void cPlayer::UpdateSpringArm()
{
	glm::vec3 basePosition = pPlayerSkeletalMesh->positionXYZ + SpringArmOffset;
	glm::vec3 desiredCameraPos;

	// Choose camera position based on aiming state
	if (!bIsAiming)
	{
		desiredCameraPos = basePosition - getForwardVector() * SpringArmLength;
	}
	else
	{
		desiredCameraPos = basePosition - getPlayerForward() * SpringArmLength + getPlayerRight() * 0.5f;
	}

	// Collision check
	glm::vec3 direction = glm::normalize(desiredCameraPos - basePosition);
	float maxDistance = glm::length(desiredCameraPos - basePosition);
	float actualDistance = LineTrace(direction, program, maxDistance); // Replace with your shader program ID

	// Adjusted target location
	glm::vec3 targetCameraLocation;
	const float collisionTolerance = 0.3f; // avoid reacting to minor collisions

	if (actualDistance < (maxDistance - collisionTolerance))
	{
		// Collision hit: place camera just before wall
		targetCameraLocation = basePosition + direction * (actualDistance - 0.1f);
	}
	else
	{
		// No significant collision
		targetCameraLocation = desiredCameraPos;
	}

	// Smoothly interpolate camera location
	float smoothFactor = 0.15f; // adjust between 0 (no movement) and 1 (instant)
	CameraLocation = glm::mix(CameraLocation, targetCameraLocation, smoothFactor);

	// Apply to fly camera
	::g_pFlyCamera->setEyeLocation(CameraLocation);
}



void cPlayer::AffectHealth(float damage)
{
	bIsHit = true;
	Health -= damage ;
	//std::cout << "Player damaged, Remaining health : " << Health << std::endl;
}

void cPlayer::AnimateSpringArmLength()
{
	float a = SpringArmLengthUnAimed;
	float b = SpringArmLengthAimed;

	cTimeline* t = new cTimeline();
	t->SetDuration(0.1f);
	t->SetEaseMode(EaseMode::EaseInOut);
	t->SetAutoRemove(true);

	t->OnUpdate = [this, a, b](float alpha)
		{
			// On Update
			SpringArmLength = glm::mix(a, b, alpha);
		};

	t->OnFinished = []()
		{
			// On Finish
		};

	pTimelineManager->AddTimeline(t);

	if (bIsAiming)
	{
		t->Play();
	}
	else if(!bIsAiming)
	{
		t->Reverse();
	}
}

void cPlayer::AnimateSprinArmOffset()
{
	glm::vec3 a = SpringArmOffsetUnAimed;
	glm::vec3 b = SpringArmOffsetAimed;

	cTimeline* t = new cTimeline();
	t->SetDuration(0.1f);
	t->SetEaseMode(EaseMode::EaseInOut);

	t->OnUpdate = [this, a, b](float alpha)
		{
			// On Update
			SpringArmOffset = glm::mix(a, b, alpha);
		};

	t->OnFinished = []()
		{
			// On Finish
		};

	pTimelineManager->AddTimeline(t);

	if (bIsAiming)
	{
		t->Play();
	}
	else
	{
		t->Reverse();
	}
}

void cPlayer::AnimateFov()
{
	float a = FovUnAimed;
	float b = FovAimed;

	cTimeline* t = new cTimeline();
	t->SetDuration(0.1f);
	t->SetEaseMode(EaseMode::EaseInOut);

	t->OnUpdate = [this, a, b](float alpha)
		{
			// On Update
			FoV = glm::mix(a, b, alpha);
		};

	t->OnFinished = []()
		{
			// On Finish
		};

	pTimelineManager->AddTimeline(t);

	if (bIsAiming)
	{
		t->Play();
	}
	else
	{
		t->Reverse();
	}
}

//float cPlayer::LineTrace(glm::vec3 direction, float maxDistance)
//{
//
//	cPhysics::sLine LASERbeam;
//	glm::vec3 LASERbeam_Offset = glm::vec3(0.0f, -0.5f, 0.0f);
//	float distance{};
//
//	if (::g_bShowLASERBeam)
//	{
//		//LASERbeam.startXYZ = ::g_pFlyCamera->getEyeLocation();
//		//LASERbeam.startXYZ = CameraLocation;
//		LASERbeam.startXYZ = this->pPlayerSkeletalMesh->positionXYZ;
//
//		// Move the LASER below the camera
//		LASERbeam.startXYZ += LASERbeam_Offset;
//		glm::vec3 LASER_ball_location = LASERbeam.startXYZ;
//
//		DrawDebugSphere(LASER_ball_location, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 0.05f, programToUse);
//		LASERbeam.endXYZ = LASER_ball_location;
//	}
//
//	float shortestDistance = maxDistance;
//
//	//Draw the end of this LASER beam
//	DrawDebugSphere(LASERbeam.endXYZ, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, programToUse);
//
//	// Now draw a different coloured ball wherever we get a collision with a triangle
//	std::vector<cPhysics::sCollision_RayTriangleInMesh> vec_RayTriangle_Collisions;
//	::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, vec_RayTriangle_Collisions, false);
//
//	glm::vec4 triColour = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
//	float triangleSize = 0.25f;
//
//	for (std::vector<cPhysics::sCollision_RayTriangleInMesh>::iterator itTriList = vec_RayTriangle_Collisions.begin();
//		itTriList != vec_RayTriangle_Collisions.end(); itTriList++)
//	{
//		for (std::vector<cPhysics::sTriangle>::iterator itTri = itTriList->vecTriangles.begin();
//			itTri != itTriList->vecTriangles.end(); itTri++)
//		{
//			DrawDebugSphere(itTri->intersectionPoint, triColour, triangleSize, programToUse);
//			triColour.r -= 0.1f;
//			triColour.g -= 0.1f;
//			triColour.b += 0.2f;
//			triangleSize *= 1.25f;
//			distance = glm::distance(itTri->intersectionPoint, CameraLocation);
//			if (distance < shortestDistance)
//			{
//				shortestDistance = distance;
//			}
//		}
//	}
//	return shortestDistance;
//}


void cPlayer::ADS()
{
	AnimateSprinArmOffset();
	AnimateSpringArmLength();
	AnimateFov();
}

void cPlayer::AddInteraction(cMasterInteractable* interactable)
{
	auto it = std::find(Interactables.begin(), Interactables.end(), interactable);
	if (it != Interactables.end())
	{
		std::cout << "Interactable Already Added\n";
	}
	else
	{
		Interactables.push_back(interactable);
	}

}

void cPlayer::RemoveInteraction(cMasterInteractable* interactable)
{
	// Find the interactable in the vector
	auto it = std::find(Interactables.begin(), Interactables.end(), interactable);

	// If the interactable is found, remove it from the vector
	if (it != Interactables.end())
	{
		Interactables.erase(it);  // Erase the interactable from the vector
	}
}

void cPlayer::GetBestInteraction()
{
	std::vector <float> dots;
	size_t index = 0;
	float maxValue = 0.0f;
	for (cMasterInteractable* interactable : Interactables)
	{
		// 1. Normalize the vector from the camera to the interactable object
		glm::vec3 directionToInteractable = glm::normalize(interactable->pInteractableMesh->positionXYZ - g_pFlyCamera->getEyeLocation());

		// 2. Get the forward vector of the camera (assuming getForwardVector() returns the forward direction)
		glm::vec3 cameraForward = getForwardVector();

		// 3. Compute the dot product
		float dotProduct = glm::dot(directionToInteractable, cameraForward);
		dots.push_back(dotProduct);
	}

	if (!dots.empty())
	{
		// Find the iterator to the maximum element in the vector
		auto maxElementIter = std::max_element(dots.begin(), dots.end());

		// Get the index by subtracting the begin iterator from the max element iterator
		index = std::distance(dots.begin(), maxElementIter);

		// Dereference the iterator to get the value
		maxValue = *maxElementIter;

		// Output the result
		//std::cout << "Maximum value: " << maxValue << std::endl;
		//std::cout << "Index of maximum value: " << index << std::endl;
	}
	else
	{
		//std::cout << "The vector is empty!" << std::endl;
	}

	// Optionally, you can use the dot product for some condition or output:
	if (maxValue > 0.8f)  // For example, if the interactable is in front of the camera (threshold can be adjusted)
	{
		// Do something when the interactable is in the forward direction of the camera
		//std::cout << "Interactable is in front of the camera!" << std::endl;
		SetBestInteraction(Interactables[index]);
	}
	else
	{
		SetBestInteraction(nullptr);
	}
}

void cPlayer::SetBestInteraction(cMasterInteractable* interactable)
{
	cMasterInteractable* pNewInteraction = interactable;
	if (pNewInteraction)
	{
		if (pNewInteraction != pSelectedInteraction)
		{
			if (pSelectedInteraction)
			{
				pSelectedInteraction->ToggleHighlight(false);
			}
			else
			{
				pSelectedInteraction = pNewInteraction;
				pSelectedInteraction->ToggleHighlight(true);
			}
		}
	}
	else
	{
		if (pSelectedInteraction)
		{
			pSelectedInteraction->ToggleHighlight(false);
			pSelectedInteraction = nullptr;
		}
	}
}

bool cPlayer::ShouldUpdateInteraction()
{
	if (Interactables.size() > 0 && !bIsAiming)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void cPlayer::UpdateInteraction()
{
	if (ShouldUpdateInteraction())
	{
		GetBestInteraction();
	}
	else
	{
		SetBestInteraction(nullptr);
	}
}



void cPlayer::PlayAnimation(Animation* animation, float animationSpeed)
{
	if (pPlayerAnimator->GetCurrentAnimation() != animation) {
		currentAnimation = animation;
		pPlayerAnimator->SetCurrentAnimation(animation);
		pPlayerAnimator->ResetCurrentTime();
		pPlayerAnimator->SpeedUpAnimation(animationSpeed);
	}
}


// Global persistent vars
float previousHealth = 100.0f;
float damageFlashTimer = 0.0f;
const float damageFlashDuration = 0.3f;

void DrawZombieShooterHUD(float deltaTime) {
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	ImVec2 screenSize = ImGui::GetIO().DisplaySize;

	// ---- HEALTH SETTINGS ----
	float currentHealth = pPlayerReference->Health;
	float healthPercent = glm::clamp(currentHealth / 100.0f, 0.0f, 1.0f);

	// Damage flash logic
	if (currentHealth < previousHealth) {
		damageFlashTimer = damageFlashDuration;
	}
	previousHealth = currentHealth;
	if (damageFlashTimer > 0.0f) {
		damageFlashTimer -= deltaTime;
	}

	// Heartbeat pulse
	float glowAlpha = 0.0f;
	if (healthPercent < 0.25f) {
		float pulse = (sinf(ImGui::GetTime() * 6.5f) + 1.0f) * 0.5f;
		glowAlpha = 150.0f * pulse;
	}

	// ---- POSITIONS ----
	ImVec2 healthPos = ImVec2(40, screenSize.y - 100);
	ImVec2 healthBarSize = ImVec2(260, 24);

	ImVec2 ammoPos = ImVec2(40, screenSize.y - 60);

	// ---- HEALTH BAR ----
	ImU32 bgColor = IM_COL32(40, 40, 40, 200);
	ImU32 fillColor = (healthPercent > 0.5f) ? IM_COL32(0, 200, 0, 255) :
		(healthPercent > 0.25f) ? IM_COL32(255, 165, 0, 255) :
		IM_COL32(200, 0, 0, 255);

	ImU32 borderColor = (damageFlashTimer > 0.0f) ? IM_COL32(255, 0, 0, 255) : IM_COL32(255, 0, 0, 150);

	// Background + Fill + Border
	drawList->AddRectFilled(healthPos, ImVec2(healthPos.x + healthBarSize.x, healthPos.y + healthBarSize.y), bgColor, 6.0f);
	drawList->AddRectFilled(healthPos,
		ImVec2(healthPos.x + healthBarSize.x * healthPercent, healthPos.y + healthBarSize.y),
		fillColor, 6.0f);
	drawList->AddRect(healthPos, ImVec2(healthPos.x + healthBarSize.x, healthPos.y + healthBarSize.y), borderColor, 6.0f, 0, 2.0f);

	// Heartbeat Glow (behind everything)
	if (glowAlpha > 0.0f) {
		ImU32 glowColor = IM_COL32(255, 0, 0, (int)glowAlpha);
		drawList->AddRect(ImVec2(healthPos.x - 6, healthPos.y - 6), ImVec2(healthPos.x + healthBarSize.x + 6, healthPos.y + healthBarSize.y + 6),
			glowColor, 10.0f, 0, 4.0f);
	}

	// Health text
	char healthText[32];
	snprintf(healthText, sizeof(healthText), "HEALTH: %d%%", (int)(healthPercent * 100));
	ImVec2 textSize = ImGui::CalcTextSize(healthText);
	drawList->AddText(ImVec2(healthPos.x + (healthBarSize.x - textSize.x) * 0.5f, healthPos.y + 3),
		IM_COL32(255, 255, 255, 230), healthText);

	// ---- AMMO BAR SETTINGS ----
	int maxAmmo = 30;  // Max ammo, you can dynamically set based on the player's weapon
	float ammoFraction = glm::clamp((float)pPlayerReference->currentAmmo / maxAmmo, 0.0f, 1.0f);

	ImVec2 ammoBarSize = ImVec2(200, 18); // Ammo bar size
	ImVec2 ammoBarPos = ammoPos;  // Position below the health bar

	// Animated ammo bar
	static float animatedAmmoFraction = 1.0f;
	animatedAmmoFraction += (ammoFraction - animatedAmmoFraction) * deltaTime * 5.0f;

	// Bar colors
	ImU32 ammoBg = IM_COL32(100, 100, 100, 255); // Set background to gray for better text visibility
	ImU32 ammoFill = (ammoFraction > 0.5f) ? IM_COL32(230, 230, 230, 255) :
		(ammoFraction > 0.25f) ? IM_COL32(255, 160, 100, 255) :
		IM_COL32(255, 60, 60, 255);
	ImU32 ammoBorder = IM_COL32(255, 255, 255, 180);

	// Draw ammo bar
	drawList->AddRectFilled(ammoBarPos, ImVec2(ammoBarPos.x + ammoBarSize.x, ammoBarPos.y + ammoBarSize.y), ammoBg, 6.0f);
	drawList->AddRectFilled(ammoBarPos,
		ImVec2(ammoBarPos.x + ammoBarSize.x * animatedAmmoFraction, ammoBarPos.y + ammoBarSize.y),
		ammoFill, 6.0f);
	drawList->AddRect(ammoBarPos, ImVec2(ammoBarPos.x + ammoBarSize.x, ammoBarPos.y + ammoBarSize.y), ammoBorder, 6.0f, 0, 1.5f);

	// Optional: bullet text overlay
	char ammoBarText[32];
	snprintf(ammoBarText, sizeof(ammoBarText), "AMMO: %d / %d", pPlayerReference->currentAmmo, maxAmmo);
	ImVec2 ammoTextSize = ImGui::CalcTextSize(ammoBarText);
	drawList->AddText(ImVec2(ammoBarPos.x + (ammoBarSize.x - ammoTextSize.x) * 0.5f, ammoBarPos.y + 1),
		IM_COL32(0, 0, 0, 180), ammoBarText);  // Shadow effect for better visibility
	//drawList->AddText(ImVec2(ammoBarPos.x + (ammoBarSize.x - ammoTextSize.x) * 0.5f - 1, ammoBarPos.y),
	//	IM_COL32(0, 0, 100, 230), ammoBarText);  // White text
}






void DrawCrosshair(int screenWidth, int screenHeight) {
	// Calculate the center of the screen
	ImVec2 center(screenWidth * 0.5f, screenHeight * 0.5f);

	// Define the size and thickness of the crosshair
	float size = 20.0f;
	float thickness = 3.0f;
	ImU32 color = IM_COL32(255, 255, 255, 255);  // White color

	// Get the draw list for background drawing
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();

	// Horizontal line (crosshair)
	drawList->AddLine(
		ImVec2(center.x - size, center.y),  // Start point (left)
		ImVec2(center.x + size, center.y),  // End point (right)
		color,  // Crosshair color
		thickness  // Line thickness
	);

	// Vertical line (crosshair)
	drawList->AddLine(
		ImVec2(center.x, center.y - size),  // Start point (top)
		ImVec2(center.x, center.y + size),  // End point (bottom)
		color,  // Crosshair color
		thickness  // Line thickness
	);
}
