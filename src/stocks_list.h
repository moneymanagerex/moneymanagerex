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
    enum LIST_COL
    {
        LIST_COL_ICON = 0,
        LIST_COL_ID,
        LIST_COL_DATE,
        LIST_COL_NAME,
        LIST_COL_SYMBOL,
        LIST_COL_NUMBER,
        LIST_COL_PRICE,
        LIST_COL_VALUE,
        LIST_COL_REAL_GAIN_LOSS,
        LIST_COL_GAIN_LOSS,
        LIST_COL_CURRENT,
        LIST_COL_CURRVALUE,
        LIST_COL_PRICEDATE,
        LIST_COL_COMMISSION,
        LIST_COL_NOTES,
        LIST_COL_size, // number of columns
    };

private:
    static const std::vector<ListColumnInfo> col_info_all();
    int col_sort();

public:
    StocksListCtrl(mmStocksPanel* cp, wxWindow *parent, wxWindowID winid = wxID_ANY);
    ~StocksListCtrl();

    void doRefreshItems(int64 trx_id = -1);
    void OnNewStocks(wxCommandEvent& event);
    void OnDeleteStocks(wxCommandEvent& event);
    void OnMoveStocks(wxCommandEvent& event);
    void OnEditStocks(wxCommandEvent& event);
    void OnOrganizeAttachments(wxCommandEvent& event);
    void OnStockWebPage(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);
    long get_selectedIndex();
    int getColumnsNumber();
    wxString getStockInfo(int selectedIndex) const;
    /* Helper Functions/data */
    Model_Stock::Data_Set m_stocks;
    /* updates thstockide checking panel data */
    int initVirtualListControl(int64 trx_id = -1, int col = 0, bool asc = true);

private:
    /* required overrides for virtual style list control */
    virtual wxString OnGetItemText(long item, long column) const;
    virtual int OnGetItemImage(long item) const;

    void OnMouseRightClick(wxMouseEvent& event);
    void OnListLeftClick(wxMouseEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnColClick(wxListEvent& event);
    void OnMarkTransaction(wxCommandEvent& event);
    void OnMarkAllTransactions(wxCommandEvent& event);
    void OnListKeyDown(wxListEvent& event);
    void OnListItemSelected(wxListEvent& event);

    mmStocksPanel* m_stock_panel;

    double GetGainLoss(long item) const;
    static double getGainLoss(const Model_Stock::Data& stock);
    double GetRealGainLoss(long item) const;
    static double getRealGainLoss(const Model_Stock::Data& stock);
    void sortTable();
};

inline long StocksListCtrl::get_selectedIndex() { return m_selected_row; }
inline int StocksListCtrl::getColumnsNumber() { return LIST_COL_size; }
inline int StocksListCtrl::col_sort() { return LIST_COL_DATE; }

#endif
