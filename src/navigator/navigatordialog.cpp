/*******************************************************
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
    NavData(NavigatorTypesInfo* r): ref(r) {}
};

wxIMPLEMENT_DYNAMIC_CLASS(mmNavigatorDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmNavigatorDialog, wxDialog)
  EVT_BUTTON(wxID_CLOSE, mmNavigatorDialog::OnClose)
  EVT_BUTTON(BTN_UP_TOP, mmNavigatorDialog::OnTop)
  EVT_BUTTON(BTN_UP, mmNavigatorDialog::OnUp)
  EVT_BUTTON(BTN_EDIT, mmNavigatorDialog::OnEdit)
  EVT_BUTTON(BTN_DOWN, mmNavigatorDialog::OnDown)
  EVT_BUTTON(BTN_DOWN_BOTTOM, mmNavigatorDialog::OnBottom)
  EVT_BUTTON(BTN_NEW, mmNavigatorDialog::OnNew)
  EVT_BUTTON(BTN_DELETE, mmNavigatorDialog::OnDelete)
  EVT_BUTTON(BTN_DEFAULT, mmNavigatorDialog::OnDefault)
  EVT_TREELIST_ITEM_CHECKED(wxID_ANY, mmNavigatorDialog::OnTreeItemChecked)
  EVT_TREELIST_SELECTION_CHANGED(wxID_ANY, mmNavigatorDialog::OnTreeSelectionChange)
wxEND_EVENT_TABLE()


mmNavigatorDialog::mmNavigatorDialog()
{
}

mmNavigatorDialog::mmNavigatorDialog(wxWindow* parent)
{
    this->SetFont(parent->GetFont());
    Create(parent, -1, _t("Navigator and account type configuration"), wxDefaultPosition, wxSize(-1, -1), wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX, "");
    createControls();
    SetIcon(mmex::getProgramIcon());
    fillControls();
    Centre();
    SetMinSize(wxSize(300, 700));
    Fit();
}

void mmNavigatorDialog::createControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainBoxSizer);

    wxBoxSizer* mainPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    mainBoxSizer->Add(mainPanelSizer, g_flagsExpand);

    wxBoxSizer* itemBoxVleft = new wxBoxSizer(wxVERTICAL);
    mainPanelSizer->Add(itemBoxVleft, g_flagsExpand);

    m_treeList = new wxTreeListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(400, 300),
                                    wxTL_3STATE |wxTL_SINGLE);
    m_treeList->AppendColumn(_t("Name"), 250);
    m_treeList->AppendColumn(_t("Selection name"));
    const auto navIconSize = Option::instance().getNavigationIconSize();
    wxImageList* imageList = new wxImageList(navIconSize, navIconSize);
    for (const auto& bundle : navtree_images_list(navIconSize)) {
        wxBitmap bitmap = bundle.GetBitmap(wxSize(navIconSize, navIconSize));
        imageList->Add(bitmap);
    }
    m_treeList->SetImageList(imageList);
    itemBoxVleft->Add(m_treeList, 1, wxEXPAND | wxALL, 5);

    wxBoxSizer* itemBoxVright = new wxBoxSizer(wxVERTICAL);
    mainPanelSizer->Add(itemBoxVright, wxSizerFlags().Left().Border(wxALL, 5));
    itemBoxVright->AddSpacer(20);

    m_up_top = new wxButton(this, BTN_UP_TOP, _t("Top"));
    m_up_top->SetBitmap(mmBitmapBundle(png::UPARROW, mmBitmapButtonSize));
    m_up_top->Enable(false);
    itemBoxVright->Add(m_up_top, g_flagsV);

    m_up = new wxBitmapButton(this, BTN_UP, mmBitmapBundle(png::UPARROW, mmBitmapButtonSize));
    m_up->Enable(false);
    itemBoxVright->Add(m_up, g_flagsV);

    m_edit = new wxButton(this, BTN_EDIT, _t("&Edit"));
    m_edit->Enable(false);
    itemBoxVright->Add(m_edit, g_flagsV);

    m_down = new wxBitmapButton(this, BTN_DOWN, mmBitmapBundle(png::DOWNARROW, mmBitmapButtonSize));
    m_down->Enable(false);
    itemBoxVright->Add(m_down, g_flagsV);

    m_down_bottom = new wxButton(this, BTN_DOWN_BOTTOM, _t("Bottom"));
    m_down_bottom->SetBitmap(mmBitmapBundle(png::DOWNARROW, mmBitmapButtonSize));
    m_down_bottom->Enable(false);
    itemBoxVright->Add(m_down_bottom, g_flagsV);

    itemBoxVright->AddSpacer(15);
    itemBoxVright->Add(new wxButton(this, BTN_NEW, _t("New")), g_flagsV);

    itemBoxVright->AddSpacer(15);
    m_delete = new wxButton(this, BTN_DELETE, _t("Delete"));
    m_delete->Enable(false);
    itemBoxVright->Add(m_delete, g_flagsV);

    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    mainBoxSizer->Add(btnSizer, g_flagsCenter);

    btnSizer->Add(new wxButton(this, wxID_CLOSE, _t("&Close")), 0, wxALL, 5);
    btnSizer->Add(new wxButton(this, BTN_DEFAULT, _t("Restore default")), 0, wxALL, 5);

    this->Layout();
}

void mmNavigatorDialog::fillControls()
{
    m_treeList->DeleteAllItems();
    wxTreeListItem root = m_treeList->GetRootItem();

    NavigatorTypesInfo* ainfo = NavigatorTypes::instance().getFirstAccount();
    while (ainfo != nullptr) {
        appendAccountItem(root, ainfo);
        ainfo = NavigatorTypes::instance().getNextAccount(ainfo);
    }
    m_treeList->Expand(root);

}

wxTreeListItem mmNavigatorDialog::appendAccountItem(wxTreeListItem parent, NavigatorTypesInfo* ainfo)
{
    wxTreeListItem item = m_treeList->AppendItem(parent, ainfo->name);
    m_treeList->SetItemText(item, 1, ainfo->navTyp > NavigatorTypes::NAV_TYP_PANEL ? ainfo->choice : "");
    m_treeList->SetItemImage(item, ainfo->imageId);
    m_treeList->SetItemData(item, new NavData(ainfo));
    m_treeList->CheckItem(item, ainfo->navTyp != NavigatorTypes::NAV_TYP_PANEL ? wxCHK_UNDETERMINED : (ainfo->active ? wxCHK_CHECKED : wxCHK_UNCHECKED));
    return item;
}

void mmNavigatorDialog::OnTop(wxCommandEvent&)
{
    wxTreeListItem sel = m_treeList->GetSelection();
    wxTreeListItem parent = m_treeList->GetItemParent(sel);
    wxTreeListItem newItem = m_treeList->InsertItem(parent, wxTLI_FIRST, m_treeList->GetItemText(sel, 0));

    moveItemData(sel, newItem);

    updateButtonState();
}

void mmNavigatorDialog::OnUp(wxCommandEvent&)
{
    moveSelectedItem(-1);
    updateButtonState();
}

void mmNavigatorDialog::OnTreeSelectionChange(wxTreeListEvent&)
{
    updateButtonState();
}

void mmNavigatorDialog::OnTreeItemChecked(wxTreeListEvent& event)
{
    wxTreeListItem item = event.GetItem();
    NavData* data = static_cast<NavData*> (m_treeList->GetItemData(item));
    if (data->ref->navTyp != NavigatorTypes::NAV_TYP_PANEL) {
        m_treeList->CheckItem(item, wxCHK_UNDETERMINED);
    }
}

void mmNavigatorDialog::OnEdit(wxCommandEvent&)
{
    wxTreeListItem item = m_treeList->GetSelection();
    NavData* data = static_cast<NavData*> (m_treeList->GetItemData(item));
    mmNavigatorEditDialog dlg(this, data->ref);
    if (dlg.ShowModal() == wxID_OK) {
        dlg.updateInfo(data->ref);
        if (data->ref->navTyp == NavigatorTypes::NAV_TYP_ACCOUNT || data->ref->navTyp == NavigatorTypes::NAV_TYP_OTHER) {
            m_treeList->SetItemText(item, 1, data->ref->choice);
        }
        else if (data->ref->navTyp == NavigatorTypes::NAV_TYP_PANEL) {
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

void mmNavigatorDialog::OnDown(wxCommandEvent&)
{
    moveSelectedItem(1);
    updateButtonState();
}

void mmNavigatorDialog::OnBottom(wxCommandEvent&)
{
    wxTreeListItem sel = m_treeList->GetSelection();
    wxTreeListItem parent = m_treeList->GetItemParent(sel);
    wxTreeListItem newItem = m_treeList->InsertItem(parent, wxTLI_LAST, m_treeList->GetItemText(sel, 0));

    moveItemData(sel, newItem);

    updateButtonState();
}

void mmNavigatorDialog::OnDelete(wxCommandEvent&)
{
    wxTreeListItem item = m_treeList->GetSelection();
    NavData* data = static_cast<NavData*> (m_treeList->GetItemData(item));
    if (NavigatorTypes::instance().DeleteEntry(data->ref)) {
        m_treeList->DeleteItem(item);
    }
}

void mmNavigatorDialog::OnClose(wxCommandEvent&)
{
    updateItemsRecursive(m_treeList->GetRootItem());
    NavigatorTypes::instance().SaveSequenceAndState();
    EndModal(wxID_CLOSE);
}

void mmNavigatorDialog::OnDefault(wxCommandEvent&)
{
    if (wxMessageBox(_t("Do you really want to restore the default values?\n\nAll customization will be lost!")
        , _t("Set to default")
        , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION) == wxYES)
    {
        NavigatorTypes::instance().SetToDefault();
        fillControls();
    }
}

void mmNavigatorDialog::updateButtonState() {
    int selIdx = -1;
    int count = 0;
    bool nonfixed = false;

    wxTreeListItem sel = m_treeList->GetSelection();
    if (sel.IsOk()) {
        wxTreeListItem parent = m_treeList->GetItemParent(sel);
        if (parent.IsOk()) {
            wxTreeListItems siblings = getChildrenList(parent);
            selIdx = findItemIndex(siblings, sel);
            count  = static_cast<int>(siblings.size());
        }
        NavData* data = static_cast<NavData*> (m_treeList->GetItemData(sel));
        nonfixed = data->ref->id >= NavigatorTypes::NAV_ENTRY_size;
    }
    bool isvalid = selIdx > -1;
    m_up_top->Enable(selIdx > 0);
    m_up->Enable(selIdx > 0);
    m_edit->Enable(isvalid);
    m_down->Enable(isvalid && selIdx < count- 1);
    m_down_bottom->Enable(isvalid && selIdx < count - 1);
    m_delete->Enable(nonfixed);
}

void mmNavigatorDialog::moveSelectedItem(int direction)
{
    wxTreeListItem sel = m_treeList->GetSelection();
    if (sel.IsOk()) {
        wxTreeListItem parent = m_treeList->GetItemParent(sel);
        if (parent.IsOk()) {
            wxTreeListItems siblings = getChildrenList(parent);
            int idx = findItemIndex(siblings, sel);
            if (idx != -1) {
                int newIdx = idx + (direction > 0 ? 1 : -2);
                if (newIdx >= -1 && newIdx < static_cast<int>(siblings.size())) {
                    wxString name = m_treeList->GetItemText(sel, 0);
                    wxTreeListItem newItem;
                    if (newIdx > -1) {
                        wxTreeListItem previousItem = siblings[newIdx];
                        if (previousItem.IsOk())
                            newItem = m_treeList->InsertItem(parent, previousItem, name);
                        else
                            newItem = m_treeList->AppendItem(parent, name);
                    }
                    else {
                        newItem = m_treeList->InsertItem(parent, wxTLI_FIRST, name);
                    }
                    moveItemData(sel, newItem);
                }
            }
        }
    }
}

void mmNavigatorDialog::moveItemData(wxTreeListItem sel, wxTreeListItem newItem)
{
    copyTreeItemData(sel, newItem);
    wxTreeListItems children = getChildrenList(sel);
    for (auto& child : children) {
        cloneSubtree(child, newItem);
    }
    m_treeList->DeleteItem(sel);
    m_treeList->Select(newItem);
}

wxTreeListItems mmNavigatorDialog::getChildrenList(wxTreeListItem parent)
{
    wxTreeListItems items;
    wxTreeListItem child = m_treeList->GetFirstChild(parent);
    while (child.IsOk()) {
        items.push_back(child);
        child = m_treeList->GetNextSibling(child);
    }
    return items;
}

void mmNavigatorDialog::cloneSubtree(wxTreeListItem src, wxTreeListItem dstParent)
{
    wxTreeListItem newItem = m_treeList->AppendItem(dstParent, m_treeList->GetItemText(src, 0));
    copyTreeItemData(src, newItem);
    wxTreeListItem child = m_treeList->GetFirstChild(src);
    while (child.IsOk()) {
        cloneSubtree(child, newItem);
        child = m_treeList->GetNextSibling(child);
    }
}

int mmNavigatorDialog::findItemIndex(const wxTreeListItems& items, const wxTreeListItem& target)
{
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        if (items[i] == target)
            return static_cast<int>(i);
    }
    return -1;
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
    m_treeList->CheckItem(dst, data->ref->navTyp != NavigatorTypes::NAV_TYP_PANEL ? wxCHK_UNDETERMINED : data->ref->active ? wxCHK_CHECKED : wxCHK_UNCHECKED);
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
            data->ref->active = m_treeList->GetCheckedState(child) == wxCHK_CHECKED || data->ref->navTyp != NavigatorTypes::NAV_TYP_PANEL;
            updateItemsRecursive(child);
            child = m_treeList->GetNextSibling(child);
            // Special for Trash:
            if (data->ref->id == NavigatorTypes::NAV_ENTRY_DELETED_TRANSACTIONS) {
                Option::instance().setHideDeletedTransactions(!data->ref->active);
                mmGUIFrame* mainFrame = wxDynamicCast(this->GetParent(), mmGUIFrame);
                if (mainFrame) {
                    mainFrame->SetTrashState(data->ref->active);
                }
            }
        }
    }
}