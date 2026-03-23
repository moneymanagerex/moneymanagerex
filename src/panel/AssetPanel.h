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
#include "AssetList.h"
#include "mmframe.h"

class wxButton;

class AssetPanel : public PanelBase
{
    friend class AssetList;

public:
    enum EIcons
    {
        ICON_PROPERTY,
        ICON_CAR,
        ICON_HOUSEHOLD_OBJ,
        ICON_ART,
        ICON_JEWELLERY,
        ICON_CASH,
        ICON_OTHER,
        ICON_UPARROW,   // the 'ARROW's need to be last in the list
        ICON_DOWNARROW
    };

private:
    wxDECLARE_EVENT_TABLE();
    enum {
        IDC_PANEL_ASSET_STATIC_DETAILS = wxID_HIGHEST + 1220,
        IDC_PANEL_ASSET_STATIC_DETAILS_MINI,
    };

private:
    AssetModel::DataA m_asset_a;
    int m_asset_type_id_n; // -1 (All), AssetType
    wxString m_tip;

    mmGUIFrame*   w_frame      = nullptr;
    AssetList*    w_list       = nullptr;
    wxStaticText* w_header     = nullptr;
    wxButton*     w_filter_btn = nullptr;

public:
    AssetPanel(
        mmGUIFrame* frame,
        wxWindow* parent_win,
        wxWindowID win_id,
        const wxString& name="AssetPanel"
    );

public:
    // override PanelBase
    virtual auto buildPage() const -> wxString override {
        return w_list->buildPage(_t("Assets"));
    }
    virtual void sortList() override;

    void refreshList() { w_list->doRefreshItems(); }

private:
    bool create(
        wxWindow* parent_win,
        wxWindowID win_id,
        const wxPoint& pos,
        const wxSize& size,
        long style,
        const wxString& name
    );
    void createControls();
    void setAccountParameters(const AccountData* account);
    void enableEditDeleteButtons(bool enable);
    void addAssetTrans(const int selected_index);
    void viewAssetTrans(const int selected_index);
    auto initAssetTxnListCtrl(wxWindow* parent) -> wxListCtrl*;
    void loadAssetTransactions(wxListCtrl* listCtrl, int64 assetId);
    void fillAssetListRow(wxListCtrl* listCtrl, long index, const TrxData& txn);
    void bindAssetListEvents(wxListCtrl* listCtrl);
    void copySelectedRowsToClipboard(wxListCtrl* listCtrl);
    void gotoAssetAccount(const int selected_index);
    void updateExtraAssetData(int selIndex);
    int  initVirtualListControl(int64 trx_id = -1);
    auto getItem(long item, int col_id) -> wxString;

    // Event handlers
    void onNewAsset(wxCommandEvent& event) { w_list->onNewAsset(event); }
    void onDeleteAsset(wxCommandEvent& event) { w_list->onDeleteAsset(event); }
    void onEditAsset(wxCommandEvent& event) { w_list->onEditAsset(event); }
    void onOpenAttachment(wxCommandEvent& event) { w_list->onOpenAttachment(event); }
    void onAddAssetTrans(wxCommandEvent& event) { w_list->onAddAssetTrans(event); }
    void onViewAssetTrans(wxCommandEvent& event) { w_list->onViewAssetTrans(event); }
    void onMouseLeftDown(wxCommandEvent& event);
    void onViewPopupSelected(wxCommandEvent& event);
    void onSearchTxtEntered(wxCommandEvent& event);
};
