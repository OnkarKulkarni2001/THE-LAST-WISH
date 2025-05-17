#include "cBullet.h"
#include "cPlayer.h"
#include <vector>
#include <iostream>
//#include <glm/glm.hpp>
#include "cParticleEmitter_2.h"

extern cPlayer* pPlayerReference;
std::vector<cBullet*> vecBullets;
extern std::vector<cMasterZombie*> AllZombies;
extern std::vector<sMesh*> g_vecMeshesToDraw;
//extern cParticleEmmiterOwn* g_myParticleEmmiter;
extern cParticleEmitter_2* g_pParticles;
extern GLuint program;
extern bool bIsAiming;
extern float FoV;
extern GLFWwindow* window;
extern bool bUseThreading;
glm::vec3 getCameraLookDirection();

cBullet::cBullet()
{
}

cBullet::cBullet(sMesh* bulletMesh, glm::vec3 fireDirection)
{
	//this->bulletMesh = bulletMesh;
	//this->bulletPhysX = CreatePhysX();
	////this->bulletSpawn = pPlayerReference->pPlayerSkeletalMesh->positionXYZ + glm::vec3(0.0f, 1.5f, 0.0f) + (pPlayerReference->getPlayerForward() * 1.5f);
	//this->bulletSpawn = PxVec3(pPlayerReference->pPlayerSkeletalMesh->positionXYZ.x, 3.0f, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.z) +
	//					PxVec3(pPlayerReference->getPlayerForward().x, 0.0f, pPlayerReference->getPlayerForward().z) * 1.5f;
	//
	//this->bulletVelocity = PxVec3(pPlayerReference->getPlayerForward().x, 0.0f, pPlayerReference->getPlayerForward().z) * bulletSpeed;
	//this->bulletPhysX->setLinearVelocity(bulletVelocity);


	//vecBullets.push_back(this);

/////////////////////////////////////////////////////////////////////////////////////////
	//this->bulletMesh = bulletMesh;
	//this->bulletPhysX = CreatePhysX(this->bulletSpawn, this->bulletVelocity);
	//this->bulletSpawn = PxVec3(pPlayerReference->pPlayerSkeletalMesh->positionXYZ.x, 3.0f, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.z) +
	//	PxVec3(pPlayerReference->getPlayerForward().x, 0.0f, pPlayerReference->getPlayerForward().z) * 1.5f;
	//this->bulletVelocity = PxVec3(fireDirection.x, fireDirection.y, fireDirection.z) * bulletSpeed;
	////this->bulletVelocity = PxVec3(pPlayerReference->getPlayerForward().x, 0.0f, pPlayerReference->getPlayerForward().z) * bulletSpeed;
	//this->bulletPhysX->setLinearVelocity(bulletVelocity);

	//vecBullets.push_back(this);

	//glUniform3f(glGetUniformLocation(program, "muzzlePos"), bulletSpawn.x, bulletSpawn.y, bulletSpawn.z);
	//glUniform3f(glGetUniformLocation(program, "fireDir"), fireDirection.x, fireDirection.y, fireDirection.z);

	//pPlayerReference->currentAmmo--;
	//CreateParticleTrail(); // <- Here


	this->bulletMesh = bulletMesh;
	this->bulletSpawn = PxVec3(bulletMesh->positionXYZ.x, bulletMesh->positionXYZ.y, bulletMesh->positionXYZ.z);

	this->bulletVelocity = PxVec3(fireDirection.x, fireDirection.y, fireDirection.z) * bulletSpeed;

	this->bulletPhysX = CreatePhysX(this->bulletSpawn, this->bulletVelocity); // Modify CreatePhysX too
	vecBullets.push_back(this);

	glUniform3f(glGetUniformLocation(program, "muzzlePos"), bulletSpawn.x, bulletSpawn.y, bulletSpawn.z);
	glUniform3f(glGetUniformLocation(program, "fireDir"), fireDirection.x, fireDirection.y, fireDirection.z);

	pPlayerReference->currentAmmo--;
}

cBullet::~cBullet()
{
	/*this->bulletPhysX->release();
	this->bulletPhysX = nullptr;*/
}

