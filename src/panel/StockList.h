/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2010-2021 Nikolay Akimov
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

#include "base/defs.h"
#include <wx/tglbtn.h>
#include "model/StockModel.h"
#include "_ListBase.h"

class wxListEvent;
class StockPanel;

/* Custom ListCtrl class that implements virtual LC style */
class StockList: public ListBase
{
    friend class StockPanel;

    DECLARE_NO_COPY_CLASS(StockList)
    wxDECLARE_EVENT_TABLE();

public:
    enum LIST_ID
    {
        LIST_ID_ICON = 0,
        LIST_ID_ID,
        LIST_ID_DATE,
        LIST_ID_NAME,
        LIST_ID_SYMBOL,
        LIST_ID_NUMBER,
        LIST_ID_PRICE,
        LIST_ID_VALUE,
        LIST_ID_REAL_GAIN_LOSS,
        LIST_ID_GAIN_LOSS,
        LIST_ID_CURRENT,
        LIST_ID_CURRVALUE,
        LIST_ID_PRICEDATE,
        LIST_ID_COMMISSION,
        LIST_ID_NOTES,
        LIST_ID_size, // number of columns
    };

private:
    static const std::vector<ListColumnInfo> LIST_INFO;

    StockModel::DataA m_stock_a;
    long m_select_n = -1;
    double m_invest_value;
    double m_market_value;

    StockPanel* w_panel;
    wxSharedPtr<wxListItemAttr> w_loss_attr1;  // Style for loss
    wxSharedPtr<wxListItemAttr> w_loss_attr2;  // style for loss alternate

public:
    StockList(
        StockPanel* panel,
        wxWindow* parent_win,
        wxWindowID win_id = wxID_ANY
    );
    ~StockList();

    // override ListBase
    virtual void onColClick(wxListEvent& event) override;
    virtual int  getSortIcon(bool asc) const override;

    // override wxListCtrl
    virtual auto OnGetItemText(long item, long col_nr) const -> wxString override;
    virtual int  OnGetItemImage(long item) const override;
    virtual auto OnGetItemAttr(long item) const -> wxListItemAttr* override;

    void doRefreshItems(int64 trx_id = -1);
    long get_selectedIndex() { return m_select_n; }
    auto getStockInfo(int selectedIndex, bool addtotal) const -> wxString;
    int  initVirtualListControl(int64 trx_id = -1);
    void getInvestmentBalance(double& invest_value, double& market_value);

private:
    static int  col_sort() { return LIST_ID_DATE; }
    static auto getRealGainLoss(const StockData& stock_d) -> double;

    void sortList();
    void createSummary();
    auto getGainLoss(long item) const -> double {
        return m_stock_a[item].current_gain();
    }
    auto getRealGainLoss(long item) const -> double {
        return StockList::getRealGainLoss(m_stock_a[item]);
    }

    void onNewStocks(wxCommandEvent& event);
    void onDeleteStocks(wxCommandEvent& event);
    void onMoveStocks(wxCommandEvent& event);
    void onEditStocks(wxCommandEvent& event);
    void onOrganizeAttachments(wxCommandEvent& event);
    void onStockWebPage(wxCommandEvent& event);
    void onOpenAttachment(wxCommandEvent& event);
    void onMouseRightClick(wxMouseEvent& event);
    void onListLeftClick(wxMouseEvent& event);
    void onListItemActivated(wxListEvent& event);
    void onMarkTransaction(wxCommandEvent& event);
    void onMarkAllTransactions(wxCommandEvent& event);
    void onListKeyDown(wxListEvent& event);
    void onListItemSelected(wxListEvent& event);
};
