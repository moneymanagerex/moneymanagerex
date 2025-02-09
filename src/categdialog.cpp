/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015, 2016, 2020, 2022 Nikolay Akimov
 Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)

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
#include "attachmentdialog.h"
#include "images_list.h"
#include "relocatecategorydialog.h"
#include "util.h"
#include "option.h"
#include "paths.h"
#include "constants.h"
#include "webapp.h"
#include "model/Model_Setting.h"
#include "model/Model_Infotable.h"
#include "model/Model_Payee.h"
#include "model/Model_CustomFieldData.h"

wxIMPLEMENT_DYNAMIC_CLASS(mmCategDialog, wxDialog);
wxIMPLEMENT_DYNAMIC_CLASS(mmCategDialogTreeCtrl, wxTreeCtrl);

wxBEGIN_EVENT_TABLE(mmCategDialog, wxDialog)
EVT_BUTTON(wxID_OK, mmCategDialog::OnBSelect)
EVT_BUTTON(wxID_CANCEL, mmCategDialog::OnCancel)
EVT_BUTTON(wxID_ADD, mmCategDialog::OnAdd)
EVT_BUTTON(wxID_REMOVE, mmCategDialog::OnDelete)
EVT_BUTTON(wxID_EDIT, mmCategDialog::OnEdit)
EVT_TEXT(wxID_FIND, mmCategDialog::OnTextChanged)
EVT_TREE_SEL_CHANGED(wxID_ANY, mmCategDialog::OnSelChanged)
EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY, mmCategDialog::OnSelChanged)
EVT_TREE_ITEM_ACTIVATED(wxID_ANY, mmCategDialog::OnDoubleClicked)
EVT_TREE_ITEM_MENU(wxID_ANY, mmCategDialog::OnItemRightClick)
EVT_TREE_ITEM_COLLAPSED(wxID_ANY, mmCategDialog::OnItemCollapseOrExpand)
EVT_TREE_ITEM_EXPANDED(wxID_ANY, mmCategDialog::OnItemCollapseOrExpand)
EVT_TREE_BEGIN_DRAG(wxID_ANY, mmCategDialog::OnBeginDrag)
EVT_TREE_END_DRAG(wxID_ANY, mmCategDialog::OnEndDrag)
EVT_MENU(wxID_ANY, mmCategDialog::OnMenuSelected)
wxEND_EVENT_TABLE()

mmCategDialogTreeCtrl::mmCategDialogTreeCtrl(wxWindow *parent, const wxWindowID id,
    const wxPoint& pos, const wxSize& size,
    long style)
    : wxTreeCtrl(parent, id, pos, size, style)
{
}

// Only need to override the OnCompareItems sort method to make it case insensitive, locale
int mmCategDialogTreeCtrl::OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2)
{
    return CaseInsensitiveLocaleCmp(GetItemText(item1).Lower(),GetItemText(item2).Lower());
}

mmCategDialog::~mmCategDialog()
{
    Model_Infotable::instance().setSize("CATEGORIES_DIALOG_SIZE", GetSize());
}

mmCategDialog::mmCategDialog()
{
    // Initialize fields in constructor
    m_categ_id = -1;
    m_init_selected_categ_id = -1;
    m_selectedItemId = 0;
    m_IsSelection = false;
    m_refresh_requested = false;
}

mmCategDialog::mmCategDialog(wxWindow* parent
    , bool bIsSelection
    , int64 category_id)
{
    // Initialize fields in constructor
    m_categ_id = category_id;
    m_init_selected_categ_id = category_id;
    m_selectedItemId = 0;
    m_IsSelection = bIsSelection;
    m_refresh_requested = false;

    this->SetFont(parent->GetFont());
    Create(parent);
    const wxAcceleratorEntry entries[] =
    {
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F2, wxID_EDIT),
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_INSERT, wxID_ADD),
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE)
    };

    wxAcceleratorTable tab(sizeof(entries) / sizeof(*entries), entries);
    SetAcceleratorTable(tab);
}

