/*******************************************************
Copyright (C) 2014 Stefano Giorgio
Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
********************************************************/

#pragma once

#include "optionsettingsbase.h"

class OptionSettingsAttachment : public OptionSettingsBase
{
    wxDECLARE_EVENT_TABLE();

public:
    OptionSettingsAttachment();

    OptionSettingsAttachment(wxWindow *parent
    , wxWindowID id = wxID_ANY
    , const wxPoint &pos = wxDefaultPosition
    , const wxSize &size = wxDefaultSize
    , long style = wxTAB_TRAVERSAL
    , const wxString &name = "OptionSettingsAttachment");

    ~OptionSettingsAttachment();

public:
    virtual bool SaveSettings();

private:
    void Create();

    void OnAttachmentsButton(wxCommandEvent& WXUNUSED(event));
    void OnAttachmentsPathChanged(wxCommandEvent& WXUNUSED(event));
    void OnAttachmentsSubfolderChanged(wxCommandEvent& event);

private:
    wxComboBox* m_attachments_path = nullptr;
    wxStaticText* m_attachments_preview = nullptr;
    wxCheckBox* m_attachments_subfolder = nullptr;
    wxCheckBox* m_delete_attachments = nullptr;
    wxCheckBox* m_trash_attachments = nullptr;
    wxCheckBox* m_duplicate_attachments = nullptr;
    wxString m_old_path;

    enum
    {
        ID_DIALOG_OPTIONS_TEXTCTRL_ATTACHMENT = wxID_HIGHEST + 10,
        ID_DIALOG_OPTIONS_BUTTON_ATTACHMENTSFOLDER,
        ID_DIALOG_OPTIONS_CHECKBOX_ATTACHMENTSSUBFOLDER,
    };
};