PxRigidDynamic* cBullet::CreatePhysX(PxVec3 spawn, PxVec3 velocity)
{
	// PhysX Setup
	//PxVec3 physxPosition(this->bulletMesh->positionXYZ.x, 5.0f, this->bulletMesh->positionXYZ.z);
	//PxTransform bulletTransform(physxPosition);

	//// Create a dynamic rigid body
	//PxRigidDynamic* pBulletActor = gPhysics->createRigidDynamic(bulletTransform);
	//PxShape* shape = gPhysics->createShape(PxSphereGeometry(0.3f), *gMaterial);
	//pBulletActor->attachShape(*shape);
	//PxRigidBodyExt::updateMassAndInertia(*pBulletActor, 1.0f);

	//// Set initial velocity
	//PxVec3 bulletVelocity = PxVec3(pPlayerReference->getPlayerForward().x, pPlayerReference->getPlayerForward().y, pPlayerReference->getPlayerForward().z) * bulletSpeed;
	//pBulletActor->setLinearVelocity(bulletVelocity);

	//// Add to the PhysX scene
	//gScene->addActor(*pBulletActor);

	//return pBulletActor;
	
	//////////////////////////////////////////////////////////////////////////////////////////
	//PxVec3 physxPosition(this->bulletMesh->positionXYZ.x, 5.0f, this->bulletMesh->positionXYZ.z);
	//PxTransform bulletTransform(physxPosition);

	//PxRigidDynamic* pBulletActor = gPhysics->createRigidDynamic(bulletTransform);
	//PxShape* shape = gPhysics->createShape(PxSphereGeometry(0.1f), *gMaterial); // smaller, more accurate
	//shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
	//shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);

	//PxFilterData filterData;
	//filterData.word0 = COLLISION_BULLET;
	//filterData.word1 = COLLISION_ZOMBIE;
	//shape->setSimulationFilterData(filterData);
	//
	//pBulletActor->attachShape(*shape);
	//PxRigidBodyExt::updateMassAndInertia(*pBulletActor, 0.1f); // lighter bullets
	//pBulletActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false); // enable gravity
	//pBulletActor->setLinearDamping(0.01f); // low air resistance
	//pBulletActor->setAngularDamping(0.0f); // no spin loss
	//pBulletActor->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true); // for high-speed collision

	//PxVec3 bulletVelocity = PxVec3(pPlayerReference->getPlayerForward().x, pPlayerReference->getPlayerForward().y, pPlayerReference->getPlayerForward().z) * bulletSpeed;
	//pBulletActor->setLinearVelocity(bulletVelocity);

	//gScene->addActor(*pBulletActor);
	//return pBulletActor;
	spawn.y += 2.5f;
	PxTransform bulletTransform(spawn);
	std::cout << "Bullet Start Position: "
		<< "X: " << spawn.x << " Y: " << spawn.y << " Z: " << spawn.z << std::endl;

	PxRigidDynamic* pBulletActor = gPhysics->createRigidDynamic(bulletTransform);
	PxShape* shape = gPhysics->createShape(PxSphereGeometry(0.1f), *gMaterial);
	shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
	shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);

	PxFilterData filterData;
	filterData.word0 = COLLISION_BULLET;
	filterData.word1 = COLLISION_ZOMBIE;
	shape->setSimulationFilterData(filterData);

	pBulletActor->attachShape(*shape);
	PxRigidBodyExt::updateMassAndInertia(*pBulletActor, 0.1f);
	pBulletActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
	pBulletActor->setLinearDamping(0.01f);
	pBulletActor->setAngularDamping(0.0f);
	pBulletActor->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);

	pBulletActor->setLinearVelocity(velocity);
	gScene->addActor(*pBulletActor);

	return pBulletActor;
}

