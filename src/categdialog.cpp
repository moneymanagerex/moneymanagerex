/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#include "relocatecategorydialog.h"
#include "categdialog.h"
#include "util.h"
#include "mmOption.h"
#include "defs.h"
#include "paths.h"
#include "model/Model_Setting.h"
#include "model/Model_Infotable.h"

IMPLEMENT_DYNAMIC_CLASS( mmCategDialog, wxDialog )

BEGIN_EVENT_TABLE( mmCategDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmCategDialog::OnBSelect)
    EVT_BUTTON(wxID_CANCEL, mmCategDialog::OnCancel)
    EVT_BUTTON(wxID_ADD, mmCategDialog::OnAdd)
    EVT_BUTTON(wxID_REMOVE, mmCategDialog::OnDelete)
    EVT_BUTTON(wxID_EDIT, mmCategDialog::OnEdit)
    EVT_TREE_SEL_CHANGED(wxID_ANY, mmCategDialog::OnSelChanged)
    EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY, mmCategDialog::OnSelChanged)
    EVT_TREE_ITEM_ACTIVATED(wxID_ANY,  mmCategDialog::OnDoubleClicked)
    EVT_TREE_ITEM_MENU(wxID_ANY, mmCategDialog::OnItemRightClick)
    EVT_MENU_RANGE(0, 9, mmCategDialog::OnMenuSelected)
END_EVENT_TABLE()

mmCategDialog::mmCategDialog( )
{
    // Initialize fields in constructor
    categID_ = -1;
    subcategID_ = -1;
    selectedItemId_ = 0;
}

mmCategDialog::mmCategDialog(mmCoreDB* core,
                             wxWindow* parent, bool bEnableSelect, bool bEnableRelocate,
                             wxWindowID id, const wxString& caption,
                             const wxPoint& pos, const wxSize& size, long style )
{
    // Initialize fields in constructor
    core_ = core;
    categID_ = -1;
    subcategID_ = -1;
    selectedItemId_ = 0;
    bEnableSelect_ = bEnableSelect;
    bEnableRelocate_ = bEnableRelocate;

    //Get Hidden Categories id from stored string
    hidden_categs_.clear();
    wxString sSettings = Model_Infotable::instance().GetStringInfo("HIDDEN_CATEGS_ID", "");
    wxStringTokenizer token(sSettings, ";");
    while (token.HasMoreTokens())
    {
        hidden_categs_.Add( token.GetNextToken() );
    }
    //

    Create(parent, id, caption, pos, size, style);
}

bool mmCategDialog::Create( wxWindow* parent, wxWindowID id,
                           const wxString& caption, const wxPoint& pos,
                           const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());
    fillControls();

    Centre();
    return TRUE;
}

void mmCategDialog::fillControls()
{
    if (!core_) return;

    treeCtrl_->DeleteAllItems();
    root_ = treeCtrl_->AddRoot(_("Categories"));
    selectedItemId_ = root_;
    treeCtrl_->SetItemBold(root_, true);
    treeCtrl_->SetFocus ();
    NormalColor_ = treeCtrl_->GetItemTextColour(root_);
    bool bResult = Model_Setting::instance().GetBoolSetting("SHOW_HIDDEN_CATEGS", true);
    cbShowAll_->SetValue(bResult);

    for (const auto& category: core_->categoryList_.entries_)
    {
        wxTreeItemId maincat;
        bool bShow = categShowStatus(category->categID_, -1);
        if (cbShowAll_->IsChecked() || bShow)
        {
            maincat = treeCtrl_->AppendItem(root_, category->categName_);
            treeCtrl_->SetItemData(maincat, new mmTreeItemCateg(category->categID_, -1));
            if (!bShow) treeCtrl_->SetItemTextColour(maincat, wxColour("GREY"));

            for (const auto& sub_category: category->children_)
            {
                bShow = categShowStatus(category->categID_, sub_category->categID_);
                if (cbShowAll_->IsChecked() || bShow)
                {
                    wxTreeItemId subcat = treeCtrl_->AppendItem(maincat, sub_category->categName_);
                    treeCtrl_->SetItemData(subcat, new mmTreeItemCateg(category->categID_, sub_category->categID_));
                    if (!bShow) treeCtrl_->SetItemTextColour(subcat, wxColour("GREY"));

                    if (categID_ == category->categID_ && subcategID_ == sub_category->categID_)
                        selectedItemId_ = subcat;
                }
            }
            treeCtrl_->SortChildren(maincat);
        }
    }
    treeCtrl_->Expand(root_);
    bResult = Model_Setting::instance().GetBoolSetting("EXPAND_CATEGS_TREE", false);
    if (bResult) treeCtrl_->ExpandAll();
    cbExpand_->SetValue(bResult);

    treeCtrl_->SortChildren(root_);
    treeCtrl_->SelectItem(selectedItemId_);
    treeCtrl_->EnsureVisible(selectedItemId_);

    textCtrl_->SetValue("");
    selectButton_->Disable();
    deleteButton_->Disable();
    editButton_->Disable();
    addButton_->Enable();
    btnCateg_relocate_->Enable(bEnableRelocate_);
}

void mmCategDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 1);
    wxBoxSizer* itemBoxSizer33 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer33);

    btnCateg_relocate_ = new wxBitmapButton(this,
        wxID_STATIC, wxBitmap(relocate_categories_xpm));
    btnCateg_relocate_->Connect(wxID_STATIC, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(mmCategDialog::OnCategoryRelocation), NULL, this);
    btnCateg_relocate_->SetToolTip(_("Reassign all categories to another category"));

    cbExpand_ = new wxCheckBox(this, wxID_STATIC, _("Expand"), wxDefaultPosition,
        wxDefaultSize, wxCHK_2STATE);
    cbExpand_->Connect(wxID_STATIC, wxEVT_COMMAND_CHECKBOX_CLICKED,
        wxCommandEventHandler(mmCategDialog::OnExpandChbClick), NULL, this);

    cbShowAll_ = new wxCheckBox(this, wxID_SELECTALL, _("Show All"), wxDefaultPosition,
        wxDefaultSize, wxCHK_2STATE);
    cbShowAll_->SetToolTip(_("Show all hidden categories"));
    cbShowAll_->Connect(wxID_SELECTALL, wxEVT_COMMAND_CHECKBOX_CLICKED,
        wxCommandEventHandler(mmCategDialog::OnShowHiddenChbClick), NULL, this);

    itemBoxSizer33->Add(btnCateg_relocate_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);
    itemBoxSizer33->AddSpacer(10);
    itemBoxSizer33->Add(cbExpand_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);
    itemBoxSizer33->AddSpacer(10);
    itemBoxSizer33->Add(cbShowAll_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

#if defined (__WXGTK__) || defined (__WXMAC__)
    treeCtrl_ = new wxTreeCtrl( this, wxID_ANY,
        wxDefaultPosition, wxSize(200, 380));
#else
    treeCtrl_ = new wxTreeCtrl( this, wxID_ANY,
        wxDefaultPosition, wxSize(200, 380), wxTR_SINGLE
        | wxTR_HAS_BUTTONS
        | wxTR_ROW_LINES );
#endif
    itemBoxSizer3->Add(treeCtrl_, 1, wxGROW|wxALL, 1);

    textCtrl_ = new wxTextCtrl( this, wxID_STATIC, "");
    itemBoxSizer2->Add(textCtrl_, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 1);
    textCtrl_->SetToolTip(_("Enter the name of the category to add or edit here"));

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxGROW|wxALL, 5);

    addButton_ = new wxButton( this, wxID_ADD);
    itemBoxSizer5->Add(addButton_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);
     addButton_->SetToolTip(_("Add a new category"));

    editButton_ = new wxButton( this, wxID_EDIT);
    itemBoxSizer5->Add(editButton_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);
    editButton_->SetToolTip(_("Edit the name of an existing category"));

    deleteButton_ = new wxButton( this, wxID_REMOVE);
    itemBoxSizer5->Add(deleteButton_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);
    deleteButton_->SetToolTip(_("Delete an existing category. The category cannot be used by existing transactions."));

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxGROW|wxALL, 5);

    selectButton_ = new wxButton( this, wxID_OK,
        _("&Select"));
    itemBoxSizer9->Add(selectButton_, 1, wxALIGN_CENTER_VERTICAL|wxALL, 1);
    selectButton_->SetToolTip(_("Select the currently selected category as the selected category for the transaction"));

    //Some interfaces has no any close buttons, it may confuse user. Cancel button added
    wxButton* itemCancelButton = new wxButton( this, wxID_CANCEL);
    itemBoxSizer9->Add(itemCancelButton);
}

