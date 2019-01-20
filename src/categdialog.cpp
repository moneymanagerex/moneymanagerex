/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016 Nikolay Akimov

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

#include "categdialog.h"
#include "images_list.h"
#include "relocatecategorydialog.h"
#include "util.h"
#include "option.h"
#include "paths.h"
#include "constants.h"
#include "webapp.h"
#include "Model_Setting.h"
#include "Model_Payee.h"
#include "Model_Infotable.h"

wxIMPLEMENT_DYNAMIC_CLASS(mmCategDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmCategDialog, wxDialog)
EVT_BUTTON(wxID_OK, mmCategDialog::OnBSelect)
EVT_BUTTON(wxID_CANCEL, mmCategDialog::OnCancel)
EVT_BUTTON(wxID_ADD, mmCategDialog::OnAdd)
EVT_BUTTON(wxID_REMOVE, mmCategDialog::OnDelete)
EVT_BUTTON(wxID_EDIT, mmCategDialog::OnEdit)
EVT_TREE_SEL_CHANGED(wxID_ANY, mmCategDialog::OnSelChanged)
EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY, mmCategDialog::OnSelChanged)
EVT_TREE_ITEM_ACTIVATED(wxID_ANY, mmCategDialog::OnDoubleClicked)
EVT_TREE_ITEM_MENU(wxID_ANY, mmCategDialog::OnItemRightClick)
EVT_MENU(wxID_ANY, mmCategDialog::OnMenuSelected)
wxEND_EVENT_TABLE()

mmCategDialog::mmCategDialog()
: m_treeCtrl(nullptr)
, m_buttonAdd(nullptr)
, m_buttonEdit(nullptr)
, m_buttonSelect(nullptr)
, m_buttonDelete(nullptr)
, m_buttonRelocate(nullptr)
, m_cbExpand(nullptr)
, m_cbShowAll(nullptr)
{
    // Initialize fields in constructor
    m_categ_id = -1;
    m_subcateg_id = -1;
    m_init_selected_categ_id = -1;
    m_init_selected_subcateg_id = -1;
    selectedItemId_ = 0;
    m_enable_select = false;
    m_enable_relocate = false;
    m_refresh_requested = false;
}

mmCategDialog::mmCategDialog(wxWindow* parent
    , int category_id, int subcategory_id
    , bool bEnableRelocate, bool bEnableSelect)
{
    // Initialize fields in constructor
    m_categ_id = category_id;
    m_subcateg_id = subcategory_id;
    m_init_selected_categ_id = category_id;
    m_init_selected_subcateg_id = subcategory_id;
    selectedItemId_ = 0;
    m_enable_select = bEnableSelect;
    m_enable_relocate = bEnableRelocate;
    m_refresh_requested = false;

    //Get Hidden Categories id from stored string
    m_hidden_categs.clear();
    wxString sSettings = Model_Infotable::instance().GetStringInfo("HIDDEN_CATEGS_ID", "");
    wxStringTokenizer token(sSettings, ";");
    while (token.HasMoreTokens())
    {
        m_hidden_categs.Add(token.GetNextToken());
    }

    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX | wxRESIZE_BORDER;
    Create(parent, ID_DIALOG_CATEGORY, _("Organize Categories")
        , wxDefaultPosition, wxSize(500, 300), style);
}

bool mmCategDialog::Create(wxWindow* parent, wxWindowID id
    , const wxString& caption, const wxPoint& pos
    , const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    fillControls();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmex::getProgramIcon());
    SetMinSize(wxSize(316, 316));
    Centre();
    return TRUE;
}

