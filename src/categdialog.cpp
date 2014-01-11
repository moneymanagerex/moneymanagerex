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
#include "model/Model_Payee.h"
#include "model/Model_Infotable.h"
#include "../resources/relocate_categories.xpm"

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
    EVT_MENU(wxID_ANY, mmCategDialog::OnMenuSelected)
END_EVENT_TABLE()

mmCategDialog::mmCategDialog( )
: m_treeCtrl()
, m_textCtrl()
, m_buttonAdd()
, m_buttonEdit()
, m_buttonSelect()
, m_buttonDelete()
, m_buttonRelocate()
, m_cbExpand()
, m_cbShowAll()
{
    // Initialize fields in constructor
    categID_ = -1;
    subcategID_ = -1;
    selectedItemId_ = 0;
    bEnableSelect_ = false;
    bEnableRelocate_ = false;
    refreshRequested_ = false;
}

mmCategDialog::mmCategDialog(wxWindow* parent
    , bool bEnableSelect, bool bEnableRelocate)
{
    // Initialize fields in constructor
    categID_ = -1;
    subcategID_ = -1;
    selectedItemId_ = 0;
    bEnableSelect_ = bEnableSelect;
    bEnableRelocate_ = bEnableRelocate;
    refreshRequested_ = false;

    //Get Hidden Categories id from stored string
    hidden_categs_.clear();
    wxString sSettings = Model_Infotable::instance().GetStringInfo("HIDDEN_CATEGS_ID", "");
    wxStringTokenizer token(sSettings, ";");
    while (token.HasMoreTokens())
    {
        hidden_categs_.Add( token.GetNextToken() );
    }

    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX | wxRESIZE_BORDER;
    Create(parent, ID_DIALOG_CATEGORY, _("Organize Categories")
        , wxDefaultPosition, wxSize(500, 300), style);
}

bool mmCategDialog::Create(wxWindow* parent, wxWindowID id
    , const wxString& caption, const wxPoint& pos
    , const wxSize& size, long style)
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
    m_treeCtrl->DeleteAllItems();
    root_ = m_treeCtrl->AddRoot(_("Categories"));
    selectedItemId_ = root_;
    m_treeCtrl->SetItemBold(root_, true);
    m_treeCtrl->SetFocus ();
    NormalColor_ = m_treeCtrl->GetItemTextColour(root_);
    bool bResult = Model_Setting::instance().GetBoolSetting("SHOW_HIDDEN_CATEGS", true);
    m_cbShowAll->SetValue(bResult);

    Model_Category::Data_Set categories = Model_Category::instance().all();
    for (const Model_Category::Data& category : categories)
    {
        wxTreeItemId maincat;
        bool bShow = categShowStatus(category.CATEGID, -1);
        if (m_cbShowAll->IsChecked() || bShow)
        {
            maincat = m_treeCtrl->AppendItem(root_, category.CATEGNAME);
            Model_Subcategory::Data subcat = 0;
            m_treeCtrl->SetItemData(maincat, new mmTreeItemCateg(category, subcat));
            if (!bShow) m_treeCtrl->SetItemTextColour(maincat, wxColour("GREY"));

            for (const Model_Subcategory::Data& sub_category : Model_Category::sub_category(category))
            {
                bShow = categShowStatus(category.CATEGID, sub_category.SUBCATEGID);
                if (m_cbShowAll->IsChecked() || bShow)
                {
                    wxTreeItemId subcat = m_treeCtrl->AppendItem(maincat, sub_category.SUBCATEGNAME);
                    m_treeCtrl->SetItemData(subcat, new mmTreeItemCateg(category, sub_category));
                    if (!bShow) m_treeCtrl->SetItemTextColour(subcat, wxColour("GREY"));

                    if (categID_ == category.CATEGID && subcategID_ == sub_category.SUBCATEGID)
                        selectedItemId_ = subcat;
                }
            }
            m_treeCtrl->SortChildren(maincat);
        }
    }
    m_treeCtrl->Expand(root_);
    bResult = Model_Setting::instance().GetBoolSetting("EXPAND_CATEGS_TREE", false);
    if (bResult) m_treeCtrl->ExpandAll();
    m_cbExpand->SetValue(bResult);

    m_treeCtrl->SortChildren(root_);
    m_treeCtrl->SelectItem(selectedItemId_);
    m_treeCtrl->EnsureVisible(selectedItemId_);

    m_textCtrl->SetValue("");
    m_buttonSelect->Disable();
    m_buttonDelete->Disable();
    m_buttonEdit->Disable();
    m_buttonAdd->Enable();
    m_buttonRelocate->Enable(bEnableRelocate_);
}

void mmCategDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 1);
    wxBoxSizer* itemBoxSizer33 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer33);

    m_buttonRelocate = new wxBitmapButton(this
        , wxID_ANY, wxBitmap(relocate_categories_xpm));
    m_buttonRelocate->Connect(wxID_STATIC, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmCategDialog::OnCategoryRelocation), NULL, this);
    m_buttonRelocate->SetToolTip(_("Reassign all categories to another category"));

    m_cbExpand = new wxCheckBox(this, wxID_ANY, _("Expand"), wxDefaultPosition
        , wxDefaultSize, wxCHK_2STATE);
    m_cbExpand->Connect(wxID_ANY, wxEVT_COMMAND_CHECKBOX_CLICKED,
        wxCommandEventHandler(mmCategDialog::OnExpandChbClick), NULL, this);

    m_cbShowAll = new wxCheckBox(this, wxID_SELECTALL, _("Show All"), wxDefaultPosition
        , wxDefaultSize, wxCHK_2STATE);
    m_cbShowAll->SetToolTip(_("Show all hidden categories"));
    m_cbShowAll->Connect(wxID_SELECTALL, wxEVT_COMMAND_CHECKBOX_CLICKED
        , wxCommandEventHandler(mmCategDialog::OnShowHiddenChbClick), NULL, this);

    itemBoxSizer33->Add(m_buttonRelocate, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);
    itemBoxSizer33->AddSpacer(10);
    itemBoxSizer33->Add(m_cbExpand, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);
    itemBoxSizer33->AddSpacer(10);
    itemBoxSizer33->Add(m_cbShowAll, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1);

#if defined (__WXGTK__) || defined (__WXMAC__)
    m_treeCtrl = new wxTreeCtrl( this, wxID_ANY,
        wxDefaultPosition, wxSize(200, 380));
#else
    m_treeCtrl = new wxTreeCtrl( this, wxID_ANY,
        wxDefaultPosition, wxSize(200, 380), wxTR_SINGLE
        | wxTR_HAS_BUTTONS
        | wxTR_ROW_LINES );
#endif
    itemBoxSizer3->Add(m_treeCtrl, 1, wxGROW|wxALL, 1);

    m_textCtrl = new wxTextCtrl(this, wxID_ANY, "");
    itemBoxSizer2->Add(m_textCtrl, 0, wxGROW | wxALIGN_CENTER_VERTICAL | wxALL, 1);
    m_textCtrl->SetToolTip(_("Enter the name of the category to add or edit here"));

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxGROW|wxALL, 5);

    m_buttonAdd = new wxButton(this, wxID_ADD, _("&Add "));
    itemBoxSizer5->Add(m_buttonAdd, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1);
    m_buttonAdd->SetToolTip(_("Add a new category"));

    m_buttonEdit = new wxButton(this, wxID_EDIT, _("&Edit "));
    itemBoxSizer5->Add(m_buttonEdit, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1);
    m_buttonEdit->SetToolTip(_("Edit the name of an existing category"));

    m_buttonDelete = new wxButton(this, wxID_REMOVE, _("&Delete "));
    itemBoxSizer5->Add(m_buttonDelete, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1);
    m_buttonDelete->SetToolTip(_("Delete an existing category. The category cannot be used by existing transactions."));

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxGROW | wxALL, 5);

    m_buttonSelect = new wxButton(this, wxID_OK
        , _("&Select"));
    itemBoxSizer9->Add(m_buttonSelect, 1, wxALIGN_CENTER_VERTICAL | wxALL, 1);
    m_buttonSelect->SetToolTip(_("Select the currently selected category as the selected category for the transaction"));

    //Some interfaces has no any close buttons, it may confuse user. Cancel button added
    wxButton* itemCancelButton = new wxButton(this, wxID_CANCEL, _("&Cancel "));
    itemBoxSizer9->Add(itemCancelButton);
}

