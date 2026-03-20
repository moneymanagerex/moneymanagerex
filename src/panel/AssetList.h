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
#include "_PanelBase.h"
#include "mmframe.h"

class wxListEvent;
class wxButton;
class AssetPanel;

/* Custom ListCtrl class that implements virtual LC style */
class AssetList: public ListBase
{
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

    AssetPanel* m_panel = nullptr;

public:
    AssetList(AssetPanel* cp, wxWindow *parent, wxWindowID winid = wxID_ANY);

protected:
    virtual int getSortIcon(bool asc) const override;
    virtual void OnColClick(wxListEvent& event) override;

private:
    // required overrides for virtual style list control
    virtual wxString OnGetItemText(long item, long col_nr) const override;
    virtual int OnGetItemImage(long item) const override;

public:
    void OnNewAsset(wxCommandEvent& event);
    void OnEditAsset(wxCommandEvent& event);
    void OnDeleteAsset(wxCommandEvent& event);
    void OnDuplicateAsset(wxCommandEvent& event);
    void OnOrganizeAttachments(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);
    void OnAddAssetTrans(wxCommandEvent& WXUNUSED(event));
    void OnViewAssetTrans(wxCommandEvent& WXUNUSED(event));
    void OnGotoAssetAccount(wxCommandEvent& WXUNUSED(event));

    void doRefreshItems(int64 trx_id = -1);

private:
    void OnMouseRightClick(wxMouseEvent& event);
    void OnListLeftClick(wxMouseEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnListKeyDown(wxListEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnEndLabelEdit(wxListEvent& event);
    bool EditAsset(AssetData* pEntry);
};
