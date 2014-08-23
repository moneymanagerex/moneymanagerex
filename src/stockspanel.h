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
#include <wx/tglbtn.h>
#include "model/Model_Stock.h"
#include "model/Model_Currency.h"
#include "model/Model_Account.h"

static const wxString STOCKTIPS[] = { 
    _("Using MMEX it is possible to track stocks/mutual funds investments."),
    _("To create new stocks entry the Symbol, Number of shares and Purchase prise should be entered."),
    _("Sample of UK (HSBC HLDG) share: HSBA.L"),
    _("If the Stock Name field is empty it will be filed when prices updated") };

class wxListEvent;
class mmStocksPanel;

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
    void OnOpenAttachment(wxCommandEvent& event);
    long get_selectedIndex() { return m_selected_row; }
    int getColumnsNumber() { return ColName_.size(); }
    wxString getStockInfo(int selectedIndex) const;
    /* Helper Functions/data */
    Model_Stock::Data_Set m_stocks;
    /* updates thstockide checking panel data */
    int initVirtualListControl(int trx_id = -1, int col = 0, bool asc = true);

private:
    void save_column_width(int width);
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
    void OnItemResize(wxListEvent& event);

    mmStocksPanel* stock_panel_;
    enum EColumn
    {
        COL_DATE = 0,
        COL_NAME,
        COL_NUMBER,
        COL_VALUE,
        COL_GAIN_LOSS,
        COL_CURRENT,
        COL_PRICEDATE,
        COL_NOTES,
        COL_MAX, // number of columns
    };
    std::map<int, wxString> ColName_;
    wxImageList* m_imageList;
    double getGainLoss(long item) const;
    void sortTable();
};

/* ------------------------------------------------------- */
class mmStocksPanel : public mmPanelBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmStocksPanel(
        int accountID,
        wxWindow *parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = wxPanelNameStr
    );
    ~mmStocksPanel();

    bool Create( wxWindow *parent, wxWindowID winid,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                 const wxString& name = wxPanelNameStr);

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
    void OnListItemSelected(int selectedIndex);
    //void OnViewPopupSelected(wxCommandEvent& event);

    int accountID_;
    Model_Currency::Data * m_currency;
    StocksListCtrl* listCtrlAccount_;
    void updateExtraStocksData(int selIndex);
    wxStaticText* stock_details_short_;
    void updateHeader();

    wxString BuildPage() const;

private:
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
