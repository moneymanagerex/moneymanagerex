/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2011, 2012 Stefano Giorgio
Copyright (C) 2013, 2014, 2020, 2021 Nikolay Akimov
Copyright (C) 2021-2024 Mark Whalley (mark@ipx.co.uk)

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
#include "fusedtransaction.h"

class mmCheckingPanel;

class TransactionListCtrl : public mmListCtrl
{
public:
    TransactionListCtrl(
        mmCheckingPanel* cp,
        wxWindow* parent,
        const wxWindowID id = wxID_ANY
    );
    ~TransactionListCtrl();

private:
    friend class mmCheckingPanel;

    enum LIST_COL
    {
        LIST_COL_IMGSTATUS = 0,
        LIST_COL_ID,
        LIST_COL_DATE,
        LIST_COL_TIME,
        LIST_COL_NUMBER,
        LIST_COL_ACCOUNT,
        LIST_COL_PAYEE_STR,
        LIST_COL_STATUS,
        LIST_COL_CATEGORY,
        LIST_COL_TAGS,
        LIST_COL_WITHDRAWAL,
        LIST_COL_DEPOSIT,
        LIST_COL_BALANCE,
        LIST_COL_CREDIT,
        LIST_COL_NOTES,
        LIST_COL_DELETEDTIME,
        LIST_COL_UDFC01,
        LIST_COL_UDFC02,
        LIST_COL_UDFC03,
        LIST_COL_UDFC04,
        LIST_COL_UDFC05,
        LIST_COL_UPDATEDTIME,
        LIST_COL_SN,
        LIST_COL_size, // number of columns
        LIST_COL_def_sort1 = LIST_COL_DATE,
        LIST_COL_def_sort2 = LIST_COL_ID 
    };
    static const std::vector<ListColumnInfo> col_info_all();

    enum
    {
        MENU_TREEPOPUP_MARKRECONCILED = wxID_HIGHEST + 200,
        MENU_TREEPOPUP_MARKUNRECONCILED,
        MENU_TREEPOPUP_MARKVOID,
        MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP,
        MENU_TREEPOPUP_MARKDUPLICATE,
        MENU_TREEPOPUP_MARKDELETE,

        MENU_TREEPOPUP_VIEW_OTHER_ACCOUNT,
        MENU_TREEPOPUP_VIEW_SPLIT_CATEGORIES,
        MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS,
        MENU_TREEPOPUP_CREATE_REOCCURANCE,
        MENU_TREEPOPUP_FIND,
        MENU_TREEPOPUP_COPYTEXT,
        MENU_SUBMENU_MARK_ALL,

        MENU_VIEW_,
        MENU_VIEW_DELETE_TRANS,
        MENU_VIEW_DELETE_FLAGGED,

        MENU_ON_SELECT_ALL,
        MENU_ON_COPY_TRANSACTION,
        MENU_ON_PASTE_TRANSACTION,
        MENU_ON_NEW_TRANSACTION,
        MENU_ON_DUPLICATE_TRANSACTION,
        MENU_ON_ENTER_SCHEDULED,
        MENU_ON_SKIP_SCHEDULED,

        MENU_ON_SET_UDC0, // default color
        MENU_ON_SET_UDC1, // user-defined color 1
        MENU_ON_SET_UDC2, // user-defined color 2
        MENU_ON_SET_UDC3, // user-defined color 3
        MENU_ON_SET_UDC4, // user-defined color 4
        MENU_ON_SET_UDC5, // user-defined color 5
        MENU_ON_SET_UDC6, // user-defined color 6
        MENU_ON_SET_UDC7, // user-defined color 7

