/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2014 - 2021 Nikolay Akimov
 Copyright (C) 2021-2025 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 Klaus Wich

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

#include "JournalPanel.h"
#include "JournalList.h"

#include "base/_defs.h"
#include <float.h>
#include <algorithm>
#include <wx/clipbrd.h>
#include <wx/srchctrl.h>
#include <wx/sound.h>

#include "base/_constants.h"
#include "base/mmTips.h"
#include "util/mmPath.h"
#include "util/mmImage.h"
#include "util/mmSplitterWindow.h"
#include "util/mmNavigatorList.h"
#include "util/_util.h"
#include "util/_simple.h"
#include "util/mmCalcValidator.h"

#include "model/_all.h"
#include "model/PrefModel.h"
#include "model/Journal.h"

#include "manager/DateRangeManager.h"
#include "dialog/SplitDialog.h"
#include "dialog/AttachmentDialog.h"
#include "dialog/SchedDialog.h"
#include "dialog/TrxFilterDialog.h"
#include "dialog/TrxShareDialog.h"
#include "dialog/TrxUpdateDialog.h"
#include "dialog/ReconcileDialog.h"

#include "app/mmFrame.h"

// -- static

const std::vector<std::pair<JournalPanel::FILTER_ID, wxString>> JournalPanel::FILTER_NAME =
{
    { FILTER_ID_DATE,        "Date" },
    { FILTER_ID_DATE_RANGE,  "DateRange" },
    { FILTER_ID_DATE_PICKER, "DatePicker" },
};

const wxString JournalPanel::filter_id_name(FILTER_ID id) {
    return FILTER_NAME[id].second;
}

wxBEGIN_EVENT_TABLE(JournalPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,                   JournalPanel::onNewTrx)
    EVT_BUTTON(wxID_EDIT,                  JournalPanel::onEditTrx)
    EVT_BUTTON(wxID_DUPLICATE,             JournalPanel::onDuplicateTrx)
    EVT_BUTTON(wxID_UNDELETE,              JournalPanel::onRestoreTrx)
    EVT_BUTTON(wxID_REMOVE,                JournalPanel::onDeleteTrx)
    EVT_BUTTON(wxID_PASTE,                 JournalPanel::onEnterSched)
    EVT_BUTTON(wxID_IGNORE,                JournalPanel::onSkipSched)
    EVT_BUTTON(wxID_FILE,                  JournalPanel::onOpenAttachment)
    EVT_BUTTON(ID_DATE_RANGE_BUTTON,       JournalPanel::onDateRangePopup)
    EVT_BUTTON(ID_FILTER_BUTTON,           JournalPanel::onFilter)
    EVT_MENU(ID_FILTER_ADVANCED,           JournalPanel::onFilter)
    EVT_MENU(ID_DATE_RANGE_EDIT,           JournalPanel::onDateRangeEdit)
    EVT_TOGGLEBUTTON(ID_SCHEDULED_BUTTON,  JournalPanel::onScheduled)
    EVT_MENU_RANGE(
        ID_DATE_RANGE_MIN,
        ID_DATE_RANGE_MAX,                 JournalPanel::onDateRangeSelect)
    EVT_MENU_RANGE(
        TrxType::e_withdrawal,
        TrxType::e_transfer,               JournalPanel::onNewTrx)
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_FIND,   JournalPanel::onSearchTxtEntered)
    EVT_DATE_CHANGED(ID_START_DATE_PICKER, JournalPanel::onStartDateChanged)
    EVT_DATE_CHANGED(ID_END_DATE_PICKER,   JournalPanel::onEndDateChanged)
wxEND_EVENT_TABLE()

void JournalPanel::loadDateRanges(
    std::vector<mmDateRange2::Range>* date_range_a,
    int* date_range_m,
    bool all_ranges
) {
    date_range_a->clear();
    *date_range_m = -1;
    int src_i = 0;
    int src_m = PrefModel::instance().getCheckingRangeM();
    for (const auto& range : PrefModel::instance().getCheckingRangeA()) {
        if (date_range_a->size() > ID_DATE_RANGE_MAX - ID_DATE_RANGE_MIN) {
            break;
        }
        if (src_i == src_m) {
            *date_range_m = date_range_a->size();
        }
        if (all_ranges || !range.hasPeriodS()) {
            date_range_a->push_back(range);
        }
        src_i++;
    }
    if (*date_range_m < 0) {
        *date_range_m = date_range_a->size();
    }
}

void JournalPanel::mmPlayTransactionSound()
{
    int play = SettingModel::instance().getInt(INIDB_USE_TRANSACTION_SOUND, 0);
    if (!play)
        return;

    wxString wav_path = mmPath::getPathResource(
        (play == 2) ? mmPath::TRANS_SOUND2 : mmPath::TRANS_SOUND1
    );
    wxLogDebug("%s", wav_path);

    wxSound registerSound(wav_path);
    if (registerSound.IsOk())
        registerSound.Play(wxSOUND_ASYNC);
}

// -- state

wxString JournalPanel::getPanelTitle() const
{
    if (isAllTrans())
        return _t("All Transactions");
    else if (isDeletedTrans())
        return _t("Deleted Transactions");
    else if (isGroup()) {
        if (m_account_group_id == -3)
            return _t("Favorites");
        else {
            int account_Type = -(static_cast<int>(m_account_group_id.GetValue()) + 4);
            if (account_Type >= mmNavigatorItem::TYPE_ID_size) {
                account_Type += mmNavigatorItem::NAV_IDXDIFF;
            }
            return mmNavigatorList::instance().getAccountSectionName(account_Type);
        }
    }
    else if (m_account_n)
        return wxString::Format(_t("Account View: %s"), m_account_n->m_name);
    else
        return "";
}

// -- constructor

JournalPanel::JournalPanel(
    mmFrame* frame,
    wxWindow* perent_win,
    int64 account_group_id,
    const std::vector<int64>& group_ids /*={}*/
) :
    m_account_group_id(account_group_id),
    w_frame(frame)
{
    if (isAccount()) {
        m_account_id = m_account_group_id;
        m_account_n = AccountModel::instance().get_idN_data_n(m_account_id);
        m_currency_n = AccountModel::instance().get_data_currency_p(*m_account_n);
    }
    else if (isGroup()) {
        m_account_id_m = std::set<int64>(group_ids.begin(), group_ids.end());
        m_currency_n = CurrencyModel::instance().get_base_data_n();
    }
    else {
        m_currency_n = CurrencyModel::instance().get_base_data_n();
    }
    m_use_account_specific_filter = PrefModel::instance().getUsePerAccountFilter();

    create(perent_win);
    mmThemeAutoColour(this);
    Fit();
}

// We cannot use OnClose() event because this class deletes
// via DestroyChildren() of its parent.
JournalPanel::~JournalPanel()
{
}

bool JournalPanel::create(
    wxWindow* perent_win,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
) {
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    if (!wxPanel::Create(perent_win, mmID_CHECKING, pos, size, style, name))
        return false;

    this->windowsFreezeThaw();

    createControls();
    loadFilter();

    this->windowsFreezeThaw();
    UsageModel::instance().pageview(this);
    return true;
}

