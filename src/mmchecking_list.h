/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2011, 2012 Stefano Giorgio
Copyright (C) 2013, 2014, 2020, 2021 Nikolay Akimov
Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)

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

#ifndef MM_EX_CHECKING_LIST_H_
#define MM_EX_CHECKING_LIST_H_

#include "mmpanelbase.h"
#include "mmcheckingpanel.h"

class mmCheckingPanel;

class TransactionListCtrl : public mmListCtrl
{
public:

    TransactionListCtrl(mmCheckingPanel* cp
        , wxWindow* parent
        , const wxWindowID id = wxID_ANY);

    ~TransactionListCtrl();

    void createColumns(mmListCtrl &lst);
    Model_Checking::Full_Data_Set m_trans;
    void markSelectedTransaction();
    void DeleteTransactionsByStatus(const wxString& status);
    void DeleteViewedTransactions();
public:
    enum EColumn
    {
        COL_IMGSTATUS = 0,
        COL_ID,
        COL_DATE,
        COL_NUMBER,
        COL_ACCOUNT,
        COL_PAYEE_STR,
        COL_STATUS,
        COL_CATEGORY,
        COL_WITHDRAWAL,
        COL_DEPOSIT,
        COL_BALANCE,
        COL_CREDIT,
        COL_NOTES,
        COL_UDFC01,
        COL_UDFC02,
        COL_UDFC03,
        COL_UDFC04,
        COL_UDFC05,
        COL_MAX, // number of columns
        COL_DEF_SORT = COL_DATE, // don't omit any columns before this
        COL_DEF_SORT2 = COL_ID 
    };
    EColumn toEColumn(long col);
public:
    EColumn g_sortcol; // index of primary column to sort by
    EColumn prev_g_sortcol; // index of secondary column to sort by
    bool g_asc; // asc\desc sorting for primary sort column
    bool prev_g_asc; // asc\desc sorting for secondary sort column

    bool getSortOrder() const;
    EColumn getSortColumn() const { return m_sortCol; }

    void setSortOrder(bool asc) { m_asc = asc; }
    void setSortColumn(EColumn col) { m_sortCol = col; }
    void setVisibleItemIndex(long v);

    void setColumnImage(EColumn col, int image);
public:
    void OnNewTransaction(wxCommandEvent& event);
    void OnNewTransferTransaction(wxCommandEvent& event);
    void OnDeleteTransaction(wxCommandEvent& event);
    void OnDeleteViewedTransaction(wxCommandEvent& event);
    void OnEditTransaction(wxCommandEvent& event);
    void OnDuplicateTransaction(wxCommandEvent& event);
    void OnSetUserColour(wxCommandEvent& event);
    void OnMoveTransaction(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);
    void OnViewOtherAccount(wxCommandEvent& event);
    // Displays the split categories for the selected transaction
    void OnViewSplitTransaction(wxCommandEvent& event);
    void OnOrganizeAttachments(wxCommandEvent& event);
    void OnCreateReoccurance(wxCommandEvent& event);
    void refreshVisualList(bool filter = true);
    void sortTable();
public:
    std::vector<int> getSelectedForCopy() const;

    std::vector<int> getSelectedId() const;
    void setSelectedID(int v);
    void doSearchText(const wxString& value);
    /* Getter for Virtual List Control */
    const wxString getItem(long item, long column, bool realenum = false) const;

protected:
    /* Sort Columns */
    virtual void OnColClick(wxListEvent& event);

private:
    void markItem(long selectedItem);

    std::vector<int> m_selectedForCopy; // the copied transactions (held for pasting)
    std::vector<int> m_pasted_id;       // the last pasted transactions
    std::vector<int> m_selected_id;     // the selected transactions
    enum
    {
        MENU_TREEPOPUP_MARKRECONCILED = wxID_HIGHEST + 150,
        MENU_TREEPOPUP_MARKUNRECONCILED,
        MENU_TREEPOPUP_MARKVOID,
        MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP,
        MENU_TREEPOPUP_MARKDUPLICATE,
        MENU_TREEPOPUP_MARKDELETE,

        MENU_TREEPOPUP_VIEW_OTHER_ACCOUNT,
        MENU_TREEPOPUP_VIEW_SPLIT_CATEGORIES,
        MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS,
        MENU_TREEPOPUP_CREATE_REOCCURANCE,
        MENU_SUBMENU_MARK_ALL,

        MENU_VIEW_,
        MENU_VIEW_DELETE_TRANS,
        MENU_VIEW_DELETE_FLAGGED,

        MENU_ON_SELECT_ALL,
        MENU_ON_COPY_TRANSACTION,
        MENU_ON_PASTE_TRANSACTION,
        MENU_ON_NEW_TRANSACTION,
        MENU_ON_DUPLICATE_TRANSACTION,

