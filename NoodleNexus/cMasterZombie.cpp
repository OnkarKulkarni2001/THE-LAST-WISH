#include "cMasterZombie.h"
#include <iostream>
#include <chrono>
#include <glm/gtx/euler_angles.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>  // Include this header for glm::rotation
#include <algorithm>
#include "cPlayer.h"
#include "cSoundManager.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

extern cPlayer* pPlayerReference;
extern std::vector<sMesh*> g_vecMeshesToDraw;
extern float FoV;
float collisionCooldown = 0.0f;
float cooldownDuration = 2.0f; // e.g. 2 seconds
extern bool bIsAiming;
extern bool bUseThreading;
extern PxRigidDynamic* gPlayerBody;
extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern cPhysXWraper* g_pPhysX;
extern PxPhysics* gPhysics;
extern PxControllerManager* pControllerManager; // Manager for controllers
extern cSoundManager* g_pSoundManager;

Animation* GetAnimationByName(std::string name, std::vector<sAnimationNames*> vecAnimNames);
PxTriangleMesh* CreateMyTriangleMesh(physx::PxPhysics* physics, const std::vector<physx::PxVec3>& vertices, const std::vector<uint32_t>& indices);
bool bHasScreamed = false;


extern cParticleEmitter_2* g_pParticles;


cMasterZombie::cMasterZombie(Model* zombieModel) : bIsThreadRunning(false), hThread(NULL)
{
	this->zombieModel = zombieModel;

	//this->zombieModel = new Model();

	PxCapsuleControllerDesc desc;
	desc.height = 1.8f;
	desc.radius = 0.4f;
	desc.position = PxExtendedVec3(zombieModel->positionXYZ.x, 4.0f, zombieModel->positionXYZ.z); // Starting position
	desc.material = gMaterial; // Static friction, dynamic friction, restitution
	desc.slopeLimit = cosf(PxPi / 3); // 60 degrees slope limit
	desc.contactOffset = 0.1f;
	desc.stepOffset = 0.5f;
	desc.upDirection = PxVec3(0, 1, 0); // Y-Up world
	desc.reportCallback = nullptr; // Optional: custom hit report callback
	zombieController = pControllerManager->createController(desc);
	PxRigidDynamic* zombieActor = zombieController->getActor();

	// Step 1: Wake it up to ensure it is active in simulation
	zombieActor->wakeUp();

	// Step 2: Mark it as KINEMATIC so it can be controlled and still interact with raycasts
	zombieActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

	// Step 3: Ensure it is in a collision group that is raycastable
	PxFilterData filterData;
	filterData.word0 = 1; // Custom collision group for zombies (set as needed)
	filterData.word1 = 1; // Interaction mask (who it collides with, e.g., bullets)

	PxShape* shape;
	zombieActor->getShapes(&shape, 1);
	shape->setQueryFilterData(filterData);

	// Optional: enable scene query flags (raycast/overlap/sweep)
	shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
	pZombieControllerManager = pControllerManager;

	isPhysicsActive = true;
	//zombieRigidActor = zombieController->getActor();

	//zombieCritical = new CRITICAL_SECTION();
	//InitializeCriticalSection(zombieCritical);

	//InitializeCriticalSection(zombieCritical);

}

