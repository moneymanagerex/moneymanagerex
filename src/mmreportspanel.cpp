/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)
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

#include "assetdialog.h"
#include "attachmentdialog.h"
#include "filtertrans.h"
#include "mmreportspanel.h"
#include "images_list.h"
#include "mmex.h"
#include "mmframe.h"
#include "paths.h"
#include "platfdep.h"
#include "sharetransactiondialog.h"
#include "transdialog.h"
#include "daterangedialog.h"
#include "util.h"
#include "reports/htmlbuilder.h"
#include "model/allmodel.h"
#include <wx/wrapsizer.h>
#include "option.h"
#include <budgetentrydialog.h>
#include <vector>
#include <string>
#include <iomanip>

wxBEGIN_EVENT_TABLE(mmReportsPanel, wxPanel)
    EVT_CHOICE(ID_CHOICE_DATE_RANGE, mmReportsPanel::OnDateRangeChanged)
    EVT_CHOICE(ID_CHOICE_YEAR, mmReportsPanel::OnYearChanged)
    EVT_CHOICE(ID_CHOICE_BUDGET, mmReportsPanel::OnBudgetChanged)
    EVT_CHOICE(ID_CHOICE_ACCOUNTS, mmReportsPanel::OnAccountChanged)
    EVT_DATE_CHANGED(wxID_ANY, mmReportsPanel::OnStartEndDateChanged)
    EVT_TIME_CHANGED(wxID_ANY, mmReportsPanel::OnStartEndDateChanged)
    EVT_CHOICE(ID_CHOICE_CHART, mmReportsPanel::OnChartChanged)
    EVT_SPINCTRL(ID_CHOICE_FORWARD_MONTHS, mmReportsPanel::OnForwardMonthsChangedSpin)
    EVT_TEXT_ENTER(ID_CHOICE_FORWARD_MONTHS, mmReportsPanel::OnForwardMonthsChangedText)
    EVT_BUTTON(ID_FILTER_PERIOD, mmReportsPanel::OnPeriodSelectPopup)
    EVT_MENU(ID_EDIT_DATE_RANGES,  mmReportsPanel::onEditDateRanges)
    EVT_MENU_RANGE(
        ID_FILTER_DATE_MIN,
        ID_FILTER_DATE_MAX,
        mmReportsPanel::onFilterDateMenu)
    EVT_BUTTON(wxID_ANY, mmReportsPanel::OnShiftPressed)

wxEND_EVENT_TABLE()

mmReportsPanel::mmReportsPanel(
    mmPrintableBase* rb, bool cleanupReport, wxWindow *parent, mmGUIFrame* frame,
    wxWindowID winid, const wxPoint& pos,
    const wxSize& size, long style,
    const wxString& name)
    : m_frame(frame)
    , rb_(rb)
    , cleanup_(cleanupReport)
{
    Create(parent, winid, pos, size, style, name);
}

mmReportsPanel::~mmReportsPanel()
{
    if (cleanup_ && rb_) {
        delete rb_;
    }

    m_all_date_ranges.clear();
    clearVFprintedFiles("rep");
}

bool mmReportsPanel::Create(wxWindow *parent, wxWindowID winid
    , const wxPoint& pos, const wxSize& size, long style
    , const wxString& name)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    m_use_dedicated_filter = !Option::instance().getUseCombinedTransactionFilter();

    rb_->restoreReportSettings();

    CreateControls();
    if (m_use_dedicated_filter && (rb_->report_parameters() & rb_->RepParams::DATE_RANGE)) {
        loadFilterSettings();
        updateFilter();
        if (m_filter_id == mmCheckingPanel::FILTER_ID_DATE_PICKER) {
            m_start_date->SetValue(m_current_date_range.getDateS());
            m_end_date->SetValue(m_current_date_range.getDateT());
            m_bitmapDataPeriodFilterBtn->SetLabel(_t("Date range"));
        }
    }
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    saveReportText();

    int id = rb_->getReportId();
    this->SetLabel(id < 0 ? "Custom Report" : rb_->getReportTitle(false));

    return true;
}