void mmCategDialog::fillControls()
{
    m_treeCtrl->DeleteAllItems();
    root_ = m_treeCtrl->AddRoot(_("Categories"));
    selectedItemId_ = root_;
    m_treeCtrl->SetItemBold(root_, true);
    m_treeCtrl->SetFocus();
    NormalColor_ = m_treeCtrl->GetItemTextColour(root_);
    bool show_hidden_categs = Model_Setting::instance().GetBoolSetting("SHOW_HIDDEN_CATEGS", true);
    m_cbShowAll->SetValue(show_hidden_categs);

    const auto &categories = Model_Category::instance().all();
    for (const Model_Category::Data& category : categories)
    {
        wxTreeItemId maincat;
        bool bShow = categShowStatus(category.CATEGID, -1);
        if (m_cbShowAll->IsChecked() || bShow || category.CATEGID == m_init_selected_categ_id)
        {
            maincat = m_treeCtrl->AppendItem(root_, category.CATEGNAME);
            Model_Subcategory::Data subcat;
            m_treeCtrl->SetItemData(maincat, new mmTreeItemCateg(category, subcat));
            if (!bShow)
                m_treeCtrl->SetItemTextColour(maincat, wxColour("GREY"));

            for (const auto &sub_category : Model_Category::sub_category(category))
            {
                bShow = categShowStatus(category.CATEGID, sub_category.SUBCATEGID);
                if (m_cbShowAll->IsChecked() || bShow || sub_category.SUBCATEGID == m_init_selected_subcateg_id)
                {
                    wxTreeItemId subcateg = m_treeCtrl->AppendItem(maincat, sub_category.SUBCATEGNAME);
                    m_treeCtrl->SetItemData(subcateg, new mmTreeItemCateg(category, sub_category));
                    if (!bShow) 
                        m_treeCtrl->SetItemTextColour(subcateg, wxColour("GREY"));

                    if (m_categ_id == category.CATEGID && m_subcateg_id == sub_category.SUBCATEGID)
                        selectedItemId_ = subcateg;
                }
            }
            m_treeCtrl->SortChildren(maincat);
        }
    }
    m_treeCtrl->Expand(root_);
    bool expand_categs_tree = Model_Setting::instance().GetBoolSetting("EXPAND_CATEGS_TREE", false);
    if (expand_categs_tree) m_treeCtrl->ExpandAll();
    m_cbExpand->SetValue(expand_categs_tree);

    m_treeCtrl->SortChildren(root_);
    m_treeCtrl->SelectItem(selectedItemId_);
    m_treeCtrl->EnsureVisible(selectedItemId_);

    m_buttonSelect->Disable();
    m_buttonEdit->Disable();
    m_buttonAdd->Enable();
    m_buttonRelocate->Enable(m_enable_relocate);

    setTreeSelection(m_categ_id, m_subcateg_id);
}

void mmCategDialog::CreateControls()
{
    wxBoxSizer* mainSizerVertical = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainSizerVertical);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    mainSizerVertical->Add(itemBoxSizer3, g_flagsExpand);
    wxBoxSizer* itemBoxSizer33 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer33);

    m_buttonRelocate = new wxBitmapButton(this
        , wxID_REPLACE_ALL, mmBitmap(png::CATEGORY_RELOCATION));
    m_buttonRelocate->Connect(wxID_REPLACE_ALL, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmCategDialog::OnCategoryRelocation), nullptr, this);
    m_buttonRelocate->SetToolTip(_("Reassign all categories to another category"));

    m_cbExpand = new wxCheckBox(this, wxID_ANY, _("Expand"), wxDefaultPosition
        , wxDefaultSize, wxCHK_2STATE);
    m_cbExpand->Connect(wxID_ANY, wxEVT_COMMAND_CHECKBOX_CLICKED,
        wxCommandEventHandler(mmCategDialog::OnExpandChbClick), nullptr, this);

    m_cbShowAll = new wxCheckBox(this, wxID_SELECTALL, _("Show All"), wxDefaultPosition
        , wxDefaultSize, wxCHK_2STATE);
    m_cbShowAll->SetToolTip(_("Show all hidden categories"));
    m_cbShowAll->Connect(wxID_SELECTALL, wxEVT_COMMAND_CHECKBOX_CLICKED
        , wxCommandEventHandler(mmCategDialog::OnShowHiddenChbClick), nullptr, this);

    itemBoxSizer33->Add(m_buttonRelocate, g_flagsH);
    itemBoxSizer33->AddSpacer(10);
    itemBoxSizer33->Add(m_cbExpand, g_flagsH);
    itemBoxSizer33->AddSpacer(10);
    itemBoxSizer33->Add(m_cbShowAll, g_flagsH);

#if defined (__WXGTK__) || defined (__WXMAC__)
    m_treeCtrl = new wxTreeCtrl( this, wxID_ANY,
        wxDefaultPosition, wxSize(200, 380));