void JournalPanel::createControls()
{
    wxBoxSizer* sizerV = new wxBoxSizer(wxVERTICAL);

    /* ---------------------- */

    wxFlexGridSizer* sizerVHeader = new wxFlexGridSizer(0, 1, 0, 0);
    sizerVHeader->AddGrowableCol(0, 0);
    sizerV->Add(sizerVHeader, g_flagsBorder1V);

    w_header_text = new wxStaticText(this, wxID_STATIC, "");
    w_header_text->SetFont(this->GetFont().Larger().Bold());
    sizerVHeader->Add(w_header_text, g_flagsExpandBorder1);

    wxBoxSizer* sizerHInfo = new wxBoxSizer(wxHORIZONTAL);
    w_header_balance = new wxStaticText(this, wxID_STATIC, "");
    sizerHInfo->Add(w_header_balance, g_flagsH);
    w_header_credit = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(100,-1));
    sizerHInfo->Add(w_header_credit, g_flagsH);
    sizerVHeader->Add(sizerHInfo, g_flagsBorder1V);

    wxBoxSizer* sizerHCtrl = new wxBoxSizer(wxHORIZONTAL);
    w_range_btn = new wxButton(this, ID_DATE_RANGE_BUTTON);
    w_range_btn->SetBitmap(mmImage::bitmapBundle(mmImage::png::TRANSFILTER, mmImage::bitmapButtonSize));
    sizerHCtrl->Add(w_range_btn, g_flagsH);

    w_range_btn->SetMinSize(wxSize(200 + PrefModel::instance().getIconSize() * 2, -1));

    w_start_date = new wxDatePickerCtrl(this, ID_START_DATE_PICKER,
        wxDefaultDateTime, wxDefaultPosition, wxDefaultSize,
        wxDP_DROPDOWN | wxDP_ALLOWNONE
    );
    w_start_date->SetRange(wxInvalidDateTime, wxDateTime::Now());
    sizerHCtrl->Add(w_start_date, g_flagsH);

    w_end_date = new wxDatePickerCtrl(this, ID_END_DATE_PICKER,
        wxDefaultDateTime, wxDefaultPosition, wxDefaultSize,
        wxDP_DROPDOWN | wxDP_ALLOWNONE
    );
    sizerHCtrl->Add(w_end_date, g_flagsH);

    // Filter for transaction details
    w_filter_btn = new wxButton(this, ID_FILTER_BUTTON, _tu("Filter…"));
    w_filter_btn->SetMinSize(wxSize(150 + PrefModel::instance().getIconSize() * 2, -1));
    sizerHCtrl->Add(
        w_filter_btn,
        0,
        wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxUP | wxDOWN | wxLEFT,
        5
    );

    w_filter_reset_btn = new wxBitmapButton(this, wxID_ANY,
        mmImage::bitmapBundle(mmImage::png::CLEAR, mmImage::bitmapButtonSize)
    );
    mmToolTip(w_filter_reset_btn, _t("Reset filter"));
    w_filter_reset_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
        &JournalPanel::onFilterCancel, this
    );
    sizerHCtrl->Add(
        w_filter_reset_btn,
        0,
        wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,
        2
    );

    if (!isDeletedTrans()) {
        sizerHCtrl->AddSpacer(15);
        const auto& size = w_range_btn->GetSize().GetY();
        w_header_scheduled = new wxBitmapToggleButton(
            this, ID_SCHEDULED_BUTTON, mmImage::bitmapBundle(mmImage::png::RECURRING),
            wxDefaultPosition, wxSize(size, size)
        );
        sizerHCtrl->Add(w_header_scheduled, g_flagsH);
        sizerHCtrl->AddSpacer(10);
    }
    w_header_sortOrder = new wxStaticText(this, wxID_STATIC, "");
    sizerHCtrl->Add(w_header_sortOrder, g_flagsH);

    sizerHCtrl->AddStretchSpacer(1);
    if (isAccount()) {
        wxBitmapButton* btn = new wxBitmapButton(this, wxID_ANY,
            mmImage::bitmapBundle(mmImage::png::TRXNUM, mmImage::bitmapButtonSize)
        );
        mmToolTip(btn, _t("Reconcile"));
        btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &JournalPanel::onReconcile, this);
        sizerHCtrl->Add(btn, 0, wxALIGN_CENTER_VERTICAL, 5);
    }

    sizerV->Add(sizerHCtrl, 0, wxEXPAND | wxALL, 15);

    w_range_btn->Connect(wxEVT_RIGHT_DOWN,
        wxMouseEventHandler(JournalPanel::onButtonRightDown),
        nullptr, this
    );

    /* ---------------------- */

    mmSplitterWindow* splitterListFooter = new mmSplitterWindow(
        this, wxID_ANY,
        wxDefaultPosition, wxSize(200, 200),
        wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER,
        mmImage::themeMetaColour(mmImage::COLOR_LISTPANEL)
    );

    w_image_a.push_back(mmImage::bitmapBundle(mmImage::png::UNRECONCILED));
    w_image_a.push_back(mmImage::bitmapBundle(mmImage::png::RECONCILED));
    w_image_a.push_back(mmImage::bitmapBundle(mmImage::png::VOID_STAT));
    w_image_a.push_back(mmImage::bitmapBundle(mmImage::png::FOLLOW_UP));
    w_image_a.push_back(mmImage::bitmapBundle(mmImage::png::DUPLICATE_STAT));
    w_image_a.push_back(mmImage::bitmapBundle(mmImage::png::UPARROW));
    w_image_a.push_back(mmImage::bitmapBundle(mmImage::png::DOWNARROW));

    w_list = new JournalList(this, splitterListFooter);
    w_list->SetSmallImages(w_image_a);
    w_list->SetNormalImages(w_image_a);

    wxPanel* panelFooter = new wxPanel(
        splitterListFooter, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxNO_BORDER | wxTAB_TRAVERSAL
    );

    splitterListFooter->SplitHorizontally(w_list, panelFooter);
    splitterListFooter->SetMinimumPaneSize(100);
    splitterListFooter->SetSashGravity(1.0);

    sizerV->Add(splitterListFooter, g_flagsExpandBorder1);

    this->SetSizerAndFit(sizerV);

    wxBoxSizer* sizerVFooter = new wxBoxSizer(wxVERTICAL);
    panelFooter->SetSizer(sizerVFooter);

    wxBoxSizer* sizerHButtons = new wxBoxSizer(wxHORIZONTAL);
    sizerVFooter->Add(sizerHButtons, wxSizerFlags(g_flagsExpandBorder1).Proportion(0));

    w_delete_btn = new wxButton(panelFooter, wxID_REMOVE, _t("&Delete "));
    mmToolTip(w_delete_btn, _t("Delete all selected transactions"));

    if (!isDeletedTrans()) {
        w_new_btn = new wxButton(panelFooter, wxID_NEW, _t("&New "));
        mmToolTip(w_new_btn, _t("New Transaction"));
        sizerHButtons->Add(w_new_btn, 0, wxRIGHT, 2);

        w_edit_btn = new wxButton(panelFooter, wxID_EDIT, _t("&Edit "));
        mmToolTip(w_edit_btn, _t("Edit all selected transactions"));
        sizerHButtons->Add(w_edit_btn, 0, wxRIGHT, 2);
        w_edit_btn->Enable(false);

        w_dup_btn = new wxButton(panelFooter, wxID_DUPLICATE, _t("D&uplicate "));
        mmToolTip(w_dup_btn, _t("Duplicate selected transaction"));
        sizerHButtons->Add(w_dup_btn, 0, wxRIGHT, 2);
        w_dup_btn->Enable(false);

        sizerHButtons->Add(w_delete_btn, 0, wxRIGHT, 2);
        w_delete_btn->Enable(false);

        w_enter_btn = new wxButton(panelFooter, wxID_PASTE, _t("Ente&r"));
        mmToolTip(w_enter_btn, _t("Enter Next Scheduled Transaction Occurrence"));
        sizerHButtons->Add(w_enter_btn, 0, wxRIGHT, 2);
        w_enter_btn->Enable(false);

        w_skip_btn = new wxButton(panelFooter, wxID_IGNORE, _t("&Skip"));
        mmToolTip(w_skip_btn, _t("Skip Next Scheduled Transaction Occurrence"));
        sizerHButtons->Add(w_skip_btn, 0, wxRIGHT, 2);
        w_skip_btn->Enable(false);

        const auto& btnDupSize = w_dup_btn->GetSize();
        w_attachment_btn = new wxBitmapButton(
            panelFooter, wxID_FILE, mmImage::bitmapBundle(mmImage::png::CLIP), wxDefaultPosition,
            wxSize(30, btnDupSize.GetY())
        );
        mmToolTip(w_attachment_btn, _t("Open attachments"));
        sizerHButtons->Add(w_attachment_btn, 0, wxRIGHT, 5);
        w_attachment_btn->Enable(false);

        w_attachment_btn->Connect(
            wxEVT_RIGHT_DOWN,
            wxMouseEventHandler(JournalPanel::onButtonRightDown),
            nullptr, this
        );
        w_new_btn->Connect(
            wxEVT_RIGHT_DOWN,
            wxMouseEventHandler(JournalPanel::onButtonRightDown),
            nullptr, this
        );
    }
    else {
        w_restore_btn = new wxButton(panelFooter, wxID_UNDELETE, _t("&Restore "));
        mmToolTip(w_restore_btn, _t("Restore selected transaction"));
        sizerHButtons->Add(w_restore_btn, 0, wxRIGHT, 5);
        w_restore_btn->Enable(false);

        sizerHButtons->Add(w_delete_btn, 0, wxRIGHT, 5);
        w_delete_btn->Enable(false);
    }

    wxSearchCtrl* searchCtrl = new wxSearchCtrl(
        panelFooter,
        wxID_FIND, wxEmptyString, wxDefaultPosition,
        wxSize(100, w_delete_btn->GetSize().GetHeight()),
        wxTE_NOHIDESEL, wxDefaultValidator
    );
    searchCtrl->SetDescriptiveText(_t("Search"));
    sizerHButtons->Add(searchCtrl, g_flagsExpandBorder1);
    mmToolTip(searchCtrl,
        _t("Enter any string to find it in the nearest transaction data") + "\n\n" +
        _tu("Tip: Wildcard characters such as the question mark (?) and the asterisk (*) can be used in search criteria.") + "\n" +
        _tu("The question mark (?) is used to match a single character, for example \"s?t\" finds both \"sat\" and \"set\".") + "\n" +
        _tu("The asterisk (*) is used to match any number of characters, for example \"s*d\" finds both \"sad\" and \"started\".") + "\n" +
        _tu("Use the asterisk (*) at the beginning to find any string in the middle of the sentence.") + "\n" +
        _t("Use regex: to match using regular expressions.")
    );

    // Infobar-mini
    w_mini_text = new wxStaticText(panelFooter, wxID_STATIC, "");
    sizerHButtons->Add(w_mini_text, 1, wxGROW | wxTOP | wxLEFT, 5);

    // Infobar
    w_info_text = new wxStaticText(
        panelFooter, wxID_STATIC, "", wxDefaultPosition, wxSize(200, -1),
        wxTE_MULTILINE | wxTE_WORDWRAP
    );
    sizerVFooter->Add(w_info_text, g_flagsExpandBorder1);
    mmToolTip(w_info_text, _t("Click to copy to clipboard"));

    w_info_text->Bind(wxEVT_LEFT_DOWN,
        [this, infoPanel = w_info_text](wxMouseEvent& event) {
            onInfoPanelClick(event, infoPanel);
        }
    );

    this->Fit();
    //Show tips when no any transaction selected
    showTips();
}

