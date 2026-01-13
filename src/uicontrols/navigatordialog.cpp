/*******************************************************
 Copyright (C) 2025 Klaus Wich

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 ********************************************************/

#include "navigatordialog.h"

#include "daterangeeditdialog.h"
#include "constants.h"
#include "images_list.h"
#include "option.h"
#include "navigatoreditdialog.h"
#include "Model_Account.h"
#include "mmframe.h"

struct NavData : public wxClientData
{
    NavigatorTypesInfo* ref;
    explicit NavData(NavigatorTypesInfo* r): ref(r) {}
};

wxIMPLEMENT_DYNAMIC_CLASS(mmNavigatorDialog, wxDialog);

mmNavigatorDialog::mmNavigatorDialog()
{
}

mmNavigatorDialog::~mmNavigatorDialog()
{
    Model_Infotable::instance().setSize(DIALOG_SIZE, GetSize());
}

mmNavigatorDialog::mmNavigatorDialog(wxWindow* parent):genericTreeListDialog(parent, _t("Navigator and account type configuration"))
{
    init();  // must be called first!!
    SetSize(Model_Infotable::instance().getSize(DIALOG_SIZE));
    m_treeList->Bind(wxEVT_TREELIST_ITEM_CHECKED, &mmNavigatorDialog::OnTreeItemChecked, this);
}

void mmNavigatorDialog::createColumns() {
    m_treeList->AppendColumn(_t("Name"), 250);
    m_treeList->AppendColumn(_t("Selection name"));

    // Add imagelist
    const auto navIconSize = Option::instance().getNavigationIconSize();
    wxImageList* imageList = new wxImageList(navIconSize, navIconSize);
    for (const auto& bundle : navtree_images_list(navIconSize)) {
        wxBitmap bitmap = bundle.GetBitmap(wxSize(navIconSize, navIconSize));
        imageList->Add(bitmap);
    }
    m_treeList->SetImageList(imageList);

#ifdef __WXMAC__
    int iconWidth = imageList->GetSize().GetWidth();
    int spaceWidth = m_treeList->GetTextExtent(" ").GetWidth();
    m_spaces = (iconWidth / spaceWidth) + 1;
#endif
}

void mmNavigatorDialog::createMiddleElements(wxBoxSizer* itemBox) {
    m_edit = new wxButton(this, BTN_EDIT, _t("Edit"));
    Bind(wxEVT_BUTTON, &mmNavigatorDialog::OnEdit, this, BTN_EDIT);
    m_edit->Enable(false);
    itemBox->Add(m_edit, g_flagsV);
}

void mmNavigatorDialog::createBottomElements(wxBoxSizer* itemBox) {
    itemBox->AddSpacer(15);
    itemBox->Add(new wxButton(this, BTN_NEW, _t("New")), g_flagsV);
    Bind(wxEVT_BUTTON, &mmNavigatorDialog::OnNew, this, BTN_NEW);

    itemBox->AddSpacer(15);
    m_delete = new wxButton(this, BTN_DELETE, _t("Delete"));
    Bind(wxEVT_BUTTON, &mmNavigatorDialog::OnDelete, this, BTN_DELETE);

    m_delete->Enable(false);
    itemBox->Add(m_delete, g_flagsV);

    itemBox->AddSpacer(70);
    wxButton* btn = new wxButton(this, BTN_RESET_NAMES, _t("Reset names"));
    Bind(wxEVT_BUTTON, &mmNavigatorDialog::OnNameReset, this, BTN_RESET_NAMES);

    itemBox->Add(btn, 0, wxALL, 5);
    mmToolTip(btn, _t("Reset the standard names to default values"));
}

void mmNavigatorDialog::fillControls(wxTreeListItem root)
{
    NavigatorTypesInfo* ainfo = NavigatorTypes::instance().getFirstAccount();
    while (ainfo != nullptr) {
        appendAccountItem(root, ainfo);
        ainfo = NavigatorTypes::instance().getNextAccount(ainfo);
    }
}

wxTreeListItem mmNavigatorDialog::appendAccountItem(wxTreeListItem parent, NavigatorTypesInfo* ainfo)
{
    #ifdef __WXMAC__
        wxString text = NavigatorTypes::GetTranslatedName(ainfo);
        text.Prepend(wxString(' ', m_spaces));
        wxTreeListItem item = m_treeList->AppendItem(parent, text);
    #else
        wxTreeListItem item = m_treeList->AppendItem(parent, NavigatorTypes::GetTranslatedName(ainfo));
    #endif

    m_treeList->SetItemText(item, 1, ainfo->navTyp > NavigatorTypes::NAV_TYP_PANEL ? NavigatorTypes::GetTranslatedSelection(ainfo) : "");
    m_treeList->SetItemImage(item, ainfo->imageId);
    m_treeList->SetItemData(item, new NavData(ainfo));
    if (ainfo->type == NavigatorTypes::TYPE_ID_SHARES) {
        m_treeList->CheckItem(item, Option::instance().getHideShareAccounts() ? wxCHK_UNCHECKED : wxCHK_CHECKED);
    }
    else {
        m_treeList->CheckItem(item, ainfo->navTyp != NavigatorTypes::NAV_TYP_PANEL ? wxCHK_UNDETERMINED : (ainfo->active ? wxCHK_CHECKED : wxCHK_UNCHECKED));
    }
    return item;
}

