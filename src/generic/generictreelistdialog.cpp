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

#include "generictreelistdialog.h"

#include "daterangeeditdialog.h"
#include "constants.h"
#include "images_list.h"
#include "option.h"


wxIMPLEMENT_DYNAMIC_CLASS(genericTreeListDialog, wxDialog);


genericTreeListDialog::genericTreeListDialog() {}

genericTreeListDialog::~genericTreeListDialog() {}

genericTreeListDialog::genericTreeListDialog(wxWindow* parent, wxString title)
{
    this->SetFont(parent->GetFont());
    Create(parent, -1, title, wxDefaultPosition, wxSize(-1, -1), wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX, "");
    SetIcon(mmex::getProgramIcon());
}

// virtual methods reimplemented in derived class:
void genericTreeListDialog::createColumns() {}

void genericTreeListDialog::createBottomElements(wxBoxSizer* WXUNUSED(itemBox)) {}

void genericTreeListDialog::createMiddleElements(wxBoxSizer* WXUNUSED(itemBox)) {}

void genericTreeListDialog::fillControls(wxTreeListItem WXUNUSED(root)) {}

void genericTreeListDialog::updateControlState(int WXUNUSED(selIdx), wxClientData* WXUNUSED(selData)) {}

void genericTreeListDialog::setDefault() {}

void genericTreeListDialog::closeAction() {}

void genericTreeListDialog::copyTreeItemData(wxTreeListItem WXUNUSED(src), wxTreeListItem WXUNUSED(dst)) {}

// methods called from derived class:
void genericTreeListDialog::init(long liststyle)  // Must be called in constructor from derived class !!!
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainBoxSizer);

    wxBoxSizer* mainPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    mainBoxSizer->Add(mainPanelSizer, g_flagsExpand);

    wxBoxSizer* itemBoxVleft = new wxBoxSizer(wxVERTICAL);
    mainPanelSizer->Add(itemBoxVleft, g_flagsExpand);

    m_treeList = new wxTreeListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(400, 300), liststyle);
    m_treeList->Bind(wxEVT_TREELIST_SELECTION_CHANGED, &genericTreeListDialog::OnTreeSelectionChange, this);

    createColumns();

    itemBoxVleft->Add(m_treeList, 1, wxEXPAND | wxALL, 5);

    wxBoxSizer* itemBoxVright = new wxBoxSizer(wxVERTICAL);
    mainPanelSizer->Add(itemBoxVright, wxSizerFlags().Left().Border(wxALL, 5));
    itemBoxVright->AddSpacer(20);

    m_up_top = new wxButton(this, BTN_UP_TOP, _tu("Top"));
    m_up_top->SetBitmap(mmBitmapBundle(png::UPARROW, mmBitmapButtonSize));
    m_up_top->Enable(false);
    m_up_top->Bind(wxEVT_BUTTON, &genericTreeListDialog::OnTop, this);

    itemBoxVright->Add(m_up_top, g_flagsV);

    m_up = new wxBitmapButton(this, BTN_UP, mmBitmapBundle(png::UPARROW, mmBitmapButtonSize));
    m_up->Enable(false);
    m_up->Bind(wxEVT_BUTTON, &genericTreeListDialog::OnUp, this);

    itemBoxVright->Add(m_up, g_flagsV);

    createMiddleElements(itemBoxVright);  // to be overwritten

    m_down = new wxBitmapButton(this, BTN_DOWN, mmBitmapBundle(png::DOWNARROW, mmBitmapButtonSize));
    m_down->Enable(false);
    m_down->Bind(wxEVT_BUTTON, &genericTreeListDialog::OnDown, this);
    itemBoxVright->Add(m_down, g_flagsV);

    m_down_bottom = new wxButton(this, BTN_DOWN_BOTTOM, _tu("Bottom"));
    m_down_bottom->SetBitmap(mmBitmapBundle(png::DOWNARROW, mmBitmapButtonSize));
    m_down_bottom->Enable(false);
    m_down_bottom->Bind(wxEVT_BUTTON, &genericTreeListDialog::OnBottom, this);

    itemBoxVright->Add(m_down_bottom, g_flagsV);

    createBottomElements(itemBoxVright);  // to be overwritten

    wxButton* btn = new wxButton(this, BTN_DEFAULT, _tu("Restore default"));
    itemBoxVright->Add(btn, 0, wxALL, 5);
    btn->Bind(wxEVT_BUTTON, &genericTreeListDialog::OnDefault, this);

    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    mainBoxSizer->Add(btnSizer, g_flagsCenter);

    btn = new wxButton(this, wxID_CLOSE, _tu("&Close"));
    btnSizer->Add(btn, 0, wxALL, 5);
    btn->Bind(wxEVT_BUTTON, &genericTreeListDialog::OnClose, this);

    this->Layout();

    Bind(wxEVT_CLOSE_WINDOW, &genericTreeListDialog::OnCloseWin, this);

    reloadTree();
    m_treeList->Expand(m_treeList->GetRootItem());
    Centre();
    SetMinSize(wxSize(200, 200));
    Fit();
}