void mmCategDialog::OnAdd(wxCommandEvent& /*event*/)
{
    if (mmIniOptions::instance().disableCategoryModify_)
        return;

    wxString text = wxGetTextFromUser(_("Enter the name for the new category:")
        , _("Add Category"), textCtrl_->GetValue());
    if (text.IsEmpty())
        return;

    if (selectedItemId_ == root_)
    {
        if (core_->categoryList_.CategoryExists(text))
        {
            wxString errMsg = _("Category with same name exists");
            errMsg << "\n\n" << _("Tip: If category added now, check bottom of list.\nCategory will be in sorted order next time dialog appears");
            wxMessageBox(errMsg, _("Organise Categories: Adding Error"),wxOK|wxICON_ERROR);
            return;
        }
        int categID = core_->categoryList_.AddCategory(text);

        wxTreeItemId tid = treeCtrl_->AppendItem(selectedItemId_, text);
        treeCtrl_->SetItemData(tid, new mmTreeItemCateg( categID, -1));
        treeCtrl_->Expand(selectedItemId_);

        return;
    }

    mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>(treeCtrl_->GetItemData(selectedItemId_));
    int categID = iData->getCategID();
    int subcategID = iData->getSubCategID();
    if (subcategID == -1) // not subcateg
    {
        if (core_->categoryList_.GetSubCategoryID(categID, text) != -1)
        {
            wxMessageBox(_("Sub Category with same name exists"),
                _("Organise Categories: Adding Error"),wxOK|wxICON_ERROR);
            return;
        }
        int subcategID = core_->categoryList_.AddSubCategory(categID, text);

        wxTreeItemId tid = treeCtrl_->AppendItem(selectedItemId_, text);
        treeCtrl_->SetItemData(tid, new mmTreeItemCateg(categID, subcategID));
        treeCtrl_->Expand(selectedItemId_);
        return;
    }

    wxMessageBox(_("Invalid Parent Category. Choose Root or Main Category node."),
        _("Organise Categories: Adding Error"),wxOK|wxICON_ERROR);

 }

void mmCategDialog::showCategDialogDeleteError(wxString deleteCategoryErrMsg, bool category)
{

    if (category)
        deleteCategoryErrMsg << "\n\n" << _("Tip: Change all transactions using this Category to\nanother Category using the relocate command:");
    else
        deleteCategoryErrMsg << "\n\n" << _("Tip: Change all transactions using this Sub-Category to\nanother Sub-Category using the relocate command:");

    deleteCategoryErrMsg << "\n\n" << _("Tools -> Relocation of -> Categories");

    wxMessageBox(deleteCategoryErrMsg,_("Organise Categories: Delete Error"), wxOK|wxICON_ERROR);
}

void mmCategDialog::OnDelete(wxCommandEvent& /*event*/)
{
    if (mmIniOptions::instance().disableCategoryModify_)
        return;

    if (selectedItemId_ == root_ || !selectedItemId_ )
        return;

    mmTreeItemCateg* iData
        = dynamic_cast<mmTreeItemCateg*>(treeCtrl_->GetItemData(selectedItemId_));
    int categID = iData->getCategID();
    int subcategID = iData->getSubCategID();

    if (subcategID == -1)
    {
        if (!core_->categoryList_.DeleteCategory(categID))
        {
            showCategDialogDeleteError(_("Category in use."));
            return;
        }
    }
    else
    {
        if (!core_->categoryList_.DeleteSubCategory(categID, subcategID))
        {
            showCategDialogDeleteError(_("Sub-Category in use."), false);
            return;
        }
    }

    treeCtrl_->Delete(selectedItemId_);

    wxString sIndex = wxString::Format("*%i:%i*",categID, subcategID);
    wxString sSettings = "";
    for (size_t i = 0; i < hidden_categs_.GetCount(); i++)
    {
        if (subcategID != -1 && hidden_categs_[i] == sIndex)
            hidden_categs_.RemoveAt(i, i);
        else if (subcategID == -1 && hidden_categs_[i].Contains(wxString::Format("*%i:",categID)))
            hidden_categs_.RemoveAt(i, i);
        else
	        sSettings << hidden_categs_[i] << ";";    
    }
    sIndex.RemoveLast(1);

    Model_Infotable::instance().Set("HIDDEN_CATEGS_ID", sSettings);
}

