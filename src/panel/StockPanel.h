/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2010-2021 Nikolay Akimov
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)
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

#include "model/TrxShareModel.h"
#include "mmframe.h"
#include "_PanelBase.h"
#include "StockList.h"

class wxListEvent;
class TrxShareModel;

class StockPanel : public PanelBase
{
    friend class StockList;

    wxDECLARE_EVENT_TABLE();

private:
    int64 m_account_id = -1;
    const CurrencyData* m_currency_n = nullptr;
    wxString m_last_update;
    wxDateTime m_last_refresh;
    bool m_refresh_status;

    mmGUIFrame*     w_frame;
    StockList*      w_list           = nullptr;
    wxStaticText*   w_header_title   = nullptr;
    wxStaticText*   w_header_total   = nullptr;
    wxStaticText*   w_details        = nullptr;
    wxStaticText*   w_details_short  = nullptr;
    wxChoice*       w_filter_choice  = nullptr;
    wxBitmapButton* w_attachment_btn = nullptr;
    wxBitmapButton* w_refresh_btn    = nullptr;

public:
    StockPanel(
        int64 account_id,
        mmGUIFrame* frame,
        wxWindow* parent_win,
        wxWindowID win_id = mmID_STOCKS
    );
    ~StockPanel();

    // override PanelBase
    virtual auto buildPage() const -> wxString override;
    virtual void sortList() override {}

    void refreshList();
    void displayAccountDetails(int64 account_id);

private:
    bool create(
        wxWindow* parent_win,
        wxWindowID win_id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "StockPanel"
    );
    void createControls();

    void enableEditDeleteButtons(bool en);
    void onListItemActivated(int selectedIndex);
    int  addStockTransaction(int selectedIndex);
    void onListItemSelected(int selectedIndex);

    void viewStockTransactions(int selectedIndex);
    auto initStockTxnListCtrl(wxWindow* parent) -> wxListCtrl*;
    void loadStockTransactions(wxListCtrl* listCtrl, wxString symbol, int64 stockId);
    void fillListRow(wxListCtrl* listCtrl, long index, const TrxData& txn, const TrxShareData& share_entry);
    void bindListEvents(wxListCtrl* listCtrl);
    void copySelectedRowsToClipboard(wxListCtrl* listCtrl);
    int  getFilter();
    void updateHeader();

    void call_dialog(int selectedIndex);
    auto totalShares() -> const wxString;

    void updateExtraStocksData(int selIndex);
    bool onlineQuoteRefresh(wxString& sError);
    auto getPanelTitle(const AccountData& account) const -> wxString;

    // Event handlers
    void onNewStocks(wxCommandEvent& event) { w_list->onNewStocks(event); }
    void onDeleteStocks(wxCommandEvent& event) { w_list->onDeleteStocks(event); }
    void onMoveStocks(wxCommandEvent& event) { w_list->onMoveStocks(event); }
    void onEditStocks(wxCommandEvent& event) { w_list->onEditStocks(event); }
    void onOpenAttachment(wxCommandEvent& event) { w_list->onOpenAttachment(event); }
    void onRefreshQuotes(wxCommandEvent& event);
};
