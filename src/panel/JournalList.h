/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2011, 2012 Stefano Giorgio
Copyright (C) 2013, 2014, 2020, 2021 Nikolay Akimov
Copyright (C) 2021-2025 Mark Whalley (mark@ipx.co.uk)

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

#include <optional>
#include "model/Journal.h"
#include "_ListBase.h"

class JournalPanel;

class JournalList : public ListBase
{
    friend class JournalPanel;

    DECLARE_NO_COPY_CLASS(JournalList)
    wxDECLARE_EVENT_TABLE();

private:
    enum LIST_ID
    {
        LIST_ID_ICON = 0,
        LIST_ID_ID,
        LIST_ID_DATE,
        LIST_ID_TIME,
        LIST_ID_NUMBER,
        LIST_ID_ACCOUNT,
        LIST_ID_PAYEE_STR,
        LIST_ID_STATUS,
        LIST_ID_CATEGORY,
        LIST_ID_TAGS,
        LIST_ID_WITHDRAWAL,
        LIST_ID_DEPOSIT,
        LIST_ID_BALANCE,
        LIST_ID_CREDIT,
        LIST_ID_NOTES,
        LIST_ID_DELETEDTIME,
        LIST_ID_UDFC01,
        LIST_ID_UDFC02,
        LIST_ID_UDFC03,
        LIST_ID_UDFC04,
        LIST_ID_UDFC05,
        LIST_ID_UPDATEDTIME,
        LIST_ID_SN,
        LIST_ID_size
    };
    static const std::vector<ListColumnInfo> LIST_INFO;

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
    Journal::DataExtA m_journal_xa;
    long m_top_item_n = -1; // where to display the list again after refresh
    bool m_balance_valid = false;
    wxString m_filter;
    wxString m_copy_text;
    std::vector<JournalKey> m_select_key_a; // selected transactions
    std::vector<JournalKey> m_copy_key_a;   // copied transactions
    std::vector<JournalKey> m_paste_key_a;  // last pasted transactions

    JournalPanel* w_panel = nullptr;
    wxSharedPtr<wxListItemAttr> w_attr1;  // style 1
    wxSharedPtr<wxListItemAttr> w_attr2;  // style 2
    wxSharedPtr<wxListItemAttr> w_attr3;  // style 3 (reserved)
    wxSharedPtr<wxListItemAttr> w_attr4;  // style 4 (reserved)
    wxSharedPtr<wxListItemAttr> w_attr5;  // style 5 (reserved for not reconciled)
    wxSharedPtr<wxListItemAttr> w_attr6;  // style 6 (reserved for not reconciled)
    wxSharedPtr<wxListItemAttr> w_attr11; // user-defined style 1
    wxSharedPtr<wxListItemAttr> w_attr12; // user-defined style 2
    wxSharedPtr<wxListItemAttr> w_attr13; // user-defined style 3
    wxSharedPtr<wxListItemAttr> w_attr14; // user-defined style 4
    wxSharedPtr<wxListItemAttr> w_attr15; // user-defined style 5
    wxSharedPtr<wxListItemAttr> w_attr16; // user-defined style 6
    wxSharedPtr<wxListItemAttr> w_attr17; // user-defined style 7
    wxSharedPtr<wxListItemAttr> w_attr21; // user-defined style 1 for not reconciled
    wxSharedPtr<wxListItemAttr> w_attr22; // user-defined style 2 for not reconciled
    wxSharedPtr<wxListItemAttr> w_attr23; // user-defined style 3 for not reconciled
    wxSharedPtr<wxListItemAttr> w_attr24; // user-defined style 4 for not reconciled
    wxSharedPtr<wxListItemAttr> w_attr25; // user-defined style 5 for not reconciled
    wxSharedPtr<wxListItemAttr> w_attr26; // user-defined style 6 for not reconciled
    wxSharedPtr<wxListItemAttr> w_attr27; // user-defined style 7 for not reconciled

public:
    JournalList(
        JournalPanel* panel,
        wxWindow* parent_win,
        const wxWindowID win_id = wxID_ANY
    );
    ~JournalList();

    // override ListBase
    virtual void onColClick(wxListEvent& event) override;
    virtual int  getSortIcon(bool asc) const override;

