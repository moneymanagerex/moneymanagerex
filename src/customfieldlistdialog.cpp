/*******************************************************
Copyright (C) 2016 Gabriele-V

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


mmCustomFieldListDialog::mmCustomFieldListDialog (wxWindow* parent, const wxString& RefType) :
    m_RefType(RefType)
    , m_field_id(-1)
    #ifdef _DEBUG
        , debug_(true)
    #else
        , debug_(false)
    #endif
{
    if (debug_) ColName_[FIELD_ID] = _("#");
    ColName_[FIELD_DESCRIPTION] = _("Name");
    ColName_[FIELD_TYPE] = _("Type");
    if (debug_) ColName_[FIELD_PROPERTIES] = _("Properties");

    Create(parent);
}

void mmCustomFieldListDialog::Create(wxWindow* parent)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    long style = wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER;

    wxString WindowTitle = wxString::Format(_("Personalize custom fields | %s"), m_RefType);
    if (!wxDialog::Create(parent, wxID_ANY, WindowTitle, wxDefaultPosition, wxDefaultSize, style))
        return;

    CreateControls();
    fillControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    fillControls();
    SetIcon(mmex::getProgramIcon());
    Centre();
}

void mmCustomFieldListDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxVERTICAL);

    fieldListBox_ = new wxDataViewListCtrl( this
        , wxID_ANY, wxDefaultPosition, wxSize(460, 500)/*, wxDV_HORIZ_RULES*/);

    if (debug_) fieldListBox_->AppendTextColumn(ColName_[FIELD_ID], wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER);
    fieldListBox_->AppendTextColumn(ColName_[FIELD_DESCRIPTION], wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER);
    fieldListBox_->AppendTextColumn(ColName_[FIELD_TYPE], wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER);
    if (debug_) fieldListBox_->AppendTextColumn(ColName_[FIELD_PROPERTIES], wxDATAVIEW_CELL_INERT, wxLIST_AUTOSIZE_USEHEADER);
    mainBoxSizer->Add(fieldListBox_, wxSizerFlags(g_flagsExpand).Border(wxALL, 10));

    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    mainBoxSizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Center());
    wxStdDialogButtonSizer* buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxButton* btnClose = new wxButton(buttons_panel, wxID_OK, wxGetTranslation(g_CloseLabel));
    buttons_sizer->Add(btnClose, g_flagsH);

    wxBitmapButton* magicButton = new wxBitmapButton(buttons_panel, wxID_APPLY, mmBitmap(png::RUN));
    magicButton->SetToolTip(_("Other tools"));
    buttons_sizer->Add(magicButton, g_flagsH);

    Center();
    this->SetSizer(mainBoxSizer);
}

void mmCustomFieldListDialog::fillControls()
{    
    fieldListBox_->DeleteAllItems();

    Model_CustomField::Data_Set fields = Model_CustomField::instance().find(Model_CustomField::DB_Table_CUSTOMFIELD::REFTYPE(m_RefType));
    if (fields.size() == 0) return;

    std::sort(fields.begin(), fields.end(), SorterByDESCRIPTION());
    int firstInTheListID = -1;
    for (const auto &entry : fields)
    {
        if (firstInTheListID == -1) firstInTheListID = entry.FIELDID;
        wxVector<wxVariant> data;
        if (debug_) data.push_back(wxVariant(wxString::Format("%i", entry.FIELDID)));
        data.push_back(wxVariant(entry.DESCRIPTION));
        data.push_back(wxVariant(wxGetTranslation(entry.TYPE)));
        if (debug_)
        {
            wxString Properties = entry.PROPERTIES;
            Properties.Replace("\n", "", true);
            data.push_back(wxVariant(Properties));
        }
        fieldListBox_->AppendItem(data, (wxUIntPtr)entry.FIELDID);
    }

    m_field_id = firstInTheListID;
}

void mmCustomFieldListDialog::OnListItemSelected(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();
    int selected_index = fieldListBox_->ItemToRow(item);

    if (selected_index >= 0)
        m_field_id = (int)fieldListBox_->GetItemData(item);
    else
        m_field_id = -1;
}

void mmCustomFieldListDialog::AddField()
{
    mmCustomFieldEditDialog dlg(this, (Model_CustomField::Data*)nullptr, m_RefType);
    if (dlg.ShowModal() != wxID_OK)
        return;
    fillControls();
}

void mmCustomFieldListDialog::EditField()
{
    Model_CustomField::Data *field = Model_CustomField::instance().get(m_field_id);
    if (field)
    {
        mmCustomFieldEditDialog dlg(this, field, m_RefType);
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
        wxString::Format(_("This function will massive search & replace for \"%s\" custom field values\n"
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

    wxMessageBox(wxString::Format(_("%i occurrences founded and replaced!"), (int)data.size())
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
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_UPDATE_FIELD, _("&Massive content update ")));
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