void genericTreeListDialog::updateButtonState() {
    int selIdx = -1;
    int count = 0;
    wxClientData* clientdata = nullptr;

    wxTreeListItem sel = m_treeList->GetSelection();
    if (sel.IsOk()) {
        wxTreeListItem parent = m_treeList->GetItemParent(sel);
        if (parent.IsOk()) {
            wxTreeListItems siblings = getChildrenList(parent);
            selIdx = findItemIndex(siblings, sel);
            count = static_cast<int>(siblings.size());
        }
        clientdata = m_treeList->GetItemData(sel);
    }

    m_up_top->Enable(selIdx > 0);
    m_up->Enable(selIdx > 0);
    m_down->Enable(selIdx > -1 && selIdx < count- 1);
    m_down_bottom->Enable(selIdx > -1 && selIdx < count - 1);

    updateControlState(selIdx, clientdata);
}

void genericTreeListDialog::reloadTree()
{
    m_treeList->DeleteAllItems();
    fillControls(m_treeList->GetRootItem());
}

// private methods:
void genericTreeListDialog::moveSelectedItem(int direction)
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

void genericTreeListDialog::moveItemData(wxTreeListItem sel, wxTreeListItem newItem)
{
    copyTreeItemData(sel, newItem);
    wxTreeListItems children = getChildrenList(sel);
    for (auto& child : children) {
        cloneSubtree(child, newItem);
    }
    m_treeList->DeleteItem(sel);
    m_treeList->Select(newItem);
}

wxTreeListItems genericTreeListDialog::getChildrenList(wxTreeListItem parent)
{
    wxTreeListItems items;
    wxTreeListItem child = m_treeList->GetFirstChild(parent);
    while (child.IsOk()) {
        items.push_back(child);
        child = m_treeList->GetNextSibling(child);
    }
    return items;
}

void genericTreeListDialog::cloneSubtree(wxTreeListItem src, wxTreeListItem dstParent)
{
    wxTreeListItem newItem = m_treeList->AppendItem(dstParent, m_treeList->GetItemText(src, 0));
    copyTreeItemData(src, newItem);
    wxTreeListItem child = m_treeList->GetFirstChild(src);
    while (child.IsOk()) {
        cloneSubtree(child, newItem);
        child = m_treeList->GetNextSibling(child);
    }
}

int genericTreeListDialog::findItemIndex(const wxTreeListItems& items, const wxTreeListItem& target)
{
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        if (items[i] == target)
            return i;
    }
    return -1;
}

void genericTreeListDialog::OnTop(wxCommandEvent&)
{
    wxTreeListItem sel = m_treeList->GetSelection();
    wxTreeListItem parent = m_treeList->GetItemParent(sel);
    wxTreeListItem newItem = m_treeList->InsertItem(parent, wxTLI_FIRST, m_treeList->GetItemText(sel, 0));

    moveItemData(sel, newItem);

    updateButtonState();
}

void genericTreeListDialog::OnUp(wxCommandEvent&)
{
    moveSelectedItem(-1);
    updateButtonState();
}

void genericTreeListDialog::OnTreeSelectionChange(wxTreeListEvent&)
{
    updateButtonState();
}

void genericTreeListDialog::OnDown(wxCommandEvent&)
{
    moveSelectedItem(1);
    updateButtonState();
}

void genericTreeListDialog::OnBottom(wxCommandEvent&)
{
    wxTreeListItem sel = m_treeList->GetSelection();
    wxTreeListItem parent = m_treeList->GetItemParent(sel);
    wxTreeListItem newItem = m_treeList->InsertItem(parent, wxTLI_LAST, m_treeList->GetItemText(sel, 0));

    moveItemData(sel, newItem);

    updateButtonState();
}

void genericTreeListDialog::OnClose(wxCommandEvent&)
{
    closeAction();
    EndModal(wxID_CLOSE);
}

void genericTreeListDialog::OnCloseWin(wxCloseEvent&)  // for dialog menu
{
    closeAction();
    EndModal(wxID_CLOSE);
}

void genericTreeListDialog::OnDefault(wxCommandEvent&)
{
    if (wxMessageBox(_tu("Do you really want to restore the default values?\n\nAll customization will be lost!")
        , _tu("Restore default")
        , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION) == wxYES)
    {
        setDefault();
        reloadTree();
    }
}
