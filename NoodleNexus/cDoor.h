#pragma once

#include "cMasterInteractable.h"

class cDoor : public cMasterInteractable
{
public:
    cDoor(sMesh* doorMesh);
    virtual ~cDoor();

    virtual void Interact()  override;  // Override Interact function

    PxRigidStatic* DoorPhys;
    sMesh* doorMesh;
    bool ShouldOpen();
    void DoorAnime();
    void rotateWall(PxRigidStatic* wallActor, const PxVec3& position, float rotationYDegrees);
    void createStaticWall(const PxVec3& position, const PxVec3& halfExtents, const glm::vec3& rotation);
};
