/*******************************************************
 Copyright (C) 2025, 2026 Klaus Wich

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

#include "NavigatorDialog.h"

#include "base/_constants.h"
#include "util/mmImage.h"

#include "model/AccountModel.h"
#include "model/PrefModel.h"

#include "DateRangeDialog.h"
#include "NavigatorEditDialog.h"
#include "app/mmFrame.h"

struct NavData : public wxClientData
{
    mmNavigatorItem* ref;
    explicit NavData(mmNavigatorItem* r): ref(r) {}
};

wxIMPLEMENT_DYNAMIC_CLASS(NavigatorDialog, wxDialog);

NavigatorDialog::NavigatorDialog()
{
}

NavigatorDialog::~NavigatorDialog()
{
    InfoModel::instance().saveSize(DIALOG_SIZE, GetSize());
}

NavigatorDialog::NavigatorDialog(wxWindow* parent):TreeListDialog(parent, _t("Navigator and account type configuration"))
{
    init();  // must be called first!!
    SetSize(InfoModel::instance().getSize(DIALOG_SIZE));
    m_treeList->Bind(wxEVT_TREELIST_ITEM_CHECKED, &NavigatorDialog::OnTreeItemChecked, this);
}

void NavigatorDialog::createColumns() {
    m_treeList->AppendColumn(_t("Name"), 250);
    m_treeList->AppendColumn(_t("Selection name"));
    wxImageList* imageList = NavTreeIconImages::instance().getImageList();
    m_treeList->SetImageList(imageList);

#ifdef __WXMAC__
    int iconWidth = imageList->GetSize().GetWidth();
    int spaceWidth = m_treeList->GetTextExtent(" ").GetWidth();
    m_spaces = (iconWidth / spaceWidth) + 1;
#endif
}

void NavigatorDialog::createMiddleElements(wxBoxSizer* itemBox) {
    m_edit = new wxButton(this, BTN_EDIT, _t("&Edit"));
    Bind(wxEVT_BUTTON, &NavigatorDialog::OnEdit, this, BTN_EDIT);
    m_edit->Enable(false);
    itemBox->Add(m_edit, g_flagsV);
}

void NavigatorDialog::createBottomElements(wxBoxSizer* itemBox) {
    itemBox->AddSpacer(15);
    itemBox->Add(new wxButton(this, BTN_NEW, _t("&New")), g_flagsV);
    Bind(wxEVT_BUTTON, &NavigatorDialog::OnNew, this, BTN_NEW);

    itemBox->AddSpacer(15);
    m_delete = new wxButton(this, BTN_DELETE, _t("&Delete"));
    Bind(wxEVT_BUTTON, &NavigatorDialog::OnDelete, this, BTN_DELETE);

    m_delete->Enable(false);
    itemBox->Add(m_delete, g_flagsV);

    itemBox->AddSpacer(70);
    wxButton* btn = new wxButton(this, BTN_RESET_NAMES, _t("Re&store default names"));
    Bind(wxEVT_BUTTON, &NavigatorDialog::OnNameReset, this, BTN_RESET_NAMES);

    itemBox->Add(btn, 0, wxALL, 5);
    mmToolTip(btn, _t("Restore default names"));
}

void NavigatorDialog::fillControls(wxTreeListItem root)
{
    mmNavigatorItem* ainfo = mmNavigatorList::instance().getFirstAccount();
    while (ainfo != nullptr) {
        appendAccountItem(root, ainfo);
        ainfo = mmNavigatorList::instance().getNextAccount(ainfo);
    }
}

wxTreeListItem NavigatorDialog::appendAccountItem(wxTreeListItem parent, mmNavigatorItem* ainfo)
{
    #ifdef __WXMAC__
        wxString text = mmNavigatorList::GetTranslatedName(ainfo);
        text.Prepend(wxString(' ', m_spaces));
        wxTreeListItem item = m_treeList->AppendItem(parent, text);
    #else
        wxTreeListItem item = m_treeList->AppendItem(parent, mmNavigatorList::GetTranslatedName(ainfo));
    #endif

    m_treeList->SetItemText(item, 1, ainfo->navTyp > mmNavigatorItem::NAV_TYP_PANEL ? mmNavigatorList::GetTranslatedSelection(ainfo) : "");
    m_treeList->SetItemImage(item, ainfo->imageId);
    m_treeList->SetItemData(item, new NavData(ainfo));
    switch (ainfo->type) {
        case mmNavigatorItem::TYPE_ID_SHARES :
            m_treeList->CheckItem(item, PrefModel::instance().getHideShareAccounts() ? wxCHK_UNCHECKED : wxCHK_CHECKED);
            break;

        case mmNavigatorItem::TYPE_ID_ASSET :
            m_treeList->CheckItem(item, (ainfo->active ? wxCHK_CHECKED : wxCHK_UNCHECKED));
            break;

        default:
            m_treeList->CheckItem(item, ainfo->navTyp != mmNavigatorItem::NAV_TYP_PANEL ? wxCHK_UNDETERMINED : (ainfo->active ? wxCHK_CHECKED : wxCHK_UNCHECKED));
    }
    return item;
}

void NavigatorDialog::OnEdit(wxCommandEvent&)
{
    wxTreeListItem item = m_treeList->GetSelection();
    NavData* data = static_cast<NavData*> (m_treeList->GetItemData(item));
    NavigatorEditDialog dlg(this, data->ref);
    if (dlg.ShowModal() == wxID_OK) {
        dlg.updateInfo(data->ref);
        if (data->ref->navTyp > mmNavigatorItem::NAV_TYP_PANEL) {
            m_treeList->SetItemText(item, 1, data->ref->choice);
        }
        else if (data->ref->navTyp == mmNavigatorItem::NAV_TYP_PANEL || data->ref->type == mmNavigatorItem::TYPE_ID_SHARES || data->ref->type == mmNavigatorItem::TYPE_ID_ASSET) {
            m_treeList->CheckItem(item, data->ref->active ? wxCHK_CHECKED : wxCHK_UNCHECKED);
        }
        m_treeList->SetItemImage(item, data->ref->imageId);
        m_treeList->SetItemText(item, 0, data->ref->name);
    }
}

void NavigatorDialog::OnNew(wxCommandEvent&)
{
    NavigatorEditDialog dlg(this, nullptr);
    if (dlg.ShowModal() == wxID_OK) {
        mmNavigatorItem* info = mmNavigatorList::instance().FindOrCreateEntry(-1);
        info->navTyp = mmNavigatorItem::NAV_TYP_ACCOUNT;
        dlg.updateInfo(info);
        m_treeList->Select(appendAccountItem(m_treeList->GetRootItem(), info));
        updateButtonState();
    }
}

void NavigatorDialog::OnDelete(wxCommandEvent&)
{
    wxTreeListItem item = m_treeList->GetSelection();
    NavData* data = static_cast<NavData*> (m_treeList->GetItemData(item));
    if (mmNavigatorList::instance().DeleteEntry(data->ref)) {
        m_treeList->DeleteItem(item);
    }
}

void NavigatorDialog::OnTreeItemChecked(wxTreeListEvent& event)
{
    wxTreeListItem item = event.GetItem();
    NavData* data = static_cast<NavData*> (m_treeList->GetItemData(item));
    if (data->ref->navTyp != mmNavigatorItem::NAV_TYP_PANEL && data->ref->type != mmNavigatorItem::TYPE_ID_SHARES && data->ref->type != mmNavigatorItem::TYPE_ID_ASSET) {
        m_treeList->CheckItem(item, wxCHK_UNDETERMINED);
    }
}

void NavigatorDialog::closeAction()
{
    updateItemsRecursive(m_treeList->GetRootItem());
    mmNavigatorList::instance().SaveSequenceAndState();
}

void NavigatorDialog::setDefault()
{
    mmNavigatorList::instance().SetToDefault();
    // FIXME: The application is not ready for dynamic account types.
    // Much of the code assumes well known and fixed account types.
    // See also the comment in mmNavigatorList::DeleteEntry
    AccountModel::instance().dangerous_reset_unknown_types();
}

void NavigatorDialog::OnNameReset(wxCommandEvent&)
{
    if (wxMessageBox(_t("Do you really want to restore the default names?")
        , _t("Restore default names")
        , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION) == wxYES)
    {
        mmNavigatorList::instance().SetToDefault();
        mmNavigatorList::instance().LoadFromDB(true);
        reloadTree();
    }
}

void NavigatorDialog::updateControlState(int selIdx, wxClientData* selData)
{
    bool nonfixed = false;
    if (selIdx > -1) {
        NavData* data = static_cast<NavData*> (selData);
        nonfixed = data->ref->type >= mmNavigatorItem::NAV_ENTRY_size;
    }
    m_edit->Enable(selIdx > -1);
    m_delete->Enable(nonfixed);
}

void NavigatorDialog::copyTreeItemData(wxTreeListItem src, wxTreeListItem dst) {
    const int colCount = m_treeList->GetColumnCount();
    for (int c = 1; c < colCount; ++c) {
        m_treeList->SetItemText(dst, c, m_treeList->GetItemText(src, c));
    }
    NavData* data = static_cast<NavData*> (m_treeList->GetItemData(src));
    data->ref->active = m_treeList->GetCheckedState(src) == wxCHK_CHECKED || data->ref->navTyp != mmNavigatorItem::NAV_TYP_PANEL;
    m_treeList->SetItemImage(dst, data->ref->imageId);
    m_treeList->SetItemData(dst, new NavData(data->ref));
    if (data->ref->navTyp == mmNavigatorItem::NAV_TYP_PANEL || data->ref->type == mmNavigatorItem::TYPE_ID_SHARES || data->ref->type == mmNavigatorItem::TYPE_ID_ASSET) {
        m_treeList->CheckItem(dst, data->ref->active ? wxCHK_CHECKED : wxCHK_UNCHECKED);
    }
    else {
        m_treeList->CheckItem(dst, wxCHK_UNDETERMINED);
    }
}

void NavigatorDialog::updateItemsRecursive(wxTreeListItem item)
{
    int idx = 0;
    wxTreeListItem child = m_treeList->GetFirstChild(item);
    while (child.IsOk()) {
        NavData* data = static_cast<NavData*> (m_treeList->GetItemData(child));
        mmNavigatorItem* ainfo = data->ref;
        if (ainfo) {
            data->ref->seq_no = idx++;
            data->ref->active = m_treeList->GetCheckedState(child) == wxCHK_CHECKED || (data->ref->navTyp != mmNavigatorItem::NAV_TYP_PANEL && data->ref->type != mmNavigatorItem::TYPE_ID_SHARES && data->ref->type != mmNavigatorItem::TYPE_ID_ASSET);
            updateItemsRecursive(child);
            child = m_treeList->GetNextSibling(child);
            // Special for Trash:
            if (data->ref->type == mmNavigatorItem::NAV_ENTRY_DELETED_TRANSACTIONS) {
                PrefModel::instance().saveHideDeletedTransactions(!data->ref->active);
                mmFrame* mainFrame = wxDynamicCast(this->GetParent(), mmFrame);
                if (mainFrame) {
                    mainFrame->SetTrashState(data->ref->active);
                }
            }
            // Special for Share Accounts:
            else if (data->ref->type == mmNavigatorItem::TYPE_ID_SHARES) {
                PrefModel::instance().saveHideShareAccounts(!data->ref->active);
                mmFrame* mainFrame = wxDynamicCast(this->GetParent(), mmFrame);
                if (mainFrame) {
                    mainFrame->SetShareAccountState(data->ref->active);
                }
            }
        }
    }
}
