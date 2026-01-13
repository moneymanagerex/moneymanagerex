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
#include "navigatortypes.h"
#include "generic/generictreelistdialog.h"


typedef std::vector<wxTreeListItem> wxTreeListItems;


class mmNavigatorDialog: public genericTreeListDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmNavigatorDialog);

public:
    mmNavigatorDialog();
    ~mmNavigatorDialog();
    mmNavigatorDialog(wxWindow* parent);

    enum {
        BTN_EDIT = BTN_size,
        BTN_NEW,
        BTN_DELETE,
        BTN_RESET_NAMES
    };

protected:
    void createColumns() override;
    void closeAction() override;
    void createMiddleElements(wxBoxSizer* itemBox) override;
    void createBottomElements(wxBoxSizer* itemBox) override;
    void fillControls(wxTreeListItem root) override;
    void updateControlState(int selIdx, wxClientData* selData) override;
    void setDefault() override;
    void copyTreeItemData(wxTreeListItem src, wxTreeListItem dst) override;

private:
    const std::string DIALOG_SIZE  = "NAVIGATOR_DIALOG_SIZE";

#ifdef __WXMAC__
    int m_spaces;   // For MAC workaround
#endif

    wxButton* m_edit = nullptr;
    wxButton* m_delete = nullptr;

    wxTreeListItem appendAccountItem(wxTreeListItem parent, NavigatorTypesInfo* ainfo);
    void updateItemsRecursive(wxTreeListItem item);

    void OnEdit(wxCommandEvent&);
    void OnNew(wxCommandEvent&);
    void OnDelete(wxCommandEvent&);
    void OnNameReset(wxCommandEvent&);
    void OnTreeItemChecked(wxTreeListEvent&);
};
