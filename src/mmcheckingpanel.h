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

#ifndef _MM_EX_CHECKINGPANEL_H_
#define _MM_EX_CHECKINGPANEL_H_
//----------------------------------------------------------------------------
#include "filtertransdialog.h"
#include "mmpanelbase.h"
#include "reports/mmDateRange.h"
//----------------------------------------------------------------------------
class mmBankTransaction;
class TransactionListCtrl;
class mmFilterTransactionsDialog;
class mmDateRange;
//----------------------------------------------------------------------------

class mmCheckingPanel : public mmPanelBase
{
public:

    mmCheckingPanel(
        mmCoreDB* core,
        int accountID,
        wxWindow *parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = wxPanelNameStr
    );

    ~mmCheckingPanel();

    /// Display the split categories for the selected transaction.
    void DisplaySplitCategories(int transID);
    /// Refresh account screen with new details
    void DisplayAccountDetails(int accountID);

    void SetSelectedTransaction(int transID);

    void RefreshList();

private:
    DECLARE_EVENT_TABLE()
    friend class TransactionListCtrl; // needs access to m_core, initdb_, ...

    mmDateRange* date_range_;
    wxButton* btnNew_;
    wxButton* btnEdit_;
    wxButton* btnDuplicate_;
    wxButton* btnDelete_;
    wxStaticText* header_text_;
    wxStaticText* info_panel_;
    wxStaticText* info_panel_mini_;
    wxStaticText* stxtMainFilter_;
    wxStaticText* statTextTransFilter_;
    wxStaticBitmap* bitmapTransFilter_;
    wxStaticBitmap* bitmapMainFilter_;
    mmFilterTransactionsDialog* transFilterDlg_;

    wxString currentView_;
    wxDateTime quickFilterBeginDate_;
    wxDateTime quickFilterEndDate_;
    bool transFilterActive_;
    double filteredBalance_;
    double account_balance_;
    double reconciled_balance_;

    TransactionListCtrl *m_listCtrlAccount;
    int m_AccountID;
    wxScopedPtr<wxImageList> m_imageList;
    std::vector<mmBankTransaction*> m_trans;

    std::shared_ptr<wxSQLite3Database> getDb() const;
    void initViewTransactionsHeader();
    void initFilterSettings();
    void setAccountSummary();
    void sortTable();
    void filterTable();
    void markSelectedTransaction(int trans_id);
    void CreateControls();

    bool Create(
        wxWindow *parent,
        wxWindowID winid,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = wxPanelNameStr
    );
    void enableEditDeleteButtons(bool en);

    void OnNewTransaction(wxCommandEvent& event);
    void OnDeleteTransaction(wxCommandEvent& event);
    void OnEditTransaction(wxCommandEvent& event);
    void OnDuplicateTransaction(wxCommandEvent& event);
    void OnMoveTransaction(wxCommandEvent& event);
    void OnMouseLeftDown( wxMouseEvent& event );
    void OnViewPopupSelected(wxCommandEvent& event);
    void OnFilterTransactions(wxMouseEvent& event);
    void OnSearchTxtEntered(wxCommandEvent& event);
    void OnFilterResetToViewAll(wxMouseEvent& event);

    void DeleteViewedTransactions();
    void DeleteFlaggedTransactions(const wxString& status);
    void SetTransactionFilterState(bool active);

    /* updates the checking panel data */
    void initVirtualListControl(int trans_id = -1);
    void showTips();
    void updateExtraTransactionData(int selIndex);

    /* Getter for Virtual List Control */
    wxString getItem(long item, long column) const;

    wxString getMiniInfoStr(int selIndex) const;
};
//----------------------------------------------------------------------------
#endif // _MM_EX_CHECKINGPANEL_H_
//----------------------------------------------------------------------------

