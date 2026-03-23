/*******************************************************
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

#include "model/AssetModel.h"
#include "model/AccountModel.h"
#include "_ListBase.h"
#include "mmframe.h"

class wxListEvent;
class wxButton;
class AssetPanel;

/* Custom ListCtrl class that implements virtual LC style */
class AssetList: public ListBase
{
    friend class AssetPanel;

public:
    enum LIST_ID
    {
        LIST_ID_ICON = 0,
        LIST_ID_ID,
        LIST_ID_NAME,
        LIST_ID_DATE,
        LIST_ID_TYPE,
        LIST_ID_VALUE_INITIAL,
        LIST_ID_VALUE_CURRENT,
        LIST_ID_NOTES,
        LIST_ID_size, // number of columns
    };

private:
    DECLARE_NO_COPY_CLASS(AssetList)
    wxDECLARE_EVENT_TABLE();
    enum {
        MENU_TREEPOPUP_NEW = wxID_HIGHEST + 1200,
        MENU_TREEPOPUP_ADDTRANS,
        MENU_TREEPOPUP_VIEWTRANS,
        MENU_TREEPOPUP_GOTOACCOUNT,
        MENU_TREEPOPUP_EDIT,
        MENU_TREEPOPUP_DELETE,
        MENU_ON_DUPLICATE_TRANSACTION,
        MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS,
    };

private:
    static const std::vector<ListColumnInfo> LIST_INFO;
    long m_selected_row = -1;

    AssetPanel* w_panel = nullptr;

public:
    AssetList(AssetPanel* panel, wxWindow* parent_win, wxWindowID win_id = wxID_ANY);

private:
    // override ListBase
    virtual void onColClick(wxListEvent& event) override;
    virtual int  getSortIcon(bool asc) const override;

    // override wxListCtrl
    virtual auto OnGetItemText(long item, long col_nr) const -> wxString override;
    virtual int  OnGetItemImage(long item) const override;

    void doRefreshItems(int64 trx_id = -1);
    bool editAsset(AssetData* asset_n);

    void onNewAsset(wxCommandEvent& event);
    void onEditAsset(wxCommandEvent& event);
    void onDeleteAsset(wxCommandEvent& event);
    void onDuplicateAsset(wxCommandEvent& event);
    void onOrganizeAttachments(wxCommandEvent& event);
    void onOpenAttachment(wxCommandEvent& event);
    void onAddAssetTrans(wxCommandEvent& WXUNUSED(event));
    void onViewAssetTrans(wxCommandEvent& WXUNUSED(event));
    void onGotoAssetAccount(wxCommandEvent& WXUNUSED(event));
    void onMouseRightClick(wxMouseEvent& event);
    void onListLeftClick(wxMouseEvent& event);
    void onListItemActivated(wxListEvent& event);
    void onListKeyDown(wxListEvent& event);
    void onListItemSelected(wxListEvent& event);
    void onEndLabelEdit(wxListEvent& event);
};