bool mmCategDialog::Create(wxWindow* parent, wxWindowID id
    , const wxString& caption, const wxString& name
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    SetEvtHandlerEnabled(false);
    CreateControls();
    fillControls();

    m_treeCtrl->CollapseAll();
    bool expand_categs_tree = Model_Setting::instance().getBool("EXPAND_CATEGS_TREE", false);
    if (expand_categs_tree)
        m_treeCtrl->ExpandAll();
    else
        m_treeCtrl->CollapseAll();
    m_treeCtrl->Expand(root_);
    m_tbExpand->SetValue(expand_categs_tree);
    m_tbCollapse->SetValue(!expand_categs_tree);
    m_tbShowAll->SetValue(Model_Setting::instance().getBool("SHOW_HIDDEN_CATEGS", true));
    saveCurrentCollapseState();

    m_maskTextCtrl->SetFocus();
    SetEvtHandlerEnabled(true);

    mmSetSize(this);
    Centre();
    SetIcon(mmex::getProgramIcon());

    m_treeCtrl->EnsureVisible(m_selectedItemId);
    m_treeCtrl->SelectItem(m_selectedItemId);
    if (m_init_selected_categ_id != -1) m_treeCtrl->SetFocus();
    return true;
}

void mmCategDialog::saveCurrentCollapseState()
{
    wxTreeItemIdValue treeDummyValue;

    // Determine which categories were collapsed prior to being called, so we can retain state
    wxTreeItemId id = m_treeCtrl->GetFirstChild(m_treeCtrl->GetRootItem(), treeDummyValue);

    m_categoryVisible.clear();
    while (id.IsOk())
    {
        mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>(m_treeCtrl->GetItemData(id));
        if (iData)
            m_categoryVisible[iData->getCategData()->CATEGID] = m_treeCtrl->IsExpanded(id);
        id = m_treeCtrl->GetNextSibling(id);
    }
}

bool mmCategDialog::AppendSubcategoryItems(wxTreeItemId parent, const Model_Category::Data* category) {
    bool show_hidden_categs = m_tbShowAll->GetValue();
    bool catDisplayed = false;
    for (auto& subcat : m_categ_children[category->CATEGID]) {
        // Check if the subcategory should be shown
        bool subcatDisplayed = (show_hidden_categs || subcat.ACTIVE.GetValue() || subcat.CATEGID == m_init_selected_categ_id) && Model_Category::full_name(subcat.CATEGID).Lower().Matches(m_maskStr + "*");
        // Append it to get the item ID
        wxTreeItemId newId = m_treeCtrl->AppendItem(parent, subcat.CATEGNAME);
        // Check if any subcategories are not filtered out
        subcatDisplayed |= AppendSubcategoryItems(newId, &subcat);
        if (subcatDisplayed)
        {
            m_treeCtrl->SetItemData(newId, new mmTreeItemCateg(subcat));
            if (!categShowStatus(subcat.CATEGID)) m_treeCtrl->SetItemTextColour(newId, wxColour("GREY"));
            if (m_categ_id == subcat.CATEGID) m_selectedItemId = newId;
        }
        // otherwise the subcategory and all descendants are filtered out, so delete the item
        else m_treeCtrl->Delete(newId);
        catDisplayed |= subcatDisplayed;
    }
    return catDisplayed;
}

void mmCategDialog::fillControls()
{
    m_processExpandCollapse = false;
    m_treeCtrl->DeleteAllItems();
    root_ = m_treeCtrl->AddRoot(_("Categories"));
    m_selectedItemId = root_;
    m_treeCtrl->SetItemBold(root_, true);
    m_treeCtrl->SetFocus();
    NormalColor_ = m_treeCtrl->GetItemTextColour(root_);
    bool show_hidden_categs = Model_Setting::instance().getBool("SHOW_HIDDEN_CATEGS", true);

    const wxString match = m_maskStr + "*";
    wxTreeItemId maincat = root_;
    m_categ_children.clear();
    for (Model_Category::Data cat : Model_Category::instance().all(Model_Category::COL_CATEGNAME)) {
        m_categ_children[cat.PARENTID].push_back(cat);
    }

    for (auto& category : m_categ_children[-1])
    {
        bool cat_bShow = categShowStatus(category.CATEGID);
        bool catDisplayed = (show_hidden_categs || cat_bShow || category.CATEGID == m_init_selected_categ_id) && Model_Category::full_name(category.CATEGID).Lower().Matches(match);

        // Append top level category to root_ to get the item ID
        maincat = m_treeCtrl->AppendItem(root_, category.CATEGNAME);
        // If the category has any subcategories, append them
        catDisplayed |= AppendSubcategoryItems(maincat, &category);
        // If the main category or any subcategory are shown
        if (catDisplayed) {
            m_treeCtrl->SetItemData(maincat, new mmTreeItemCateg(category));
            if (!cat_bShow)
                m_treeCtrl->SetItemTextColour(maincat, wxColour("GREY"));
            if (m_categ_id == category.CATEGID)
                m_selectedItemId = maincat;
            if (m_maskStr.IsEmpty() && (m_categoryVisible.count(category.CATEGID) > 0) && !m_categoryVisible.at(category.CATEGID))
                m_treeCtrl->CollapseAllChildren(maincat);
            else m_treeCtrl->ExpandAllChildren(maincat);
        }
        // otherwise the category and all descendants are filtered out, so delete the item
        else m_treeCtrl->Delete(maincat);        
    }

    m_treeCtrl->SortChildren(root_);
    m_treeCtrl->Expand(root_);

    if (m_maskStr.IsEmpty())
        saveCurrentCollapseState();

    m_treeCtrl->SelectItem(m_selectedItemId);
    m_treeCtrl->EnsureVisible(m_selectedItemId);

    m_buttonSelect->Disable();
    if (!m_IsSelection)
        m_buttonSelect->Hide();
    m_buttonEdit->Disable();
    m_buttonAdd->Enable();
    m_buttonRelocate->Enable(!m_IsSelection);

    setTreeSelection(m_categ_id);
    m_processExpandCollapse = true;
}

