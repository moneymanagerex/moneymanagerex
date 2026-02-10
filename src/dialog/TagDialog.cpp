/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 Gabriele-V
 Copyright (C) 2022  Mark Whalley (mark@ipx.co.uk)

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

#include "constants.h"
#include "util/util.h"
#include "paths.h"

#include "model/InfotableModel.h"
#include "model/TagLinkModel.h"
#include "model/TagModel.h"
#include "model/TransactionModel.h"
#include "model/TransactionSplitModel.h"

#include "TagDialog.h"

wxIMPLEMENT_DYNAMIC_CLASS(TagDialog, wxDialog);

wxBEGIN_EVENT_TABLE(TagDialog, wxDialog)
EVT_BUTTON(wxID_ADD, TagDialog::OnAdd)
EVT_BUTTON(wxID_EDIT, TagDialog::OnEdit)
EVT_BUTTON(wxID_REMOVE, TagDialog::OnDelete)
EVT_TEXT(wxID_FIND, TagDialog::OnTextChanged)
EVT_LISTBOX(wxID_VIEW_LIST, TagDialog::OnListSelChanged)
EVT_CHECKLISTBOX(wxID_VIEW_LIST, TagDialog::OnCheckboxSelChanged)
EVT_BUTTON(wxID_OK, TagDialog::OnOk)
EVT_BUTTON(wxID_CANCEL, TagDialog::OnCancel)
wxEND_EVENT_TABLE()

TagDialog::TagDialog() : isSelection_(false)
{
}

TagDialog::~TagDialog()
{
    InfotableModel::instance().setSize("TAG_DIALOG_SIZE", GetSize());
}

TagDialog::TagDialog(wxWindow* parent, bool isSelection, const wxArrayString& selectedTags) : isSelection_(isSelection), selectedTags_(selectedTags)
{
    this->SetFont(parent->GetFont());
    Create(parent);
    if (isSelection_)
    {
        for (const auto& tag : selectedTags)
        {
            int index = tagListBox_->FindString(tag);
            if (index != wxNOT_FOUND)
                dynamic_cast<wxCheckListBox*>(tagListBox_)->Check(index);
        }
    }

    SetMinSize(wxSize(300, 420));
    Fit();
}

bool TagDialog::Create(wxWindow* parent, wxWindowID id
    , const wxString& caption, const wxString& name
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    SetEvtHandlerEnabled(false);

    CreateControls();
    mmThemeAutoColour(this);
    SetEvtHandlerEnabled(true);

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmex::getProgramIcon());

    Fit();
    mmSetSize(this);
    Centre();
    return true;
}

void TagDialog::CreateControls()
{
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(boxSizer);

    //--------------------------
    for (const auto& tag : TagModel::instance().all(DB_Table_TAG_V1::COL_TAGNAME))
        tagList_.Add(tag.TAGNAME);

    if (!isSelection_)
        tagListBox_ = new wxListBox(this, wxID_VIEW_LIST, wxDefaultPosition, wxDefaultSize, tagList_, wxLB_EXTENDED | wxLB_SORT);
    else
        tagListBox_ = new wxCheckListBox(this, wxID_VIEW_LIST, wxDefaultPosition, wxDefaultSize, tagList_, wxLB_EXTENDED | wxLB_SORT);

    boxSizer->Add(tagListBox_, g_flagsExpand);

    wxPanel* searchPanel = new wxPanel(this, wxID_ANY);
    boxSizer->Add(searchPanel, wxSizerFlags(g_flagsExpand).Proportion(0));
    wxBoxSizer* search_sizer = new wxBoxSizer(wxHORIZONTAL);
    searchPanel->SetSizer(search_sizer);

    searchCtrl_ = new wxSearchCtrl(searchPanel, wxID_FIND);
    search_sizer->Add(new wxStaticText(searchPanel, wxID_STATIC, _t("Search")), g_flagsH);
    search_sizer->Add(searchCtrl_, g_flagsExpand);

    wxPanel* buttonsPanel = new wxPanel(this, wxID_ANY);
    boxSizer->Add(buttonsPanel, wxSizerFlags(g_flagsV).Center());
    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxVERTICAL);
    buttonsPanel->SetSizer(buttonsSizer);

    wxStdDialogButtonSizer* editButtonSizer = new wxStdDialogButtonSizer;
    buttonsSizer->Add(editButtonSizer, wxSizerFlags(g_flagsV).Border(wxALL, 0).Center());

    buttonAdd_ = new wxButton(buttonsPanel, wxID_ADD, _t("&Add "));
    editButtonSizer->Add(buttonAdd_, g_flagsH);
    mmToolTip(buttonAdd_, _t("Add a new tag"));

    buttonEdit_ = new wxButton(buttonsPanel, wxID_EDIT, _t("&Edit "));
    editButtonSizer->Add(buttonEdit_, g_flagsH);
    buttonEdit_->Enable(false);
    mmToolTip(buttonEdit_, _t("Edit the name of an existing tag"));

    buttonDelete_ = new wxButton(buttonsPanel, wxID_REMOVE, _t("&Delete "));
    editButtonSizer->Add(buttonDelete_, g_flagsH);
    buttonDelete_->Enable(false);
    mmToolTip(buttonDelete_, _t("Delete an existing tag. The tag is unable to be used by existing transactions."));

    //--------------------------
    wxStdDialogButtonSizer* dlgButtonSizer = new wxStdDialogButtonSizer();
    boxSizer->Add(dlgButtonSizer, wxSizerFlags(g_flagsV).Centre());

    wxButton* itemButton24 = new wxButton(this, wxID_OK, (isSelection_ ? _t("Select") : _t("&OK ")));
    dlgButtonSizer->Add(itemButton24, g_flagsH);

    wxButton* itemButton25 = new wxButton(this, wxID_CANCEL, wxGetTranslation(isSelection_ ? g_CancelLabel : g_CloseLabel));
    dlgButtonSizer->Add(itemButton25, g_flagsH);
}

