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

#include "filtertransdialog.h"
#include "mmcheckingpanel.h"
#include "mmchecking_list.h"
#include "fusedtransaction.h"
#include "paths.h"
#include "constants.h"
#include "images_list.h"
#include "util.h"
#include "mmex.h"
#include "mmframe.h"
#include "mmTips.h"
#include "mmSimpleDialogs.h"
#include "splittransactionsdialog.h"
#include "transdialog.h"
#include "validators.h"
#include "transactionsupdatedialog.h"
#include "attachmentdialog.h"
#include "model/allmodel.h"
#include "sharetransactiondialog.h"
#include "assetdialog.h"
#include "billsdepositsdialog.h"
#include "daterangedialog.h"
#include <wx/clipbrd.h>
#include <float.h>

#include <wx/srchctrl.h>
#include <algorithm>
#include <wx/sound.h>

//----------------------------------------------------------------------------

const std::vector<std::pair<mmCheckingPanel::FILTER_ID, wxString> > mmCheckingPanel::FILTER_NAME =
{
    { mmCheckingPanel::FILTER_ID_DATE,     wxString("Date") },
    { mmCheckingPanel::FILTER_ID_DATE_RANGE, wxString("DateRange") },
    { mmCheckingPanel::FILTER_ID_DATE_PICKER, wxString("DatePicker") },
};

//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(mmCheckingPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,             mmCheckingPanel::onNewTransaction)
    EVT_BUTTON(wxID_EDIT,            mmCheckingPanel::onEditTransaction)
    EVT_BUTTON(wxID_DUPLICATE,       mmCheckingPanel::onDuplicateTransaction)
    EVT_BUTTON(wxID_UNDELETE,        mmCheckingPanel::onRestoreTransaction)
    EVT_BUTTON(wxID_REMOVE,          mmCheckingPanel::onDeleteTransaction)
    EVT_BUTTON(wxID_PASTE,           mmCheckingPanel::onEnterScheduled)
    EVT_BUTTON(wxID_IGNORE,          mmCheckingPanel::onSkipScheduled)
    EVT_BUTTON(wxID_FILE,            mmCheckingPanel::onOpenAttachment)
    EVT_BUTTON(mmID_FILTER,          mmCheckingPanel::onFilterPopup)
    EVT_BUTTON(mmID_FILTER_TRANSACTION_DETAIL, mmCheckingPanel::onFilterAdvanced)
    EVT_MENU(mmID_FILTER_ADVANCED,   mmCheckingPanel::onFilterAdvanced)
    EVT_MENU(mmID_EDIT_DATE_RANGES,  mmCheckingPanel::onEditDateRanges)
    EVT_TOGGLEBUTTON(mmID_SCHEDULED, mmCheckingPanel::onScheduled)
    EVT_MENU_RANGE(
        mmID_FILTER_DATE_MIN,
        mmID_FILTER_DATE_MAX,
        mmCheckingPanel::onFilterDate)
    EVT_MENU_RANGE(
        Model_Checking::TYPE_ID_WITHDRAWAL,
        Model_Checking::TYPE_ID_TRANSFER,
        mmCheckingPanel::onNewTransaction
    )
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_FIND, mmCheckingPanel::onSearchTxtEntered)
    EVT_DATE_CHANGED(mmID_DATE_PICK_LOW,  mmCheckingPanel::onDatePickLow)
    EVT_DATE_CHANGED(mmID_DATE_PICK_HIGH,  mmCheckingPanel::onDatePickHigh)
    wxEND_EVENT_TABLE()

//----------------------------------------------------------------------------

mmCheckingPanel::mmCheckingPanel(
    mmGUIFrame *frame,
    wxWindow *parent,
    int64 checking_id,
    const std::vector<int64> &group_ids // = {}
) :
    m_checking_id(checking_id),
    m_frame(frame)
{
    if (isAccount()) {
        m_account_id = m_checking_id;
        m_account = Model_Account::instance().get(m_account_id);
        m_currency = Model_Account::currency(m_account);
    }
    else if (isGroup()) {
        m_account_type = -(m_checking_id.ToLong() + 4);
        m_group_ids = std::set<int64>(group_ids.begin(), group_ids.end());
        m_currency = Model_Currency::GetBaseCurrency();
    }
    else {
        m_currency = Model_Currency::GetBaseCurrency();
    }
    m_use_account_specific_filter = Option::instance().getUsePerAccountFilter();
    loadDateRanges(&m_date_range_a, &m_date_range_m, isAccount());

    create(parent);
    Fit();
}

// We cannot use OnClose() event because this class deletes via DestroyChildren() of its parent.
mmCheckingPanel::~mmCheckingPanel()
{
}

bool mmCheckingPanel::create(
    wxWindow* parent,
    const wxPoint& pos, const wxSize& size,
    long style, const wxString& name
) {
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    if (!wxPanel::Create(parent, mmID_CHECKING, pos, size, style, name))
        return false;

    this->windowsFreezeThaw();
    createControls();
    loadFilterSettings();
    updateFilter(true);
    updateFilterTooltip();

    refreshList();
    this->windowsFreezeThaw();
    Model_Usage::instance().pageview(this);
    return true;
}

// Refresh account screen with new details
void mmCheckingPanel::loadAccount(int64 account_id)
{
    wxASSERT (account_id >= 1);

    m_lc->setVisibleItemIndex(-1);
    m_checking_id = account_id;
    m_account_id = account_id;
    m_account_type = -1;
    m_group_ids = {};
    m_account = Model_Account::instance().get(m_account_id);
    m_currency = Model_Account::currency(m_account);
    m_use_account_specific_filter = Option::instance().getUsePerAccountFilter();

    loadFilterSettings();
    updateFilter();
    updateFilterTooltip();

    refreshList();
    showTips();
    enableButtons(false, false, false, false, false, false);
}

//----------------------------------------------------------------------------

