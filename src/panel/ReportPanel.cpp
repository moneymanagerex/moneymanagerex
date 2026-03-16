/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2021-2025 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025, 2026 Klaus Wich

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
#include <vector>
#include <string>
#include <iomanip>
#include <wx/wrapsizer.h>

#include "mmex.h"
#include "base/platfdep.h"
#include "base/paths.h"
#include "base/images_list.h"
#include "util/_util.h"

#include "model/_all.h"
#include "model/PrefModel.h"
#include "model/TrxFilter.h"

#include "mmframe.h"
#include "ReportPanel.h"

#include "manager/DateRangeManager.h"
#include "dialog/AssetDialog.h"
#include "dialog/AttachmentDialog.h"
#include "dialog/TrxDialog.h"
#include "dialog/TrxShareDialog.h"
#include "dialog/BudgetEntryDialog.h"
#include "report/htmlbuilder.h"
#include "uicontrols/navigatortypes.h"

wxBEGIN_EVENT_TABLE(ReportPanel, wxPanel)
    EVT_CHOICE(ID_YEAR_CHOICE,               ReportPanel::onYearChanged)
    EVT_CHOICE(ID_BUDGET_CHOICE,             ReportPanel::onBudgetChanged)
    EVT_CHOICE(ID_ACCOUNT_CHOICE,            ReportPanel::onAccountChanged)
    EVT_CHOICE(ID_STOCK_CHOICE,              ReportPanel::onStockChanged)
    EVT_TEXT_ENTER(ID_FILTER_GENERIC_CHOICE, ReportPanel::onFilterChanged)
    EVT_CHOICE(ID_SELECTION_GENERIC_CHOICE,  ReportPanel::onSelectionChanged)
    EVT_DATE_CHANGED(ID_START_DATE_PICKER,   ReportPanel::onStartEndDateChanged)
    EVT_TIME_CHANGED(ID_START_DATE_PICKER,   ReportPanel::onStartEndDateChanged)
    EVT_DATE_CHANGED(ID_END_DATE_PICKER,     ReportPanel::onStartEndDateChanged)
    EVT_TIME_CHANGED(ID_END_DATE_PICKER,     ReportPanel::onStartEndDateChanged)
    EVT_DATE_CHANGED(ID_SINGLE_DATE_PICKER,  ReportPanel::onSingleDateChanged)
    EVT_TIME_CHANGED(ID_SINGLE_DATE_PICKER,  ReportPanel::onSingleDateChanged)
    EVT_CHOICE(ID_CHART_CHOICE,              ReportPanel::onChartChanged)
    EVT_SPINCTRL(ID_FORWARD_MONTHS,          ReportPanel::onForwardMonthsChangedSpin)
    EVT_TEXT_ENTER(ID_FORWARD_MONTHS,        ReportPanel::onForwardMonthsChangedText)
    EVT_BUTTON(ID_DATE_RANGE_BUTTON,         ReportPanel::onDateRangePopup)
    EVT_MENU(ID_DATE_RANGE_EDIT,             ReportPanel::onDateRangeEdit)
    EVT_MENU_RANGE(
        ID_DATE_RANGE_MIN,
        ID_DATE_RANGE_MAX,
        ReportPanel::onDateRangeSelect)
    EVT_BUTTON(wxID_ANY, ReportPanel::onShiftPressed)
wxEND_EVENT_TABLE()

ReportPanel::ReportPanel(
    ReportBase* rb, bool cleanup, wxWindow *parent, mmGUIFrame* frame,
    wxWindowID winid, const wxPoint& pos,
    const wxSize& size, long style,
    const wxString& name
) :
    m_rb(rb),
    m_cleanup(cleanup),
    w_frame(frame)
{
    Create(parent, winid, pos, size, style, name);
}

ReportPanel::~ReportPanel()
{
    if (m_cleanup && m_rb) {
        delete m_rb;
    }

    clearVFprintedFiles("rep");
}

bool ReportPanel::Create(
    wxWindow *parent, wxWindowID winid,
    const wxPoint& pos, const wxSize& size, long style,
    const wxString& name
) {
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    m_use_account_specific_filter = PrefModel::instance().getUsePerAccountFilter();

    m_filter_id = JournalPanel::FILTER_ID_DATE;

    m_rb->extractParameters();
    m_rb->restoreReportSettings();

    CreateControls();

    if (m_rb->getParameters() > 0) {
        loadFilterSettings();
        updateFilter();
    }
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    saveReportText();

    ReportBase::REPORT_ID id = m_rb->getReportId();
    this->SetLabel(id < 0 ? "Custom Report" : m_rb->getTitle(false));

    return true;
}