        MENU_ON_SET_UDC0, //Default color
        MENU_ON_SET_UDC1, //User defined color 1
        MENU_ON_SET_UDC2, //User defined color 2
        MENU_ON_SET_UDC3, //User defined color 3
        MENU_ON_SET_UDC4, //User defined color 4
        MENU_ON_SET_UDC5, //User defined color 5
        MENU_ON_SET_UDC6, //User defined color 6
        MENU_ON_SET_UDC7, //User defined color 7

        MENU_TREEPOPUP_NEW_WITHDRAWAL,
        MENU_TREEPOPUP_NEW_DEPOSIT,
        MENU_TREEPOPUP_NEW_TRANSFER,
        MENU_TREEPOPUP_EDIT2,
        MENU_TREEPOPUP_MOVE2,
        MENU_TREEPOPUP_DELETE2,
        MENU_TREEPOPUP_DELETE_VIEWED,
        MENU_TREEPOPUP_DELETE_FLAGGED,
        MENU_TREEPOPUP_DELETE_UNRECONCILED,
        ID_PANEL_CHECKING_STATIC_BITMAP_VIEW,
    };
private:
    DECLARE_NO_COPY_CLASS(TransactionListCtrl)
    wxDECLARE_EVENT_TABLE();

    mmCheckingPanel* m_cp;

    wxSharedPtr<wxListItemAttr> m_attr1;  // style1
    wxSharedPtr<wxListItemAttr> m_attr2;  // style2
    wxSharedPtr<wxListItemAttr> m_attr3;  // style, for future dates
    wxSharedPtr<wxListItemAttr> m_attr4;  // style, for future dates
    wxSharedPtr<wxListItemAttr> m_attr11; // user defined style 1
    wxSharedPtr<wxListItemAttr> m_attr12; // user defined style 2
    wxSharedPtr<wxListItemAttr> m_attr13; // user defined style 3
    wxSharedPtr<wxListItemAttr> m_attr14; // user defined style 4
    wxSharedPtr<wxListItemAttr> m_attr15; // user defined style 5
    wxSharedPtr<wxListItemAttr> m_attr16; // user defined style 6
    wxSharedPtr<wxListItemAttr> m_attr17; // user defined style 7

    /* required overrides for virtual style list control */
    virtual wxString OnGetItemText(long item, long column) const;
    virtual int OnGetItemColumnImage(long item, long column) const;
    virtual wxListItemAttr* OnGetItemAttr(long item) const;

    void OnMouseRightClick(wxMouseEvent& event);
    void OnListLeftClick(wxMouseEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnListItemDeSelected(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnMarkTransaction(wxCommandEvent& event);
    void OnListKeyDown(wxListEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnSelectAll(wxCommandEvent& WXUNUSED(event));
    void OnCopy(wxCommandEvent& WXUNUSED(event));
    void OnPaste(wxCommandEvent& WXUNUSED(event));
    void OnListItemFocused(wxListEvent& WXUNUSED(event));
    int OnPaste(Model_Checking::Data* tran);

    bool TransactionLocked(int AccountID, const wxString& transdate);
    void FindSelectedTransactions();
    bool CheckForClosedAccounts();
    void setExtraTransactionData(bool single);
    void SortTransactions(int sortcol, bool ascend);
private:
    /* The topmost visible item - this will be used to set
    where to display the list again after refresh */
    long m_topItemIndex;
    EColumn m_sortCol;
    wxString m_today;
    bool m_firstSort;
};

//----------------------------------------------------------------------------

inline bool TransactionListCtrl::getSortOrder() const { return m_asc; }
inline std::vector<int> TransactionListCtrl::getSelectedForCopy() const { return m_selectedForCopy; }

inline std::vector<int> TransactionListCtrl::getSelectedId() const { return m_selected_id; }

inline void TransactionListCtrl::setVisibleItemIndex(long v) { m_topItemIndex = v; }

#endif // MM_EX_CHECKING_LIST_H_

inline static bool SorterByUDFC01(const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j) { return (i.UDFC01 < j.UDFC01); }
inline static bool SorterByUDFC02(const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j) { return (i.UDFC02 < j.UDFC02); }
inline static bool SorterByUDFC03(const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j) { return (i.UDFC03 < j.UDFC03); }
inline static bool SorterByUDFC04(const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j) { return (i.UDFC04 < j.UDFC04); }
inline static bool SorterByUDFC05(const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j) { return (i.UDFC05 < j.UDFC05); }

inline static bool SorterByUDFC01_val(const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j) { return (i.UDFC01_val < j.UDFC01_val); }
inline static bool SorterByUDFC02_val(const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j) { return (i.UDFC02_val < j.UDFC02_val); }
inline static bool SorterByUDFC03_val(const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j) { return (i.UDFC03_val < j.UDFC03_val); }
inline static bool SorterByUDFC04_val(const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j) { return (i.UDFC04_val < j.UDFC04_val); }
inline static bool SorterByUDFC05_val(const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j) { return (i.UDFC05_val < j.UDFC05_val); }
