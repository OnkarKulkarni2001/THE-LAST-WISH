#pragma once

#include "sMesh.h"
#include "cPlayer.h"
#include "PhysXWraper/cPhysXWraper.h"
#include "include/PhysX/PxPhysicsAPI.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
using namespace physx;

class cMasterInteractable
{
public:
    cMasterInteractable();
    virtual ~cMasterInteractable();

    // Components
    sMesh* pInteractableMesh;
    sMesh* pInteractableTrigger;

    // Core Functions
    virtual void InteractableBeginPlay(sMesh* pInteractableMesh);
    virtual void InteractableUpdate(double deltaTime);

    virtual void ToggleHighlight(bool bHighlight);
    virtual void Interact();  // This should be overridden in child classes

    // Helper Functions
    void TriggerBeginOverlap();
    void TriggerEndOverlap();
    bool CheckTrigger();

protected:
    bool bIsPlayerOverlapping = false;
};
