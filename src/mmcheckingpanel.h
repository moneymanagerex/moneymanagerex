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
#include <wx/tglbtn.h>
#include "mmpanelbase.h"
#include "constants.h"
#include "fusedtransaction.h"
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
    enum EIcons
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
        , int64 accountID
        , int id = wxID_ANY
    );

    ~mmCheckingPanel();

    // Display the split categories for the selected transaction.
    void DisplaySplitCategories(Fused_Transaction::IdB fused_id);
    // Refresh account screen with new details
    void DisplayAccountDetails(int64 accountID = -1);

    void SetSelectedTransaction(Fused_Transaction::IdRepeat fused_id);

    void RefreshList();
    void ResetColumnView();

    wxString BuildPage() const;


private:
    enum ID_TRX
    {
        ID_TRX_FILTER = wxID_HIGHEST + 50,
        ID_TRX_SCHEDULED
    };
    enum FILTER_ID
    {
        FILTER_ID_ALL = 0,
        FILTER_ID_TODAY,
        FILTER_ID_CURRENTMONTH,
        FILTER_ID_LAST30,
        FILTER_ID_LAST90,
        FILTER_ID_LASTMONTH,
        FILTER_ID_LAST3MONTHS,
        FILTER_ID_LAST12MONTHS,
        FILTER_ID_CURRENTYEAR,
        FILTER_ID_CURRENTFINYEAR,
        FILTER_ID_LASTYEAR,
        FILTER_ID_LASTFINYEAR,
        FILTER_ID_STATEMENTDATE,
        FILTER_ID_DIALOG,
        FILTER_ID_MAX,
    };

public:
    static wxArrayString FILTER_STR;
    static const wxString FILTER_STR_ALL;
    static const wxString FILTER_STR_DIALOG;

private:
    static const std::vector<std::pair<FILTER_ID, wxString> > FILTER_CHOICES;
    static wxArrayString filter_str_all();

    wxDECLARE_EVENT_TABLE();
    friend class TransactionListCtrl; // needs access to m_core, initdb_, ...

    wxButton* m_bitmapTransFilter = nullptr;
    wxButton* m_btnNew = nullptr;
    wxButton* m_btnEdit = nullptr;
    wxButton* m_btnDuplicate = nullptr;
    wxButton* m_btnDelete = nullptr;
    wxButton* m_btnEnter = nullptr;
    wxButton* m_btnSkip = nullptr;
    wxButton* m_btnRestore = nullptr;
    wxButton* m_btnAttachment = nullptr;
    wxStaticText* m_header_text = nullptr;
    wxBitmapToggleButton* m_header_scheduled = nullptr;
    wxStaticText* m_header_sortOrder = nullptr;
    wxGauge* m_header_credit = nullptr;
    wxStaticText* m_header_balance = nullptr;
    wxStaticText* m_info_panel = nullptr;
    wxStaticText* m_info_panel_mini = nullptr;

    wxSharedPtr<mmFilterTransactionsDialog> m_trans_filter_dlg;

private:
    int64 m_AccountID = -1;
    bool isAllAccounts_ = false; // TRUE = All accounts are displayed
    bool isTrash_ = false; // TRUE = Deleted transactions are displayed
    int m_filter_id;
    bool m_scheduled_selected;
    bool m_scheduled_allowed;
    wxString m_sortSaveTitle;   // Used for saving sort settings
    bool m_transFilterActive = false;
    wxString m_begin_date;
    wxString m_end_date;
    double m_account_flow = 0.0;
    double m_account_balance = 0.0;
    double m_account_recbalance = 0.0;
    bool m_show_reconciled;

    TransactionListCtrl* m_listCtrlAccount = nullptr;
    Model_Account::Data* m_account = nullptr;
    Model_Currency::Data* m_currency = nullptr;   // the account currency if single account otherwise the base currency

    wxVector<wxBitmapBundle> m_images;

    void initFilterChoices();
    void updateFilterState();
    void saveFilterChoices();
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
    void OnEditTransaction(wxCommandEvent& event);
    void OnDeleteTransaction(wxCommandEvent& event);
    void OnDuplicateTransaction(wxCommandEvent& event);
    void OnEnterScheduled(wxCommandEvent& event);
    void OnSkipScheduled(wxCommandEvent& event);
    void OnRestoreTransaction(wxCommandEvent& event);
    void OnMoveTransaction(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);
    void OnMouseLeftDown(wxCommandEvent& event);
    void OnButtonRightDown(wxMouseEvent& event);
    void OnViewPopupSelected(wxCommandEvent& event);
    void OnScheduled(wxCommandEvent& event);
    void OnSearchTxtEntered(wxCommandEvent& event);

    /* updates the checking panel data */
    void showTips();
    void updateScheduledToolTip();
    void updateExtraTransactionData(bool single, int repeat_num, bool foreign);
    void enableButtons(bool edit, bool dup, bool del, bool enter, bool skip, bool attach);
    wxString GetPanelTitle(const Model_Account::Data& account) const;
    static void mmPlayTransactionSound();
    mmGUIFrame* m_frame = nullptr;
};
//----------------------------------------------------------------------------
#endif // MM_EX_CHECKINGPANEL_H_
//----------------------------------------------------------------------------

