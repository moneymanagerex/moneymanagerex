#pragma once

#include "defs.h"
#include "constants.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"

class mmOptionSettingsBase : public wxPanel
{
public:
    virtual void SaveSettings() = 0;

    wxControl* SetBoldFont(wxControl* control);
    bool GetIniDatabaseCheckboxValue(const wxString& dbField, bool defaultState);
};