void mmCheckingPanel::createControls()
{
    wxBoxSizer* sizerV = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizerV);

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
    m_bitmapTransFilter = new wxButton(this, mmID_FILTER);
    m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
    sizerHCtrl->Add(m_bitmapTransFilter, g_flagsH);

    m_bitmapTransFilter->SetMinSize(wxSize(200 + Option::instance().getIconSize() * 2, -1));

    DateRange2 tmprange = DateRange2();
    tmprange.setSpec(m_date_range_a[0]);  // set to all

    fromDateCtrl = new wxDatePickerCtrl(this, mmID_DATE_PICK_LOW, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN|wxDP_ALLOWNONE);
    fromDateCtrl->SetValue(tmprange.checking_start().IsValid() ? tmprange.checking_start() : DATE_MIN);
    fromDateCtrl->SetRange(wxInvalidDateTime, wxDateTime::Now());
    sizerHCtrl->Add(fromDateCtrl, g_flagsH);

    toDateCtrl = new wxDatePickerCtrl(this, mmID_DATE_PICK_HIGH, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN|wxDP_ALLOWNONE);
    toDateCtrl->SetValue(tmprange.checking_end().IsValid() ? tmprange.checking_end() : wxDateTime::Now());
    sizerHCtrl->Add(toDateCtrl, g_flagsH);

    m_btnTransDetailFilter = new wxButton(this, mmID_FILTER_TRANSACTION_DETAIL, _tu("Filter…"));  // Filter for transaction details
    m_btnTransDetailFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
    m_btnTransDetailFilter->SetMinSize(wxSize(150 + Option::instance().getIconSize() * 2, -1));
    sizerHCtrl->Add(m_btnTransDetailFilter, g_flagsH);

    if (!isDeletedTrans()) {
        sizerHCtrl->AddSpacer(15);
        const auto& size = m_bitmapTransFilter->GetSize().GetY();
        m_header_scheduled = new wxBitmapToggleButton(
            this, mmID_SCHEDULED, mmBitmapBundle(png::RECURRING),
            wxDefaultPosition, wxSize(size, size)
        );
        sizerHCtrl->Add(m_header_scheduled, g_flagsH);
        sizerHCtrl->AddSpacer(10);
    }
    m_header_sortOrder = new wxStaticText(this, wxID_STATIC, "");
    sizerHCtrl->Add(m_header_sortOrder, g_flagsH);
    sizerVHeader->Add(sizerHCtrl, g_flagsBorder1H);

    m_bitmapTransFilter->Connect(wxEVT_RIGHT_DOWN,
        wxMouseEventHandler(mmCheckingPanel::onButtonRightDown), nullptr, this);

    /* ---------------------- */

    wxSplitterWindow* splitterListFooter = new wxSplitterWindow(
        this, wxID_ANY, wxDefaultPosition, wxSize(200, 200),
        wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER
    );

    m_images.push_back(mmBitmapBundle(png::UNRECONCILED));
    m_images.push_back(mmBitmapBundle(png::RECONCILED));
    m_images.push_back(mmBitmapBundle(png::VOID_STAT));
    m_images.push_back(mmBitmapBundle(png::FOLLOW_UP));
    m_images.push_back(mmBitmapBundle(png::DUPLICATE_STAT));
    m_images.push_back(mmBitmapBundle(png::UPARROW));
    m_images.push_back(mmBitmapBundle(png::DOWNARROW));

    m_lc = new TransactionListCtrl(this, splitterListFooter);
    m_lc->SetSmallImages(m_images);
    m_lc->SetNormalImages(m_images);

    wxPanel* panelFooter = new wxPanel(
        splitterListFooter, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxNO_BORDER | wxTAB_TRAVERSAL
    );
    mmThemeMetaColour(panelFooter, meta::COLOR_LISTPANEL);

    splitterListFooter->SplitHorizontally(m_lc, panelFooter);
    splitterListFooter->SetMinimumPaneSize(100);
    splitterListFooter->SetSashGravity(1.0);

    sizerV->Add(splitterListFooter, g_flagsExpandBorder1);

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
            wxMouseEventHandler(mmCheckingPanel::onButtonRightDown),
            nullptr, this
        );
        m_btnNew->Connect(
            wxEVT_RIGHT_DOWN,
            wxMouseEventHandler(mmCheckingPanel::onButtonRightDown),
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
        _tu("Tips: Wildcard characters—question mark (?), asterisk (*)—can be used in search criteria.") + "\n" +
        _tu("Use the question mark (?) to find any single character—for example, “s?t” finds “sat” and “set”.") + "\n" +
        _tu("Use the asterisk (*) to find any number of characters—for example, “s*d” finds “sad” and “started”.") + "\n" +
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

    //Show tips when no any transaction selected
    showTips();
}

void mmCheckingPanel::updateHeader()
{
    m_header_text->SetLabelText(getPanelTitle());
    m_header_credit->Hide();
    if (m_account)
    {
        wxString summary = wxString::Format("%s%s",
            _t("Account Bal: "),
            Model_Account::toCurrency(m_balance, m_account)
        );
        if (m_show_reconciled) summary.Append(wxString::Format("     %s%s     %s%s",
            _t("Reconciled Bal: "),
            Model_Account::toCurrency(m_reconciled_balance, m_account),
            _t("Diff: "),
            Model_Account::toCurrency(m_balance - m_reconciled_balance, m_account)
        ));
        summary.Append(wxString::Format("     %s%s",
            _t("Filtered Flow: "),
            Model_Account::toCurrency(m_flow, m_account)
        ));
        if (m_account->CREDITLIMIT != 0.0) {
            double limit = 100.0 * ((m_balance < 0.0) ? -m_balance / m_account->CREDITLIMIT : 0.0);
            summary.Append(wxString::Format("   %s %.1f%%",
                _t("Credit Limit:"),
                limit
            ));
            m_header_credit->SetRange(std::max(100.0, limit));
            m_header_credit->SetValue(limit);
            m_header_credit->Show();
        }
        if (Model_Account::type_id(m_account) == Model_Account::TYPE_ID_INVESTMENT || Model_Account::type_id(m_account) == Model_Account::TYPE_ID_ASSET)
        {
            std::pair<double, double> investment_bal = Model_Account::investment_balance(m_account);
            summary.Append(wxString::Format("     %s%s", _t("Market Value: "), Model_Account::toCurrency(investment_bal.first, m_account)));
            summary.Append(wxString::Format("     %s%s", _t("Invested: "), Model_Account::toCurrency(investment_bal.second, m_account)));
        }
        m_header_balance->SetLabelText(summary);
    }
    this->Layout();
}