void mmCategDialog::OnAdd(wxCommandEvent& /*event*/)
{
    wxString text = wxGetTextFromUser(_("Enter the name for the new category:")
        , _("Add Category"), m_textCtrl->GetValue());
    if (text.IsEmpty())
        return;

    if (selectedItemId_ == root_)
    {
        Model_Category::Data_Set categories = Model_Category::instance().find(Model_Category::CATEGNAME(text));
        if (!categories.empty())
        {
            wxString errMsg = _("Category with same name exists");
            errMsg << "\n\n" << _("Tip: If category added now, check bottom of list.\nCategory will be in sorted order next time dialog appears");
            wxMessageBox(errMsg, _("Organise Categories: Adding Error"), wxOK|wxICON_ERROR);
            return;
        }
        Model_Category::Data *category = Model_Category::instance().create();
        category->CATEGNAME = text;
        Model_Category::instance().save(category);

        wxTreeItemId tid = m_treeCtrl->AppendItem(selectedItemId_, text);
        Model_Subcategory::Data subcat = 0;
        m_treeCtrl->SetItemData(tid, new mmTreeItemCateg(*category, subcat));
        m_treeCtrl->Expand(selectedItemId_);

        return;
    }

    mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>(m_treeCtrl->GetItemData(selectedItemId_));
    if (iData->getSubCategData()->SUBCATEGID == -1) // not subcateg
    {
        Model_Subcategory::Data_Set subcategories = Model_Category::sub_category(iData->getCategData());
        for (const auto& subcategory : subcategories)
        {
            if (subcategory.SUBCATEGNAME == text)
            {
                wxMessageBox(_("Sub Category with same name exists")
                    , _("Organise Categories: Adding Error"), wxOK | wxICON_ERROR);
                return;
            }
        }
        Model_Subcategory::Data *subcategory = Model_Subcategory::instance().create();
        subcategory->SUBCATEGNAME = text;
        subcategory->CATEGID = iData->getCategData()->CATEGID;
        Model_Subcategory::instance().save(subcategory);

        wxTreeItemId tid = m_treeCtrl->AppendItem(selectedItemId_, text);
        m_treeCtrl->SetItemData(tid, new mmTreeItemCateg(*iData->getCategData(), *subcategory));
        m_treeCtrl->Expand(selectedItemId_);
        return;
    }

    wxMessageBox(_("Invalid Parent Category. Choose Root or Main Category node.")
        , _("Organise Categories: Adding Error"),wxOK|wxICON_ERROR);

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
    if (selectedItemId_ == root_ || !selectedItemId_ )
        return;

    mmTreeItemCateg* iData
        = dynamic_cast<mmTreeItemCateg*>(m_treeCtrl->GetItemData(selectedItemId_));
    int categID = iData->getCategData()->CATEGID;
    int subcategID = -1;

    if (iData->getSubCategData()->SUBCATEGID == -1) // not subcateg
    {
        if (Model_Category::is_used(categID))
        {
            showCategDialogDeleteError(_("Category in use."));
            return;
        }
        else
        {
            Model_Category::instance().remove(categID);
        }
    }
    else
    {
        subcategID = iData->getSubCategData()->SUBCATEGID;
        if (Model_Category::is_used(categID, subcategID))
        {
            showCategDialogDeleteError(_("Sub-Category in use."), false);
            return;
        }
        else
        {
            Model_Subcategory::instance().remove(subcategID);
        }
    }

    m_treeCtrl->Delete(selectedItemId_);

    //Clear categories associated with payees
    Model_Payee::Data_Set payees = Model_Payee::instance().all();
    for (auto &payee : payees)
    {
        if (payee.CATEGID == categID || (payee.SUBCATEGID == subcategID && subcategID != -1))
        {
            payee.CATEGID = -1;
            payee.SUBCATEGID = -1;
        }
    }
    Model_Payee::instance().save(payees);

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
    if (selectedItemId_ != root_ && selectedItemId_ )
    {
        EndModal(wxID_OK); 
    }
    else
        return;
}

