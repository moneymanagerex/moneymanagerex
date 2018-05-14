/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#ifndef MM_EX_STOCKSPANEL_H_
#define MM_EX_STOCKSPANEL_H_

#include "mmpanelbase.h"
#include "Model_Stock.h"
#include "Model_Currency.h"
#include "Model_Account.h"
class wxListEvent;
class mmStocksPanel;
class mmGUIFrame;
class wxStaticText;
class wxBitmapButton;

/* Custom ListCtrl class that implements virtual LC style */
class StocksListCtrl: public mmListCtrl
{
    DECLARE_NO_COPY_CLASS(StocksListCtrl)
    wxDECLARE_EVENT_TABLE();

public:
    StocksListCtrl(mmStocksPanel* cp, wxWindow *parent, wxWindowID winid = wxID_ANY);
    ~StocksListCtrl();

    void doRefreshItems(int trx_id = -1);
    void OnNewStocks(wxCommandEvent& event);
    void OnDeleteStocks(wxCommandEvent& event);
    void OnMoveStocks(wxCommandEvent& event);
    void OnEditStocks(wxCommandEvent& event);
    void OnOrganizeAttachments(wxCommandEvent& event);
    void OnStockWebPage(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);
    long get_selectedIndex() { return m_selected_row; }
    int getColumnsNumber() { return COL_MAX; }
    int col_sort() { return COL_DATE; }
    wxString getStockInfo(int selectedIndex) const;
    /* Helper Functions/data */
    Model_Stock::Data_Set m_stocks;
    /* updates thstockide checking panel data */
    int initVirtualListControl(int trx_id = -1, int col = 0, bool asc = true);

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
    enum EColumn
    {
        COL_ICON = 0,
        COL_ID,
        COL_DATE,
        COL_NAME,
        COL_SYMBOL,
        COL_NUMBER,
        COL_PRICE,
        COL_VALUE,
        COL_GAIN_LOSS,
        COL_CURRENT,
        COL_CURRVALUE,
        COL_PRICEDATE,
        COL_COMMISSION,
        COL_NOTES,
        COL_MAX, // number of columns
    };
    wxImageList* m_imageList;
    double GetGainLoss(long item) const;
    void sortTable();
};

/* ------------------------------------------------------- */
class mmStocksPanel : public mmPanelBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmStocksPanel(
        int accountID,
        mmGUIFrame* frame,
        wxWindow *parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "mmStocksPanel"
    );
    ~mmStocksPanel();

    bool Create( wxWindow *parent, wxWindowID winid,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                 const wxString& name = "mmStocksPanel");

    void CreateControls();

    /* Event handlers for Buttons */
    void OnNewStocks(wxCommandEvent& event);
    void OnDeleteStocks(wxCommandEvent& event);
    void OnMoveStocks(wxCommandEvent& event);
    void OnEditStocks(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);
    void OnRefreshQuotes(wxCommandEvent& event);
    //Unhide the Edit and Delete buttons if any record selected
    void enableEditDeleteButtons(bool en);
    void OnListItemActivated(int selectedIndex);
    void AddStockTransaction(int selectedIndex);
    void OnListItemSelected(int selectedIndex);
    //void OnViewPopupSelected(wxCommandEvent& event);

    void ViewStockTransactions(int selectedIndex);

    int m_account_id;
    Model_Currency::Data * m_currency;
    void updateExtraStocksData(int selIndex);
    wxStaticText* stock_details_short_;
    void updateHeader();

    wxString BuildPage() const;
    mmGUIFrame* m_frame;

private:
    StocksListCtrl* listCtrlAccount_;
    wxStaticText* stock_details_;
    void call_dialog(int selectedIndex);
    void sortTable() {}
    const wxString Total_Shares();

    wxStaticText* header_text_;
    wxStaticText* header_total_;
    wxBitmapButton* attachment_button_;
    wxBitmapButton* refresh_button_;

    bool DownloadIsRequired(void);
    bool onlineQuoteRefresh(wxString& sError);
    wxString GetPanelTitle(const Model_Account::Data& account) const;

    wxString strLastUpdate_;
    bool StocksRefreshStatus_;
    wxDateTime LastRefreshDT_;

};

#endif