bool cMasterZombie::IsInCameraView() {
	//EnterCriticalSection(zombieCritical);
	glm::vec3 direction = glm::normalize(-g_pFlyCamera->getEyeLocation() + this->zombieModel->positionXYZ);
	glm::vec3 cameraForward = pPlayerReference->getForwardVector();
	float dot = glm::dot(direction, cameraForward);
	dot = glm::clamp(dot, -1.0f, 1.0f);
	//dot = glm::clamp(dot, -1.0f, 1.0f);
	float angle = glm::acos(dot); // Radians


	
	float halfFOV;
	// Check the angle to see if it's within the camera's FOV
	if (bIsAiming)
	{
		halfFOV = FoV * 2.0f;
	}
	else {
		halfFOV = FoV;
	}

	bool inFOV = angle <= halfFOV;

	// Check the distance between the camera and the zombie
	float distance = glm::distance(g_pFlyCamera->getEyeLocation(), this->zombieModel->positionXYZ);
	float maxVisibleDistance = 15.0f; // You can set this distance limit based on your game logic

	bool inVisibleRange = distance <= maxVisibleDistance;

	if (bIsAiming && distance <= 5.0f)
	{
		//halfFOV = FoV * 2.0f;
		return true;
	}
	//LeaveCriticalSection(zombieCritical);

	// Return true if the zombie is both in the camera's FOV and within visible range
	return inFOV && inVisibleRange;
}

// Worker thread function definition
//DWORD WINAPI ThreadFunction(LPVOID lpParam) {
//	cMasterZombie* pZombie = reinterpret_cast<cMasterZombie*>(lpParam);
//
//	//// Check if the thread should run and perform actions in a loop
//	//while (pZombie->bIsThreadRunning) {
//	//	// Protect shared resources with critical section
//	//	EnterCriticalSection(pZombie->zombieCritical);
//
//	//	bool isInView = pZombie->IsInCameraView();  // Call the function
//
//	//	// Do something with the result, e.g., update visibility or trigger actions
//	//	if (isInView) {
//	//		std::cout << "Zombie is in camera view." << std::endl;
//	//	}
//	//	else {
//	//		std::cout << "Zombie is out of camera view." << std::endl;
//	//	}
//
//	//	// Release the critical section
//	//	LeaveCriticalSection(pZombie->zombieCritical);
//
//	//	// Sleep for a while to prevent overloading the CPU
//	//	Sleep(100);  // Adjust the sleep duration as per your needs
//	//}
//	 
//	
//	//while (pZombie->bIsThreadRunning) {
//	//	pZombie->ToggleVisibilityAndPhysics();
//	//	Sleep(100);  // 100ms delay for smoother update
//	//}
//
//	while (pZombie->bIsThreadRunning) {
//		EnterCriticalSection(pZombie->zombieCritical);
//		pZombie->ZombieUpdate(pZombie->zombieDeltaTime);
//		LeaveCriticalSection(pZombie->zombieCritical);
//
//		Sleep(16); // ~60 FPS simulation rate
//	}
//
//
//	return 0;
//}

//// Function to start the worker thread
//void cMasterZombie::StartWorkerThread() {
//	bIsThreadRunning = true;  // Set thread to running
//	DWORD threadId;
//	this->hThread = CreateThread(
//		nullptr,           // Default security attributes
//		0,                 // Default stack size
//		ThreadFunction,    // Worker thread function
//		this,              // Pass the current object as parameter
//		0,                 // No flags
//		&threadId          // Store the thread ID
//	);
//
//	if (hThread == nullptr) {
//		std::cerr << "Error creating worker thread!" << std::endl;
//		return;
//	}
//
//	// Close the thread handle when no longer needed
//	CloseHandle(hThread);
//}
//
//// Function to stop the worker thread
//void cMasterZombie::StopWorkerThread() {
//	if (bIsThreadRunning) {
//		bIsThreadRunning = false;  // Stop the worker thread
//		WaitForSingleObject(hThread, INFINITE);  // Wait for the thread to finish
//		CloseHandle(hThread);  // Close the thread handle
//		hThread = NULL;
//	}
//}

void cMasterZombie::ToggleVisibilityAndPhysics() 
{
	bool bIsInCameraView = IsInCameraView();
	if (bIsInCameraView && !isPhysicsActive) 
	{
		this->zombieModel->bIsVisible = true;
		if(!zombieController) CreateCapsulePhysics();
		isPhysicsActive = true;
	}
	else if (!bIsInCameraView && isPhysicsActive) 
	{
		this->zombieModel->bIsVisible = false;
		if (zombieController) DisableCapsulePhysics();
		isPhysicsActive = false;
	}
}

