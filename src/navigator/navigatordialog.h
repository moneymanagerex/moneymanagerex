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

#pragma once

#include <wx/wx.h>
#include <wx/treelist.h>
#include <wx/sizer.h>
#include <vector>

#include "daterange2.h"
#include "navigatortypes.h"


typedef std::vector<wxTreeListItem> wxTreeListItems;


class mmNavigatorDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmNavigatorDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmNavigatorDialog();
    mmNavigatorDialog(wxWindow* parent);

    enum {
        BTN_BASE = 0,
        BTN_UP_TOP,
        BTN_UP,
        BTN_EDIT,
        BTN_NEW,
        BTN_DOWN,
        BTN_DOWN_BOTTOM,
        BTN_DELETE,
        BTN_DEFAULT
    };

private:
    wxTreeListCtrl* m_treeList;
    wxButton* m_up_top = nullptr;
    wxBitmapButton* m_up = nullptr;
    wxButton* m_edit = nullptr;
    wxBitmapButton* m_down = nullptr;
    wxButton* m_down_bottom = nullptr;
    wxButton* m_delete = nullptr;

    void createControls();
    void fillControls();
    wxTreeListItem appendAccountItem(wxTreeListItem parent, NavigatorTypesInfo* ainfo);

    void OnTop(wxCommandEvent&);
    void OnUp(wxCommandEvent&);
    void OnEdit(wxCommandEvent&);
    void OnDown(wxCommandEvent&);
    void OnBottom(wxCommandEvent&);
    void OnNew(wxCommandEvent&);
    void OnClose(wxCommandEvent&);
    void OnCancel(wxCommandEvent&);
    void OnDelete(wxCommandEvent&);
    void OnDefault(wxCommandEvent&);
    void OnTreeSelectionChange(wxTreeListEvent&);
    void OnTreeItemChecked(wxTreeListEvent&);

    void updateButtonState();
    void moveSelectedItem(int direction);
    void moveItemData(wxTreeListItem sel, wxTreeListItem newItem);
    wxTreeListItems getChildrenList(wxTreeListItem parent);
    void cloneSubtree(wxTreeListItem src, wxTreeListItem dstParent);
    int findItemIndex(const wxTreeListItems& items, const wxTreeListItem& target);
    void copyTreeItemData(wxTreeListItem src, wxTreeListItem dst);
    void updateItemsRecursive(wxTreeListItem item);
};
