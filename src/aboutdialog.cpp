/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 - 2020 Nikolay Akimov

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
 *******************************************************/
#include "aboutdialog.h"
#include "constants.h"
#include "paths.h"
#include "reports/htmlbuilder.h"
#include <wx/statline.h>
#include <wx/version.h>
#include <wx/regex.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmAboutDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmAboutDialog, wxDialog)
    EVT_HTML_LINK_CLICKED(wxID_ANY, mmAboutDialog::handleLink)
wxEND_EVENT_TABLE()

mmAboutDialog::mmAboutDialog()
{
}

mmAboutDialog::mmAboutDialog(wxWindow* parent, int tabToOpenNo, const wxString &name)
    : aboutText_(nullptr)
    , authorsText_(nullptr)
    , sponsorsText_(nullptr)
    , licenseText_(nullptr)
    , privacyText_(nullptr)
{
    const wxString caption = (tabToOpenNo == 4)
        ? _("License agreement")
        : wxString::Format("%s - %s", ::mmex::getProgramName(), ::mmex::getTitleProgramVersion());
    createWindow(parent, wxID_ANY, caption, wxDefaultPosition
        , wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX, tabToOpenNo, name);
    SetMinClientSize(wxSize(300, 400));
}

bool mmAboutDialog::createWindow(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , int tabToOpenNo
    , const wxString &name
)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    bool ok = wxDialog::Create(parent, id, caption, pos, size, style, name);

    if (ok)
    {
        createControls(tabToOpenNo);
        initControls();
        this->SetIcon(::mmex::getProgramIcon());
        this->Centre();
    }

    return ok;
}

void mmAboutDialog::initControls()
{
    mmHTMLBuilder hb;
    wxString html = getProgramDescription(2);
    html.Replace("\n", "<br>");
    hb.addHeader(1, ::mmex::getProgramName());
    hb.addText(html);
    hb.end();
    html = hb.getHTMLText();
    aboutText_->SetPage(html);

    wxArrayString data;
    data.Add("");

    hb.clear();

    //Read data from file
    wxString filePath = ::mmex::getPathDoc(mmex::F_CONTRIB, false);
    if (wxFileName::FileExists(filePath))
    {
        wxFileInputStream input(filePath);
        wxTextInputStream text(input);
        wxRegEx link(R"(\[([^][]+)\]\(([^\(\)]+)\))", wxRE_EXTENDED);
        int part = 0;

        while (input.IsOk() && !input.Eof())
        {
            wxString line = text.ReadLine();
            if (line.StartsWith("============="))
                line = "";
            else if (line.StartsWith("##"))
            {
                line.Replace("##", "<H3>");
                line.Append("</H3>\n");
            }
            else
                line << "<br>\n";

            if (line.StartsWith("-------------"))
            {
                hb.addText(data[part]);
                hb.end();
                data[part] = hb.getHTMLText();
                ++part;
                hb.clear();
                data.Add("");
            }
            else
            {
                link.Replace(&line, R"(<a href='\2'>\1</a>)");
                data[part] << line;
            }
        }
    }

    authorsText_->SetPage(data[0]);
    if (data.GetCount() > 1) sponsorsText_->SetPage(data[1]);
    if (data.GetCount() > 2) licenseText_->SetPage(data[2]);
    if (data.GetCount() > 3) privacyText_->SetPage(data[3]);
}

void mmAboutDialog::createControls(int tabToOpenNo)
{
    wxBoxSizer* itemBoxSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer);

    //Create tabs
    wxNotebook* aboutNotebook = new wxNotebook(this
        , wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);
    aboutNotebook->SetMinSize(wxSize(400, 500));

    wxPanel* aboutTab = new wxPanel(aboutNotebook, wxID_ANY);
    aboutNotebook->AddPage(aboutTab, _("About"));
    wxBoxSizer *aboutSizer = new wxBoxSizer(wxVERTICAL);
    aboutTab->SetSizer(aboutSizer);

    wxPanel* authorsTab = new wxPanel(aboutNotebook, wxID_ANY);
    aboutNotebook->AddPage(authorsTab, _("Authors"));
    wxBoxSizer *authorsSizer = new wxBoxSizer(wxVERTICAL);
    authorsTab->SetSizer(authorsSizer);

    wxPanel* sponsorsTab = new wxPanel(aboutNotebook, wxID_ANY);
    aboutNotebook->AddPage(sponsorsTab, _("Sponsors"));
    wxBoxSizer *sponsorsSizer = new wxBoxSizer(wxVERTICAL);
    sponsorsTab->SetSizer(sponsorsSizer);

    wxPanel* licenseTab = new wxPanel(aboutNotebook, wxID_ANY);
    aboutNotebook->AddPage(licenseTab, _("License"));
    wxBoxSizer *licenseSizer = new wxBoxSizer(wxVERTICAL);
    licenseTab->SetSizer(licenseSizer);

    wxPanel* privacyTab = new wxPanel(aboutNotebook, wxID_ANY);
    aboutNotebook->AddPage(privacyTab, _("Privacy"));
    wxBoxSizer *privacySizer = new wxBoxSizer(wxVERTICAL);
    privacyTab->SetSizer(privacySizer);

    aboutText_ = new wxHtmlWindow(aboutTab
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    aboutSizer->Add(aboutText_, g_flagsExpand);

    authorsText_ = new wxHtmlWindow(authorsTab
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    authorsSizer->Add(authorsText_, g_flagsExpand);

    sponsorsText_ = new wxHtmlWindow(sponsorsTab
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    sponsorsSizer->Add(sponsorsText_, g_flagsExpand);

    licenseText_ = new wxHtmlWindow(licenseTab
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    licenseSizer->Add(licenseText_, g_flagsExpand);

    privacyText_ = new wxHtmlWindow(privacyTab
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    privacySizer->Add(privacyText_, g_flagsExpand);

    itemBoxSizer->Add(aboutNotebook, g_flagsExpand);

    wxButton* buttonOk = new wxButton(this, wxID_OK, _("&OK "));
    buttonOk->SetDefault();
    buttonOk->SetFocus();
    itemBoxSizer->Add(buttonOk, g_flagsCenter);

    aboutNotebook->ChangeSelection(tabToOpenNo);

    GetSizer()->Fit(this);
}

void mmAboutDialog::handleLink(wxHtmlLinkEvent& event)
{
    wxHtmlLinkInfo linkInfo = event.GetLinkInfo();
    wxString url = linkInfo.GetHref();
    wxLaunchDefaultBrowser(url);
}
