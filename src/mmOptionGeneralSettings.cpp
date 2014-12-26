#include "mmOptionGeneralSettings.h"

/*******************************************************/
BEGIN_EVENT_TABLE(mmOptionGeneralSettings, wxPanel)


END_EVENT_TABLE()
/*******************************************************/

mmOptionGeneralSettings::mmOptionGeneralSettings()
{

}

mmOptionGeneralSettings::mmOptionGeneralSettings(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    Create(parent, id, pos, size, style, name);
}

mmOptionGeneralSettings::~mmOptionGeneralSettings()
{

}

void mmOptionGeneralSettings::Create(wxWindow *parent
    , wxWindowID id
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
}

void mmOptionGeneralSettings::SaveSettings()
{
    wxMessageBox("General Settings", "mmOptionSettingsPanel");
}