    // override wxListCtrl
    virtual auto OnGetItemText(long item, long col_nr) const -> wxString override;
    virtual auto OnGetItemAttr(long item) const -> wxListItemAttr* override;
    virtual int  OnGetItemColumnImage(long item, long col_nr) const override;

private:
    void setColumnsInfo();
    void refreshVisualList(bool filter = true);
    void sortList();
    template<class Compare>
    void sortBy(Compare comp, bool ascend);
    void sortTransactions(int col_id, bool ascend);
    auto pasteTrx(const TrxData* tran) -> int64;
    auto getItem(long item, int col_id) const -> const wxString;
    void setExtraTransactionData(const bool single);
    void markItem(long selectedItem);
    void setSelectedId(JournalKey journal_key);
    auto getSelectKeyA() const -> std::vector<JournalKey> { return m_select_key_a; }
    auto getCopyKeyA() const -> std::vector<JournalKey> { return m_copy_key_a; }
    void setSelectKeyA();
    int  getColNr_X(int xPos);
    void doSearchText(const wxString& value);
    void setVisibleItemIndex(long item) { m_top_item_n = item; }
    void markSelectedTransaction();
    void deleteTransactionsByStatus(std::optional<TrxStatus> status_n);
    bool checkForClosedAccounts();
    bool checkTransactionLocked(int64 account_id, mmDate date);
    void setAutomaticColumnSize();

    void onChar(wxKeyEvent& event);
    void onMouseRightClick(wxMouseEvent& event);
    void onListItemActivated(wxListEvent& event);
    void onListItemSelected(wxListEvent&);
    void onListItemDeSelected(wxListEvent&);
    void onListItemFocused(wxListEvent& WXUNUSED(event));
    void onListKeyDown(wxListEvent& event);
    void onNewTrx(wxCommandEvent& event);
    void onDeleteTrx(wxCommandEvent& event);
    void onRestoreTrx(wxCommandEvent& event);
    void onRestoreViewedTrx(wxCommandEvent&);
    void onEditTrx(wxCommandEvent& event);
    void onMoveTrx(wxCommandEvent& event);
    void onViewOtherAccount(wxCommandEvent& event);
    void onViewTrxSplit(wxCommandEvent& event);
    void onOrganizeAttachments(wxCommandEvent& event);
    void onCreateReoccurance(wxCommandEvent& event);
    void onFind(wxCommandEvent&);
    void onCopyText(wxCommandEvent&);
    void onMarkTrx(wxCommandEvent& event);
    void onDeleteViewedTrx(wxCommandEvent& event);
    void onSelectAll(wxCommandEvent& WXUNUSED(event));
    void onCopy(wxCommandEvent& WXUNUSED(event));
    void onPaste(wxCommandEvent& WXUNUSED(event));
    void onDuplicateTrx(wxCommandEvent& event);
    void onEnterSched(wxCommandEvent& event);
    void onSkipSched(wxCommandEvent& event);
    void onSetUserColour(wxCommandEvent& event);
    void onOpenAttachment(wxCommandEvent& event);
    void onSize(wxSizeEvent& event);
    //void onListLeftClick(wxMouseEvent& event);
};

//----------------------------------------------------------------------------

inline static bool SorterByUDFC01(
    const TrxModel::DataExt& i, const TrxModel::DataExt& j
) {
    return (i.UDFC_content[0] < j.UDFC_content[0]);
}

inline static bool SorterByUDFC02(
    const TrxModel::DataExt& i, const TrxModel::DataExt& j
) {
    return (i.UDFC_content[1] < j.UDFC_content[1]);
}

inline static bool SorterByUDFC03(
    const TrxModel::DataExt& i, const TrxModel::DataExt& j
) {
    return (i.UDFC_content[2] < j.UDFC_content[2]);
}

inline static bool SorterByUDFC04(
    const TrxModel::DataExt& i, const TrxModel::DataExt& j
) {
    return (i.UDFC_content[3] < j.UDFC_content[3]);
}

inline static bool SorterByUDFC05(
    const TrxModel::DataExt& i, const TrxModel::DataExt& j
) {
    return (i.UDFC_content[4] < j.UDFC_content[4]);
}

inline static bool SorterByUDFC01_val(
    const TrxModel::DataExt& i, const TrxModel::DataExt& j
) {
    return (i.UDFC_value[0] < j.UDFC_value[0]);
}

inline static bool SorterByUDFC02_val(
    const TrxModel::DataExt& i, const TrxModel::DataExt& j
) {
    return (i.UDFC_value[1] < j.UDFC_value[1]);
}

inline static bool SorterByUDFC03_val(
    const TrxModel::DataExt& i, const TrxModel::DataExt& j
) {
    return (i.UDFC_value[2] < j.UDFC_value[2]);
}

inline static bool SorterByUDFC04_val(
    const TrxModel::DataExt& i, const TrxModel::DataExt& j
) {
    return (i.UDFC_value[3] < j.UDFC_value[3]);
}

inline static bool SorterByUDFC05_val(
    const TrxModel::DataExt& i, const TrxModel::DataExt& j
) {
    return (i.UDFC_value[4] < j.UDFC_value[4]);
}
