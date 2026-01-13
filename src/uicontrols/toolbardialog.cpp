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

#include "toolbardialog.h"
#include "mmframe.h"

struct TypeDataRef : public wxClientData
{
    ToolBarEntries::ToolBarEntry* ref;
    explicit TypeDataRef(ToolBarEntries::ToolBarEntry* r): ref(r) {}
};


wxIMPLEMENT_DYNAMIC_CLASS(mmToolbarDialog, wxDialog);

mmToolbarDialog::mmToolbarDialog()
{
    m_delete = nullptr;
}

mmToolbarDialog::~mmToolbarDialog()
{
    Model_Infotable::instance().setSize(DIALOG_SIZE, GetSize());
}

mmToolbarDialog::mmToolbarDialog(wxWindow* parent):genericTreeListDialog(parent, _t("Toolbar configuration"))
{
    m_delete = nullptr;
    init(wxTL_3STATE | wxTL_SINGLE | wxTL_NO_HEADER);
    SetSize(Model_Infotable::instance().getSize(DIALOG_SIZE));
}

void mmToolbarDialog::createColumns() {
    m_treeList->AppendColumn("");
    wxImageList* imageList = ToolBarEntries::instance().getImageList();
    m_treeList->SetImageList(imageList);

#ifdef __WXMAC__
    int iconWidth = imageList->GetSize().GetWidth();
    int spaceWidth = m_treeList->GetTextExtent(" ").GetWidth();
    m_spaces = (iconWidth / spaceWidth) + 1;
#endif
}

void mmToolbarDialog::closeAction() {
    updateTree();
    ToolBarEntries::instance().Save();
    mmGUIFrame* mainFrame = wxDynamicCast(this->GetParent(), mmGUIFrame);
    if (mainFrame) {
        mainFrame->PopulateToolBar();
    }
};

void mmToolbarDialog::createBottomElements(wxBoxSizer* itemBox) {
    itemBox->AddSpacer(50);
    itemBox->Add(new wxButton(this, BTN_NEW_SEPARATOR, _t("Add separator")), g_flagsV);
    Bind(wxEVT_BUTTON, &mmToolbarDialog::OnNew, this, BTN_NEW_SEPARATOR);

    itemBox->Add(new wxButton(this, BTN_NEW_SPACE, _t("Add space")), g_flagsV);
    Bind(wxEVT_BUTTON, &mmToolbarDialog::OnNew, this, BTN_NEW_SPACE);

    itemBox->Add(new wxButton(this, BTN_NEW_STRETCHER, _t("Add stretch space")), g_flagsV);
    Bind(wxEVT_BUTTON, &mmToolbarDialog::OnNew, this, BTN_NEW_STRETCHER);

    itemBox->AddSpacer(10);
    m_delete = new wxButton(this, BTN_DELETE, _t("&Delete"));
    m_delete->Enable(false);
    m_delete->Bind(wxEVT_BUTTON, &mmToolbarDialog::OnDelete, this);
    itemBox->Add(m_delete, g_flagsV);

    itemBox->AddSpacer(70);
}

void mmToolbarDialog::updateControlState(int WXUNUSED(selIdx), wxClientData* WXUNUSED(selData))
{
    bool enable = false;
    wxTreeListItem sel = m_treeList->GetSelection();
    if (sel.IsOk()) {
        TypeDataRef* data = static_cast<TypeDataRef*> (m_treeList->GetItemData(sel));
        if (data) {
            enable = data->ref->type != ToolBarEntries::TOOLBAR_BTN;
        }
    }
    m_delete->Enable(enable);
}

void mmToolbarDialog::setDefault() {
    ToolBarEntries::instance().SetToDefault();
    m_treeList->SetImageList(ToolBarEntries::instance().getImageList());
    reloadTree();
}

void mmToolbarDialog::fillControls(wxTreeListItem root)
{
    ToolBarEntries::ToolBarEntry* ainfo = ToolBarEntries::instance().getFirstEntry();
    while (ainfo != nullptr) {
        appendItem(root, ainfo);
        ainfo = ToolBarEntries::instance().getNextEntry(ainfo);
    }
}