void mmCheckingPanel::updateFilter(bool firstinit)
{
    if (m_filter_id == FILTER_ID_DATE_RANGE) {
        m_bitmapTransFilter->SetLabel(m_current_date_range.getName());
        // Set active if other than 'all'
        m_bitmapTransFilter->SetBitmap(mmBitmapBundle((m_current_date_range.getName() == m_date_range_a[0].getName()) ? png::TRANSFILTER : png::TRANSFILTER_ACTIVE, mmBitmapButtonSize));
        fromDateCtrl->SetValue(m_current_date_range.checking_start().IsValid() ? m_current_date_range.checking_start() : DATE_MIN);
        toDateCtrl->SetValue(m_current_date_range.checking_end().IsValid() ? m_current_date_range.checking_end() : wxDateTime::Now());
    }
    else if (m_filter_id == FILTER_ID_DATE_PICKER) {
        m_bitmapTransFilter->SetLabel(_t("Date range"));
        m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER_ACTIVE, mmBitmapButtonSize));
        if (firstinit) {
            fromDateCtrl->SetValue(m_current_date_range.getDateS());
            toDateCtrl->SetValue(m_current_date_range.getDateT());
        }
    }
    else if (firstinit) {
        m_current_date_range.setSpec(m_date_range_a[0]); // init with 'all'
        m_bitmapTransFilter->SetLabel(m_current_date_range.getName());
        m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
    }

    m_btnTransDetailFilter->SetBitmap(mmBitmapBundle(m_filter_advanced ?
                                png::TRANSFILTER_ACTIVE : png::TRANSFILTER, mmBitmapButtonSize));

    if (!isDeletedTrans()) {
        m_header_scheduled->SetValue(m_scheduled_selected);
        m_header_scheduled->Enable(m_scheduled_enable);
        updateScheduledToolTip();
    }
}

void mmCheckingPanel::updateFilterTooltip()
{
    if (m_trans_filter_dlg && m_trans_filter_dlg->mmIsSomethingChecked()) {
        m_btnTransDetailFilter->SetToolTip(m_trans_filter_dlg->mmGetDescriptionToolTip());
    }
    else {
        m_btnTransDetailFilter->UnsetToolTip();
    }
}

void mmCheckingPanel::setFilterDate(DateRange2::Spec& spec)
{
    m_filter_id = FILTER_ID_DATE;
    m_current_date_range = DateRange2();
    if (isAccount()) m_current_date_range.setDateS(
        Model_Account::DateOf(m_account->STATEMENTDATE)
    );
    m_current_date_range.setSpec(spec);
    m_scheduled_enable = (!isDeletedTrans() &&
        m_current_date_range.checking_end() != wxInvalidDateTime
    );
    saveFilterSettings();
    updateFilter();
}

void mmCheckingPanel::setFilterAdvanced()
{
    loadFilterSettings();
    updateFilter();
    updateFilterTooltip();
    refreshList();
}

//----------------------------------------------------------------------------

void mmCheckingPanel::loadFilterSettings()
{
    Document j_doc;
    m_scheduled_selected = false;

    j_doc = Model_Infotable::instance().getJdoc(m_use_account_specific_filter ? wxString::Format("CHECK_FILTER_DEDICATED_%lld", m_checking_id) : "CHECK_FILTER_ALL", "{}");
    int fid = 0;

    if (JSON_GetIntValue(j_doc, "FILTER_ID", fid)) {
        m_filter_id = static_cast<FILTER_ID>(fid);
    } else
    {
        m_filter_id = FILTER_ID_DATE_RANGE;
        m_current_date_range.setSpec(m_date_range_a[0]); // init with 'all'
    }
    if (m_filter_id == FILTER_ID_DATE_RANGE) {
        wxString j_filter;
        if (JSON_GetStringValue(j_doc, "FILTER_DATE", j_filter)) {
            // get range spec:
            bool notfound = true;
            for (const auto& spec : m_date_range_a) {
                if (spec.getName() == j_filter) {
                    m_current_date_range.setSpec(spec);
                    notfound = false;
                    break;
                }
            }
            if (notfound) {
                m_current_date_range.setSpec(m_date_range_a[0]); // init with 'all'
            }
        }
        if (isAccount()) m_current_date_range.setDateS(Model_Account::DateOf(m_account->STATEMENTDATE));
    }
    else if (m_filter_id == FILTER_ID_DATE_PICKER) {
        wxString p_filter;
        wxDateTime newdate;
        wxString::const_iterator end;
        if (JSON_GetStringValue(j_doc, "FILTER_DATE_BEGIN", p_filter)) {
            m_current_date_range.setDateS(newdate.ParseFormat(p_filter, "%Y-%m-%d", &end) ? newdate : wxInvalidDateTime);
        }
        if (JSON_GetStringValue(j_doc, "FILTER_DATE_END", p_filter)) {
            m_current_date_range.setDateT(newdate.ParseFormat(p_filter, "%Y-%m-%d", &end) ? newdate : wxInvalidDateTime);
        }
    }

    if (!isDeletedTrans()) {
        JSON_GetBoolValue(j_doc, "SCHEDULED", m_scheduled_selected);
    }

    wxString j_str = Model_Infotable::instance().getString(
            wxString::Format("CHECK_FILTER_ID_ADV_%lld", m_checking_id),"{}");
    m_trans_filter_dlg.reset(new mmFilterTransactionsDialog(this, m_account_id, false, j_str));
    m_filter_advanced = m_trans_filter_dlg->mmIsSomethingChecked() ? true : false;
    updateScheduledEnable();
}

void  mmCheckingPanel::updateScheduledEnable()
{
    m_scheduled_enable = !(isDeletedTrans() ||
        (m_filter_id == FILTER_ID_DATE_PICKER ? m_current_date_range.getDateT().IsValid() && m_current_date_range.getDateT() < wxDateTime::Today() :
                                                m_current_date_range.checking_end().IsValid() && m_current_date_range.checking_end() < wxDateTime::Today())
                                            || (!m_current_date_range.getDateT().IsValid() && m_current_date_range.getDateS().IsValid() && m_current_date_range.getDateS() < wxDateTime::Today()));
}