void mmCategDialog::CreateControls()
{
    wxBoxSizer* mainSizerVertical = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    mainSizerVertical->Add(itemBoxSizer3, g_flagsExpand);
    wxStdDialogButtonSizer* itemBoxSizer33 = new wxStdDialogButtonSizer;
    itemBoxSizer3->Add(itemBoxSizer33, wxSizerFlags(g_flagsV).Border(wxALL, 0).Center());

    m_buttonRelocate = new wxBitmapButton(this
        , wxID_REPLACE_ALL, mmBitmapBundle(png::RELOCATION, mmBitmapButtonSize));
    m_buttonRelocate->Connect(wxID_REPLACE_ALL, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmCategDialog::OnCategoryRelocation), nullptr, this);
    mmToolTip(m_buttonRelocate, _("Merge Categories"));

    m_tbCollapse = new wxToggleButton(this, ID_COLLAPSE, _("C&ollapse All"), wxDefaultPosition
        , wxDefaultSize);
    m_tbCollapse->Connect(wxID_ANY, wxEVT_TOGGLEBUTTON,
        wxCommandEventHandler(mmCategDialog::OnExpandOrCollapseToggle), nullptr, this);

    m_tbExpand = new wxToggleButton(this, ID_EXPAND, _("E&xpand All"), wxDefaultPosition
        , wxDefaultSize);
    m_tbExpand->Connect(wxID_ANY, wxEVT_TOGGLEBUTTON,
        wxCommandEventHandler(mmCategDialog::OnExpandOrCollapseToggle), nullptr, this);

    m_tbShowAll = new wxToggleButton(this, wxID_SELECTALL, _("&Show All"), wxDefaultPosition
        , wxDefaultSize);
    mmToolTip(m_tbShowAll, _("Show all hidden categories"));
    m_tbShowAll->Connect(wxID_SELECTALL, wxEVT_TOGGLEBUTTON
        , wxCommandEventHandler(mmCategDialog::OnShowHiddenToggle), nullptr, this);

    itemBoxSizer33->Add(m_buttonRelocate, g_flagsH);
    itemBoxSizer33->AddSpacer(10);
    itemBoxSizer33->Add(m_tbCollapse, g_flagsH);
    itemBoxSizer33->AddSpacer(10);
    itemBoxSizer33->Add(m_tbExpand, g_flagsH);
    itemBoxSizer33->AddSpacer(10);
    itemBoxSizer33->Add(m_tbShowAll, g_flagsH);

#if defined (__WXGTK__) || defined (__WXMAC__)
    m_treeCtrl = new mmCategDialogTreeCtrl(this, wxID_ANY,
        wxDefaultPosition, wxSize(200, 380));
#else
    m_treeCtrl = new mmCategDialogTreeCtrl(this, wxID_ANY
        , wxDefaultPosition, wxSize(200, 380)
        , wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_ROW_LINES);
