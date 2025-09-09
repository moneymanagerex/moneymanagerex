/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013, 2014, 2020, 2021, 2022 Nikolay Akimov
 Copyright (C) 2021-2024 Mark Whalley (mark@ipx.co.uk)

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public as published by
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

#include <wx/clipbrd.h>
#include <algorithm>
#include <wx/sound.h>

#include "assetdialog.h"
#include "attachmentdialog.h"
#include "billsdepositsdialog.h"
#include "constants.h"
#include "filtertransdialog.h"
#include "fusedtransaction.h"
#include "images_list.h"
#include "mmchecking_list.h"
#include "mmcheckingpanel.h"
#include "mmframe.h"
#include "mmSimpleDialogs.h"
#include "sharetransactiondialog.h"
#include "transactionsupdatedialog.h"
#include "transdialog.h"
#include "util.h"
#include "model/Model_Setting.h"
#include "option.h"

//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(TransactionListCtrl, mmListCtrl)
    EVT_CHAR(TransactionListCtrl::onChar)
    EVT_LEFT_DOWN(TransactionListCtrl::onListLeftClick)
    EVT_RIGHT_DOWN(TransactionListCtrl::onMouseRightClick)

    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,  TransactionListCtrl::onListItemActivated)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,   TransactionListCtrl::onListItemSelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY, TransactionListCtrl::onListItemDeSelected)
    EVT_LIST_ITEM_FOCUSED(wxID_ANY,    TransactionListCtrl::onListItemFocused)
    EVT_LIST_KEY_DOWN(wxID_ANY,        TransactionListCtrl::onListKeyDown)

    EVT_MENU(MENU_TREEPOPUP_WITHDRAWAL,            TransactionListCtrl::onNewTransaction)
    EVT_MENU(MENU_TREEPOPUP_DEPOSIT,               TransactionListCtrl::onNewTransaction)
    EVT_MENU(MENU_TREEPOPUP_TRANSFER,              TransactionListCtrl::onNewTransaction)
    EVT_MENU(MENU_TREEPOPUP_DELETE2,               TransactionListCtrl::onDeleteTransaction)
    EVT_MENU(MENU_TREEPOPUP_RESTORE,               TransactionListCtrl::onRestoreTransaction)
    EVT_MENU(MENU_TREEPOPUP_RESTORE_VIEWED,        TransactionListCtrl::onRestoreViewedTransaction)
    EVT_MENU(MENU_TREEPOPUP_EDIT2,                 TransactionListCtrl::onEditTransaction)
    EVT_MENU(MENU_TREEPOPUP_MOVE2,                 TransactionListCtrl::onMoveTransaction)
    EVT_MENU(MENU_TREEPOPUP_VIEW_OTHER_ACCOUNT,    TransactionListCtrl::onViewOtherAccount)
    EVT_MENU(MENU_TREEPOPUP_VIEW_SPLIT_CATEGORIES, TransactionListCtrl::onViewSplitTransaction)
    EVT_MENU(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS,  TransactionListCtrl::onOrganizeAttachments)
    EVT_MENU(MENU_TREEPOPUP_CREATE_REOCCURANCE,    TransactionListCtrl::onCreateReoccurance)
    EVT_MENU(MENU_TREEPOPUP_FIND,                  TransactionListCtrl::onFind)
    EVT_MENU(MENU_TREEPOPUP_COPYTEXT,              TransactionListCtrl::onCopyText)
    EVT_MENU_RANGE(
        MENU_TREEPOPUP_MARKRECONCILED,
        MENU_TREEPOPUP_MARKDELETE,
        TransactionListCtrl::onMarkTransaction
    )
    EVT_MENU_RANGE(
        MENU_TREEPOPUP_DELETE_VIEWED,
        MENU_TREEPOPUP_DELETE_UNRECONCILED,
        TransactionListCtrl::onDeleteViewedTransaction
    )

    EVT_MENU(MENU_ON_SELECT_ALL,            TransactionListCtrl::onSelectAll)
    EVT_MENU(MENU_ON_COPY_TRANSACTION,      TransactionListCtrl::onCopy)
    EVT_MENU(MENU_ON_PASTE_TRANSACTION,     TransactionListCtrl::onPaste)
    EVT_MENU(MENU_ON_NEW_TRANSACTION,       TransactionListCtrl::onNewTransaction)
    EVT_MENU(MENU_ON_DUPLICATE_TRANSACTION, TransactionListCtrl::onDuplicateTransaction)
    EVT_MENU(MENU_ON_ENTER_SCHEDULED,       TransactionListCtrl::onEnterScheduled)
    EVT_MENU(MENU_ON_SKIP_SCHEDULED,        TransactionListCtrl::onSkipScheduled)
    EVT_MENU_RANGE(
        MENU_ON_SET_UDC0,
        MENU_ON_SET_UDC7,
        TransactionListCtrl::onSetUserColour
    )
wxEND_EVENT_TABLE();

const std::vector<ListColumnInfo> TransactionListCtrl::LIST_INFO = {
    { LIST_ID_ICON,        true, _n("Icon"),         25,  _FC, false },
    { LIST_ID_ID,          true, _n("ID"),           _WA, _FR, true },
    { LIST_ID_DATE,        true, _n("Date"),         112, _FL, true },
    { LIST_ID_TIME,        true, _n("Time"),         70,  _FL, true },
    { LIST_ID_NUMBER,      true, _n("Number"),       70,  _FL, true },
    { LIST_ID_ACCOUNT,     true, _n("Account"),      100, _FL, true },
    { LIST_ID_PAYEE_STR,   true, _n("Payee"),        150, _FL, true },
    { LIST_ID_STATUS,      true, _n("Status"),       _WH, _FC, true },
    { LIST_ID_CATEGORY,    true, _n("Category"),     150, _FL, true },
    { LIST_ID_TAGS,        true, _n("Tags"),         250, _FL, true },
    { LIST_ID_WITHDRAWAL,  true, _n("Withdrawal"),   _WH, _FR, true },
    { LIST_ID_DEPOSIT,     true, _n("Deposit"),      _WH, _FR, true },
    { LIST_ID_BALANCE,     true, _n("Balance"),      _WH, _FR, true },
    { LIST_ID_CREDIT,      true, _n("Credit"),       _WH, _FR, true },
    { LIST_ID_NOTES,       true, _n("Notes"),        250, _FL, true },
    { LIST_ID_DELETEDTIME, true, _n("Deleted On"),   _WA, _FL, true },
    { LIST_ID_UDFC01,      false, "",                100, _FL, true },
    { LIST_ID_UDFC02,      false, "",                100, _FL, true },
    { LIST_ID_UDFC03,      false, "",                100, _FL, true },
    { LIST_ID_UDFC04,      false, "",                100, _FL, true },
    { LIST_ID_UDFC05,      false, "",                100, _FL, true },
    { LIST_ID_UPDATEDTIME, true, _n("Last Updated"), _WA, _FL, true },
    { LIST_ID_SN,          true, _n("SN"),           _WA, _FR, true },
};

//----------------------------------------------------------------------------

TransactionListCtrl::TransactionListCtrl(
    mmCheckingPanel *cp,
    wxWindow *parent,
    const wxWindowID id
) :
    mmListCtrl(parent, id),
    m_cp(cp),
    m_attr1(new wxListItemAttr(
        *bestFontColour(m_cp->isAllTrans() ?
            mmThemeMetaColour(meta::COLOR_LISTALT0A) :
            mmThemeMetaColour(meta::COLOR_LISTALT0)
        ),
        m_cp->isAllTrans() ?
            mmThemeMetaColour(meta::COLOR_LISTALT0A) :
            mmThemeMetaColour(meta::COLOR_LISTALT0),
        GetFont()
    )),
    m_attr2(new wxListItemAttr(
        *bestFontColour(mmThemeMetaColour(meta::COLOR_LIST)),
        mmThemeMetaColour(meta::COLOR_LIST),
        GetFont()
    )),
    m_attr3(new wxListItemAttr(
        mmThemeMetaColour(meta::COLOR_LISTFUTURE),
        m_cp->isAllTrans() ?
            mmThemeMetaColour(meta::COLOR_LISTALT0A) :
            mmThemeMetaColour(meta::COLOR_LISTALT0),
        GetFont()
    )),
    m_attr4(new wxListItemAttr(
        mmThemeMetaColour(meta::COLOR_LISTFUTURE),
        wxNullColour, GetFont()
    )),
    m_attr5(new wxListItemAttr(
        *bestFontColour(m_cp->isAllTrans() ?
            mmThemeMetaColour(meta::COLOR_LISTALT0A) :
            mmThemeMetaColour(meta::COLOR_LISTALT0)
        ),
        m_cp->isAllTrans() ?
            mmThemeMetaColour(meta::COLOR_LISTALT0A) :
            mmThemeMetaColour(meta::COLOR_LISTALT0),
        GetFont().Bold()
    )),
    m_attr6(new wxListItemAttr(
        *bestFontColour(mmThemeMetaColour(meta::COLOR_LIST)),
        mmThemeMetaColour(meta::COLOR_LIST),
        GetFont().Bold()
    )),
    m_attr11(new wxListItemAttr(
        *bestFontColour(mmColors::userDefColor1),
        mmColors::userDefColor1,
        GetFont()
    )),
    m_attr12(new wxListItemAttr(
        *bestFontColour(mmColors::userDefColor2),
        mmColors::userDefColor2,
        GetFont()
    )),
    m_attr13(new wxListItemAttr(
        *bestFontColour(mmColors::userDefColor3),
        mmColors::userDefColor3,
        GetFont()
    )),
    m_attr14(new wxListItemAttr(
        *bestFontColour(mmColors::userDefColor4),
        mmColors::userDefColor4,
        GetFont()
    )),
    m_attr15(new wxListItemAttr(
        *bestFontColour(mmColors::userDefColor5),
        mmColors::userDefColor5,
        GetFont()
    )),
    m_attr16(new wxListItemAttr(
        *bestFontColour(mmColors::userDefColor6),
        mmColors::userDefColor6,
        GetFont()
    )),
    m_attr17(new wxListItemAttr(
        *bestFontColour(mmColors::userDefColor7),
        mmColors::userDefColor7,
        GetFont()
    )),
    m_attr21(new wxListItemAttr(
        *bestFontColour(mmColors::userDefColor1),
        mmColors::userDefColor1,
        GetFont().Bold()
    )),
    m_attr22(new wxListItemAttr(
        *bestFontColour(mmColors::userDefColor2),
        mmColors::userDefColor2,
        GetFont().Bold()
    )),
    m_attr23(new wxListItemAttr(
        *bestFontColour(mmColors::userDefColor3),
        mmColors::userDefColor3,
        GetFont().Bold()
    )),
    m_attr24(new wxListItemAttr(
        *bestFontColour(mmColors::userDefColor4),
        mmColors::userDefColor4,
        GetFont().Bold()
    )),
    m_attr25(new wxListItemAttr(
        *bestFontColour(mmColors::userDefColor5),
        mmColors::userDefColor5,
        GetFont().Bold()
    )),
    m_attr26(new wxListItemAttr(
        *bestFontColour(mmColors::userDefColor6),
        mmColors::userDefColor6,
        GetFont().Bold()
    )),
    m_attr27(new wxListItemAttr(
        *bestFontColour(mmColors::userDefColor7),
        mmColors::userDefColor7,
        GetFont().Bold()
    ))
{
    wxASSERT(m_cp);
    m_selected_id.clear();
    m_selectedForCopy.clear();
    mmThemeMetaColour(this, meta::COLOR_LISTPANEL);

    const wxAcceleratorEntry entries[] = {
        wxAcceleratorEntry(wxACCEL_CTRL, 'A', MENU_ON_SELECT_ALL),
        wxAcceleratorEntry(wxACCEL_CTRL, 'C', MENU_ON_COPY_TRANSACTION),
        wxAcceleratorEntry(wxACCEL_CTRL, 'V', MENU_ON_PASTE_TRANSACTION),
        wxAcceleratorEntry(wxACCEL_CTRL, '0', MENU_ON_SET_UDC0),
        wxAcceleratorEntry(wxACCEL_CTRL, '1', MENU_ON_SET_UDC1),
        wxAcceleratorEntry(wxACCEL_CTRL, '2', MENU_ON_SET_UDC2),
        wxAcceleratorEntry(wxACCEL_CTRL, '3', MENU_ON_SET_UDC3),
        wxAcceleratorEntry(wxACCEL_CTRL, '4', MENU_ON_SET_UDC4),
        wxAcceleratorEntry(wxACCEL_CTRL, '5', MENU_ON_SET_UDC5),
        wxAcceleratorEntry(wxACCEL_CTRL, '6', MENU_ON_SET_UDC6),
        wxAcceleratorEntry(wxACCEL_CTRL, '7', MENU_ON_SET_UDC7)
    };
    wxAcceleratorTable tab(sizeof(entries) / sizeof(*entries), entries);
    SetAcceleratorTable(tab);

    setColumnsInfo();
    createColumns();

    m_today = Option::instance().UseTransDateTime() ?
        wxDateTime::Now().FormatISOCombined() :
        wxDateTime(23, 59, 59, 999).FormatISOCombined();

    SetSingleStyle(wxLC_SINGLE_SEL, false);
}

