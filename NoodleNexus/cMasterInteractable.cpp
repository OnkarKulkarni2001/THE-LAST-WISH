#include "cMasterInteractable.h"
#include <iostream>
#include "cSoundManager.h"

extern cPlayer* pPlayerReference;
extern std::vector<sMesh*> g_vecMeshesToDraw;
extern cSoundManager* g_pSoundManager;

glm::vec3 WorldToScreen(const glm::vec3& worldPos, const glm::mat4& view, const glm::mat4& proj, int screenWidth, int screenHeight) {
    glm::vec4 clipSpace = proj * view * glm::vec4(worldPos, 1.0f);
    if (clipSpace.w <= 0) return glm::vec3(-1.0f); // Behind camera

    glm::vec3 ndc = glm::vec3(clipSpace) / clipSpace.w;
    glm::vec3 screenPos;
    screenPos.x = (ndc.x * 0.5f + 0.5f) * screenWidth;
    screenPos.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * screenHeight;
    screenPos.z = ndc.z;
    return screenPos;
}


cMasterInteractable::cMasterInteractable() {}

cMasterInteractable::~cMasterInteractable() {}

void cMasterInteractable::InteractableBeginPlay(sMesh* InteractableMesh)
{
    pInteractableMesh = InteractableMesh;

    // Spawn interactable Sphere
    pInteractableTrigger = new sMesh();
    pInteractableTrigger->modelFileName = "assets/models/Sphere_radius_1_xyz_N_uv.ply";
    pInteractableTrigger->positionXYZ = InteractableMesh->positionXYZ;
    pInteractableTrigger->rotationEulerXYZ = glm::vec3(0.0f);
    pInteractableTrigger->bDoNotLight = true;
    pInteractableTrigger->uniformScale = 2.0f;
    pInteractableTrigger->bIsWireframe = true;
    pInteractableTrigger->bIsVisible = false;

    g_vecMeshesToDraw.push_back(pInteractableTrigger);
}

void cMasterInteractable::InteractableUpdate(double deltaTime)
{
    if (CheckTrigger())
    {
        //std::cout << "Overlapped\n";
    }
}

void cMasterInteractable::ToggleHighlight(bool bHighlight)
{
    if (bHighlight)
    {
        //std::cout << "Highlight On\n";
    } 
    else
    {
        //std::cout << "Highlight Off\n";
    }
        
}

void cMasterInteractable::Interact()
{
    //std::cout << "Default Interaction\n";
}

void cMasterInteractable::TriggerBeginOverlap()
{
    //::cout << "Player is Here\n";
    pPlayerReference->AddInteraction(this);
}

void cMasterInteractable::TriggerEndOverlap()
{
    //std::cout << "Player is Out\n";
    pPlayerReference->RemoveInteraction(this);
}

bool cMasterInteractable::CheckTrigger()
{
    glm::vec3 playerPosition = pPlayerReference->pPlayerSkeletalMesh->positionXYZ;
    glm::vec3 triggerPosition = pInteractableTrigger->positionXYZ;

    float distance = glm::length(playerPosition - triggerPosition);
    float radius = pInteractableTrigger->uniformScale;
    bool isOverlapping = distance <= radius;

    if (isOverlapping && !bIsPlayerOverlapping)
    {
        TriggerBeginOverlap();
        bIsPlayerOverlapping = true;
    }
    else if (!isOverlapping && bIsPlayerOverlapping)
    {
        TriggerEndOverlap();
        bIsPlayerOverlapping = false;
    }

    return isOverlapping;
}