#endif
    mmThemeMetaColour(m_treeCtrl, meta::COLOR_NAVPANEL);
    mmThemeMetaColour(m_treeCtrl, meta::COLOR_NAVPANEL_FONT, true);
    itemBoxSizer3->Add(m_treeCtrl, g_flagsExpand);

    wxPanel* searchPanel = new wxPanel(this, wxID_ANY);
    mainSizerVertical->Add(searchPanel, wxSizerFlags(g_flagsExpand).Proportion(0));
    wxBoxSizer* search_sizer = new wxBoxSizer(wxHORIZONTAL);
    searchPanel->SetSizer(search_sizer);

    m_maskTextCtrl = new wxSearchCtrl(searchPanel, wxID_FIND);
    search_sizer->Add(new wxStaticText(searchPanel, wxID_STATIC, _("Search")), g_flagsH);
    search_sizer->Add(m_maskTextCtrl, g_flagsExpand);

    wxPanel* buttonsPanel = new wxPanel(this, wxID_ANY);
    mainSizerVertical->Add(buttonsPanel, wxSizerFlags(g_flagsV).Center());
    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxVERTICAL);
    buttonsPanel->SetSizer(buttonsSizer);

    wxStdDialogButtonSizer* itemBoxSizer66 = new wxStdDialogButtonSizer;
    buttonsSizer->Add(itemBoxSizer66, wxSizerFlags(g_flagsV).Border(wxALL, 0).Center());

    m_buttonAdd = new wxButton(buttonsPanel, wxID_ADD, _("&Add "));
    itemBoxSizer66->Add(m_buttonAdd, g_flagsH);
    mmToolTip(m_buttonAdd, _("Add a new category"));

    m_buttonEdit = new wxButton(buttonsPanel, wxID_EDIT, _("&Edit "));
    itemBoxSizer66->Add(m_buttonEdit, g_flagsH);
    mmToolTip(m_buttonEdit, _("Edit the name of an existing category"));

    m_buttonDelete = new wxButton(buttonsPanel, wxID_REMOVE, _("&Delete "));
    itemBoxSizer66->Add(m_buttonDelete, g_flagsH);
    mmToolTip(m_buttonDelete, _("Delete an existing category. The category is unable to be used by existing transactions."));

    wxStdDialogButtonSizer* itemBoxSizer9 = new wxStdDialogButtonSizer;
    buttonsSizer->Add(itemBoxSizer9, wxSizerFlags(g_flagsV).Border(wxALL, 0).Center());

    m_buttonSelect = new wxButton(buttonsPanel, wxID_OK, _("&Select"));
    itemBoxSizer9->Add(m_buttonSelect, g_flagsH);
    mmToolTip(m_buttonSelect, _("Select the currently selected category as the selected category for the transaction"));

    //Some interfaces has no any close buttons, it may confuse user. Cancel button added
    wxButton* itemCancelButton = new wxButton(buttonsPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    itemBoxSizer9->Add(itemCancelButton, g_flagsH);

    this->SetSizerAndFit(mainSizerVertical);
}

bool mmCategDialog::validateName(wxString name)
{
    if (wxNOT_FOUND != name.Find(':'))
    {
        wxString errMsg = _("Name contains category delimiter.");
        errMsg << "\n\n" << _("The colon (:) character is used to separate categories and subcategories"
            " and therefore should not be used in the name");
        wxMessageBox(errMsg, _("Category Manager: Invalid Name"), wxOK | wxICON_ERROR);
        return false;
    }
    return true;
}

void mmCategDialog::OnAdd(wxCommandEvent& /*event*/)
{
    wxString prompt_msg = _("Enter the name for the new category:");
    const wxString& text = wxGetTextFromUser(prompt_msg, _("Add Category"), "");
    if (text.IsEmpty() || !validateName(text))
        return;
    Model_Category::Data* selectedCategory = dynamic_cast<mmTreeItemCateg*>(m_treeCtrl->GetItemData(m_selectedItemId))->getCategData();
    Model_Category::Data* category = Model_Category::instance().create();
    category->CATEGNAME = text;
    category->ACTIVE = 1;
    if (m_selectedItemId == root_) {
        const auto& categories = Model_Category::instance().find(Model_Category::CATEGNAME(text), Model_Category::PARENTID(-1));
        if (!categories.empty())
        {
            wxMessageBox(_("A category with this name already exists for the parent"), _("Category Manager: Adding Error"), wxOK | wxICON_ERROR);
            return;
        }
        category->PARENTID = -1;
    }
    else {
        const auto& categories = Model_Category::instance().find(Model_Category::CATEGNAME(text), Model_Category::PARENTID(selectedCategory->CATEGID));
        if (!categories.empty())
        {
            wxMessageBox(_("A category with this name already exists for the parent"), _("Category Manager: Adding Error"), wxOK | wxICON_ERROR);
            return;
        }
        category->PARENTID = selectedCategory->CATEGID;
    }

    Model_Category::instance().save(category);
    mmWebApp::MMEX_WebApp_UpdateCategory();

    wxTreeItemId tid = m_treeCtrl->AppendItem(m_selectedItemId, text);
    m_treeCtrl->SetItemData(tid, new mmTreeItemCateg(*category));
    m_treeCtrl->SortChildren(m_selectedItemId);
    m_treeCtrl->Expand(m_selectedItemId);
    m_treeCtrl->SelectItem(tid);
    m_treeCtrl->SetFocus();
    m_refresh_requested = true;
    m_categ_id = category->CATEGID;
    return;
}

