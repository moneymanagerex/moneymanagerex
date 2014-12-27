#pragma once

#include "defs.h"

class mmOptionSettingsBase : public wxPanel
{
public:
    virtual void SaveSettings() = 0;
};