bool mmReportsPanel::saveReportText(bool initial)
{
    if (!rb_) return false;

    rb_->initial_report(initial);

    if (m_use_dedicated_filter) {
        if (m_filter_id == mmCheckingPanel::FILTER_ID_DATE_RANGE || m_filter_id == mmCheckingPanel::FILTER_ID_DATE_PICKER) {
            mmDateRange* date_range = new mmDateRange();
            if (rb_->report_parameters() & mmPrintableBase::RepParams::DATE_RANGE)
            {
                wxDateTime td = m_start_date->GetValue();
                date_range->start_date(td);
                td = m_end_date->GetValue();
                date_range->end_date(td);
            }
            rb_->date_range(date_range, 0);
        }
    }
    else {
        if (m_date_ranges)
        {
            int selectedItem = m_date_ranges->GetSelection();
            wxASSERT(selectedItem >= 0 && selectedItem < static_cast<int>(m_date_ranges->GetCount()));

            int rp = rb_->report_parameters();
            if (rp & rb_->RepParams::DATE_RANGE)
            {
                mmDateRange* date_range = static_cast<mmDateRange*>(m_date_ranges->GetClientData(selectedItem));

                if (date_range->title() == "Custom")
                {
                    wxDateTime begin_date = m_start_date->GetValue();
                    wxDateTime end_date = m_end_date->GetValue();
                    date_range->start_date(begin_date);
                    date_range->end_date(end_date);
                    m_start_date->Enable();
                    m_end_date->Enable();
                }
                rb_->date_range(date_range, selectedItem);
            }

            if (rp & (rb_->RepParams::BUDGET_DATES | rb_->RepParams::ONLY_YEARS))
            {
                wxString id_str = "0";
                wxStringClientData* obj =
                    static_cast<wxStringClientData*>(m_date_ranges->GetClientObject(selectedItem));
                if (obj) id_str = obj->GetData();
                int64 id = std::stoll(id_str.ToStdString());
                rb_->setSelection(id);
            }
        }
    }
    /**/
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String(wxTRANSLATE("Report"));
    json_writer.Key("name");
    json_writer.String(rb_->getReportTitle(false).utf8_str());

    const auto time = wxDateTime::UNow();

    const auto& name = getVFname4print("rep", rb_->getHTMLText());
    browser_->LoadURL(name);

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    const auto t = wxString::FromUTF8(json_buffer.GetString());
    wxLogDebug("%s", t);
    Model_Usage::instance().AppendToUsage(t);
    Model_Usage::instance().pageview(this, rb_, (wxDateTime::UNow() - time).GetMilliseconds().ToLong());

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
void mmReportsPanel::loadFilterSettings() {
    m_date_range_a.clear();
    m_date_range_m = -1;
    int src_i = 0;
    int src_m = Option::instance().getCheckingRangeM();
    for (const auto& spec : Option::instance().getCheckingRangeA()) {
        if (m_date_range_a.size() > ID_FILTER_DATE_MAX - ID_FILTER_DATE_MIN) {
            break;
        }
        if (src_i == src_m) {
            m_date_range_m = m_date_range_a.size();
        }
        if (!spec.hasPeriodS()) {
            m_date_range_a.push_back(spec);
        }
        src_i++;
    }
    if (m_date_range_m < 0) {
        m_date_range_m = m_date_range_a.size();
    }

    // for the moment use bank account filter, tbd.
    Document j_doc = Model_Infotable::instance().getJdoc("CHECK_FILTER_ALL" , "{}");

    int fid = 0;
    if (JSON_GetIntValue(j_doc, "FILTER_ID", fid)) {
        m_filter_id = static_cast<mmCheckingPanel::FILTER_ID>(fid);
    }
    if (m_filter_id == mmCheckingPanel::FILTER_ID_DATE_RANGE) {
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
    }
    else if (m_filter_id == mmCheckingPanel::FILTER_ID_DATE_PICKER) {
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
}

void mmReportsPanel::saveFilterSettings() {
    if (m_use_dedicated_filter) {
        wxString key = "CHECK_FILTER_ALL";
        Document j_doc = Model_Infotable::instance().getJdoc(key, "{}");
        Model_Infotable::saveFilterInt(j_doc, "FILTER_ID", m_filter_id);
        Model_Infotable::saveFilterString(j_doc, "FILTER_NAME", mmCheckingPanel::getFilterName(m_filter_id));
        Model_Infotable::saveFilterString(j_doc, "FILTER_DATE", m_current_date_range.getSpec().getName());
        if (m_start_date) {
            Model_Infotable::saveFilterString(j_doc, "FILTER_DATE_BEGIN", m_start_date->GetValue().IsValid() ? m_start_date->GetValue().FormatISODate() : "");
        }
        if (m_end_date) {
            Model_Infotable::saveFilterString(j_doc, "FILTER_DATE_END", m_end_date->GetValue().IsValid() ? m_end_date->GetValue().FormatISODate() : "");
        }
        Model_Infotable::instance().setJdoc(key, j_doc);
    }
}

void mmReportsPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel(this, wxID_ANY);
    itemBoxSizer2->Add(itemPanel3, 0, wxGROW | wxALL, 0);

    wxWrapSizer* itemBoxSizerHeader = new wxWrapSizer();
    itemPanel3->SetSizer(itemBoxSizerHeader);

    wxStaticText* itemStaticText9 = new wxStaticText(itemPanel3, wxID_ANY, "");
    itemBoxSizerHeader->Add(itemStaticText9, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    int sel_id = -1;

    if (rb_)
    {
        int rp = rb_->report_parameters();

        if (rp == 0) {
            itemPanel3->SetMinSize(wxSize(0, 0));
            itemPanel3->Fit();
        }

        if (rp & rb_->RepParams::DATE_RANGE)
        {
            if (m_use_dedicated_filter) {

                m_bitmapDataPeriodFilterBtn = new wxButton(itemPanel3, ID_FILTER_PERIOD, _tu("Period…"));
                m_bitmapDataPeriodFilterBtn->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
                m_bitmapDataPeriodFilterBtn->SetMinSize(
                    wxSize(200 + Option::instance().getIconSize() * 2, -1)
                );
                itemBoxSizerHeader->Add(m_bitmapDataPeriodFilterBtn, g_flagsH);
                itemBoxSizerHeader->AddSpacer(5);
            }
            else {
                wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                    , wxID_ANY, _t("Period:"));
                mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
                itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
                itemBoxSizerHeader->AddSpacer(5);

                m_date_ranges = new wxChoice(itemPanel3, ID_CHOICE_DATE_RANGE);
                m_date_ranges->SetName("DateRanges");
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentMonth()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentMonthToDate()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLastMonth()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast30Days()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast90Days()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast3Months()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast12Months()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentYear()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentYearToDate()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLastYear()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLastYearBefore()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentFinancialYear()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentFinancialYearToDate()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLastFinancialYear()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmAllTime()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast365Days()));
                m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmSpecifiedRange(wxDate::Today().SetDay(1), wxDateTime(23,59,59,999))));

                for (const auto & date_range : m_all_date_ranges) {
                    m_date_ranges->Append(date_range.get()->local_title(), date_range.get());
                }

                sel_id = rb_->getDateSelection().GetValue();
                if (sel_id < 0 || static_cast<size_t>(sel_id) >= m_all_date_ranges.size()) {
                    sel_id = 0;
                }
                m_date_ranges->SetSelection(sel_id);

                itemBoxSizerHeader->Add(m_date_ranges, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
                itemBoxSizerHeader->AddSpacer(5);
            }

            m_start_date = new mmDatePickerCtrl(itemPanel3, ID_CHOICE_START_DATE
                , wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
            itemBoxSizerHeader->Add(m_start_date->mmGetLayoutWithTime(), 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);

            m_end_date = new mmDatePickerCtrl(itemPanel3, ID_CHOICE_END_DATE
                , wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
            itemBoxSizerHeader->Add(m_end_date->mmGetLayoutWithTime(), 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);

            if (!m_use_dedicated_filter) {
                wxSharedPtr<mmDateRange> date_range = m_all_date_ranges.at(sel_id);
                m_start_date->SetValue(date_range.get()->start_date());
                m_start_date->Enable(false);
                m_end_date->SetValue(date_range.get()->end_date());
                m_end_date->Enable(false);
            }
        }
        else if (rp & rb_->RepParams::SINGLE_DATE)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, _t("Date:"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            long date_style = wxDP_DROPDOWN | wxDP_SHOWCENTURY;
            m_start_date = new mmDatePickerCtrl(itemPanel3, ID_CHOICE_START_DATE
                , wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, date_style);
            m_start_date->SetValue(wxDateTime::Today());
            m_start_date->Enable(true);

            itemBoxSizerHeader->Add(m_start_date, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }
        else if (rp & rb_->RepParams::MONTHES)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, _t("Date:"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);

            mmDateYearMonth* up_down_month = new mmDateYearMonth(itemPanel3);
            up_down_month->Connect(wxEVT_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmDateYearMonth::OnButtonPress), nullptr, this);
            rb_->setSelection(m_shift);

            itemBoxSizerHeader->Add(up_down_month, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & rb_->RepParams::TIME)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, _t("Time:"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);

            m_time = new wxTimePickerCtrl(itemPanel3, ID_CHOICE_TIME);

            itemBoxSizerHeader->Add(m_time, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & rb_->RepParams::ONLY_YEARS)
        {
            cleanupmem_ = true;
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, _t("Year:"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);

            m_date_ranges = new wxChoice(itemPanel3, ID_CHOICE_YEAR, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_SORT);

            const int y = wxDateTime::Today().GetYear();
            for (int i = y - 100; i <= y + 100; i++)
            {
                const wxString name = wxString::Format("%i", i);
                m_date_ranges->Append(name, new wxStringClientData(name));
            }

            m_date_ranges->SetStringSelection(wxString::Format("%i", y));

            itemBoxSizerHeader->Add(m_date_ranges, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }
        else if (rp & rb_->RepParams::BUDGET_DATES)
        {
            cleanupmem_ = true;
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, _t("Budget:"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);

            m_date_ranges = new wxChoice(itemPanel3, ID_CHOICE_BUDGET, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_SORT);

            int64 sel_id = rb_->getDateSelection();
            wxString sel_name;
            for (const auto& e : Model_Budgetyear::instance().all(Model_Budgetyear::COL_BUDGETYEARNAME))
            {
                const wxString& name = e.BUDGETYEARNAME;

                if (rb_->getReportId() == mmPrintableBase::Reports::BudgetCategorySummary || name.length() == 4) // Only years for performance report
                {
                    m_date_ranges->Append(name, new wxStringClientData(wxString::Format("%lld", e.BUDGETYEARID)));
                    if (sel_id == e.BUDGETYEARID)
                        sel_name = e.BUDGETYEARNAME;
                }
            }

            if (!sel_name.IsEmpty())
                m_date_ranges->SetStringSelection(sel_name);
            else
                m_date_ranges->SetSelection(0);

            itemBoxSizerHeader->Add(m_date_ranges, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & rb_->RepParams::ACCOUNTS_LIST)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3, wxID_ANY, _t("Accounts:"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            m_accounts = new wxChoice(itemPanel3, ID_CHOICE_ACCOUNTS);
            m_accounts->Append(_t("All Accounts"));
            m_accounts->Append(_tu("Specific Accounts…"));
            for (int i = 0; i < Model_Account::TYPE_ID_size; ++i) {
                wxString type = Model_Account::type_name(i);
                m_accounts->Append(wxGetTranslation(type), new wxStringClientData(type));
            }
            m_accounts->SetSelection(rb_->getAccountSelection());

            itemBoxSizerHeader->Add(m_accounts, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & rb_->RepParams::FORWARD_MONTHS)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, _t("Future Months:"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            m_forwardMonths = new wxSpinCtrl(itemPanel3, ID_CHOICE_FORWARD_MONTHS
                ,wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER);
            m_forwardMonths->SetRange(1, 120);
            m_forwardMonths->SetValue(rb_->getForwardMonths());
            itemBoxSizerHeader->Add(m_forwardMonths, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & rb_->RepParams::CHART)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, _t("Chart:"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            m_chart = new wxChoice(itemPanel3, ID_CHOICE_CHART);
            m_chart->Append(_t("Show"));
            m_chart->Append(_t("Hide"));
            m_chart->SetSelection(rb_->getChartSelection());

            itemBoxSizerHeader->Add(m_chart, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }
    }

    browser_ = wxWebView::New();
#ifdef __WXMAC__
    // With WKWebView handlers need to be registered before creation
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
    browser_->Create(this, mmID_BROWSER);
#else
    browser_->Create(this, mmID_BROWSER);
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
#endif
    Bind(wxEVT_WEBVIEW_NEWWINDOW, &mmReportsPanel::OnNewWindow, this, mmID_BROWSER);

    itemBoxSizer2->Add(browser_, 1, wxGROW | wxALL, 1);
}

void mmReportsPanel::PrintPage()
{
    browser_->Print();
}

void mmReportsPanel::OnYearChanged(wxCommandEvent& event)
{
    const auto i = event.GetString();
    wxLogDebug("-------- %s", i);
    saveReportText(false);
}

void mmReportsPanel::OnBudgetChanged(wxCommandEvent& event)
{
    const auto i = event.GetString();
    wxLogDebug("-------- %s", i);
    saveReportText(false);
    rb_->setReportSettings();
}


void mmReportsPanel::OnDateRangeChanged(wxCommandEvent& WXUNUSED(event))
{
    auto i = this->m_date_ranges->GetSelection();
    const mmDateRange* date_range = static_cast<mmDateRange*>(this->m_date_ranges->GetClientData(i));
    if (date_range)
    {
        m_start_date->Enable(false);
        this->m_start_date->SetValue(date_range->start_date());
        m_end_date->Enable(false);
        this->m_end_date->SetValue(date_range->end_date());
        rb_->setSelection(i);
        rb_->setReportSettings();
    }
    saveReportText(false);
}

void mmReportsPanel::OnAccountChanged(wxCommandEvent& WXUNUSED(event))
{
    if (rb_)
    {
        int sel = m_accounts->GetSelection();
        if ((sel == 1) || (sel != rb_->getAccountSelection()))
        {
            wxString accountSelection;
            wxStringClientData* type_obj = static_cast<wxStringClientData *>(m_accounts->GetClientObject(sel));
            if (type_obj) {
                accountSelection = type_obj->GetData();
            }
            rb_->setAccounts(sel, accountSelection);

            saveReportText(false);
            rb_->setReportSettings();
        }
    }
}

void mmReportsPanel::OnStartEndDateChanged(wxDateEvent& event)
{
    if (m_use_dedicated_filter) {
        wxObject* eo = event.GetEventObject();
        if (eo) {
            if (m_start_date->isItMyDateControl(eo)) {
                //wxLogDebug("Start date changed to %s", m_start_date->GetValue().FormatISODate());
                if (m_start_date->GetValue() > m_end_date->GetValue()) {
                    m_end_date->SetValue(m_start_date->GetValue());
                    wxLogDebug("End date changed to %s", m_start_date->GetValue().FormatISODate());
                }
            }
            else if (m_end_date->isItMyDateControl(eo)) {
                if (m_start_date->GetValue() > m_end_date->GetValue()) {
                    m_start_date->SetValue(m_end_date->GetValue());
                    wxLogDebug("Start date changed to %s", m_start_date->GetValue().FormatISODate());
                }
            }
        }

        m_filter_id = mmCheckingPanel::FILTER_ID_DATE_PICKER;
        updateFilter();
        saveFilterSettings();
    }

    if (rb_) {
        saveReportText(false);
        rb_->setReportSettings();
    }
}

void mmReportsPanel::OnChartChanged(wxCommandEvent& WXUNUSED(event))
{
    if (rb_)
    {
        int sel = m_chart->GetSelection();
        if ((sel == 1) || (sel != rb_->getChartSelection()))
        {
            rb_->chart(sel);
            saveReportText(false);
            rb_->setReportSettings();
        }
    }
}

void mmReportsPanel::OnForwardMonthsChangedSpin(wxSpinEvent& WXUNUSED(event))
{
    if (rb_)
    {
        int sel = m_forwardMonths->GetValue();
        if (sel != rb_->getForwardMonths())
        {
            rb_->setForwardMonths(sel);
            saveReportText(false);
            rb_->setReportSettings();
        }
    }
}

void mmReportsPanel::OnForwardMonthsChangedText(wxCommandEvent& event)
{
    m_forwardMonths->SetValue(event.GetString());
    wxSpinEvent evt;
    OnForwardMonthsChangedSpin(evt);
}


void mmReportsPanel::OnShiftPressed(wxCommandEvent& event)
{
    if (rb_)
    {
        m_shift = event.GetInt();
        rb_->setSelection(m_shift);
        saveReportText(false);
    }
}

void mmReportsPanel::OnNewWindow(wxWebViewEvent& evt)
{
    const wxURI escapedURI(evt.GetURL());
    const wxString uri = escapedURI.BuildUnescapedURI();
    wxString sData;

    if (rb_->report_parameters() & rb_->RepParams::DATE_RANGE)
    {
        auto idx = this->m_date_ranges->GetSelection();
        const mmDateRange* date_range = static_cast<mmDateRange*>(this->m_date_ranges->GetClientData(idx));
        if (date_range)
        {
            this->m_start_date->SetValue(date_range->start_date());
            this->m_end_date->SetValue(date_range->end_date());
            rb_->setSelection(idx);
            rb_->setReportSettings();
            rb_->m_filter.setDateRange(date_range->start_date(), date_range->end_date());
        }
    }
    wxRegEx pattern(R"(^(https?:)|(file:)\/\/)");
    if (pattern.Matches(uri))
    {
        wxLaunchDefaultBrowser(uri);
    } else if (uri.StartsWith("back:", &sData))
    {
        browser_->GoBack();
    } else if (uri.StartsWith("viewtrans:", &sData))
    {
        wxStringTokenizer tokenizer(sData, ":");
        int i =0;
        int64 catID = -1;
        int64 subCatID = -1;
        int64 payeeID = -1;
        // categoryID, subcategoryID, payeeID
        //      subcategoryID = -2 means inlude all sub categories for the given category
        while ( tokenizer.HasMoreTokens() )
        {
            switch (i++) {
            case 0:
                catID = std::stoll(tokenizer.GetNextToken().ToStdString());
                break;
            case 1:
                subCatID = std::stoll(tokenizer.GetNextToken().ToStdString());
                break;
            case 2:
                payeeID = std::stoll(tokenizer.GetNextToken().ToStdString());
                break;
            default:
                break;
            }
        }

        if (catID > 0)
        {
            std::vector<int64> cats;
            if (-2 == subCatID) // include all sub categories
            {
                for (const auto& subCategory : Model_Category::sub_tree(Model_Category::instance().get(catID)))
                {
                    cats.push_back(subCategory.CATEGID);
                }
            }
            cats.push_back(catID);
            rb_->m_filter.setCategoryList(cats);
        }

        if (payeeID > 0)
        {
            wxArrayInt64 payees;
            payees.push_back(payeeID);
            rb_->m_filter.setPayeeList(payees);
        }

        const wxString report = rb_->m_filter.getHTML();
        const auto name = getVFname4print("repdetail", report);
        browser_->LoadURL(name);
    }
    else if (uri.StartsWith("trxid:", &sData))
    {
        long long transID = -1;
        if (sData.ToLongLong(&transID)) {
            const Model_Checking::Data* transaction = Model_Checking::instance().get(transID);
            if (transaction && transaction->TRANSID > -1)
            {
                const Model_Account::Data* account = Model_Account::instance().get(transaction->ACCOUNTID);
                if (account) {
                    m_frame->setNavTreeAccount(account->ACCOUNTNAME);
                    m_frame->setGotoAccountID(transaction->ACCOUNTID, { transID, 0 });
                    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
                    m_frame->GetEventHandler()->AddPendingEvent(event);
                }
            }
        }
    }
    else if (uri.StartsWith("trx:", &sData))
    {
        long long transId = -1;
        if (sData.ToLongLong(&transId))
        {
            Model_Checking::Data* transaction = Model_Checking::instance().get(transId);
            if (transaction && transaction->TRANSID > -1)
            {
                if (Model_Checking::foreignTransaction(*transaction))
                {
                    Model_Translink::Data translink = Model_Translink::TranslinkRecord(transId);
                    if (translink.LINKTYPE == Model_Stock::refTypeName)
                    {
                        ShareTransactionDialog dlg(m_frame, &translink, transaction);
                        if (dlg.ShowModal() == wxID_OK)
                        {
                            rb_->getHTMLText();
                            saveReportText();
                        }
                    }
                    else
                    {
                        mmAssetDialog dlg(m_frame, &translink, transaction);
                        if (dlg.ShowModal() == wxID_OK)
                        {
                            rb_->getHTMLText();
                            saveReportText();
                        }
                    }
                }
                else
                {
                    mmTransDialog dlg(m_frame, -1, {transId, false});
                    if (dlg.ShowModal() != wxID_CANCEL)
                    {
                        rb_->getHTMLText();
                        saveReportText();
                    }
                }
                const auto name = getVFname4print("rep", getPrintableBase()->getHTMLText());
                browser_->LoadURL(name);
            }
        }
    }
    else if (uri.StartsWith("attachment:", &sData))
    {
        const wxString RefType = sData.BeforeFirst('|');
        long long refId;
        sData.AfterFirst('|').ToLongLong(&refId);

        if (Model_Attachment::reftype_id(RefType) != -1 && refId > 0)
        {
            mmAttachmentManage::OpenAttachmentFromPanelIcon(m_frame, RefType, refId);
            const auto name = getVFname4print("rep", getPrintableBase()->getHTMLText());
            browser_->LoadURL(name);
        }
    }
    else if (uri.StartsWith("budget:", &sData))
    {

        std::vector<std::string> parms;
        wxStringTokenizer tokenizer(sData, "|");
        while (tokenizer.HasMoreTokens())
        {
            //"budget: " << estimateVal << "|" << Model_Currency::toString(actual, Model_Currency::GetBaseCurrency()) << "|" << catID << "|" << budget_year << "|" << month + 1;
            wxString token = tokenizer.GetNextToken();
            parms.push_back(std::string(token.mb_str()));

        }
        //format month 2 digits leading 0
        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << std::stoi(parms[4]);
        std::string formattedMonth = oss.str();

        //get yearId from year_name
        int64 budgetYearID = Model_Budgetyear::instance().Get(parms[3] + "-" + formattedMonth);

        //if budgetYearID doesn't exist then return
        if (budgetYearID == -1)
        {
            wxLogInfo("Monthly budget not found!");
            return;
        }

        //get model budget for yearID and catID
        Model_Budget::Data_Set budget = Model_Budget::instance().find(Model_Budget::BUDGETYEARID(budgetYearID), Model_Budget::CATEGID(std::stoll(parms[2])));

        Model_Budget::Data* entry = 0;
        if (budget.empty())
        {
            entry = Model_Budget::instance().create();
            entry->BUDGETYEARID = budgetYearID;
            entry->CATEGID = std::stoll(parms[2]);
            entry->PERIOD = "";
            entry->AMOUNT = 0.0;
            entry->ACTIVE = 1;
            Model_Budget::instance().save(entry);
        }
        else
            entry = &budget[0];

        double estimated;
        Model_Currency::fromString(parms[0], estimated, Model_Currency::GetBaseCurrency());
        double actual;
        Model_Currency::fromString(parms[1], actual, Model_Currency::GetBaseCurrency());

        //open budgetEntry dialog
        mmBudgetEntryDialog dlg(m_frame, entry, Model_Currency::toCurrency(estimated), Model_Currency::toCurrency(actual));
        if (dlg.ShowModal() == wxID_OK)
        {
            //refresh report
            saveReportText(false);
            rb_ ->setReportSettings();

        }
    }

    evt.Skip();
}

void mmReportsPanel::OnPeriodSelectPopup(wxCommandEvent& event)
{
    wxMenu menu;
    int i = 0;
    while (i < m_date_range_m) {
        menu.Append(ID_FILTER_DATE_MIN + i, m_date_range_a[i].getName());
        i++;
    }

    if (i +1 < static_cast<int>(m_date_range_a.size())) { //only add separator if there are more entries
        menu.AppendSeparator();
    }
    if (i < static_cast<int>(m_date_range_a.size())) {
        wxMenu* menu_more(new wxMenu);
        menu.AppendSubMenu(menu_more, _tu("More date ranges…"));
        while (i < static_cast<int>(m_date_range_a.size())) {
            menu_more->Append(ID_FILTER_DATE_MIN + i, m_date_range_a[i].getName());
            i++;
        }
    }
    menu.AppendSeparator();
    menu.Append(ID_EDIT_DATE_RANGES, _tu("Edit date ranges…"));

    PopupMenu(&menu);
    event.Skip();
}

void mmReportsPanel::onFilterDateMenu(wxCommandEvent& event)
{
    int i = event.GetId() - ID_FILTER_DATE_MIN;
    if (i < 0 || i >= static_cast<int>(m_date_range_a.size())) {
        return;
    }

    m_current_date_range = DateRange2();
    m_current_date_range.setSpec(m_date_range_a[i]);

    m_filter_id = mmCheckingPanel::FILTER_ID_DATE_RANGE;
    updateFilter();
    saveFilterSettings();
}

void mmReportsPanel::updateFilter()
{
    if (m_use_dedicated_filter) {
        if (m_filter_id == mmCheckingPanel::FILTER_ID_DATE_RANGE) {
            m_bitmapDataPeriodFilterBtn->SetLabel(m_current_date_range.getName());
            m_bitmapDataPeriodFilterBtn->SetBitmap(mmBitmapBundle((m_current_date_range.getName() != m_date_range_a[0].getName() ? png::TRANSFILTER_ACTIVE : png::TRANSFILTER), mmBitmapButtonSize));

            m_start_date->SetValue(m_current_date_range.checking_start().IsValid() ? m_current_date_range.checking_start() : wxDateTime(static_cast <time_t>(0)));
            m_end_date->SetValue(m_current_date_range.checking_end().IsValid() ? m_current_date_range.checking_end() : wxDateTime::Today());
        }
        else if (m_filter_id == mmCheckingPanel::FILTER_ID_DATE_PICKER) {
            m_bitmapDataPeriodFilterBtn->SetLabel(_t("Date range"));
            m_bitmapDataPeriodFilterBtn->SetBitmap(mmBitmapBundle(png::TRANSFILTER_ACTIVE, mmBitmapButtonSize));
        }
    }
}

void mmReportsPanel::onEditDateRanges(wxCommandEvent& WXUNUSED(event))
{
    mmDateRangeDialog dlg(this, &m_date_range_a, &m_date_range_m);
    if (dlg.ShowModal() == wxID_OK) {
        if (m_date_range_a.size() == 0) {
            int src_i = 0;
            int src_m = Option::instance().getCheckingRangeM();
            for (const auto& spec : Option::instance().getCheckingRangeA()) {
                if (m_date_range_a.size() > ID_FILTER_DATE_MAX - ID_FILTER_DATE_MIN) {
                    break;
                }
                if (src_i == src_m) {
                    m_date_range_m = m_date_range_a.size();
                }
                if (!spec.hasPeriodS()) {
                    m_date_range_a.push_back(spec);
                }
                src_i++;
            }
        }
        // Verify if current filter is still valid otherwise reset to "ALL"
        if (m_use_dedicated_filter && m_filter_id == mmCheckingPanel::FILTER_ID_DATE_RANGE) {
            wxString curname = m_current_date_range.getName();
            bool isDeleted = true;
            for (const auto& spec : m_date_range_a) {
                if (spec.getName() == curname) {
                    isDeleted = false;
                    break;
                }
            }
            if (isDeleted) {
                m_current_date_range = DateRange2();
                m_current_date_range.setSpec(m_date_range_a[0]);

                m_filter_id = mmCheckingPanel::FILTER_ID_DATE_RANGE;
                updateFilter();
                saveFilterSettings();
            }
        }
    }
}