void mmNavigatorDialog::OnEdit(wxCommandEvent&)
{
    wxTreeListItem item = m_treeList->GetSelection();
    NavData* data = static_cast<NavData*> (m_treeList->GetItemData(item));
    mmNavigatorEditDialog dlg(this, data->ref);
    if (dlg.ShowModal() == wxID_OK) {
        dlg.updateInfo(data->ref);
        if (data->ref->navTyp > NavigatorTypes::NAV_TYP_PANEL) {
            m_treeList->SetItemText(item, 1, data->ref->choice);
        }
        else if (data->ref->navTyp == NavigatorTypes::NAV_TYP_PANEL || data->ref->type == NavigatorTypes::TYPE_ID_SHARES) {
            m_treeList->CheckItem(item, data->ref->active ? wxCHK_CHECKED : wxCHK_UNCHECKED);
        }
        m_treeList->SetItemImage(item, data->ref->imageId);
        m_treeList->SetItemText(item, 0, data->ref->name);
    }
}

void mmNavigatorDialog::OnNew(wxCommandEvent&)
{
    mmNavigatorEditDialog dlg(this, nullptr);
    if (dlg.ShowModal() == wxID_OK) {
        NavigatorTypesInfo* info = NavigatorTypes::instance().FindOrCreateEntry(-1);
        info->navTyp = NavigatorTypes::NAV_TYP_ACCOUNT;
        dlg.updateInfo(info);
        m_treeList->Select(appendAccountItem(m_treeList->GetRootItem(), info));
        updateButtonState();
    }
}

void mmNavigatorDialog::OnDelete(wxCommandEvent&)
{
    wxTreeListItem item = m_treeList->GetSelection();
    NavData* data = static_cast<NavData*> (m_treeList->GetItemData(item));
    if (NavigatorTypes::instance().DeleteEntry(data->ref)) {
        m_treeList->DeleteItem(item);
    }
}

void mmNavigatorDialog::OnTreeItemChecked(wxTreeListEvent& event)
{
    wxTreeListItem item = event.GetItem();
    NavData* data = static_cast<NavData*> (m_treeList->GetItemData(item));
    if (data->ref->navTyp != NavigatorTypes::NAV_TYP_PANEL && data->ref->type != NavigatorTypes::TYPE_ID_SHARES) {
        m_treeList->CheckItem(item, wxCHK_UNDETERMINED);
    }
}

void mmNavigatorDialog::closeAction()
{
    updateItemsRecursive(m_treeList->GetRootItem());
    NavigatorTypes::instance().SaveSequenceAndState();
}

void mmNavigatorDialog::setDefault()
{
    NavigatorTypes::instance().SetToDefault();
    Model_Account::instance().resetUnknownAccountTypes();
}

void mmNavigatorDialog::OnNameReset(wxCommandEvent&)
{
    if (wxMessageBox(_t("Do you really want to reset the configured names?")
        , _t("Reset names")
        , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION) == wxYES)
    {
        NavigatorTypes::instance().SetToDefault();
        NavigatorTypes::instance().LoadFromDB(true);
        reloadTree();
    }
}

void mmNavigatorDialog::updateControlState(int selIdx, wxClientData* selData)
{
    bool nonfixed = false;
    if (selIdx > -1) {
        NavData* data = static_cast<NavData*> (selData);
        nonfixed = data->ref->type >= NavigatorTypes::NAV_ENTRY_size;
    }
    m_edit->Enable(selIdx > -1);
    m_delete->Enable(nonfixed);
}

void mmNavigatorDialog::copyTreeItemData(wxTreeListItem src, wxTreeListItem dst) {
    const int colCount = m_treeList->GetColumnCount();
    for (int c = 1; c < colCount; ++c) {
        m_treeList->SetItemText(dst, c, m_treeList->GetItemText(src, c));
    }
    NavData* data = static_cast<NavData*> (m_treeList->GetItemData(src));
    data->ref->active = m_treeList->GetCheckedState(src) == wxCHK_CHECKED || data->ref->navTyp != NavigatorTypes::NAV_TYP_PANEL;
    m_treeList->SetItemImage(dst, data->ref->imageId);
    m_treeList->SetItemData(dst, new NavData(data->ref));
    if (data->ref->navTyp == NavigatorTypes::NAV_TYP_PANEL || data->ref->type == NavigatorTypes::TYPE_ID_SHARES) {
        m_treeList->CheckItem(dst, data->ref->active ? wxCHK_CHECKED : wxCHK_UNCHECKED);
    }
    else {
        m_treeList->CheckItem(dst, wxCHK_UNDETERMINED);
    }
}

void mmNavigatorDialog::updateItemsRecursive(wxTreeListItem item)
{
    int idx = 0;
    wxTreeListItem child = m_treeList->GetFirstChild(item);
    while (child.IsOk()) {
        NavData* data = static_cast<NavData*> (m_treeList->GetItemData(child));
        NavigatorTypesInfo* ainfo = data->ref;
        if (ainfo) {
            data->ref->seq_no = idx++;
            data->ref->active = m_treeList->GetCheckedState(child) == wxCHK_CHECKED || (data->ref->navTyp != NavigatorTypes::NAV_TYP_PANEL && data->ref->type != NavigatorTypes::TYPE_ID_SHARES);
            updateItemsRecursive(child);
            child = m_treeList->GetNextSibling(child);
            // Special for Trash:
            if (data->ref->type == NavigatorTypes::NAV_ENTRY_DELETED_TRANSACTIONS) {
                Option::instance().setHideDeletedTransactions(!data->ref->active);
                mmGUIFrame* mainFrame = wxDynamicCast(this->GetParent(), mmGUIFrame);
                if (mainFrame) {
                    mainFrame->SetTrashState(data->ref->active);
                }
            }
            // Special for Share Accounts:
            if (data->ref->type == NavigatorTypes::TYPE_ID_SHARES) {
                Option::instance().setHideShareAccounts(!data->ref->active);
                mmGUIFrame* mainFrame = wxDynamicCast(this->GetParent(), mmGUIFrame);
                if (mainFrame) {
                    mainFrame->SetShareAccountState(data->ref->active);
                }
            }
        }
    }
}
