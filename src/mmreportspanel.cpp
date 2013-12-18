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
#include "mmcheckingpanel.h"
#include "util.h"
#include "reports/htmlbuilder.h"
#include "mmex.h"
#include "model/Model_Account.h"
#include "model/Model_Checking.h"
#include <wx/webview.h>
#include <wx/webviewfshandler.h>

class WebViewHandlerStatic : public wxWebViewHandler
{
public:
    WebViewHandlerStatic(const wxString& protocol)
        : wxWebViewHandler(protocol)
    {
        m_fs = new wxFileSystem();
    }

    virtual ~WebViewHandlerStatic()
    {
    //    wxDELETE(m_fs);
    }

    virtual wxFSFile* GetFile (const wxString &uri)
    {
        wxString content = uri.substr(9, uri.length()).BeforeLast('/');

        return m_fs->OpenFile(content);
    }   

private:
    wxFileSystem* m_fs;
};

BEGIN_EVENT_TABLE(mmReportsPanel, wxPanel)
    EVT_HTML_LINK_CLICKED(wxID_ANY, mmReportsPanel::OnLinkClicked)
END_EVENT_TABLE()

mmReportsPanel::mmReportsPanel(
        mmPrintableBase* rb, bool cleanupReport, wxWindow *parent,
        wxWindowID winid, const wxPoint& pos,
        const wxSize& size, long style,
        const wxString& name )
: rb_(rb)
, cleanup_(cleanupReport)
{
    Create(parent, winid, pos, size, style, name);
}

mmReportsPanel::~mmReportsPanel()
{
    if (cleanup_ && rb_)
        delete rb_;
}

bool mmReportsPanel::Create( wxWindow *parent, wxWindowID winid,
            const wxPoint& pos, const wxSize& size, long style,
            const wxString& name  )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    htmlWindow_ -> SetPage(getReportText(), "");
    return TRUE;
}

wxString mmReportsPanel::getReportText()
{
    if (!rb_) return "coming soon...";
    wxGetApp().m_frame->SetStatusText(rb_->version());
    return rb_->getHTMLText();
}

void mmReportsPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel( this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(itemPanel3, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    itemPanel3->SetSizer(itemBoxSizerVHeader);

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel3, wxID_ANY,
        _("REPORTS"));
    int font_size = this->GetFont().GetPointSize() + 2;
    itemStaticText9->SetFont(wxFont(font_size, wxSWISS, wxNORMAL, wxBOLD, FALSE, ""));
    itemBoxSizerVHeader->Add(itemStaticText9, 0, wxALL, 1);

    htmlWindow_ = wxWebView::New(this, wxID_ANY);
    htmlWindow_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
    htmlWindow_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new WebViewHandlerStatic(".")));
/*
    htmlWindow_ = new wxWebView( this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize,
        wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
*/
    itemBoxSizer2->Add(htmlWindow_, 1, wxGROW|wxALL, 1);
}

void mmReportsPanel::sortTable()
{
}

void mmReportsPanel::OnLinkClicked(wxHtmlLinkEvent& event)
{
    wxHtmlLinkInfo link_info = event.GetLinkInfo();
    wxString sInfo = link_info.GetHref();
    wxString sData;
    bool bIsTrxId = sInfo.StartsWith("TRXID:", &sData);
    bool isAcct = sInfo.StartsWith("ACCT:", &sData);
    bool isStock = sInfo.StartsWith("STOCK:", &sData);
	bool bIsSort = sInfo.StartsWith("SORT:", &sData);
    mmGUIFrame* frame = wxGetApp().m_frame;
    if (sInfo == "billsdeposits")
    {
        frame->setNavTreeSection(_("Repeating Transactions"));
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_BILLSDEPOSITS);
        frame->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (sInfo == "Assets")
    {
        frame->setNavTreeSection(_("Assets"));
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_ASSETS);
        frame->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (isAcct)
    {
        long id = -1;
        sData.ToLong(&id);
        frame->setGotoAccountID(id);
        const Model_Account::Data* account = Model_Account::instance().get(id);
        frame->setAccountNavTreeSection(account->ACCOUNTNAME);
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
        frame->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (isStock)
    {
        long id = -1;
        sData.ToLong(&id);
        frame->setGotoAccountID(id);
        const Model_Account::Data* account = Model_Account::instance().get(id);
        frame->setAccountNavTreeSection(account->ACCOUNTNAME);
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_STOCKS);
        frame->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (bIsTrxId)
    {
        long transID = -1;
        sData.ToLong(&transID);
        if (transID > 0)
        {
            const Model_Checking::Data* transaction = Model_Checking::instance().get(transID);
            if (transaction)
            {
                int account_id = transaction->ACCOUNTID;
                frame->setGotoAccountID(account_id, transID);
                const Model_Account::Data* account = Model_Account::instance().get(account_id);
                frame->setAccountNavTreeSection(account->ACCOUNTNAME);
                wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
                frame->GetEventHandler()->AddPendingEvent(evt);
            }
        }
    }
	else if (bIsSort)
	{
        long sortColumn = -1;
        sData.ToLong(&sortColumn);
		rb_ -> setSortColumn(sortColumn);
		htmlWindow_ -> SetPage(getReportText(), "");
	}
    else
        wxLaunchDefaultBrowser(sInfo);
}
