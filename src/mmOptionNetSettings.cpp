#include "mmOptionNetSettings.h"

/*******************************************************/
BEGIN_EVENT_TABLE(mmOptionNetSettings, wxPanel)


END_EVENT_TABLE()
/*******************************************************/

mmOptionNetSettings::mmOptionNetSettings()
{

}

mmOptionNetSettings::mmOptionNetSettings(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    Create(parent, id, pos, size, style, name);
}

mmOptionNetSettings::~mmOptionNetSettings()
{

}

void mmOptionNetSettings::Create(wxWindow *parent
    , wxWindowID id
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
}

void mmOptionNetSettings::SaveSettings()
{
    wxMessageBox("Network Settings", "mmOptionSettingsPanel");
}