void mmCategDialog::OnDoubleClicked(wxTreeEvent& /*event*/)
{
    if (selectedItemId_ != root_ && selectedItemId_ && bEnableSelect_)
    {
        mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>
            (m_treeCtrl->GetItemData(selectedItemId_));
        categID_ = iData->getCategData()->CATEGID;
        subcategID_ = iData->getSubCategData()->SUBCATEGID;
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
    if (selectedItemId != selectedItemId_) m_treeCtrl->SelectItem(selectedItemId_);

    m_textCtrl->SetValue(m_treeCtrl->GetItemText(selectedItemId_));

    mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>
        (m_treeCtrl->GetItemData(selectedItemId_));
    categID_ = -1;
    subcategID_ = -1;
    if (iData)
    {
        categID_ = iData->getCategData()->CATEGID;
        subcategID_ = iData->getSubCategData()->SUBCATEGID;
    }

    if (selectedItemId_ == root_)
    {
        m_textCtrl->SetValue("");
        m_buttonSelect->Disable();
        m_buttonDelete->Disable();
    }
    else
    {
        m_buttonSelect->Enable(bEnableSelect_);
        bool bUsed = Model_Category::is_used(categID_, subcategID_);
        if (subcategID_ == -1)
        {
	        Model_Category::Data *category = Model_Category::instance().get(categID_);
	        Model_Subcategory::Data_Set subcategories = Model_Category::sub_category(category);
	        for (const auto& s : subcategories) bUsed = bUsed || Model_Category::is_used(categID_, s.SUBCATEGID);
        }

        m_buttonDelete->Enable(!bUsed);
    }
    m_buttonAdd->Enable(subcategID_ == -1);
    m_buttonEdit->Enable(selectedItemId_ != root_);
}

void mmCategDialog::OnEdit(wxCommandEvent& /*event*/)
{
    if (selectedItemId_ == root_ || !selectedItemId_ )
        return;

    wxString old_name = m_treeCtrl->GetItemText(selectedItemId_);
    wxString msg = wxString::Format(_("Enter a new name for %s"), old_name);
    wxString text = wxGetTextFromUser(msg
        , _("Edit Category"), m_textCtrl->GetValue());
    if (text.IsEmpty())
        return;
    m_textCtrl->SetValue(text);

    mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>
        (m_treeCtrl->GetItemData(selectedItemId_));

    if (iData->getSubCategData()->SUBCATEGID == -1) // not subcateg
    {
        Model_Category::Data_Set categories = Model_Category::instance().find(Model_Category::CATEGNAME(text));
        if (!categories.empty())
        {
            wxString errMsg = _("Category with same name exists");
            wxMessageBox(errMsg, _("Organise Categories: Editing Error"), wxOK|wxICON_ERROR);
            return;
        }
        Model_Category::Data* category = iData->getCategData();
        category->CATEGNAME = text;
        Model_Category::instance().save(category);
    }
    else
    {
        Model_Category::Data* category = iData->getCategData();
        Model_Subcategory::Data_Set subcategories = Model_Category::sub_category(category);
        for (const auto &entry : subcategories)
        {
            if (entry.SUBCATEGNAME == text)
            {
                wxString errMsg = _("Sub Category with same name exists");
                wxMessageBox(errMsg, _("Organise Categories: Editing Error"), wxOK|wxICON_ERROR);
                return;
            }
        }
        Model_Subcategory::Data* sub_category = iData->getSubCategData();
        sub_category->SUBCATEGNAME = text;
        Model_Subcategory::instance().save(sub_category);
    }

    m_treeCtrl->SetItemText(selectedItemId_, text);

    refreshRequested_ = true;
}

wxTreeItemId mmCategDialog::getTreeItemFor(wxTreeItemId itemID, const wxString& itemText)
{
    wxTreeItemIdValue treeDummyValue;

    bool searching = true;
    wxTreeItemId catID = m_treeCtrl->GetFirstChild(itemID, treeDummyValue);
    while (catID.IsOk() && searching)
    {
        if ( itemText == m_treeCtrl->GetItemText(catID))
            searching = false;
        else
            catID = m_treeCtrl->GetNextChild(itemID, treeDummyValue);
    }
    return catID;
}

void mmCategDialog::setTreeSelection(int &category_id, int &subcategory_id)
{
    Model_Category::Data_Set categories = Model_Category::instance().find(Model_Category::CATEGID(category_id));
    if (!categories.empty())
    {
        Model_Category::Data *category = Model_Category::instance().get(category_id);
        Model_Subcategory::Data *subcategory = (subcategory_id != -1 ? Model_Subcategory::instance().get(subcategory_id) : 0);
        wxString categoryName = "", subCategoryName = "";
        if (category) categoryName = category->CATEGNAME;
        if (subcategory) subCategoryName = subcategory->SUBCATEGNAME;
        setTreeSelection(categoryName, subCategoryName);
    }
}

void mmCategDialog::setTreeSelection(const wxString& catName, const wxString& subCatName)
{
    if ( !catName.IsEmpty() )
    {
        wxTreeItemId catID = getTreeItemFor(m_treeCtrl->GetRootItem(), catName);
        if (catID.IsOk() && m_treeCtrl->ItemHasChildren(catID))
        {
            if (subCatName.IsEmpty()) {
                m_treeCtrl->SelectItem(catID);
            } else {
                m_treeCtrl->ExpandAllChildren(catID);
                wxTreeItemId subCatID = getTreeItemFor(catID, subCatName);
                m_treeCtrl->SelectItem(subCatID);
            }
        } else {
            m_treeCtrl->SelectItem(catID);
        }
    }
}

void mmCategDialog::OnCategoryRelocation(wxCommandEvent& /*event*/)
{
    relocateCategoryDialog dlg(this, categID_, subcategID_);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msgStr;
        msgStr << _("Category Relocation Completed.") << "\n\n"
               << wxString::Format( _("Records have been updated in the database: %i"),
                    dlg.updatedCategoriesCount());
        wxMessageBox(msgStr,_("Category Relocation Result"));
        refreshRequested_ = true;
        fillControls();
    }
}