        MENU_TREEPOPUP_WITHDRAWAL,
        MENU_TREEPOPUP_DEPOSIT,
        MENU_TREEPOPUP_TRANSFER,
        MENU_TREEPOPUP_EDIT2,
        MENU_TREEPOPUP_MOVE2,
        MENU_TREEPOPUP_DELETE2,
        MENU_TREEPOPUP_DELETE_VIEWED,
        MENU_TREEPOPUP_DELETE_FLAGGED,
        MENU_TREEPOPUP_DELETE_UNRECONCILED,
        MENU_TREEPOPUP_RESTORE,
        MENU_TREEPOPUP_RESTORE_VIEWED,
        ID_PANEL_CHECKING_STATIC_BITMAP_VIEW,
    };

private:
    Fused_Transaction::Full_Data_Set m_trans;
    long m_topItemIndex = -1; // where to display the list again after refresh
    int g_sortCol1 = LIST_COL_def_sort1; // 0-based number of primary sorting column
    int g_sortCol2 = LIST_COL_def_sort2; // 0-based number of secondary sorting column
    bool g_sortAsc1 = true;         // asc/desc order for primary sorting column
    bool g_sortAsc2 = true;         // asc/desc order for secondary sorting column
    wxString m_today;
    bool m_firstSort = true;
    wxString rightClickFilter_;
    wxString copyText_;
    std::vector<Fused_Transaction::IdRepeat> m_selectedForCopy; // copied transactions
    std::vector<Fused_Transaction::IdRepeat> m_pasted_id;       // last pasted transactions
    std::vector<Fused_Transaction::IdRepeat> m_selected_id;     // selected transactions

    DECLARE_NO_COPY_CLASS(TransactionListCtrl)
    wxDECLARE_EVENT_TABLE();
    mmCheckingPanel* m_cp = nullptr;
    wxSharedPtr<wxListItemAttr> m_attr1;  // style 1
    wxSharedPtr<wxListItemAttr> m_attr2;  // style 2
    wxSharedPtr<wxListItemAttr> m_attr3;  // style 3 (reserved)
    wxSharedPtr<wxListItemAttr> m_attr4;  // style 4 (reserved)
    wxSharedPtr<wxListItemAttr> m_attr11; // user-defined style 1
    wxSharedPtr<wxListItemAttr> m_attr12; // user-defined style 2
    wxSharedPtr<wxListItemAttr> m_attr13; // user-defined style 3
    wxSharedPtr<wxListItemAttr> m_attr14; // user-defined style 4
    wxSharedPtr<wxListItemAttr> m_attr15; // user-defined style 5
    wxSharedPtr<wxListItemAttr> m_attr16; // user-defined style 6
    wxSharedPtr<wxListItemAttr> m_attr17; // user-defined style 7

private:
    void resetColumns();
    void refreshVisualList(bool filter = true);
    void sortTable();
    template<class Compare>
    void sortBy(Compare comp, bool ascend);
    void sortTransactions(int sortcol, bool ascend);

private:
    // required overrides for virtual style list control
    virtual wxString OnGetItemText(long item, long column) const;
    virtual int OnGetItemColumnImage(long item, long column) const;
    virtual wxListItemAttr* OnGetItemAttr(long item) const;

protected:
    virtual void OnColClick(wxListEvent& event);

private:
    void onChar(wxKeyEvent& event);
    void onListLeftClick(wxMouseEvent& event);
    void onMouseRightClick(wxMouseEvent& event);

    void onListItemActivated(wxListEvent& event);
    void onListItemSelected(wxListEvent&);
    void onListItemDeSelected(wxListEvent&);
    void onListItemFocused(wxListEvent& WXUNUSED(event));
    void onListKeyDown(wxListEvent& event);

    void onNewTransaction(wxCommandEvent& event);
    void onDeleteTransaction(wxCommandEvent& event);
    void onRestoreTransaction(wxCommandEvent& event);
    void onRestoreViewedTransaction(wxCommandEvent&);
    void onEditTransaction(wxCommandEvent& event);
    void onMoveTransaction(wxCommandEvent& event);
    void onViewOtherAccount(wxCommandEvent& event);
    void onViewSplitTransaction(wxCommandEvent& event);
    void onOrganizeAttachments(wxCommandEvent& event);
    void onCreateReoccurance(wxCommandEvent& event);
    void onFind(wxCommandEvent&);
    void onCopyText(wxCommandEvent&);
    void onMarkTransaction(wxCommandEvent& event);
    void onDeleteViewedTransaction(wxCommandEvent& event);