void mmCategDialog::OnBSelect(wxCommandEvent& /*event*/)
{
    if (selectedItemId_ == root_ || !selectedItemId_ )
    {
        return;
    }

    EndModal(wxID_OK);
}

void mmCategDialog::OnDoubleClicked(wxTreeEvent& /*event*/)
{
    if (selectedItemId_ != root_ && selectedItemId_ && bEnableSelect_)
    {
        mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>
            (treeCtrl_->GetItemData(selectedItemId_));
        categID_ = iData->getCategID();
        subcategID_ = iData->getSubCategID();
        EndModal(wxID_OK);
    }
    else
        return;
}

void mmCategDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmCategDialog::OnSelChanged(wxTreeEvent& event)
{
    wxTreeItemId selectedItemId = selectedItemId_;
    selectedItemId_ = event.GetItem();
    if (!selectedItemId_) return;
    if (selectedItemId != selectedItemId_) treeCtrl_->SelectItem(selectedItemId_);

    textCtrl_->SetValue(treeCtrl_->GetItemText(selectedItemId_));

    mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>
        (treeCtrl_->GetItemData(selectedItemId_));
    categID_ = -1;
    subcategID_ = -1;
    if (iData)
    {
        categID_ = iData->getCategID();
        subcategID_ = iData->getSubCategID();
    }

    if (selectedItemId_ == root_ || !selectedItemId_)
    {
        textCtrl_->SetValue("");
        selectButton_->Disable();
        deleteButton_->Disable();
        editButton_->Disable();
        addButton_->Enable();
    }
    else
    {
        selectButton_->Enable(bEnableSelect_);
        bool bIncome = false;
        bool bUsed = core_->bTransactionList_.IsCategoryUsed(categID_, subcategID_, bIncome, subcategID_ == -1)
                || core_->bTransactionList_.IsCategoryUsedBD(categID_, subcategID_, subcategID_ == -1);
        deleteButton_->Enable(!bUsed);
        editButton_->Enable();
        if (subcategID_ != -1)
        {
            // this is a sub categ, cannot add
            addButton_->Disable();
        }
        else
            addButton_->Enable();
    }
}

void mmCategDialog::OnEdit(wxCommandEvent& /*event*/)
{
    if (mmIniOptions::instance().disableCategoryModify_)
        return;

    if (selectedItemId_ == root_ || !selectedItemId_ )
        return;

    wxString old_name = treeCtrl_->GetItemText(selectedItemId_);
    wxString msg = wxString::Format(_("Enter a new name for %s"), old_name);
    wxString text = wxGetTextFromUser(msg
        , _("Edit Category"), textCtrl_->GetValue());
    if (text.IsEmpty())
        return;
    textCtrl_->SetValue(text);

    mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>
        (treeCtrl_->GetItemData(selectedItemId_));
    int categID = iData->getCategID();
    int subcategID = iData->getSubCategID();

    if (!core_->categoryList_.UpdateCategory(categID, subcategID, text))
    {
        wxString errMsg = _("Update Failed");
        errMsg << "          ";  // added to adjust dialog size
        wxMessageBox(errMsg, _("Organise Categories"),wxOK|wxICON_ERROR);
        return;
    }

    treeCtrl_->SetItemText(selectedItemId_, text);
}

wxTreeItemId mmCategDialog::getTreeItemFor(wxTreeItemId itemID, const wxString& itemText)
{
    wxTreeItemIdValue treeDummyValue;

    bool searching = true;
    wxTreeItemId catID = treeCtrl_->GetFirstChild(itemID, treeDummyValue);
    while (catID.IsOk() && searching)
    {
        if ( itemText == treeCtrl_->GetItemText(catID))
            searching = false;
        else
            catID = treeCtrl_->GetNextChild(itemID, treeDummyValue);
    }
    return catID;
}

