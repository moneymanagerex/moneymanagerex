/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2010-2021 Nikolay Akimov
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "stocks_list.h"
#include "mmpanelbase.h"
#include <wx/tglbtn.h>
#include "mmframe.h"

class wxListEvent;
class mmStocksPanel;

class mmStocksPanel : public mmPanelBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmStocksPanel(
        int64 accountID,
        mmGUIFrame* frame,
        wxWindow *parent,
        wxWindowID winid = mmID_STOCKS
    );
    ~mmStocksPanel();

    bool Create( wxWindow *parent, wxWindowID winid,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                 const wxString& name = "mmStocksPanel");

    void CreateControls();
    void DisplayAccountDetails(int64 accountID);
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
    void RefreshList();
    //void OnViewPopupSelected(wxCommandEvent& event);

    void ViewStockTransactions(int selectedIndex);

    int64 m_account_id = -1;
    Model_Currency::Data * m_currency = nullptr;
    void updateExtraStocksData(int selIndex);
    wxStaticText* stock_details_short_ = nullptr;
    void updateHeader();

    wxString BuildPage() const;
    mmGUIFrame* m_frame;

private:
    StocksListCtrl* listCtrlAccount_ = nullptr;
    wxStaticText* stock_details_ = nullptr;
    void call_dialog(int selectedIndex);
    void sortTable() {}
    const wxString Total_Shares();

    wxStaticText* header_text_ = nullptr;
    wxStaticText* header_total_ = nullptr;
    wxBitmapButton* attachment_button_ = nullptr;
    wxBitmapButton* refresh_button_ = nullptr;

    bool onlineQuoteRefresh(wxString& sError);
    wxString GetPanelTitle(const Model_Account::Data& account) const;

    wxString strLastUpdate_;
    bool StocksRefreshStatus_;
    wxDateTime LastRefreshDT_;

};

#endif
