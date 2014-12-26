#include "mmOptionViewSettings.h"

/*******************************************************/
BEGIN_EVENT_TABLE(mmOptionViewSettings, wxPanel)


END_EVENT_TABLE()
/*******************************************************/

mmOptionViewSettings::mmOptionViewSettings()
{

}

mmOptionViewSettings::mmOptionViewSettings(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    Create(parent, id, pos, size, style, name);
}

mmOptionViewSettings::~mmOptionViewSettings()
{

}

void mmOptionViewSettings::Create(wxWindow *parent
    , wxWindowID id
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
}

void mmOptionViewSettings::SaveSettings()
{
    wxMessageBox("View Settings", "mmOptionSettingsPanel");
}