wxTreeListItem mmToolbarDialog::appendItem(wxTreeListItem parent, ToolBarEntries::ToolBarEntry* ainfo)
{
   wxString text = ainfo->type == ToolBarEntries::TOOLBAR_BTN ? wxGetTranslation(ainfo->helpstring) :
        (ainfo->type == ToolBarEntries::TOOLBAR_SEPARATOR ? std::string(60, '-') :
        (ainfo->type == ToolBarEntries::TOOLBAR_STRETCH ? "| <<<<<=== " + _t("Spacer") + " ===>>>>> |" : "|         " + _t("Spacer") + "         |"));
#ifdef __WXMAC__
    if (ainfo->type == ToolBarEntries::TOOLBAR_BTN) {
        text.Prepend(wxString(' ', m_spaces));
    }
#endif
    wxTreeListItem item = m_treeList->AppendItem(parent, text);
    if (ainfo->type == ToolBarEntries::TOOLBAR_BTN) {
        m_treeList->SetItemImage(item, ainfo->imageListID);
    }
    m_treeList->CheckItem(item, ainfo->active ? wxCHK_CHECKED : wxCHK_UNCHECKED);
    m_treeList->SetItemData(item, new TypeDataRef(ainfo));
    return item;
}

void mmToolbarDialog::OnDelete(wxCommandEvent&)
{
    updateTree();
    wxTreeListItem item = m_treeList->GetSelection();
    TypeDataRef* data = static_cast<TypeDataRef*> (m_treeList->GetItemData(item));
    if (ToolBarEntries::instance().DeleteEntry(data->ref)) {
        reloadTree();
    }
    updateButtonState();
}

void mmToolbarDialog::OnNew(wxCommandEvent& event)
{
    updateTree();
    ToolBarEntries::ToolBarEntry* ainfo = nullptr;
    wxTreeListItem sel_item = m_treeList->GetSelection();
    if (sel_item.IsOk()) {
        TypeDataRef* data = static_cast<TypeDataRef*> (m_treeList->GetItemData(sel_item));
        ainfo = data->ref;
    }
    ToolBarEntries::instance().newEntry(event.GetId() == BTN_NEW_SEPARATOR ? ToolBarEntries::TOOLBAR_SEPARATOR :
                                        (event.GetId() == BTN_NEW_SPACE ? ToolBarEntries::TOOLBAR_SPACER : ToolBarEntries::TOOLBAR_STRETCH), ainfo);
    reloadTree();
    updateButtonState();
}

void mmToolbarDialog::copyTreeItemData(wxTreeListItem src, wxTreeListItem dst) {
    const int colCount = m_treeList->GetColumnCount();
    for (int c = 1; c < colCount; ++c) {
        m_treeList->SetItemText(dst, c, m_treeList->GetItemText(src, c));
    }
    TypeDataRef* data = static_cast<TypeDataRef*> (m_treeList->GetItemData(src));
    data->ref->active = m_treeList->GetCheckedState(src) == wxCHK_CHECKED;
    if (data->ref->type == ToolBarEntries::TOOLBAR_BTN) {
        m_treeList->SetItemImage(dst, data->ref->imageListID);
    }
    m_treeList->SetItemData(dst, new TypeDataRef(data->ref));
    m_treeList->CheckItem(dst, data->ref->active ? wxCHK_CHECKED : wxCHK_UNCHECKED);
}

void mmToolbarDialog::updateTree()
{
    int idx = 0;
    // reindex tree:
    for (wxTreeListItem item = m_treeList->GetFirstItem(); item.IsOk(); item = m_treeList->GetNextItem(item) ) {
        TypeDataRef* data = static_cast<TypeDataRef*> (m_treeList->GetItemData(item));
        data->ref->seq_no = idx++;
        data->ref->active = m_treeList->GetCheckedState(item) == wxCHK_CHECKED;
    }
    ToolBarEntries::instance().SortEntriesBySeq();
}
