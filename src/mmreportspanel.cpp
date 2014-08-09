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
#include "platfdep.h"
#include "mmcheckingpanel.h"
#include "util.h"
#include "reports/htmlbuilder.h"
#include "mmex.h"
#include "mmframe.h"
#include "transdialog.h"
#include "paths.h"
#include "webserver.h"
#include "model/Model_Account.h"
#include "model/Model_Checking.h"
#include "model/Model_Usage.h"

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
                if (transaction)
                {
                    mmTransDialog dlg(nullptr, -1, transID);
                    if (dlg.ShowModal() == wxID_OK)
                    {
                        m_reportPanel->rb_->getHTMLText();
                    }
                    m_reportPanel->browser_->LoadURL(getURL(mmex::getReportIndex()));
                }
            }
        }

        return nullptr;
    }   
private:
    mmReportsPanel *m_reportPanel;
};

BEGIN_EVENT_TABLE(mmReportsPanel, wxPanel)
END_EVENT_TABLE()

mmReportsPanel::mmReportsPanel(
        mmPrintableBase* rb, bool cleanupReport, wxWindow *parent, mmGUIFrame* frame,
        wxWindowID winid, const wxPoint& pos,
        const wxSize& size, long style,
        const wxString& name )
: rb_(rb)
, cleanup_(cleanupReport)
, m_frame(frame)
{
    Create(parent, winid, pos, size, style, name);
}

mmReportsPanel::~mmReportsPanel()
{
    if (cleanup_ && rb_)
        delete rb_;
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

    getReportText();
    browser_->LoadURL(getURL(mmex::getReportIndex()));
    return TRUE;
}

wxString mmReportsPanel::getReportText()
{
    htmlreport_ = "coming soon..."; //TODO: ??
    if (rb_)
    {
        json::Object o;
        o["module"] = json::String("Report");
        o["name"] = json::String(rb_->title().ToStdString());
        o["start"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdString());

        htmlreport_ = rb_->getHTMLText();

        wxFileOutputStream index_output(mmex::getReportIndex());
        wxTextOutputStream index_file(index_output);
        index_file << htmlreport_;
        index_output.Close();
        o["end"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdString());
        Model_Usage::instance().append(o);
    }
    return htmlreport_;
}

void mmReportsPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel(this, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer2->Add(itemPanel3, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    itemPanel3->SetSizer(itemBoxSizerVHeader);

    wxStaticText* itemStaticText9 = new wxStaticText(itemPanel3
        , wxID_ANY, _("REPORTS"));
    itemStaticText9->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizerVHeader->Add(itemStaticText9, 0, wxALL, 1);

    browser_ = wxWebView::New(this, wxID_ANY);
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerReportsPage(this, "trxid")));
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerReportsPage(this, "trx")));

    itemBoxSizer2->Add(browser_, 1, wxGROW|wxALL, 1);
}

void mmReportsPanel::PrintPage()
{
    browser_->Print();
}