TransactionListCtrl::~TransactionListCtrl()
{}

//----------------------------------------------------------------------------

void TransactionListCtrl::setColumnsInfo()
{
    if (m_cp->isDeletedTrans()) {
        m_setting_name = "DELETED";
        o_col_order_prefix = "ALLTRANS";
        o_col_width_prefix = "ALLTRANS_COLV2";
        o_sort_prefix = "DELETED";
    }
    else if (m_cp->isAccount()) {
        m_setting_name = "TRANS1";
        // note: migrate from CHECKING_COLUMNORDER for all account types
        o_col_order_prefix = "CHECKING";
        o_col_width_prefix = "CHECK2_COLV2";
        o_sort_prefix = "CHECK";
    }
    else {
        m_setting_name = "TRANS2";
        o_col_order_prefix = "ALLTRANS";
        o_col_width_prefix = "ALLTRANS_COLV2";
        // note: MULTI_{SORT_COL,ASC}*` are ignored (not migrated)
        o_sort_prefix = "ALLTRANS";
    }

    m_col_info_id = LIST_INFO;
    m_col_disabled_id.clear();
    m_col_id_nr.clear();

    if (!Option::instance().UseTransDateTime())
        m_col_disabled_id.insert(LIST_ID_TIME);
    if (m_cp->isAccount() && m_cp->m_account->CREDITLIMIT == 0)
        m_col_disabled_id.insert(LIST_ID_CREDIT);

    m_col_id_nr.push_back(LIST_ID_ICON);
    m_col_id_nr.push_back(LIST_ID_SN);
    m_col_id_nr.push_back(LIST_ID_ID);
    m_col_id_nr.push_back(LIST_ID_DATE);
    m_col_id_nr.push_back(LIST_ID_TIME);
    m_col_id_nr.push_back(LIST_ID_NUMBER);
    if (!m_cp->isAccount())
        m_col_id_nr.push_back(LIST_ID_ACCOUNT);
    m_col_id_nr.push_back(LIST_ID_PAYEE_STR);
    m_col_id_nr.push_back(LIST_ID_STATUS);
    m_col_id_nr.push_back(LIST_ID_CATEGORY);
    m_col_id_nr.push_back(LIST_ID_TAGS);
    m_col_id_nr.push_back(LIST_ID_WITHDRAWAL);
    m_col_id_nr.push_back(LIST_ID_DEPOSIT);
    if (m_cp->isAccount()) {
        m_col_id_nr.push_back(LIST_ID_BALANCE);
        m_col_id_nr.push_back(LIST_ID_CREDIT);
    }
    m_col_id_nr.push_back(LIST_ID_NOTES);
    if (m_cp->isDeletedTrans())
        m_col_id_nr.push_back(LIST_ID_DELETEDTIME);

    const auto& ref_type = Model_Checking::refTypeName;
    int col_id = LIST_ID_UDFC01;
    for (const auto& udfc_entry : Model_CustomField::UDFC_FIELDS()) {
        if (col_id > LIST_ID_UDFC05) break;
        if (udfc_entry.empty()) continue;

        const auto& name = Model_CustomField::getUDFCName(ref_type, udfc_entry);
        if (!name.IsEmpty() && name != udfc_entry) {
            m_col_info_id[col_id].header = name;
            const auto& type = Model_CustomField::getUDFCType(ref_type, udfc_entry);
            if (type == Model_CustomField::TYPE_ID_DECIMAL || type == Model_CustomField::TYPE_ID_INTEGER)
                m_col_info_id[col_id].format = _FR;
            else if (type == Model_CustomField::TYPE_ID_BOOLEAN)
                m_col_info_id[col_id].format = _FC;
            m_col_id_nr.push_back(col_id);
        }
        col_id++;
    }

    m_col_id_nr.push_back(LIST_ID_UPDATEDTIME);

    m_sort_col_id = { LIST_ID_DATE, LIST_ID_ID };
}

int TransactionListCtrl::getSortIcon(bool asc) const
{
    return asc ? mmCheckingPanel::ICON_DESC : mmCheckingPanel::ICON_ASC;
}