void mmCategDialog::OnExpandChbClick(wxCommandEvent& /*event*/)
{
    if (m_cbExpand->IsChecked())
    {
        m_treeCtrl->ExpandAll();
        m_treeCtrl->SelectItem(selectedItemId_);
    }
    else
    {
        m_treeCtrl->CollapseAll();
        m_treeCtrl->Expand(root_);
        m_treeCtrl->SelectItem(selectedItemId_);
    }
    m_treeCtrl->EnsureVisible(selectedItemId_);
    Model_Setting::instance().Set("EXPAND_CATEGS_TREE", m_cbExpand->IsChecked());
}

void mmCategDialog::OnShowHiddenChbClick(wxCommandEvent& /*event*/)
{
    Model_Setting::instance().Set("SHOW_HIDDEN_CATEGS", m_cbShowAll->IsChecked());
    fillControls();
}

void mmCategDialog::OnMenuSelected(wxCommandEvent& event)
{
    int id = event.GetId();

    wxString index = wxString::Format("*%i:%i*",categID_, subcategID_);
    if (id == MENU_ITEM_HIDE)
    {
        m_treeCtrl->SetItemTextColour(selectedItemId_, wxColour("GREY"));
        if (hidden_categs_.Index(index) == wxNOT_FOUND )
            hidden_categs_.Add(index);
    }
    else if (id == MENU_ITEM_UNHIDE)
    {
        m_treeCtrl->SetItemTextColour(selectedItemId_, NormalColor_);
        hidden_categs_.Remove(index);
    }
    else if (id == MENU_ITEM_CLEAR)
    {
        hidden_categs_.Clear();
    }

    wxString sSettings = "";
    for (const auto& item : hidden_categs_)
    {
        sSettings.Append(item + ";");
    }
    sSettings.RemoveLast(1);

    Model_Infotable::instance().Set("HIDDEN_CATEGS_ID", sSettings);

    if (!m_cbShowAll->IsChecked() || id == MENU_ITEM_CLEAR) fillControls();
}

void mmCategDialog::OnItemRightClick(wxTreeEvent& event)
{
    wxMenu* mainMenu = new wxMenu;
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_ITEM_HIDE, _("Hide Selected Category")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_ITEM_UNHIDE, _("Unhide Selected Category")));
    mainMenu->AppendSeparator();
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_ITEM_CLEAR, _("Clear Settings")));

    bool bItemHidden = (m_treeCtrl->GetItemTextColour(selectedItemId_) != NormalColor_);
    mainMenu->Enable(MENU_ITEM_HIDE, !bItemHidden && (selectedItemId_ != root_));
    mainMenu->Enable(MENU_ITEM_UNHIDE, bItemHidden && (selectedItemId_ != root_));

    PopupMenu(mainMenu, event.GetPoint());
    delete mainMenu;
    event.Skip();
}

bool mmCategDialog::categShowStatus(int categId, int subCategId)
{
    wxString index = wxString::Format("*%i:%i*",categId, subCategId);
    return hidden_categs_.Index(index) == wxNOT_FOUND;
}

wxString mmCategDialog::getFullCategName()
{
    Model_Category::Data *category = Model_Category::instance().get(categID_);
    Model_Subcategory::Data *subcategory = (subcategID_ != -1 ? Model_Subcategory::instance().get(subcategID_) : 0);
    return Model_Category::full_name(category, subcategory);
}
