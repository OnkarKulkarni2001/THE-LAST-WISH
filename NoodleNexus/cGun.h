#pragma once

#include "cMasterInteractable.h"

class cGun : public cMasterInteractable
{
public:
    cGun();
    virtual ~cGun();

    virtual void Interact()  override;  // Override Interact function

};
