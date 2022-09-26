/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2011, 2012 Stefano Giorgio
Copyright (C) 2013, 2014 Nikolay Akimov
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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

#ifndef MM_EX_CHECKINGPANEL_H_
#define MM_EX_CHECKINGPANEL_H_
//----------------------------------------------------------------------------
#include "mmpanelbase.h"
#include "constants.h"
#include "reports/mmDateRange.h"
#include "model/Model_Checking.h"
#include "model/Model_Account.h"
#include <map>
//----------------------------------------------------------------------------
class mmCheckingPanel;
class mmFilterTransactionsDialog;
class mmGUIFrame;
class TransactionListCtrl;
//----------------------------------------------------------------------------

class mmCheckingPanel : public mmPanelBase
{
public:

    enum EIcons //m_imageList
    {
        ICON_UNRECONCILED,
        ICON_RECONCILED,
        ICON_VOID,
        ICON_FOLLOWUP,
        ICON_DUPLICATE,
        ICON_DESC,
        ICON_ASC,
    };

    mmCheckingPanel(wxWindow* parent
        , mmGUIFrame* frame
        , int accountID
        , int id = wxID_ANY
    );

    ~mmCheckingPanel();

    // Display the split categories for the selected transaction.
    void DisplaySplitCategories(int transID);
    // Refresh account screen with new details
    void DisplayAccountDetails(int accountID = -1);

    void SetSelectedTransaction(int transID);

    void RefreshList();

    wxString BuildPage() const;


private:
    enum
    {
        ID_TRX_FILTER = wxID_HIGHEST + 50,
    };

    enum menu
    {
        MENU_VIEW_ALLTRANSACTIONS = 0,
        MENU_VIEW_TODAY,
        MENU_VIEW_CURRENTMONTH,
        MENU_VIEW_LAST30,
        MENU_VIEW_LAST90,
        MENU_VIEW_LASTMONTH,
        MENU_VIEW_LAST3MONTHS,
        MENU_VIEW_LAST12MONTHS,
        MENU_VIEW_CURRENTYEAR,
        MENU_VIEW_CURRENTFINANCIALYEAR,
        MENU_VIEW_LASTYEAR,
        MENU_VIEW_LASTFINANCIALYEAR,
        MENU_VIEW_STATEMENTDATE,
        MENU_VIEW_FILTER_DIALOG,
    };
private:
   static wxArrayString menu_labels()
    {
        wxArrayString items;
        items.Add(VIEW_TRANS_ALL_STR); //0
        items.Add(VIEW_TRANS_TODAY_STR);
        items.Add(VIEW_TRANS_CURRENT_MONTH_STR);
        items.Add(VIEW_TRANS_LAST_30_DAYS_STR);
        items.Add(VIEW_TRANS_LAST_90_DAYS_STR);
        items.Add(VIEW_TRANS_LAST_MONTH_STR);  //5
        items.Add(VIEW_TRANS_LAST_3MONTHS_STR);
        items.Add(VIEW_TRANS_LAST_12MONTHS_STR);
        items.Add(VIEW_TRANS_CURRENT_YEAR_STR);
        items.Add(VIEW_TRANS_CRRNT_FIN_YEAR_STR);
        items.Add(VIEW_TRANS_LAST_YEAR_STR); //10
        items.Add(VIEW_TRANS_LAST_FIN_YEAR_STR);
        items.Add(VIEW_TRANS_SINCE_STATEMENT_STR);
        items.Add(VIEW_TRANS_FILTER_DIALOG_STR);
        return items;
    } 

    wxDECLARE_EVENT_TABLE();
    friend class TransactionListCtrl; // needs access to m_core, initdb_, ...

    wxButton* m_bitmapTransFilter;
    wxButton* m_btnNew;
    wxButton* m_btnEdit;
    wxButton* m_btnDuplicate;
    wxButton* m_btnDelete;
    wxButton* m_btnAttachment;
    wxStaticText* m_header_text;
    wxStaticText* m_header_sortOrder;
    wxStaticText* m_header_balance;
    wxStaticText* m_info_panel;
    wxStaticText* m_info_panel_mini;

    wxSharedPtr<mmFilterTransactionsDialog> m_trans_filter_dlg;

private:
    int m_currentView;
    int m_AccountID;
    bool isAllAccounts_; // TRUE = All accounts are displayed
    wxString m_sortSaveTitle;   // Used for saving sort settings
    bool m_transFilterActive;
    wxString m_begin_date;
    wxString m_end_date;
    double m_filteredBalance;
    double m_account_balance;
    double m_reconciled_balance;

    TransactionListCtrl* m_listCtrlAccount;
    Model_Account::Data* m_account;
    Model_Currency::Data* m_currency;   // the account currency if single account otherwise the base currency
    wxScopedPtr<wxImageList> m_imageList;

    void initViewTransactionsHeader();
    void initFilterSettings();
    void setAccountSummary();
    void sortTable();
    void filterTable();
    void CreateControls();

    bool Create(
        wxWindow* parent,
        wxWindowID winid = mmID_CHECKING,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "mmCheckingPanel" 
    );

private:

    void OnNewTransaction(wxCommandEvent& event);
    void OnDeleteTransaction(wxCommandEvent& event);
    void OnEditTransaction(wxCommandEvent& event);
    void OnDuplicateTransaction(wxCommandEvent& event);
    void OnMoveTransaction(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);
    void OnMouseLeftDown(wxCommandEvent& event);
    void OnButtonRightDown(wxMouseEvent& event);
    void OnViewPopupSelected(wxCommandEvent& event);
    void OnSearchTxtEntered(wxCommandEvent& event);

    /* updates the checking panel data */
    void showTips();
    void updateExtraTransactionData(bool single, bool foreign);
    void enableTransactionButtons(bool editDelete, bool duplicate, bool attach);
    wxString GetPanelTitle(const Model_Account::Data& account) const;
    static void mmPlayTransactionSound();
    mmGUIFrame* m_frame;
};
//----------------------------------------------------------------------------
#endif // MM_EX_CHECKINGPANEL_H_
//----------------------------------------------------------------------------