    void onSelectAll(wxCommandEvent& WXUNUSED(event));
    void onCopy(wxCommandEvent& WXUNUSED(event));
    void onPaste(wxCommandEvent& WXUNUSED(event));
    int64 onPaste(Model_Checking::Data* tran);
    void onDuplicateTransaction(wxCommandEvent& event);
    void onEnterScheduled(wxCommandEvent& event);
    void onSkipScheduled(wxCommandEvent& event);
    void onSetUserColour(wxCommandEvent& event);
    void onOpenAttachment(wxCommandEvent& event);

private:
    const wxString getItem(long item, long column, bool realenum = false) const;
    void setColumnImage(int col, int image);
    void setExtraTransactionData(const bool single);
    void markItem(long selectedItem);
    void setSelectedId(Fused_Transaction::IdRepeat sel_id);
    std::vector<Fused_Transaction::IdRepeat> getSelectedId() const;
    std::vector<Fused_Transaction::IdRepeat> getSelectedForCopy() const;
    void findSelectedTransactions();
    void setSortOrder(bool asc);
    bool getSortOrder() const;
    int getColumnFromPosition(int xPos);
    void doSearchText(const wxString& value);
    void setVisibleItemIndex(long v);
    void markSelectedTransaction();
    void deleteTransactionsByStatus(const wxString& status);
    bool checkForClosedAccounts();
    bool checkTransactionLocked(int64 AccountID, const wxString& transdate);
};

//----------------------------------------------------------------------------

inline void TransactionListCtrl::setSortOrder(bool asc)
{
    m_asc = asc;
}
inline bool TransactionListCtrl::getSortOrder() const {
    return m_asc;
}

inline std::vector<Fused_Transaction::IdRepeat> TransactionListCtrl::getSelectedId() const
{
    return m_selected_id;
}
inline std::vector<Fused_Transaction::IdRepeat> TransactionListCtrl::getSelectedForCopy() const
{
    return m_selectedForCopy;
}

inline void TransactionListCtrl::setVisibleItemIndex(long v)
{
    m_topItemIndex = v;
}

#endif // MM_EX_CHECKING_LIST_H_

inline static bool SorterByUDFC01(
    const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j
) {
    return (i.UDFC_content[0] < j.UDFC_content[0]);
}

inline static bool SorterByUDFC02(
    const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j
) {
    return (i.UDFC_content[1] < j.UDFC_content[1]);
}

inline static bool SorterByUDFC03(
    const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j
) {
    return (i.UDFC_content[2] < j.UDFC_content[2]);
}

inline static bool SorterByUDFC04(
    const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j
) {
    return (i.UDFC_content[3] < j.UDFC_content[3]);
}

inline static bool SorterByUDFC05(
    const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j
) {
    return (i.UDFC_content[4] < j.UDFC_content[4]);
}

inline static bool SorterByUDFC01_val(
    const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j
) {
    return (i.UDFC_value[0] < j.UDFC_value[0]);
}

inline static bool SorterByUDFC02_val(
    const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j
) {
    return (i.UDFC_value[1] < j.UDFC_value[1]);
}

inline static bool SorterByUDFC03_val(
    const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j
) {
    return (i.UDFC_value[2] < j.UDFC_value[2]);
}

inline static bool SorterByUDFC04_val(
    const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j
) {
    return (i.UDFC_value[3] < j.UDFC_value[3]);
}

inline static bool SorterByUDFC05_val(
    const Model_Checking::Full_Data& i, const Model_Checking::Full_Data& j
) {
    return (i.UDFC_value[4] < j.UDFC_value[4]);
}

