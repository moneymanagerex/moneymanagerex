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

#include "base/defs.h"
#include <float.h>
#include <algorithm>
#include <wx/clipbrd.h>
#include <wx/srchctrl.h>
#include <wx/sound.h>

#include "base/constants.h"
#include "mmex.h"
#include "base/paths.h"
#include "base/images_list.h"
#include "util/_util.h"
#include "util/_simple.h"
#include "util/mmTips.h"
#include "util/mmCalcValidator.h"

#include "model/_all.h"
#include "model/Journal.h"

#include "mmframe.h"
#include "JournalPanel.h"
#include "JournalList.h"

#include "manager/DateRangeManager.h"
#include "dialog/SplitDialog.h"
#include "dialog/TrxDialog.h"
#include "dialog/AttachmentDialog.h"
#include "dialog/AssetDialog.h"
#include "dialog/SchedDialog.h"
#include "dialog/TrxFilterDialog.h"
#include "dialog/TrxShareDialog.h"
#include "dialog/TrxUpdateDialog.h"
#include "uicontrols/reconciledialog.h"
#include "uicontrols/navigatortypes.h"

//----------------------------------------------------------------------------

const std::vector<std::pair<JournalPanel::FILTER_ID, wxString> > JournalPanel::FILTER_NAME =
{
    { JournalPanel::FILTER_ID_DATE,        wxString("Date") },
    { JournalPanel::FILTER_ID_DATE_RANGE,  wxString("mmDateRange") },
    { JournalPanel::FILTER_ID_DATE_PICKER, wxString("DatePicker") }
};

//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(JournalPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,           JournalPanel::onNewTransaction)
    EVT_BUTTON(wxID_EDIT,          JournalPanel::onEditTransaction)
    EVT_BUTTON(wxID_DUPLICATE,     JournalPanel::onDuplicateTransaction)
    EVT_BUTTON(wxID_UNDELETE,      JournalPanel::onRestoreTransaction)
    EVT_BUTTON(wxID_REMOVE,        JournalPanel::onDeleteTransaction)
    EVT_BUTTON(wxID_PASTE,         JournalPanel::onEnterScheduled)
    EVT_BUTTON(wxID_IGNORE,        JournalPanel::onSkipScheduled)
    EVT_BUTTON(wxID_FILE,          JournalPanel::onOpenAttachment)
    EVT_BUTTON(ID_FILTER,          JournalPanel::onFilterPopup)
    EVT_BUTTON(ID_FILTER_TRANS,    JournalPanel::onFilterAdvanced)
    EVT_MENU(ID_FILTER_ADVANCED,   JournalPanel::onFilterAdvanced)
    EVT_MENU(ID_DATE_RANGE_EDIT,   JournalPanel::onEditDateRanges)
    EVT_TOGGLEBUTTON(ID_SCHEDULED, JournalPanel::onScheduled)
    EVT_MENU_RANGE(
        ID_DATE_RANGE_MIN,
        ID_DATE_RANGE_MAX,
        JournalPanel::onFilterDate)
    EVT_MENU_RANGE(
        TrxType::e_withdrawal,
        TrxType::e_transfer,
        JournalPanel::onNewTransaction
    )
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_FIND,  JournalPanel::onSearchTxtEntered)
    EVT_DATE_CHANGED(ID_DATE_PICKER_LOW,  JournalPanel::onDatePickLow)
    EVT_DATE_CHANGED(ID_DATE_PICKER_HIGH, JournalPanel::onDatePickHigh)
    wxEND_EVENT_TABLE()

//----------------------------------------------------------------------------

JournalPanel::JournalPanel(
    mmGUIFrame* frame,
    wxWindow* parent,
    int64 checking_id,
    const std::vector<int64>& group_ids // = {}
) :
    m_checking_id(checking_id),
    m_frame(frame)
{
    if (isAccount()) {
        m_account_id = m_checking_id;
        m_account_n = AccountModel::instance().get_id_data_n(m_account_id);
        m_currency_n = AccountModel::instance().get_data_currency_p(*m_account_n);
    }
    else if (isGroup()) {
        m_group_ids = std::set<int64>(group_ids.begin(), group_ids.end());
        m_currency_n = CurrencyModel::instance().get_base_data_n();
    }
    else {
        m_currency_n = CurrencyModel::instance().get_base_data_n();
    }
    m_use_account_specific_filter = PrefModel::instance().getUsePerAccountFilter();
    loadDateRanges(&m_date_range_a, &m_date_range_m, isAccount());

    create(parent);
    mmThemeAutoColour(this);
    Fit();
}

// We cannot use OnClose() event because this class deletes via DestroyChildren() of its parent.
JournalPanel::~JournalPanel()
{
}

bool JournalPanel::create(
    wxWindow* parent,
    const wxPoint& pos, const wxSize& size,
    long style, const wxString& name
) {
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    if (!wxPanel::Create(parent, mmID_CHECKING, pos, size, style, name))
        return false;

    this->windowsFreezeThaw();

    createControls();
    setFilterAdvanced(true);

    this->windowsFreezeThaw();
    UsageModel::instance().pageview(this);
    return true;
}

// Refresh account screen with new details
void JournalPanel::loadAccount(int64 account_id)
{
    wxASSERT (account_id >= 1);

    m_lc->setVisibleItemIndex(-1);
    m_checking_id = account_id;
    m_account_id = account_id;
    m_group_ids = {};
    m_account_n = AccountModel::instance().get_id_data_n(m_account_id);
    m_currency_n = AccountModel::instance().get_data_currency_p(*m_account_n);
    m_use_account_specific_filter = PrefModel::instance().getUsePerAccountFilter();

    loadFilterSettings();
    updateFilter();
    updateFilterTooltip();

    //refreshList();
    showTips();
    enableButtons(false, false, false, false, false, false);
}

//----------------------------------------------------------------------------

