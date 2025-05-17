#pragma once

#include "cMasterInteractable.h"

class cHealth : public cMasterInteractable
{
public:
    cHealth();
    virtual ~cHealth();

    virtual void Interact()  override;  // Override Interact function

    bool bIsPickedKey;
};
