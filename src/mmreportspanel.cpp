/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2017 James Higley

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

#include "mmreportspanel.h"
#include "attachmentdialog.h"
#include "mmex.h"
#include "mmframe.h"
#include "mmcheckingpanel.h"
#include "paths.h"
#include "platfdep.h"
#include "transdialog.h"
#include "util.h"
#include "reports/htmlbuilder.h"
#include "reports/reportbase.h"
#include "Model_Attachment.h"
#include "Model_Usage.h"
#include "Model_Budgetyear.h"
#include "Model_Infotable.h"
#include <wx/webviewfshandler.h>

class WebViewHandlerReportsPage : public wxWebViewHandler
{
public:
    WebViewHandlerReportsPage(mmReportsPanel *panel, const wxString& protocol)
        : wxWebViewHandler(protocol), m_reportPanel(panel)
    {
    }

    virtual ~WebViewHandlerReportsPage()
    {
    }

    virtual wxFSFile* GetFile (const wxString &uri)
    {
        mmGUIFrame* frame = m_reportPanel->m_frame;
        wxString sData;
        if (uri.StartsWith("trxid:", &sData))
        {
            long transID = -1;
            if (sData.ToLong(&transID)) {
                const Model_Checking::Data* transaction = Model_Checking::instance().get(transID);
                if (transaction)
                {
                    const Model_Account::Data* account = Model_Account::instance().get(transaction->ACCOUNTID);
                    if (account)
                    {
                        frame->setAccountNavTreeSection(account->ACCOUNTNAME);
                        frame->setGotoAccountID(transaction->ACCOUNTID, transID);
                        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
                        frame->GetEventHandler()->AddPendingEvent(evt);
                    }
                }
            }
        }
        else if (uri.StartsWith("trx:", &sData))
        {
            long transID = -1;
            if (sData.ToLong(&transID)) {
                const Model_Checking::Data* transaction = Model_Checking::instance().get(transID);
                if (transaction)
                {
                    mmTransDialog dlg(nullptr, -1, transID, 0);
                    if (dlg.ShowModal() == wxID_OK)
                    {
                        m_reportPanel->rb_->getHTMLText();
                    }
                    m_reportPanel->browser_->LoadURL(getURL(mmex::getReportFullFileName(this->GetName())));
                }
            }
        }
        if (uri.StartsWith("attachment:", &sData))
        {
            const wxString RefType = sData.BeforeFirst('|');
            const int RefId = wxAtoi(sData.AfterFirst('|'));

            if (Model_Attachment::instance().all_type().Index(RefType) != wxNOT_FOUND && RefId > 0)
            {
                mmAttachmentManage::OpenAttachmentFromPanelIcon(nullptr, RefType, RefId);
                m_reportPanel->browser_->LoadURL(getURL(mmex::getReportFullFileName(this->GetName())));
            }
        }

        return nullptr;
    }   
private:
    mmReportsPanel *m_reportPanel;
};

wxBEGIN_EVENT_TABLE(mmReportsPanel, wxPanel)
    EVT_CHOICE(ID_CHOICE_DATE_RANGE, mmReportsPanel::OnDateRangeChanged)
    EVT_CHOICE(ID_CHOICE_ACCOUNTS, mmReportsPanel::OnAccountChanged)
    EVT_DATE_CHANGED(wxID_ANY, mmReportsPanel::OnStartEndDateChanged)
    EVT_BUTTON(ID_PREV_REPORT, mmReportsPanel::OnPrevReport)
    EVT_BUTTON(ID_NEXT_REPORT, mmReportsPanel::OnNextReport)
    EVT_CHOICE(ID_CHOICE_CHART, mmReportsPanel::OnChartChanged)
wxEND_EVENT_TABLE()