#else
    m_treeCtrl = new wxTreeCtrl(this, wxID_ANY
        , wxDefaultPosition, wxSize(200, 380)
        , wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_ROW_LINES);
#endif
    itemBoxSizer3->Add(m_treeCtrl, g_flagsExpand);

    wxPanel* buttonsPanel = new wxPanel(this, wxID_ANY);
    mainSizerVertical->Add(buttonsPanel, wxSizerFlags(g_flagsV).Center());
    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxVERTICAL);
    buttonsPanel->SetSizer(buttonsSizer);

    wxStdDialogButtonSizer* itemBoxSizer66 = new wxStdDialogButtonSizer;
    buttonsSizer->Add(itemBoxSizer66);

    m_buttonAdd = new wxButton(buttonsPanel, wxID_ADD, _("&Add "));
    itemBoxSizer66->Add(m_buttonAdd, g_flagsH);
    m_buttonAdd->SetToolTip(_("Add a new category"));

    m_buttonEdit = new wxButton(buttonsPanel, wxID_EDIT, _("&Edit "));
    itemBoxSizer66->Add(m_buttonEdit, g_flagsH);
    m_buttonEdit->SetToolTip(_("Edit the name of an existing category"));

    m_buttonDelete = new wxButton(buttonsPanel, wxID_REMOVE, _("&Delete "));
    itemBoxSizer66->Add(m_buttonDelete, g_flagsH);
    m_buttonDelete->SetToolTip(_("Delete an existing category. The category cannot be used by existing transactions."));

    wxStdDialogButtonSizer* itemBoxSizer9 = new wxStdDialogButtonSizer;
    buttonsSizer->Add(itemBoxSizer9, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));

    m_buttonSelect = new wxButton(buttonsPanel, wxID_OK, _("&Select"));
    itemBoxSizer9->Add(m_buttonSelect, wxSizerFlags(g_flagsExpand).Proportion(4));
    m_buttonSelect->SetToolTip(_("Select the currently selected category as the selected category for the transaction"));

    //Some interfaces has no any close buttons, it may confuse user. Cancel button added
    wxButton* itemCancelButton = new wxButton(buttonsPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    itemBoxSizer9->Add(itemCancelButton, g_flagsH);
}