//bool cBullet::CheckPhysXCollision(PxRigidDynamic* bulletActor, PxController* zombieController) {
//	//if (!bulletActor || !zombieController) return false;
//
//	// Get bullet and zombie positions
//	PxTransform bulletPose = bulletActor->getGlobalPose();
//	PxExtendedVec3 zombiePosEx = zombieController->getPosition();
//	PxVec3 zombiePos(float(zombiePosEx.x), float(zombiePosEx.y), float(zombiePosEx.z));
//
//	// Adjust bullet Y to match zombie's coordinate system
//	PxVec3 bulletOrigin(bulletPose.p.x, bulletPose.p.y, bulletPose.p.z); // Undo PhysX Y-offset
//
//	// Raycast from bullet toward zombie
//	PxVec3 dir = zombiePos - bulletOrigin;
//	float distance = dir.magnitude();
//	//if (distance < 0.001f) return false;
//	dir /= distance;
//
//	/*PxRaycastBuffer hit;
//	bool isHit = gScene->raycast(bulletOrigin, dir, distance, hit, PxHitFlag::eDEFAULT,
//		PxQueryFilterData(PxQueryFlag::eDYNAMIC));
//
//	std::cout << "isHit: " << isHit << std::endl;
//	std::cout << "hit.block.actor: " << zombieController->getActor() << std::endl;
//	std::cout << "return: " << bool(isHit && hit.block.actor == zombieController->getActor()) << std::endl;
//	return isHit && hit.block.actor == zombieController->getActor();*/
//	PxRaycastBuffer hit;
//	bool isHit = gScene->raycast(
//		bulletOrigin, dir, distance, hit,
//		PxHitFlag::eDEFAULT,
//		PxQueryFilterData(PxQueryFlag::eDYNAMIC | PxQueryFlag::eANY_HIT)
//	);
//
//	if (isHit) {
//		for (PxU32 i = 0; i < hit.nbTouches; ++i) {
//			if (hit.touches[i].actor == zombieController->getActor()) {
//				std::cout << "HIT confirmed with zombie actor!" << std::endl;
//				return true;
//			}
//		}
//		std::cout << "Ray hit something else." << std::endl;
//	}
//	return false;
//}

bool cBullet::CheckPhysXCollision(PxRigidDynamic* bulletActor, PxController* zombieController) {
	if (!bulletActor || !zombieController) return false;

	// Get bullet and zombie positions
	PxVec3 bulletPos = bulletActor->getGlobalPose().p;

	PxExtendedVec3 zombiePosEx = zombieController->getPosition();
	PxVec3 zombiePos(static_cast<float>(zombiePosEx.x), static_cast<float>(zombiePosEx.y), static_cast<float>(zombiePosEx.z));

	// Define proximity threshold (collision radius)
	const float hitRadius = 1.0f; // Adjust this as needed based on bullet/zombie size

	// Calculate distance between bullet and zombie
	float distance = (bulletPos - zombiePos).magnitude();

	if (distance <= hitRadius) {
		//std::cout << "Proximity HIT: Distance = " << distance << std::endl;
		return true;
	}
	else {
		//std::cout << "Too far: Distance = " << distance << std::endl;
		return false;
	}
}
//bool cBullet::CheckPhysXCollision(PxRigidDynamic* bulletActor, PxController* zombieController) {
//	if (!bulletActor || !zombieController) return false;
//
//	PxTransform bulletPose = bulletActor->getGlobalPose();
//	PxExtendedVec3 zombiePosEx = zombieController->getPosition();
//	PxVec3 zombiePos(float(zombiePosEx.x), float(zombiePosEx.y), float(zombiePosEx.z));
//	PxVec3 bulletOrigin = bulletPose.p;
//
//	PxVec3 dir = zombiePos - bulletOrigin;
//	float distance = dir.normalize(); // Normalizes and returns old length
//
//	PxRaycastBuffer hit;
//	bool isHit = gScene->raycast(
//		bulletOrigin, dir, distance, hit,
//		PxHitFlag::eDEFAULT,
//		PxQueryFilterData(PxQueryFlag::eDYNAMIC)
//	);
//
//	if (isHit && hit.block.actor == zombieController->getActor()) {
//		std::cout << "HIT confirmed with zombie actor!" << std::endl;
//		return true;
//	}
//	return false;
//}


