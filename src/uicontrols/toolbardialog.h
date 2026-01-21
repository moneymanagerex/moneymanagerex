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
#include "generic/generictreelistdialog.h"
#include "toolbartypes.h"
#include "constants.h"


class mmToolbarDialog: public genericTreeListDialog
{
private:
    wxDECLARE_DYNAMIC_CLASS(mmToolbarDialog);

    enum {
        BTN_NEW_SEPARATOR = BTN_size,
        BTN_NEW_SPACE,
        BTN_NEW_STRETCHER,
        BTN_DELETE,
    };

public:
    mmToolbarDialog();
    ~mmToolbarDialog() override;
    mmToolbarDialog(wxWindow* parent);

private:
    const std::string DIALOG_SIZE  = "TOOLBAR_DIALOG_SIZE";

#ifdef __WXMAC__
    int m_spaces;   // For MAC workaround
#endif

    wxButton* m_delete;

    void fillControls(wxTreeListItem root) override;
    void createColumns() override;
    wxTreeListItem appendItem(wxTreeListItem parent, ToolBarEntries::ToolBarEntry* ainfo);
    void createBottomElements(wxBoxSizer* itemBox) override;
    void updateControlState(int selIdx, wxClientData* selData) override;
    void copyTreeItemData(wxTreeListItem src, wxTreeListItem dst) override;
    void closeAction() override;
    void setDefault() override;

    void OnNew(wxCommandEvent&);
    void OnDelete(wxCommandEvent&);

    void updateTree();
};