void mmCheckingPanel::saveFilterSettings()
{
    wxString key = m_use_account_specific_filter ? wxString::Format("CHECK_FILTER_DEDICATED_%lld", m_checking_id) : "CHECK_FILTER_ALL";
    Document j_doc = Model_Infotable::instance().getJdoc(key, "{}");
    Model_Infotable::saveFilterInt(j_doc, "FILTER_ID", m_filter_id);
    Model_Infotable::saveFilterString(j_doc, "FILTER_NAME", FILTER_NAME[m_filter_id].second);
    Model_Infotable::saveFilterString(j_doc, "FILTER_DATE", m_current_date_range.getSpec().getName());
    Model_Infotable::saveFilterString(j_doc, "FILTER_DATE_BEGIN", fromDateCtrl->GetValue().IsValid() ? fromDateCtrl->GetValue().FormatISODate() : "");
    Model_Infotable::saveFilterString(j_doc, "FILTER_DATE_END", toDateCtrl->GetValue().IsValid() ? toDateCtrl->GetValue().FormatISODate() : "");
    if (!isDeletedTrans()) {
        Model_Infotable::saveFilterBool(j_doc, "SCHEDULED", m_scheduled_selected);
    }
    Model_Infotable::instance().setJdoc(key, j_doc);

    key = wxString::Format("CHECK_FILTER_ID_ADV_%lld", m_checking_id);
    Model_Infotable::instance().setString(key, m_filter_advanced ? m_trans_filter_dlg->mmGetJsonSettings() : "{}");
}

//----------------------------------------------------------------------------

void mmCheckingPanel::refreshList()
{
    m_lc->refreshVisualList();
}