void TransactionListCtrl::refreshVisualList(bool filter)
{
    wxLogDebug("refreshVisualList: %i selected, filter: %d", GetSelectedItemCount(), filter);

    // Grab the selected transactions unless we have freshly pasted transactions in which case use them
    if (m_pasted_id.empty()) {
        findSelectedTransactions();
    }
    else {
        m_selected_id.clear();
        m_selected_id.insert(std::end(m_selected_id), std::begin(m_pasted_id), std::end(m_pasted_id));
        m_pasted_id.clear();    // Now clear them
    }

    m_today = Option::instance().UseTransDateTime() ?
        wxDateTime::Now().FormatISOCombined() :
        wxDateTime(23, 59, 59, 999).FormatISOCombined();
    this->SetEvtHandlerEnabled(false);
    Hide();

    if (filter)
        m_cp->filterList();
    SetItemCount(m_trans.size());
    Show();
    sortList();
    markSelectedTransaction();

    long i = static_cast<long>(m_trans.size());
    if (m_topItemIndex > i || m_topItemIndex < 0)
        m_topItemIndex = getSortAsc(0) ? i - 1 : 0;

    i = 0;
    for(const auto& entry : m_trans) {
        int64 id = !entry.m_repeat_num ? entry.TRANSID : entry.m_bdid;
        for (const auto& item : m_selected_id) {
            if (item.first == id && item.second == entry.m_repeat_num) {
                SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                SetItemState(i, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
                EnsureVisible(i);
            }
        }
        i++;
    }
    findSelectedTransactions();

    if (m_topItemIndex >= 0 && m_topItemIndex < i && m_selected_id.empty())
        EnsureVisible(m_topItemIndex);

    m_cp->updateHeader();
    setExtraTransactionData(GetSelectedItemCount() == 1);
    this->SetEvtHandlerEnabled(true);
    Refresh();
    Update();
    SetFocus();
}

void TransactionListCtrl::sortList()
{
    if (m_trans.empty()) return;

    sortTransactions(getSortColId(1), getSortAsc(1));
    sortTransactions(getSortColId(0), getSortAsc(0));

    wxString sortText = wxString::Format(
        "%s:  %s %s / %s %s", _t("Sorted by"),
        getColHeader(getSortColId(0), true), getSortAsc(0) ? L"\u25B2" : L"\u25BC",
        getColHeader(getSortColId(1), true), getSortAsc(1) ? L"\u25B2" : L"\u25BC"
    );
    m_cp->m_header_sortOrder->SetLabelText(sortText);

    if (getSortColId(0) == LIST_ID_SN)
        m_cp->showTips(_t("SN (Sequence Number) has the same order as Date/ID (or Date/Time/ID if Time is enabled)."));
    else if (getSortColId(0) == LIST_ID_ID)
        m_cp->showTips(_t("ID (identification number) is increasing with the time of creation in the database."));
    else if (getSortColId(0) == LIST_ID_BALANCE)
        m_cp->showTips(_t("Balance is calculated in the order of SN (Sequence Number)."));

    RefreshItems(0, m_trans.size() - 1);
}

template<class Compare>
void TransactionListCtrl::sortBy(Compare comp, bool ascend)
{
    if (ascend)
        std::stable_sort(this->m_trans.begin(), this->m_trans.end(), comp);
    else
        std::stable_sort(this->m_trans.rbegin(), this->m_trans.rend(), comp);
}

void TransactionListCtrl::sortTransactions(int col_id, bool ascend)
{
    const auto& ref_type = Model_Checking::refTypeName;
    Model_CustomField::TYPE_ID type;

    switch (col_id) {
    case TransactionListCtrl::LIST_ID_SN:
        sortBy(Fused_Transaction::SorterByFUSEDTRANSSN(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_ID:
        sortBy(Fused_Transaction::SorterByFUSEDTRANSID(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_NUMBER:
        sortBy(Model_Checking::SorterByNUMBER(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_ACCOUNT:
        sortBy(SorterByACCOUNTNAME(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_PAYEE_STR:
        sortBy(SorterByPAYEENAME(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_STATUS:
        sortBy(SorterBySTATUS(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_CATEGORY:
        sortBy(SorterByCATEGNAME(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_TAGS:
        sortBy(Model_Checking::SorterByTAGNAMES(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_WITHDRAWAL:
        sortBy(Model_Checking::SorterByWITHDRAWAL(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_DEPOSIT:
        sortBy(Model_Checking::SorterByDEPOSIT(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_BALANCE:
        sortBy(Model_Checking::SorterByBALANCE(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_CREDIT:
        sortBy(Model_Checking::SorterByBALANCE(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_NOTES:
        sortBy(SorterByNOTES(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_DATE:
        sortBy(Model_Checking::SorterByTRANSDATE_DATE(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_TIME:
        sortBy(Model_Checking::SorterByTRANSDATE_TIME(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_DELETEDTIME:
        sortBy(SorterByDELETEDTIME(), ascend);
        break;
    case TransactionListCtrl::LIST_ID_UDFC01:
        type = Model_CustomField::getUDFCType(ref_type, "UDFC01");
        if (type == Model_CustomField::TYPE_ID_DECIMAL || type == Model_CustomField::TYPE_ID_INTEGER)
            sortBy(SorterByUDFC01_val, ascend);
        else
            sortBy(SorterByUDFC01, ascend);
        break;
    case TransactionListCtrl::LIST_ID_UDFC02:
        type = Model_CustomField::getUDFCType(ref_type, "UDFC02");
        if (type == Model_CustomField::TYPE_ID_DECIMAL || type == Model_CustomField::TYPE_ID_INTEGER)
            sortBy(SorterByUDFC02_val, ascend);
        else
            sortBy(SorterByUDFC02, ascend);
        break;
    case TransactionListCtrl::LIST_ID_UDFC03:
        type = Model_CustomField::getUDFCType(ref_type, "UDFC03");
        if (type == Model_CustomField::TYPE_ID_DECIMAL || type == Model_CustomField::TYPE_ID_INTEGER)
            sortBy(SorterByUDFC03_val, ascend);
        else
            sortBy(SorterByUDFC03, ascend);
        break;
    case TransactionListCtrl::LIST_ID_UDFC04:
        type = Model_CustomField::getUDFCType(ref_type, "UDFC04");
        if (type == Model_CustomField::TYPE_ID_DECIMAL || type == Model_CustomField::TYPE_ID_INTEGER)
            sortBy(SorterByUDFC04_val, ascend);
        else
            sortBy(SorterByUDFC04, ascend);
        break;
    case TransactionListCtrl::LIST_ID_UDFC05:
        type = Model_CustomField::getUDFCType(ref_type, "UDFC05");
        if (type == Model_CustomField::TYPE_ID_DECIMAL || type == Model_CustomField::TYPE_ID_INTEGER)
            sortBy(SorterByUDFC05_val, ascend);
        else
            sortBy(SorterByUDFC05, ascend);
        break;
    case TransactionListCtrl::LIST_ID_UPDATEDTIME:
        sortBy(SorterByLASTUPDATEDTIME(), ascend);
        break;
    default:
        break;
    }
}

//----------------------------------------------------------------------------

wxString TransactionListCtrl::OnGetItemText(long item, long col_nr) const
{
    return getItem(item, getColId_Nr(static_cast<int>(col_nr)));
}

// Returns the icon to be shown for each transaction for the required column
int TransactionListCtrl::OnGetItemColumnImage(long item, long col_nr) const
{
    if (m_trans.empty())
        return -1;

    int col_id = getColId_Nr(static_cast<int>(col_nr));
    if (col_id != LIST_ID_ICON)
        return -1;

    wxString status = getItem(item, LIST_ID_STATUS);
    if (status.length() > 1) status = status.Mid(2, 1);

    if (status == Model_Checking::STATUS_KEY_FOLLOWUP)
        return mmCheckingPanel::ICON_FOLLOWUP;
    else if (status == Model_Checking::STATUS_KEY_RECONCILED)
        return mmCheckingPanel::ICON_RECONCILED;
    else if (status == Model_Checking::STATUS_KEY_VOID)
        return mmCheckingPanel::ICON_VOID;
    else if (status == Model_Checking::STATUS_KEY_DUPLICATE)
        return mmCheckingPanel::ICON_DUPLICATE;
    else
        return mmCheckingPanel::ICON_UNRECONCILED;
}

// Failed wxASSERT will hang application if active modal dialog presents on screen.
// Assertion's message box will be hidden until you press tab to activate one.
wxListItemAttr* TransactionListCtrl::OnGetItemAttr(long item) const
{
    if (item < 0 || item >= static_cast<int>(m_trans.size())) return 0;

    bool in_the_future = Model_Checking::TRANSDATE(m_trans[item]).FormatISOCombined() > m_today;
    if (in_the_future && Option::instance().getDoNotColorFuture()) {
        return (item % 2 ? m_attr3.get() : m_attr4.get());
    }

    bool mark_not_reconciled = Option::instance().getDoSpecialColorReconciled() && !in_the_future && m_trans[item].STATUS != Model_Checking::STATUS_KEY_RECONCILED;

    // apply alternating background pattern
    int user_color_id = m_trans[item].COLOR.GetValue();
    if (user_color_id < 0 || user_color_id > 7) {
        user_color_id = 0;
    }
    switch (user_color_id) {
    case 0:
        if (in_the_future) {
            return (item % 2 ? m_attr3.get() : m_attr4.get());
        }
        break;
    case 1: return mark_not_reconciled ? m_attr21.get() : m_attr11.get();
    case 2: return mark_not_reconciled ? m_attr22.get() : m_attr12.get();
    case 3: return mark_not_reconciled ? m_attr23.get() : m_attr13.get();
    case 4: return mark_not_reconciled ? m_attr24.get() : m_attr14.get();
    case 5: return mark_not_reconciled ? m_attr25.get() : m_attr15.get();
    case 6: return mark_not_reconciled ? m_attr26.get() : m_attr16.get();
    case 7: return mark_not_reconciled ? m_attr27.get() : m_attr17.get();
    }

    return mark_not_reconciled ? (item % 2 ? m_attr5.get() : m_attr6.get()) : (item % 2 ? m_attr1.get() : m_attr2.get());
}

void TransactionListCtrl::OnColClick(wxListEvent& event)
{
    findSelectedTransactions();
    int col_nr = (event.GetId() == MENU_HEADER_SORT) ?  m_sel_col_nr : event.GetColumn();
    if (!isValidColNr(col_nr))
        return;
    int col_id = getColId_Nr(col_nr);
    if (!m_col_info_id[col_id].sortable)
        return;

    if (m_sort_col_id[0] != col_id) {
        m_sort_col_id[1] = m_sort_col_id[0];
        m_sort_asc[1] = m_sort_asc[0];
        m_sort_col_id[0] = col_id;
    }
    else if (event.GetId() != MENU_HEADER_SORT) {
        m_sort_asc[0] = !m_sort_asc[0];
    }

    // #7080: Decouple DATE and ID, since SN may be used instead of ID.
    /*
        // If primary is DATE, then set secondary to ID in the same direction
        if (getSortColId(0) == LIST_ID_DATE) {
            m_sort_col_id[1] = LIST_ID_ID;
            m_sort_asc[1] = m_sort_asc[0];
        }
    */

    updateSortIcon();
    savePreferences();

    refreshVisualList(false);
}

//----------------------------------------------------------------------------

// If any of these keys are encountered, the search for the event handler
// should continue as these keys may be processed by the operating system.
void TransactionListCtrl::onChar(wxKeyEvent& event)
{

    int key = event.GetKeyCode();
    if (key == WXK_ALT ||
        key == WXK_COMMAND ||
        key == WXK_UP ||
        key == WXK_DOWN ||
        key == WXK_LEFT ||
        key == WXK_RIGHT ||
        key == WXK_HOME ||
        key == WXK_END ||
        key == WXK_PAGEUP ||
        key == WXK_PAGEDOWN ||
        key == WXK_NUMPAD_UP ||
        key == WXK_NUMPAD_DOWN ||
        key == WXK_NUMPAD_LEFT ||
        key == WXK_NUMPAD_RIGHT ||
        key == WXK_NUMPAD_PAGEDOWN ||
        key == WXK_NUMPAD_PAGEUP ||
        key == WXK_NUMPAD_HOME ||
        key == WXK_NUMPAD_END ||
        key == WXK_DELETE ||
        key == WXK_NUMPAD_DELETE ||
        key == WXK_TAB ||
        key == WXK_RETURN ||
        key == WXK_NUMPAD_ENTER ||
        key == WXK_SPACE ||
        key == WXK_NUMPAD_SPACE
    ) {
        event.Skip();
    }
}

void TransactionListCtrl::onListLeftClick(wxMouseEvent& event)
{
    wxLogDebug("onListLeftClick: %i selected", GetSelectedItemCount());
    event.Skip();
}

void TransactionListCtrl::onMouseRightClick(wxMouseEvent& event)
{
    rightClickFilter_ = "";
    copyText_ = "";
    wxLogDebug("onMouseRightClick: %i selected", GetSelectedItemCount());
    int selected = GetSelectedItemCount();

    bool is_nothing_selected = (selected < 1);
    bool multiselect = (selected > 1);
    bool type_transfer = false;
    bool have_category = false;
    bool is_foreign = false;
    if (selected == 1) {
        Fused_Transaction::IdRepeat id = m_selected_id[0];
        Fused_Transaction::Full_Data tran = !id.second ?
            Fused_Transaction::Full_Data(*Model_Checking::instance().get(id.first)) :
            Fused_Transaction::Full_Data(*Model_Billsdeposits::instance().get(id.first));

        if (Model_Checking::type_id(tran.TRANSCODE) == Model_Checking::TYPE_ID_TRANSFER)
            type_transfer = true;
        if (!tran.has_split())
            have_category = true;
        if (Model_Checking::foreignTransaction(tran))
            is_foreign = true;
    }
    wxMenu menu;
    if (!m_cp->isDeletedTrans()) {
        menu.Append(MENU_TREEPOPUP_WITHDRAWAL, _tu("New &Withdrawal…"));
        menu.Append(MENU_TREEPOPUP_DEPOSIT, _tu("New &Deposit…"));
        if (Model_Account::instance().all_checking_account_names(true).size() > 1)
            menu.Append(MENU_TREEPOPUP_TRANSFER, _tu("New &Transfer…"));

        menu.AppendSeparator();

        menu.Append(MENU_TREEPOPUP_EDIT2, (1 == selected) ? _tu("&Edit Transaction…") : _tu("&Edit Transactions…"));
        if (is_nothing_selected)
            menu.Enable(MENU_TREEPOPUP_EDIT2, false);

        menu.Append(MENU_ON_COPY_TRANSACTION, wxPLURAL("&Copy Transaction", "&Copy Transactions", selected));
        if (is_nothing_selected)
            menu.Enable(MENU_ON_COPY_TRANSACTION, false);

        if (m_cp->isAccount()) {   // Enable paste in single account view
            int toPaste = m_selectedForCopy.size();
            menu.Append(
                MENU_ON_PASTE_TRANSACTION,
                wxString::Format(
                    wxPLURAL("&Paste Transaction", "&Paste Transactions (%d)",
                        (toPaste < 2) ? 1 : toPaste
                    ),
                    toPaste
                )
            );
            if (toPaste < 1)
                menu.Enable(MENU_ON_PASTE_TRANSACTION, false);
        }

        menu.Append(MENU_ON_DUPLICATE_TRANSACTION, _tu("D&uplicate Transaction…"));
        if (is_nothing_selected || multiselect)
            menu.Enable(MENU_ON_DUPLICATE_TRANSACTION, false);

        menu.Append(MENU_TREEPOPUP_MOVE2, (1 == selected) ? _tu("&Move Transaction…") : _tu("&Move Transactions…"));
        if (is_nothing_selected || type_transfer || (Model_Account::money_accounts_num() < 2) || is_foreign)
            menu.Enable(MENU_TREEPOPUP_MOVE2, false);

        menu.AppendSeparator();

        menu.Append(MENU_TREEPOPUP_VIEW_OTHER_ACCOUNT, _t("&View In Other Account"));
        if (!m_cp->isAccount() || is_nothing_selected || multiselect || is_foreign || !type_transfer)
            menu.Enable(MENU_TREEPOPUP_VIEW_OTHER_ACCOUNT, false);

        menu.Append(MENU_TREEPOPUP_VIEW_SPLIT_CATEGORIES, _t("&View Split Categories"));
        if (is_nothing_selected || multiselect || have_category)
            menu.Enable(MENU_TREEPOPUP_VIEW_SPLIT_CATEGORIES, false);

        menu.Append(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, _tu("&Organize Attachments…"));
        if (is_nothing_selected || multiselect)
            menu.Enable(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, false);

        menu.Append(
            MENU_TREEPOPUP_CREATE_REOCCURANCE,
            _tu("Create Scheduled T&ransaction…")
        );
        if (is_nothing_selected || multiselect)
            menu.Enable(MENU_TREEPOPUP_CREATE_REOCCURANCE, false);
    }
    else {
        menu.Append(
            MENU_TREEPOPUP_RESTORE,
            (1 == selected) ? _tu("&Restore selected transaction…") : _tu("&Restore selected transactions…")
        );
        if (is_nothing_selected)
            menu.Enable(MENU_TREEPOPUP_RESTORE, false);
        menu.Append(
            MENU_TREEPOPUP_RESTORE_VIEWED,
            _tu("Restore &all transactions in current view…")
        );
    }
    bool columnIsAmount = false;
    int col_nr = getColNr_X(event.GetX());
    int flags;
    unsigned long row = HitTest(event.GetPosition(), flags);
    if (row < m_trans.size() && (flags & wxLIST_HITTEST_ONITEM) && col_nr < getColNrSize()) {
        int col_id = getColId_Nr(col_nr);
        wxString menuItemText;
        wxString refType = Model_Checking::refTypeName;
        wxDateTime datetime;
        wxString dateFormat = Option::instance().getDateFormat();

        switch (col_id) {
        case LIST_ID_SN:
            copyText_ = m_trans[row].displaySN;
            break;
        case LIST_ID_ID:
            copyText_ = m_trans[row].displayID;
            break;
        case LIST_ID_DATE: {
            copyText_ = menuItemText = mmGetDateTimeForDisplay(m_trans[row].TRANSDATE);
            wxString strDate = Model_Checking::TRANSDATE(m_trans[row]).FormatISODate();
            rightClickFilter_ = "{\n\"DATE1\": \"" + strDate + "\",\n\"DATE2\" : \"" + strDate + "T23:59:59" + "\"\n}";
            break;
        }
        case LIST_ID_NUMBER:
            copyText_ = menuItemText = m_trans[row].TRANSACTIONNUMBER;
            rightClickFilter_ = "{\n\"NUMBER\": \"" + menuItemText + "\"\n}";
            break;
        case LIST_ID_ACCOUNT:
            copyText_ = menuItemText = m_trans[row].ACCOUNTNAME;
            rightClickFilter_ = "{\n\"ACCOUNT\": [\n\"" + menuItemText + "\"\n]\n}";
            break;
        case LIST_ID_PAYEE_STR:
            copyText_ = m_trans[row].PAYEENAME;
            if (!Model_Checking::is_transfer(m_trans[row].TRANSCODE)) {
                menuItemText = m_trans[row].PAYEENAME;
                rightClickFilter_ = "{\n\"PAYEE\": \"" + menuItemText + "\"\n}";
            }
            break;
        case LIST_ID_STATUS:
            copyText_ = menuItemText = Model_Checking::status_name(m_trans[row].STATUS);
            rightClickFilter_ = "{\n\"STATUS\": \"" + menuItemText + "\"\n}";
            break;
        case LIST_ID_CATEGORY:
            copyText_ = m_trans[row].CATEGNAME;
            if (!m_trans[row].has_split()) {
                menuItemText = m_trans[row].CATEGNAME;
                rightClickFilter_ = "{\n\"CATEGORY\": \"" + menuItemText + "\",\n\"SUBCATEGORYINCLUDE\": false\n}";
            }
            break;
        case LIST_ID_TAGS:
            if (!m_trans[row].has_split() && m_trans[row].has_tags()) {
                copyText_ = menuItemText = m_trans[row].TAGNAMES;
                // build the tag filter json
                for (const auto& tag : m_trans[row].m_tags) {
                    rightClickFilter_ += (rightClickFilter_.IsEmpty() ? "{\n\"TAGS\": [\n" : ",\n") + wxString::Format("%lld", tag.TAGID);
                }
                rightClickFilter_ += "\n]\n}";
            }
            break;
        case LIST_ID_WITHDRAWAL: {
            columnIsAmount = true;
            Model_Account::Data* account = Model_Account::instance().get(m_trans[row].ACCOUNTID_W);
            Model_Currency::Data* currency = account ? Model_Currency::instance().get(account->CURRENCYID) : nullptr;
            if (currency) {
                copyText_ = Model_Currency::toString(m_trans[row].TRANSAMOUNT_W, currency);
                menuItemText = wxString::Format("%.2f", m_trans[row].TRANSAMOUNT_W);
                rightClickFilter_ = "{\n\"AMOUNT_MIN\": " + menuItemText + ",\n\"AMOUNT_MAX\" : " + menuItemText + "\n}";
            }
            break;
        }
        case LIST_ID_DEPOSIT: {
            columnIsAmount = true;
            Model_Account::Data* account = Model_Account::instance().get(m_trans[row].ACCOUNTID_D);
            Model_Currency::Data* currency = account ? Model_Currency::instance().get(account->CURRENCYID) : nullptr;
            if (currency) {
                copyText_ = Model_Currency::toString(m_trans[row].TRANSAMOUNT_D, currency);
                menuItemText = wxString::Format("%.2f", m_trans[row].TRANSAMOUNT_D);
                rightClickFilter_ = "{\n\"AMOUNT_MIN\": " + menuItemText + ",\n\"AMOUNT_MAX\" : " + menuItemText + "\n}";
            }
            break;
        }
        case LIST_ID_BALANCE:
            copyText_ = Model_Currency::toString(m_trans[row].ACCOUNT_BALANCE, m_cp->m_currency);
            break;
        case LIST_ID_CREDIT:
            copyText_ = Model_Currency::toString(
                m_cp->m_account->CREDITLIMIT + m_trans[row].ACCOUNT_BALANCE,
                m_cp->m_currency
            );
            break;
        case LIST_ID_NOTES:
            copyText_ = menuItemText = m_trans[row].NOTES;
            rightClickFilter_ = "{\n\"NOTES\": \"" + menuItemText + "\"\n}";
            break;
        case LIST_ID_DELETEDTIME:
            datetime.ParseISOCombined(m_trans[row].DELETEDTIME);
            if(datetime.IsValid())
                copyText_ = mmGetDateTimeForDisplay(datetime.FromUTC().FormatISOCombined(), dateFormat + " %H:%M:%S");
            break;
        case LIST_ID_UPDATEDTIME:
            datetime.ParseISOCombined(m_trans[row].LASTUPDATEDTIME);
            if (datetime.IsValid())
                copyText_ = mmGetDateTimeForDisplay(datetime.FromUTC().FormatISOCombined(), dateFormat + " %H:%M:%S");
            break;
        case LIST_ID_UDFC01:
            copyText_ = menuItemText = m_trans[row].UDFC_content[0];
            rightClickFilter_ = wxString::Format("{\n\"CUSTOM%lld\": \"" + menuItemText + "\"\n}", Model_CustomField::getUDFCID(refType, "UDFC01"));
            break;
        case LIST_ID_UDFC02:
            copyText_ = menuItemText = m_trans[row].UDFC_content[1];
            rightClickFilter_ = wxString::Format("{\n\"CUSTOM%lld\": \"" + menuItemText + "\"\n}", Model_CustomField::getUDFCID(refType, "UDFC02"));
            break;
        case LIST_ID_UDFC03:
            copyText_ = menuItemText = m_trans[row].UDFC_content[2];
            rightClickFilter_ = wxString::Format("{\n\"CUSTOM%lld\": \"" + menuItemText + "\"\n}", Model_CustomField::getUDFCID(refType, "UDFC03"));
            break;
        case LIST_ID_UDFC04:
            copyText_ = menuItemText = m_trans[row].UDFC_content[3];
            rightClickFilter_ = wxString::Format("{\n\"CUSTOM%lld\": \"" + menuItemText + "\"\n}", Model_CustomField::getUDFCID(refType, "UDFC04"));
            break;
        case LIST_ID_UDFC05:
            copyText_ = menuItemText = m_trans[row].UDFC_content[4];
            rightClickFilter_ = wxString::Format("{\n\"CUSTOM%lld\": \"" + menuItemText + "\"\n}", Model_CustomField::getUDFCID(refType, "UDFC05"));
            break;
        default:
            break;
        }

        if (!menuItemText.IsEmpty() || !copyText_.IsEmpty()) {
            menu.AppendSeparator();
            if (!menuItemText.IsEmpty()) {
                if (menuItemText.length() > 30)
                    menuItemText = menuItemText.SubString(0, 30).Append(L"\u2026");
                menu.Append(MENU_TREEPOPUP_FIND, wxString::Format(
                    _t("&Find all transactions with %s '%s'"),
                    (columnIsAmount ? _t("Amount") : getColHeader(col_id, true)),
                    menuItemText
                ));
            }
            if (!copyText_.IsEmpty())
                menu.Append(MENU_TREEPOPUP_COPYTEXT, _t("Cop&y Text to Clipboard"));
        }
    }

    menu.AppendSeparator();
    wxMenu* subGlobalOpMenuDelete = new wxMenu();
    subGlobalOpMenuDelete->Append(
        MENU_TREEPOPUP_DELETE2,
        !m_cp->isDeletedTrans() ? 
            (1 == selected) ? _tu("&Delete selected transaction…") : _tu("&Delete selected transactions…") :
            (1 == selected) ? _tu("&Permanently delete selected transaction…") : _tu("&Permanently delete selected transactions…")
    );
    if (is_nothing_selected)
        subGlobalOpMenuDelete->Enable(MENU_TREEPOPUP_DELETE2, false);
    subGlobalOpMenuDelete->AppendSeparator();
    subGlobalOpMenuDelete->Append(
        MENU_TREEPOPUP_DELETE_VIEWED,
        !m_cp->isDeletedTrans() ?
            _tu("Delete &all transactions in current view…") :
            _tu("Permanently delete &all transactions in current view…")
    );
    if (!m_cp->isDeletedTrans()) {
        subGlobalOpMenuDelete->Append(
            MENU_TREEPOPUP_DELETE_FLAGGED,
            _tu("Delete Viewed “&Follow Up” Transactions…")
        );
        subGlobalOpMenuDelete->Append(
            MENU_TREEPOPUP_DELETE_UNRECONCILED,
            _tu("Delete Viewed “&Unreconciled” Transactions…")
        );
    }
    menu.Append(MENU_TREEPOPUP_DELETE2, _t("De&lete "), subGlobalOpMenuDelete);

    if (!m_cp->isDeletedTrans()) {
        menu.AppendSeparator();

        wxMenu* subGlobalOpMenuMark = new wxMenu();
        subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARKUNRECONCILED, _t("&Unreconciled"));
        if (is_nothing_selected)
            subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKUNRECONCILED, false);
        subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARKRECONCILED, _t("&Reconciled"));
        if (is_nothing_selected)
            subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKRECONCILED, false);
        subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARKVOID, _t("&Void"));
        if (is_nothing_selected)
            subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKVOID, false);
        subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP, _t("&Follow Up"));
        if (is_nothing_selected)
            subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP, false);
        subGlobalOpMenuMark->Append(MENU_TREEPOPUP_MARKDUPLICATE, _t("D&uplicate"));
        if (is_nothing_selected)
            subGlobalOpMenuMark->Enable(MENU_TREEPOPUP_MARKDUPLICATE, false);
        menu.AppendSubMenu(subGlobalOpMenuMark, _t("Mar&k as"));

        // Disable menu items not ment for foreign transactions
        if (is_foreign) {
            menu.Enable(MENU_ON_COPY_TRANSACTION, false);
            menu.Enable(MENU_ON_PASTE_TRANSACTION, false);
            menu.Enable(MENU_ON_DUPLICATE_TRANSACTION, false);
        }
    }
    PopupMenu(&menu, event.GetPosition());
}

//----------------------------------------------------------------------------

void TransactionListCtrl::onListItemActivated(wxListEvent& /*event*/)
{
    wxLogDebug("onListItemActivated: %i selected", GetSelectedItemCount());
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_EDIT2);
    AddPendingEvent(evt);
}

void TransactionListCtrl::onListItemSelected(wxListEvent&)
{
    wxLogDebug("onListItemSelected: %i selected", GetSelectedItemCount());
    findSelectedTransactions();
    setExtraTransactionData(GetSelectedItemCount() == 1);
}

void TransactionListCtrl::onListItemDeSelected(wxListEvent&)
{
    wxLogDebug("onListItemDeSelected: %i selected", GetSelectedItemCount());
    findSelectedTransactions();
    setExtraTransactionData(GetSelectedItemCount() == 1);
}

void TransactionListCtrl::onListItemFocused(wxListEvent& WXUNUSED(event))
{
    wxLogDebug("onListItemFocused: %i selected", GetSelectedItemCount());
    findSelectedTransactions();
    setExtraTransactionData(GetSelectedItemCount() == 1);
}

void TransactionListCtrl::onListKeyDown(wxListEvent& event)
{
    if (wxGetKeyState(WXK_COMMAND) || wxGetKeyState(WXK_ALT) || wxGetKeyState(WXK_CONTROL))
        return event.Skip();

    int key = event.GetKeyCode();
    m_topItemIndex = GetTopItem() + GetCountPerPage() - 1;

    if (!m_cp->isDeletedTrans()) {
        if (key == wxKeyCode('R')) {
            // Reconciled
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARKRECONCILED);
            onMarkTransaction(evt);
        }
        else if (key == wxKeyCode('U')) {
            // Unreconciled
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARKUNRECONCILED);
            onMarkTransaction(evt);
        }
        else if (key == wxKeyCode('F')) {
            // Follow Up
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP);
            onMarkTransaction(evt);
        }
        else if (key == wxKeyCode('D')) {
            // Duplicate
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARKDUPLICATE);
            onMarkTransaction(evt);
        }
        else if (key == wxKeyCode('V')) {
            // Void
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARKVOID);
            onMarkTransaction(evt);
        }
        else if (key == WXK_DELETE || key == WXK_NUMPAD_DELETE)
        {
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_DELETE2);
            onDeleteTransaction(evt);
        }
        else {
            event.Skip();
            return;
        }
    }
    else {
        if (key == WXK_DELETE || key == WXK_NUMPAD_DELETE) {
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_DELETE2);
            onDeleteTransaction(evt);
        }
        else if (key == wxKeyCode('R')) {
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_RESTORE);
            onRestoreTransaction(evt);
        }
        else {
            event.Skip();
            return;
        }
    }
}

//----------------------------------------------------------------------------

void TransactionListCtrl::onNewTransaction(wxCommandEvent& event)
{
    int id = event.GetId();
    int type;

    switch (id) {
    case MENU_TREEPOPUP_WITHDRAWAL:
        type = Model_Checking::TYPE_ID_WITHDRAWAL;
        break;
    case MENU_TREEPOPUP_DEPOSIT:
        type = Model_Checking::TYPE_ID_DEPOSIT;
        break;
    case MENU_TREEPOPUP_TRANSFER:
        type = Model_Checking::TYPE_ID_TRANSFER;
        break;
    default:
        type = Model_Checking::TYPE_ID_WITHDRAWAL;
        break;
    }

    mmTransDialog dlg(this, m_cp->m_account_id, {0, false}, false, type);
    int i = dlg.ShowModal();
    if (i != wxID_CANCEL) {
        m_selected_id.clear();
        m_pasted_id.push_back({dlg.GetTransactionID(), 0});
        m_cp->mmPlayTransactionSound();
        refreshVisualList();

        if (i == wxID_NEW) {
            onNewTransaction(event);
        }
    }
}

void TransactionListCtrl::onDeleteTransaction(wxCommandEvent& WXUNUSED(event))
{
    // check if any transactions selected 
    int sel = GetSelectedItemCount();
    if (sel < 1) return;

    findSelectedTransactions();
    int retainDays = Model_Setting::instance().getInt("DELETED_TRANS_RETAIN_DAYS", 30);

    //ask if they want to delete
    wxString text = (m_cp->isDeletedTrans() || retainDays == 0)?
        wxString::Format(wxPLURAL("Do you want to permanently delete the selected transaction?"
        , "Do you want to permanently delete the %i selected transactions?", sel)
        , sel)
        :
        wxString::Format(wxPLURAL("Do you want to delete the selected transaction?"
            , "Do you want to delete the %i selected transactions?", sel)
            , sel);
    text += "\n\n";
    text += ((m_cp->isDeletedTrans() || retainDays == 0) ? _t("Unable to undo this action.")
        : _t("Deleted transactions will be temporarily stored and can be restored from the Deleted Transactions view."));

    wxMessageDialog msgDlg(this
        , text
        , _t("Confirm Transaction Deletion")
        , wxYES_NO | wxYES_DEFAULT | (m_cp->isDeletedTrans() ? wxICON_ERROR : wxICON_WARNING));

    if (msgDlg.ShowModal() == wxID_YES) {
        wxString deletionTime = wxDateTime::Now().ToUTC().FormatISOCombined();
        std::set<std::pair<wxString, int64>> assetStockAccts;
        Model_Checking::instance().Savepoint();
        Model_Attachment::instance().Savepoint();
        Model_Splittransaction::instance().Savepoint();
        Model_CustomFieldData::instance().Savepoint();
        for (const auto& id : m_selected_id) {
            if (id.second) continue;
            Model_Checking::Data* trx = Model_Checking::instance().get(id.first);

            if (checkTransactionLocked(trx->ACCOUNTID, trx->TRANSDATE)) {
                continue;
            }

            if (m_cp->isDeletedTrans() || retainDays == 0) {
                // remove also removes split transactions, translink entries, attachments, and custom field data
                Model_Checking::instance().remove(id.first);
            }
            else {
                trx->DELETEDTIME = deletionTime;
                Model_Checking::instance().save(trx);
                Model_Translink::Data_Set translink = Model_Translink::instance().find(Model_Translink::CHECKINGACCOUNTID(trx->TRANSID));
                if (!translink.empty()) {
                    assetStockAccts.emplace(translink.at(0).LINKTYPE, translink.at(0).LINKRECORDID);
                }
            }
            m_selectedForCopy.erase(std::remove(m_selectedForCopy.begin(), m_selectedForCopy.end(), id)
              , m_selectedForCopy.end());
        }
        m_selected_id.clear();
        Model_CustomFieldData::instance().ReleaseSavepoint();
        Model_Splittransaction::instance().ReleaseSavepoint();
        Model_Attachment::instance().ReleaseSavepoint();
        Model_Checking::instance().ReleaseSavepoint();

        if (!assetStockAccts.empty()) {
            for (const auto& i : assetStockAccts) {
                if (i.first == "Asset") Model_Translink::UpdateAssetValue(Model_Asset::instance().get(i.second));
                else if (i.first == "Stock") Model_Stock::UpdatePosition(Model_Stock::instance().get(i.second));
            }
        }
    }
    refreshVisualList();
    m_cp->m_frame->RefreshNavigationTree();
}

void TransactionListCtrl::onRestoreTransaction(wxCommandEvent& WXUNUSED(event))
{
    // check if any transactions selected
    int sel = GetSelectedItemCount();
    if (sel < 1) return;

    findSelectedTransactions();

    //ask if they want to restore
    const wxString text = wxString::Format(
        wxPLURAL(
            "Do you want to restore the selected transaction?",
            "Do you want to restore the %i selected transactions?", sel
        ),
        sel
    );

    wxMessageDialog msgDlg(
        this,
        text,
        _t("Confirm Transaction Restore"),
        wxYES_NO | wxYES_DEFAULT | wxICON_WARNING
    );

    if (msgDlg.ShowModal() == wxID_YES) {
        std::set<std::pair<wxString, int64>> assetStockAccts;
        for (const auto& id : m_selected_id) {
            if (!id.second) {
                Model_Checking::Data* trx = Model_Checking::instance().get(id.first);
                trx->DELETEDTIME.Clear();
                Model_Checking::instance().save(trx);
                Model_Translink::Data_Set translink = Model_Translink::instance().find(
                    Model_Translink::CHECKINGACCOUNTID(trx->TRANSID)
                );
                if (!translink.empty()) {
                    assetStockAccts.emplace(translink.at(0).LINKTYPE, translink.at(0).LINKRECORDID);
                }
            }
        }
        m_selected_id.clear();
        if (!assetStockAccts.empty()) {
            for (const auto& i : assetStockAccts) {
                if (i.first == "Asset")
                    Model_Translink::UpdateAssetValue(Model_Asset::instance().get(i.second));
                else if (i.first == "Stock")
                    Model_Stock::UpdatePosition(Model_Stock::instance().get(i.second));
            }
        }
    }

    refreshVisualList();
    m_cp->m_frame->RefreshNavigationTree();
}

void TransactionListCtrl::onRestoreViewedTransaction(wxCommandEvent&)
{
    wxMessageDialog msgDlg(
        this,
        _t("Do you want to restore all of the transactions shown?"),
        _t("Confirm Transaction Restore"),
        wxYES_NO | wxNO_DEFAULT | wxICON_ERROR
    );
    if (msgDlg.ShowModal() == wxID_YES) {
        std::set<std::pair<wxString, int64>> assetStockAccts;
        for (const auto& tran : this->m_trans) {
            if (tran.m_repeat_num) continue;
            Model_Checking::Data* trx = Model_Checking::instance().get(tran.TRANSID);
            trx->DELETEDTIME.Clear();
            Model_Checking::instance().save(trx);
            Model_Translink::Data_Set translink = Model_Translink::instance().find(
                Model_Translink::CHECKINGACCOUNTID(trx->TRANSID)
            );
            if (!translink.empty()) {
                assetStockAccts.emplace(translink.at(0).LINKTYPE, translink.at(0).LINKRECORDID);
            }
        }
        if (!assetStockAccts.empty()) {
            for (const auto& i : assetStockAccts) {
                if (i.first == "Asset")
                    Model_Translink::UpdateAssetValue(Model_Asset::instance().get(i.second));
                else if (i.first == "Stock")
                    Model_Stock::UpdatePosition(Model_Stock::instance().get(i.second));
            }
        }
    }

    refreshVisualList();
    m_cp->m_frame->RefreshNavigationTree();
}

void TransactionListCtrl::onEditTransaction(wxCommandEvent& /*event*/)
{
    // check if anything to edit
    if (GetSelectedItemCount() < 1) return;
    findSelectedTransactions();

    // edit multiple transactions
    if (m_selected_id.size() > 1) {
        std::vector<int64> transid;
        for (const auto& id : m_selected_id)
            if (!id.second)
                transid.push_back(id.first);
        if (transid.size() == 0) return;
        if (!checkForClosedAccounts()) return;
        transactionsUpdateDialog dlg(this, transid);
        if (dlg.ShowModal() == wxID_OK)
            refreshVisualList();
        return;
    }

    // edit single transaction
    Fused_Transaction::IdRepeat id = m_selected_id[0];
    if (!id.second) {
        Model_Checking::Data* checking_entry = Model_Checking::instance().get(id.first);
        if (checkTransactionLocked(checking_entry->ACCOUNTID, checking_entry->TRANSDATE))
            return;

        if (!Model_Translink::instance().find(Model_Translink::CHECKINGACCOUNTID(id.first)).empty()) {
            Model_Translink::Data translink = Model_Translink::TranslinkRecord(id.first);
            if (translink.LINKTYPE == Model_Stock::refTypeName) {
                ShareTransactionDialog dlg(this, &translink, checking_entry);
                if (dlg.ShowModal() == wxID_OK)
                    refreshVisualList();
            }
            else if (translink.LINKTYPE == Model_Asset::refTypeName) {
                mmAssetDialog dlg(this, &translink, checking_entry);
                if (dlg.ShowModal() == wxID_OK)
                    refreshVisualList();
            }
            else {
                wxASSERT(false);
            }
        }
        else {
            mmTransDialog dlg(this, m_cp->m_account_id, {id.first, false});
            if (dlg.ShowModal() != wxID_CANCEL)
                refreshVisualList();
        }
    }
    else {
        mmBDDialog dlg(this, id.first, false, false);
        if ( dlg.ShowModal() == wxID_OK )
            refreshVisualList();
    }
    m_topItemIndex = GetTopItem() + GetCountPerPage() - 1;
}

void TransactionListCtrl::onMoveTransaction(wxCommandEvent& /*event*/)
{
    findSelectedTransactions();
    int sel = GetSelectedItemCount();

    //ask if they want to move
    const wxString text = wxString::Format(
        wxPLURAL("Do you want to move the selected transaction?"
            , "Do you want to move the %i selected transactions?", sel)
        , sel);
    wxMessageDialog msgDlg(this
        , text
        , _t("Confirm Transaction Move")
        , wxYES_NO | wxYES_DEFAULT | wxICON_ERROR);

    if (msgDlg.ShowModal() == wxID_YES) {
        const wxString headerMsg = (1 == sel) ? _tu("Moving transaction to…") : 
                                    wxString::Format(_tu("Moving %i transactions to…"), sel);

        mmSingleChoiceDialog scd(this
            , _t("Select the destination Account ")
            , headerMsg
            , Model_Account::instance().all_checking_account_names());
        if (scd.ShowModal() == wxID_OK) {
            int64 dest_account_id = -1;
            wxString dest_account_name = scd.GetStringSelection();
            Model_Account::Data* dest_account = Model_Account::instance().get(dest_account_name);
            if (dest_account)
                dest_account_id = dest_account->ACCOUNTID;
            else
                return;
            std::vector<int64> skip_trx;
            Model_Checking::instance().Savepoint();
            for (const auto& id : m_selected_id) {
                if (!id.second) {
                    Model_Checking::Data* trx = Model_Checking::instance().get(id.first);
                    if (checkTransactionLocked(trx->ACCOUNTID, trx->TRANSDATE)
                            || Model_Checking::foreignTransaction(*trx)
                            || Model_Checking::type_id(trx->TRANSCODE) == Model_Checking::TYPE_ID_TRANSFER
                            || trx->TRANSDATE < dest_account->INITIALDATE
                    ) {
                        skip_trx.push_back(trx->TRANSID);
                    } else {
                        trx->ACCOUNTID = dest_account_id;
                        Model_Checking::instance().save(trx);
                    }
                }
            }
            Model_Checking::instance().ReleaseSavepoint();
            if (!skip_trx.empty()) {
                const wxString detail = wxString::Format("%s\n%s: %zu\n%s: %zu"
                                , _t("This is due to some elements of the transaction or account detail not allowing the move")
                                , _t("Moved"), m_selected_id.size() - skip_trx.size()
                                , _t("Not moved"), skip_trx.size());
                mmErrorDialogs::MessageWarning(this
                    , detail
                    , _t("Unable to move some transactions."));
            }
            //TODO: enable report to detail transactions that are unable to be moved
            refreshVisualList();
        }
    }
}

void TransactionListCtrl::onViewOtherAccount(wxCommandEvent& /*event*/)
{
    // we can only get here for a single transfer transaction
    findSelectedTransactions();
    Fused_Transaction::IdRepeat id = m_selected_id[0];

    Fused_Transaction::Full_Data tran = !id.second ?
        Fused_Transaction::Full_Data(*Model_Checking::instance().get(id.first)) :
        Fused_Transaction::Full_Data(*Model_Billsdeposits::instance().get(id.first));

    int64 gotoAccountID = (m_cp->m_account_id == tran.ACCOUNTID) ? tran.TOACCOUNTID : tran.ACCOUNTID;
    wxString gotoAccountName = (m_cp->m_account_id == tran.ACCOUNTID) ? tran.TOACCOUNTNAME : tran.ACCOUNTNAME;

    m_cp->m_frame->setNavTreeAccount(gotoAccountName);
    m_cp->m_frame->setGotoAccountID(gotoAccountID, id);
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
    m_cp->m_frame->GetEventHandler()->AddPendingEvent(event);
}

void TransactionListCtrl::onViewSplitTransaction(wxCommandEvent& /*event*/)
{
    // we can only view a single transaction
    if (GetSelectedItemCount() != 1) return;
    findSelectedTransactions();
    Fused_Transaction::IdRepeat id = m_selected_id[0];

    m_cp->displaySplitCategories({id.first, id.second != 0});
}

void TransactionListCtrl::onOrganizeAttachments(wxCommandEvent& /*event*/)
{
    // we only support a single transaction
    if (GetSelectedItemCount() != 1) return;
    findSelectedTransactions();
    Fused_Transaction::IdRepeat id = m_selected_id[0];

    const wxString refType = !id.second ?
        Model_Checking::refTypeName :
        Model_Billsdeposits::refTypeName;
    mmAttachmentDialog dlg(this, refType, id.first);
    dlg.ShowModal();
    refreshVisualList();
}

void TransactionListCtrl::onCreateReoccurance(wxCommandEvent& /*event*/)
{
     // we only support a single transaction
    if (GetSelectedItemCount() != 1) return;
    findSelectedTransactions();
    Fused_Transaction::IdRepeat id = m_selected_id[0];

    if (!id.second) {
        mmBDDialog dlg(this, 0, false, false);
        dlg.SetDialogParameters(id.first);
        if (dlg.ShowModal() == wxID_OK)
            wxMessageBox(_t("Scheduled transaction saved."));
    }
}

void TransactionListCtrl::onFind(wxCommandEvent&)
{
    if (rightClickFilter_.IsEmpty())
        return;
    // save the filter as the "Advanced" filter for All Transactions
    Model_Infotable::instance().setString("CHECK_FILTER_ID_ADV_-1", rightClickFilter_);
    // set All Transactions to use the "Advanced" filter
    Model_Infotable::instance().setString(
        "CHECK_FILTER_-1",
        "{ \"FILTER\": \"" + mmCheckingPanel::FILTER_NAME_ADVANCED + "\" }"
    );
    // Navigate to the All Transactions panel
    m_cp->m_frame->setNavTreeSection(wxTRANSLATE("All Transactions"));
    wxTreeItemId currentId = m_cp->m_frame->GetNavTreeSelection();
    wxTreeItemId allTransactionsId = m_cp->m_frame->GetNavTreeSelection();
    if (currentId.IsOk() && currentId == allTransactionsId) {
        m_cp->m_trans_filter_dlg.reset(
            new mmFilterTransactionsDialog(this, -1, false, rightClickFilter_)
        );
        m_cp->setFilterAdvanced();
        refreshVisualList();
    }
    else
        m_cp->m_frame->SetNavTreeSelection(m_cp->m_frame->GetNavTreeSelection());
}

void TransactionListCtrl::onCopyText(wxCommandEvent&)
{
    if (!copyText_.IsEmpty()) {
        if (wxTheClipboard->Open()) {
            wxTheClipboard->SetData(new wxTextDataObject(copyText_));
            wxTheClipboard->Close();
        }
    }
}

void TransactionListCtrl::onMarkTransaction(wxCommandEvent& event)
{
    findSelectedTransactions();
    int evt = event.GetId();
    //bool bRefreshRequired = false;
    wxString status = "";
    switch (evt) {
    case MENU_TREEPOPUP_MARKRECONCILED:         status = Model_Checking::STATUS_KEY_RECONCILED; break;
    case MENU_TREEPOPUP_MARKUNRECONCILED:       status = Model_Checking::STATUS_KEY_NONE; break;
    case MENU_TREEPOPUP_MARKVOID:               status = Model_Checking::STATUS_KEY_VOID; break;
    case MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP: status = Model_Checking::STATUS_KEY_FOLLOWUP; break;
    case MENU_TREEPOPUP_MARKDUPLICATE:          status = Model_Checking::STATUS_KEY_DUPLICATE; break;
    default: wxASSERT(false);
    }

    Model_Checking::instance().Savepoint();

    for (int row = 0; row < GetItemCount(); row++) {
        if (GetItemState(row, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED) {
            Model_Account::Data* account = Model_Account::instance().get(m_trans[row].ACCOUNTID);
            const auto statement_date = Model_Account::DateOf(account->STATEMENTDATE).FormatISODate();
            wxString strDate = Model_Checking::TRANSDATE(m_trans[row]).FormatISODate();
            if (!Model_Account::BoolOf(account->STATEMENTLOCKED)
                || strDate > statement_date
            ) {
                //bRefreshRequired |= (status == Model_Checking::STATUS_KEY_VOID) || (m_trans[row].STATUS == Model_Checking::STATUS_KEY_VOID);
                if (!m_trans[row].m_repeat_num) {
                    m_trans[row].STATUS = status;
                    Model_Checking::instance().save(&m_trans[row]);
                }
            }
        }
    }

    Model_Checking::instance().ReleaseSavepoint();

    refreshVisualList();
}

void TransactionListCtrl::onDeleteViewedTransaction(wxCommandEvent& event)
{
    auto i = event.GetId();
    int retainDays = Model_Setting::instance().getInt("DELETED_TRANS_RETAIN_DAYS", 30);

    if (i == MENU_TREEPOPUP_DELETE_VIEWED) {
        wxString text = !(m_cp->isDeletedTrans() || retainDays == 0)
            ? _t("Do you want to delete all the transactions shown?")
            : _t("Do you want to permanently delete all the transactions shown?");

        text += "\n\n";
        text += !(m_cp->isDeletedTrans() || retainDays == 0)
            ? _t("Deleted transactions will be temporarily stored and can be restored from the Deleted Transactions view.")
            : _t("Unable to undo this action.");

        wxMessageDialog msgDlg(this
            , text
            , _t("Confirm Transaction Deletion")
            , wxYES_NO | wxNO_DEFAULT | (m_cp->isDeletedTrans() ? wxICON_ERROR : wxICON_WARNING));
        if (msgDlg.ShowModal() == wxID_YES) {
            deleteTransactionsByStatus("");
        }
    }
    else if (i == MENU_TREEPOPUP_DELETE_FLAGGED) {
        wxMessageDialog msgDlg(this
            , wxString::Format(_tu("Do you want to delete all the “%s” transactions shown?"), _t("Follow Up"))
            , _t("Confirm Transaction Deletion")
            , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
        if (msgDlg.ShowModal() == wxID_YES) {
            deleteTransactionsByStatus(Model_Checking::STATUS_NAME_FOLLOWUP);
        }
    }
    else if (i == MENU_TREEPOPUP_DELETE_UNRECONCILED) {
        wxMessageDialog msgDlg(this
            , wxString::Format(_tu("Do you want to delete all the “%s” transactions shown?"), _t("Unreconciled"))
            , _t("Confirm Transaction Deletion")
            , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
        if (msgDlg.ShowModal() == wxID_YES) {
            deleteTransactionsByStatus(Model_Checking::STATUS_NAME_NONE);
        }
    }
    refreshVisualList();
    m_cp->m_frame->RefreshNavigationTree();
}

//----------------------------------------------------------------------------

void TransactionListCtrl::onSelectAll(wxCommandEvent& WXUNUSED(event))
{
    m_selected_id.clear();
    SetEvtHandlerEnabled(false);
    std::set<Fused_Transaction::IdRepeat> unique_ids;
    for (int row = 0; row < GetItemCount(); row++) {
        SetItemState(row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        const auto& tran = m_trans[row];
        Fused_Transaction::IdRepeat id = { !tran.m_repeat_num ? tran.TRANSID : tran.m_bdid, tran.m_repeat_num };
        if (unique_ids.find(id) == unique_ids.end()) {
            m_selected_id.push_back(id);
            unique_ids.insert(id);
        }
    }
    SetEvtHandlerEnabled(true);
    setExtraTransactionData(GetSelectedItemCount() == 1);
}

void TransactionListCtrl::onCopy(wxCommandEvent& WXUNUSED(event))
{
    // we can't copy deleted items or there is nothing to copy
    if (m_cp->isDeletedTrans() || GetSelectedItemCount() < 1) return;

    // collect the selected transactions for copy
    findSelectedTransactions();
    m_selectedForCopy = m_selected_id;

    if (wxTheClipboard->Open()) {
        const wxString seperator = "\t";
        wxString data = "";
        for (int row = 0; row < GetItemCount(); row++) {
            if (GetItemState(row, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED) {
                for (int col_vo = 0; col_vo < getColNrSize(); ++col_vo) {
                    int col_nr = getColNr_Vo(col_vo);
                    if (!isHiddenColNr(col_nr)) {
                        data += inQuotes(OnGetItemText(row, col_nr), seperator);
                        data += seperator;
                    }
                }
                data += "\n";
            }
        }
        wxTheClipboard->SetData(new wxTextDataObject(data));
        wxTheClipboard->Close();
    }
}

void TransactionListCtrl::onPaste(wxCommandEvent& WXUNUSED(event))
{
    // we can't paste with multiple accounts open, deleted items, or if there is nothing to paste
    if (!m_cp->isAccount() || m_selectedForCopy.size() < 1)
        return;

    findSelectedTransactions();
    Model_Checking::instance().Savepoint();
    m_pasted_id.clear();    // make sure the list is empty before we paste
    for (const auto& id : m_selectedForCopy) {
        if (!id.second) {
            Model_Checking::Data* tran = Model_Checking::instance().get(id.first);
            if (Model_Checking::foreignTransaction(*tran)) continue;
            onPaste(tran);
        }
    }
    Model_Checking::instance().ReleaseSavepoint();
    refreshVisualList();
}

int64 TransactionListCtrl::onPaste(Model_Checking::Data* tran)
{
    wxASSERT(m_cp->isAccount());

    bool useOriginalDate = Model_Setting::instance().getBool(INIDB_USE_ORG_DATE_COPYPASTE, false);
    bool useOriginalState = Model_Setting::instance().getBool(INIDB_USE_ORG_STATE_DUPLICATE_PASTE, false);

    //TODO: the clone function can't clone split transactions, or custom data
    Model_Checking::Data* copy = Model_Checking::instance().clone(tran);
    if (!useOriginalDate) copy->TRANSDATE = wxDateTime::Now().FormatISOCombined();
    if (!useOriginalState) {
        copy->STATUS = Model_Checking::status_key(Option::instance().getTransStatusReconciled()); // Use default status on copy insert
    }
    if (Model_Checking::type_id(copy->TRANSCODE) != Model_Checking::TYPE_ID_TRANSFER ||
        (m_cp->m_account_id != copy->ACCOUNTID && m_cp->m_account_id != copy->TOACCOUNTID)
    )
    copy->ACCOUNTID = m_cp->m_account_id;
    int64 transactionID = Model_Checking::instance().save(copy);
    m_pasted_id.push_back({transactionID, 0});   // add the newly pasted transaction

    // Clone transaction tags
    Model_Taglink::Cache copy_taglinks;
    wxString reftype = Model_Checking::refTypeName;
    for (const auto& link : Model_Taglink::instance().find(Model_Taglink::REFTYPE(reftype), Model_Taglink::REFID(tran->TRANSID))) {
        Model_Taglink::Data* taglink = Model_Taglink::instance().clone(&link);
        taglink->REFID = transactionID;
        copy_taglinks.push_back(taglink);
    }

    // Clone split transactions
    reftype = Model_Splittransaction::refTypeName;
    for (const auto& split_item : Model_Checking::split(tran)) {
        Model_Splittransaction::Data *copy_split_item = Model_Splittransaction::instance().clone(&split_item);
        copy_split_item->TRANSID = transactionID;
        int64 splittransID = Model_Splittransaction::instance().save(copy_split_item);

        // Clone split tags
        for (const auto& link : Model_Taglink::instance().find(
            Model_Taglink::REFTYPE(reftype),
            Model_Taglink::REFID(split_item.SPLITTRANSID)
        )) {
            Model_Taglink::Data* taglink = Model_Taglink::instance().clone(&link);
            taglink->REFID = splittransID;
            copy_taglinks.push_back(taglink);
        }
    }
    Model_Taglink::instance().save(copy_taglinks);

    // Clone duplicate custom fields
    const auto& data_set = Model_CustomFieldData::instance().find(
        Model_CustomFieldData::REFID(tran->TRANSID)
    );
    if (data_set.size() > 0) {
        Model_CustomFieldData::instance().Savepoint();
        for (const auto& item : data_set) {
            Model_CustomFieldData::Data* fieldData = Model_CustomFieldData::instance().create();
            fieldData->FIELDID = item.FIELDID;
            fieldData->REFID = transactionID;
            fieldData->CONTENT = item.CONTENT;
            Model_CustomFieldData::instance().save(fieldData);
        }
        Model_CustomFieldData::instance().ReleaseSavepoint();
    }

    // Clone attachments if wanted
    if (Model_Infotable::instance().getBool("ATTACHMENTSDUPLICATE", false)) {
        const wxString& RefType = Model_Checking::refTypeName;
        mmAttachmentManage::CloneAllAttachments(RefType, tran->TRANSID, transactionID);
    }

    return transactionID;
}

void TransactionListCtrl::onDuplicateTransaction(wxCommandEvent& WXUNUSED(event))
{
    // we can only duplicate a single transaction
    if (GetSelectedItemCount() != 1) return;
    findSelectedTransactions();
    Fused_Transaction::IdRepeat id = m_selected_id[0];

    mmTransDialog dlg(this, m_cp->m_account_id, {id.first, id.second != 0}, true);
    if (dlg.ShowModal() != wxID_CANCEL) {
        m_selected_id.clear();
        m_pasted_id.push_back({dlg.GetTransactionID(), 0});
        m_cp->mmPlayTransactionSound();
        refreshVisualList();
    }
    m_topItemIndex = GetTopItem() + GetCountPerPage() - 1;
}

void TransactionListCtrl::onEnterScheduled(wxCommandEvent& WXUNUSED(event))
{
    if (GetSelectedItemCount() != 1) return;
    findSelectedTransactions();
    Fused_Transaction::IdRepeat id = m_selected_id[0];

    if (id.second == 1) {
        mmBDDialog dlg(this, id.first, false, true);
        if ( dlg.ShowModal() == wxID_OK ) {
            refreshVisualList();
        }
    }
}

void TransactionListCtrl::onSkipScheduled(wxCommandEvent& WXUNUSED(event))
{
    if (GetSelectedItemCount() != 1) return;
    findSelectedTransactions();
    Fused_Transaction::IdRepeat id = m_selected_id[0];

    if (id.second == 1) {
        Model_Billsdeposits::instance().completeBDInSeries(id.first);
        refreshVisualList();
    }
}

void TransactionListCtrl::onSetUserColour(wxCommandEvent& event)
{
    if (m_cp->isDeletedTrans())
        return;

    findSelectedTransactions();
    int user_color_id = event.GetId();
    user_color_id -= MENU_ON_SET_UDC0;
    wxLogDebug("id: %i", user_color_id);

    Model_Checking::instance().Savepoint();
    Model_Billsdeposits::instance().Savepoint();
    for (const auto& id : m_selected_id) {
        if (!id.second) {
            Model_Checking::Data* tran = Model_Checking::instance().get(id.first);
            if (tran) {
                tran->COLOR = user_color_id;
                Model_Checking::instance().save(tran);
            }
        }
        else {
            Model_Billsdeposits::Data* bill = Model_Billsdeposits::instance().get(id.first);
            if (bill) {
                bill->COLOR = user_color_id;
                Model_Billsdeposits::instance().save(bill);
            }
        }
    }
    Model_Billsdeposits::instance().ReleaseSavepoint();
    Model_Checking::instance().ReleaseSavepoint();
    m_topItemIndex = GetTopItem() + GetCountPerPage() - 1;

    refreshVisualList();
}

void TransactionListCtrl::onOpenAttachment(wxCommandEvent& WXUNUSED(event))
{
    // we can only open a single transaction
    if (GetSelectedItemCount() != 1) return;
    findSelectedTransactions();
    Fused_Transaction::IdRepeat id = m_selected_id[0];

    const wxString refType = !id.second ?
        Model_Checking::refTypeName :
        Model_Billsdeposits::refTypeName;
    mmAttachmentManage::OpenAttachmentFromPanelIcon(this, refType, id.first);
    refreshVisualList();
}

//----------------------------------------------------------------------------

wxString UDFCFormatHelper(Model_CustomField::TYPE_ID type, wxString data)
{
    wxString formattedData = data;
    bool v = false;
    if (!data.empty()) {
        switch (type) {
        case Model_CustomField::TYPE_ID_DATE:
            formattedData = mmGetDateTimeForDisplay(data);
            break;
        case Model_CustomField::TYPE_ID_BOOLEAN:
            v = wxString("TRUE|true|1").Contains(data);
            formattedData = (v) ? L"\u2713" : L"\u2717";
            break;
        default:
            break;
        }
    }
    return formattedData;
}

const wxString TransactionListCtrl::getItem(long item, int col_id) const
{
    if (item < 0 || item >= static_cast<int>(m_trans.size()))
        return "";
    // TODO: add isHiddenColId(col_id)
    if (isDisabledColId(col_id))
        return "";
    const Fused_Transaction::Full_Data& fused = m_trans.at(item);

    wxString value = wxEmptyString;
    wxDateTime datetime;
    wxString dateFormat = Option::instance().getDateFormat();
    switch (col_id) {
    case LIST_ID_SN:
        return fused.displaySN;
    case LIST_ID_ID:
        return fused.displayID;
    case LIST_ID_ACCOUNT:
        return fused.ACCOUNTNAME;
    case LIST_ID_DATE:
        return mmGetDateForDisplay(fused.TRANSDATE);
    case LIST_ID_TIME:
        return mmGetTimeForDisplay(fused.TRANSDATE);
    case LIST_ID_NUMBER:
        return fused.TRANSACTIONNUMBER;
    case LIST_ID_CATEGORY:
        return fused.CATEGNAME;
    case LIST_ID_PAYEE_STR:
        return fused.is_foreign_transfer() ?
            (Model_Checking::type_id(fused.TRANSCODE) == Model_Checking::TYPE_ID_DEPOSIT ? "< " : "> ") + fused.PAYEENAME :
            fused.PAYEENAME;
    case LIST_ID_STATUS:
        return fused.is_foreign() ? "< " + fused.STATUS : fused.STATUS;
    case LIST_ID_NOTES: {
        value = fused.NOTES;
        if (!fused.displayID.Contains(".")) {
            for (const auto& split : fused.m_splits)
                value += wxString::Format(" %s", split.NOTES);
        }
        value.Replace("\n", " ");
        if (fused.has_attachment())
            value.Prepend(mmAttachmentManage::GetAttachmentNoteSign());
        return value.Trim(false);
    }
    case LIST_ID_TAGS:
        value = fused.TAGNAMES;
        if (!fused.displayID.Contains(".")) {
            const wxString splitRefType = Model_Splittransaction::refTypeName;
            for (const auto& split : fused.m_splits) {
                wxString tagnames;
                std::map<wxString, int64> tags = Model_Taglink::instance().get(splitRefType, split.SPLITTRANSID);
                std::map<wxString, int64, caseInsensitiveComparator> sortedTags(tags.begin(), tags.end());
                for (const auto& tag : sortedTags)
                    tagnames.Append(tag.first + " ");
                if (!tagnames.IsEmpty())
                    value.Append((value.IsEmpty() ? "" : ", ") + tagnames.Trim());
            }
        }
        return value.Trim();
    case LIST_ID_DELETEDTIME:
        datetime.ParseISOCombined(fused.DELETEDTIME);
        if(!datetime.IsValid())
            return wxString("");
        return mmGetDateTimeForDisplay(datetime.FromUTC().FormatISOCombined(), dateFormat + " %H:%M:%S");
    case LIST_ID_UDFC01:
        return UDFCFormatHelper(fused.UDFC_type[0], fused.UDFC_content[0]);
    case LIST_ID_UDFC02:
        return UDFCFormatHelper(fused.UDFC_type[1], fused.UDFC_content[1]);
    case LIST_ID_UDFC03:
        return UDFCFormatHelper(fused.UDFC_type[2], fused.UDFC_content[2]);
    case LIST_ID_UDFC04:
        return UDFCFormatHelper(fused.UDFC_type[3], fused.UDFC_content[3]);
    case LIST_ID_UDFC05:
        return UDFCFormatHelper(fused.UDFC_type[4], fused.UDFC_content[4]);
    case LIST_ID_UPDATEDTIME:
        datetime.ParseISOCombined(fused.LASTUPDATEDTIME);
        if (!datetime.IsValid())
            return wxString("");
        return mmGetDateTimeForDisplay(datetime.FromUTC().FormatISOCombined(), dateFormat + " %H:%M:%S");
    }

    switch (col_id) {
    case LIST_ID_WITHDRAWAL:
        if (!m_cp->isAccount()) {
            Model_Account::Data* account = Model_Account::instance().get(fused.ACCOUNTID_W);
            Model_Currency::Data* currency = account ?
                Model_Currency::instance().get(account->CURRENCYID) : nullptr;
            if (currency)
                value = Model_Currency::toCurrency(fused.TRANSAMOUNT_W, currency);
        }
        else if (fused.ACCOUNTID_W == m_cp->m_account_id) {
            value = Model_Currency::toString(fused.TRANSAMOUNT_W, m_cp->m_currency);
        }
        if (!value.IsEmpty() && Model_Checking::status_id(fused.STATUS) == Model_Checking::STATUS_ID_VOID)
            value = "* " + value;
        return value;
    case LIST_ID_DEPOSIT:
        if (!m_cp->isAccount()) {
            Model_Account::Data* account = Model_Account::instance().get(fused.ACCOUNTID_D);
            Model_Currency::Data* currency = account ?
                Model_Currency::instance().get(account->CURRENCYID) : nullptr;
            if (currency)
                value = Model_Currency::toCurrency(fused.TRANSAMOUNT_D, currency);
        }
        else if (fused.ACCOUNTID_D == m_cp->m_account_id) {
            value = Model_Currency::toString(fused.TRANSAMOUNT_D, m_cp->m_currency);
        }
        if (!value.IsEmpty() && Model_Checking::status_id(fused.STATUS) == Model_Checking::STATUS_ID_VOID)
            value = "* " + value;
        return value;
    case LIST_ID_BALANCE:
        return Model_Currency::toString(fused.ACCOUNT_BALANCE, m_cp->m_currency);
    case LIST_ID_CREDIT:
        return Model_Currency::toString(
            m_cp->m_account->CREDITLIMIT + fused.ACCOUNT_BALANCE,
            m_cp->m_currency
        );
    }

    return value;
}

void TransactionListCtrl::setExtraTransactionData(const bool single)
{
    int repeat_num = 0;
    bool isForeign = false;
    if (single) {
        Fused_Transaction::IdRepeat id = m_selected_id[0];
        Fused_Transaction::Data tran = !id.second ?
            Fused_Transaction::Data(*Model_Checking::instance().get(id.first)) :
            Fused_Transaction::Data(*Model_Billsdeposits::instance().get(id.first));
        if (Model_Checking::foreignTransaction(tran))
            isForeign = true;
        repeat_num = id.second;
    }
    m_cp->updateExtraTransactionData(single, repeat_num, isForeign);
}

void TransactionListCtrl::markItem(long selectedItem)
{
    //First of all any items should be unselected
    long cursel = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (cursel != wxNOT_FOUND)
        SetItemState(cursel, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);

    //Then finded item will be selected
    SetItemState(selectedItem, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    EnsureVisible(selectedItem);
    return;
}

void TransactionListCtrl::setSelectedId(Fused_Transaction::IdRepeat sel_id)
{
    int i = 0;
    for (const auto& fused : m_trans) {
        if (fused.m_repeat_num == sel_id.second && fused.TRANSID == sel_id.first) {
            SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            SetItemState(i, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
            m_topItemIndex = i;
            break;
        }
        i++;
    }
}

void TransactionListCtrl::findSelectedTransactions()
{
    // find the selected transactions
    long x = 0;
    m_selected_id.clear();
    std::set<Fused_Transaction::IdRepeat> unique_ids;
    for (const auto& tran : m_trans) {
        if (GetItemState(x++, wxLIST_STATE_SELECTED) != wxLIST_STATE_SELECTED)
            continue;
        int64 id = !tran.m_repeat_num ? tran.TRANSID : tran.m_bdid;
        if (unique_ids.find({id, tran.m_repeat_num}) == unique_ids.end()) {
            m_selected_id.push_back({id, tran.m_repeat_num});
            unique_ids.insert({id, tran.m_repeat_num});
        }
    }
}

int TransactionListCtrl::getColNr_X(int xPos)
{
    int col_vo = 0;
    int x = -GetScrollPos(wxHORIZONTAL);
    for (col_vo = 0; col_vo < GetColumnCount(); col_vo++) {
        x += GetColumnWidth(getColNr_Vo(col_vo));
        if (x >= xPos) break;
    }
    if (col_vo >= GetColumnCount())
        return -1;
    return getColNr_Vo(col_vo);
}

void TransactionListCtrl::doSearchText(const wxString& value)
{
    const wxString pattern = value.Lower().Append("*");

    long last = static_cast<long>(GetItemCount() - 1);
    if (m_selected_id.size() > 1) {
        SetEvtHandlerEnabled(false);
        for (long i = 0; i < last; i++) {
            long cursel = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (cursel != wxNOT_FOUND)
                SetItemState(cursel, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
        }
        SetEvtHandlerEnabled(true);
    }

    long selectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

    if (selectedItem < 0 || selectedItem > last) //nothing selected
        selectedItem = getSortAsc(0) ? last + 1  : -1;

    while (true) {
        getSortAsc(0) ? selectedItem-- : selectedItem++;
        if (selectedItem < 0 || selectedItem >= static_cast<long>(m_trans.size()))
            break;

        wxString test1 = Model_Currency::fromString2CLocale(value);
        double v;
        if (test1.ToCDouble(&v)) {
            try {
                double amount = m_trans.at(selectedItem).TRANSAMOUNT;
                double to_trans_amount = m_trans.at(selectedItem).TOTRANSAMOUNT;
                if (v == amount || v == to_trans_amount) {
                    return markItem(selectedItem);
                }
            }
            catch (std::exception & ex) {
                wxLogDebug("%s | row invalid %ld", ex.what(), selectedItem);
            }

        }

        for (const auto& t : {
            LIST_ID_NOTES, LIST_ID_NUMBER, LIST_ID_PAYEE_STR, LIST_ID_CATEGORY,
            LIST_ID_DATE, LIST_ID_TAGS, LIST_ID_DELETEDTIME, LIST_ID_UDFC01,
            LIST_ID_UDFC02, LIST_ID_UDFC03, LIST_ID_UDFC04, LIST_ID_UDFC05
        }) {
            const auto test = getItem(selectedItem, t).Lower();
            if (test.empty())
                continue;
            if (test.Matches(pattern)) {
                return markItem(selectedItem);
            }
        }

        for (const auto& entry : m_trans.at(selectedItem).ATTACHMENT_DESCRIPTION) {
            wxString test = entry.Lower();
            if (test.Matches(pattern)) {
                return markItem(selectedItem);
            }
        }

    }

    wxLogDebug("Searching finished");
    selectedItem = getSortAsc(0) ? last : 0;
    long cursel = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    SetItemState(cursel, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    EnsureVisible(selectedItem);
}

void TransactionListCtrl::markSelectedTransaction()
{
    long i = 0;
    for (const auto & tran : m_trans) {
        Fused_Transaction::IdRepeat id = { !tran.m_repeat_num ? tran.TRANSID : tran.m_bdid,
            tran.m_repeat_num };
        //reset any selected items in the list
        if (GetItemState(i, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
            SetItemState(i, 0, wxLIST_STATE_SELECTED);
        if (!m_selected_id.empty()) {
            // discover where the transaction has ended up in the list
            if (getSortAsc(0)) {
                if (m_topItemIndex < i && id == m_selected_id.back())
                    m_topItemIndex = i;
            } else {
                if (m_topItemIndex > i && id == m_selected_id.back())
                    m_topItemIndex = i;
            }
        }
        ++i;
    }

    if (m_trans.empty()) return;

    if (m_selected_id.empty()) {
        i = static_cast<long>(m_trans.size()) - 1;
        if (!getSortAsc(0))
            i = 0;
        EnsureVisible(i);
    }
}

void TransactionListCtrl::deleteTransactionsByStatus(const wxString& status)
{
    int retainDays = Model_Setting::instance().getInt("DELETED_TRANS_RETAIN_DAYS", 30);
    wxString deletionTime = wxDateTime::Now().ToUTC().FormatISOCombined();
    std::set<std::pair<wxString, int64>> assetStockAccts;
    const auto s = Model_Checking::status_key(status);
    Model_Checking::instance().Savepoint();
    Model_Attachment::instance().Savepoint();
    Model_Splittransaction::instance().Savepoint();
    Model_CustomFieldData::instance().Savepoint();
    for (const auto& tran : this->m_trans) {
        if (tran.m_repeat_num) continue;
        if (tran.STATUS == s || (s.empty() && status.empty())) {
            if (m_cp->isDeletedTrans() || retainDays == 0) {
                // remove also removes any split transactions, translink entries, attachments, and custom field data
                Model_Checking::instance().remove(tran.TRANSID);
            }
            else {
                Model_Checking::Data* trx = Model_Checking::instance().get(tran.TRANSID);
                trx->DELETEDTIME = deletionTime;
                Model_Checking::instance().save(trx);
                Model_Translink::Data_Set translink = Model_Translink::instance().find(Model_Translink::CHECKINGACCOUNTID(trx->TRANSID));
                if (!translink.empty()) {
                    assetStockAccts.emplace(translink.at(0).LINKTYPE, translink.at(0).LINKRECORDID);
                }
            }
        }
    }

    if (!assetStockAccts.empty()) {
        for (const auto& i : assetStockAccts) {
            if (i.first == "Asset") Model_Translink::UpdateAssetValue(Model_Asset::instance().get(i.second));
            else if (i.first == "Stock") Model_Stock::UpdatePosition(Model_Stock::instance().get(i.second));
        }
    }

    Model_Splittransaction::instance().ReleaseSavepoint();
    Model_Attachment::instance().ReleaseSavepoint();
    Model_Checking::instance().ReleaseSavepoint();
    Model_CustomFieldData::instance().ReleaseSavepoint();
}

bool TransactionListCtrl::checkForClosedAccounts()
{
    int closedTrx = 0;
    for (const auto& id : m_selected_id) {
        Fused_Transaction::Data tran = !id.second ?
            Fused_Transaction::Data(*Model_Checking::instance().get(id.first)) :
            Fused_Transaction::Data(*Model_Billsdeposits::instance().get(id.first));
        Model_Account::Data* account = Model_Account::instance().get(tran.ACCOUNTID);
        if (account && Model_Account::STATUS_ID_CLOSED == Model_Account::status_id(account)) {
            closedTrx++;
            continue;
        }
        Model_Account::Data* to_account = Model_Account::instance().get(tran.TOACCOUNTID);
        if (to_account && Model_Account::STATUS_ID_CLOSED == Model_Account::status_id(account))
            closedTrx++;
    }

    if (!closedTrx)
        return true;
    else {
        const wxString text = wxString::Format(
            wxPLURAL("You are about to edit a transaction involving an account that is closed."
            , "The edit will affect %i transactions involving an account that is closed.", GetSelectedItemCount())
            , closedTrx) + "\n\n" + _t("Do you want to perform the edit?");
        if (wxMessageBox(text, _t("Closed Account Check"), wxYES_NO | wxICON_WARNING) == wxYES)
            return true;
    }
    return false;
}

bool TransactionListCtrl::checkTransactionLocked(int64 accountID, const wxString& transdate)
{
    Model_Account::Data* account = Model_Account::instance().get(accountID);
    if (Model_Account::BoolOf(account->STATEMENTLOCKED)) {
        wxDateTime transaction_date;
        if (transaction_date.ParseDate(transdate)) {
            if (transaction_date <= Model_Account::DateOf(account->STATEMENTDATE)) {
                wxMessageBox(wxString::Format(
                    _t("Locked transaction to date: %s\n\n"
                      "Reconciled transactions.")
                    , mmGetDateTimeForDisplay(account->STATEMENTDATE))
                    , _t("MMEX Transaction Check"), wxOK | wxICON_WARNING);
                return true;
            }
        }
    }
    return false;
}
