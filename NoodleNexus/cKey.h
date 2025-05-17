#pragma once

#include "cMasterInteractable.h"

class cKey : public cMasterInteractable
{
public:
    cKey();
    virtual ~cKey();

    virtual void Interact()  override;  // Override Interact function

    bool bIsPickedKey;
};