// Handle Categories Drag/Drop

void mmCategDialog::OnBeginDrag(wxTreeEvent& event)
{
    auto sourceItem = event.GetItem();
    if (sourceItem == m_treeCtrl->GetRootItem())
        return;

    mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>
        (m_treeCtrl->GetItemData(sourceItem));
    m_dragSourceCATEGID = iData->getCategData()->CATEGID;
    event.Allow();
}

void mmCategDialog::OnEndDrag(wxTreeEvent& event)
{
    auto destItem = event.GetItem();
    int64 categID = -1;

    if (destItem != root_) {
        Model_Category::Data* newParent = dynamic_cast<mmTreeItemCateg*>(m_treeCtrl->GetItemData(destItem))->getCategData();
        if(newParent) categID = newParent->CATEGID;
    }

    if (categID == -1 || categID == m_dragSourceCATEGID) return;

    Model_Category::Data* sourceCat = Model_Category::instance().get(m_dragSourceCATEGID);

    if (categID == sourceCat->PARENTID) return;

    if (!Model_Category::instance().find(Model_Category::CATEGNAME(sourceCat->CATEGNAME), Model_Category::PARENTID(categID)).empty() && sourceCat->PARENTID != categID)
    {
        wxMessageBox(_("Unable to move a subcategory to a category that already has a subcategory with that name. Consider renaming before moving.")
            , _("A subcategory with this name already exists")
            , wxOK | wxICON_ERROR);
        return;
    }

    wxString subtree_root;
    for (const auto& subcat : Model_Category::sub_tree(sourceCat))
    {
        if (subcat.PARENTID == sourceCat->CATEGID) subtree_root = subcat.CATEGNAME;
        if (subcat.CATEGID == categID)
        {
            wxMessageBox(wxString::Format("Unable to move a category to one of its own descendants.\n\nConsider first relocating subcategory %s to move the subtree.", subtree_root)
                , _("Target category is a descendant")
                , wxOK | wxICON_ERROR);
            return;
        }
    }

    wxString moveMessage = wxString::Format(
        _u("Do you want to to move\n“%1$s”\nto:\n“%2$s”?")
        , Model_Category::full_name(m_dragSourceCATEGID)
        , categID != -1 ? Model_Category::full_name(categID) : _("Top level"));
    wxMessageDialog msgDlg(this, moveMessage, _("Confirm Move"),
        wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);
    if (msgDlg.ShowModal() != wxID_YES)
        return;

    sourceCat->PARENTID = categID;
    Model_Category::instance().save(sourceCat);
    
    m_refresh_requested = true;
    m_categ_id = categID;
    fillControls();
}

//

void mmCategDialog::showCategDialogDeleteError(bool category)
{
    wxString deleteCategoryErrMsg = category ? _("Category in use.") : _("Subcategory in use.");
    if (category)
        deleteCategoryErrMsg << "\n\n" << _("Tip: Change all transactions using this Category to\n"
            "another Category using the merge command:");
    else
        deleteCategoryErrMsg << "\n\n" << _("Tip: Change all transactions using this Subcategory to\n"
            "another Category using the merge command:");

    deleteCategoryErrMsg << "\n\n" << _u("Tools → Merge → Categories");

    wxMessageBox(deleteCategoryErrMsg, _("Category Manager: Delete Error"), wxOK | wxICON_ERROR);
}