void cMasterZombie::CreateCapsulePhysics() {
	PxCapsuleControllerDesc desc;
	desc.height = 1.8f;
	desc.radius = 0.4f;
	desc.position = PxExtendedVec3(zombieModel->positionXYZ.x, 4.0f, zombieModel->positionXYZ.z); // Starting position
	desc.material = gMaterial; // Static friction, dynamic friction, restitution
	desc.slopeLimit = cosf(PxPi / 3); // 60 degrees slope limit
	desc.contactOffset = 0.1f;
	desc.stepOffset = 0.5f;
	desc.upDirection = PxVec3(0, 1, 0); // Y-Up world
	desc.reportCallback = nullptr; // Optional: custom hit report callback
	zombieController = pControllerManager->createController(desc);

	PxRigidDynamic* zombieActor = zombieController->getActor();

	zombieActor->wakeUp();

	//// Step 2: Mark it as KINEMATIC so it can be controlled and still interact with raycasts
	zombieActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	zombieActor->setName("Zombie");

	//PxTriangleMesh* triangleMesh = CreateMyTriangleMesh(g_PhysX, vertices, indices);
	//if (!triangleMesh) {
	//	// Handle the error if triangle mesh creation fails
	//	return;
	//}

	PxShape* shape;
	zombieActor->getShapes(&shape, 1);
	// Step 3: Ensure it is in a collision group that is raycastable
	PxFilterData filterData;
	//filterData.word0 = 1; // Custom collision group for zombies (set as needed)
	//filterData.word1 = 1; // Interaction mask (who it collides with, e.g., bullets)
	filterData.word0 = COLLISION_ZOMBIE;  // This actor is a zombie
	filterData.word1 = COLLISION_BULLET;  // It should collide with bullets

	shape->setSimulationFilterData(filterData);
	shape->setQueryFilterData(filterData);

	// Optional: enable scene query flags (raycast/overlap/sweep)
	shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);

	//zombieActor->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);

	pZombieControllerManager = pControllerManager;

	//pZombieSkeletalMesh->positionXYZ = glm::vec3(zombieController->getPosition().x, pZombieSkeletalMesh->positionXYZ.y, zombieController->getPosition().z);
}

void cMasterZombie::DisableCapsulePhysics() {
	if (!zombieController) return;

	PxRigidDynamic* actor = zombieController->getActor();
	if (actor) {
		actor->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);
	}

	// Correct: release the controller itself, the manager will handle the rest
	zombieController->release();
	zombieController = nullptr;

}


cMasterZombie::~cMasterZombie()
{
	//StopWorkerThread();  // Ensure the thread is cleaned up when the object is destroyed
	//DeleteCriticalSection(zombieCritical);  // Clean up the critical section

	pZombieControllerManager->release();
	pZombieControllerManager = nullptr;

}

void cMasterZombie::ZombieBeginPlay(Model* zombieModel, Animator* zombieAnimator)
{
	pZombieSkeletalMesh = zombieModel;
	pZombieAnimator = zombieAnimator;
	//pZombieAnimator->currentAnimation = pZombieAnimator->animations["Idle"];
	//ZombieState = IDLE;
	

	pDamageMesh = new sMesh();
	pDamageMesh->modelFileName = "assets/models/Sphere_radius_1_xyz_N_uv.ply";
	pDamageMesh->bDoNotLight = true;
	pDamageMesh->bIsVisible = false;
	pDamageMesh->bIsWireframe = true;
	pDamageMesh->uniformScale = 0.3f;
	g_vecMeshesToDraw.push_back(pDamageMesh);

	//this->pMyParticleEmitter = new cParticleEmitter_2();
	//this->pMyParticleEmitter->SetMaximumNumberOfParticles(50000); // small emitter just for headburst

}

