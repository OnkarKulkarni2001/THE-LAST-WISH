#pragma once

#include "sharedThings.h"
#include "PhysXShared.h"
#include "cMasterZombie.h"

class cBullet 
{
public:
	sMesh* bulletMesh = nullptr;
	PxRigidDynamic* bulletPhysX = nullptr;
	PxVec3 bulletSpawn;
	PxVec3 bulletVelocity;
	bool bIsAlive;
	float lifeTime = 10.0f;
	float currentTime = 0.0f;
	float bulletSpeed = 100.0f;
	glm::vec3 previousPosition;

	cBullet();

	cBullet(sMesh* bulletMesh, glm::vec3 fireDirection);

	//cBullet(sMesh* bulletMesh);
	~cBullet();

	PxRigidDynamic* CreatePhysX(PxVec3 spawn, PxVec3 velocity);

	//PxRigidDynamic* CreatePhysX();
	bool CheckPhysXCollision(PxRigidDynamic* bulletActor, PxController* zombieController);
	bool CheckRaycastCollision(PxVec3 startPosition, PxVec3 direction);
	void HandleBulletHit(cMasterZombie* zombie);
	void CheckCollision();
	void Update(float deltaTime);
	void CreateParticleTrail();
	static void DestroyBullet(cBullet* bullet);
};