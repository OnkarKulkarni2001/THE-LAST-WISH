#pragma once

#include "cMasterInteractable.h"

class cAmmo : public cMasterInteractable
{
public:
    cAmmo();
    virtual ~cAmmo();

    virtual void Interact()  override;  // Override Interact function

    void AddLight();
    void RemoveLight();
};