bool bVisibilityForced = false;
void cMasterZombie::ZombieUpdate(float deltaTime)
{
	//CriticalSectionGuard lock(zombieCritical);

	//std::cout << "ZombieUpdate called" << std::endl;
	if(health > 0.0f) ToggleVisibilityAndPhysics();

	//pMyParticleEmitter->Update(deltaTime);

	glm::vec3 rightHandPos = pZombieSkeletalMesh->GetBonePosition(pZombieAnimator, "mixamorig_RightHand");
	//std::cout << "RightHand: " << rightHandPos.x << " ," << rightHandPos.y << " ," << rightHandPos.z << std::endl;
	zombieDeltaTime = deltaTime;
	
	pDamageMesh->positionXYZ = rightHandPos;

	if (health <= 0.0f) 
	{
		if (!bIsDead)
		{
			ZombieState = DIE;
			PlayAnimation(GetAnimationByName("Die", vecZombieAnims));
			GetAnimationByName("Die", vecZombieAnims)->SetLooping(GetAnimationByName("Die", vecZombieAnims), false);
	
			//bIsDead = true;
			g_pSoundManager->PlaySoundEffect("ZombieDeath");
			DieZombieDie();
			DisableCapsulePhysics();
		}
		
		return;
		//GetAnimationByName("Walk", vecZombieAnims)->SetLooping(GetAnimationByName("Walk", vecZombieAnims), false);
		//GetAnimationByName("Attack", vecZombieAnims)->SetLooping(GetAnimationByName("Attack", vecZombieAnims), false);
	}

	if (ZombieState != DIE)
	{
		float distanceToPlayer = glm::distance(
			pZombieSkeletalMesh->positionXYZ,
			pPlayerReference->pPlayerSkeletalMesh->positionXYZ
		);

		// Update zombie state based on distance
		if (distanceToPlayer <= AttackRange)
		{
			ZombieState = ATTACK;

		}
		else if (distanceToPlayer <= ChaseRange)
		{
			ZombieState = CHASE;
			if (!bHasScreamed)
			{
				bHasScreamed = true;
				FMOD_VECTOR zombiePos;
				zombiePos.x = pZombieSkeletalMesh->positionXYZ.x;
				zombiePos.y = pZombieSkeletalMesh->positionXYZ.y;
				zombiePos.z = pZombieSkeletalMesh->positionXYZ.z;
				g_pSoundManager->Play3DSound("ZombieScream", zombiePos);
			}
		}
		else
		{
			ZombieState = IDLE;
		}

		ZombieStateMachine();
		MoveZombie();
	}
	else
	{
		ZombieStateMachine(); // Still play "Die" animation if needed
	}

	if (ZombieState == ATTACK)
	{
		if (DamageSphereCollisionCheck())
		{
			if (collisionCooldown <= 0.0f)
			{
				HandleCollision();
				collisionCooldown = cooldownDuration;
			}
		}
	}
}

//void cMasterZombie::PlayAnimation(Animation* animation, float animationSpeed)
//{
//	if (pZombieAnimator->GetCurrentAnimation() != animation) {
//		currentAnimation = animation;
//		pZombieAnimator->SetCurrentAnimation(animation);
//		pZombieAnimator->ResetCurrentTime();
//		pZombieAnimator->SpeedUpAnimation(animationSpeed);
//	}
//}

void cMasterZombie::ZombieStateMachine()
{
	switch (ZombieState)
	{
	case IDLE:
		PlayAnimation(GetAnimationByName("Idle", vecZombieAnims));
		//pZombieAnimator->SetAnimation("Idle");
		break;
	case CHASE:
		PlayAnimation(GetAnimationByName("Walk", vecZombieAnims));
		//pZombieAnimator->SetAnimation("Walk");
		break;
	case ATTACK:
		PlayAnimation(GetAnimationByName("Attack", vecZombieAnims));
		//pZombieAnimator->SetAnimation("Attack");
		break;
	case DIE:
		MoveSpeed = 0.0f;
		

		break;
	default:
		PlayAnimation(GetAnimationByName("Idle", vecZombieAnims));
		break;
	}
}