void JournalPanel::createControls()
{
    wxBoxSizer* sizerV = new wxBoxSizer(wxVERTICAL);
    //this->SetSizer(sizerV);

    /* ---------------------- */

    wxFlexGridSizer* sizerVHeader = new wxFlexGridSizer(0, 1, 0, 0);
    sizerVHeader->AddGrowableCol(0, 0);
    sizerV->Add(sizerVHeader, g_flagsBorder1V);

    m_header_text = new wxStaticText(this, wxID_STATIC, "");
    m_header_text->SetFont(this->GetFont().Larger().Bold());
    sizerVHeader->Add(m_header_text, g_flagsExpandBorder1);

    wxBoxSizer* sizerHInfo = new wxBoxSizer(wxHORIZONTAL);
    m_header_balance = new wxStaticText(this, wxID_STATIC, "");
    sizerHInfo->Add(m_header_balance, g_flagsH);
    m_header_credit = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(100,-1));
    sizerHInfo->Add(m_header_credit, g_flagsH);
    sizerVHeader->Add(sizerHInfo, g_flagsBorder1V);

    wxBoxSizer* sizerHCtrl = new wxBoxSizer(wxHORIZONTAL);
    m_bitmapTransFilter = new wxButton(this, ID_FILTER);
    m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
    sizerHCtrl->Add(m_bitmapTransFilter, g_flagsH);

    m_bitmapTransFilter->SetMinSize(wxSize(200 + PrefModel::instance().getIconSize() * 2, -1));

    mmDateRange2 tmprange = mmDateRange2();
    tmprange.setRange(m_date_range_a[0]);  // set to all

    fromDateCtrl = new wxDatePickerCtrl(this, ID_DATE_PICKER_LOW, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN|wxDP_ALLOWNONE);
    fromDateCtrl->SetValue(
        tmprange.rangeStart().value_or(mmDate::min()).getDateTime()
    );
    fromDateCtrl->SetRange(wxInvalidDateTime, wxDateTime::Now());
    sizerHCtrl->Add(fromDateCtrl, g_flagsH);

    toDateCtrl = new wxDatePickerCtrl(this, ID_DATE_PICKER_HIGH, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN|wxDP_ALLOWNONE);
    toDateCtrl->SetValue(
        tmprange.rangeEnd().value_or(mmDate::today()).getDateTime()
    );
    sizerHCtrl->Add(toDateCtrl, g_flagsH);

    // Filter for transaction details
    m_btnTransDetailFilter = new wxButton(this, ID_FILTER_TRANS, _tu("Filter…"));
    m_btnTransDetailFilter->SetMinSize(wxSize(150 + PrefModel::instance().getIconSize() * 2, -1));
    sizerHCtrl->Add(m_btnTransDetailFilter, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxUP | wxDOWN | wxLEFT, 5);

    m_btnTransDetailFilterCancel = new wxBitmapButton(this, wxID_ANY, mmBitmapBundle(png::CLEAR, mmBitmapButtonSize));
    mmToolTip(m_btnTransDetailFilterCancel, _t("Reset filter"));
    m_btnTransDetailFilterCancel->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &JournalPanel::onFilterAdvancedCancel, this);
    sizerHCtrl->Add(m_btnTransDetailFilterCancel, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);

    if (!isDeletedTrans()) {
        sizerHCtrl->AddSpacer(15);
        const auto& size = m_bitmapTransFilter->GetSize().GetY();
        m_header_scheduled = new wxBitmapToggleButton(
            this, ID_SCHEDULED, mmBitmapBundle(png::RECURRING),
            wxDefaultPosition, wxSize(size, size)
        );
        sizerHCtrl->Add(m_header_scheduled, g_flagsH);
        sizerHCtrl->AddSpacer(10);
    }
    m_header_sortOrder = new wxStaticText(this, wxID_STATIC, "");
    sizerHCtrl->Add(m_header_sortOrder, g_flagsH);

    if (isAccount()) {
        sizerHCtrl->AddStretchSpacer(1);
        sizerHCtrl->AddSpacer(100);

        wxBitmapButton* btn = new wxBitmapButton(this, wxID_ANY, mmBitmapBundle(png::TRXNUM, mmBitmapButtonSize));
        mmToolTip(btn, _t("Reconcile"));
        btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &JournalPanel::onReconcile, this);
        sizerHCtrl->Add(btn, 0, wxALIGN_CENTER_VERTICAL, 5);
    }

    sizerV->Add(sizerHCtrl, 0, wxEXPAND | wxALL, 10);

    m_bitmapTransFilter->Connect(wxEVT_RIGHT_DOWN,
        wxMouseEventHandler(JournalPanel::onButtonRightDown), nullptr, this);

    /* ---------------------- */

    mmSplitterWindow* splitterListFooter = new mmSplitterWindow(
        this, wxID_ANY, wxDefaultPosition, wxSize(200, 200),
        wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER, mmThemeMetaColour(meta::COLOR_LISTPANEL)
    );

    m_images.push_back(mmBitmapBundle(png::UNRECONCILED));
    m_images.push_back(mmBitmapBundle(png::RECONCILED));
    m_images.push_back(mmBitmapBundle(png::VOID_STAT));
    m_images.push_back(mmBitmapBundle(png::FOLLOW_UP));
    m_images.push_back(mmBitmapBundle(png::DUPLICATE_STAT));
    m_images.push_back(mmBitmapBundle(png::UPARROW));
    m_images.push_back(mmBitmapBundle(png::DOWNARROW));

    m_lc = new JournalList(this, splitterListFooter);
    m_lc->SetSmallImages(m_images);
    m_lc->SetNormalImages(m_images);

    wxPanel* panelFooter = new wxPanel(
        splitterListFooter, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxNO_BORDER | wxTAB_TRAVERSAL
    );

    splitterListFooter->SplitHorizontally(m_lc, panelFooter);
    splitterListFooter->SetMinimumPaneSize(100);
    splitterListFooter->SetSashGravity(1.0);

    sizerV->Add(splitterListFooter, g_flagsExpandBorder1);

    this->SetSizerAndFit(sizerV);

    wxBoxSizer* sizerVFooter = new wxBoxSizer(wxVERTICAL);
    panelFooter->SetSizer(sizerVFooter);

    wxBoxSizer* sizerHButtons = new wxBoxSizer(wxHORIZONTAL);
    sizerVFooter->Add(sizerHButtons, wxSizerFlags(g_flagsExpandBorder1).Proportion(0));

    m_btnDelete = new wxButton(panelFooter, wxID_REMOVE, _t("&Delete "));
    mmToolTip(m_btnDelete, _t("Delete all selected transactions"));

    if (!isDeletedTrans()) {
        m_btnNew = new wxButton(panelFooter, wxID_NEW, _t("&New "));
        mmToolTip(m_btnNew, _t("New Transaction"));
        sizerHButtons->Add(m_btnNew, 0, wxRIGHT, 2);

        m_btnEdit = new wxButton(panelFooter, wxID_EDIT, _t("&Edit "));
        mmToolTip(m_btnEdit, _t("Edit all selected transactions"));
        sizerHButtons->Add(m_btnEdit, 0, wxRIGHT, 2);
        m_btnEdit->Enable(false);

        m_btnDuplicate = new wxButton(panelFooter, wxID_DUPLICATE, _t("D&uplicate "));
        mmToolTip(m_btnDuplicate, _t("Duplicate selected transaction"));
        sizerHButtons->Add(m_btnDuplicate, 0, wxRIGHT, 2);
        m_btnDuplicate->Enable(false);

        sizerHButtons->Add(m_btnDelete, 0, wxRIGHT, 2);
        m_btnDelete->Enable(false);

        m_btnEnter = new wxButton(panelFooter, wxID_PASTE, _t("Ente&r"));
        mmToolTip(m_btnEnter, _t("Enter Next Scheduled Transaction Occurrence"));
        sizerHButtons->Add(m_btnEnter, 0, wxRIGHT, 2);
        m_btnEnter->Enable(false);

        m_btnSkip = new wxButton(panelFooter, wxID_IGNORE, _t("&Skip"));
        mmToolTip(m_btnSkip, _t("Skip Next Scheduled Transaction Occurrence"));
        sizerHButtons->Add(m_btnSkip, 0, wxRIGHT, 2);
        m_btnSkip->Enable(false);

        const auto& btnDupSize = m_btnDuplicate->GetSize();
        m_btnAttachment = new wxBitmapButton(
            panelFooter, wxID_FILE, mmBitmapBundle(png::CLIP), wxDefaultPosition,
            wxSize(30, btnDupSize.GetY())
        );
        mmToolTip(m_btnAttachment, _t("Open attachments"));
        sizerHButtons->Add(m_btnAttachment, 0, wxRIGHT, 5);
        m_btnAttachment->Enable(false);

        m_btnAttachment->Connect(
            wxEVT_RIGHT_DOWN,
            wxMouseEventHandler(JournalPanel::onButtonRightDown),
            nullptr, this
        );
        m_btnNew->Connect(
            wxEVT_RIGHT_DOWN,
            wxMouseEventHandler(JournalPanel::onButtonRightDown),
            nullptr, this
        );
    }
    else {
        m_btnRestore = new wxButton(panelFooter, wxID_UNDELETE, _t("&Restore "));
        mmToolTip(m_btnRestore, _t("Restore selected transaction"));
        sizerHButtons->Add(m_btnRestore, 0, wxRIGHT, 5);
        m_btnRestore->Enable(false);

        sizerHButtons->Add(m_btnDelete, 0, wxRIGHT, 5);
        m_btnDelete->Enable(false);
    }

    wxSearchCtrl* searchCtrl = new wxSearchCtrl(
        panelFooter,
        wxID_FIND, wxEmptyString, wxDefaultPosition,
        wxSize(100, m_btnDelete->GetSize().GetHeight()),
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
    m_info_panel_mini = new wxStaticText(panelFooter, wxID_STATIC, "");
    sizerHButtons->Add(m_info_panel_mini, 1, wxGROW | wxTOP | wxLEFT, 5);

    // Infobar
    m_info_panel = new wxStaticText(
        panelFooter, wxID_STATIC, "", wxDefaultPosition, wxSize(200, -1),
        wxTE_MULTILINE | wxTE_WORDWRAP
    );
    sizerVFooter->Add(m_info_panel, g_flagsExpandBorder1);
    mmToolTip(m_info_panel, _t("Click to copy to clipboard"));

    m_info_panel->Bind(wxEVT_LEFT_DOWN,
                       [this, infoPanel = m_info_panel](wxMouseEvent& event)
                       {
                           onInfoPanelClick(event, infoPanel);
                       });

    this->Fit();
    //Show tips when no any transaction selected
    showTips();
}

