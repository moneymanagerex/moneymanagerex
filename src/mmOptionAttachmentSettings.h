#pragma once

#include "mmOptionBaseSettings.h"

class mmOptionAttachmentSettings : public mmOptionSettingsBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmOptionAttachmentSettings();

    mmOptionAttachmentSettings(wxWindow *parent
    , wxWindowID id = wxID_ANY
    , const wxPoint &pos = wxDefaultPosition
    , const wxSize &size = wxDefaultSize
    , long style = wxTAB_TRAVERSAL
    , const wxString &name = wxPanelNameStr);

    ~mmOptionAttachmentSettings();

public:
    virtual void SaveSettings();

private:
    void Create(wxWindow *parent
        , wxWindowID id
        , const wxPoint& pos
        , const wxSize& size
        , long style
        , const wxString &name);

    void OnAttachmentsButton(wxCommandEvent& event);
    void OnAttachmentsMenu(wxCommandEvent& event);
    void OnAttachmentsPathChanged(wxCommandEvent& event);
    void OnAttachmentsSubfolderChanged(wxCommandEvent& event);

private:
    wxCheckBox* m_attachments_subfolder;
    wxCheckBox* m_delete_attachments;
    wxCheckBox* m_trash_attachments;

    enum
    {
        ID_DIALOG_OPTIONS_TEXTCTRL_ATTACHMENT = wxID_HIGHEST + 10,
        ID_DIALOG_OPTIONS_BUTTON_ATTACHMENTSFOLDER,
        ID_DIALOG_OPTIONS_STATICTEXT_ATTACHMENTSTEXT,
        ID_DIALOG_OPTIONS_CHECKBOX_ATTACHMENTSSUBFOLDER,
    };
};