void TagDialog::fillControls()
{
    Freeze();
    wxArrayString filteredList;
    for (const auto& tag : tagList_)
        if (tag.Lower().Matches(mask_string_ + "*"))
            filteredList.Add(tag);

    tagListBox_->Set(filteredList);

    if (isSelection_)
    {
        // reselect previously selected items
        for (const auto& tag : selectedTags_)
        {
            int index = tagListBox_->FindString(tag);
            if (index != wxNOT_FOUND)
                dynamic_cast<wxCheckListBox*>(tagListBox_)->Check(index);
        }
    }

    buttonEdit_->Disable();
    buttonDelete_->Disable();
    Thaw();
}

bool TagDialog::validateName(const wxString& name)
{
    if (name == "&" || name == "|")
    {
        wxString errMsg = _t("Invalid tag name");
        errMsg << "\n\n" << _t("Tag names may not be the '&' or '|' characters because these are reserved for filter operators");
        wxMessageBox(errMsg, _t("Tag Manager: Invalid Name"), wxOK | wxICON_ERROR);
        return false;
    } else if (name.Find(' ') != wxNOT_FOUND)
    {
        wxString errMsg = _t("Name contains tag delimiter.");
        errMsg << "\n\n" << _t("Tag names may not contain the space (' ') character");
        wxMessageBox(errMsg, _t("Tag Manager: Invalid Name"), wxOK | wxICON_ERROR);
        return false;
    }
    return true;
    
}

void TagDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_OK);
}

void TagDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void TagDialog::OnAdd(wxCommandEvent& WXUNUSED(event))
{
    wxString prompt_msg = _t("Enter the name for the new tag:");
    wxString text = wxGetTextFromUser(prompt_msg, _t("Add Tag"), "");

    while (!validateName(text))
        text = wxGetTextFromUser(prompt_msg, _t("Add Tag"), text);

    if (text.IsEmpty())
        return;

    const auto& tags = TagModel::instance().find(TagModel::TAGNAME(text));
    if (!tags.empty())
    {
        wxMessageBox(_t("A tag with this name already exists"), _t("Tag Manager: Adding Error"), wxOK | wxICON_ERROR);
        return;
    }

    TagModel::Data* tag = TagModel::instance().create();
    tag->TAGNAME = text;
    tag->ACTIVE = 1;
    TagModel::instance().save(tag);
    refreshRequested_ = true;
    tagList_.Add(text);
    fillControls();
    setSelectedString(text);
}

void TagDialog::OnEdit(wxCommandEvent& WXUNUSED(event))
{
    wxArrayInt selections;
    wxString old_name;
    
    tagListBox_->GetSelections(selections);
    if (selections.IsEmpty()) return;
    old_name = tagListBox_->GetString(selections[0]); 

    const wxString msg = wxString::Format(_t("Enter a new name for '%s'"), old_name);
    wxString text = wxGetTextFromUser(msg, _t("Edit Tag"), old_name);

    while (!validateName(text))
        text = wxGetTextFromUser(msg, _t("Edit Tag"), text);

    if (text.IsEmpty() || old_name == text)
        return;

    TagModel::Data* tag = TagModel::instance().get(text);
    if (tag)
    {
        wxString errMsg = _t("A tag with this name already exists");
        wxMessageBox(errMsg, _t("Tag Manager: Editing Error"), wxOK | wxICON_ERROR);
        return;
    }

    tag = TagModel::instance().get(old_name);
    tag->TAGNAME = text;
    TagModel::instance().save(tag);
    tagList_.Remove(old_name);
    tagList_.Add(text);
    int index = selectedTags_.Index(old_name);
    if (index != wxNOT_FOUND)
    {
        selectedTags_.RemoveAt(index);
        selectedTags_.Add(text);
    }
    refreshRequested_ = true;
    fillControls();
    setSelectedString(text);
}

