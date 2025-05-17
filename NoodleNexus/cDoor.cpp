#include "cDoor.h"
#include <iostream>
#include "cTimeline.h"
#include "cTimelineManager.h"
#include <glm/glm.hpp>
#include "cSoundManager.h"

extern cPlayer* pPlayerReference;
extern cTimelineManager* pTimelineManager;
extern PxScene* gScene;
extern PxMaterial* gMaterial;
extern cPhysXWraper* g_pPhysX;
extern PxPhysics* gPhysics;
extern cSoundManager* g_pSoundManager;
bool doOnce = false;

cDoor::cDoor(sMesh* doorMesh) {
    this->doorMesh = doorMesh;
    createStaticWall(PxVec3(doorMesh->positionXYZ.x, 5.0f, doorMesh->positionXYZ.z), PxVec3(2.0f, 1.5f, 0.5f), doorMesh->rotationEulerXYZ);
}

cDoor::~cDoor() {}
void rotateWall(PxRigidStatic* wallActor, const PxVec3& position, float rotationYDegrees);

void cDoor::Interact()
{
    //std::cout << "The door opens!\n";
    //pInteractableMesh->positionXYZ.y += 5.0f;  // Example: move door up
    //std::cout << "DoorName: " << pInteractableMesh->uniqueFriendlyName << std::endl;
    if (ShouldOpen() /*&& !doOnce*/)
    {
        //doOnce = true;
        DoorAnime();
        g_pSoundManager->PlaySoundEffect("MetalDoorOpen");
    }
}

bool cDoor::ShouldOpen()
{
    for (sMesh* pKeyMesh : pPlayerReference->pickedKeys) {
        if (pKeyMesh->uniqueFriendlyName == "RedKey" && (this->pInteractableMesh->uniqueFriendlyName == "Door10" || this->pInteractableMesh->uniqueFriendlyName == "Door11")) return true;
        if (pKeyMesh->uniqueFriendlyName == "BlueKey" && (this->pInteractableMesh->uniqueFriendlyName == "Door2" || this->pInteractableMesh->uniqueFriendlyName == "Door3")) return true;
        if (pKeyMesh->uniqueFriendlyName == "GreenKey" && (this->pInteractableMesh->uniqueFriendlyName == "Door4" || this->pInteractableMesh->uniqueFriendlyName == "Door5")) return true;
        if (pKeyMesh->uniqueFriendlyName == "YellowKey" && (this->pInteractableMesh->uniqueFriendlyName == "Door8" || this->pInteractableMesh->uniqueFriendlyName == "Door9")) return true;
        if (this->pInteractableMesh->uniqueFriendlyName != "Door10" &&
            this->pInteractableMesh->uniqueFriendlyName != "Door11" &&
            this->pInteractableMesh->uniqueFriendlyName != "Door2" &&
            this->pInteractableMesh->uniqueFriendlyName != "Door3" &&
            this->pInteractableMesh->uniqueFriendlyName != "Door4" &&
            this->pInteractableMesh->uniqueFriendlyName != "Door5" &&
            this->pInteractableMesh->uniqueFriendlyName != "Door8" &&
            this->pInteractableMesh->uniqueFriendlyName != "Door9" ) return true;
    }

    return false;
}

void cDoor::DoorAnime()
{

    float startPosition = pInteractableMesh->rotationEulerXYZ.y ;
    float endPosition ; // Move up 5 units
    if (pInteractableMesh->rotationEulerXYZ.y > 0.0f) {
        endPosition = pInteractableMesh->rotationEulerXYZ.y + 80.0f;
    }
    else {
        endPosition = pInteractableMesh->rotationEulerXYZ.y - 80.0f;
    }

    cTimeline* t = new cTimeline();
    t->SetDuration(1.0f);
    t->SetEaseMode(EaseMode::EaseInOut);
    t->Play();

    t->OnUpdate = [this, startPosition, endPosition](float alpha)
        {
            // Interpolate between start and end using glm::mix
            pInteractableMesh->rotationEulerXYZ.y = glm::mix(startPosition, endPosition, alpha);
        };

    t->OnFinished = []()
        {
            // Optional: finalize or log
            // std::cout << "Door finished opening.\n";
        };

    pTimelineManager->AddTimeline(t);

    rotateWall(DoorPhys, PxVec3(0, this->doorMesh->positionXYZ.y, 0), pInteractableMesh->rotationEulerXYZ.y);
}

void cDoor::rotateWall(PxRigidStatic* wallActor, const PxVec3& position, float rotationYDegrees)
{
    PxQuat rotationQuat(glm::radians(rotationYDegrees), PxVec3(0, 1, 0));
    PxTransform newTransform(position, rotationQuat);
    wallActor->setGlobalPose(newTransform);
}

void cDoor::createStaticWall(const PxVec3& position, const PxVec3& halfExtents, const glm::vec3& rotation)
{
    // Create a material (friction, restitution)
    PxMaterial* material = gMaterial;

    // Define the shape of the wall (box)
    PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtents), *material);

    // Define transform with rotation
    PxTransform transform(position, PxQuat(glm::radians(rotation.y), PxVec3(0, 1, 0)));

    // Create static rigid body (it won’t move)
    DoorPhys = gPhysics->createRigidStatic(transform);
    DoorPhys->attachShape(*shape);

    // Add actor to the scene
    gScene->addActor(*DoorPhys);

    // Release shape (safe to release after attaching)
    shape->release();
}