void JournalPanel::updateHeader()
{
    m_header_text->SetLabelText(getPanelTitle());
    m_header_credit->Hide();
    if (m_account_n) {
        wxString summary = wxString::Format("%s%s",
            _t("Account Bal: "),
            AccountModel::instance().value_number_currency(*m_account_n, m_balance)
        );
        if (m_show_reconciled) summary.Append(wxString::Format("     %s%s     %s%s",
            _t("Reconciled Bal: "),
            AccountModel::instance().value_number_currency(*m_account_n, m_reconciled_balance),
            _t("Diff: "),
            AccountModel::instance().value_number_currency(*m_account_n, m_balance - m_reconciled_balance)
        ));
        summary.Append(wxString::Format("     %s%s",
            _t("Filtered Flow: "),
            AccountModel::instance().value_number_currency(*m_account_n, m_flow)
        ));
        if (m_account_n->m_credit_limit != 0.0) {
            double limit = 100.0 * ((m_balance < 0.0) ? -m_balance / m_account_n->m_credit_limit : 0.0);
            summary.Append(wxString::Format("   %s %.1f%%",
                _t("Credit Limit:"),
                limit
            ));
            m_header_credit->SetRange(std::max(100.0, limit));
            m_header_credit->SetValue(limit);
            m_header_credit->Show();
        }
        if (AccountModel::type_id(*m_account_n) == NavigatorTypes::TYPE_ID_INVESTMENT ||
            AccountModel::type_id(*m_account_n) == NavigatorTypes::TYPE_ID_ASSET
        ) {
            std::pair<double, double> investment_bal = AccountModel::instance().get_data_investment_balance(*m_account_n);
            summary.Append(wxString::Format("     %s%s",
                _t("Market Value: "),
                AccountModel::instance().value_number_currency(*m_account_n, investment_bal.first)
            ));
            summary.Append(wxString::Format("     %s%s",
                _t("Invested: "),
                AccountModel::instance().value_number_currency(*m_account_n, investment_bal.second)
            ));
        }
        m_header_balance->SetLabelText(summary);
    }
    this->Layout();
}

