/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015, 2016, 2020, 2022 Nikolay Akimov
 Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 Klaus Wich

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

#include <set>
#include "base/constants.h"
#include "base/paths.h"
#include "base/images_list.h"
#include "util/_util.h"

#include "model/AttachmentModel.h"
#include "model/FieldValueModel.h"
#include "model/InfoModel.h"
#include "model/PayeeModel.h"
#include "model/SettingModel.h"
#include "model/PrefModel.h"

#include "CategoryManager.h"
#include "dialog/AttachmentDialog.h"
#include "dialog/MergeCategoryDialog.h"
#include "import_export/webapp.h"

wxIMPLEMENT_DYNAMIC_CLASS(CategoryManager, wxDialog);
wxIMPLEMENT_DYNAMIC_CLASS(mmCategDialogTreeCtrl, wxTreeCtrl);

wxBEGIN_EVENT_TABLE(CategoryManager, wxDialog)
    EVT_BUTTON(wxID_OK,                 CategoryManager::OnBSelect)
    EVT_BUTTON(wxID_CANCEL,             CategoryManager::OnCancel)
    EVT_BUTTON(wxID_ADD,                CategoryManager::OnAdd)
    EVT_BUTTON(wxID_REMOVE,             CategoryManager::OnDelete)
    EVT_BUTTON(wxID_EDIT,               CategoryManager::OnEdit)
    EVT_TOGGLEBUTTON(wxID_SELECTALL,    CategoryManager::OnShowHiddenToggle)
    EVT_TEXT(wxID_FIND,                 CategoryManager::OnTextChanged)
    EVT_TREE_SEL_CHANGED(wxID_ANY,      CategoryManager::OnSelChanged)
    EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY, CategoryManager::OnSelChanged)
    EVT_TREE_ITEM_ACTIVATED(wxID_ANY,   CategoryManager::OnDoubleClicked)
    EVT_TREE_ITEM_MENU(wxID_ANY,        CategoryManager::OnItemRightClick)
    EVT_TREE_ITEM_COLLAPSED(wxID_ANY,   CategoryManager::OnItemCollapseOrExpand)
    EVT_TREE_ITEM_EXPANDED(wxID_ANY,    CategoryManager::OnItemCollapseOrExpand)
    EVT_TREE_BEGIN_DRAG(wxID_ANY,       CategoryManager::OnBeginDrag)
    EVT_TREE_END_DRAG(wxID_ANY,         CategoryManager::OnEndDrag)
    EVT_MENU(wxID_ANY,                  CategoryManager::OnMenuSelected)
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

CategoryManager::~CategoryManager()
{
    InfoModel::instance().setSize("CATEGORIES_DIALOG_SIZE", GetSize());
}

CategoryManager::CategoryManager()
{
    // Initialize fields in constructor
    m_categ_id = -1;
    m_init_selected_categ_id = -1;
    m_selectedItemId = 0;
    m_IsSelection = false;
    m_refresh_requested = false;
}

