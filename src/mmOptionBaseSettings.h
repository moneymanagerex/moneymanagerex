#pragma once

#include "defs.h"

class mmOptionSettingsBase : public wxPanel
{
public:
    virtual void SaveSettings() = 0;

    wxControl* SetBoldFont(wxControl* control);
    bool GetIniDatabaseCheckboxValue(wxString dbField, bool defaultState);

};
