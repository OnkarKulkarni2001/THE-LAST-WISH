#pragma once
#include "PhysXWraper/cPhysXWraper.h"
#include "include/PhysX/PxPhysicsAPI.h"
#include "include/PhysX/cooking/PxCooking.h"

using namespace physx;


extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern cPhysXWraper* g_pPhysX;
extern PxPhysics* gPhysics;

#define COLLISION_BULLET  (1 << 0)
#define COLLISION_ZOMBIE  (1 << 1)