void cMasterZombie::HeadBurst()
{


	glm::vec3 headPosition = this->pZombieSkeletalMesh->positionXYZ;
	headPosition.y += 1.7f;
	//pMyParticleEmitter->SetSourceLocation(headPosition);

	// Ignore setting velocity here — handled inside headburst
	// ::g_pParticles->SetInitalVelocity(...);

	//pMyParticleEmitter->CreateBurstParticles(200);

	g_pParticles->SetSourceLocation(headPosition);
	if (bUseThreading) {
		//g_pParticles->CreateBurstParticles(30000, headPosition);
		const unsigned int totalParticles = 5'000;
		const unsigned int particlesPerFrame = 1000;
		unsigned int particlesRemaining = totalParticles;

		while (particlesRemaining > 0)
		{
			unsigned int particlesThisFrame = std::min(particlesRemaining, particlesPerFrame);
			g_pParticles->CreateBurstParticles(particlesThisFrame, headPosition);
			particlesRemaining -= particlesThisFrame;
		}
	}
	else g_pParticles->CreateBurstParticles(5'000, headPosition);
}

void cMasterZombie::PlayAnimation(Animation* animation, float animationSpeed)
{
	if (pZombieAnimator->GetCurrentAnimation() != animation) {
		currentAnimation = animation;
		pZombieAnimator->ResetCurrentTime();
		pZombieAnimator->SetCurrentAnimation(animation);
		pZombieAnimator->SpeedUpAnimation(animationSpeed);
	}
}

bool cMasterZombie::DamageSphereCollisionCheck()
{
	// Get positions
	glm::vec3 damageCenter = pDamageMesh->positionXYZ;
	glm::vec3 playerPos = pPlayerReference->pPlayerSkeletalMesh->positionXYZ;

	// Define the damage radius (you can adjust this)
	const float damageRadius = 0.07f;

	// Check distance between player and damage center
	float distance = glm::distance(glm::vec2(damageCenter.x, damageCenter.z), glm::vec2(playerPos.x, playerPos.z));



	if (distance <= damageRadius)
	{
		// Player is within damage radius


		// Optionally, apply damage to the player here
		// pPlayerReference->ApplyDamage(10.0f);
		pPlayerReference->AffectHealth(10.0f);
		return true;
	}

	return false;
}

void cMasterZombie::HandleCollision()
{
	//std::cout << "Zombie hit the player!\n";
	pPlayerReference->AffectHealth(10.0f);
}


void cMasterZombie::MoveZombie()
{
	if (ZombieState == CHASE)
	{
		glm::vec3 direction = glm::normalize(
			pPlayerReference->pPlayerSkeletalMesh->positionXYZ -
			pZombieSkeletalMesh->positionXYZ
		);

		float targetYaw = glm::degrees(atan2(direction.x, direction.z));

		// Normalize yaw
		float& currentYaw = pZombieSkeletalMesh->rotationEulerXYZ.y;
		float deltaYaw = targetYaw - currentYaw;

		if (deltaYaw > 180.0f) deltaYaw -= 360.0f;
		if (deltaYaw < -180.0f) deltaYaw += 360.0f;

		// Smooth turning
		if (std::abs(deltaYaw) > RotationRate)
			currentYaw += RotationRate * glm::sign(deltaYaw);
		else
			currentYaw = targetYaw;

		// Move forward
		//pZombieSkeletalMesh->positionXYZ.x += direction.x * MoveSpeed;
		//pZombieSkeletalMesh->positionXYZ.z += direction.z * MoveSpeed;
		PxControllerFilters filters;

		if (isPhysicsActive) {
			zombieController->move(PxVec3(direction.x, 0.0f, direction.z) * MoveSpeed * zombieDeltaTime, 0.001f, zombieDeltaTime, filters);
			pZombieSkeletalMesh->positionXYZ = glm::vec3(zombieController->getPosition().x, pZombieSkeletalMesh->positionXYZ.y, zombieController->getPosition().z);
		}
	}
}

void cMasterZombie::createZombieActor(const PxVec3& position, float radius, float halfHeight)
{
	// 1. Create material
	PxMaterial* material = gPhysics->createMaterial(0.5f, 0.5f, 0.5f); // friction, restitution

	// 2. Create capsule shape
	PxCapsuleGeometry capsuleGeometry(radius, halfHeight);
	PxShape* capsuleShape = gPhysics->createShape(capsuleGeometry, *material);

	// 3. Create dynamic rigid body for AI Zombie
	PxTransform transform(position);
	zombieActor = gPhysics->createRigidDynamic(transform);

	// 4. Attach shape to the actor
	zombieActor->attachShape(*capsuleShape);

	zombieActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

	// 5. Set actor properties
	PxRigidBodyExt::updateMassAndInertia(*zombieActor, 50.0f); // mass of the zombie

	// 6. Add actor to scene
	gScene->addActor(*zombieActor);

	// 7. Clean up shape (safe after attaching to actor)
	capsuleShape->release();

	// (Optional) You can store `zombieActor` somewhere to control movement later!
}

void cMasterZombie::CreateTriangulatedZombie(physx::PxPhysics* physics, physx::PxScene* scene, const std::vector<physx::PxVec3>& vertices, const std::vector<uint32_t>& indices, const physx::PxTransform& transform) {
	// Create the triangle mesh for collision detection
	physx::PxTriangleMesh* triangleMesh = CreateMyTriangleMesh(physics, vertices, indices);
	if (!triangleMesh) {
		// Handle error (failed to create the triangle mesh)
		return;
	}

	// Create a shape using the triangle mesh for the zombie
	physx::PxShape* shape = physics->createShape(physx::PxTriangleMeshGeometry(triangleMesh), *gMaterial); // gMaterial is assumed to be your material, initialize it as needed

	// Create a rigid dynamic actor for the zombie
	physx::PxRigidDynamic* zombie = physics->createRigidDynamic(transform);

	// Attach the shape to the zombie's rigid body
	zombie->attachShape(*shape);

	// Optionally, adjust the mass and inertia for the zombie's rigid body
	physx::PxRigidBodyExt::updateMassAndInertia(*zombie, 10.0f); // Adjust the mass as needed

	// Add the zombie to the scene for physics simulation
	scene->addActor(*zombie);
}

void cMasterZombie::DieZombieDie()
{
	bIsDead = true;
	this->HeadBurst();
	//pZombieSkeletalMesh->bIsVisible = false;
	//std::cout << "ZOMBIE DEATH CALLED\n";
	//this->zombieModel->bIsVisible = false;
	//Model* pHeadlessZombie = new Model("res/Zombie/ZombieNoHead.dae");
	//this->zombieModel = pHeadlessZombie;
	//this->zombieModel->bIsVisible = true;
}


DWORD WINAPI cMasterZombie::ToggleThreadStart(LPVOID lpParam)
{
	cMasterZombie* pZombie = (cMasterZombie*)lpParam;

	EnterCriticalSection(pZombie->zombieCritical);

	pZombie->ToggleVisibilityAndPhysics();

	LeaveCriticalSection(pZombie->zombieCritical);

	return 0;
}

void cMasterZombie::ToggleVisibilityAndPhysicsThreaded()
{
	HANDLE hThread = CreateThread(
		NULL,               // default security
		0,                  // default stack size
		cMasterZombie::ToggleThreadStart,  // thread function
		(LPVOID)this,       // pass "this" pointer
		0,                  // run immediately
		NULL                // don't care about thread id
	);

	if (hThread)
	{
		CloseHandle(hThread);  // We don't need to track or wait for it
	}
}

