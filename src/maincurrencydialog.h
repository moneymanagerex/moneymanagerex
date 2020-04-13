/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 Gabriele-V

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

#include "defs.h"
#include <map>
#include <vector>
#include <wx/dataview.h>

class wxDatePickerCtrl;
class mmTextCtrl;

struct CurrencyHistoryRate
{
    wxString BaseCurrency;
    wxDateTime Date;
    wxString Currency;
    double Rate;
};

class mmMainCurrencyDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmMainCurrencyDialog);
    wxDECLARE_EVENT_TABLE();

public:
    /// Constructors
    mmMainCurrencyDialog( ) {}

    mmMainCurrencyDialog(wxWindow* parent
        , int currencyID = -1, bool bEnableSelect = true);

    static bool Execute(wxWindow* parent, int& currencyID);
    // Return the base currency
    static bool Execute(int& currencyID);

private:
    enum cols
    {
        CURR_BASE = 0,
        CURR_SYMBOL,
        CURR_NAME,
        BASE_RATE,
        ID_DIALOG = wxID_HIGHEST + 600,
        HISTORY_ADD,
        HISTORY_DELETE,
        HISTORY_UPDATE,
        HISTORY_DELUNUSED,
        MENU_ITEM1,
        MENU_ITEM2,
        MENU_ITEM3
    };

    bool Create(wxWindow* parent
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);

    /// Creates the controls and sizers

    void CreateControls();
    void OnBtnAdd(wxCommandEvent& event);
    void OnBtnEdit(wxCommandEvent& event);
    void OnBtnSelect(wxCommandEvent& event);
    void OnBtnDelete(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnListItemActivated(wxDataViewEvent& event);
    void OnListItemSelected(wxDataViewEvent& event);
    void fillControls();
    void OnShowHiddenChbClick(wxCommandEvent& event);

    void ShowCurrencyHistory();
    void OnHistoryAdd(wxCommandEvent& event);
    void OnHistoryDelete(wxCommandEvent& WXUNUSED(event));
    void OnHistoryUpdate(wxCommandEvent& WXUNUSED(event));
    void OnHistoryDeleteUnused(wxCommandEvent& WXUNUSED(event));
    void OnHistorySelected(wxListEvent& event);
    void OnHistoryDeselected(wxListEvent& WXUNUSED(event));

    void OnOnlineUpdateCurRate(wxCommandEvent& event);
    bool OnlineUpdateCurRate(int curr_id = -1, bool hide = true);
    void OnItemRightClick(wxDataViewEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    bool SetBaseCurrency(int& baseCurrencyID);

    wxDataViewListCtrl* currencyListBox_;
    std::map<int, wxString> ColName_;
    bool bEnableSelect_;
    wxButton* itemButtonEdit_;
    wxButton* itemButtonDelete_;
    wxBitmapButton* buttonDownloadHistory_;
    wxBitmapButton* buttonDelUnusedHistory_;
    wxCheckBox* cbShowAll_;
    wxListCtrl* valueListBox_;
    wxDatePickerCtrl* valueDatePicker_;
    mmTextCtrl* valueTextBox_;
    wxStaticBox* historyStaticBox_;
    wxButton* historyButtonAdd_;
    wxButton* historyButtonDelete_;

    int m_currency_id;
    bool m_static_dialog;

    std::vector<CurrencyHistoryRate> _BceCurrencyHistoryRatesList;
    bool ConvertHistoryRates(const std::vector<CurrencyHistoryRate>& Bce, std::vector<CurrencyHistoryRate>& ConvertedRate, const wxString& BaseCurrencySymbol);
    bool GetOnlineHistory(std::map<wxDateTime, double> &historical_rates, const wxString &symbol, wxString &msg);
};