bool cBullet::CheckRaycastCollision(PxVec3 startPosition, PxVec3 direction) {
	// Maximum distance for the raycast (bullet range)
	PxReal maxDistance = 1000.0f;

	// Create a raycast buffer to hold the result of the raycast
	PxRaycastBuffer hit;

	// Perform the raycast
	bool hitResult = gScene->raycast(
		startPosition, direction, maxDistance, hit,
		PxHitFlag::eDEFAULT,
		PxQueryFilterData(PxQueryFlag::eDYNAMIC | PxQueryFlag::eANY_HIT)
	);

	// Check if the raycast hit something
	if (hitResult) 
	{
		// Iterate through the touches in the hit buffer
		for (PxU32 i = 0; i < hit.nbTouches; ++i) {
			// If the hit shape is valid and has user data (assumed to be a pointer to a zombie)
			if (hit.touches[i].shape && hit.touches[i].shape->getActor()->userData != nullptr) {
				// Retrieve the actor's user data, assumed to be a pointer to cMasterZombie
				cMasterZombie* zombie = reinterpret_cast<cMasterZombie*>(hit.touches[i].shape->getActor()->userData);

				// Handle the bullet hit on the zombie
				HandleBulletHit(zombie);

				// Bullet hit a zombie, return true
				return true;
			}
		}
	}

	return false; // No hit detected
}




void cBullet::HandleBulletHit(cMasterZombie* zombie) {
	zombie->health -= 25.0f;
	std::cout << "zombie hit, current health: " << zombie->health << std::endl;
}

void cBullet::CheckCollision() 
{
	for (int i = vecBullets.size()-1; i >= 0; i--)
	{
		for (cMasterZombie* enemy : AllZombies)
		{
			if (!enemy || !enemy->zombieController) continue;
			if (!vecBullets[i]->bulletPhysX) continue;

			//PxVec3 bulletStartPosition = vecBullets[i]->bulletPhysX->getGlobalPose().p;
			//PxVec3 bulletDirection = vecBullets[i]->bulletPhysX->getLinearVelocity().getNormalized();

			//std::cout << "Bullet Start Position: "
			//	<< "X: " << bulletStartPosition.x << " Y: " << bulletStartPosition.y << " Z: " << bulletStartPosition.z << std::endl;

			if (CheckPhysXCollision(vecBullets[i]->bulletPhysX, enemy->zombieController))
			//if (CheckRaycastCollision(bulletSpawn, this->bulletVelocity))
			{
				std::cout << "Bullet hit a zombie!" << std::endl;
				// Reduce health of the specific zombie hit
				HandleBulletHit(enemy);

				// Hide and remove the bullet
				//bullet.pBulletMesh->bIsVisible = false;
				//gScene->removeActor(*bullet.pPhysXActor);
				////delete bullet.pBulletMesh;
				//BulletDataVector.erase(BulletDataVector.begin() + i);
				//vecBullets[i]->bulletMesh->bIsVisible = false;
				DestroyBullet(vecBullets[i]);

				break; // Stop checking for other zombies once hit
			}
		}
	}
}

void cBullet::Update(float deltaTime)
{
	this->currentTime += deltaTime;
	//CreateParticleTrail();
	glUniform1f(glGetUniformLocation(program, "timeSinceShot"), this->currentTime);

	if (currentTime < lifeTime) 
	{
		if (this->bulletPhysX != nullptr) 
		{
			this->bulletMesh->positionXYZ = glm::vec3(this->bulletPhysX->getGlobalPose().p.x, this->bulletPhysX->getGlobalPose().p.y - 2.5f, this->bulletPhysX->getGlobalPose().p.z);
			//std::cout << "bullet position: (" << this->bulletMesh->positionXYZ.x << ", " << this->bulletMesh->positionXYZ.y - 2.5f << ", " << this->bulletMesh->positionXYZ.z << ")" << std::endl;
			CheckCollision();
		}
	}
	else {
		DestroyBullet(this);
	}
}

void cBullet::CreateParticleTrail() {
	
	//::g_pParticles->SetInitalVelocity(
	//	glm::vec3(-1.0f, 3.0f, -1.0f),        // Min
	//	glm::vec3(1.0f, 10.0f, 1.0f));       // Max
	//
	//::g_pParticles->CreateParticlesImmediately(10);

	//::g_pParticles->SetSourceLocation(this->bulletMesh->positionXYZ);
	//::g_pParticles->SetInitalVelocity(
	//	glm::vec3(-0.5f, 0.2f, -0.5f),
	//	glm::vec3(0.5f, 1.0f, 0.5f)
	//);
	//::g_pParticles->SetNumberOfParticlesPerFrame(5.0f);
	//::g_pParticles->TurnOn(); // ensure emitter is active
}