void JournalPanel::updateFilter(bool firstinit)
{
    if (m_filter_id == FILTER_ID_DATE_RANGE) {
        m_bitmapTransFilter->SetLabel(m_current_date_range.rangeName());
        // Set active if other than 'all'
        m_bitmapTransFilter->SetBitmap(mmBitmapBundle(
            (m_current_date_range.rangeName() == m_date_range_a[0].getName())
                ? png::TRANSFILTER : png::TRANSFILTER_ACTIVE,
            mmBitmapButtonSize
        ));
        fromDateCtrl->SetValue(
            m_current_date_range.rangeStart().value_or(mmDate::min()).getDateTime()
        );
        toDateCtrl->SetValue(
            m_current_date_range.rangeEnd().value_or(mmDate::today()).getDateTime()
        );
    }
    else if (m_filter_id == FILTER_ID_DATE_PICKER) {
        m_bitmapTransFilter->SetLabel(_t("Date range"));
        m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER_ACTIVE, mmBitmapButtonSize));
        if (firstinit) {
            // FIXME: get[ST]Date are not the start and end date
            fromDateCtrl->SetValue(
                m_current_date_range.getSDateN().getDateTimeN()
            );
            toDateCtrl->SetValue(
                m_current_date_range.getTDate().getDateTime()
            );
        }
    }
    else if (firstinit) {
        m_current_date_range.setRange(m_date_range_a[0]); // init with 'all'
        m_bitmapTransFilter->SetLabel(m_current_date_range.rangeName());
        m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
    }

    m_btnTransDetailFilter->SetBitmap(mmBitmapBundle(m_filter_advanced ?
                                png::TRANSFILTER_ACTIVE : png::TRANSFILTER, mmBitmapButtonSize));

    m_btnTransDetailFilterCancel->Enable(m_filter_advanced);

    if (!isDeletedTrans()) {
        m_header_scheduled->SetValue(m_scheduled_selected);
        m_header_scheduled->Enable(m_scheduled_enable);
        updateScheduledToolTip();
    }
}

void JournalPanel::updateFilterTooltip()
{
    if (m_trans_filter_dlg && m_trans_filter_dlg->mmIsSomethingChecked()) {
        m_btnTransDetailFilter->SetToolTip(m_trans_filter_dlg->mmGetDescriptionToolTip());
    }
    else {
        m_btnTransDetailFilter->UnsetToolTip();
    }
}

void JournalPanel::setFilterDate(mmDateRange2::Range& range)
{
    m_filter_id = FILTER_ID_DATE;
    m_current_date_range = mmDateRange2();
    if (isAccount()) {
        m_current_date_range.setSDateN(m_account_n->m_stmt_date_n);
    }
    m_current_date_range.setRange(range);
    m_scheduled_enable = !isDeletedTrans() && m_current_date_range.rangeEnd().has_value();
    saveFilterSettings();
    updateFilter();
}

void JournalPanel::setFilterAdvanced(bool firstinit)
{
    loadFilterSettings();
    updateFilter(firstinit);
    updateFilterTooltip();
    refreshList();
}

//----------------------------------------------------------------------------

void JournalPanel::loadFilterSettings()
{
    Document j_doc;
    m_scheduled_selected = false;

    j_doc = InfoModel::instance().getJdoc(m_use_account_specific_filter ? wxString::Format("CHECK_FILTER_DEDICATED_%lld", m_checking_id) : "CHECK_FILTER_ALL", "{}");
    int fid = 0;

    if (JSON_GetIntValue(j_doc, "FILTER_ID", fid)) {
        m_filter_id = static_cast<FILTER_ID>(fid);
    } else
    {
        m_filter_id = FILTER_ID_DATE_RANGE;
        m_current_date_range.setRange(m_date_range_a[0]); // init with 'all'
    }
    if (m_filter_id == FILTER_ID_DATE_RANGE) {
        wxString j_filter;
        if (JSON_GetStringValue(j_doc, "FILTER_DATE", j_filter)) {
            // get range specification:
            bool notfound = true;
            for (const auto& range : m_date_range_a) {
                if (range.getName() == j_filter) {
                    m_current_date_range.setRange(range);
                    notfound = false;
                    break;
                }
            }
            if (notfound) {
                m_current_date_range.setRange(m_date_range_a[0]); // init with 'all'
            }
        }
        if (isAccount()) {
            m_current_date_range.setSDateN(m_account_n->m_stmt_date_n);
        }
    }
    else if (m_filter_id == FILTER_ID_DATE_PICKER) {
        wxString p_filter;
        wxDateTime newdate;
        wxString::const_iterator end;
        if (JSON_GetStringValue(j_doc, "FILTER_DATE_BEGIN", p_filter)) {
            // FIXME: setSDateN is the account statement date, not the start date
            m_current_date_range.setSDateN(
                newdate.ParseFormat(p_filter, "%Y-%m-%d", &end)
                ? mmDate(newdate)
                : mmDateN()
            );
        }
        if (JSON_GetStringValue(j_doc, "FILTER_DATE_END", p_filter)) {
            // FIXME: setTDate is the date of today, should not be changed here
            m_current_date_range.setTDate(
                newdate.ParseFormat(p_filter, "%Y-%m-%d", &end)
                ? mmDate(newdate)
                : mmDate::today()
            );
        }
    }

    if (!isDeletedTrans()) {
        JSON_GetBoolValue(j_doc, "SCHEDULED", m_scheduled_selected);
    }

    wxString j_str = InfoModel::instance().getString(
            wxString::Format("CHECK_FILTER_ID_ADV_%lld", m_checking_id),"{}");
    m_trans_filter_dlg.reset(new TrxFilterDialog(this, m_account_id, false, j_str));
    m_filter_advanced = m_trans_filter_dlg->mmIsSomethingChecked();
    updateScheduledEnable();
}

void  JournalPanel::updateScheduledEnable()
{
    // FIXME: get[ST]Date are not the start and end date
    m_scheduled_enable = !isDeletedTrans() && (
        m_filter_id == FILTER_ID_DATE_PICKER
        ? m_current_date_range.getTDate() >= mmDate::today()
        : !m_current_date_range.rangeEnd().has_value()
            || m_current_date_range.rangeEnd().value() >= mmDate::today()
    );
}

