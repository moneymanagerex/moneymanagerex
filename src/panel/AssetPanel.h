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

public:
    AssetModel::DataA m_assets;
    int m_filter_type; // -1 (All), AssetType
private:
    wxString tips_;

public:
    mmGUIFrame* m_frame = nullptr;
private:
    AssetList* m_lc = nullptr;
    wxButton* m_bitmapTransFilter = nullptr;
    wxStaticText* header_text_ = nullptr;

public:
    AssetPanel(
        mmGUIFrame* frame,
        wxWindow *parent,
        wxWindowID winid,
        const wxString& name="AssetPanel"
    );

    void updateExtraAssetData(int selIndex);
    int initVirtualListControl(int64 trx_id = -1);
    wxString getItem(long item, int col_id);

    wxString BuildPage() const { return m_lc->BuildPage(_t("Assets")); }

    void AddAssetTrans(const int selected_index);
    void ViewAssetTrans(const int selected_index);
    wxListCtrl* InitAssetTxnListCtrl(wxWindow* parent);
    void LoadAssetTransactions(wxListCtrl* listCtrl, int64 assetId);
    void FillAssetListRow(wxListCtrl* listCtrl, long index, const TrxData& txn);
    void BindAssetListEvents(wxListCtrl* listCtrl);
    void CopySelectedRowsToClipboard(wxListCtrl* listCtrl);
    void GotoAssetAccount(const int selected_index);
    void RefreshList() { m_lc->doRefreshItems(); }

private:
    void enableEditDeleteButtons(bool enable);
    void OnSearchTxtEntered(wxCommandEvent& event);

    bool Create(
        wxWindow *parent,
        wxWindowID winid,
        const wxPoint& pos,
        const wxSize& size,
        long style,
        const wxString& name
    );
    void CreateControls();

    /* Event handlers for Buttons */
    void OnNewAsset(wxCommandEvent& event);
    void OnDeleteAsset(wxCommandEvent& event);
    void OnEditAsset(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);
    void OnMouseLeftDown(wxCommandEvent& event);
    void OnAddAssetTrans(wxCommandEvent& event);
    void OnViewAssetTrans(wxCommandEvent& event);

    void OnViewPopupSelected(wxCommandEvent& event);
    void sortList();
    void SetAccountParameters(const AccountData* account);

};
