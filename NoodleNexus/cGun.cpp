#include "cGun.h"
#include <iostream>

extern cPlayer* pPlayerReference;

cGun::cGun()
{

}

cGun::~cGun()
{

}

void cGun::Interact()
{
	//Attach Gun To Player
	std::cout << "Gun Utha\n";
	pInteractableMesh->bIsVisible = false;
}