bool ReportPanel::saveReportText()
{
    if (!m_rb)
        return false;

    if (m_rb->getParameters() & ReportBase::M_DATE_RANGE) {
        m_rb->setDateRange(m_date_range);
        m_rb->setDateSelection(0);
    }

    if (m_rb->getParameters() & (ReportBase::M_BUDGET | ReportBase::M_YEAR)) {
        int selectedItem = w_year_choice->GetSelection();
        wxString id_str = "0";
        wxStringClientData* obj =
            static_cast<wxStringClientData*>(w_year_choice->GetClientObject(selectedItem));
        if (obj) id_str = obj->GetData();
        int64 id = std::stoll(id_str.ToStdString());
        m_rb->setDateSelection(id);
    }

    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String(wxTRANSLATE("Report"));
    json_writer.Key("name");
    json_writer.String(m_rb->getTitle(false).utf8_str());

    const auto time = wxDateTime::UNow();

    const auto& name = getVFname4print("rep", m_rb->getHTMLText());
    w_browser->LoadURL(name);

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    const auto t = wxString::FromUTF8(json_buffer.GetString());
    wxLogDebug("%s", t);
    UsageModel::instance().append_usage(t);
    UsageModel::instance().pageview(this, m_rb, (wxDateTime::UNow() - time).GetMilliseconds().ToLong());

    return true;
}

// Adjust wxStaticText size after font change
// Workaround for not auto Layout() after SetFont()
void mmSetOwnFont(wxStaticText* w, const wxFont& font)
{
    w->SetOwnFont(font);
    wxString label = w->GetLabelText();
    if (!label.IsEmpty())
        w->SetInitialSize(w->GetTextExtent(label));
}

// function only used for new filter
void ReportPanel::loadFilterSettings() {
    wxString key = m_use_account_specific_filter
        ? wxString::Format("REPORT_FILTER_DEDICATED_%d", m_rb->getReportId())
        : "REPORT_FILTER_ALL";
    Document j_doc = InfoModel::instance().getJdoc(key, "{}");

    int fid = 0;
    if (JSON_GetIntValue(j_doc, "FILTER_ID", fid)) {
        m_filter_id = static_cast<JournalPanel::FILTER_ID>(fid);
    }
    else {
        // no filter found => set to date range
        m_filter_id = JournalPanel::FILTER_ID_DATE_RANGE;
    }

    loadDateRanges(&m_date_range_a, &m_date_range_m);
    if (m_filter_id == JournalPanel::FILTER_ID_DATE_RANGE) {
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
            // init with 'All'
            m_date_range.setRange(m_date_range_a[0]);
        }
    }
    else if (m_filter_id == JournalPanel::FILTER_ID_DATE_PICKER) {
        // Load start/end date pickers from settings.
        // The date range is configured later in updateFilter().
        if (w_start_date_picker) {
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
            w_start_date_picker->SetValue(start_dateTime);
            w_end_date_picker->SetValue(end_dateTime);
        }
        else {
            wxLogDebug("ReportPanel warning: Date Pickers are not available");
        }
    }

    if (w_stocks_choice) {
        int idx = -1;
        if (JSON_GetIntValue(j_doc, "FILTER_STOCK_NAME_IDX", idx)) {
            if (idx > -1 && static_cast<unsigned int>(idx) < w_stocks_choice->GetCount()) {
                w_stocks_choice->SetSelection(idx);
            }
        }
    }

    if (w_filter) {
        auto map = m_rb->getFilterMap();
        wxString token = map.count("name") > 0 ? "FILTER_STRING_" + removeQuotes(map["name"]) : "FILTER_STRING_VALUE";
        wxString filter_str;
        if (JSON_GetStringValue(j_doc, token.ToUTF8(), filter_str)) {
            w_filter->SetValue(filter_str);
        }
    }
}