void cBullet::DestroyBullet(cBullet* bullet)
{
	/*for (int bulletIndex = 0; bulletIndex < vecBullets.size(); bulletIndex++) {
		std::vector<sMesh*>::iterator it = std::find(g_vecMeshesToDraw.begin(), g_vecMeshesToDraw.end(), vecBullets[bulletIndex]->bulletMesh);
		if (it != g_vecMeshesToDraw.end()) {
			g_vecMeshesToDraw.erase(it);
		}
		vecBullets[bulletIndex]->bulletPhysX->release();
		vecBullets[bulletIndex]->bulletPhysX = nullptr;
		delete vecBullets[bulletIndex];
		vecBullets.erase(bulletIndex + vecBullets.begin());
	}*/

	if (!bullet) return;

	// Remove mesh from draw list
	std::vector<sMesh*>::iterator it = std::find(g_vecMeshesToDraw.begin(), g_vecMeshesToDraw.end(), bullet->bulletMesh);
	if (it != g_vecMeshesToDraw.end()) {
		g_vecMeshesToDraw.erase(it);
	}

	// Release PhysX actor
	if (bullet->bulletPhysX) {
		//gScene->removeActor(*bullet->bulletPhysX);
		bullet->bulletPhysX->release();
		bullet->bulletPhysX = nullptr;
	}

	// Erase from vecBullets
	std::vector<cBullet*>::iterator bulletIt = std::find(vecBullets.begin(), vecBullets.end(), bullet);
	if (bulletIt != vecBullets.end()) {
		vecBullets.erase(bulletIt);
	}

	// Delete bullet object
	delete bullet;

}


glm::vec3 getCameraLookDirection()
{
	// Assuming screen center (normalized device coordinates)
	float ndcX = 0.0f;  // center
	float ndcY = 0.0f;  // center
	glm::vec4 rayClip(ndcX, ndcY, -1.0f, 1.0f);

	float ratio;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	ratio = width / (float)height;

	glm::mat4 matProjection = glm::mat4(1.0f);
	glm::mat4 matView = glm::mat4(1.0f);
	glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);


	//--------------------------------------------------------------------------PlayerCam----------------------------------------------------------------------------------------
	if (!bIsAiming) {
		matView = glm::lookAt(
			::g_pFlyCamera->getEyeLocation(),
			glm::vec3(pPlayerReference->pPlayerSkeletalMesh->positionXYZ.x, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.y + 1.0f, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.z),
			upVector);
	}
	else {
		matView = glm::lookAt(
			::g_pFlyCamera->getEyeLocation() /*+ pPlayerReference->getPlayerRight() * 0.5f - pPlayerReference->getPlayerForward() * pPlayerReference->SpringArmLength*/,
			glm::vec3(pPlayerReference->pPlayerSkeletalMesh->positionXYZ.x, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.y + 1.0f + pPlayerReference->getForwardVector().y, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.z) +
			glm::vec3(pPlayerReference->getPlayerRight().x * 0.4f, 0.5f, pPlayerReference->getPlayerRight().z * 0.4f),
			upVector);
	}
	if (pPlayerReference->Health <= 0.0f) {
		matView = glm::lookAt(
			::g_pFlyCamera->getEyeLocation(),
			glm::vec3(pPlayerReference->pPlayerSkeletalMesh->positionXYZ),
			upVector);
	}
	//--------------------------------------------------------------------------PlayerCam----------------------------------------------------------------------------------------
	matProjection = glm::perspective(FoV,
		ratio,
		0.1f,
		5000.0f);

	// Inverse projection to get eye space
	glm::vec4 rayEye = glm::inverse(matProjection) * rayClip;
	rayEye.z = -1.0f;
	rayEye.w = 0.0f;

	// World space direction
	glm::vec3 rayWorld = glm::vec3(glm::inverse(matView) * rayEye);
	rayWorld = glm::normalize(rayWorld);
	return rayWorld;
}