void mmCategDialog::OnAdd(wxCommandEvent& WXUNUSED(event))
{
    wxString prompt_msg = _("Enter the name for the new category:");
    if (selectedItemId_ == root_)
    {
        prompt_msg << "\n\n" << _("Tip: If category added now, check bottom of list.");
    }

    const wxString& text = wxGetTextFromUser(prompt_msg, _("Add Category"), "");
    if (text.IsEmpty())
        return;

    if (selectedItemId_ == root_)
    {
        const auto &categories = Model_Category::instance().find(Model_Category::CATEGNAME(text));
        if (!categories.empty())
        {
            wxString errMsg = _("Category with same name exists");
            errMsg << "\n\n" << _("Tip: If category added now, check bottom of list.\n"
                "Category will be in sorted order next time dialog appears");
            wxMessageBox(errMsg, _("Organise Categories: Adding Error"), wxOK | wxICON_ERROR);
            return;
        }
        Model_Category::Data *category = Model_Category::instance().create();
        category->CATEGNAME = text;
        Model_Category::instance().save(category);
        mmWebApp::MMEX_WebApp_UpdateCategory();

        wxTreeItemId tid = m_treeCtrl->AppendItem(selectedItemId_, text);
        Model_Subcategory::Data subcat;
        m_treeCtrl->SetItemData(tid, new mmTreeItemCateg(*category, subcat));
        m_treeCtrl->Expand(selectedItemId_);
    }

    mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>(m_treeCtrl->GetItemData(selectedItemId_));
    if (!iData) return; // node added at root level
    if (iData->getSubCategData()->SUBCATEGID == -1) // not subcateg
    {
        const auto &subcategories = Model_Category::sub_category(iData->getCategData());
        for (const auto& subcategory : subcategories)
        {
            if (subcategory.SUBCATEGNAME.Upper() == text.Upper())
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
        mmWebApp::MMEX_WebApp_UpdateCategory();

        wxTreeItemId tid = m_treeCtrl->AppendItem(selectedItemId_, text);
        m_treeCtrl->SetItemData(tid, new mmTreeItemCateg(*iData->getCategData(), *subcategory));
        m_treeCtrl->Expand(selectedItemId_);
        m_refresh_requested = true;
        return;
    }

    wxMessageBox(_("Invalid Parent Category. Choose Root or Main Category node.")
        , _("Organise Categories: Adding Error"), wxOK | wxICON_ERROR);

}

void mmCategDialog::showCategDialogDeleteError(bool category)
{
    wxString deleteCategoryErrMsg = category ? _("Category in use.") : _("Sub-Category in use.");
    if (category)
        deleteCategoryErrMsg << "\n\n" << _("Tip: Change all transactions using this Category to\n"
            "another Category using the relocate command:");
    else
        deleteCategoryErrMsg << "\n\n" << _("Tip: Change all transactions using this Sub-Category to\n"
            "another Sub-Category using the relocate command:");

    deleteCategoryErrMsg << "\n\n" << _("Tools -> Relocation of -> Categories");

    wxMessageBox(deleteCategoryErrMsg, _("Organise Categories: Delete Error"), wxOK | wxICON_ERROR);
}

void mmCategDialog::OnDelete(wxCommandEvent& WXUNUSED(event))
{
    if (!selectedItemId_ || selectedItemId_ == root_)
        return;

    if (m_treeCtrl->ItemHasChildren(selectedItemId_))
        return; //TODO: Show error message "Delete childs first"

    mmTreeItemCateg* iData
        = dynamic_cast<mmTreeItemCateg*>(m_treeCtrl->GetItemData(selectedItemId_));
    wxTreeItemId PreviousItem = m_treeCtrl->GetPrevVisible(selectedItemId_);
    int categID = iData->getCategData()->CATEGID;
    int subcategID = iData->getSubCategData()->SUBCATEGID;

    if (subcategID == -1)
    {
        if (Model_Category::is_used(categID) || categID == m_init_selected_categ_id)
            return showCategDialogDeleteError();
        else
        {
            Model_Category::instance().remove(categID);
            mmWebApp::MMEX_WebApp_UpdateCategory();
        }
    }
    else
    {
        if (Model_Category::is_used(categID, subcategID) || ((categID == m_init_selected_categ_id) && (subcategID == m_init_selected_subcateg_id)))
            return showCategDialogDeleteError(false);
        else
        {
            Model_Subcategory::instance().remove(subcategID);
            mmWebApp::MMEX_WebApp_UpdateCategory();
        }
    }

    m_refresh_requested = true;
    m_treeCtrl->Delete(selectedItemId_);

    //Clear categories associated with payees
    auto payees = Model_Payee::instance().all();
    for (auto &payee : payees)
    {
        if (payee.CATEGID == categID || (payee.SUBCATEGID == subcategID && subcategID != -1))
        {
            payee.CATEGID = -1;
            payee.SUBCATEGID = -1;
        }
    }
    Model_Payee::instance().save(payees);
    mmWebApp::MMEX_WebApp_UpdatePayee();

    wxString sIndex = wxString::Format("*%i:%i*", categID, subcategID);
    wxString sSettings = "";
    for (size_t i = 0; i < m_hidden_categs.GetCount(); i++)
    {
        if (subcategID != -1 && m_hidden_categs[i] == sIndex)
            m_hidden_categs.RemoveAt(i, i);
        else if (subcategID == -1 && m_hidden_categs[i].Contains(wxString::Format("*%i:", categID)))
            m_hidden_categs.RemoveAt(i, i);
        else
            sSettings << m_hidden_categs[i] << ";";
    }
    sIndex.RemoveLast(1);

    Model_Infotable::instance().Set("HIDDEN_CATEGS_ID", sSettings);

    m_treeCtrl->SelectItem(PreviousItem);
    selectedItemId_ = PreviousItem;
}

void mmCategDialog::OnBSelect(wxCommandEvent& WXUNUSED(event))
{
    if (selectedItemId_ != root_ && selectedItemId_)
        EndModal(wxID_OK);
}

void mmCategDialog::OnDoubleClicked(wxTreeEvent& WXUNUSED(event))
{
    if (selectedItemId_ != root_ && selectedItemId_ && m_enable_select)
    {
        mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>
            (m_treeCtrl->GetItemData(selectedItemId_));
        m_categ_id = iData->getCategData()->CATEGID;
        m_subcateg_id = iData->getSubCategData()->SUBCATEGID;
        EndModal(wxID_OK);
    }
}

void mmCategDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void mmCategDialog::OnSelChanged(wxTreeEvent& event)
{
    wxTreeItemId selectedItemId = selectedItemId_;
    selectedItemId_ = event.GetItem();

    if (!selectedItemId_) return;
    if (selectedItemId != selectedItemId_) m_treeCtrl->SelectItem(selectedItemId_);

    mmTreeItemCateg* iData =
        dynamic_cast<mmTreeItemCateg*>(m_treeCtrl->GetItemData(selectedItemId_));
    if (!iData) return;

    m_categ_id = iData->getCategData()->CATEGID;
    m_subcateg_id = iData->getSubCategData()->SUBCATEGID;

    if (selectedItemId_ == root_)
    {
        m_buttonSelect->Disable();
    }
    else
    {
        m_buttonSelect->Enable(m_enable_select);
        bool bUsed = Model_Category::is_used(m_categ_id, m_subcateg_id);
        if (m_subcateg_id == -1)
        {
            Model_Category::Data *category = Model_Category::instance().get(m_categ_id);
            const auto &subcategories = Model_Category::sub_category(category);
            for (const auto &s : subcategories)
                bUsed = (bUsed || Model_Category::is_used(m_categ_id, s.SUBCATEGID));
        }

        m_buttonDelete->SetForegroundColour(!bUsed && !m_treeCtrl->ItemHasChildren(selectedItemId_) ? wxNullColour : wxColor(*wxRED));
    }
    m_buttonAdd->Enable(m_subcateg_id == -1);
    m_buttonEdit->Enable(selectedItemId_ != root_);
}

void mmCategDialog::OnEdit(wxCommandEvent& WXUNUSED(event))
{
    if (selectedItemId_ == root_ || !selectedItemId_)
        return;

    const wxString old_name = m_treeCtrl->GetItemText(selectedItemId_);
    const wxString msg = wxString::Format(_("Enter a new name for %s"), old_name);
    const wxString text = wxGetTextFromUser(msg
        , _("Edit Category"), old_name);
    if (text.IsEmpty() || old_name == text)
        return;

    mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>
        (m_treeCtrl->GetItemData(selectedItemId_));

    if (iData->getSubCategData()->SUBCATEGID == -1) // not subcateg
    {
        Model_Category::Data_Set categories = Model_Category::instance().find(Model_Category::CATEGNAME(text));
        if (!categories.empty() && (old_name == text))
        {
            wxString errMsg = _("Category with same name exists");
            wxMessageBox(errMsg, _("Organise Categories: Editing Error"), wxOK | wxICON_ERROR);
            return;
        }
        Model_Category::Data* category = iData->getCategData();
        category->CATEGNAME = text;
        Model_Category::instance().save(category);
        mmWebApp::MMEX_WebApp_UpdateCategory();
    }
    else
    {
        Model_Category::Data* category = iData->getCategData();
        const auto &subcategories = Model_Category::sub_category(category);
        for (const auto &entry : subcategories)
        {
            if (entry.SUBCATEGNAME == text)
            {
                wxString errMsg = _("Sub Category with same name exists");
                wxMessageBox(errMsg, _("Organise Categories: Editing Error"), wxOK | wxICON_ERROR);
                return;
            }
        }
        Model_Subcategory::Data* sub_category = iData->getSubCategData();
        sub_category->SUBCATEGNAME = text;
        Model_Subcategory::instance().save(sub_category);
        mmWebApp::MMEX_WebApp_UpdateCategory();
    }

    m_treeCtrl->SetItemText(selectedItemId_, text);

    m_refresh_requested = true;
}

wxTreeItemId mmCategDialog::getTreeItemFor(const wxTreeItemId& itemID, const wxString& itemText)
{
    wxTreeItemIdValue treeDummyValue;

    bool searching = true;
    wxTreeItemId catID = m_treeCtrl->GetFirstChild(itemID, treeDummyValue);
    while (catID.IsOk() && searching)
    {
        if (itemText == m_treeCtrl->GetItemText(catID))
            searching = false;
        else
            catID = m_treeCtrl->GetNextChild(itemID, treeDummyValue);
    }
    return catID;
}

void mmCategDialog::setTreeSelection(int category_id, int subcategory_id)
{
    const auto &categories = Model_Category::instance().find(Model_Category::CATEGID(category_id));
    if (!categories.empty())
    {
        Model_Category::Data *category = Model_Category::instance().get(category_id);
        Model_Subcategory::Data *subcategory = (subcategory_id != -1 ? Model_Subcategory::instance().get(subcategory_id) : 0);
        wxString categoryName = "", subCategoryName = "";
        if (category)
            categoryName = category->CATEGNAME;

        if (subcategory)
            subCategoryName = subcategory->SUBCATEGNAME;

        setTreeSelection(categoryName, subCategoryName);
        m_categ_id = category_id;
        m_subcateg_id = subcategory_id;
    }
}

void mmCategDialog::setTreeSelection(const wxString& catName, const wxString& subCatName)
{
    if (!catName.IsEmpty())
    {
        wxTreeItemId catID = getTreeItemFor(m_treeCtrl->GetRootItem(), catName);
        if (catID.IsOk())
        {
            m_treeCtrl->SelectItem(catID);
            if (!subCatName.IsEmpty() && m_treeCtrl->ItemHasChildren(catID))
            {
                m_treeCtrl->ExpandAllChildren(catID);
                wxTreeItemId subCatID = getTreeItemFor(catID, subCatName);
                if (subCatID.IsOk())
                    m_treeCtrl->SelectItem(subCatID);
            }
        }
    }
}

void mmCategDialog::OnCategoryRelocation(wxCommandEvent& WXUNUSED(event))
{
    relocateCategoryDialog dlg(this, m_categ_id, m_subcateg_id);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msgStr;
        msgStr << _("Category Relocation Completed.") << "\n\n"
            << wxString::Format(_("Records have been updated in the database: %i"),
            dlg.updatedCategoriesCount());
        wxMessageBox(msgStr, _("Category Relocation Result"));
        m_refresh_requested = true;
        fillControls();
    }
}

