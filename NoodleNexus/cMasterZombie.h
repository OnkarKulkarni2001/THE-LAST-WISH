#pragma once
#include "sMesh.h"
#include <glm/common.hpp>
#include "GLCommon.h"
#include "cPhysics.h"
#include <glm/glm.hpp>
#include "sharedThings.h"
#include <vector>
#include "cMasterInteractable.h"
#include "Animation/Model.h"
#include "Animation/Animator.h"
#include "Animation/Animation.h"
#include "PhysXShared.h"
#include "cParticleEmitter_2.h"
//#include "Windows.h"


// Forward declarations for Windows types
typedef void* HANDLE;
struct _RTL_CRITICAL_SECTION;
typedef _RTL_CRITICAL_SECTION CRITICAL_SECTION;

typedef unsigned long DWORD;
typedef void* LPVOID;

#ifndef WINAPI
#define WINAPI __stdcall
#endif

// Forward declaration for the thread function
//DWORD WINAPI ThreadFunction(LPVOID lpParam);

enum eZombieState
{
    IDLE,
    CHASE,
    ATTACK,
    DIE
};

class cMasterZombie
{
public:
    CRITICAL_SECTION* zombieCritical;  // Critical section for thread safety
    bool bIsThreadRunning;             // Flag to control the thread
    HANDLE hThread;
    bool bIsDead = false;
    //cMasterZombie();
    cMasterZombie(Model* zombieModel);
    ~cMasterZombie();

    bool IsInCameraView();


    // Function to start the worker thread
    void StartWorkerThread();

    // Function to stop the worker thread
    void StopWorkerThread();

    void ToggleVisibilityAndPhysics();

    void ToggleVisibilityAndPhysicsThreaded();
    static DWORD WINAPI ToggleThreadStart(LPVOID lpParam);  // Helper
    //CRITICAL_SECTION* zombieCriticalSection;

    Model* zombieModel;
    //Model* zombie
    //sMesh* pCapsuleCollision;

    void ZombieBeginPlay(Model* zombieModel, Animator* zombieAnimator);
    void CreateCapsulePhysics();

    void DisableCapsulePhysics();

    void ZombieUpdate(float deltaTime);

    //void PlayAnimation(Animation* animation, float animationSpeed = 1.0f);

    void ZombieUpdateRender();

    void ZombieUpdateThreaded(float deltaTime);

    void ZombieStateMachine();

    void HeadBurst();

    void PlayAnimation(Animation* animation, float animationSpeed = 1.0f);

    PxController* zombieController;
    PxControllerManager* pZombieControllerManager;
    Model* pZombieSkeletalMesh;
    Animator* pZombieAnimator;
    PxRigidDynamic* zombieActor;
    PxRigidDynamic* zombieShell;
    bool isPhysicsActive = false;
    float zombieDeltaTime;

    sMesh* pDamageMesh;
    Animation* currentAnimation;

    std::vector<sAnimationNames*> vecZombieAnims;
    eZombieState ZombieState;

    float health = 100.0f;
    float MoveSpeed = 1.0f;
    float RotationRate = 2.5f;
    float AttackRange = 1.2f;
    float ChaseRange = 10.0f;

    bool DamageSphereCollisionCheck();
    void HandleCollision();

    void MoveZombie();

    void createZombieActor(const PxVec3& position, float radius, float halfHeight);

    void CreateTriangulatedZombie(physx::PxPhysics* physics, physx::PxScene* scene, const std::vector<physx::PxVec3>& vertices, const std::vector<uint32_t>& indices, const physx::PxTransform& transform);

    void DieZombieDie();

    cParticleEmitter_2* pMyParticleEmitter = nullptr;
};