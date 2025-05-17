#include "cKey.h"
extern cPlayer* pPlayerReference;

cKey::cKey()
{
	bIsPickedKey = false;
}

cKey::~cKey()
{
}

void cKey::Interact()
{
	pPlayerReference->pickedKeys.push_back(this->pInteractableMesh);
	bIsPickedKey = true;
	std::cout << "Key picked" << std::endl;
	pInteractableMesh->bIsVisible = false;
	pInteractableTrigger->bIsVisible = false;
}
