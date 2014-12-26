#include "mmOptionMiscSettings.h"

/*******************************************************/
BEGIN_EVENT_TABLE(mmOptionMiscSettings, wxPanel)


END_EVENT_TABLE()
/*******************************************************/

mmOptionMiscSettings::mmOptionMiscSettings()
{

}

mmOptionMiscSettings::mmOptionMiscSettings(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    Create(parent, id, pos, size, style, name);
}

mmOptionMiscSettings::~mmOptionMiscSettings()
{

}

void mmOptionMiscSettings::Create(wxWindow *parent
    , wxWindowID id
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
}

void mmOptionMiscSettings::SaveSettings()
{
    wxMessageBox("Misc Settings", "mmOptionSettingsPanel");
}