// -- methods

// Refresh account screen with new details
void JournalPanel::loadAccount(int64 account_id)
{
    wxASSERT (account_id >= 1);

    w_list->setVisibleItemIndex(-1);
    m_account_group_id = account_id;
    m_account_id = account_id;
    m_account_id_m = {};
    m_account_n = AccountModel::instance().get_idN_data_n(m_account_id);
    m_currency_n = AccountModel::instance().get_data_currency_p(*m_account_n);
    m_use_account_specific_filter = PrefModel::instance().getUsePerAccountFilter();

    loadFilter();

    showTips();
    enableButtons(false, false, false, false, false, false);
}

void JournalPanel::refreshList()
{
    w_list->refreshVisualList();
}

void JournalPanel::resetColumnView()
{
    w_list->DeleteAllColumns();
    w_list->setColumnsInfo();
    w_list->createColumns();
    w_list->refreshVisualList();
}

void JournalPanel::setSelectedTransaction(JournalKey journal_key)
{
    w_list->setSelectedId(journal_key);
    refreshList();
    w_list->SetFocus();
}

// -- methods

void JournalPanel::loadFilter()
{
    loadFilterSettings();
    updateFilter();
    updateFilterTooltip();
    refreshList();
}

void JournalPanel::loadFilterSettings()
{
    wxString key = m_use_account_specific_filter
        ? wxString::Format("CHECK_FILTER_DEDICATED_%lld", m_account_group_id)
        : "CHECK_FILTER_ALL";
    Document j_doc = InfoModel::instance().getJdoc(key, "{}");

    int fid = 0;
    if (JSON_GetIntValue(j_doc, "FILTER_ID", fid)) {
        m_filter_id = static_cast<FILTER_ID>(fid);
    }
    else {
        m_filter_id = FILTER_ID_DATE_RANGE;
    }

    loadDateRanges(&m_date_range_a, &m_date_range_m, isAccount());

    if (m_filter_id == FILTER_ID_DATE_RANGE) {
        // recreate m_date_range in order to reload parameters from setting,
        // refresh the date of today, and clear the default start/end dates
        m_date_range = mmDateRange2();

        // load m_date_range from settings.
        // the start/end date pickers are configured later in updateFilter().
        wxString j_filter;
        bool found = false;
        if (JSON_GetStringValue(j_doc, "FILTER_DATE", j_filter)) {
            // find range specification
            for (const auto& range : m_date_range_a) {
                if (range.getName() == j_filter) {
                    m_date_range.setRange(range);
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            // init with 'all'
            m_date_range.setRange(m_date_range_a[0]);
        }
        if (isAccount()) {
            m_date_range.setSDateN(m_account_n->m_stmt_date_n);
        }
    }
    else if (m_filter_id == FILTER_ID_DATE_PICKER) {
        // Load start/end date pickers from settings.
        // The date range is configured later in updateFilter().
        wxString date_str;
        wxString::const_iterator end;
        wxDateTime start_dateTime, end_dateTime;
        if (JSON_GetStringValue(j_doc, "FILTER_DATE_BEGIN", date_str)) {
            start_dateTime.ParseFormat(date_str, "%Y-%m-%d", &end);
        }
        if (JSON_GetStringValue(j_doc, "FILTER_DATE_END", date_str)) {
            end_dateTime.ParseFormat(date_str, "%Y-%m-%d", &end);
        }
        // initialize pickers (also when start/end dates are undefined)
        w_start_date->SetValue(start_dateTime);
        w_end_date->SetValue(end_dateTime);
    }

    m_scheduled_enable = !isDeletedTrans();
    m_scheduled_selected = false;
    if (!isDeletedTrans()) {
        JSON_GetBoolValue(j_doc, "SCHEDULED", m_scheduled_selected);
    }

    wxString j_str = InfoModel::instance().getString(
        wxString::Format("CHECK_FILTER_ID_ADV_%lld", m_account_group_id),
        "{}"
    );
    w_filter_dlg.reset(new TrxFilterDialog(this, m_account_id, false, j_str));
    m_filter_advanced = w_filter_dlg->mmIsSomethingChecked();
}

void JournalPanel::saveFilterSettings()
{
    wxString key = m_use_account_specific_filter
        ? wxString::Format("CHECK_FILTER_DEDICATED_%lld", m_account_group_id)
        : "CHECK_FILTER_ALL";
    Document j_doc = InfoModel::instance().getJdoc(key, "{}");
    InfoModel::saveFilterInt(j_doc, "FILTER_ID", m_filter_id);
    InfoModel::saveFilterString(j_doc, "FILTER_NAME", filter_id_name(m_filter_id));

    if (m_filter_id == FILTER_ID_DATE_RANGE) {
        if (!m_date_range.rangeName().IsEmpty()) {
            InfoModel::saveFilterString(j_doc, "FILTER_DATE", m_date_range.rangeName());
            InfoModel::saveFilterString(j_doc, "FILTER_DATE_BEGIN", "");
            InfoModel::saveFilterString(j_doc, "FILTER_DATE_END", "");
        }
        else {
            wxLogError("JournalPanel::saveFilterSettings(): m_date_range.rangeName() is empty");
        }
    }
    else if (m_filter_id == FILTER_ID_DATE_PICKER) {
        InfoModel::saveFilterString(j_doc, "FILTER_DATE", "");
        InfoModel::saveFilterString(j_doc,
            "FILTER_DATE_BEGIN",
            mmDateN(w_start_date->GetValue()).isoDateN()
        );
        InfoModel::saveFilterString(j_doc,
            "FILTER_DATE_END",
            mmDateN(w_end_date->GetValue()).isoDateN()
        );
    }

    if (!isDeletedTrans()) {
        InfoModel::saveFilterBool(j_doc,
            "SCHEDULED",
            m_scheduled_selected
        );
    }
    InfoModel::instance().saveJdoc(key, j_doc);

    key = wxString::Format("CHECK_FILTER_ID_ADV_%lld", m_account_group_id);
    InfoModel::instance().saveString(key,
        m_filter_advanced
            ? w_filter_dlg->mmGetJsonSettings()
            : "{}"
    );
}

void JournalPanel::filterList()
{
    bool ignore_future = PrefModel::instance().getIgnoreFutureTransactions();
    mmDate range_start = m_date_range.rangeStartN().value();
    mmDate range_end = m_date_range.rangeEndN().value();

    int sn = 0; // sequence number
    m_flow = 0.0;
    m_balance = m_account_n ? m_account_n->m_open_balance : 0.0;
    m_reconciled_balance = m_today_reconciled_balance = m_balance;
    m_show_reconciled = false;

    w_list->m_journal_xa.clear();

    TrxModel::DataA trx_a =
        isDeletedTrans() ? TrxModel::instance().find_data_a(
            TrxModel::WHERE_DATE(OP_GE, range_start),
            TrxModel::WHERE_DATE(OP_LE, range_end),
            TrxModel::WHERE_IS_DELETED(true)
        )
        : isAccount() ? TrxModel::instance().find_data_a(
            TableClause::BEGIN_OR(),
                TrxCol::WHERE_ACCOUNTID(OP_EQ, m_account_n->m_id),
                TrxCol::WHERE_TOACCOUNTID(OP_EQ, m_account_n->m_id),
            TableClause::END(),
            TrxModel::WHERE_DATE(OP_LE, range_end),
            TrxModel::WHERE_IS_DELETED(false)
        )
        : TrxModel::instance().find_data_a(
            TrxModel::WHERE_DATE(OP_GE, range_start),
            TrxModel::WHERE_DATE(OP_LE, range_end),
            TrxModel::WHERE_IS_DELETED(false)
        );
    if (PrefModel::instance().getUseTransDateTime()) {
        std::sort(trx_a.begin(), trx_a.end(), TrxData::SorterByDateTimeId());
    }
    else {
        std::sort(trx_a.begin(), trx_a.end(), TrxData::SorterByDateId());
    }

    const auto trxId_tpA_m = TrxSplitModel::instance().find_all_mTrxId();
    const auto trxId_glA_m = TagLinkModel::instance().find_refType_mRefId(
        TrxModel::s_ref_type
    );
    const auto trxId_fvA_m = FieldValueModel::instance().find_refType_mRefId(
        TrxModel::s_ref_type
    );
    const auto trxId_attA_m = AttachmentModel::instance().find_refType_mRefId(
        TrxModel::s_ref_type
    );

    static wxArrayString udfc_fields = FieldModel::UDFC_FIELDS();
    int64 udfc_id[5];
    FieldTypeN udfc_type[5];
    int udfc_scale[5];
    for (int i = 0; i < 5; i++) {
        // note: udfc_fields starts with ""
        wxString field = udfc_fields[i+1];
        udfc_id[i] = FieldModel::instance().get_udfc_id_n(
            TrxModel::s_ref_type, field
        );
        udfc_type[i] = FieldModel::instance().get_udfc_type_n(
            TrxModel::s_ref_type, field
        );
        udfc_scale[i] = FieldModel::getDigitScale(
            FieldModel::instance().get_udfc_properties_n(
                TrxModel::s_ref_type,
                field
            )
        );
    }

    SchedModel::DataA sched_a;
    std::map<int64, SchedSplitModel::DataA> schedId_qpA_m;
    std::map<int64, TagLinkModel::DataA> schedId_glA_m;
    std::map<int64, FieldValueModel::DataA> schedId_fvA_m;
    std::map<int64, AttachmentModel::DataA> schedId_attA_m;
    typedef std::tuple<
        int    /* sched_i: index into sched_a */,
        mmDate /* date */,
        int    /* repeat_id */
    > SchedIndex;
    std::vector<SchedIndex> sched_index_a;

    if (m_scheduled_enable && m_scheduled_selected) {
        sched_a = m_account_n
            ? AccountModel::instance().find_id_sched_a(m_account_n->m_id)
            : SchedModel::instance().find_data_a(
                TableClause::ORDERBY(SchedCol::s_primary_name)
            );
        schedId_qpA_m = SchedSplitModel::instance().find_all_mSchedId();
        schedId_glA_m = TagLinkModel::instance().find_refType_mRefId(
            SchedModel::s_ref_type
        );
        schedId_fvA_m = FieldValueModel::instance().find_refType_mRefId(
            SchedModel::s_ref_type
        );
        schedId_attA_m = AttachmentModel::instance().find_refType_mRefId(
            SchedModel::s_ref_type
        );

        for (unsigned int sched_i = 0; sched_i < sched_a.size(); ++sched_i) {
            int limit = 1000;  // this is enough for daily repetitions for one year
            std::vector<mmDate> date_a = sched_a[sched_i].unroll(range_end, limit);
            for (unsigned int repeat_id = 1; repeat_id <= date_a.size(); ++repeat_id) {
                sched_index_a.push_back({
                    sched_i, date_a[repeat_id-1], repeat_id
                });
            }
        }
        if (PrefModel::instance().getUseTransDateTime()) {
            std::sort(sched_index_a.begin(), sched_index_a.end(),
                [sched_a](const SchedIndex& a, const SchedIndex& b) -> bool {
                    int a_sched_i = std::get<0>(a);
                    int b_sched_i = std::get<0>(b);
                    const wxString a_date = std::get<1>(a).isoDate();
                    const wxString b_date = std::get<1>(b).isoDate();
                    const wxString a_isoTime = sched_a[a_sched_i].m_isoTime();
                    const wxString b_isoTime = sched_a[b_sched_i].m_isoTime();
                    return a_date < b_date || (a_date == b_date && (
                        a_isoTime < b_isoTime || (a_isoTime == b_isoTime &&
                            a_sched_i < b_sched_i
                        )
                    ));
                }
            );
        }
        else {
            std::sort(sched_index_a.begin(), sched_index_a.end(),
                [](const SchedIndex& a, const SchedIndex& b) -> bool {
                    int a_sched_i = std::get<0>(a);
                    int b_sched_i = std::get<0>(b);
                    const wxString a_date = std::get<1>(a).isoDate();
                    const wxString b_date = std::get<1>(b).isoDate();
                    return a_date < b_date || (a_date == b_date &&
                        a_sched_i < b_sched_i
                    );
                }
            );
        }
    }

    auto trx_it = trx_a.begin();
    auto sched_index_it = sched_index_a.begin();
    while (trx_it != trx_a.end() || sched_index_it != sched_index_a.end()) {
        int sched_i = -1;
        mmDateTime trx_dateTime = mmDateTime::invalid();
        int repeat_id = -1;
        TrxData sched_trx_d;
        const TrxData* trx_n = nullptr;

        if (trx_it != trx_a.end() && (sched_index_it == sched_index_a.end() ||
            trx_it->m_date() <= std::get<1>(*sched_index_it)
        )) {
            trx_dateTime = trx_it->m_datetime;
            trx_n = &(*trx_it);
            trx_it++;
        }
        else {
            sched_i = std::get<0>(*sched_index_it);
            mmDate trx_date = std::get<1>(*sched_index_it);
            trx_dateTime = PrefModel::instance().getUseTransDateTime()
                ? mmDateTime(trx_date, sched_a[sched_i].m_isoTime())
                : mmDateTime(trx_date);
            repeat_id = std::get<2>(*sched_index_it);
            sched_trx_d = Journal::execute_bill(sched_a[sched_i], trx_dateTime);
            trx_n = &sched_trx_d;
            sched_index_it++;
        }

        if (isGroup() &&
            m_account_id_m.find(trx_n->m_account_id) == m_account_id_m.end() &&
            m_account_id_m.find(trx_n->m_to_account_id_n) == m_account_id_m.end()
        )
            continue;
        if (isDeletedTrans() != trx_n->is_deleted())
            continue;

        // check if trx_n is in future, with granularity of a day
        bool is_future = trx_n->m_date() > mmDate::today();

        if (ignore_future && is_future)
            break;

        // update m_balance even if tran is filtered out
        double account_flow = 0.0;
        if (isAccount()) {
            // assertion: !trx_n->is_deleted()
            account_flow = trx_n->account_flow(m_account_id);
            m_balance += account_flow;
            if (trx_n->is_reconciled()) {
                m_reconciled_balance += account_flow;
                if (!is_future)
                    m_today_reconciled_balance += account_flow;
            }
            else
                m_show_reconciled = true;
        }

        if (trx_dateTime.date() < range_start || trx_dateTime.date() > range_end)
            continue;

        Journal::DataExt journal_dx = (repeat_id < 0)
            ? Journal::DataExt(*trx_n, trxId_tpA_m, trxId_glA_m)
            : Journal::DataExt(sched_a[sched_i], trx_dateTime, repeat_id,
                schedId_qpA_m, schedId_glA_m
            );

        bool expandSplits = false;
        if (m_filter_advanced) {
            int txnMatch = w_filter_dlg->mmIsRecordMatches(*trx_n, journal_dx.m_tp_a);
            if (txnMatch) {
                expandSplits = (txnMatch < static_cast<int>(journal_dx.m_tp_a.size()) + 1);
            }
            else {
              continue;
            }
        }

        journal_dx.PAYEENAME = journal_dx.real_payee_name(m_account_id);
        if (isAccount()) {
            if (journal_dx.m_account_w_id_n != m_account_id) {
                journal_dx.m_account_w_id_n = -1; journal_dx.m_amount_w = 0.0;
            }
            if (journal_dx.m_account_d_id_n != m_account_id) {
                journal_dx.m_account_d_id_n = -1; journal_dx.m_amount_d = 0.0;
            }
            journal_dx.m_account_flow = account_flow;
            journal_dx.m_account_balance = m_balance;
        }

        if (repeat_id < 0 &&
            trxId_attA_m.find(trx_n->m_id) != trxId_attA_m.end()
        ) {
            for (const auto& att_d : trxId_attA_m.at(trx_n->m_id))
                journal_dx.ATTACHMENT_DESCRIPTION.Add(att_d.m_description);
        }
        else if (repeat_id > 0 &&
            schedId_attA_m.find(journal_dx.m_sched_id) != schedId_attA_m.end()
        ) {
            for (const auto& att_d : schedId_attA_m.at(journal_dx.m_sched_id))
                journal_dx.ATTACHMENT_DESCRIPTION.Add(att_d.m_description);
        }

        for (int i = 0; i < 5; i++) {
            journal_dx.UDFC_type[i] = FieldTypeN();
            journal_dx.UDFC_value[i] = -DBL_MAX;
        }

        if (repeat_id < 0 &&
            trxId_fvA_m.find(trx_n->m_id) != trxId_fvA_m.end()
        ) {
            for (const auto& udfc : trxId_fvA_m.at(trx_n->m_id)) {
                for (int i = 0; i < 5; i++) {
                    if (udfc.m_field_id == udfc_id[i]) {
                        journal_dx.UDFC_type[i] = udfc_type[i];
                        journal_dx.UDFC_content[i] = udfc.m_content;
                        journal_dx.UDFC_value[i] = cleanseNumberStringToDouble(
                            udfc.m_content, udfc_scale[i] > 0
                        );
                        break;
                    }
                }
            }
        }
        else if (repeat_id > 0 &&
            schedId_fvA_m.find(journal_dx.m_sched_id) != schedId_fvA_m.end()
        ) {
            for (const auto& udfc : schedId_fvA_m.at(journal_dx.m_sched_id)) {
                for (int i = 0; i < 5; i++) {
                    if (udfc.m_field_id == udfc_id[i]) {
                        journal_dx.UDFC_type[i] = udfc_type[i];
                        journal_dx.UDFC_content[i] = udfc.m_content;
                        journal_dx.UDFC_value[i] = cleanseNumberStringToDouble(
                            udfc.m_content, udfc_scale[i] > 0
                        );
                        break;
                    }
                }
            }
        }

        wxString marker = (repeat_id < 0) ? "" : "*";
        journal_dx.SN = ++sn;
        journal_dx.displaySN = wxString::Format("%s%ld", marker, journal_dx.SN);
        if (repeat_id > 0)
            journal_dx.displayID = wxString::Format("%s%ld", marker, journal_dx.m_sched_id);

        if (!expandSplits) {
            w_list->m_journal_xa.push_back(journal_dx);
            if (isAccount())
                m_flow += account_flow;
            continue;
        }

        int splitIndex = 1;
        wxString tranTagnames = journal_dx.TAGNAMES;
        wxString tranDisplaySN = journal_dx.displaySN;
        wxString tranDisplayID = journal_dx.displayID;
        for (const auto& tp_d : journal_dx.m_tp_a) {
            if (m_filter_advanced &&
                !w_filter_dlg->mmIsSplitRecordMatches<TrxSplitModel>(tp_d)
            ) {
                  continue;
            }

            journal_dx.displaySN = tranDisplaySN + "." + wxString::Format("%i", splitIndex);
            journal_dx.displayID = tranDisplayID + "." + wxString::Format("%i", splitIndex);
            splitIndex++;
            journal_dx.m_category_id_n = tp_d.m_category_id;
            journal_dx.CATEGNAME       = CategoryModel::instance().get_id_fullname(tp_d.m_category_id);
            journal_dx.m_amount        = tp_d.m_amount;
            journal_dx.m_notes         = trx_n->m_notes;
            journal_dx.TAGNAMES        = tranTagnames;
            TrxData journal_trx_dx = journal_dx;
            TrxData journal_split_dx = journal_dx;
            journal_split_dx.m_notes = tp_d.m_notes;
            if (m_filter_advanced &&
                !w_filter_dlg->mmIsRecordMatches<TrxModel>(journal_split_dx, true) &&
                !w_filter_dlg->mmIsRecordMatches<TrxModel>(journal_trx_dx, true)
            ) {
                continue;
            }

            if (isAccount()) {
                journal_dx.m_account_flow = journal_trx_dx.account_flow(m_account_id);
                m_flow += journal_dx.m_account_flow;
            }
            journal_dx.m_notes.Append((trx_n->m_notes.IsEmpty() ? "" : " ") + tp_d.m_notes);
            wxString tag_names;
            for (const auto& tag_name_id : TagLinkModel::instance().find_ref_mTagName(
                (repeat_id < 0 ? TrxSplitModel::s_ref_type : SchedSplitModel::s_ref_type),
                tp_d.m_id
            )) {
                tag_names.Append(tag_name_id.first + " ");
            }
            if (!tag_names.IsEmpty()) {
                journal_dx.TAGNAMES.Append(
                    (journal_dx.TAGNAMES.IsEmpty() ? "" : ", ") +
                    tag_names.Trim()
                );
            }
            w_list->m_journal_xa.push_back(journal_dx);
        }
    }
}

// -- methods

void JournalPanel::updateHeader()
{
    w_header_text->SetLabelText(getPanelTitle());
    w_header_credit->Hide();
    if (m_account_n) {
        wxString summary = wxString::Format("%s%s",
            _t("Account Bal: "),
            AccountModel::instance().value_number_currency(*m_account_n, m_balance)
        );
        if (m_show_reconciled) summary.Append(wxString::Format("     %s%s     %s%s",
            _t("Reconciled Bal: "),
            AccountModel::instance().value_number_currency(
                *m_account_n,
                m_reconciled_balance
            ),
            _t("Diff: "),
            AccountModel::instance().value_number_currency(
                *m_account_n,
                m_balance - m_reconciled_balance
            )
        ));
        summary.Append(wxString::Format("     %s%s",
            _t("Filtered Flow: "),
            AccountModel::instance().value_number_currency(*m_account_n, m_flow)
        ));
        if (m_account_n->m_credit_limit != 0.0) {
            double limit = (m_balance < 0.0)
                ? 100.0 * (-m_balance) / m_account_n->m_credit_limit
                : 0.0;
            summary.Append(wxString::Format("   %s %.1f%%",
                _t("Credit Limit:"),
                limit
            ));
            w_header_credit->SetRange(std::max(100.0, limit));
            w_header_credit->SetValue(limit);
            w_header_credit->Show();
        }
        if (AccountModel::type_id(*m_account_n) == mmNavigatorItem::TYPE_ID_INVESTMENT ||
            AccountModel::type_id(*m_account_n) == mmNavigatorItem::TYPE_ID_ASSET
        ) {
            std::pair<double, double> investment_bal =
                AccountModel::instance().get_data_investment_balance(*m_account_n);
            summary.Append(wxString::Format("     %s%s",
                _t("Market Value: "),
                AccountModel::instance().value_number_currency(
                    *m_account_n,
                    investment_bal.first
                )
            ));
            summary.Append(wxString::Format("     %s%s",
                _t("Invested: "),
                AccountModel::instance().value_number_currency(
                    *m_account_n,
                    investment_bal.second
                )
            ));
        }
        w_header_balance->SetLabelText(summary);
    }
    this->Layout();
}

void JournalPanel::updateFilter()
{
    if (m_filter_id < FILTER_ID_DATE_RANGE || m_filter_id > FILTER_ID_DATE_PICKER)
        m_filter_id = FILTER_ID_DATE_RANGE;

    if (m_filter_id == FILTER_ID_DATE_RANGE) {
        w_range_btn->SetLabel(m_date_range.rangeName());
        w_range_btn->SetBitmap(mmImage::bitmapBundle(
            // FIXME: refine the condition below
            (m_date_range.rangeName() != m_date_range_a[0].getName()
                ? mmImage::png::TRANSFILTER_ACTIVE
                : mmImage::png::TRANSFILTER
            ),
            mmImage::bitmapButtonSize
        ));
        // TODO: calculate default start/end dates from model
        m_date_range.setDefStartDateN(mmDate::min());
        m_date_range.setDefEndDateN(mmDate::today().plusDateSpan(wxDateSpan::Days(30)));
        // copy from date range to start/end pickers
        w_start_date->SetValue(
            m_date_range.rangeStartN().value().dateTime()
        );
        w_end_date->SetValue(
            m_date_range.rangeEndN().value().dateTime()
        );
    }
    else if (m_filter_id == FILTER_ID_DATE_PICKER) {
        w_range_btn->SetLabel(_t("Date range"));
        w_range_btn->SetBitmap(mmImage::bitmapBundle(
            mmImage::png::TRANSFILTER_ACTIVE,
            mmImage::bitmapButtonSize
        ));
        // set date range to default ('All') and copy default start/end dates from pickers.
        m_date_range = mmDateRange2();
        mmDateRange2::Range range = m_date_range.getRange();
        range.setName(_t("Date range"));
        m_date_range.setRange(range);
        m_date_range.setDefStartDateN(mmDateN(w_start_date->GetValue()));
        m_date_range.setDefEndDateN(mmDateN(w_end_date->GetValue()));
        if (isAccount()) {
            m_date_range.setSDateN(m_account_n->m_stmt_date_n);
        }
    }

    w_filter_btn->SetBitmap(mmImage::bitmapBundle(
        m_filter_advanced
            ? mmImage::png::TRANSFILTER_ACTIVE
            : mmImage::png::TRANSFILTER,
        mmImage::bitmapButtonSize
    ));

    w_filter_reset_btn->Enable(m_filter_advanced);

    if (!isDeletedTrans()) {
        w_header_scheduled->SetValue(m_scheduled_selected);
        w_header_scheduled->Enable(m_scheduled_enable);
        updateScheduledToolTip();
    }
}

void JournalPanel::updateFilterTooltip()
{
    if (w_filter_dlg && w_filter_dlg->mmIsSomethingChecked()) {
        w_filter_btn->SetToolTip(w_filter_dlg->mmGetDescriptionToolTip());
    }
    else {
        w_filter_btn->UnsetToolTip();
    }
}

void JournalPanel::updateScheduledToolTip()
{
   mmToolTip(w_header_scheduled,
        !m_scheduled_enable ?
        _t("Scheduled transactions are not available, because the current filter ends in the past") : !m_scheduled_selected ? _t("Click to show scheduled transactions. This feature works best with filter choices that extend into the future (e.g., Current Month).") :
        _t("Click to hide scheduled transactions."));
}

void JournalPanel::updateExtraTransactionData(bool single, int repeat_id, bool foreign)
{
    if (single) {
        enableButtons(
            /* Edit      */ true,
            /* Duplicate */ !foreign,
            /* Delete    */ repeat_id < 0,
            /* Enter     */ repeat_id == 1,
            /* Skip      */ repeat_id == 1,
            /* attach    */ true
        );

        long x = -1;
        for (x = 0; x < w_list->GetItemCount(); x++) {
            if (w_list->GetItemState(x, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
                break;
        }

        Journal::DataExt journal_dx(w_list->m_journal_xa[x]);
        wxString miniStr = journal_dx.info();
        //Show only first line but full string set as tooltip
        if (miniStr.Find("\n") > 1 && !miniStr.IsEmpty()) {
            w_mini_text->SetLabelText(
                miniStr.substr(0, miniStr.Find("\n")) + wxString::FromUTF8Unchecked(" …")
            );
            mmToolTip(w_mini_text, miniStr);
        }
        else {
            w_mini_text->SetLabelText(miniStr);
            mmToolTip(w_mini_text, miniStr);
        }

        wxString notesStr = journal_dx.m_notes;
        if (journal_dx.m_repeat_id < 0) {
            for (const auto& tp_d : TrxSplitModel::instance().find_data_a(
                TrxSplitCol::WHERE_TRANSID(OP_EQ, journal_dx.m_id)
            )) {
                if (!tp_d.m_notes.IsEmpty()) {
                    notesStr += notesStr.empty() ? "" : "\n";
                    notesStr += tp_d.m_notes;
                }
            }
            if (journal_dx.has_attachment()) {
                for (const auto& att_d : AttachmentModel::instance().find_ref_data_a(
                    TrxModel::s_ref_type, journal_dx.m_id)
                ) {
                    notesStr += notesStr.empty() ? "" : "\n";
                    notesStr += _t("Attachment") + " " + att_d.m_description + " " + att_d.m_filename;
                }
            }
        }
        else {
            for (const auto& qp_d : SchedSplitModel::instance().find_data_a(
                SchedSplitCol::WHERE_TRANSID(OP_EQ, journal_dx.m_sched_id)
            )) {
                if (!qp_d.m_notes.IsEmpty()) {
                    notesStr += notesStr.empty() ? "" : "\n";
                    notesStr += qp_d.m_notes;
                }
            }
            if (journal_dx.has_attachment()) {
                for (const auto& att_d : AttachmentModel::instance().find_ref_data_a(
                    SchedModel::s_ref_type, journal_dx.m_sched_id)
                ) {
                    notesStr += notesStr.empty() ? "" : "\n";
                    notesStr += _t("Attachment") + " " + att_d.m_description + " " + att_d.m_filename;
                }
            }
        }
        // Not displaying any selected transactions in w_info_text, clear selected
        // transaction balance var
        m_info_panel_selectedbal.clear();
        w_info_text->SetLabelText(notesStr);
    }
    else /* !single */ {
        w_mini_text->SetLabelText("");
        const auto selected_a = w_list->getSelectKeyA();
        if (selected_a.size() > 0) {
            bool selected_bill = false;
            for (const auto& journal_key : selected_a)
                if (journal_key.is_scheduled()) {
                    selected_bill = true;
                    break;
                }

            enableButtons(
                /* Edit      */ true,
                /* Duplicate */ false,
                /* Delete    */ !selected_bill,
                /* Enter     */ false,
                /* Skip      */ false,
                /* attach    */ false
            );

            double flow = 0;
            mmDateN min_dateN;
            mmDateN max_dateN;
            long item = -1;
            while (true) {
                item = w_list->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
                if (item == -1) break;
                const CurrencyData* curr = AccountModel::instance().get_id_currency_p(
                    w_list->m_journal_xa[item].m_account_id
                );
                if (m_account_id < 0 && w_list->m_journal_xa[item].is_transfer())
                    continue;
                double convrate = (curr != m_currency_n)
                    ? CurrencyHistoryModel::instance().get_id_date_rate(
                        curr->m_id,
                        w_list->m_journal_xa[item].m_date()
                    )
                    : 1.0;
                flow += convrate * w_list->m_journal_xa[item].account_flow(
                    (m_account_id < 0) ? w_list->m_journal_xa[item].m_account_id : m_account_id
                );
                mmDate date = w_list->m_journal_xa[item].m_date();
                if (!min_dateN.has_value() || date < min_dateN.value())
                    min_dateN = date;
                if (!max_dateN.has_value() || max_dateN.value() < date)
                    max_dateN = date;
            }

            mmDate min_date = min_dateN.value();
            int days = max_dateN.value().daysSince(min_date);

            wxString msg;
            wxString selectedBal = CurrencyModel::instance().toCurrency(flow, m_currency_n);
            m_info_panel_selectedbal = selectedBal;
            msg = wxString::Format(_t("Transactions selected: %zu"), selected_a.size());
            msg += "\n";
            msg += wxString::Format(
                    _t("Selected transactions total: %s"),
                    selectedBal
                );
            msg += "\n";
            msg += wxString::Format(
                _t("Days between selected transactions: %d"),
                days
            );
#ifdef __WXMAC__    // See issue #2914
            msg = "";
#endif
            w_info_text->SetLabelText(msg);
        }
        else /* selected.size() == 0 */ {
            enableButtons(false, false, false, false, false, false);
            showTips();
        }
    }
}

void JournalPanel::enableButtons(
    bool edit, bool dup, bool del, bool enter, bool skip, bool attach
) {
    if (!isDeletedTrans()) {
        w_edit_btn->Enable(edit);
        w_dup_btn->Enable(dup);
        w_delete_btn->Enable(del);
        w_enter_btn->Enable(enter);
        w_skip_btn->Enable(skip);
        w_attachment_btn->Enable(attach);
    }
    else {
        w_restore_btn->Enable(edit);
        w_delete_btn->Enable(del);
    }
}

void JournalPanel::showTips(const wxString& tip)
{
    // Not displaying any selected transactions in w_info_text,
    // clear selected transaction balance var
    m_info_panel_selectedbal.clear();

    w_info_text->SetLabelText(
        PrefModel::instance().getShowMoneyTips()
            ? tip
            : ""
    );
    m_show_tips = true;
}

void JournalPanel::showTips()
{
    // Not displaying any selected transactions in w_info_text,
    // clear selected transaction balance var
    m_info_panel_selectedbal.clear();

    if (m_show_tips) {
        m_show_tips = false;
        return;
    }

    wxString tip = PrefModel::instance().getShowMoneyTips()
        ? wxGetTranslation(wxString::FromUTF8(
            mmTips[rand() % (sizeof(mmTips) / sizeof(wxString))].ToStdString()
        ))
        : "";
    w_info_text->SetLabelText(tip);
}

void JournalPanel::datePickProceed() {
    m_filter_id = FILTER_ID_DATE_PICKER;
    updateFilter();
    saveFilterSettings();
    refreshList();
}

void JournalPanel::displaySplitCategories(JournalKey journal_key)
{
    Journal::Data journal = Journal::get_id_data(journal_key);
    std::vector<Split> splits;
    for (const auto& tp_d : Journal::split(journal)) {
        Split split_d;
        split_d.m_category_id = tp_d.m_category_id;
        split_d.m_amount      = tp_d.m_amount;
        split_d.m_notes       = tp_d.m_notes;
        // add tags
        for (const TagLinkData& gl_d : TagLinkModel::instance().find_data_a(
            //TagLinkCol::WHERE_REFTYPE(OP_EQ, split_ref_type.key_n()),
            TagLinkCol::WHERE_REFID(OP_EQ, tp_d.m_id))
        ) {
            split_d.m_tag_id_a.push_back(gl_d.m_tag_id);
        }
        splits.push_back(split_d);
    }
    if (splits.empty()) return;
    SplitDialog splitTransDialog(this, splits, m_account_id, true);

    //splitTransDialog.SetDisplaySplitCategories();
    splitTransDialog.ShowModal();
}

// -- event handlers

void JournalPanel::onDateRangePopup(wxCommandEvent& event)
{
    wxMenu menu;
    int i = 0;
    while (i < m_date_range_m) {
        menu.Append(ID_DATE_RANGE_MIN + i, m_date_range_a[i].getName());
        i++;
    }

    // add separator if there are more entries
    if (i + 1 < static_cast<int>(m_date_range_a.size())) {
        menu.AppendSeparator();
    }
    if (i < static_cast<int>(m_date_range_a.size())) {
        wxMenu* menu_more(new wxMenu);
        menu.AppendSubMenu(menu_more, _tu("More date ranges…"));
        while (i < static_cast<int>(m_date_range_a.size())) {
            menu_more->Append(ID_DATE_RANGE_MIN + i, m_date_range_a[i].getName());
            i++;
        }
    }
    menu.AppendSeparator();
    menu.Append(ID_DATE_RANGE_EDIT, _tu("Edit date ranges…"));

    PopupMenu(&menu);
    w_range_btn->Layout();
    event.Skip();
}

void JournalPanel::onDateRangeSelect(wxCommandEvent& event)
{
    int i = event.GetId() - ID_DATE_RANGE_MIN;
    if (i < 0 || i >= static_cast<int>(m_date_range_a.size()))
        return;

    // recreate m_date_range in order to reload parameters from setting,
    // refresh the date of today, and clear the default start/end dates
    m_date_range = mmDateRange2();
    m_date_range.setRange(m_date_range_a[i]);
    if (isAccount()) {
        m_date_range.setSDateN(m_account_n->m_stmt_date_n);
    }

    m_filter_id = FILTER_ID_DATE_RANGE;
    updateFilter();
    saveFilterSettings();
    refreshList();
}

void JournalPanel::onStartDateChanged(wxDateEvent& event) {
    // Check if low date < = high date
    if (w_end_date->GetValue().IsValid() &&
        event.GetDate().IsLaterThan(w_end_date->GetValue())
    ) {
        wxLogDebug("onStartDateChanged to Date =>: %s", event.GetDate().FormatISODate());
        w_end_date->SetValue(event.GetDate());
    }
    datePickProceed();
}

void JournalPanel::onEndDateChanged(wxDateEvent& event) {
    // Check if high date <= low date
    if (w_start_date->GetValue().IsValid() &&
        event.GetDate().IsEarlierThan(w_start_date->GetValue())
    ) {
        w_start_date->SetValue(event.GetDate());
    }
    datePickProceed();
}

void JournalPanel::onFilter(wxCommandEvent& WXUNUSED(event))
{
    wxString j_str = InfoModel::instance().getString(
        wxString::Format("CHECK_FILTER_ID_ADV_%lld", m_account_group_id),
        "{}"
    );
    w_filter_dlg.reset(
        new TrxFilterDialog(this, m_account_group_id, false, j_str)
    );
    w_filter_dlg->ShowModal();

    loadFilter();
}

void JournalPanel::onFilterCancel(wxCommandEvent& WXUNUSED(event))
{
    InfoModel::instance().saveString(
        wxString::Format("CHECK_FILTER_ID_ADV_%lld", m_account_group_id),
        ""
    );

    loadFilter();
}

void JournalPanel::onDateRangeEdit(wxCommandEvent& WXUNUSED(event))
{
    DateRangeManager dlg(this, DateRangeManager::TYPE_ID_CHECKING);
    if (dlg.ShowModal() != wxID_OK)
        return;

    loadDateRanges(&m_date_range_a, &m_date_range_m, isAccount());

    if (m_filter_id == FILTER_ID_DATE_RANGE) {
        // recreate m_date_range in order to reload parameters from setting,
        // refresh the date of today, and clear the default start/end dates
        m_date_range = mmDateRange2();

        // find and reload the range specification (it may have been changed)
        bool found = false;
        for (const auto& range : m_date_range_a) {
            if (range.getName() == m_date_range.rangeName()) {
                m_date_range.setRange(range);
                found = true;
                break;
            }
        }
        if (!found) {
            // set range to 'All'
            m_date_range.setRange(m_date_range_a[0]);
        }
        if (isAccount()) {
            m_date_range.setSDateN(m_account_n->m_stmt_date_n);
        }

        updateFilter();
        saveFilterSettings();
    }
}

void JournalPanel::onScheduled(wxCommandEvent&)
{
    if (!isDeletedTrans()) {
        m_scheduled_selected = w_header_scheduled->GetValue();
        updateScheduledToolTip();
        saveFilterSettings();
    }
    refreshList();
}

void JournalPanel::onSearchTxtEntered(wxCommandEvent& event)
{
    const wxString search_string = event.GetString();
    if (search_string.IsEmpty())
        return;
    w_list->doSearchText(search_string);
}

void JournalPanel::onButtonRightDown(wxMouseEvent& event)
{
    int id = event.GetId();
    switch (id)
    {
    case ID_DATE_RANGE_BUTTON: {
        wxCommandEvent evt(wxID_ANY, ID_FILTER_ADVANCED);
        onFilter(evt);
        break;
    }
    case wxID_FILE: {
        auto selected_a = w_list->getSelectKeyA();
        if (selected_a.size() == 1) {
            AttachmentDialog dlg(this,
                selected_a[0].ref_type(), selected_a[0].ref_id()
            );
            dlg.ShowModal();
            refreshList();
        }
        break;
    }
    case wxID_NEW: {
        wxMenu menu;
        menu.Append(TrxType::e_withdrawal, _tu("&New Withdrawal…"));
        menu.Append(TrxType::e_deposit,    _tu("&New Deposit…"));
        menu.Append(TrxType::e_transfer,   _tu("&New Transfer…"));
        PopupMenu(&menu);
    }
    default:
        break;
    }
}

void JournalPanel::onInfoPanelClick(wxMouseEvent& event, wxStaticText* infoPanel)
{
    wxString clipboardValue = !m_info_panel_selectedbal.IsEmpty()
        ? m_info_panel_selectedbal
        : infoPanel->GetLabel();

    if (!clipboardValue.IsEmpty()) {
        // Copy to clipboard
        if (wxTheClipboard->Open()) {
            wxTheClipboard->SetData(new wxTextDataObject(clipboardValue));
            wxTheClipboard->Close();
            this->Layout();
        }
        m_info_panel_selectedbal.empty();
    }

    event.Skip();
}

void JournalPanel::onReconcile(wxCommandEvent& WXUNUSED(event))
{
    ReconcileDialog dlg(wxGetTopLevelParent(this), m_account_n, this);
    if (dlg.ShowModal() == wxID_OK) {
        refreshList();
    }
}
