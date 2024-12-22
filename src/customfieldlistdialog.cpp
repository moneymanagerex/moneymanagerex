/*******************************************************
Copyright (C) 2016 Gabriele-V
Copyright (C) 2016, 2020 - 2022 Nikolay Akimov

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

#include "customfieldlistdialog.h"
#include "customfieldeditdialog.h"
#include "constants.h"
#include "images_list.h"
#include "mmSimpleDialogs.h"
#include "paths.h"
#include "util.h"
#include "Model_CustomFieldData.h"

#include <wx/dataview.h>
#include <wx/mimetype.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmCustomFieldListDialog, wxDialog);

wxBEGIN_EVENT_TABLE( mmCustomFieldListDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmCustomFieldListDialog::OnClose)
    EVT_BUTTON(wxID_APPLY, mmCustomFieldListDialog::OnMagicButton)
    EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, mmCustomFieldListDialog::OnListItemSelected)
    EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, mmCustomFieldListDialog::OnItemRightClick)
    EVT_MENU_RANGE(MENU_NEW_FIELD, MENU_UPDATE_FIELD, mmCustomFieldListDialog::OnMenuSelected)
    EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, mmCustomFieldListDialog::OnListItemActivated)
wxEND_EVENT_TABLE()


mmCustomFieldListDialog::mmCustomFieldListDialog (wxWindow* parent)
{
    if (debug_) ColName_[FIELD_ID] = _("#");
    ColName_[FIELD_REF] = _("Attribute of");
    ColName_[FIELD_NAME] = _("Name");
    ColName_[FIELD_TYPE] = _("Type");
    ColName_[FIELD_PROPERTIES] = _("Properties");

    this->SetFont(parent->GetFont());
    Create(parent);
}

void mmCustomFieldListDialog::Create(wxWindow* parent)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    long style = wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER;

    wxString WindowTitle = wxString::Format(_("Custom Field Manager"));
    if (!wxDialog::Create(parent, wxID_ANY, WindowTitle, wxDefaultPosition, wxDefaultSize, style))
        return;
    CreateControls();
    fillControls();
    SetIcon(mmex::getProgramIcon());
    Fit();
    Centre();
}

void mmCustomFieldListDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxVERTICAL);

    fieldListBox_ = new wxDataViewListCtrl( this, wxID_ANY);
    fieldListBox_->SetMinSize(wxSize(512, 256));

    if (debug_) fieldListBox_->AppendTextColumn(ColName_[FIELD_ID], wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER);
    fieldListBox_->AppendTextColumn(ColName_[FIELD_REF], wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER);
    fieldListBox_->AppendTextColumn(ColName_[FIELD_NAME], wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER);
    fieldListBox_->AppendTextColumn(ColName_[FIELD_TYPE], wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER);
    fieldListBox_->AppendTextColumn(ColName_[FIELD_PROPERTIES], wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER);
    mainBoxSizer->Add(fieldListBox_, wxSizerFlags(g_flagsExpand).Border(wxALL, 10));

    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    mainBoxSizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Center());
    wxStdDialogButtonSizer* buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxButton* btnClose = new wxButton(buttons_panel, wxID_OK, wxGetTranslation(g_CloseLabel));
    buttons_sizer->Add(btnClose, g_flagsH);

    wxBitmapButton* magicButton = new wxBitmapButton(buttons_panel, wxID_APPLY, mmBitmapBundle(png::MORE_OPTIONS, mmBitmapButtonSize));
    mmToolTip(magicButton, _("Other tools"));
    buttons_sizer->Add(magicButton, g_flagsH);

    this->SetSizer(mainBoxSizer);
}

void mmCustomFieldListDialog::fillControls()
{
    fieldListBox_->DeleteAllItems();

    Model_CustomField::Data_Set fields = Model_CustomField::instance().all();
    if (fields.empty()) return;

    std::sort(fields.begin(), fields.end(), SorterByDESCRIPTION());
    int64 firstInTheListID = -1;
    for (const auto& entry : fields)
    {
        if (firstInTheListID == -1) firstInTheListID = entry.FIELDID;
        wxVector<wxVariant> data;
        if (debug_) data.push_back(wxVariant(wxString::Format("%lld", entry.FIELDID)));
        data.push_back(wxVariant(wxGetTranslation(entry.REFTYPE)));
        data.push_back(wxVariant(entry.DESCRIPTION));
        data.push_back(wxVariant(wxGetTranslation(entry.TYPE)));

        wxString Properties = entry.PROPERTIES;
        Properties.Replace("\n", "", true);
        data.push_back(wxVariant(Properties));

        fieldListBox_->AppendItem(data, static_cast<wxUIntPtr>(entry.FIELDID.GetValue()));
    }

    m_field_id = firstInTheListID;
}

void mmCustomFieldListDialog::OnListItemSelected(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    int selected_index = fieldListBox_->ItemToRow(item);

    if (selected_index >= 0)
        m_field_id = static_cast<int64>(fieldListBox_->GetItemData(item));
    else
        m_field_id = -1;
}

void mmCustomFieldListDialog::AddField()
{
    mmCustomFieldEditDialog dlg(this, nullptr);
    if (dlg.ShowModal() != wxID_OK)
        return;
    fillControls();
}

void mmCustomFieldListDialog::EditField()
{
    Model_CustomField::Data *field = Model_CustomField::instance().get(m_field_id);
    if (field)
    {
        mmCustomFieldEditDialog dlg(this, field);
        if (dlg.ShowModal() != wxID_OK)
            return;
        fillControls();
    }
}

void mmCustomFieldListDialog::DeleteField()
{
    Model_CustomField::Data *field = Model_CustomField::instance().get(m_field_id);
    if (field)
    {
        int DeleteResponse = wxMessageBox(
            _("Do you really want to delete this custom field and all its data?")
            , _("Confirm Custom Field Deletion")
            , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
        if (DeleteResponse == wxYES)
        {
            Model_CustomField::instance().Delete(m_field_id);
            m_field_id = -1;
            fillControls();
        }
    }
}

void mmCustomFieldListDialog::UpdateField()
{
    Model_CustomField::Data *field = Model_CustomField::instance().get(m_field_id);
    if (!field)
        return;

    int UpdateResponse = wxMessageBox(
        wxString::Format(_("This function will bulk search & replace for \"%s\" custom field values\n"
            "It will match & replace only complete field value, no partial or middle-value replaces allowed\n"
            "Please consider that there isn't any validation!"),field->DESCRIPTION)
        , _("Confirm Custom Field Content Update")
        , wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
    if (UpdateResponse != wxYES)
        return;

    const wxString txtSearch = wxGetTextFromUser(_("Find what"), _("Update Custom Field Content"));
    if (txtSearch == "")
    {
        int Response = wxMessageBox(
            _("Do you want to update blank content?\n"
                "Press no if you want to abort replace procedure!")
            , _("Update Custom Field Content")
            , wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
        if (Response != wxYES)
            return;
    }

    const wxString txtReplace = wxGetTextFromUser(_("Replace with"), _("Update Custom Field Content"));
    if (txtReplace == "")
    {
        int Response = wxMessageBox(
            _("Do you want to update to blank?\n"
                "Press no if you want to abort replace procedure!")
            , _("Update Custom Field Content")
            , wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
        if (Response != wxYES)
            return;
    }

    auto data = Model_CustomFieldData::instance().find(Model_CustomFieldData::FIELDID(m_field_id),
        Model_CustomFieldData::CONTENT(txtSearch));
    for (auto &d : data)
    {
        d.CONTENT = txtReplace;
    }
    Model_CustomFieldData::instance().save(data);

    wxMessageBox(wxString::Format(wxPLURAL("%zu occurrence founded and replaced!", "%zu occurrences founded and replaced!", data.size()), data.size())
        , _("Update Custom Field Content"), wxOK | wxICON_INFORMATION);
}

void mmCustomFieldListDialog::OnMenuSelected(wxCommandEvent& event)
{
    switch(event.GetId())
    {
        case MENU_NEW_FIELD: AddField(); break;
        case MENU_EDIT_FIELD: EditField(); break;
        case MENU_DELETE_FIELD: DeleteField(); break;
        case MENU_UPDATE_FIELD: UpdateField(); break;
        default: break;
    }
}

void mmCustomFieldListDialog::OnMagicButton(wxCommandEvent& WXUNUSED(event))
{
    wxDataViewEvent evt;
    OnItemRightClick(evt);
}

void mmCustomFieldListDialog::OnItemRightClick(wxDataViewEvent& event)
{
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY) ;
    evt.SetEventObject( this );

    Model_CustomField::Data *field = Model_CustomField::instance().get(m_field_id);

    wxMenu* mainMenu = new wxMenu;
    if (field) mainMenu->SetTitle(field->DESCRIPTION);
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_NEW_FIELD, _("&Add ")));
    mainMenu->AppendSeparator();
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_EDIT_FIELD, _("&Edit ")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_DELETE_FIELD, _("&Remove ")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_UPDATE_FIELD, _("&Bulk content update ")));
    if (!field)
    {
        mainMenu->Enable(MENU_EDIT_FIELD, false);
        mainMenu->Enable(MENU_DELETE_FIELD, false);
        mainMenu->Enable(MENU_UPDATE_FIELD, false);
    }

    PopupMenu(mainMenu);
    delete mainMenu;
    event.Skip();
}

void mmCustomFieldListDialog::OnListItemActivated(wxDataViewEvent& WXUNUSED(event))
{
    mmCustomFieldListDialog::EditField();
}

void mmCustomFieldListDialog::OnClose(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_OK);
}
