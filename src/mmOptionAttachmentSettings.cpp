#include "mmOptionAttachmentSettings.h"

/*******************************************************/
BEGIN_EVENT_TABLE(mmOptionAttachmentSettings, wxPanel)


END_EVENT_TABLE()
/*******************************************************/

mmOptionAttachmentSettings::mmOptionAttachmentSettings()
{

}

mmOptionAttachmentSettings::mmOptionAttachmentSettings(wxWindow *parent
    , wxWindowID id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
{
    Create(parent, id, pos, size, style, name);
}

mmOptionAttachmentSettings::~mmOptionAttachmentSettings()
{

}

void mmOptionAttachmentSettings::Create(wxWindow *parent
    , wxWindowID id
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString &name)
{
    wxPanel::Create(parent, id, pos, size, style, name);
}

void mmOptionAttachmentSettings::SaveSettings()
{
    wxMessageBox("Attachment Settings", "mmOptionSettingsPanel");
}