void mmCheckingPanel::filterList()
{
    m_lc->m_trans.clear();

    int sn = 0; // sequence number
    m_flow = 0.0;
    m_balance = m_account ? m_account->INITIALBAL : 0.0;
    m_reconciled_balance = m_balance;
    m_show_reconciled = false;

    const wxString tranRefType = Model_Checking::refTypeName;
    const wxString billRefType = Model_Billsdeposits::refTypeName;
    const wxString tranSplitRefType = Model_Splittransaction::refTypeName;
    const wxString billSplitRefType = Model_Budgetsplittransaction::refTypeName;

    static wxArrayString udfc_fields = Model_CustomField::UDFC_FIELDS();
    int64 udfc_id[5];
    Model_CustomField::TYPE_ID udfc_type[5];
    int udfc_scale[5];
    for (int i = 0; i < 5; i++) {
        // note: udfc_fields starts with ""
        wxString field = udfc_fields[i+1];
        udfc_id[i] = Model_CustomField::getUDFCID(tranRefType, field);
        udfc_type[i] = Model_CustomField::getUDFCType(tranRefType, field);
        udfc_scale[i] = Model_CustomField::getDigitScale(
            Model_CustomField::getUDFCProperties(tranRefType, field)
        );
    }

    auto tranFieldData = Model_CustomFieldData::instance().get_all(Model_Checking::refTypeName);

    bool ignore_future = Option::instance().getIgnoreFutureTransactions();
    const wxString today_date = Option::instance().UseTransDateTime() ?
        wxDateTime::Now().FormatISOCombined() :
        wxDateTime(23, 59, 59, 999).FormatISOCombined();

    const auto trans = m_account ?
        Model_Account::transactionsByDateTimeId(m_account) :
        Model_Checking::instance().allByDateTimeId();
    const auto trans_splits = Model_Splittransaction::instance().get_all();
    const auto trans_tags = Model_Taglink::instance().get_all(tranRefType);
    const auto trans_attachments = Model_Attachment::instance().get_all(Model_Checking::refTypeName);

    wxString date_start_str, date_end_str;
    wxDateTime date_end = wxDateTime::Now() + wxTimeSpan::Days(30);
    if (m_filter_id == FILTER_ID_DATE_PICKER) {
        date_start_str = (fromDateCtrl->GetValue().IsValid() ? fromDateCtrl->GetValue() : wxDateTime(static_cast<time_t>(0))).FormatISODate();
        date_end_str = (toDateCtrl->GetValue().IsValid() ? toDateCtrl->GetValue() : date_end).FormatISODate() + "~";
    } else {
        date_start_str = m_current_date_range.checking_start_str();
        // find last un-deleted transaction and use that if later than current date + 30 days
        for (auto it = trans.rbegin(); it != trans.rend(); ++it)
        {
            const Model_Checking::Data* tran = &(*it);
            if (tran && ( isDeletedTrans() || tran->DELETEDTIME.IsEmpty()))
            {
                date_end = (Model_Checking::TRANSDATE(tran) > date_end) ? Model_Checking::TRANSDATE(tran) : date_end;
                break;
            }
        }
        date_end_str = m_current_date_range.checking_end().IsValid() ? m_current_date_range.checking_end_str() :
            date_end.FormatISODate() + "~";
    }
    std::map<int64, Model_Budgetsplittransaction::Data_Set> bills_splits;
    std::map<int64, Model_Taglink::Data_Set> bills_tags;
    std::map<int64, Model_Attachment::Data_Set> bills_attachments;
    Model_Billsdeposits::Data_Set bills;
    typedef std::tuple<
        int /* i */,
        wxString /* date */,
        int /* repeat_num */
    > bills_index_t;
    std::vector<bills_index_t> bills_index;
    if (m_scheduled_enable && m_scheduled_selected) {
        bills_splits = Model_Budgetsplittransaction::instance().get_all();
        bills_tags = Model_Taglink::instance().get_all(billRefType);
        bills_attachments = Model_Attachment::instance().get_all(Model_Billsdeposits::refTypeName);
        bills = m_account ?
            Model_Account::billsdeposits(m_account) :
            Model_Billsdeposits::instance().all();
        for (unsigned int i = 0; i < bills.size(); ++i) {
            int limit = 1000;  // this is enough for daily repetitions for one year
            auto dates = Model_Billsdeposits::unroll(bills[i], date_end_str, limit);
            for (unsigned int repeat_num = 1; repeat_num <= dates.size(); ++repeat_num)
                bills_index.push_back({i, dates[repeat_num-1], repeat_num});
        }
        std::stable_sort(
            bills_index.begin(), bills_index.end(),
            [](const bills_index_t& a, const bills_index_t& b) -> bool {
                return std::get<1>(a) < std::get<1>(b);
            }
        );
    }

    auto trans_it = trans.begin();
    auto bills_it = bills_index.begin();
    while (trans_it != trans.end() || bills_it != bills_index.end()) {
        int bill_i = 0;
        wxString tran_date;
        int repeat_num = 0;
        Model_Checking::Data bill_tran;
        const Model_Checking::Data* tran = nullptr;

        if (trans_it != trans.end())
            tran_date = Model_Checking::TRANSDATE(*trans_it).FormatISOCombined();
        if (trans_it != trans.end() &&
            (bills_it == bills_index.end() || tran_date.Left(10) <= std::get<1>(*bills_it).Left(10))
        ) {
            tran = &(*trans_it);
            trans_it++;
        }
        else {
            bill_i = std::get<0>(*bills_it);
            tran_date = std::get<1>(*bills_it);
            repeat_num = std::get<2>(*bills_it);
            bill_tran = Fused_Transaction::execute_bill(bills[bill_i], tran_date);
            tran = &bill_tran;
            bills_it++;
        }

        if (isGroup() &&
            m_group_ids.find(tran->ACCOUNTID) == m_group_ids.end() &&
            m_group_ids.find(tran->TOACCOUNTID) == m_group_ids.end()
        )
            continue;
        if (isDeletedTrans() != !tran->DELETEDTIME.IsEmpty())
            continue;
        if (ignore_future && tran_date > today_date)
            break;

        // update m_balance even if tran is filtered out
        double account_flow = 0.0;
        if (isAccount()) {
            // assertion: tran->DELETEDTIME.IsEmpty()
            account_flow = Model_Checking::account_flow(tran, m_account_id);
            m_balance += account_flow;
            if (Model_Checking::status_id(tran->STATUS) == Model_Checking::STATUS_ID_RECONCILED)
                m_reconciled_balance += account_flow;
            else
                m_show_reconciled = true;
        }

        if (tran_date < date_start_str || tran_date > date_end_str)
            continue;

        Fused_Transaction::Full_Data full_tran = (repeat_num == 0) ?
            Fused_Transaction::Full_Data(*tran, trans_splits, trans_tags) :
            Fused_Transaction::Full_Data(bills[bill_i], tran_date, repeat_num, bills_splits, bills_tags);

        bool expandSplits = false;
        if (m_filter_advanced) {
            int txnMatch = m_trans_filter_dlg->mmIsRecordMatches(*tran, full_tran.m_splits);
            if (txnMatch) {
                expandSplits = (txnMatch < static_cast<int>(full_tran.m_splits.size()) + 1);
            }
            else {
              continue;
            }
        }

        full_tran.PAYEENAME = full_tran.real_payee_name(m_account_id);
        if (isAccount()) {
            if (full_tran.ACCOUNTID_W != m_account_id) {
                full_tran.ACCOUNTID_W = -1; full_tran.TRANSAMOUNT_W = 0.0;
            }
            if (full_tran.ACCOUNTID_D != m_account_id) {
                full_tran.ACCOUNTID_D = -1; full_tran.TRANSAMOUNT_D = 0.0;
            }
            full_tran.ACCOUNT_FLOW = account_flow;
            full_tran.ACCOUNT_BALANCE = m_balance;
        }

        if (repeat_num == 0 && trans_attachments.find(tran->TRANSID) != trans_attachments.end()) {
            for (const auto& entry : trans_attachments.at(tran->TRANSID))
                full_tran.ATTACHMENT_DESCRIPTION.Add(entry.DESCRIPTION);
        }
        else if (repeat_num > 0 && bills_attachments.find(full_tran.m_bdid) != bills_attachments.end()) {
            for (const auto& entry : bills_attachments.at(full_tran.m_bdid))
                full_tran.ATTACHMENT_DESCRIPTION.Add(entry.DESCRIPTION);
        }

        for (int i = 0; i < 5; i++) {
            full_tran.UDFC_type[i] = Model_CustomField::TYPE_ID_UNKNOWN;
            full_tran.UDFC_value[i] = -DBL_MAX;
        }

        if (repeat_num == 0 && tranFieldData.find(tran->TRANSID) != tranFieldData.end()) {
            for (const auto& udfc : tranFieldData.at(tran->TRANSID)) {
                for (int i = 0; i < 5; i++) {
                    if (udfc.FIELDID == udfc_id[i]) {
                        full_tran.UDFC_type[i] = udfc_type[i];
                        full_tran.UDFC_content[i] = udfc.CONTENT;
                        full_tran.UDFC_value[i] = cleanseNumberStringToDouble(
                            udfc.CONTENT, udfc_scale[i] > 0
                        );
                        break;
                    }
                }
            }
        }
        else if (repeat_num > 0 && tranFieldData.find(-full_tran.m_bdid) != tranFieldData.end()) {
            for (const auto& udfc : tranFieldData.at(-full_tran.m_bdid)) {
                for (int i = 0; i < 5; i++) {
                    if (udfc.FIELDID == udfc_id[i]) {
                        full_tran.UDFC_type[i] = udfc_type[i];
                        full_tran.UDFC_content[i] = udfc.CONTENT;
                        full_tran.UDFC_value[i] = cleanseNumberStringToDouble(
                            udfc.CONTENT, udfc_scale[i] > 0
                        );
                        break;
                    }
                }
            }
        }

        wxString marker = (repeat_num == 0) ? "" : "*";
        full_tran.SN = ++sn;
        full_tran.displaySN = wxString::Format("%s%ld", marker, full_tran.SN);
        if (repeat_num > 0)
            full_tran.displayID = wxString::Format("%s%ld", marker, full_tran.m_bdid);

        if (!expandSplits) {
            m_lc->m_trans.push_back(full_tran);
            if (isAccount())
                m_flow += account_flow;
            continue;
        }

        int splitIndex = 1;
        wxString tranTagnames = full_tran.TAGNAMES;
        wxString tranDisplaySN = full_tran.displaySN;
        wxString tranDisplayID = full_tran.displayID;
        for (const auto& split : full_tran.m_splits) {
            if (m_filter_advanced) {
              if (!m_trans_filter_dlg->mmIsSplitRecordMatches<Model_Splittransaction>(split))
                  continue;
            }
            full_tran.displaySN = tranDisplaySN + "." + wxString::Format("%i", splitIndex);
            full_tran.displayID = tranDisplayID + "." + wxString::Format("%i", splitIndex);
            splitIndex++;
            full_tran.CATEGID = split.CATEGID;
            full_tran.CATEGNAME = Model_Category::full_name(split.CATEGID);
            full_tran.TRANSAMOUNT = split.SPLITTRANSAMOUNT;
            full_tran.NOTES = tran->NOTES;
            full_tran.TAGNAMES = tranTagnames;
            Model_Checking::Data splitWithTxnNotes = full_tran;
            Model_Checking::Data splitWithSplitNotes = full_tran;
            splitWithSplitNotes.NOTES = split.NOTES;
            if (m_filter_advanced) {
              if (
                  !m_trans_filter_dlg->mmIsRecordMatches<Model_Checking>(splitWithSplitNotes, true) &&
                  !m_trans_filter_dlg->mmIsRecordMatches<Model_Checking>(splitWithTxnNotes, true)
              ) {
                continue;
              }
            }
            if (isAccount()) {
                full_tran.ACCOUNT_FLOW = Model_Checking::account_flow(splitWithTxnNotes, m_account_id);
                m_flow += full_tran.ACCOUNT_FLOW;
            }
            full_tran.NOTES.Append((tran->NOTES.IsEmpty() ? "" : " ") + split.NOTES);
            wxString tagnames;
            const wxString reftype = (repeat_num == 0) ? tranSplitRefType : billSplitRefType;
            for (const auto& tag : Model_Taglink::instance().get(reftype, split.SPLITTRANSID))
                tagnames.Append(tag.first + " ");
            if (!tagnames.IsEmpty())
                full_tran.TAGNAMES.Append((full_tran.TAGNAMES.IsEmpty() ? "" : ", ") + tagnames.Trim());
            m_lc->m_trans.push_back(full_tran);
        }
    }
}