void mmCategDialog::setTreeSelection(const wxString& catName, const wxString& subCatName)
{
    if ( !catName.IsEmpty() )
    {
        wxTreeItemId catID = getTreeItemFor(treeCtrl_->GetRootItem(), catName);
        if (catID.IsOk() && treeCtrl_->ItemHasChildren(catID))
        {
            if (subCatName.IsEmpty()) {
                treeCtrl_->SelectItem(catID);
            } else {
                treeCtrl_->ExpandAllChildren(catID);
                wxTreeItemId subCatID = getTreeItemFor(catID, subCatName);
                treeCtrl_->SelectItem(subCatID);
            }
        } else {
            treeCtrl_->SelectItem(catID);
        }
    }
}

void mmCategDialog::OnCategoryRelocation(wxCommandEvent& /*event*/)
{
    relocateCategoryDialog dlg(core_, this, categID_, subcategID_);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msgStr;
        msgStr << _("Category Relocation Completed.") << "\n\n"
               << wxString::Format( _("Records have been updated in the database: %s"),
                    dlg.updatedCategoriesCount())
               << "\n\n"
               << _("MMEX must be shutdown and restarted for all the changes to be seen.");
        wxMessageBox(msgStr,_("Category Relocation Result"));
        mmOptions::instance().databaseUpdated_ = true;
    }
}

void mmCategDialog::OnExpandChbClick(wxCommandEvent& /*event*/)
{
    if (cbExpand_->IsChecked())
    {
        treeCtrl_->ExpandAll();
        treeCtrl_->SelectItem(selectedItemId_);
    }
    else
    {
        treeCtrl_->CollapseAll();
        treeCtrl_->Expand(root_);
        treeCtrl_->SelectItem(selectedItemId_);
    }
    treeCtrl_->EnsureVisible(selectedItemId_);
    Model_Setting::instance().Set("EXPAND_CATEGS_TREE", cbExpand_->IsChecked());
}

void mmCategDialog::OnShowHiddenChbClick(wxCommandEvent& /*event*/)
{
    if (cbShowAll_->IsChecked())
    {
        treeCtrl_->SelectItem(selectedItemId_);
    }
    else
    {
        treeCtrl_->SelectItem(selectedItemId_);
    }
    Model_Setting::instance().Set("SHOW_HIDDEN_CATEGS", cbShowAll_->IsChecked());
    fillControls();
}

void mmCategDialog::OnMenuSelected(wxCommandEvent& event)
{
    int id = event.GetId();

    wxString index = wxString::Format("*%i:%i*",categID_, subcategID_);
    if (id == 0)
    {
        treeCtrl_->SetItemTextColour(selectedItemId_, wxColour("GREY"));
        if (hidden_categs_.Index(index) == wxNOT_FOUND )
            hidden_categs_.Add(index);
    }
    else if (id == 1)
    {
        treeCtrl_->SetItemTextColour(selectedItemId_, NormalColor_);
        hidden_categs_.Remove(index);
    }
    else if (id == 2)
    {
        hidden_categs_.Clear();
    }

    wxString sSettings = "";
    for (size_t i = 0; i < hidden_categs_.GetCount(); i++)
    {
        sSettings.Append(hidden_categs_[i]).Append(";");
    }
    sSettings.RemoveLast(1);

    Model_Infotable::instance().Set("HIDDEN_CATEGS_ID", sSettings);

    if (!cbShowAll_->IsChecked() || id == 2) fillControls();
    event.Skip();
}

void mmCategDialog::OnItemRightClick(wxTreeEvent& event)
{
    wxMenu* mainMenu = new wxMenu;
    mainMenu->Append(new wxMenuItem(mainMenu, 0, _("Hide Selected Category")));
    mainMenu->Append(new wxMenuItem(mainMenu, 1, _("Unhide Selected Category")));
    mainMenu->AppendSeparator();
    mainMenu->Append(new wxMenuItem(mainMenu, 2, _("Clear Settings")));

    bool bItemHidden = (treeCtrl_->GetItemTextColour(selectedItemId_) != NormalColor_);
    mainMenu->Enable(0, !bItemHidden && (selectedItemId_ != root_));
    mainMenu->Enable(1, bItemHidden && (selectedItemId_ != root_));

    PopupMenu(mainMenu, event.GetPoint());
    delete mainMenu;
    event.Skip();
}

bool mmCategDialog::categShowStatus(int categId, int subCategId)
{
    wxString index = wxString::Format("*%i:%i*",categId, subCategId);
    return hidden_categs_.Index(index) == wxNOT_FOUND;
}