void JournalPanel::saveFilterSettings()
{
    wxString key = m_use_account_specific_filter ? wxString::Format("CHECK_FILTER_DEDICATED_%lld", m_checking_id) : "CHECK_FILTER_ALL";
    Document j_doc = InfoModel::instance().getJdoc(key, "{}");
    InfoModel::saveFilterInt(j_doc, "FILTER_ID", m_filter_id);
    InfoModel::saveFilterString(j_doc, "FILTER_NAME", FILTER_NAME[m_filter_id].second);
    InfoModel::saveFilterString(j_doc, "FILTER_DATE", m_current_date_range.getRange().getName());
    InfoModel::saveFilterString(j_doc, "FILTER_DATE_BEGIN", fromDateCtrl->GetValue().IsValid() ? fromDateCtrl->GetValue().FormatISODate() : "");
    InfoModel::saveFilterString(j_doc, "FILTER_DATE_END", toDateCtrl->GetValue().IsValid() ? toDateCtrl->GetValue().FormatISODate() : "");
    if (!isDeletedTrans()) {
        InfoModel::saveFilterBool(j_doc, "SCHEDULED", m_scheduled_selected);
    }
    InfoModel::instance().setJdoc(key, j_doc);

    key = wxString::Format("CHECK_FILTER_ID_ADV_%lld", m_checking_id);
    InfoModel::instance().setString(key, m_filter_advanced ? m_trans_filter_dlg->mmGetJsonSettings() : "{}");
}

//----------------------------------------------------------------------------

void JournalPanel::refreshList()
{
    m_lc->refreshVisualList();
}