void mmCheckingPanel::sortList()
{
    m_lc->sortList();
}

//----------------------------------------------------------------------------

void mmCheckingPanel::updateExtraTransactionData(bool single, int repeat_num, bool foreign)
{
    if (single) {
        enableButtons(
            /* Edit      */ true,
            /* Duplicate */ !foreign,
            /* Delete    */ !repeat_num,
            /* Enter     */ repeat_num == 1,
            /* Skip      */ repeat_num == 1,
            /* attach    */ true
        );

        long x = -1;
        for (x = 0; x < m_lc->GetItemCount(); x++) {
            if (m_lc->GetItemState(x, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
                break;
        }

        Fused_Transaction::Full_Data full_tran(m_lc->m_trans[x]);
        wxString miniStr = full_tran.info();
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

        wxString notesStr = full_tran.NOTES;
        if (!full_tran.m_repeat_num) {
            auto splits = Model_Splittransaction::instance().find(
                Model_Splittransaction::TRANSID(full_tran.TRANSID)
            );
            for (const auto& split : splits)
                if (!split.NOTES.IsEmpty()) {
                    notesStr += notesStr.empty() ? "" : "\n";
                    notesStr += split.NOTES;
                }
            if (full_tran.has_attachment()) {
                const wxString& refType = Model_Checking::refTypeName;
                Model_Attachment::Data_Set attachments = Model_Attachment::instance().FilterAttachments(refType, full_tran.TRANSID);
                for (const auto& i : attachments) {
                    notesStr += notesStr.empty() ? "" : "\n";
                    notesStr += _t("Attachment") + " " + i.DESCRIPTION + " " + i.FILENAME;
                }
            }
        }
        else {
            auto splits = Model_Budgetsplittransaction::instance().find(
                Model_Budgetsplittransaction::TRANSID(full_tran.m_bdid)
            );
            for (const auto& split : splits)
                if (!split.NOTES.IsEmpty()) {
                    notesStr += notesStr.empty() ? "" : "\n";
                    notesStr += split.NOTES;
                }
            if (full_tran.has_attachment()) {
                const wxString& refType = Model_Billsdeposits::refTypeName;
                Model_Attachment::Data_Set attachments = Model_Attachment::instance().FilterAttachments(refType, full_tran.m_bdid);
                for (const auto& i : attachments) {
                    notesStr += notesStr.empty() ? "" : "\n";
                    notesStr += _t("Attachment") + " " + i.DESCRIPTION + " " + i.FILENAME;
                }
            }
        }
        m_info_panel_selectedbal.clear(); // Not displaying any selected transactions in m_info_panel, clear selected transaction balance var
        m_info_panel->SetLabelText(notesStr);
    }
    else /* !single */ {
        m_info_panel_mini->SetLabelText("");
        const auto selected = m_lc->getSelectedId();
        if (selected.size() > 0) {
            bool selected_bill = false;
            for (const auto& id : selected)
                if (id.second) { selected_bill = true; break; }

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
                Model_Currency::Data* curr = Model_Account::currency(Model_Account::instance().get(m_lc->m_trans[item].ACCOUNTID));
                if ((m_account_id < 0) && Model_Checking::is_transfer(m_lc->m_trans[item].TRANSCODE)) continue;
                double convrate = (curr != m_currency) ? Model_CurrencyHistory::getDayRate(curr->CURRENCYID, m_lc->m_trans[item].TRANSDATE) : 1.0;
                flow += convrate * Model_Checking::account_flow(m_lc->m_trans[item], (m_account_id < 0) ? m_lc->m_trans[item].ACCOUNTID : m_account_id);
                wxString transdate = m_lc->m_trans[item].TRANSDATE;
                if (minDate > transdate || minDate.empty()) minDate = transdate;
                if (maxDate < transdate || maxDate.empty()) maxDate = transdate;
            }

            wxDateTime min_date, max_date;
            min_date.ParseISODate(minDate);
            max_date.ParseISODate(maxDate);
            int days = max_date.Subtract(min_date).GetDays();

            wxString msg;
            wxString selectedBal = Model_Currency::toCurrency(flow, m_currency);
            m_info_panel_selectedbal = selectedBal;
            msg = wxString::Format(_t("Transactions selected: %zu"), selected.size());
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

void mmCheckingPanel::enableButtons(bool edit, bool dup, bool del, bool enter, bool skip, bool attach)
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

void mmCheckingPanel::showTips()
{
    m_info_panel_selectedbal.clear(); // Not displaying any selected transactions in m_info_panel, clear selected transaction balance var

    if (m_show_tips) {
        m_show_tips = false;
        return;
    }

    if (Option::instance().getShowMoneyTips())
        m_info_panel->SetLabelText(
            wxGetTranslation(wxString::FromUTF8(
                TIPS[rand() % (sizeof(TIPS) / sizeof(wxString))]
                .ToStdString()
            ))
        );
    else
        m_info_panel->SetLabelText("");
}

void mmCheckingPanel::showTips(const wxString& tip)
{
    m_info_panel_selectedbal.clear(); // Not displaying any selected transactions in m_info_panel, clear selected transaction balance var
    if (Option::instance().getShowMoneyTips())
        m_info_panel->SetLabelText(tip);
    else
        m_info_panel->SetLabelText("");
    m_show_tips = true;
}

void mmCheckingPanel::updateScheduledToolTip()
{
   mmToolTip(m_header_scheduled,
        !m_scheduled_enable ?
        _t("Scheduled transactions are not available, because the current filter ends in the past") : !m_scheduled_selected ? _t("Click to show scheduled transactions. This feature works best with filter choices that extend into the future (e.g., Current Month).") :
        _t("Click to hide scheduled transactions."));
}

//----------------------------------------------------------------------------

void mmCheckingPanel::onFilterPopup(wxCommandEvent& event)
{
    wxMenu menu;
    int i = 0;
    while (i < m_date_range_m) {
        menu.Append(mmID_FILTER_DATE_MIN + i, m_date_range_a[i].getName());
        i++;
    }

    if (i +1 < static_cast<int>(m_date_range_a.size())) { //only add separator if there are more entries
        menu.AppendSeparator();
    }
    if (i < static_cast<int>(m_date_range_a.size())) {
        wxMenu* menu_more(new wxMenu);
        menu.AppendSubMenu(menu_more, _tu("More date ranges…"));
        while (i < static_cast<int>(m_date_range_a.size())) {
            menu_more->Append(mmID_FILTER_DATE_MIN + i, m_date_range_a[i].getName());
            i++;
        }
    }

    menu.AppendSeparator();
    menu.Append(mmID_EDIT_DATE_RANGES, _tu("Edit date ranges…"));

    PopupMenu(&menu);
    m_bitmapTransFilter->Layout();
    event.Skip();
}

void mmCheckingPanel::onFilterDate(wxCommandEvent& event)
{
    int i = event.GetId() - mmID_FILTER_DATE_MIN;
    if (i < 0 || i >= static_cast<int>(m_date_range_a.size()))
        return;

    m_filter_id = FILTER_ID_DATE_RANGE;
    m_current_date_range = DateRange2();
    if (isAccount()) {
        m_current_date_range.setDateS(Model_Account::DateOf(m_account->STATEMENTDATE));
    }
    m_current_date_range.setSpec(m_date_range_a[i]);
    updateScheduledEnable();
    saveFilterSettings();
    updateFilter();

    m_bitmapTransFilter->SetLabel(m_current_date_range.getName());
    m_bitmapTransFilter->SetBitmap(mmBitmapBundle((i > 0 ? png::TRANSFILTER_ACTIVE : png::TRANSFILTER), mmBitmapButtonSize));

    fromDateCtrl->SetValue(m_current_date_range.checking_start().IsValid() ? m_current_date_range.checking_start() : DATE_MIN);
    toDateCtrl->SetValue(m_current_date_range.checking_end().IsValid() ? m_current_date_range.checking_end() : wxDateTime::Now());

    refreshList();
}

void mmCheckingPanel::onDatePickLow(wxDateEvent& event) {
    // Check if low date < = high date
    if (toDateCtrl->GetValue().IsValid() && event.GetDate().IsLaterThan(toDateCtrl->GetValue())) {
        wxLogDebug("onDatePickLow to Date =>: %s", event.GetDate().FormatISODate());
        toDateCtrl->SetValue(event.GetDate());
    }
    datePickProceed();
}

void mmCheckingPanel::onDatePickHigh(wxDateEvent& event) {
    // Check if high date <= low date
    if (fromDateCtrl->GetValue().IsValid() && event.GetDate().IsEarlierThan(fromDateCtrl->GetValue())) {
        fromDateCtrl->SetValue(event.GetDate());
    }
    datePickProceed();
}

void mmCheckingPanel::datePickProceed() {
    m_bitmapTransFilter->SetLabel(_t("Date range"));
    m_filter_id = FILTER_ID_DATE_PICKER;
    m_current_date_range.setDateS(fromDateCtrl->GetValue().IsValid() ? fromDateCtrl->GetValue() : wxDateTime(static_cast<time_t>(0)));
    m_current_date_range.setDateT(toDateCtrl->GetValue().IsValid() ? toDateCtrl->GetValue() : wxDateTime::Now().Add(wxDateSpan(0,0,0,30)));

    m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER_ACTIVE, mmBitmapButtonSize));
    updateScheduledEnable();
    saveFilterSettings();
    updateFilter();
    refreshList();
}

void mmCheckingPanel::onFilterAdvanced(wxCommandEvent& WXUNUSED(event))
{
        wxString j_str = Model_Infotable::instance().getString(
            wxString::Format("CHECK_FILTER_ID_ADV_%lld", m_checking_id),
            "{}"
        );
        m_trans_filter_dlg.reset(
            new mmFilterTransactionsDialog(this, m_checking_id, false, j_str)
        );
    m_trans_filter_dlg->ShowModal();
    loadFilterSettings();
    updateFilter();
    updateFilterTooltip();
    refreshList();
}

void mmCheckingPanel::onEditDateRanges(wxCommandEvent& WXUNUSED(event))
{
    mmDateRangeDialog dlg(this, &m_date_range_a, &m_date_range_m);
    if (dlg.ShowModal() == wxID_OK) {
        if (m_date_range_a.size() == 0) {
            int src_i = 0;
            int src_m = Option::instance().getCheckingRangeM();
            for (const auto& spec : Option::instance().getCheckingRangeA()) {
                if (m_date_range_a.size() > mmID_FILTER_DATE_MAX - mmID_FILTER_DATE_MIN) {
                    break;
                }
                if (src_i == src_m) {
                    m_date_range_m = m_date_range_a.size();
                }
                if (isAccount() || !spec.hasPeriodS()) {
                    m_date_range_a.push_back(spec);
                }
                src_i++;
            }
        }
        // Verify if current filter is still valid otherwise reset to "ALL"
        if (m_filter_id == FILTER_ID_DATE_RANGE) {
            wxString curname = m_current_date_range.getName();
            bool isDeleted = true;
            for (const auto& spec : m_date_range_a) {
                if (spec.getName() == curname) {
                    isDeleted = false;
                    break;
                }
            }
            if (isDeleted) {
                wxCommandEvent evt = wxCommandEvent(wxEVT_NULL, mmID_FILTER_DATE_MIN);
                onFilterDate(evt);
            }
        }
    }
}

void mmCheckingPanel::onScheduled(wxCommandEvent&)
{
    if (!isDeletedTrans()) {
        m_scheduled_selected = m_header_scheduled->GetValue();
        updateScheduledToolTip();
        saveFilterSettings();
    }
    refreshList();
}

//----------------------------------------------------------------------------

void mmCheckingPanel::onNewTransaction(wxCommandEvent& event)
{
    m_lc->onNewTransaction(event);
}

void mmCheckingPanel::onEditTransaction(wxCommandEvent& event)
{
    m_lc->onEditTransaction(event);
    m_lc->SetFocus();
}

void mmCheckingPanel::onDeleteTransaction(wxCommandEvent& event)
{
    m_lc->onDeleteTransaction(event);
}

void mmCheckingPanel::onRestoreTransaction(wxCommandEvent& event)
{
    m_lc->onRestoreTransaction(event);
}

void mmCheckingPanel::onDuplicateTransaction(wxCommandEvent& event)
{
    m_lc->onDuplicateTransaction(event);
}

void mmCheckingPanel::onMoveTransaction(wxCommandEvent& event)
{
    m_lc->onMoveTransaction(event);
}

void mmCheckingPanel::onEnterScheduled(wxCommandEvent& event)
{
    m_lc->onEnterScheduled(event);
}

void mmCheckingPanel::onSkipScheduled(wxCommandEvent& event)
{
    m_lc->onSkipScheduled(event);
}

void mmCheckingPanel::onOpenAttachment(wxCommandEvent& event)
{
    m_lc->onOpenAttachment(event);
    m_lc->SetFocus();
}

void mmCheckingPanel::onSearchTxtEntered(wxCommandEvent& event)
{
    const wxString search_string = event.GetString();
    if (search_string.IsEmpty()) return;
    m_lc->doSearchText(search_string);
}

void mmCheckingPanel::onButtonRightDown(wxMouseEvent& event)
{
    int id = event.GetId();
    switch (id) {
    case mmID_FILTER: {
        wxCommandEvent evt(wxID_ANY, mmID_FILTER_ADVANCED);
        onFilterAdvanced(evt);
        break;
    }
    case wxID_FILE: {
        auto selected_id = m_lc->getSelectedId();
        if (selected_id.size() == 1) {
            const wxString refType = !selected_id[0].second ?
                Model_Checking::refTypeName :
                Model_Billsdeposits::refTypeName;
            mmAttachmentDialog dlg(this, refType, selected_id[0].first);
            dlg.ShowModal();
            refreshList();
        }
        break;
    }
    case wxID_NEW: {
        wxMenu menu;
        menu.Append(Model_Checking::TYPE_ID_WITHDRAWAL, _tu("&New Withdrawal…"));
        menu.Append(Model_Checking::TYPE_ID_DEPOSIT, _tu("&New Deposit…"));
        menu.Append(Model_Checking::TYPE_ID_TRANSFER, _tu("&New Transfer…"));
        PopupMenu(&menu);
    }
    default:
        break;
    }
}

void mmCheckingPanel::onInfoPanelClick(wxMouseEvent& event, wxStaticText* infoPanel)
{
    wxString clipboardValue = "";
    if (!m_info_panel_selectedbal.IsEmpty())
    {
        clipboardValue = m_info_panel_selectedbal;
    }
    else
    {
        clipboardValue = infoPanel->GetLabel();
    }
    if (!clipboardValue.IsEmpty())
    {
        // Copy to clipboard
        if (wxTheClipboard->Open())
        {
            wxTheClipboard->SetData(new wxTextDataObject(clipboardValue));
            wxTheClipboard->Close();
            this->Layout();
        }
        m_info_panel_selectedbal.empty();
    }

    event.Skip();
}


//----------------------------------------------------------------------------

wxString mmCheckingPanel::getPanelTitle() const
{
    if (isAllTrans())
        return _t("All Transactions");
    else if (isDeletedTrans())
        return _t("Deleted Transactions");
    else if (isGroup()) {
        if (m_checking_id == -3)
            return _t("Favorites");
        else
            return wxGetTranslation(mmGUIFrame::ACCOUNT_SECTION[m_account_type]);
    }
    else if (m_account)
        return wxString::Format(_t("Account View: %s"), m_account->ACCOUNTNAME);
    else
        return "";
}

wxString mmCheckingPanel::BuildPage() const
{
    return m_lc->BuildPage((m_account ? getPanelTitle() : ""));
}

void mmCheckingPanel::resetColumnView()
{
    m_lc->DeleteAllColumns();
    m_lc->setColumnsInfo();
    m_lc->createColumns();
    m_lc->refreshVisualList();
}

void mmCheckingPanel::setSelectedTransaction(Fused_Transaction::IdRepeat fused_id)
{
    m_lc->setSelectedId(fused_id);
    refreshList();
    m_lc->SetFocus();
}

void mmCheckingPanel::displaySplitCategories(Fused_Transaction::IdB fused_id)
{
    Fused_Transaction::Data fused = !fused_id.second ?
        Fused_Transaction::Data(*Model_Checking::instance().get(fused_id.first)) :
        Fused_Transaction::Data(*Model_Billsdeposits::instance().get(fused_id.first));
    std::vector<Split> splits;
    for (const auto& split : Fused_Transaction::split(fused)) {
        Split s;
        s.CATEGID          = split.CATEGID;
        s.SPLITTRANSAMOUNT = split.SPLITTRANSAMOUNT;
        s.NOTES            = split.NOTES;
        splits.push_back(s);
    }
    if (splits.empty()) return;
    mmSplitTransactionDialog splitTransDialog(this, splits, m_account_id, true);

    //splitTransDialog.SetDisplaySplitCategories();
    splitTransDialog.ShowModal();
}

void mmCheckingPanel::mmPlayTransactionSound()
{
    int play = Model_Setting::instance().getInt(INIDB_USE_TRANSACTION_SOUND, 0);
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
wxString mmCheckingPanel::getFilterName(FILTER_ID id) {
    return mmCheckingPanel::FILTER_NAME[id].second;
}

void mmCheckingPanel::loadDateRanges(std::vector<DateRange2::Spec>* date_range_ptr, int* range_m, bool isaccount) {
    date_range_ptr->clear();
    *range_m = -1;
    int src_i = 0;
    int src_m = Option::instance().getCheckingRangeM();
    for (const auto& spec : Option::instance().getCheckingRangeA()) {
        if (date_range_ptr->size() > mmID_FILTER_DATE_MAX - mmID_FILTER_DATE_MIN) {
            break;
        }
        if (src_i == src_m) {
            *range_m = date_range_ptr->size();
        }
        if (isaccount || !spec.hasPeriodS()) {
            date_range_ptr->push_back(spec);
        }
        src_i++;
    }
    if (*range_m < 0) {
        *range_m = date_range_ptr->size();
    }
}
