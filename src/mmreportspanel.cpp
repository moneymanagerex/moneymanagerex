/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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
#include "model/allmodel.h"

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

    virtual wxFSFile* GetFile(const wxString &uri)
    {
        mmGUIFrame* frame = m_reportPanel->m_frame;
        wxString sData;
        if (uri.StartsWith("https:", &sData))
        {
            wxLaunchDefaultBrowser(uri);
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_REPORT_BUG);
            frame->GetEventHandler()->AddPendingEvent(evt);
        }
        else if (uri.StartsWith("trxid:", &sData))
        {
            long transID = -1;
            if (sData.ToLong(&transID)) {
                const Model_Checking::Data* transaction = Model_Checking::instance().get(transID);
                if (transaction && transaction->TRANSID > -1)
                {
                    const Model_Account::Data* account = Model_Account::instance().get(transaction->ACCOUNTID);
                    if (account) {
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
                if (transaction && transaction->TRANSID > -1)
                {
                    mmTransDialog dlg(nullptr, -1, transID, 0);
                    if (dlg.ShowModal() == wxID_OK)
                    {
                        m_reportPanel->rb_->getHTMLText();
                    }
                    m_reportPanel->browser_->LoadURL(getURL(mmex::getReportIndex()));
                }
            }
        }
        else if (uri.StartsWith("attachment:", &sData))
        {
            const wxString RefType = sData.BeforeFirst('|');
            const int RefId = wxAtoi(sData.AfterFirst('|'));

            if (Model_Attachment::instance().all_type().Index(RefType) != wxNOT_FOUND && RefId > 0)
            {
                mmAttachmentManage::OpenAttachmentFromPanelIcon(nullptr, RefType, RefId);
                m_reportPanel->browser_->LoadURL(getURL(mmex::getReportIndex()));
            }
        }

        return nullptr;
    }   
private:
    mmReportsPanel *m_reportPanel;
};

enum
{
    ID_CHOICE_DATE_RANGE = wxID_HIGHEST + 1,
};

wxBEGIN_EVENT_TABLE(mmReportsPanel, wxPanel)
    EVT_CHOICE(ID_CHOICE_DATE_RANGE, mmReportsPanel::OnDateRangeChanged)
wxEND_EVENT_TABLE()

mmReportsPanel::mmReportsPanel(
    mmPrintableBase* rb, bool cleanupReport, wxWindow *parent, mmGUIFrame* frame,
    wxWindowID winid, const wxPoint& pos,
    const wxSize& size, long style,
    const wxString& name )
    : rb_(rb)
    , m_date_ranges(nullptr)
    , cleanup_(cleanupReport)
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
    if (cleanup_ && rb_)
        delete rb_;
    std::for_each(m_all_date_ranges.begin(), m_all_date_ranges.end(), std::mem_fun(&mmDateRange::destroy));
    m_all_date_ranges.clear();
}

bool mmReportsPanel::Create(wxWindow *parent, wxWindowID winid
    , const wxPoint& pos, const wxSize& size, long style
    , const wxString& name)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    wxString error;
    if (saveReportText(error))
        browser_->LoadURL(getURL(mmex::getReportIndex()));
    else
        browser_->SetPage(error, "");

    Model_Usage::instance().pageview(this);

    return TRUE;
}

bool mmReportsPanel::saveReportText(wxString& error, bool initial)
{
    error = "";
    if (rb_)
    {
        rb_->initial_report(initial);
		if (this->m_date_ranges) {
			rb_->date_range(static_cast<mmDateRange*>(
				this->m_date_ranges->GetClientData(this->m_date_ranges->GetSelection()))
				, this->m_date_ranges->GetSelection());
		}

		StringBuffer json_buffer;
		Writer<StringBuffer> json_writer(json_buffer);

		json_writer.StartObject();
		json_writer.Key("module");
		json_writer.String("Report");
		json_writer.Key("name");
		json_writer.String(rb_->getReportTitle().c_str());

		const auto file_name = rb_->getFileName();
		wxLogDebug("Report File Name: %s", file_name);

		const auto time = wxDateTime::UNow();

		if (!Model_Report::outputReportFile(rb_->getHTMLText(), file_name))
			error = _("Error");

		json_writer.Key("seconds");
		json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
		json_writer.EndObject();

		Model_Usage::instance().AppendToUsage(json_buffer.GetString());
	}
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

    if (rb_)
    {
        if (rb_->RepParams::SINGLE_DATE & rb_->report_parameters())
        {
            m_date_ranges = new wxChoice(itemPanel3, ID_CHOICE_DATE_RANGE);

            for (const auto & date_range : m_all_date_ranges)
            {
                m_date_ranges->Append(date_range->local_title(), date_range);
            }
            m_date_ranges->SetSelection(rb_->getDateSelection());

            itemBoxSizerHeader->Add(m_date_ranges, 0, wxALL, 1);
            const mmDateRange* date_range = *m_all_date_ranges.begin();
            m_start_date = new wxDatePickerCtrl(itemPanel3, wxID_ANY);
            m_start_date->SetValue(date_range->start_date());
            m_start_date->Enable(false);

            m_end_date = new wxDatePickerCtrl(itemPanel3, wxID_ANY);
            m_end_date->SetValue(date_range->end_date());
            m_end_date->Enable(false);

            itemBoxSizerHeader->Add(m_start_date, 0, wxALL, 1);
            itemBoxSizerHeader->Add(m_end_date, 0, wxALL, 1);
        }
    }

    browser_ = wxWebView::New(this, mmID_BROWSER);
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerReportsPage(this, "trxid")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerReportsPage(this, "trx")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerReportsPage(this, "attachment")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerReportsPage(this, "https")));

    itemBoxSizer2->Add(browser_, 1, wxGROW|wxALL, 1);
}

void mmReportsPanel::PrintPage()
{
    browser_->Print();
}

void mmReportsPanel::OnDateRangeChanged(wxCommandEvent& /*event*/)
{
    const mmDateRange* date_range = static_cast<mmDateRange*>(this->m_date_ranges->GetClientData(this->m_date_ranges->GetSelection()));
    this->m_start_date->SetValue(date_range->start_date());
    this->m_end_date->SetValue(date_range->end_date());
    wxString error;
    if (this->saveReportText(error, false))
        browser_->LoadURL(getURL(mmex::getReportIndex()));
    else
        browser_->SetPage(error, "");
}