void JournalPanel::filterList()
{
    m_lc->m_journal_xa.clear();

    int sn = 0; // sequence number
    m_flow = 0.0;
    m_balance = m_account_n ? m_account_n->m_open_balance : 0.0;
    m_reconciled_balance = m_today_reconciled_balance = m_balance;
    m_show_reconciled = false;

    static wxArrayString udfc_fields = FieldModel::UDFC_FIELDS();
    int64 udfc_id[5];
    FieldTypeN udfc_type[5];
    int udfc_scale[5];
    for (int i = 0; i < 5; i++) {
        // note: udfc_fields starts with ""
        wxString field = udfc_fields[i+1];
        udfc_id[i] = FieldModel::instance().get_udfc_id_n(TrxModel::s_ref_type, field);
        udfc_type[i] = FieldModel::instance().get_udfc_type_n(TrxModel::s_ref_type, field);
        udfc_scale[i] = FieldModel::getDigitScale(
            FieldModel::instance().get_udfc_properties_n(TrxModel::s_ref_type, field)
        );
    }

    auto trxId_fvA_m = FieldValueModel::instance().find_refType_mRefId(TrxModel::s_ref_type);
    auto schedId_fvA_m = FieldValueModel::instance().find_refType_mRefId(SchedModel::s_ref_type);

    bool ignore_future = PrefModel::instance().getIgnoreFutureTransactions();
    const wxString today_date = PrefModel::instance().UseTransDateTime() ?
        wxDateTime::Now().FormatISOCombined() :
        wxDateTime(23, 59, 59, 999).FormatISOCombined();

    const auto trx_a = m_account_n
        ? AccountModel::instance().find_id_trx_aBySN(m_account_n->m_id)
        : TrxModel::instance().find_all_aDateTimeId();
    const auto trxId_tpA_m = TrxSplitModel::instance().find_all_mTrxId();
    const auto trxId_glA_m = TagLinkModel::instance().find_refType_mRefId(
        TrxModel::s_ref_type
    );
    const auto refId_attA_m = AttachmentModel::instance().find_refType_mRefId(
        TrxModel::s_ref_type
    );

    wxString date_start_str, date_end_str;
    mmDate date_end = mmDate::today();
    date_end.addDateSpan(wxDateSpan::Days(30));
    if (m_filter_id == FILTER_ID_DATE_PICKER) {
        date_start_str = mmDateN(fromDateCtrl->GetValue()).value_or(mmDate::min()).isoStart();
        date_end_str = mmDateN(toDateCtrl->GetValue()).value_or(date_end).isoEnd();
    } else {
        date_start_str = m_current_date_range.rangeStartIsoStartN();
        // find last un-deleted transaction and use that if later than current date + 30 days
        for (auto it = trx_a.rbegin(); it != trx_a.rend(); ++it) {
            const TrxData* trx_n = &(*it);
            if (trx_n && (isDeletedTrans() || !trx_n->is_deleted())) {
                if (date_end < trx_n->m_date())
                    date_end = trx_n->m_date();
                // FIXME: early break
                break;
            }
        }
        date_end_str = m_current_date_range.rangeEnd()
            .value_or(date_end).isoEnd();
    }
    std::map<int64, SchedSplitModel::DataA> schedId_qpA_m;
    std::map<int64, TagLinkModel::DataA> schedId_glA_m;
    std::map<int64, AttachmentModel::DataA> schedId_attA_m;
    SchedModel::DataA sched_a;
    typedef std::tuple<
        int      /* i */,
        wxString /* date */,
        int      /* repeat_id */
    > bills_index_t;
    std::vector<bills_index_t> bills_index;
    if (m_scheduled_enable && m_scheduled_selected) {
        schedId_qpA_m = SchedSplitModel::instance().find_all_mSchedId();
        schedId_glA_m = TagLinkModel::instance().find_refType_mRefId(
            SchedModel::s_ref_type
        );
        schedId_attA_m = AttachmentModel::instance().find_refType_mRefId(
            SchedModel::s_ref_type
        );
        sched_a = m_account_n
            ? AccountModel::instance().find_id_sched_a(m_account_n->m_id)
            : SchedModel::instance().find_all();
        for (unsigned int i = 0; i < sched_a.size(); ++i) {
            int limit = 1000;  // this is enough for daily repetitions for one year
            auto dates = sched_a[i].unroll(date_end_str, limit);
            for (unsigned int repeat_id = 1; repeat_id <= dates.size(); ++repeat_id)
                bills_index.push_back({i, dates[repeat_id-1], repeat_id});
        }
        std::stable_sort(
            bills_index.begin(), bills_index.end(),
            [](const bills_index_t& a, const bills_index_t& b) -> bool {
                return std::get<1>(a) < std::get<1>(b);
            }
        );
    }

    auto trx_it = trx_a.begin();
    auto bills_it = bills_index.begin();
    while (trx_it != trx_a.end() || bills_it != bills_index.end()) {
        int bill_i = 0;
        wxString tran_date;
        int repeat_id = -1;
        TrxData bill_tran;
        const TrxData* trx_n = nullptr;

        if (trx_it != trx_a.end())
            tran_date = trx_it->m_date_time.isoDateTime();
        if (trx_it != trx_a.end() &&
            (bills_it == bills_index.end() || tran_date.Left(10) <= std::get<1>(*bills_it).Left(10))
        ) {
            trx_n = &(*trx_it);
            trx_it++;
        }
        else {
            bill_i = std::get<0>(*bills_it);
            tran_date = std::get<1>(*bills_it);
            repeat_id = std::get<2>(*bills_it);
            bill_tran = Journal::execute_bill(sched_a[bill_i], tran_date);
            trx_n = &bill_tran;
            bills_it++;
        }

        if (isGroup() &&
            m_group_ids.find(trx_n->m_account_id) == m_group_ids.end() &&
            m_group_ids.find(trx_n->m_to_account_id_n) == m_group_ids.end()
        )
            continue;
        if (isDeletedTrans() != trx_n->is_deleted())
            continue;
        if (ignore_future && tran_date > today_date)
            break;

        // update m_balance even if tran is filtered out
        double account_flow = 0.0;
        if (isAccount()) {
            // assertion: !trx_n->is_deleted()
            account_flow = trx_n->account_flow(m_account_id);
            m_balance += account_flow;
            if (trx_n->is_reconciled()) {
                m_reconciled_balance += account_flow;
                if (tran_date <= today_date)
                    m_today_reconciled_balance += account_flow;
            }
            else
                m_show_reconciled = true;
        }

        if (tran_date < date_start_str || tran_date > date_end_str)
            continue;

        Journal::DataExt journal_dx = (repeat_id < 0) ?
            Journal::DataExt(*trx_n, trxId_tpA_m, trxId_glA_m) :
            Journal::DataExt(sched_a[bill_i], tran_date, repeat_id, schedId_qpA_m, schedId_glA_m);

        bool expandSplits = false;
        if (m_filter_advanced) {
            int txnMatch = m_trans_filter_dlg->mmIsRecordMatches(*trx_n, journal_dx.m_tp_a);
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

        if (repeat_id < 0 && refId_attA_m.find(trx_n->m_id) != refId_attA_m.end()) {
            for (const auto& att_d : refId_attA_m.at(trx_n->m_id))
                journal_dx.ATTACHMENT_DESCRIPTION.Add(att_d.m_description);
        }
        else if (repeat_id > 0 && schedId_attA_m.find(journal_dx.m_sched_id) != schedId_attA_m.end()) {
            for (const auto& att_d : schedId_attA_m.at(journal_dx.m_sched_id))
                journal_dx.ATTACHMENT_DESCRIPTION.Add(att_d.m_description);
        }

        for (int i = 0; i < 5; i++) {
            journal_dx.UDFC_type[i] = FieldTypeN();
            journal_dx.UDFC_value[i] = -DBL_MAX;
        }

        if (repeat_id < 0 && trxId_fvA_m.find(trx_n->m_id) != trxId_fvA_m.end()) {
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
        else if (repeat_id > 0 && schedId_fvA_m.find(journal_dx.m_sched_id) != schedId_fvA_m.end()) {
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
            m_lc->m_journal_xa.push_back(journal_dx);
            if (isAccount())
                m_flow += account_flow;
            continue;
        }

        int splitIndex = 1;
        wxString tranTagnames = journal_dx.TAGNAMES;
        wxString tranDisplaySN = journal_dx.displaySN;
        wxString tranDisplayID = journal_dx.displayID;
        for (const auto& tp_d : journal_dx.m_tp_a) {
            if (m_filter_advanced) {
              if (!m_trans_filter_dlg->mmIsSplitRecordMatches<TrxSplitModel>(tp_d))
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
                !m_trans_filter_dlg->mmIsRecordMatches<TrxModel>(journal_split_dx, true) &&
                !m_trans_filter_dlg->mmIsRecordMatches<TrxModel>(journal_trx_dx, true)
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
            if (!tag_names.IsEmpty())
                journal_dx.TAGNAMES.Append(
                    (journal_dx.TAGNAMES.IsEmpty() ? "" : ", ") +
                    tag_names.Trim()
                );
            m_lc->m_journal_xa.push_back(journal_dx);
        }
    }
}

void JournalPanel::sortList()
{
    m_lc->sortList();
}

//----------------------------------------------------------------------------

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
        for (x = 0; x < m_lc->GetItemCount(); x++) {
            if (m_lc->GetItemState(x, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
                break;
        }

        Journal::DataExt journal_dx(m_lc->m_journal_xa[x]);
        wxString miniStr = journal_dx.info();
        //Show only first line but full string set as tooltip
        if (miniStr.Find("\n") > 1 && !miniStr.IsEmpty()) {
            m_info_panel_mini->SetLabelText(
                miniStr.substr(0, miniStr.Find("\n")) + wxString::FromUTF8Unchecked(" …")
            );
            mmToolTip(m_info_panel_mini, miniStr);
        }
        else {
            m_info_panel_mini->SetLabelText(miniStr);
            mmToolTip(m_info_panel_mini, miniStr);
        }

        wxString notesStr = journal_dx.m_notes;
        if (journal_dx.m_repeat_id < 0) {
            auto tp_a = TrxSplitModel::instance().find(
                TrxSplitCol::TRANSID(journal_dx.m_id)
            );
            for (const auto& tp_d : tp_a)
                if (!tp_d.m_notes.IsEmpty()) {
                    notesStr += notesStr.empty() ? "" : "\n";
                    notesStr += tp_d.m_notes;
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
            auto qp_a = SchedSplitModel::instance().find(
                SchedSplitCol::TRANSID(journal_dx.m_sched_id)
            );
            for (const auto& qp_d : qp_a)
                if (!qp_d.m_notes.IsEmpty()) {
                    notesStr += notesStr.empty() ? "" : "\n";
                    notesStr += qp_d.m_notes;
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
        m_info_panel_selectedbal.clear(); // Not displaying any selected transactions in m_info_panel, clear selected transaction balance var
        m_info_panel->SetLabelText(notesStr);
    }
    else /* !single */ {
        m_info_panel_mini->SetLabelText("");
        const auto selected_a = m_lc->getSelectedId();
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
            wxString maxDate;
            wxString minDate;
            long item = -1;
            while (true) {
                item = m_lc->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
                if (item == -1) break;
                const CurrencyData* curr = AccountModel::instance().get_id_currency_p(
                    m_lc->m_journal_xa[item].m_account_id
                );
                if (m_account_id < 0 && m_lc->m_journal_xa[item].is_transfer())
                    continue;
                double convrate = (curr != m_currency_n)
                    ? CurrencyHistoryModel::instance().get_id_date_rate(
                        curr->m_id,
                        m_lc->m_journal_xa[item].m_date()
                    )
                    : 1.0;
                flow += convrate * m_lc->m_journal_xa[item].account_flow(
                    (m_account_id < 0) ? m_lc->m_journal_xa[item].m_account_id : m_account_id
                );
                wxString transdate = m_lc->m_journal_xa[item].m_date_time.isoDateTime();
                if (minDate > transdate || minDate.empty())
                    minDate = transdate;
                if (maxDate < transdate || maxDate.empty())
                    maxDate = transdate;
            }

            wxDateTime min_date, max_date;
            min_date.ParseISODate(minDate);
            max_date.ParseISODate(maxDate);
            int days = max_date.Subtract(min_date).GetDays();

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
            m_info_panel->SetLabelText(msg);
        }
        else /* selected.size() == 0 */ {
            enableButtons(false, false, false, false, false, false);
            showTips();
        }
    }
}

void JournalPanel::enableButtons(bool edit, bool dup, bool del, bool enter, bool skip, bool attach)
{
    if (!isDeletedTrans()) {
        m_btnEdit->Enable(edit);
        m_btnDuplicate->Enable(dup);
        m_btnDelete->Enable(del);
        m_btnEnter->Enable(enter);
        m_btnSkip->Enable(skip);
        m_btnAttachment->Enable(attach);
    }
    else {
        m_btnRestore->Enable(edit);
        m_btnDelete->Enable(del);
    }
}

//----------------------------------------------------------------------------

void JournalPanel::showTips()
{
    m_info_panel_selectedbal.clear(); // Not displaying any selected transactions in m_info_panel, clear selected transaction balance var

    if (m_show_tips) {
        m_show_tips = false;
        return;
    }

    if (PrefModel::instance().getShowMoneyTips())
        m_info_panel->SetLabelText(
            wxGetTranslation(wxString::FromUTF8(
                mmTips[rand() % (sizeof(mmTips) / sizeof(wxString))]
                .ToStdString()
            ))
        );
    else
        m_info_panel->SetLabelText("");
}

void JournalPanel::showTips(const wxString& tip)
{
    m_info_panel_selectedbal.clear(); // Not displaying any selected transactions in m_info_panel, clear selected transaction balance var
    if (PrefModel::instance().getShowMoneyTips())
        m_info_panel->SetLabelText(tip);
    else
        m_info_panel->SetLabelText("");
    m_show_tips = true;
}

void JournalPanel::updateScheduledToolTip()
{
   mmToolTip(m_header_scheduled,
        !m_scheduled_enable ?
        _t("Scheduled transactions are not available, because the current filter ends in the past") : !m_scheduled_selected ? _t("Click to show scheduled transactions. This feature works best with filter choices that extend into the future (e.g., Current Month).") :
        _t("Click to hide scheduled transactions."));
}

//----------------------------------------------------------------------------

void JournalPanel::onFilterPopup(wxCommandEvent& event)
{
    wxMenu menu;
    int i = 0;
    while (i < m_date_range_m) {
        menu.Append(ID_DATE_RANGE_MIN + i, m_date_range_a[i].getName());
        i++;
    }

    if (i +1 < static_cast<int>(m_date_range_a.size())) { //only add separator if there are more entries
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
    m_bitmapTransFilter->Layout();
    event.Skip();
}

void JournalPanel::onFilterDate(wxCommandEvent& event)
{
    int i = event.GetId() - ID_DATE_RANGE_MIN;
    if (i < 0 || i >= static_cast<int>(m_date_range_a.size()))
        return;

    m_filter_id = FILTER_ID_DATE_RANGE;
    m_current_date_range = mmDateRange2();
    if (isAccount()) {
        m_current_date_range.setSDateN(m_account_n->m_stmt_date_n);
    }
    m_current_date_range.setRange(m_date_range_a[i]);
    updateScheduledEnable();
    saveFilterSettings();
    updateFilter();

    m_bitmapTransFilter->SetLabel(m_current_date_range.rangeName());
    m_bitmapTransFilter->SetBitmap(mmBitmapBundle((i > 0 ? png::TRANSFILTER_ACTIVE : png::TRANSFILTER), mmBitmapButtonSize));

    fromDateCtrl->SetValue(
        m_current_date_range.rangeStart().value_or(mmDate::min()).getDateTime()
    );
    toDateCtrl->SetValue(
        m_current_date_range.rangeEnd().value_or(mmDate::today()).getDateTime()
    );

    refreshList();
}

void JournalPanel::onDatePickLow(wxDateEvent& event) {
    // Check if low date < = high date
    if (toDateCtrl->GetValue().IsValid() && event.GetDate().IsLaterThan(toDateCtrl->GetValue())) {
        wxLogDebug("onDatePickLow to Date =>: %s", event.GetDate().FormatISODate());
        toDateCtrl->SetValue(event.GetDate());
    }
    datePickProceed();
}

void JournalPanel::onDatePickHigh(wxDateEvent& event) {
    // Check if high date <= low date
    if (fromDateCtrl->GetValue().IsValid() && event.GetDate().IsEarlierThan(fromDateCtrl->GetValue())) {
        fromDateCtrl->SetValue(event.GetDate());
    }
    datePickProceed();
}

void JournalPanel::datePickProceed() {
    m_bitmapTransFilter->SetLabel(_t("Date range"));
    m_filter_id = FILTER_ID_DATE_PICKER;
    // FIXME: setSDateN is the account statement date, not the start date
    m_current_date_range.setSDateN(
        mmDateN(fromDateCtrl->GetValue()).value_or(mmDate::min())
    );
    // FIXME: setTDate is the date of today, should not be changed here
    m_current_date_range.setTDate(
        mmDateN(toDateCtrl->GetValue())
            .value_or(mmDate(wxDateTime::Now().Add(wxDateSpan(0,0,0,30))))
    );

    m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER_ACTIVE, mmBitmapButtonSize));
    updateScheduledEnable();
    saveFilterSettings();
    updateFilter();
    refreshList();
}

void JournalPanel::onFilterAdvanced(wxCommandEvent& WXUNUSED(event))
{
    wxString j_str = InfoModel::instance().getString(
        wxString::Format("CHECK_FILTER_ID_ADV_%lld", m_checking_id),
        "{}"
    );
    m_trans_filter_dlg.reset(
        new TrxFilterDialog(this, m_checking_id, false, j_str)
    );
    m_trans_filter_dlg->ShowModal();
    setFilterAdvanced();
}

void JournalPanel::onFilterAdvancedCancel(wxCommandEvent& WXUNUSED(event))
{
    InfoModel::instance().setString(
        wxString::Format("CHECK_FILTER_ID_ADV_%lld", m_checking_id),
        ""
    );
    setFilterAdvanced();
}

void JournalPanel::onEditDateRanges(wxCommandEvent& WXUNUSED(event))
{
    DateRangeManager dlg(this, DateRangeManager::TYPE_ID_CHECKING);
    if (dlg.ShowModal() != wxID_OK)
        return;

    loadDateRanges(&m_date_range_a, &m_date_range_m, isAccount());

    // Verify if current filter is still valid otherwise reset to "ALL"
    if (m_filter_id == FILTER_ID_DATE_RANGE) {
        wxString curname = m_current_date_range.rangeName();
        bool isDeleted = true;
        for (const auto& range : m_date_range_a) {
            if (range.getName() == curname) {
                isDeleted = false;
                break;
            }
        }
        if (isDeleted) {
            wxCommandEvent evt = wxCommandEvent(wxEVT_NULL, ID_DATE_RANGE_MIN);
            onFilterDate(evt);
        }
    }
}

void JournalPanel::onScheduled(wxCommandEvent&)
{
    if (!isDeletedTrans()) {
        m_scheduled_selected = m_header_scheduled->GetValue();
        updateScheduledToolTip();
        saveFilterSettings();
    }
    refreshList();
}

//----------------------------------------------------------------------------

void JournalPanel::onNewTransaction(wxCommandEvent& event)
{
    m_lc->onNewTransaction(event);
}

void JournalPanel::onEditTransaction(wxCommandEvent& event)
{
    m_lc->onEditTransaction(event);
    m_lc->SetFocus();
}

void JournalPanel::onDeleteTransaction(wxCommandEvent& event)
{
    m_lc->onDeleteTransaction(event);
}

void JournalPanel::onRestoreTransaction(wxCommandEvent& event)
{
    m_lc->onRestoreTransaction(event);
}

void JournalPanel::onDuplicateTransaction(wxCommandEvent& event)
{
    m_lc->onDuplicateTransaction(event);
}

void JournalPanel::onMoveTransaction(wxCommandEvent& event)
{
    m_lc->onMoveTransaction(event);
}

void JournalPanel::onEnterScheduled(wxCommandEvent& event)
{
    m_lc->onEnterScheduled(event);
}

void JournalPanel::onSkipScheduled(wxCommandEvent& event)
{
    m_lc->onSkipScheduled(event);
}

void JournalPanel::onOpenAttachment(wxCommandEvent& event)
{
    m_lc->onOpenAttachment(event);
    m_lc->SetFocus();
}

void JournalPanel::onSearchTxtEntered(wxCommandEvent& event)
{
    const wxString search_string = event.GetString();
    if (search_string.IsEmpty()) return;
    m_lc->doSearchText(search_string);
}

void JournalPanel::onButtonRightDown(wxMouseEvent& event)
{
    int id = event.GetId();
    switch (id) {
    case ID_FILTER: {
        wxCommandEvent evt(wxID_ANY, ID_FILTER_ADVANCED);
        onFilterAdvanced(evt);
        break;
    }
    case wxID_FILE: {
        auto selected_a = m_lc->getSelectedId();
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
    wxString clipboardValue = "";
    if (!m_info_panel_selectedbal.IsEmpty()) {
        clipboardValue = m_info_panel_selectedbal;
    }
    else {
        clipboardValue = infoPanel->GetLabel();
    }
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
    mmReconcileDialog dlg(wxGetTopLevelParent(this), m_account_n, this);
    if (dlg.ShowModal() == wxID_OK) {
        refreshList();
    }
}

//----------------------------------------------------------------------------

wxString JournalPanel::getPanelTitle() const
{
    if (isAllTrans())
        return _t("All Transactions");
    else if (isDeletedTrans())
        return _t("Deleted Transactions");
    else if (isGroup()) {
        if (m_checking_id == -3)
            return _t("Favorites");
        else {
            int account_Type = -(static_cast<int>(m_checking_id.GetValue()) + 4);
            if (account_Type >= NavigatorTypes::TYPE_ID_size) {
                account_Type += NavigatorTypes::NAV_IDXDIFF;
            }
            return NavigatorTypes::instance().getAccountSectionName(account_Type);
        }
    }
    else if (m_account_n)
        return wxString::Format(_t("Account View: %s"), m_account_n->m_name);
    else
        return "";
}

wxString JournalPanel::BuildPage() const
{
    return m_lc->BuildPage((m_account_n ? getPanelTitle() : ""));
}

void JournalPanel::resetColumnView()
{
    m_lc->DeleteAllColumns();
    m_lc->setColumnsInfo();
    m_lc->createColumns();
    m_lc->refreshVisualList();
}

void JournalPanel::setSelectedTransaction(JournalKey journal_key)
{
    m_lc->setSelectedId(journal_key);
    refreshList();
    m_lc->SetFocus();
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
        splits.push_back(split_d);
    }
    if (splits.empty()) return;
    SplitDialog splitTransDialog(this, splits, m_account_id, true);

    //splitTransDialog.SetDisplaySplitCategories();
    splitTransDialog.ShowModal();
}

void JournalPanel::mmPlayTransactionSound()
{
    int play = SettingModel::instance().getInt(INIDB_USE_TRANSACTION_SOUND, 0);
    if (!play) return;

    wxString wav_path = mmex::getPathResource(
      (play == 2) ? mmex::TRANS_SOUND2 : mmex::TRANS_SOUND1
    );
    wxLogDebug("%s", wav_path);

    wxSound registerSound(wav_path);
    if (registerSound.IsOk())
        registerSound.Play(wxSOUND_ASYNC);
}


//--- static support function -----------------------------------------------------
wxString JournalPanel::getFilterName(FILTER_ID id) {
    return JournalPanel::FILTER_NAME[id].second;
}

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