mmReportsPanel::mmReportsPanel(
    mmPrintableBase* rb, bool cleanupReport, wxWindow *parent, mmGUIFrame* frame,
    wxWindowID winid, const wxPoint& pos,
    const wxSize& size, long style,
    const wxString& name )
    : m_date_ranges(nullptr)
    , m_cust_date(nullptr)
    , rb_(rb)
    , cleanup_(cleanupReport)
    , cleanupmem_(false)
    , m_frame(frame)
{
    m_all_date_ranges.push_back(new mmCurrentMonth());
    m_all_date_ranges.push_back(new mmCurrentMonthToDate());
    m_all_date_ranges.push_back(new mmLastMonth());
    m_all_date_ranges.push_back(new mmLast30Days());
    m_all_date_ranges.push_back(new mmLast90Days());
    m_all_date_ranges.push_back(new mmLast3Months());
    m_all_date_ranges.push_back(new mmLast12Months());
    m_all_date_ranges.push_back(new mmCurrentYear());
    m_all_date_ranges.push_back(new mmCurrentYearToDate());
    m_all_date_ranges.push_back(new mmLastYear());

    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);
    int month = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);

    m_all_date_ranges.push_back(new mmCurrentFinancialYear(day, month));
    m_all_date_ranges.push_back(new mmCurrentFinancialYearToDate(day, month));
    m_all_date_ranges.push_back(new mmLastFinancialYear(day, month));
    m_all_date_ranges.push_back(new mmAllTime());
    m_all_date_ranges.push_back(new mmLast365Days());

    Create(parent, winid, pos, size, style, name);
}

mmReportsPanel::~mmReportsPanel()
{
    if (cleanupmem_ && m_date_ranges)
    {
        for (unsigned int i = 0; i < m_date_ranges->GetCount(); i++)
        {
            int *id = reinterpret_cast<int*>(m_date_ranges->GetClientData(i));
            delete id;
        }
    }
    if (cleanup_ && rb_)
        delete rb_;
    std::for_each(m_all_date_ranges.begin(), m_all_date_ranges.end(), std::mem_fun(&mmDateRange::destroy));
    m_all_date_ranges.clear();
    if (m_cust_date)
        delete m_cust_date;
}

bool mmReportsPanel::Create(wxWindow *parent, wxWindowID winid
    , const wxPoint& pos, const wxSize& size, long style
    , const wxString& name)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);
    wxDateTime start = wxDateTime::UNow();

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    if (rb_)
    {
        wxString error;
        if (saveReportText(error))
            browser_->LoadURL(getURL(mmex::getReportFullFileName(rb_->file_name())));
        else
            browser_->SetPage(error, "");

        this->SetLabel(rb_->title());
    }
    Model_Usage::instance().pageview(this, (wxDateTime::UNow() - start).GetMilliseconds().ToLong());

    return true;
}

bool mmReportsPanel::saveReportText(wxString& error, bool initial)
{
    error = "";
    if (!rb_) return false;

    rb_->initial_report(initial);
    if (this->m_date_ranges)
    {
        int rp = rb_->report_parameters();
        if (rp & rb_->RepParams::DATE_RANGE)
        {
            mmDateRange* date = static_cast<mmDateRange*>
                (this->m_date_ranges->GetClientData(this->m_date_ranges->GetSelection()));
            if (date == nullptr)
            {
                if (m_cust_date == nullptr)
                {
                    wxDateTime begin_date;
                    wxDateTime end_date;
                    rb_->getDates(begin_date, end_date);
                    if (begin_date.IsValid() && end_date.IsValid())
                    {
                        m_cust_date = new mmSpecifiedRange(begin_date, end_date);
                        date = m_cust_date;
                        this->m_start_date->SetValue(begin_date);
                        this->m_end_date->SetValue(end_date);
                        m_start_date->Enable(true);
                        m_end_date->Enable(true);
                    }
                    else
                    {
                        // Reinitialize to first date selection
                        this->m_date_ranges->SetSelection(0);
                        date = *m_all_date_ranges.begin();
                        this->m_start_date->SetValue(date->start_date());
                        this->m_end_date->SetValue(date->end_date());
                        m_start_date->Enable(false);
                        m_end_date->Enable(false);
                    }
                }
                else
                    date = m_cust_date;
            }
            rb_->date_range(date, this->m_date_ranges->GetSelection());
        }
        else if (rp & (rb_->RepParams::BUDGET_DATES | rb_->RepParams::ONLY_YEARS))
            rb_->date_range(nullptr
                , *reinterpret_cast<int*>(this->m_date_ranges->GetClientData(this->m_date_ranges->GetSelection())));
    }

    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Report");
    json_writer.Key("name");
    json_writer.String(rb_->title().c_str());
    json_writer.Key("start");
    json_writer.String(wxDateTime::Now().FormatISOCombined().c_str());

    const auto file_name = rb_->file_name();
    wxLogDebug("Report File Name: %s", file_name);
    if (!Model_Report::outputReportFile(rb_->getHTMLText(), file_name))
        error = _("Error");

    json_writer.Key("end");
    json_writer.String(wxDateTime::Now().FormatISOCombined().c_str());
    json_writer.EndObject();

    Model_Usage::instance().AppendToUsage(json_buffer.GetString());

    return error.empty();
}

void mmReportsPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel(this, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer2->Add(itemPanel3, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizerHeader = new wxBoxSizer(wxHORIZONTAL);
    itemPanel3->SetSizer(itemBoxSizerHeader);

    wxStaticText* itemStaticText9 = new wxStaticText(itemPanel3
        , wxID_ANY, _("REPORTS"));
    itemStaticText9->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizerHeader->Add(itemStaticText9, 0, wxALL, 1);
    itemBoxSizerHeader->AddSpacer(30);

    if (rb_)
    {
        int rp = rb_->report_parameters();
        if (rp & rb_->RepParams::DATE_RANGE)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, "");
            itemStaticTextH1->SetFont(this->GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            itemStaticTextH1->SetLabel(_("Period:"));
            m_date_ranges = new wxChoice(itemPanel3, ID_CHOICE_DATE_RANGE);

            for (const auto & date_range: m_all_date_ranges)
            {
                m_date_ranges->Append(date_range->local_title(), date_range);
            }
            m_date_ranges->Append(_("Custom"), (mmDateRange *)nullptr);

            int sel_id = rb_->getDateSelection();
            if (sel_id < 0 || sel_id >= m_all_date_ranges.size()) {
                sel_id = 0;
            }
            m_date_ranges->SetSelection(sel_id);

            itemBoxSizerHeader->Add(m_date_ranges, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            const mmDateRange* date_range = m_all_date_ranges.at(sel_id);
            long date_style = wxDP_DROPDOWN | wxDP_SHOWCENTURY;
            m_start_date = new wxDatePickerCtrl(itemPanel3, ID_CHOICE_START_DATE
                , wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, date_style);
            m_start_date->SetValue(date_range->start_date());
            m_start_date->Enable(false);

            m_end_date = new wxDatePickerCtrl(itemPanel3, ID_CHOICE_END_DATE, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, date_style);
            m_end_date->SetValue(date_range->end_date());
            m_end_date->Enable(false);

            itemBoxSizerHeader->Add(m_start_date, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            itemBoxSizerHeader->Add(m_end_date, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(30);

            wxButton *btnPrev = new wxButton(itemPanel3, ID_PREV_REPORT, _("Previous Period"));
            btnPrev->SetToolTip(_("Previous Period"));
            itemBoxSizerHeader->Add(btnPrev, 0, wxRIGHT, 5);
            itemBoxSizerHeader->AddSpacer(5);
            wxButton *btnNext = new wxButton(itemPanel3, ID_NEXT_REPORT, _("Next Period"));
            btnNext->SetToolTip(_("Next Period"));
            itemBoxSizerHeader->Add(btnNext, 0, wxRIGHT, 5);
            itemBoxSizerHeader->AddSpacer(30);
        }
        else if (rp & rb_->RepParams::SINGLE_DATE)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, "");
            itemStaticTextH1->SetFont(this->GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            itemStaticTextH1->SetLabel(_("Date"));
            long date_style = wxDP_DROPDOWN | wxDP_SHOWCENTURY;
            m_start_date = new wxDatePickerCtrl(itemPanel3, ID_CHOICE_START_DATE
                , wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, date_style);
            m_start_date->SetValue(wxDateTime::Today());
            m_start_date->Enable(true);

            m_end_date = nullptr;

            itemBoxSizerHeader->Add(m_start_date, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(30);

            wxButton *btnPrev = new wxButton(itemPanel3, ID_PREV_REPORT, _("Previous Period"));
            btnPrev->SetToolTip(_("Previous Period"));
            itemBoxSizerHeader->Add(btnPrev, 0, wxRIGHT, 5);
            itemBoxSizerHeader->AddSpacer(5);
            wxButton *btnNext = new wxButton(itemPanel3, ID_NEXT_REPORT, _("Next Period"));
            btnNext->SetToolTip(_("Next Period"));
            itemBoxSizerHeader->Add(btnNext, 0, wxRIGHT, 5);
            itemBoxSizerHeader->AddSpacer(30);
        }
        else if (rp & (rb_->RepParams::BUDGET_DATES | rb_->RepParams::ONLY_YEARS))
        {
            cleanupmem_ = true;
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, "");
            itemStaticTextH1->SetFont(this->GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            itemStaticTextH1->SetLabel(_("Period:"));

            m_date_ranges = new wxChoice(itemPanel3, ID_CHOICE_DATE_RANGE
                , wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_SORT);

            int prev_selection = rb_->getDateSelection();
            int cur_selection = 0;
            bool sel_found = false;
            for (const auto& e : Model_Budgetyear::instance().all(Model_Budgetyear::COL_BUDGETYEARNAME))
            {
                const wxString& name = e.BUDGETYEARNAME;

                if ((rp & rb_->RepParams::ONLY_YEARS)
                    && name.length() >= 5) // Only add YEARS
                    continue;

                int id = e.BUDGETYEARID;
                m_date_ranges->Append(name, reinterpret_cast<void *>(new int(id)));

                if (!sel_found)
                {
                    if (prev_selection == id)
                        sel_found = true;
                    else
                        cur_selection++;
                }
            }
            if (!sel_found)
                m_date_ranges->SetSelection(m_date_ranges->GetCount() - 1); // Set to latest budget
            else
                m_date_ranges->SetSelection(cur_selection);
            rb_->date_range(nullptr, *reinterpret_cast<int*>(m_date_ranges->GetClientData(this->m_date_ranges->GetSelection())));

            itemBoxSizerHeader->Add(m_date_ranges, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(30);

            wxButton *btnPrev = new wxButton(itemPanel3, ID_PREV_REPORT, _("Previous Period"));
            btnPrev->SetToolTip(_("Previous Period"));
            itemBoxSizerHeader->Add(btnPrev, 0, wxRIGHT, 5);
            itemBoxSizerHeader->AddSpacer(5);
            wxButton *btnNext = new wxButton(itemPanel3, ID_NEXT_REPORT, _("Next Period"));
            btnNext->SetToolTip(_("Next Period"));
            itemBoxSizerHeader->Add(btnNext, 0, wxRIGHT, 5);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & rb_->RepParams::ACCOUNTS_LIST)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, "");
            itemStaticTextH1->SetFont(this->GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            itemStaticTextH1->SetLabel(_("Accounts: "));
            m_accounts = new wxChoice(itemPanel3, ID_CHOICE_ACCOUNTS);
            m_accounts->Append(_("All Accounts"));
            m_accounts->Append(_("Specific Accounts"));
            for (const auto& e : Model_Account::instance().TYPE_CHOICES)
            {
                if (e.first != Model_Account::INVESTMENT)
                    m_accounts->Append(wxGetTranslation(e.second));
            }
            m_accounts->SetSelection(rb_->getAccountSelection());

            itemBoxSizerHeader->Add(m_accounts, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & rb_->RepParams::CHART)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, "");
            itemStaticTextH1->SetFont(this->GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            itemStaticTextH1->SetLabel(_("Chart:"));
            m_chart = new wxChoice(itemPanel3, ID_CHOICE_CHART);
            m_chart->Append(_("Show"));
            m_chart->Append(_("Hide"));
            m_chart->SetSelection(rb_->getChartSelection());

            itemBoxSizerHeader->Add(m_chart, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }
    }

    browser_ = wxWebView::New(this, mmID_BROWSER);
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerReportsPage(this, "trxid")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerReportsPage(this, "trx")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerReportsPage(this, "attachment")));

    itemBoxSizer2->Add(browser_, 1, wxGROW|wxALL, 1);
}

void mmReportsPanel::PrintPage()
{
    browser_->Print();
}

void mmReportsPanel::OnDateRangeChanged(wxCommandEvent& WXUNUSED(event))
{
    if (rb_)
    {
        bool bGenReport = true;
        if (rb_->report_parameters() & rb_->RepParams::DATE_RANGE)
        {
            const mmDateRange* date_range = static_cast<mmDateRange*>
                (this->m_date_ranges->GetClientData(this->m_date_ranges->GetSelection()));
            if (date_range != nullptr)
            {
                this->m_start_date->SetValue(date_range->start_date());
                this->m_end_date->SetValue(date_range->end_date());
            }
            else
            {
                bGenReport = false;
            }
            m_start_date->Enable(!bGenReport);
            m_end_date->Enable(!bGenReport);
        }

        if (bGenReport)
        {
            wxString error;
            if (this->saveReportText(error, false)) {
                browser_->LoadURL(getURL(mmex::getReportFullFileName(rb_->file_name())));
            }
            else {
                browser_->SetPage(error, "");
            }
        }
    }
}

void mmReportsPanel::OnAccountChanged(wxCommandEvent& WXUNUSED(event))
{
    if (rb_)
    {
        int sel = m_accounts->GetSelection();
        if ((sel == 1) || (sel != rb_->getAccountSelection()))
        {
            wxString accountSelection = m_accounts->GetString(sel);
            rb_->accounts(sel, accountSelection);

            wxString error;
            if (this->saveReportText(error, false))
                browser_->LoadURL(getURL(mmex::getReportFullFileName(rb_->file_name())));
            else
                browser_->SetPage(error, "");
        }
    }
}

void mmReportsPanel::OnStartEndDateChanged(wxDateEvent& WXUNUSED(event))
{
    if (rb_)
    {
        if (m_cust_date)
            delete m_cust_date;
        m_cust_date = new mmSpecifiedRange(m_start_date->GetValue()
            , m_end_date ? m_end_date->GetValue() : wxDateTime::Today());

        wxString error;
        if (this->saveReportText(error, false))
            browser_->LoadURL(getURL(mmex::getReportFullFileName(rb_->file_name())));
        else
            browser_->SetPage(error, "");
    }
}

void mmReportsPanel::OnPrevReport(wxCommandEvent& event)
{
    int curSel = m_date_ranges->GetCurrentSelection();
    if (curSel > 0)
    {
        m_date_ranges->SetSelection(curSel - 1);
        OnDateRangeChanged(event);
    }
}

void mmReportsPanel::OnNextReport(wxCommandEvent& event)
{
    int curSel = m_date_ranges->GetCurrentSelection();
    if (curSel + 1 < static_cast<int>(m_date_ranges->GetStrings().GetCount()))
    {
        m_date_ranges->SetSelection(curSel + 1);
        OnDateRangeChanged(event);
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

            wxString error;
            if (this->saveReportText(error, false))
                browser_->LoadURL(getURL(mmex::getReportFullFileName(rb_->file_name())));
            else
                browser_->SetPage(error, "");
        }
    }
}