void TagDialog::OnDelete(wxCommandEvent& WXUNUSED(event))
{
    wxArrayInt selections;
    wxArrayString stringSelections;
    tagListBox_->GetSelections(selections);
    for (const auto& selection : selections)
        stringSelections.Add(tagListBox_->GetString(selection));

    if (stringSelections.IsEmpty())
        return;

    TagModel::instance().Savepoint();
    TagLinkModel::instance().Savepoint();
    TransactionModel::instance().Savepoint();
    TransactionSplitModel::instance().Savepoint();
    for (const auto& selection : stringSelections)
    {
        TagModel::Data* tag = TagModel::instance().get(selection);
        int tag_used = TagModel::instance().is_used(tag->TAGID);
        if (tag_used == 1)
        {
            wxMessageBox(wxString::Format(_t("Tag '%s' in use"), tag->TAGNAME), _t("Tag Manager: Delete Error"), wxOK | wxICON_ERROR);
            continue;
        }
        wxMessageDialog msgDlg(this, wxString::Format(_t("Deleted transactions exist which use tag '%s'."), tag->TAGNAME)
                + "\n\n" + _t("Deleting the tag will also automatically purge the associated deleted transactions.")
                + "\n\n" + _t("Do you want to continue?")
                , _t("Confirm Tag Deletion"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
        
        if (tag_used == 0 || (tag_used == -1 && msgDlg.ShowModal() == wxID_YES))
        {
            TagLinkModel::Data_Set taglinks = TagLinkModel::instance().find(TagLinkModel::TAGID(tag->TAGID));
            for (const auto& link : taglinks)
                // Taglinks for deleted transactions are either TRANSACTION or TRANSACTIONSPLIT type.
                // Remove the transactions which will delete all associated tags.
                if (link.REFTYPE == TransactionModel::refTypeName)
                    TransactionModel::instance().remove(link.REFID);
                else if (link.REFTYPE == TransactionSplitModel::refTypeName)
                    TransactionModel::instance().remove(TransactionSplitModel::instance().get(link.REFID)->TRANSID);
            TagModel::instance().remove(tag->TAGID);
            tagList_.Remove(selection);
            int index = selectedTags_.Index(selection);
            if (index != wxNOT_FOUND)
                selectedTags_.RemoveAt(index);
        }
    }
    TagModel::instance().ReleaseSavepoint();
    TagLinkModel::instance().ReleaseSavepoint();
    TransactionModel::instance().ReleaseSavepoint();
    TransactionSplitModel::instance().ReleaseSavepoint();
    refreshRequested_ = true;
    fillControls();
    int newIndex = std::min(selections[0], static_cast<int>(tagListBox_->GetCount()) - 1);
    if (newIndex >= 0)
        setSelectedItem(newIndex);
}

void TagDialog::OnTextChanged(wxCommandEvent& event)
{
    mask_string_ = event.GetString();
    if (!mask_string_.IsEmpty())
        mask_string_ = mask_string_.Lower().Prepend("*");
    fillControls();
    searchCtrl_->SetFocus();
    searchCtrl_->SetInsertionPointEnd();
}

void TagDialog::OnListSelChanged(wxCommandEvent& WXUNUSED(event))
{
    buttonEdit_->Enable(false);
    buttonDelete_->Enable(false);

    wxArrayInt selections;
    wxArrayString stringSelections;

    tagListBox_->GetSelections(selections);
    for (const auto& selection : selections)
        stringSelections.Add(tagListBox_->GetString(selection));

    int count = selections.GetCount();

    // Can only edit one tag at a time
    if (count == 1)
    {
        buttonEdit_->Enable();
    }
    // Can delete multiple tags at once as long as all are unused
    if (count > 0)
    {
        bool is_used = false;
        for (const auto& selection : stringSelections)
        {
            TagModel::Data* tag = TagModel::instance().get(selection);
            is_used |= TagModel::instance().is_used(tag->TAGID) == 1;
        }
        buttonDelete_->Enable(!is_used);
    }    
}

void TagDialog::OnCheckboxSelChanged(wxCommandEvent& event)
{
    if (dynamic_cast<wxCheckListBox*>(tagListBox_)->IsChecked(event.GetSelection()))
        selectedTags_.Add(event.GetString());
    else
    {
        int index = selectedTags_.Index(event.GetString());
        if (index != wxNOT_FOUND)
            selectedTags_.RemoveAt(index);
    }
}

void TagDialog::setSelectedItem(int index)
{
    tagListBox_->EnsureVisible(index);
    tagListBox_->SetSelection(index);

    wxCommandEvent evt;
    OnListSelChanged(evt);
}

void TagDialog::setSelectedString(const wxString& tagname)
{
    int index = tagListBox_->FindString(tagname);

    if (index != wxNOT_FOUND)
        setSelectedItem(index);
}
