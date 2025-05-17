#include "cAmmo.h"
#include "cSoundManager.h"
extern cSoundManager* g_pSoundManager;
extern cPlayer* pPlayerReference;

cAmmo::cAmmo()
{
}

cAmmo::~cAmmo()
{
}

void cAmmo::Interact()
{
	pPlayerReference->currentAmmo += 30;
	pInteractableMesh->bIsVisible = false;
	pInteractableTrigger->bIsVisible = false;
	g_pSoundManager->PlaySoundEffect("HealthPickup");
}