CategoryManager::CategoryManager(wxWindow* parent
    , bool bIsSelection
    , int64 category_id)
{
    // Initialize fields in constructor
    m_categ_id = category_id;
    m_init_selected_categ_id = category_id;
    m_selectedItemId = 0;
    m_IsSelection = bIsSelection;
    m_refresh_requested = false;
    m_hiddenColor = mmThemeMetaColour(meta::COLOR_HIDDEN);

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

bool CategoryManager::Create(wxWindow* parent, wxWindowID id
    , const wxString& caption, const wxString& name
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    SetEvtHandlerEnabled(false);
    CreateControls();
    mmThemeAutoColour(this);
    mmThemeMetaColour(m_treeCtrl, meta::COLOR_NAVPANEL);
    mmThemeMetaColour(m_treeCtrl, meta::COLOR_NAVPANEL_FONT, true);
    fillControls();

    m_treeCtrl->CollapseAll();
    bool expand_categs_tree = SettingModel::instance().getBool("EXPAND_CATEGS_TREE", false);
    if (expand_categs_tree)
        m_treeCtrl->ExpandAll();
    else
        m_treeCtrl->CollapseAll();
    m_treeCtrl->Expand(root_);
    m_tbShowAll->SetValue(SettingModel::instance().getBool("SHOW_HIDDEN_CATEGS", true));
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

void CategoryManager::saveCurrentCollapseState()
{
    wxTreeItemIdValue treeDummyValue;

    // Determine which categories were collapsed prior to being called, so we can retain state
    wxTreeItemId id = m_treeCtrl->GetFirstChild(m_treeCtrl->GetRootItem(), treeDummyValue);

    m_categoryVisible.clear();
    while (id.IsOk())
    {
        mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>(m_treeCtrl->GetItemData(id));
        if (iData)
            m_categoryVisible[iData->getCategData()->m_id] = m_treeCtrl->IsExpanded(id);
        id = m_treeCtrl->GetNextSibling(id);
    }
}

bool CategoryManager::AppendSubcategoryItems(wxTreeItemId parent, const CategoryData* category) {
    bool show_hidden_categs = m_tbShowAll->GetValue();
    bool catDisplayed = false;
    for (auto& subcat : m_categ_children[category->m_id]) {
        // Check if the subcategory should be shown
        bool subcatDisplayed = (show_hidden_categs || subcat.m_active || subcat.m_id == m_init_selected_categ_id) && CategoryModel::instance().full_name(subcat.m_id).Lower().Matches(m_maskStr + "*");
        // Append it to get the item ID
        wxTreeItemId newId = m_treeCtrl->AppendItem(parent, subcat.m_name);
        // Check if any subcategories are not filtered out
        subcatDisplayed |= AppendSubcategoryItems(newId, &subcat);
        if (subcatDisplayed)
        {
            m_treeCtrl->SetItemData(newId, new mmTreeItemCateg(subcat));
            if (!categShowStatus(subcat.m_id)) m_treeCtrl->SetItemTextColour(newId, m_hiddenColor);
            if (m_categ_id == subcat.m_id) m_selectedItemId = newId;
        }
        // otherwise the subcategory and all descendants are filtered out, so delete the item
        else m_treeCtrl->Delete(newId);
        catDisplayed |= subcatDisplayed;
    }
    return catDisplayed;
}

void CategoryManager::fillControls()
{
    m_processExpandCollapse = false;
    m_treeCtrl->DeleteAllItems();
    root_ = m_treeCtrl->AddRoot(_t("Categories"));
    m_selectedItemId = root_;
    m_treeCtrl->SetItemBold(root_, true);
    m_treeCtrl->SetFocus();
    NormalColor_ = m_treeCtrl->GetItemTextColour(root_);
    bool show_hidden_categs = SettingModel::instance().getBool("SHOW_HIDDEN_CATEGS", true);

    const wxString match = m_maskStr + "*";
    wxTreeItemId maincat = root_;
    m_categ_children.clear();
    for (CategoryData cat : CategoryModel::instance().find_all(CategoryCol::COL_ID_CATEGNAME)) {
        m_categ_children[cat.m_parent_id_n].push_back(cat);
    }

    for (auto& category : m_categ_children[-1])
    {
        bool cat_bShow = categShowStatus(category.m_id);
        bool catDisplayed = (show_hidden_categs || cat_bShow || category.m_id == m_init_selected_categ_id) && CategoryModel::instance().full_name(category.m_id).Lower().Matches(match);

        // Append top level category to root_ to get the item ID
        maincat = m_treeCtrl->AppendItem(root_, category.m_name);
        // If the category has any subcategories, append them
        catDisplayed |= AppendSubcategoryItems(maincat, &category);
        // If the main category or any subcategory are shown
        if (catDisplayed) {
            m_treeCtrl->SetItemData(maincat, new mmTreeItemCateg(category));
            if (!cat_bShow)
                m_treeCtrl->SetItemTextColour(maincat, m_hiddenColor);
            if (m_categ_id == category.m_id)
                m_selectedItemId = maincat;
            if (m_maskStr.IsEmpty() && (m_categoryVisible.find(category.m_id) != m_categoryVisible.end()) && !m_categoryVisible.at(category.m_id))
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

void CategoryManager::CreateControls()
{
    wxBoxSizer* mainSizerVertical = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    mainSizerVertical->Add(itemBoxSizer3, g_flagsExpand);
    wxStdDialogButtonSizer* itemBoxSizer33 = new wxStdDialogButtonSizer;
    itemBoxSizer3->Add(itemBoxSizer33, wxSizerFlags(g_flagsV).Border(wxALL, 0).Center());

    m_buttonRelocate = new wxBitmapButton(this
        , wxID_REPLACE_ALL, mmBitmapBundle(png::RELOCATION, mmBitmapButtonSize));
    m_buttonRelocate->Connect(wxID_REPLACE_ALL, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(CategoryManager::OnCategoryRelocation), nullptr, this);
    mmToolTip(m_buttonRelocate, _t("Merge Categories"));

    m_tbCollapse = new wxButton(this, ID_COLLAPSE, _t("C&ollapse All"), wxDefaultPosition, wxDefaultSize);
    m_tbCollapse->Connect(ID_COLLAPSE, wxEVT_BUTTON,
        wxCommandEventHandler(CategoryManager::OnExpandOrCollapseToggle), nullptr, this);

    m_tbExpand = new wxButton(this, ID_EXPAND, _t("E&xpand All"), wxDefaultPosition, wxDefaultSize);
    m_tbExpand->Connect(ID_EXPAND, wxEVT_BUTTON,
        wxCommandEventHandler(CategoryManager::OnExpandOrCollapseToggle), nullptr, this);

    m_tbShowAll = new wxToggleButton(this, wxID_SELECTALL, _t("&Show All"), wxDefaultPosition
        , wxDefaultSize);
    mmToolTip(m_tbShowAll, _t("Show all hidden categories"));

    wxButton* clearBtn = new wxButton(this, ID_CLEAR, _t("Clear Settings"));
    mmToolTip(clearBtn, _t("Remove hidden setting from all categories"));
    clearBtn->Connect(ID_CLEAR, wxEVT_BUTTON,
        wxCommandEventHandler(CategoryManager::OnClearSettings), nullptr, this);

    itemBoxSizer33->Add(m_tbCollapse, g_flagsH);
    itemBoxSizer33->AddSpacer(5);
    itemBoxSizer33->Add(m_tbExpand, g_flagsH);
    itemBoxSizer33->AddSpacer(15);
    itemBoxSizer33->Add(m_tbShowAll, g_flagsH);
    itemBoxSizer33->AddSpacer(15);
    itemBoxSizer33->Add(m_buttonRelocate, g_flagsH);
    itemBoxSizer33->AddSpacer(10);
    itemBoxSizer33->Add(clearBtn, g_flagsH);

#if defined (__WXGTK__) || defined (__WXMAC__)
    m_treeCtrl = new mmCategDialogTreeCtrl(this, wxID_ANY,
        wxDefaultPosition, wxSize(200, 380));
#else
    m_treeCtrl = new mmCategDialogTreeCtrl(this, wxID_ANY
        , wxDefaultPosition, wxSize(200, 380)
        , wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_ROW_LINES);
#endif
    itemBoxSizer3->Add(m_treeCtrl, g_flagsExpand);

    wxPanel* searchPanel = new wxPanel(this, wxID_ANY);
    mainSizerVertical->Add(searchPanel, wxSizerFlags(g_flagsExpand).Proportion(0));
    wxBoxSizer* search_sizer = new wxBoxSizer(wxHORIZONTAL);
    searchPanel->SetSizer(search_sizer);

    m_maskTextCtrl = new wxSearchCtrl(searchPanel, wxID_FIND);
    search_sizer->Add(new wxStaticText(searchPanel, wxID_STATIC, _t("Search")), g_flagsH);
    search_sizer->Add(m_maskTextCtrl, g_flagsExpand);

    wxPanel* buttonsPanel = new wxPanel(this, wxID_ANY);
    mainSizerVertical->Add(buttonsPanel, wxSizerFlags(g_flagsV).Center());
    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxVERTICAL);
    buttonsPanel->SetSizer(buttonsSizer);

    wxStdDialogButtonSizer* itemBoxSizer66 = new wxStdDialogButtonSizer;
    buttonsSizer->Add(itemBoxSizer66, wxSizerFlags(g_flagsV).Border(wxALL, 0).Center());

    m_buttonAdd = new wxButton(buttonsPanel, wxID_ADD, _t("&New "));
    itemBoxSizer66->Add(m_buttonAdd, g_flagsH);
    mmToolTip(m_buttonAdd, _t("New category"));

    m_buttonEdit = new wxButton(buttonsPanel, wxID_EDIT, _t("&Edit "));
    itemBoxSizer66->Add(m_buttonEdit, g_flagsH);
    mmToolTip(m_buttonEdit, _t("Edit category"));

    m_buttonDelete = new wxButton(buttonsPanel, wxID_REMOVE, _t("&Delete "));
    itemBoxSizer66->Add(m_buttonDelete, g_flagsH);
    mmToolTip(m_buttonDelete, _t("Delete category. The category is unable to be used by existing transactions."));

    wxStdDialogButtonSizer* itemBoxSizer9 = new wxStdDialogButtonSizer;
    buttonsSizer->Add(itemBoxSizer9, wxSizerFlags(g_flagsV).Border(wxALL, 0).Center());

    m_buttonSelect = new wxButton(buttonsPanel, wxID_OK, _t("&Select"));
    itemBoxSizer9->Add(m_buttonSelect, g_flagsH);
    mmToolTip(m_buttonSelect, _t("Select the currently selected category as the selected category for the transaction"));

    wxButton* itemCancelButton = new wxButton(buttonsPanel, wxID_CANCEL, wxGetTranslation(g_CloseLabel));
    itemBoxSizer9->Add(itemCancelButton, g_flagsH);

    this->SetSizerAndFit(mainSizerVertical);
}

bool CategoryManager::validateName(wxString name)
{
    if (wxNOT_FOUND != name.Find(':'))
    {
        wxString errMsg = _t("Name contains category delimiter.");
        errMsg << "\n\n" << _t("The colon (:) character is used to separate categories and subcategories"
            " and therefore should not be used in the name");
        wxMessageBox(errMsg, _t("Category Manager: Invalid Name"), wxOK | wxICON_ERROR);
        return false;
    }
    return true;
}

void CategoryManager::OnAdd(wxCommandEvent& /*event*/)
{
    wxString prompt_msg = _t("Enter the name for the new category:");
    const wxString& name = wxGetTextFromUser(prompt_msg, _t("Add Category"), "");
    if (name.IsEmpty() || !validateName(name))
        return;

    CategoryData* selectedCategory = dynamic_cast<mmTreeItemCateg*>(
        m_treeCtrl->GetItemData(m_selectedItemId)
    )->getCategData();
    CategoryData new_category_d = CategoryData();
    new_category_d.m_name = name;
    if (m_selectedItemId == root_) {
        const auto& category_a = CategoryModel::instance().find(
            CategoryCol::CATEGNAME(name),
            CategoryCol::PARENTID(-1)
        );
        if (!category_a.empty()) {
            wxMessageBox(
                _t("A category with this name already exists for the parent"),
                _t("Category Manager: Adding Error"),
                wxOK | wxICON_ERROR
            );
            return;
        }
        new_category_d.m_parent_id_n = -1;
    }
    else {
        const auto& category_a = CategoryModel::instance().find(
            CategoryCol::CATEGNAME(name),
            CategoryCol::PARENTID(selectedCategory->m_id)
        );
        if (!category_a.empty()) {
            wxMessageBox(
                _t("A category with this name already exists for the parent"),
                _t("Category Manager: Adding Error"),
                wxOK | wxICON_ERROR
            );
            return;
        }
        new_category_d.m_parent_id_n = selectedCategory->m_id;
    }

    CategoryModel::instance().add_data_n(new_category_d);
    mmWebApp::MMEX_WebApp_UpdateCategory();

    wxTreeItemId tid = m_treeCtrl->AppendItem(m_selectedItemId, name);
    m_treeCtrl->SetItemData(tid, new mmTreeItemCateg(new_category_d));
    m_treeCtrl->SortChildren(m_selectedItemId);
    m_treeCtrl->Expand(m_selectedItemId);
    m_treeCtrl->SelectItem(tid);
    m_treeCtrl->SetFocus();
    m_refresh_requested = true;
    m_categ_id = new_category_d.m_id;
    return;
}

// Handle Categories Drag/Drop

void CategoryManager::OnBeginDrag(wxTreeEvent& event)
{
    auto sourceItem = event.GetItem();
    if (sourceItem == m_treeCtrl->GetRootItem())
        return;

    mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>
        (m_treeCtrl->GetItemData(sourceItem));
    m_dragSourceCATEGID = iData->getCategData()->m_id;
    event.Allow();
}

void CategoryManager::OnEndDrag(wxTreeEvent& event)
{
    wxTreeItemId destItem = event.GetItem();
    int64 cat_id = -1;

    if (destItem.IsOk() && destItem != root_) {
        CategoryData* new_parent_n = dynamic_cast<mmTreeItemCateg*>(
            m_treeCtrl->GetItemData(destItem)
        )->getCategData();
        if (new_parent_n) {
            cat_id = new_parent_n->m_id;
        }
    }

    if (cat_id == -1 || cat_id == m_dragSourceCATEGID)
        return;

    CategoryData* src_cat_n = CategoryModel::instance().unsafe_get_id_data_n(m_dragSourceCATEGID);

    if (cat_id == src_cat_n->m_parent_id_n)
        return;

    if (!CategoryModel::instance().find(
        CategoryCol::CATEGNAME(src_cat_n->m_name),
        CategoryCol::PARENTID(cat_id)
    ).empty() && src_cat_n->m_parent_id_n != cat_id) {
        wxMessageBox(
            _t("Unable to move a subcategory to a category that already has a subcategory with that name. Consider renaming before moving."),
            _t("A subcategory with this name already exists"),
            wxOK | wxICON_ERROR
        );
        return;
    }

    wxString subtree_root;
    for (const auto& sub_d : CategoryModel::instance().find_data_subtree_a(*src_cat_n)) {
        if (sub_d.m_parent_id_n == src_cat_n->m_id) subtree_root = sub_d.m_name;
        if (sub_d.m_id == cat_id) {
            wxMessageBox(
                wxString::Format(
                    "Unable to move a category to one of its own descendants.\n\nConsider first relocating subcategory %s to move the subtree.",
                    subtree_root
                ),
                _t("Target category is a descendant"),
                wxOK | wxICON_ERROR
            );
            return;
        }
    }

    wxString moveMessage = wxString::Format(
        _tu("Do you want to move\n“%1$s”\nto:\n“%2$s”?"),
        CategoryModel::instance().full_name(m_dragSourceCATEGID),
        cat_id != -1 ? CategoryModel::instance().full_name(cat_id) : _t("Top level")
    );
    wxMessageDialog msgDlg(this,
        moveMessage, _t("Confirm Move"),
        wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION
    );

    if (msgDlg.ShowModal() != wxID_YES)
        return;

    src_cat_n->m_parent_id_n = cat_id;
    CategoryModel::instance().unsafe_update_data_n(src_cat_n);

    m_refresh_requested = true;
    m_categ_id = cat_id;
    fillControls();
}

//

void CategoryManager::showCategDialogDeleteError(bool category)
{
    wxString deleteCategoryErrMsg = category ? _t("Category in use.") : _t("Subcategory in use.");
    if (category)
        deleteCategoryErrMsg << "\n\n" << _t("Tip: Change all transactions using this Category to\n"
            "another Category using the merge command:");
    else
        deleteCategoryErrMsg << "\n\n" << _t("Tip: Change all transactions using this Subcategory to\n"
            "another Category using the merge command:");

    deleteCategoryErrMsg << "\n\n" << _tu("Tools → Merge → Categories");

    wxMessageBox(deleteCategoryErrMsg, _t("Category Manager: Delete Error"), wxOK | wxICON_ERROR);
}

void CategoryManager::mmDoDeleteSelectedCategory()
{
    wxTreeItemId PreviousItem = m_treeCtrl->GetPrevVisible(m_selectedItemId);

    if (CategoryModel::instance().is_used(m_categ_id) ||
        m_categ_id == m_init_selected_categ_id
    )
        return showCategDialogDeleteError();

    std::set<int64> cat_id_m;
    cat_id_m.insert(m_categ_id);
    const CategoryData* cat_n = CategoryModel::instance().get_id_data_n(m_categ_id);
    for (const auto& sub_d : CategoryModel::instance().find_data_subtree_a(*cat_n)) {
        cat_id_m.insert(sub_d.m_id);
    }

    std::set<int64> trx_id_m;
    for (int64 cat_id : cat_id_m) {
        for (const TrxData& trx_d : TrxModel::instance().find(
            TrxCol::CATEGID(cat_id)
        )) {
            trx_id_m.insert(trx_d.m_id);
        }
        for (const TrxSplitData& tp_d : TrxSplitModel::instance().find(
            TrxSplitCol::CATEGID(cat_id)
        )) {
            trx_id_m.insert(tp_d.m_trx_id);
        }
    }

    wxMessageDialog msgDlg(this,
        _t("Deleted transactions exist which use this category or one of its descendants.") + "\n\n" +
            _t("Deleting the category will also automatically purge the associated deleted transactions.") + "\n\n" +
            _t("Do you want to continue?"),
        _t("Confirm Category Deletion"),
        wxYES_NO | wxNO_DEFAULT | wxICON_WARNING
    );
    if (!trx_id_m.empty() && msgDlg.ShowModal() != wxID_YES)
        return;

    if (!trx_id_m.empty()) {
        TrxModel::instance().db_savepoint();
        TrxSplitModel::instance().db_savepoint();
        AttachmentModel::instance().db_savepoint();
        FieldValueModel::instance().db_savepoint();

        for (int64 trx_id : trx_id_m) {
            FieldValueModel::instance().purge_ref(TrxModel::s_ref_type, trx_id);
            mmAttachmentManage::DeleteAllAttachments(TrxModel::s_ref_type, trx_id);
            TrxModel::instance().purge_id(trx_id);
        }

        FieldValueModel::instance().db_release_savepoint();
        AttachmentModel::instance().db_release_savepoint();
        TrxSplitModel::instance().db_release_savepoint();
        TrxModel::instance().db_release_savepoint();
    }

    for (auto& sub_d : CategoryModel::instance().find_data_subtree_a(*cat_n)) {
        CategoryModel::instance().purge_id(sub_d.m_id);
    }
    CategoryModel::instance().purge_id(m_categ_id);

    m_refresh_requested = true;
    m_treeCtrl->Delete(m_selectedItemId);

    // Clear categories associated with payees
    auto payee_a = PayeeModel::instance().find(PayeeCol::CATEGID(m_categ_id));
    for (auto& payee_d : payee_a) {
        payee_d.m_category_id_n = -1;
    }
    PayeeModel::instance().save_data_a(payee_a);
    mmWebApp::MMEX_WebApp_UpdatePayee();

    m_treeCtrl->SelectItem(PreviousItem);
    m_selectedItemId = PreviousItem;
}

void CategoryManager::OnDelete(wxCommandEvent& /*event*/)
{
    if (m_selectedItemId && m_selectedItemId != root_) {
        mmDoDeleteSelectedCategory();
    }
}

void CategoryManager::OnBSelect(wxCommandEvent& /*event*/)
{
    if (m_selectedItemId != root_ && m_selectedItemId)
        EndModal(wxID_OK);
}

void CategoryManager::OnDoubleClicked(wxTreeEvent& /*event*/)
{
    if (m_selectedItemId != root_ && m_selectedItemId && m_IsSelection)
    {
        mmTreeItemCateg* iData = dynamic_cast<mmTreeItemCateg*>
            (m_treeCtrl->GetItemData(m_selectedItemId));
        m_categ_id = iData->getCategData()->m_id;
        EndModal(wxID_OK);
    }
}

void CategoryManager::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void CategoryManager::OnSelChanged(wxTreeEvent& event)
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
        m_categ_id = iData->getCategData()->m_id;

        m_buttonDelete->Enable(!mmIsUsed());
        m_buttonSelect->Enable(m_IsSelection && !bRootSelected && !CategoryModel::instance().is_hidden(m_categ_id));
    }

    m_buttonAdd->Enable(true);
    m_buttonEdit->Enable(!bRootSelected);
}

void CategoryManager::OnEdit(wxCommandEvent& /*event*/)
{
    if (m_selectedItemId == root_ || !m_selectedItemId)
        return;

    const wxString old_name = m_treeCtrl->GetItemText(m_selectedItemId);
    const wxString msg = wxString::Format(_t("Enter a new name for '%s'"), old_name);
    wxString text = wxGetTextFromUser(msg, _t("Edit Category"), old_name);
    if (text.IsEmpty() || old_name == text || !validateName(text)) {
        return;
    }

    CategoryData* category = dynamic_cast<mmTreeItemCateg*>(
        m_treeCtrl->GetItemData(m_selectedItemId)
    )->getCategData();

    CategoryModel::DataA category_a = CategoryModel::instance().find(
        CategoryCol::CATEGNAME(text),
        CategoryCol::PARENTID(category->m_parent_id_n)
    );
    if (!category_a.empty()) {
        wxString errMsg = _t("A category with this name already exists for the parent");
        wxMessageBox(errMsg, _t("Category Manager: Editing Error"), wxOK | wxICON_ERROR);
        return;
    }
    category->m_name = text;
    CategoryModel::instance().unsafe_save_data_n(category);
    mmWebApp::MMEX_WebApp_UpdateCategory();

    m_treeCtrl->SetItemText(m_selectedItemId, text);

    m_refresh_requested = true;
    fillControls();
}

wxTreeItemId CategoryManager::getTreeItemFor(const wxTreeItemId& itemID, const wxString& itemText, const int64 parentid)
{
    wxTreeItemIdValue treeDummyValue;

    bool searching = true;
    wxTreeItemId catID = m_treeCtrl->GetFirstChild(itemID, treeDummyValue);
    while (catID.IsOk() && searching)
    {
        if (itemText == m_treeCtrl->GetItemText(catID) && parentid == dynamic_cast<mmTreeItemCateg*>(m_treeCtrl->GetItemData(catID))->getCategData()->m_parent_id_n)
            searching = false;
        else
            catID = m_treeCtrl->GetNextChild(itemID, treeDummyValue);
    }
    return catID;
}

void CategoryManager::setTreeSelection(int64 category_id)
{
    const CategoryData* category_n = CategoryModel::instance().get_id_data_n(category_id);
    if (category_n) {
        setTreeSelection(category_n->m_name, category_n->m_parent_id_n);
    }
    m_categ_id = category_id;
}

void CategoryManager::setTreeSelection(const wxString& catName, const int64 parentid)
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

void CategoryManager::OnCategoryRelocation(wxCommandEvent& /*event*/)
{
    MergeCategoryDialog dlg(this, m_categ_id);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msgStr;
        msgStr << _t("Merge categories completed") << "\n\n"
            << wxString::Format(_t("Records have been updated in the database: %i"),
                dlg.updatedCategoriesCount());
        wxMessageBox(msgStr, _t("Merge categories result"));
        m_refresh_requested = true;
        fillControls();
    }
}

void CategoryManager::OnExpandOrCollapseToggle(wxCommandEvent& event)
{
    m_processExpandCollapse = false;
    if (event.GetId() == ID_EXPAND)
    {
        m_treeCtrl->ExpandAll();
        m_treeCtrl->SelectItem(m_selectedItemId);
    }
    else
    {
        m_treeCtrl->CollapseAll();
        m_treeCtrl->Expand(root_);
        m_treeCtrl->SelectItem(m_selectedItemId);
    }
    m_treeCtrl->EnsureVisible(m_selectedItemId);
    SettingModel::instance().setBool("EXPAND_CATEGS_TREE", event.GetId() == ID_EXPAND);
    saveCurrentCollapseState();
    m_processExpandCollapse = true;
}

void CategoryManager::OnShowHiddenToggle(wxCommandEvent& /*event*/)
{
    SettingModel::instance().setBool("SHOW_HIDDEN_CATEGS", m_tbShowAll->GetValue());
    fillControls();
}

void CategoryManager::OnTextChanged(wxCommandEvent& event)
{
    m_maskStr = event.GetString();
    if (!m_maskStr.IsEmpty())
        m_maskStr = m_maskStr.Lower().Prepend("*");
    fillControls();
    m_maskTextCtrl->SetFocus();
    m_maskTextCtrl->SetInsertionPointEnd();
}

void CategoryManager::OnMenuSelected(wxCommandEvent& event)
{
    int id = event.GetId();

    CategoryData* cat_n = CategoryModel::instance().unsafe_get_id_data_n(m_categ_id);
    switch (id)
    {
    case MENU_ITEM_EDIT: {
        wxCommandEvent noop = wxEVT_NULL;
        OnEdit(noop);
        break;
    }
    case MENU_ITEM_HIDE: {
        m_treeCtrl->SetItemTextColour(m_selectedItemId, m_hiddenColor);
        cat_n->m_active = false;
        CategoryModel::instance().unsafe_update_data_n(cat_n);
        for (auto& sub_d : CategoryModel::instance().find_data_subtree_a(*cat_n)) {
            sub_d.m_active = false;
            CategoryModel::instance().save_data_n(sub_d);
        }
        break;
    }
    case MENU_ITEM_UNHIDE: {
        m_treeCtrl->SetItemTextColour(m_selectedItemId, NormalColor_);
        cat_n->m_active = true;
        CategoryModel::instance().unsafe_update_data_n(cat_n);
        for (auto& sub_d : CategoryModel::instance().find_data_subtree_a(*cat_n)) {
            sub_d.m_active = true;
            CategoryModel::instance().save_data_n(sub_d);
        }
        break;
    }
    case MENU_ITEM_DELETE: {
        mmDoDeleteSelectedCategory();
        break;
    }
    case MENU_ITEM_ADD: {
        wxCommandEvent noop = wxEVT_NULL;
        OnAdd(noop);
        break;
    }
    }

    fillControls();
}

void CategoryManager::OnClearSettings(wxCommandEvent& /*event*/)
{
    wxMessageDialog msgDlg(this, _t("Do you want to show all categories?")
            , _t("Show all categories")
            , wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);
    if (msgDlg.ShowModal() == wxID_YES) {
        auto category_a = CategoryModel::instance().find_all();
        CategoryModel::instance().db_savepoint();
        for (auto &category_d : category_a) {
            category_d.m_active = true;
            CategoryModel::instance().save_data_n(category_d);
        }
        CategoryModel::instance().db_release_savepoint();
        fillControls();
    }
}

void CategoryManager::OnItemRightClick(wxTreeEvent& event)
{
    wxMenu mainMenu;
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_ITEM_EDIT, _tu("&Edit Category…")));
    mainMenu.AppendSeparator();
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_ITEM_HIDE, _t("&Hide Selected Category")));
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_ITEM_UNHIDE, _t("&Show Selected Category")));
    mainMenu.AppendSeparator();
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_ITEM_DELETE, _t("&Remove Category")));
    mainMenu.AppendSeparator();
    mainMenu.Append(new wxMenuItem(&mainMenu, MENU_ITEM_ADD, _tu("&New Category…")));

    bool bItemHidden = (m_treeCtrl->GetItemTextColour(m_selectedItemId) != NormalColor_);
    mainMenu.Enable(MENU_ITEM_EDIT, m_selectedItemId != root_);
    mainMenu.Enable(MENU_ITEM_HIDE, !bItemHidden && (m_selectedItemId != root_));
    mainMenu.Enable(MENU_ITEM_UNHIDE, bItemHidden && (m_selectedItemId != root_));
    mainMenu.Enable(MENU_ITEM_DELETE, !mmIsUsed());

    PopupMenu(&mainMenu, event.GetPoint());
    event.Skip();
}

void CategoryManager::OnItemCollapseOrExpand(wxTreeEvent& event)
{
    if (m_processExpandCollapse)
    {
        saveCurrentCollapseState();
    }
    event.Skip();
}

bool CategoryManager::categShowStatus(int64 categId)
{
    if (CategoryModel::instance().is_hidden(categId))
        return false;

    return true;
}

wxString CategoryManager::getFullCategName()
{
    return CategoryModel::instance().full_name(m_categ_id);
}

bool CategoryManager::mmIsUsed() const
{
    return (CategoryModel::instance().is_used(m_categ_id) || m_categ_id == m_init_selected_categ_id);
}