void mmCategDialog::OnExpandChbClick(wxCommandEvent& WXUNUSED(event))
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

void mmCategDialog::OnShowHiddenChbClick(wxCommandEvent& WXUNUSED(event))
{
    Model_Setting::instance().Set("SHOW_HIDDEN_CATEGS", m_cbShowAll->IsChecked());
    fillControls();
}

void mmCategDialog::OnMenuSelected(wxCommandEvent& event)
{
    int id = event.GetId();

    const wxString index = wxString::Format("*%i:%i*", m_categ_id, m_subcateg_id);
    if (id == MENU_ITEM_HIDE)
    {
        m_treeCtrl->SetItemTextColour(selectedItemId_, wxColour("GREY"));
        if (m_hidden_categs.Index(index) == wxNOT_FOUND)
            m_hidden_categs.Add(index);
    }
    else if (id == MENU_ITEM_UNHIDE)
    {
        m_treeCtrl->SetItemTextColour(selectedItemId_, NormalColor_);
        m_hidden_categs.Remove(index);
    }
    else if (id == MENU_ITEM_CLEAR)
    {
        m_hidden_categs.Clear();
    }

    wxString sSettings = "";
    for (const auto& item : m_hidden_categs)
    {
        sSettings.Append(item + ";");
    }
    sSettings.RemoveLast(1);

    Model_Infotable::instance().Set("HIDDEN_CATEGS_ID", sSettings);

    fillControls();
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
    if (subCategId != -1)
    {
        const wxString cat_index = wxString::Format("*%i:%i*", categId, -1);
        if (m_hidden_categs.Index(cat_index) != wxNOT_FOUND)
            return false;
    }

    const wxString index = wxString::Format("*%i:%i*", categId, subCategId);
    if (m_hidden_categs.Index(index) != wxNOT_FOUND)
        return false;
    return true;
}

wxString mmCategDialog::getFullCategName()
{
    Model_Category::Data *category = Model_Category::instance().get(m_categ_id);
    Model_Subcategory::Data *subcategory = (m_subcateg_id != -1 ? Model_Subcategory::instance().get(m_subcateg_id) : 0);
    return Model_Category::full_name(category, subcategory);
}
