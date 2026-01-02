/*******************************************************
 Copyright (C) 2025, 2026 Klaus Wich

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

#pragma once

#include "defs.h"


typedef std::vector<wxTreeListItem> wxTreeListItems;


class genericTreeListDialog: public wxDialog
{
private:
    wxDECLARE_DYNAMIC_CLASS(genericTreeListDialog);

public:
    genericTreeListDialog();
    virtual ~genericTreeListDialog();
    genericTreeListDialog(wxWindow* parent, wxString title);

protected:
    enum {
        BTN_BASE = 0,
        BTN_UP_TOP,
        BTN_UP,
        BTN_DOWN,
        BTN_DOWN_BOTTOM,
        BTN_DEFAULT,
        BTN_size
    };

    wxTreeListCtrl* m_treeList = nullptr;

    virtual void createColumns();
    virtual void closeAction();
    virtual void createMiddleElements(wxBoxSizer* itemBox);
    virtual void createBottomElements(wxBoxSizer* itemBox);
    virtual void fillControls(wxTreeListItem root);
    virtual void updateControlState(int selIdx, wxClientData* selData);
    virtual void setDefault();
    virtual void copyTreeItemData(wxTreeListItem src, wxTreeListItem dst);

    void init(long liststyle = wxTL_3STATE | wxTL_SINGLE);
    void updateButtonState();
    void reloadTree();

private:
    wxButton*       m_up_top = nullptr;
    wxBitmapButton* m_up = nullptr;
    wxBitmapButton* m_down = nullptr;
    wxButton*       m_down_bottom = nullptr;

    void moveSelectedItem(int direction);
    void moveItemData(wxTreeListItem sel, wxTreeListItem newItem);
    wxTreeListItems getChildrenList(wxTreeListItem parent);
    void cloneSubtree(wxTreeListItem src, wxTreeListItem dstParent);
    int findItemIndex(const wxTreeListItems& items, const wxTreeListItem& target);

    void OnTop(wxCommandEvent&);
    void OnUp(wxCommandEvent&);
    void OnDown(wxCommandEvent&);
    void OnBottom(wxCommandEvent&);
    void OnDefault(wxCommandEvent&);
    void OnTreeSelectionChange(wxTreeListEvent&);
    void OnClose(wxCommandEvent&);
    void OnCloseWin(wxCloseEvent&);
};