void mmCategDialog::mmDoDeleteSelectedCategory()
{
    wxTreeItemId PreviousItem = m_treeCtrl->GetPrevVisible(m_selectedItemId);
    Model_Checking::Data_Set deletedTrans;
    Model_Splittransaction::Data_Set splits;
    if (Model_Category::is_used(m_categ_id) || m_categ_id == m_init_selected_categ_id)
        return showCategDialogDeleteError();
    else {
        deletedTrans = Model_Checking::instance().find(Model_Checking::CATEGID(m_categ_id));
        for (const auto& subcat : Model_Category::sub_tree(Model_Category::instance().get(m_categ_id))) {
            Model_Checking::Data_Set trans = Model_Checking::instance().find(Model_Checking::CATEGID(subcat.CATEGID));
            deletedTrans.insert(deletedTrans.end(), trans.begin(), trans.end());
        }
        splits = Model_Splittransaction::instance().find(Model_Splittransaction::CATEGID(m_categ_id));
        for (const auto& subcat : Model_Category::sub_tree(Model_Category::instance().get(m_categ_id))) {
            Model_Splittransaction::Data_Set trans = Model_Splittransaction::instance().find(Model_Splittransaction::CATEGID(subcat.CATEGID));
            splits.insert(splits.end(), trans.begin(), trans.end());
        }
    }

    wxMessageDialog msgDlg(this, _("Deleted transactions exist which use this category or one of its descendants.")
            + "\n\n" + _("Deleting the category will also automatically purge the associated deleted transactions.")
            + "\n\n" + _("Do you want to continue?")
        , _("Confirm Category Deletion"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
    if ((deletedTrans.empty() && splits.empty()) || msgDlg.ShowModal() == wxID_YES)
    {
        if(!(deletedTrans.empty() && splits.empty())){
            Model_Checking::instance().Savepoint();
            Model_Splittransaction::instance().Savepoint();
            Model_Attachment::instance().Savepoint();
            Model_CustomFieldData::instance().Savepoint();
            const wxString& RefType = Model_Attachment::REFTYPE_STR_TRANSACTION;
            for (auto& split : splits) {
                Model_Checking::instance().remove(split.TRANSID);
                mmAttachmentManage::DeleteAllAttachments(RefType, split.TRANSID);
                Model_CustomFieldData::DeleteAllData(RefType, split.TRANSID);
            }

            for (auto& tran : deletedTrans) {
                Model_Checking::instance().remove(tran.TRANSID);
                mmAttachmentManage::DeleteAllAttachments(RefType, tran.TRANSID);
                Model_CustomFieldData::DeleteAllData(RefType, tran.TRANSID);
            }
            
            Model_Checking::instance().ReleaseSavepoint();
            Model_Splittransaction::instance().ReleaseSavepoint();
            Model_Attachment::instance().ReleaseSavepoint();
            Model_CustomFieldData::instance().ReleaseSavepoint();
        }

        for (auto& subcat : Model_Category::sub_tree(Model_Category::instance().get(m_categ_id)))
            Model_Category::instance().remove(subcat.CATEGID);

        Model_Category::instance().remove(m_categ_id);
    }
    else return;

    m_refresh_requested = true;
    m_treeCtrl->Delete(m_selectedItemId);

    //Clear categories associated with payees
    auto payees = Model_Payee::instance().find(Model_Payee::CATEGID(m_categ_id));
    for (auto& payee : payees)
    {
        payee.CATEGID = -1;
    }
    Model_Payee::instance().save(payees);
    mmWebApp::MMEX_WebApp_UpdatePayee();

    m_treeCtrl->SelectItem(PreviousItem);
    m_selectedItemId = PreviousItem;
}

void mmCategDialog::OnDelete(wxCommandEvent& /*event*/)
{
    if (m_selectedItemId && m_selectedItemId != root_) {
        mmDoDeleteSelectedCategory();
    }
}

void mmCategDialog::OnBSelect(wxCommandEvent& /*event*/)
{
    if (m_selectedItemId != root_ && m_selectedItemId)
        EndModal(wxID_OK);
}

void mmCategDialog::OnDoubleClicked(wxTreeEvent& /*event*/)
{
    if (m_selectedItemId != root_ && m_selectedItemId && m_IsSelection)
    {
        mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>
            (m_treeCtrl->GetItemData(m_selectedItemId));
        m_categ_id = iData->getCategData()->CATEGID;
        EndModal(wxID_OK);
    }
}

void mmCategDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmCategDialog::OnSelChanged(wxTreeEvent& event)
{
    wxTreeItemId selectedItemId = m_selectedItemId;
    m_selectedItemId = event.GetItem();

    if (!m_selectedItemId) return;
    if (selectedItemId != m_selectedItemId) m_treeCtrl->SelectItem(m_selectedItemId);

    m_categ_id = -1;
    const bool bRootSelected = m_selectedItemId == root_;
    mmTreeItemCateg* iData =
        dynamic_cast<mmTreeItemCateg*>(m_treeCtrl->GetItemData(m_selectedItemId));
    if (!bRootSelected && iData)
    {
        m_categ_id = iData->getCategData()->CATEGID;

        m_buttonDelete->Enable(!mmIsUsed());
        m_buttonSelect->Enable(m_IsSelection && !bRootSelected && !Model_Category::is_hidden(m_categ_id));
    }

    m_buttonAdd->Enable(true);
    m_buttonEdit->Enable(!bRootSelected);
}

void mmCategDialog::OnEdit(wxCommandEvent& /*event*/)
{
    if (m_selectedItemId == root_ || !m_selectedItemId)
        return;

    const wxString old_name = m_treeCtrl->GetItemText(m_selectedItemId);
    const wxString msg = wxString::Format(_("Enter a new name for '%s'"), old_name);
    wxString text = wxGetTextFromUser(msg, _("Edit Category"), old_name);
    if (text.IsEmpty() || old_name == text || !validateName(text)) {
        return;
    }

    Model_Category::Data* category = dynamic_cast<mmTreeItemCateg*>
        (m_treeCtrl->GetItemData(m_selectedItemId))->getCategData();

    Model_Category::Data_Set categories = Model_Category::instance().find(Model_Category::CATEGNAME(text), Model_Category::PARENTID(category->PARENTID));
    if (!categories.empty())
    {
        wxString errMsg = _("A category with this name already exists for the parent");
        wxMessageBox(errMsg, _("Category Manager: Editing Error"), wxOK | wxICON_ERROR);
        return;
    }
    category->CATEGNAME = text;
    Model_Category::instance().save(category);
    mmWebApp::MMEX_WebApp_UpdateCategory();

    m_treeCtrl->SetItemText(m_selectedItemId, text);

    m_refresh_requested = true;
    fillControls();
}

wxTreeItemId mmCategDialog::getTreeItemFor(const wxTreeItemId& itemID, const wxString& itemText, const int64 parentid)
{
    wxTreeItemIdValue treeDummyValue;

    bool searching = true;
    wxTreeItemId catID = m_treeCtrl->GetFirstChild(itemID, treeDummyValue);
    while (catID.IsOk() && searching)
    {
        if (itemText == m_treeCtrl->GetItemText(catID) && parentid == dynamic_cast<mmTreeItemCateg*>(m_treeCtrl->GetItemData(catID))->getCategData()->PARENTID)
            searching = false;
        else
            catID = m_treeCtrl->GetNextChild(itemID, treeDummyValue);
    }
    return catID;
}

void mmCategDialog::setTreeSelection(int64 category_id)
{
    Model_Category::Data* category = Model_Category::instance().get(category_id);
    if (category)
    {
        setTreeSelection(category->CATEGNAME, category->PARENTID);
    }
    m_categ_id = category_id;   
}

void mmCategDialog::setTreeSelection(const wxString& catName, const int64 parentid)
{
    if (!catName.IsEmpty())
    {
        wxTreeItemId catID = getTreeItemFor(m_treeCtrl->GetRootItem(), catName, parentid);
        if (catID.IsOk())
        {
            m_treeCtrl->SelectItem(catID);
        }
    }
}

void mmCategDialog::OnCategoryRelocation(wxCommandEvent& /*event*/)
{
    relocateCategoryDialog dlg(this, m_categ_id);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msgStr;
        msgStr << _("Merge categories completed") << "\n\n"
            << wxString::Format(_("Records have been updated in the database: %i"),
                dlg.updatedCategoriesCount());
        wxMessageBox(msgStr, _("Merge categories result"));
        m_refresh_requested = true;
        fillControls();
    }
}

void mmCategDialog::OnExpandOrCollapseToggle(wxCommandEvent& event)
{
    m_processExpandCollapse = false;
    if (event.GetId() == ID_EXPAND)
    {
        m_treeCtrl->ExpandAll();
        m_treeCtrl->SelectItem(m_selectedItemId);
        m_tbExpand->SetValue(true);
        m_tbCollapse->SetValue(false);
    } else
    {
        m_treeCtrl->CollapseAll();
        m_treeCtrl->Expand(root_);
        m_treeCtrl->SelectItem(m_selectedItemId);
        m_tbExpand->SetValue(false);
        m_tbCollapse->SetValue(true);
    }
    m_treeCtrl->EnsureVisible(m_selectedItemId);
    Model_Setting::instance().setBool("EXPAND_CATEGS_TREE", m_tbExpand->GetValue());
    saveCurrentCollapseState();
    m_processExpandCollapse = true;
}

void mmCategDialog::OnShowHiddenToggle(wxCommandEvent& /*event*/)
{
    Model_Setting::instance().setBool("SHOW_HIDDEN_CATEGS", m_tbShowAll->GetValue());
    fillControls();
}

void mmCategDialog::OnTextChanged(wxCommandEvent& event)
{
    m_maskStr = event.GetString();
    if (!m_maskStr.IsEmpty())
        m_maskStr = m_maskStr.Lower().Prepend("*");
    fillControls();
    m_maskTextCtrl->SetFocus();
    m_maskTextCtrl->SetInsertionPointEnd();
}

void mmCategDialog::OnMenuSelected(wxCommandEvent& event)
{
    int id = event.GetId();

    auto cat = Model_Category::instance().get(m_categ_id);
    switch (id)
    {
    case MENU_ITEM_HIDE:
    {
        m_treeCtrl->SetItemTextColour(m_selectedItemId, wxColour("GREY"));
        cat->ACTIVE = 0;
        Model_Category::instance().save(cat);
        for (auto& subcat : Model_Category::sub_tree(cat)) {
            subcat.ACTIVE = 0;
            Model_Category::instance().save(&subcat);
        }
        break;
    }
    case MENU_ITEM_UNHIDE:
    {
        m_treeCtrl->SetItemTextColour(m_selectedItemId, NormalColor_);
        cat->ACTIVE = 1;
        Model_Category::instance().save(cat);
        for (auto& subcat : Model_Category::sub_tree(cat)) {
            subcat.ACTIVE = 1;
            Model_Category::instance().save(&subcat);
        }
        break;
    }
    case MENU_ITEM_CLEAR:
    {
        wxMessageDialog msgDlg(this, _("Do you want to unhide all categories?")
                , _("Unhide all categories")
                , wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);
        if (msgDlg.ShowModal() == wxID_YES)
        {
            auto categList = Model_Category::instance().all();
            Model_Category::instance().Savepoint();
            for (auto &catItem : categList)
            {
                catItem.ACTIVE = 1;
                Model_Category::instance().save(&catItem);              
            }
            Model_Category::instance().ReleaseSavepoint();
        }
        break;
    }
    case MENU_ITEM_DELETE:
    {
        mmDoDeleteSelectedCategory();
        break;
    }
    }

    fillControls();
}

void mmCategDialog::OnItemRightClick(wxTreeEvent& event)
{
    wxMenu mainMenu;
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_ITEM_HIDE, _("Hide Selected Category")));
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_ITEM_UNHIDE, _("Unhide Selected Category")));
    mainMenu.AppendSeparator();
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_ITEM_DELETE, _("Remove Category")));

    mainMenu.AppendSeparator();
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_ITEM_CLEAR, _("Clear Settings")));
    bool bItemHidden = (m_treeCtrl->GetItemTextColour(m_selectedItemId) != NormalColor_);
    mainMenu.Enable(MENU_ITEM_HIDE, !bItemHidden && (m_selectedItemId != root_));
    mainMenu.Enable(MENU_ITEM_UNHIDE, bItemHidden && (m_selectedItemId != root_));
    mainMenu.Enable(MENU_ITEM_DELETE, !mmIsUsed());

    PopupMenu(&mainMenu, event.GetPoint());
    event.Skip();
}

void mmCategDialog::OnItemCollapseOrExpand(wxTreeEvent& event)
{
    if (m_processExpandCollapse)
    {
        m_tbCollapse->SetValue(false);
        m_tbExpand->SetValue(false);
        saveCurrentCollapseState();
    }
    event.Skip();
}

bool mmCategDialog::categShowStatus(int64 categId)
{
    if (Model_Category::is_hidden(categId))
        return false;
    
    return true;
}

wxString mmCategDialog::getFullCategName()
{
    return Model_Category::full_name(m_categ_id);
}

bool mmCategDialog::mmIsUsed() const
{
    return (Model_Category::is_used(m_categ_id) || m_categ_id == m_init_selected_categ_id);
}
