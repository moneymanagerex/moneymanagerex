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

#include "base/defs.h"
#include <wx/dataview.h>
#include <wx/mimetype.h>

#include "base/constants.h"
#include "base/paths.h"
#include "base/images_list.h"
#include "util/_util.h"
#include "util/_simple.h"

#include "model/FieldValueModel.h"
#include "FieldManager.h"
#include "dialog/FieldDialog.h"

wxIMPLEMENT_DYNAMIC_CLASS(FieldManager, wxDialog);

wxBEGIN_EVENT_TABLE( FieldManager, wxDialog )
    EVT_BUTTON(wxID_OK, FieldManager::OnClose)
    EVT_BUTTON(wxID_APPLY, FieldManager::OnMagicButton)
    EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, FieldManager::OnListItemSelected)
    EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, FieldManager::OnItemRightClick)
    EVT_MENU_RANGE(MENU_NEW_FIELD, MENU_UPDATE_FIELD, FieldManager::OnMenuSelected)
    EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, FieldManager::OnListItemActivated)
wxEND_EVENT_TABLE()


FieldManager::FieldManager (wxWindow* parent)
{
    if (debug_) ColName_[FIELD_ID] = "#";
    ColName_[FIELD_REF] = _t("Attribute of");
    ColName_[FIELD_NAME] = _t("Name");
    ColName_[FIELD_TYPE] = _t("Type");
    ColName_[FIELD_PROPERTIES] = _t("Properties");

    this->SetFont(parent->GetFont());
    Create(parent);
    mmThemeAutoColour(this);
}

void FieldManager::Create(wxWindow* parent)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    long style = wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER;

    wxString WindowTitle = wxString::Format(_t("Custom Field Manager"));
    if (!wxDialog::Create(parent, wxID_ANY, WindowTitle, wxDefaultPosition, wxDefaultSize, style))
        return;
    CreateControls();
    fillControls();
    SetIcon(mmex::getProgramIcon());
    Fit();
    Centre();
}

void FieldManager::CreateControls()
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
    mmToolTip(magicButton, _t("Other tools"));
    buttons_sizer->Add(magicButton, g_flagsH);

    this->SetSizer(mainBoxSizer);
}

void FieldManager::fillControls()
{
    fieldListBox_->DeleteAllItems();

    FieldModel::Data_Set fields = FieldModel::instance().get_all();
    if (fields.empty()) return;

    std::sort(fields.begin(), fields.end(), FieldTable::SorterByDESCRIPTION());
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

void FieldManager::OnListItemSelected(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    int selected_index = fieldListBox_->ItemToRow(item);

    if (selected_index >= 0)
        m_field_id = static_cast<int64>(fieldListBox_->GetItemData(item));
    else
        m_field_id = -1;
}

void FieldManager::AddField()
{
    FieldDialog dlg(this, nullptr);
    if (dlg.ShowModal() != wxID_OK)
        return;
    fillControls();
}

void FieldManager::EditField()
{
    FieldModel::Data *field = FieldModel::instance().cache_id(m_field_id);
    if (field)
    {
        FieldDialog dlg(this, field);
        if (dlg.ShowModal() != wxID_OK)
            return;
        fillControls();
    }
}

void FieldManager::DeleteField()
{
    FieldModel::Data *field = FieldModel::instance().cache_id(m_field_id);
    if (field)
    {
        int DeleteResponse = wxMessageBox(
            _t("Do you want to delete the custom field and all its data?")
            , _t("Confirm Custom Field Deletion")
            , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
        if (DeleteResponse == wxYES)
        {
            FieldModel::instance().Delete(m_field_id);
            m_field_id = -1;
            fillControls();
        }
    }
}

void FieldManager::UpdateField()
{
    FieldModel::Data *field = FieldModel::instance().cache_id(m_field_id);
    if (!field)
        return;

    int UpdateResponse = wxMessageBox(
        wxString::Format(_t("This function will bulk search & replace for \"%s\" custom field values\n"
            "It will match & replace only complete field value, no partial or middle-value replaces allowed\n"
            "Please consider that there isn't any validation!"),field->DESCRIPTION)
        , _t("Confirm Custom Field Content Update")
        , wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
    if (UpdateResponse != wxYES)
        return;

    const wxString txtSearch = wxGetTextFromUser(_t("Find what"), _t("Update Custom Field Content"));
    if (txtSearch == "")
    {
        int Response = wxMessageBox(
            _t("Do you want to update blank custom field content?\n"
                "Select No if you want to abort the replace procedure.")
            , _t("Update Custom Field Content")
            , wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
        if (Response != wxYES)
            return;
    }

    const wxString txtReplace = wxGetTextFromUser(_t("Replace with"), _t("Update Custom Field Content"));
    if (txtReplace == "")
    {
        int Response = wxMessageBox(
            _t("Do you want to update blank custom field content?\n"
                "Select No if you want to abort the replace procedure.")
            , _t("Update Custom Field Content")
            , wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
        if (Response != wxYES)
            return;
    }

    auto data = FieldValueModel::instance().find(FieldValueModel::FIELDID(m_field_id),
        FieldValueModel::CONTENT(txtSearch));
    for (auto &d : data)
    {
        d.CONTENT = txtReplace;
    }
    FieldValueModel::instance().save(data);

    wxMessageBox(wxString::Format(wxPLURAL("%zu occurrence founded and replaced!", "%zu occurrences founded and replaced!", data.size()), data.size())
        , _t("Update Custom Field Content"), wxOK | wxICON_INFORMATION);
}

void FieldManager::OnMenuSelected(wxCommandEvent& event)
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

void FieldManager::OnMagicButton(wxCommandEvent& WXUNUSED(event))
{
    wxDataViewEvent evt;
    OnItemRightClick(evt);
}

void FieldManager::OnItemRightClick(wxDataViewEvent& event)
{
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY) ;
    evt.SetEventObject( this );

    FieldModel::Data *field = FieldModel::instance().cache_id(m_field_id);

    wxMenu* mainMenu = new wxMenu;
    if (field) mainMenu->SetTitle(field->DESCRIPTION);
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_NEW_FIELD, _t("&Add ")));
    mainMenu->AppendSeparator();
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_EDIT_FIELD, _t("&Edit ")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_DELETE_FIELD, _t("&Remove ")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_UPDATE_FIELD, _t("&Bulk content update ")));
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

void FieldManager::OnListItemActivated(wxDataViewEvent& WXUNUSED(event))
{
    FieldManager::EditField();
}

void FieldManager::OnClose(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_OK);
}
