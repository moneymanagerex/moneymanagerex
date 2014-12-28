#include "mmOptionBaseSettings.h"
#include "model/Model_Setting.h"

wxControl* mmOptionSettingsBase::SetBoldFont(wxControl* control)
{
    wxFont control_font = control->GetFont();
    control_font.SetWeight(wxFONTWEIGHT_BOLD);
    control->SetFont(control_font);

    return control;
}

bool mmOptionSettingsBase::GetIniDatabaseCheckboxValue(wxString dbField, bool defaultState)
{
    bool result = Model_Setting::instance().GetBoolSetting(dbField, defaultState);
    return result;
}