void ReportPanel::saveFilterSettings() {
    wxString key = m_use_account_specific_filter
        ? wxString::Format("REPORT_FILTER_DEDICATED_%d", m_rb->getReportId())
        : "REPORT_FILTER_ALL";
    Document j_doc = InfoModel::instance().getJdoc(key, "{}");
    InfoModel::saveFilterInt(j_doc, "FILTER_ID", m_filter_id);
    InfoModel::saveFilterString(j_doc, "FILTER_NAME",
        JournalPanel::getFilterName(m_filter_id)
    );

    if (m_filter_id == JournalPanel::FILTER_ID_DATE_RANGE) {
        if (!m_date_range.rangeName().IsEmpty()) {
            InfoModel::saveFilterString(j_doc, "FILTER_DATE", m_date_range.rangeName());
            InfoModel::saveFilterString(j_doc, "FILTER_DATE_BEGIN", "");
            InfoModel::saveFilterString(j_doc, "FILTER_DATE_END", "");
        }
        else {
            wxLogError("ReportPanel::saveFilterSettings(): m_date_range.rangeName() is empty");
        }
    }
    else if (m_filter_id == JournalPanel::FILTER_ID_DATE_PICKER) {
        if (w_start_date_picker) {
            InfoModel::saveFilterString(j_doc, "FILTER_DATE_BEGIN",
                mmDateN(w_start_date_picker->GetValue()).isoDateN()
            );
        }
        else {
            wxLogDebug("ReportPanel::saveFilterSettings(): w_start_date_picker is null");
        }
        if (w_end_date_picker) {
            InfoModel::saveFilterString(j_doc, "FILTER_DATE_END",
                mmDateN(w_end_date_picker->GetValue()).isoDateN()
            );
        }
        else {
            wxLogDebug("ReportPanel::saveFilterSettings(): w_end_date_picker is null");
        }
        InfoModel::saveFilterString(j_doc, "FILTER_DATE", "");
    }

    if (w_stocks_choice) {
        InfoModel::saveFilterInt(j_doc, "FILTER_STOCK_NAME_IDX", w_stocks_choice->GetSelection());
    }

    if (w_filter) {
        auto map = m_rb->getFilterMap();
        wxString token = map.count("name") > 0 ? "FILTER_STRING_" + removeQuotes(map["name"]) : "FILTER_STRING_VALUE";
        InfoModel::saveFilterString(j_doc, token.ToUTF8(), w_filter->GetValue());
    }

    InfoModel::instance().setJdoc(key, j_doc);
}

void ReportPanel::updateFilter()
{
    if (!w_date_range_button) {
        return;
    }

    wxLogDebug("ReportPanel::updateFilter(): m_filter_id=%d", int(m_filter_id));
    if (m_filter_id == JournalPanel::FILTER_ID_DATE_RANGE) {
        w_date_range_button->SetLabel(m_date_range.rangeName());
        w_date_range_button->SetBitmap(mmBitmapBundle(
            // FIXME: refine the condition below
            (m_date_range.rangeName() != m_date_range_a[0].getName()
                ? png::TRANSFILTER_ACTIVE
                : png::TRANSFILTER
            ),
            mmBitmapButtonSize
        ));
        // TODO: calculate default start/end dates from model
        m_date_range.setDefStartDateN(mmDate::min());
        m_date_range.setDefEndDateN(mmDate::max());
        // copy from date range to start/end pickers
        w_start_date_picker->SetValue(
            m_date_range.rangeStart().value().getDateTime()
        );
        w_end_date_picker->SetValue(
            m_date_range.rangeEnd().value().getDateTime()
        );
    }
    else if (m_filter_id == JournalPanel::FILTER_ID_DATE_PICKER) {
        w_date_range_button->SetLabel(_t("Date range"));
        w_date_range_button->SetBitmap(mmBitmapBundle(
            png::TRANSFILTER_ACTIVE,
            mmBitmapButtonSize
        ));
        // set date range to default ('All') and copy default start/end dates from pickers.
        m_date_range = mmDateRange2();
        mmDateRange2::Range range = m_date_range.getRange();
        range.setName(_t("Date range"));
        m_date_range.setRange(range);
        m_date_range.setDefStartDateN(mmDateN(w_start_date_picker->GetValue()));
        m_date_range.setDefEndDateN(mmDateN(w_end_date_picker->GetValue()));
    }
}

void ReportPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel(this, wxID_ANY);
    itemBoxSizer2->Add(itemPanel3, 0, wxGROW | wxALL, 0);

    wxWrapSizer* itemBoxSizerHeader = new wxWrapSizer();
    itemPanel3->SetSizer(itemBoxSizerHeader);

    itemBoxSizerHeader->Add(new wxStaticText(itemPanel3, wxID_ANY, ""), 0, wxALL | wxALIGN_CENTER_VERTICAL, 2); // Placeholder

    if (m_rb) {
        int rp = m_rb->getParameters();

        if (rp == 0) {
            itemPanel3->SetMinSize(wxSize(0, 0));
            itemPanel3->Fit();
        }

        if (rp & ReportBase::M_DATE_RANGE) {
            w_date_range_button = new wxButton(itemPanel3, ID_DATE_RANGE_BUTTON, _tu("Period…"));
            w_date_range_button->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
            w_date_range_button->SetMinSize(
                wxSize(200 + PrefModel::instance().getIconSize() * 2, -1)
            );
            itemBoxSizerHeader->Add(w_date_range_button, g_flagsH);
            itemBoxSizerHeader->AddSpacer(5);

            w_start_date_picker = new mmDatePickerCtrl(
                itemPanel3, ID_START_DATE_PICKER,
                wxDefaultDateTime, wxDefaultPosition, wxDefaultSize,
                wxDP_DROPDOWN | wxDP_SHOWCENTURY
            );
            itemBoxSizerHeader->Add(w_start_date_picker, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);

            w_end_date_picker = new mmDatePickerCtrl(
                itemPanel3, ID_END_DATE_PICKER,
                wxDefaultDateTime, wxDefaultPosition, wxDefaultSize,
                wxDP_DROPDOWN | wxDP_SHOWCENTURY
            );
            itemBoxSizerHeader->Add(w_end_date_picker, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }
        else if (rp & ReportBase::M_SINGLE_DATE) {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, _t("Date:"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            long date_style = wxDP_DROPDOWN | wxDP_SHOWCENTURY;
            w_single_date_picker = new mmDatePickerCtrl(
                itemPanel3, ID_SINGLE_DATE_PICKER,
                wxDefaultDateTime, wxDefaultPosition, wxDefaultSize,
                date_style
            );
            w_single_date_picker->SetValue(wxDateTime::Today());
            w_single_date_picker->Enable(true);

            itemBoxSizerHeader->Add(w_single_date_picker, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }
        else if (rp & ReportBase::M_MONTHS) {
            wxStaticText* itemStaticTextH1 = new wxStaticText(
                itemPanel3, wxID_ANY, _t("Date:")
            );
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);

            mmDateYearMonth* up_down_month = new mmDateYearMonth(itemPanel3);
            up_down_month->Connect(
                wxEVT_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED,
                wxCommandEventHandler(mmDateYearMonth::OnButtonPress),
                nullptr, this
            );
            m_rb->setDateSelection(m_shift);

            itemBoxSizerHeader->Add(up_down_month, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & ReportBase::M_TIME) {
            wxStaticText* itemStaticTextH1 = new wxStaticText(
                itemPanel3, wxID_ANY, _t("Time:")
            );
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);

            w_time_picker = new wxTimePickerCtrl(itemPanel3, ID_TIME_PICKER);

            itemBoxSizerHeader->Add(w_time_picker, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & ReportBase::M_YEAR) {
            u_cleanup_mem = true;
            wxStaticText* itemStaticTextH1 = new wxStaticText(
                itemPanel3, wxID_ANY, _t("Year:")
            );
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);

            w_year_choice = new wxChoice(
                itemPanel3, ID_YEAR_CHOICE,
                wxDefaultPosition, wxDefaultSize, 0,
                nullptr, wxCB_SORT
            );

            const int y = wxDateTime::Today().GetYear();
            for (int i = y - 100; i <= y + 10; ++i) {
                const wxString name = wxString::Format("%i", i);
                w_year_choice->Append(name, new wxStringClientData(name));
            }

            w_year_choice->SetStringSelection(wxString::Format("%i", y));
            w_year_choice->SetMaxSize(wxSize(120, -1));


            itemBoxSizerHeader->Add(w_year_choice, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }
        else if (rp & ReportBase::M_BUDGET) {
            u_cleanup_mem = true;
            wxStaticText* itemStaticTextH1 = new wxStaticText(
                itemPanel3, wxID_ANY, _t("Budget:")
            );
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);

            w_year_choice = new wxChoice(
                itemPanel3, ID_BUDGET_CHOICE,
                wxDefaultPosition, wxDefaultSize, 0,
                nullptr, wxCB_SORT
            );

            int64 sel_id = m_rb->getDateSelection();
            wxString sel_name;
            for (const auto& bp_d : BudgetPeriodModel::instance().find_all(
                BudgetPeriodCol::COL_ID_BUDGETYEARNAME
            )) {
                const wxString& name = bp_d.m_name;

                // Only years for performance report
                if (m_rb->getReportId() == ReportBase::REPORT_ID::BudgetCategorySummary ||
                    name.length() == 4
                ) {
                    w_year_choice->Append(name, new wxStringClientData(wxString::Format("%lld", bp_d.m_id)));
                    if (sel_id == bp_d.m_id)
                        sel_name = bp_d.m_name;
                }
            }

            if (!sel_name.IsEmpty())
                w_year_choice->SetStringSelection(sel_name);
            else
                w_year_choice->SetSelection(0);

            itemBoxSizerHeader->Add(w_year_choice, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & ReportBase::M_ACCOUNT) {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3, wxID_ANY, _t("Accounts:"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            w_account_choice = new wxChoice(itemPanel3, ID_ACCOUNT_CHOICE);
            w_account_choice->Append(_t("All Accounts"));
            w_account_choice->Append(_tu("Specific Accounts…"));
            w_account_choice->Append(NavigatorTypes::instance().getUsedAccountTypeNames());
            w_account_choice->SetSelection(m_rb->getAccountSelection());

            itemBoxSizerHeader->Add(w_account_choice, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & ReportBase::M_FORWARD_MONTHS) {
            wxStaticText* itemStaticTextH1 = new wxStaticText(
                itemPanel3, wxID_ANY, _t("Future Months:")
            );
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            w_forward_months = new wxSpinCtrl(
                itemPanel3, ID_FORWARD_MONTHS,
                wxEmptyString, wxDefaultPosition, wxDefaultSize,
                wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER
            );
            w_forward_months->SetRange(1, 120);
            w_forward_months->SetValue(m_rb->getForwardMonths());
            itemBoxSizerHeader->Add(w_forward_months, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & ReportBase::M_CHART) {
            wxStaticText* itemStaticTextH1 = new wxStaticText(
                itemPanel3, wxID_ANY, _t("Chart:")
            );
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            w_chart_choice = new wxChoice(itemPanel3, ID_CHART_CHOICE);
            w_chart_choice->Append(_t("Show"));
            w_chart_choice->Append(_t("Hide"));
            w_chart_choice->SetSelection(m_rb->getChartSelection());

            itemBoxSizerHeader->Add(w_chart_choice, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & ReportBase::M_STOCK_NAMES)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3, wxID_ANY, _t("Stock name:"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);

            StockModel::DataA stock_a = StockModel::instance().find_all();
            std::stable_sort(stock_a.begin(), stock_a.end(), StockData::SorterBySTOCKNAME());

            wxString prevSymbol = "";
            w_stocks_choice = new wxChoice(itemPanel3, ID_STOCK_CHOICE);
            for (StockModel::Data stock : stock_a) {
                const AccountModel::Data* account = AccountModel::instance().get_id_data_n(stock.m_account_id_n);
                if (account->is_open()) {
                    if (stock.m_symbol != prevSymbol) {
                        w_stocks_choice->Append(stock.m_name);
                        prevSymbol = stock.m_symbol;
                    }
                }
            }
            w_stocks_choice->SetSelection(m_rb->getStockSelection());

            itemBoxSizerHeader->Add(w_stocks_choice, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & ReportBase::M_GENERIC_FILTER)
        {
            auto map = m_rb->getFilterMap();
            wxString name = map.count("name") > 0 ? removeQuotes(map["name"]) : _t("Filter:");
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3, wxID_ANY, name);
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);

            w_filter = new wxTextCtrl(itemPanel3, ID_FILTER_GENERIC_CHOICE, m_rb->getFilterValue(), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
            if (map.count("default") > 0) {
                w_filter->SetValue(removeQuotes(map["default"]));
            }
            itemBoxSizerHeader->Add(w_filter, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & ReportBase::M_GENERIC_SELECTION)
        {
            auto map = m_rb->getSelectionMap();
            wxString name = map.count("name") > 0 ? removeQuotes(map["name"]) : _t("Selection:");
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3, wxID_ANY, name);
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);

            w_selection_choice = new wxChoice(itemPanel3, ID_SELECTION_GENERIC_CHOICE);
            if (map.count("values") > 0) {
                wxStringTokenizer tokenizer(map["values"], ",");
                while (tokenizer.HasMoreTokens()) {
                    w_selection_choice->Append(removeQuotes(tokenizer.GetNextToken()));
                }
            }
            if (map.count("default") > 0) {
                int idx = w_selection_choice->FindString(removeQuotes(map["default"]));
                w_selection_choice->SetSelection(idx != wxNOT_FOUND ? idx : 0);
            }
            itemBoxSizerHeader->Add(w_selection_choice, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }
    }

    itemBoxSizerHeader->Add(new wxStaticText(itemPanel3, wxID_ANY, ""), 0, wxALL | wxALIGN_CENTER_VERTICAL, 2); // Placeholder

    w_browser = wxWebView::New();
#ifdef __WXMAC__
    // With WKWebView handlers need to be registered before creation
    w_browser->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
    w_browser->Create(this, mmID_BROWSER);
#else
    w_browser->Create(this, mmID_BROWSER);
    w_browser->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
#endif
    Bind(wxEVT_WEBVIEW_NEWWINDOW, &ReportPanel::onNewWindow, this, mmID_BROWSER);

    itemBoxSizer2->Add(w_browser, 1, wxGROW | wxALL, 1);
}

void ReportPanel::PrintPage()
{
    w_browser->Print();
}

void ReportPanel::onNewWindow(wxWebViewEvent& evt)
{
    const wxURI escapedURI(evt.GetURL());
    const wxString uri = escapedURI.BuildUnescapedURI();
    wxString sData;

    wxRegEx pattern(R"(^(https?:)|(file:)\/\/)");
    if (pattern.Matches(uri)) {
        wxLaunchDefaultBrowser(uri);
        evt.Veto();
    }
    else if (uri.StartsWith("back:", &sData)) {
        w_browser->GoBack();
    }
    else if (uri.StartsWith("viewtrans:", &sData)) {
        wxStringTokenizer tokenizer(sData, ":");
        int i =0;
        int64 cat_id = -1;
        int64 sub_id = -1;
        int64 payee_id = -1;
        // categoryID, subcategoryID, payee_id
        //      subcategoryID = -2 means inlude all sub categories for the given category
        while ( tokenizer.HasMoreTokens() ) {
            switch (i++) {
            case 0:
                cat_id = std::stoll(tokenizer.GetNextToken().ToStdString());
                break;
            case 1:
                sub_id = std::stoll(tokenizer.GetNextToken().ToStdString());
                break;
            case 2:
                payee_id = std::stoll(tokenizer.GetNextToken().ToStdString());
                break;
            default:
                break;
            }
        }

        if (cat_id > 0) {
            std::vector<int64> sub_id_a;
            // include all sub categories
            if (sub_id == -2) {
                const CategoryData* cat_n = CategoryModel::instance().get_id_data_n(cat_id);
                for (const auto& sub_d : CategoryModel::instance().find_data_subtree_a(*cat_n)) {
                    sub_id_a.push_back(sub_d.m_id);
                }
            }
            sub_id_a.push_back(cat_id);
            m_rb->m_filter.setCategoryList(sub_id_a);
        }

        if (payee_id > 0) {
            wxArrayInt64 payees;
            payees.push_back(payee_id);
            m_rb->m_filter.setPayeeList(payees);
        }

        const wxString report = m_rb->m_filter.getHTML();
        const auto name = getVFname4print("repdetail", report);
        w_browser->LoadURL(name);
    }
    else if (uri.StartsWith("trxid:", &sData)) {
        long long transID = -1;
        if (sData.ToLongLong(&transID)) {
            const TrxData* trx_n = TrxModel::instance().get_id_data_n(transID);
            if (trx_n && trx_n->m_id > -1) {
                const AccountData* account = AccountModel::instance().get_id_data_n(trx_n->m_account_id);
                if (account) {
                    w_frame->selectNavTreeItem(account->m_name);
                    w_frame->setGotoAccountID(trx_n->m_account_id, { transID, 0 });
                    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
                    w_frame->GetEventHandler()->AddPendingEvent(event);
                }
            }
        }
    }
    else if (uri.StartsWith("trx:", &sData)) {
        long long transId = -1;
        if (sData.ToLongLong(&transId)) {
            TrxData* trx_n = TrxModel::instance().unsafe_get_id_data_n(transId);
            if (trx_n && trx_n->m_id > -1) {
                if (TrxModel::is_foreign(*trx_n)) {
                    const TrxLinkData* tl_n = TrxLinkModel::instance().get_trx_data_n(transId);
                    if (tl_n && tl_n->m_ref_type == StockModel::s_ref_type) {
                        TrxLinkData tl_d = *tl_n;
                        TrxShareDialog dlg(w_frame, &tl_d, trx_n);
                        if (dlg.ShowModal() == wxID_OK) {
                            m_rb->getHTMLText();
                            saveReportText();
                        }
                    }
                    else if (tl_n && tl_n->m_ref_type == AssetModel::s_ref_type) {
                        TrxLinkData tl_d = *tl_n;
                        AssetDialog dlg(w_frame, &tl_d, trx_n);
                        if (dlg.ShowModal() == wxID_OK) {
                            m_rb->getHTMLText();
                            saveReportText();
                        }
                    }
                }
                else {
                    TrxDialog dlg(w_frame, -1, {transId, false});
                    if (dlg.ShowModal() != wxID_CANCEL) {
                        m_rb->getHTMLText();
                        saveReportText();
                    }
                }
                const auto name = getVFname4print("rep", getReportBase()->getHTMLText());
                w_browser->LoadURL(name);
            }
        }
    }
    else if (uri.StartsWith("attachment:", &sData)) {
        RefTypeN ref_type = RefTypeN(sData.BeforeFirst('|'));
        long long ref_id;
        sData.AfterFirst('|').ToLongLong(&ref_id);

        if (ref_type.has_value() && ref_id > 0) {
            mmAttachmentManage::OpenAttachmentFromPanelIcon(w_frame, ref_type, ref_id);
            const auto name = getVFname4print("rep", getReportBase()->getHTMLText());
            w_browser->LoadURL(name);
        }
    }
    else if (uri.StartsWith("budget:", &sData)) {

        std::vector<std::string> parms;
        wxStringTokenizer tokenizer(sData, "|");
        while (tokenizer.HasMoreTokens()) {
            //"budget: " << estimateVal << "|" << CurrencyModel::instance().toString(actual, CurrencyModel::instance().get_base_data_n()) << "|" << cat_id << "|" << budget_year << "|" << month + 1;
            wxString token = tokenizer.GetNextToken();
            parms.push_back(std::string(token.mb_str()));

        }
        //format month 2 digits leading 0
        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << std::stoi(parms[4]);
        std::string formattedMonth = oss.str();

        //get yearId from year_name
        int64 bp_id_n = BudgetPeriodModel::instance().get_name_id_n(
            parms[3] + "-" + formattedMonth
        );

        //if bp_id_n doesn't exist then return
        if (bp_id_n < 0) {
            wxLogInfo("Monthly budget not found!");
            return;
        }

        //get model budget for yearID and cat_id
        BudgetModel::DataA budget_a = BudgetModel::instance().find(
            BudgetCol::BUDGETYEARID(bp_id_n),
            BudgetCol::CATEGID(std::stoll(parms[2]))
        );

        BudgetData budget_d;
        if (budget_a.empty()) {
            budget_d = BudgetData();
            budget_d.m_period_id   = bp_id_n;
            budget_d.m_category_id = std::stoll(parms[2]);
            budget_d.m_amount      = 0.0;
            BudgetModel::instance().add_data_n(budget_d);
        }
        else
            budget_d = budget_a[0];

        double estimated;
        CurrencyModel::instance().fromString(parms[0], estimated, CurrencyModel::instance().get_base_data_n());
        double actual;
        CurrencyModel::instance().fromString(parms[1], actual, CurrencyModel::instance().get_base_data_n());

        //open budgetEntry dialog
        BudgetEntryDialog dlg(w_frame, &budget_d,
            CurrencyModel::instance().toCurrency(estimated),
            CurrencyModel::instance().toCurrency(actual)
        );
        if (dlg.ShowModal() == wxID_OK) {
            //refresh report
            saveReportText();
            m_rb ->saveReportSettings();
        }
    }

    evt.Skip();
}

void ReportPanel::onYearChanged(wxCommandEvent& event)
{
    const auto i = event.GetString();
    wxLogDebug("-------- %s", i);
    saveReportText();
}

void ReportPanel::onBudgetChanged(wxCommandEvent& event)
{
    const auto i = event.GetString();
    wxLogDebug("-------- %s", i);
    saveReportText();
    m_rb->saveReportSettings();
}

void ReportPanel::onAccountChanged(wxCommandEvent& WXUNUSED(event))
{
    if (m_rb) {
        int sel = w_account_choice->GetSelection();
        if ((sel == 1) || (sel != m_rb->getAccountSelection())) {
            m_rb->setAccounts(sel, NavigatorTypes::instance().getAccountDbTypeFromName(w_account_choice->GetString(sel)));
            saveReportText();
            m_rb->saveReportSettings();
        }
    }
}

void ReportPanel::onStockChanged(wxCommandEvent& WXUNUSED(event))
{
    if (m_rb) {
        int sel = w_stocks_choice->GetSelection();
        if ((sel == 1) || (sel != m_rb->getStockSelection())) {
            m_rb->setStockName(w_stocks_choice->GetStringSelection());
            m_rb->setStockSelection(sel);
            saveReportText();
            saveFilterSettings();
            m_rb->saveReportSettings();
        }
    }
}

void ReportPanel::onFilterChanged(wxCommandEvent& WXUNUSED(event))
{
    if (m_rb) {
        saveReportText();
        saveFilterSettings();
        m_rb->saveReportSettings();
    }
}

void ReportPanel::onSelectionChanged(wxCommandEvent& WXUNUSED(event))
{
    if (m_rb) {
        saveReportText();
        m_rb->saveReportSettings();
    }
}

void ReportPanel::onSingleDateChanged(wxDateEvent& WXUNUSED(event))
{
    if (m_rb) {
        saveReportText();
        m_rb->saveReportSettings();
    }
}

void ReportPanel::onChartChanged(wxCommandEvent& WXUNUSED(event))
{
    if (!m_rb)
        return;

    int sel = w_chart_choice->GetSelection();
    if (sel == 1 || sel != m_rb->getChartSelection()) {
        m_rb->setChartSelection(sel);
        saveReportText();
        m_rb->saveReportSettings();
    }
}

void ReportPanel::onForwardMonthsChangedSpin(wxSpinEvent& WXUNUSED(event))
{
    if (!m_rb)
        return;

    int sel = w_forward_months->GetValue();
    if (sel != m_rb->getForwardMonths()) {
        m_rb->setForwardMonths(sel);
        saveReportText();
        m_rb->saveReportSettings();
    }
}

void ReportPanel::onForwardMonthsChangedText(wxCommandEvent& event)
{
    w_forward_months->SetValue(event.GetString());
    wxSpinEvent evt;
    onForwardMonthsChangedSpin(evt);
}

void ReportPanel::onShiftPressed(wxCommandEvent& event)
{
    if (!m_rb)
        return;

    m_shift = event.GetInt();
    m_rb->setDateSelection(m_shift);
    saveReportText();
}

void ReportPanel::onDateRangePopup(wxCommandEvent& event)
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
    event.Skip();
}

void ReportPanel::onDateRangeSelect(wxCommandEvent& event)
{
    int i = event.GetId() - ID_DATE_RANGE_MIN;
    if (i < 0 || i >= static_cast<int>(m_date_range_a.size())) {
        return;
    }

    // recreate m_date_range in order to reload parameters from setting,
    // refresh the date of today, and clear the default start/end dates
    m_date_range = mmDateRange2();
    m_date_range.setRange(m_date_range_a[i]);

    m_filter_id = JournalPanel::FILTER_ID_DATE_RANGE;
    updateFilter();
    saveFilterSettings();
}

void ReportPanel::onDateRangeEdit(wxCommandEvent& WXUNUSED(event))
{
    DateRangeManager dlg(this, DateRangeManager::TYPE_ID_REPORTING);
    if (dlg.ShowModal() != wxID_OK)
        return;

    loadDateRanges(&m_date_range_a, &m_date_range_m);

    if (m_filter_id == JournalPanel::FILTER_ID_DATE_RANGE) {
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

        updateFilter();
        saveFilterSettings();
    }
}

void ReportPanel::onStartEndDateChanged(wxDateEvent& event)
{
    wxObject* eo = event.GetEventObject();
    if (eo) {
        wxDateTime start_dateTime = w_start_date_picker->GetValue();
        wxDateTime end_dateTime = w_end_date_picker->GetValue();
        if (w_start_date_picker->isItMyDateControl(eo)) {
            //wxLogDebug("Start date changed to %s", start_dateTime.FormatISODate());
            if (start_dateTime > end_dateTime) {
                w_end_date_picker->SetValue(start_dateTime);
                wxLogDebug("End date changed to %s", start_dateTime.FormatISODate());
            }
        }
        else if (w_end_date_picker->isItMyDateControl(eo)) {
            if (start_dateTime > end_dateTime) {
                w_start_date_picker->SetValue(end_dateTime);
                wxLogDebug("Start date changed to %s", end_dateTime.FormatISODate());
            }
        }
    }

    m_filter_id = JournalPanel::FILTER_ID_DATE_PICKER;
    updateFilter();
    saveFilterSettings();

    if (m_rb) {
        saveReportText();
        m_rb->saveReportSettings();
    }
}

void ReportPanel::loadDateRanges(
    std::vector<mmDateRange2::Range>* date_range_a,
    int* date_range_m,
    bool all_ranges
) {
    date_range_a->clear();
    *date_range_m = -1;
    int src_i = 0;
    int src_m = PrefModel::instance().getReportingRangeM();
    for (const auto& range : PrefModel::instance().getReportingRangeA()) {
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
