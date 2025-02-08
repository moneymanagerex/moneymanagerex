/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 - 2021 Nikolay Akimov
 Copyright (C) 2021,2024 Mark Whalley (mark@ipx.co.uk)

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

mmAboutDialog::~mmAboutDialog()
{
    const bool v = m_send_data->GetValue();
    Option::instance().setSendUsageStats(v);
}


mmAboutDialog::mmAboutDialog(wxWindow* parent, int tabToOpenNo)
{
    const wxString caption = (tabToOpenNo == 4)
        ? _("License agreement")
        : wxString::Format("%s %s", ::mmex::getProgramName(), ::mmex::getTitleProgramVersion());
    createWindow(parent, caption, tabToOpenNo);
}

bool mmAboutDialog::createWindow(wxWindow* parent
    , const wxString& caption
    , int tabToOpenNo
    , wxWindowID id
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString &name
)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    bool ok = wxDialog::Create(parent, id, caption, pos, size, style, name);

    if (ok)
    {
        createControls(tabToOpenNo);
        initControls();
        SetMinSize(wxSize(400, 600));
        this->SetIcon(::mmex::getProgramIcon());
        Fit();
        Centre();
    }

    return ok;
}

void do_read_file(std::vector<wxString>& data, const wxString& file_path)
{
    if (!wxFileName::FileExists(file_path)) return;

    mmHTMLBuilder hb;
    hb.init(true, "");
    wxFileInputStream input(file_path);
    wxTextInputStream text(input);
    wxRegEx link(R"(\[([^][]+)\]\(([^\(\)]+)\))", wxRE_EXTENDED);

    wxString txt, prev, line;
    while (input.IsOk() && !input.Eof())
    {
        prev = line;
        line = text.ReadLine();
        if (line.StartsWith("=============")) {
            line = "<!-- header -->";
        }
        else if (line.StartsWith("###"))
        {
            if (!line.empty()) {
                line.Prepend("<H3>");
                line.Replace("###", "");
                line.Append("</H3>\n");
            }
        }
        else if (line.StartsWith("##") || prev.Contains("<!-- header -->"))
        {
            if (!line.empty()) {
                line.Prepend("<H2>");
                line.Replace("##", "");
                line.Append("</H2>\n");
            }
        }
        else
            line << "<br>\n";

        prev = line;
        if (line.StartsWith("-------------") || input.Eof())
        {
            hb.addText(txt);
            hb.end(true);
            data.push_back(hb.getHTMLText());
            hb.clear();
            hb.init(true, "");
            txt.clear();
        }
        else
        {
            link.Replace(&line, R"(<a href='\2'>\1</a>)");
            txt << line;
        }
    }
    hb.end(true);
}

void mmAboutDialog::initControls()
{
    mmHTMLBuilder hb;
    hb.init(true, "");
    wxString html = getProgramDescription(2);
    html.Replace("\n", "<br>");
    hb.addHeader(1, ::mmex::getProgramName());
    hb.addText(html);
    hb.end(true);
    html = hb.getHTMLText();
    aboutText_->SetPage(html);

    hb.clear();

    //Read data from files
    wxString filePath[] = {
        ::mmex::getPathDoc(mmex::F_CONTRIB, false),
        ::mmex::getPathDoc(mmex::F_LICENSE, false)
    };

    std::vector<wxString> data;
    for (const auto& file_path : filePath)
    {
        wxLogDebug("%s", file_path);
        do_read_file(data, file_path);
    }

    if (!data.empty()) authorsText_->SetPage(data[0]);
    if (data.size() > 1) sponsorsText_->SetPage(data[1]);

    if (data.size() > 2) licenseText_->SetPage(data[2]);
    if (data.size() > 3) privacyText_->SetPage(data[3]);
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

    aboutText_ = new mmHtmlWindow(aboutTab
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    aboutSizer->Add(aboutText_, g_flagsExpand);

    authorsText_ = new mmHtmlWindow(authorsTab
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    authorsSizer->Add(authorsText_, g_flagsExpand);

    sponsorsText_ = new mmHtmlWindow(sponsorsTab
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    sponsorsSizer->Add(sponsorsText_, g_flagsExpand);

    licenseText_ = new mmHtmlWindow(licenseTab
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    licenseSizer->Add(licenseText_, g_flagsExpand);

    privacyText_ = new mmHtmlWindow(privacyTab
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    privacySizer->Add(privacyText_, g_flagsExpand);

    itemBoxSizer->Add(aboutNotebook, g_flagsExpand);

    //
    wxPanel* buttonPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer->Add(buttonPanel, wxSizerFlags(g_flagsV).Center());
    wxBoxSizer* buttonPanelSizer = new wxBoxSizer(wxVERTICAL);
    buttonPanel->SetSizer(buttonPanelSizer);

    m_send_data = new wxCheckBox(buttonPanel, wxID_ANY
        , _("Send anonymous statistics usage data"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    mmToolTip(m_send_data, _("Enable to help us sending anonymous data about MMEX usage."));

    buttonPanelSizer->Add(m_send_data, g_flagsV);

    m_send_data->Show(tabToOpenNo == 4);
    m_send_data->SetValue(Option::instance().getSendUsageStats());

    wxButton* buttonOk = new wxButton(buttonPanel, wxID_OK, _("&OK "));
    buttonOk->SetDefault();
    buttonOk->SetFocus();
    buttonPanelSizer->Add(buttonOk, g_flagsCenter);

    aboutNotebook->ChangeSelection(tabToOpenNo);
}

void mmAboutDialog::handleLink(wxHtmlLinkEvent& event)
{
    wxHtmlLinkInfo linkInfo = event.GetLinkInfo();
    const wxString url = linkInfo.GetHref();
    wxLaunchDefaultBrowser(url);
}
