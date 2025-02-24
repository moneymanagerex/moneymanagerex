/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2010-2021 Nikolay Akimov

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

#ifndef MM_EX_STOCKS_LIST_H_
#define MM_EX_STOCKS_LIST_H_

#include "mmpanelbase.h"
#include <wx/tglbtn.h>
#include "model/Model_Stock.h"

class wxListEvent;
class mmStocksPanel;

/* Custom ListCtrl class that implements virtual LC style */
class StocksListCtrl: public mmListCtrl
{
    DECLARE_NO_COPY_CLASS(StocksListCtrl)
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

public:
    Model_Stock::Data_Set m_stocks;

private:
    static const std::vector<ListColumnInfo> LIST_INFO;
    mmStocksPanel* m_stock_panel;
    long m_selected_row = -1;

public:
    StocksListCtrl(mmStocksPanel* cp, wxWindow *parent, wxWindowID winid = wxID_ANY);
    ~StocksListCtrl();

    void doRefreshItems(int64 trx_id = -1);
    long get_selectedIndex();
    wxString getStockInfo(int selectedIndex) const;
    int initVirtualListControl(int64 trx_id = -1);

    void OnNewStocks(wxCommandEvent& event);
    void OnDeleteStocks(wxCommandEvent& event);
    void OnMoveStocks(wxCommandEvent& event);
    void OnEditStocks(wxCommandEvent& event);
    void OnOrganizeAttachments(wxCommandEvent& event);
    void OnStockWebPage(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);

private:
    static int col_sort();
    double GetGainLoss(long item) const;
    static double getGainLoss(const Model_Stock::Data& stock);
    double GetRealGainLoss(long item) const;
    static double getRealGainLoss(const Model_Stock::Data& stock);
    void sortList();

    // required overrides for virtual style list control
    virtual int getSortIcon(bool asc) const override;
    virtual wxString OnGetItemText(long item, long col_nr) const override;
    virtual int OnGetItemImage(long item) const override;
    void OnColClick(wxListEvent& event) override;

    void OnMouseRightClick(wxMouseEvent& event);
    void OnListLeftClick(wxMouseEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnMarkTransaction(wxCommandEvent& event);
    void OnMarkAllTransactions(wxCommandEvent& event);
    void OnListKeyDown(wxListEvent& event);
    void OnListItemSelected(wxListEvent& event);
};

inline int StocksListCtrl::col_sort() { return LIST_ID_DATE; }
inline long StocksListCtrl::get_selectedIndex() { return m_selected_row; }

#endif
