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
#include "model/Model_Checking.h"
#include "model/Model_Account.h"
#include <map>
//----------------------------------------------------------------------------
class mmCheckingPanel;
class mmFilterTransactionsDialog;
//----------------------------------------------------------------------------

class TransactionListCtrl : public wxListCtrl
{
public:

    TransactionListCtrl(mmCheckingPanel *cp, wxWindow *parent
        , const wxWindowID id, const wxPoint& pos,const wxSize& size, long style);

    void createColumns(wxListCtrl &lst);
    enum EIcons //m_imageList
    {
        ICON_RECONCILED,
        ICON_VOID,
        ICON_FOLLOWUP,
        ICON_NONE,
        ICON_DESC,
        ICON_ASC,
        ICON_DUPLICATE,
        ICON_TRASH
    };

    enum EColumn
    {
        COL_DATE_OR_TRANSACTION_ID = 0,
        COL_TRANSACTION_NUMBER,
        COL_PAYEE_STR,
        COL_STATUS,
        COL_CATEGORY,
        COL_WITHDRAWAL,
        COL_DEPOSIT,
        COL_BALANCE,
        COL_NOTES,
        COL_MAX, // number of columns
        COL_DEF_SORT = COL_DATE_OR_TRANSACTION_ID
    };
    EColumn toEColumn(long col)
    {
        EColumn res = COL_DEF_SORT;
        if (col >= 0 && col < COL_MAX) res = static_cast<EColumn>(col);

        return res;
    }

    EColumn g_sortcol; // index of column to sort
    bool g_asc; // asc\desc sorting

    bool getSortOrder() const { return m_asc; }
    EColumn getSortColumn() const { return m_sortCol; }

    void setSortOrder(bool asc) { m_asc = asc; }
    void setSortColumn(EColumn col) { m_sortCol = col; }

    void setColumnImage(EColumn col, int image);

    void OnNewTransaction(wxCommandEvent& event);
    void OnDeleteTransaction(wxCommandEvent& event);
    void OnEditTransaction(wxCommandEvent& event);
    void OnDuplicateTransaction(wxCommandEvent& event);
    void OnSetUserColour(wxCommandEvent& event);
    void OnMoveTransaction(wxCommandEvent& event);
    /// Displays the split categories for the selected transaction
    void OnViewSplitTransaction(wxCommandEvent& event);
    long m_selectedIndex;
    long m_selectedForCopy;
    long m_selectedID;
    void refreshVisualList(int trans_id = -1);
    bool showDeletedTransactions_;

private:

    DECLARE_NO_COPY_CLASS(TransactionListCtrl)
    DECLARE_EVENT_TABLE()

    mmCheckingPanel *m_cp;

    wxListItemAttr m_attr1;  // style1
    wxListItemAttr m_attr2;  // style2
    wxListItemAttr m_attr3;  // style, for future dates
    wxListItemAttr m_attr4;  // style, for future dates
    wxListItemAttr m_attr11; // user defined style 1
    wxListItemAttr m_attr12; // user defined style 2
    wxListItemAttr m_attr13; // user defined style 3
    wxListItemAttr m_attr14; // user defined style 4
    wxListItemAttr m_attr15; // user defined style 5
    wxListItemAttr m_attr16; // user defined style 6
    wxListItemAttr m_attr17; // user defined style 7

    EColumn m_sortCol;
    bool m_asc;

    /* required overrides for virtual style list control */
    virtual wxString OnGetItemText(long item, long column) const;
    virtual int OnGetItemColumnImage(long item, long column) const;
    virtual wxListItemAttr *OnGetItemAttr(long item) const;

    void OnListRightClick(wxMouseEvent& event);
    void OnListLeftClick(wxMouseEvent& event);
    void OnItemResize(wxListEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnListItemDeselected(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnMarkTransaction(wxCommandEvent& event);
    void OnShowChbClick(wxCommandEvent& /*event*/);
    void OnMarkAllTransactions(wxCommandEvent& event);
    void OnListKeyDown(wxListEvent& event);
    void OnChar(wxKeyEvent& event);
    int OnMarkTransactionDB(const wxString& status);
    void OnCopy(wxCommandEvent& WXUNUSED(event));
    void OnPaste(wxCommandEvent& WXUNUSED(event));

    /* Sort Columns */
    void OnColClick(wxListEvent& event);

    //The topmost visible item
    long topItemIndex_;
};

//----------------------------------------------------------------------------

class mmCheckingPanel : public mmPanelBase
{
public:

    mmCheckingPanel(
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
    enum menu
    {
        MENU_VIEW_ALLTRANSACTIONS,
        MENU_VIEW_TODAY,
        MENU_VIEW_CURRENTMONTH,
        MENU_VIEW_LAST30,
        MENU_VIEW_LAST90,
        MENU_VIEW_LASTMONTH,
        MENU_VIEW_LAST3MONTHS,
        MENU_VIEW_LAST12MONTHS,
        MENU_VIEW_CURRENTYEAR,
    };
    static wxArrayString menu_labels()
    {
        wxArrayString items;
        items.Add(wxTRANSLATE("View All Transactions"));
        items.Add(wxTRANSLATE("View Today"));
        items.Add(wxTRANSLATE("View Current Month"));
        items.Add(wxTRANSLATE("View Last 30 days"));
        items.Add(wxTRANSLATE("View Last 90 days"));
        items.Add(wxTRANSLATE("View Last Month"));
        items.Add(wxTRANSLATE("View Last 3 Months"));
        items.Add(wxTRANSLATE("View Last 12 Months"));
        items.Add(wxTRANSLATE("View Current Year"));
        return items;
    }
    DECLARE_EVENT_TABLE()
    friend class TransactionListCtrl; // needs access to m_core, initdb_, ...

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

    int currentView_;
    int m_AccountID;
    int m_basecurrecyID;
    wxDateTime quickFilterBeginDate_;
    wxDateTime quickFilterEndDate_;
    bool transFilterActive_;
    double filteredBalance_;
    double account_balance_;
    double reconciled_balance_;

    TransactionListCtrl *m_listCtrlAccount;
    Model_Account::Data* m_account;
    Model_Currency::Data* m_currency;
    wxScopedPtr<wxImageList> m_imageList;
    Model_Checking::Full_Data_Set m_trans;

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
    const wxString getItem(long item, long column);

    wxString getMiniInfoStr(int selIndex) const;
};
//----------------------------------------------------------------------------
#endif // _MM_EX_CHECKINGPANEL_H_
//----------------------------------------------------------------------------

