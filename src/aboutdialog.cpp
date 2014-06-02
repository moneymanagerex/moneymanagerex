/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012, 2013 Nikolay

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
#include "reports/htmlbuilder.h"
#include "paths.h"
/*******************************************************/

#include <wx/version.h>
#include <wx/wxsqlite3.h>
#include <wx/statline.h>

IMPLEMENT_DYNAMIC_CLASS(mmAboutDialog, wxDialog)

BEGIN_EVENT_TABLE(mmAboutDialog, wxDialog)
    EVT_HTML_LINK_CLICKED(wxID_ANY, mmAboutDialog::OnLinkClicked)
END_EVENT_TABLE()

mmAboutDialog::mmAboutDialog(wxWindow* parent, const int& TabToOpen)
: about_text_()
, developers_text_()
, artwork_text_()
, sponsors_text_()
, license_text_()
{
	wxString caption = wxString(_("About")) << " " << mmex::getProgramName();
	if (TabToOpen == 4)
		caption = _("License agreement");
    Create(parent, wxID_ANY, caption, wxDefaultPosition
        , wxSize(500, 220), wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX, TabToOpen);
}

bool mmAboutDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style
	, const int& TabToOpen
    )
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    bool ok = wxDialog::Create(parent, id, caption, pos, size, style);

    if (ok)
    {
		CreateControls(TabToOpen);
        InitControls();
        GetSizer()->Fit(this);
        GetSizer()->SetSizeHints(this);
        SetIcon(mmex::getProgramIcon());
        Centre();
    }

    return ok;
}

void mmAboutDialog::InitControls()
{
    mmHTMLBuilder hb;
    wxString html = mmex::getProgramDescription();
    html.Replace("======================================\n", "");
    html.Replace("\n", "<br>");
    html << "<br><hr>" << "\n";
    hb.addHeader(3, "Money Manager Ex");
    hb.addParaText(html);
    hb.addLineBreak();
    hb.addTableCellLink(mmex::getProgramWebSite()
        , _("Website"));
    hb.addLineBreak();
    hb.addTableCellLink(mmex::getProgramForum()
        , _("Forum"));
    hb.addLineBreak();
    hb.addTableCellLink(mmex::getProgramWiki()
        , _("Wiki page"));
    hb.addLineBreak();
    hb.addTableCellLink(mmex::getProgramBugReport()
        , _("Bug reports"));
    hb.addLineBreak();
    hb.addLineBreak();
    hb.addTableCellLink(mmex::getProgramFacebookSite()
        , _("Follow MMEX on Facebook"));
    hb.addLineBreak();
    hb.addTableCellLink(mmex::getProgramTwitterSite()
        , _("Follow MMEX on Twitter"));
    hb.addLineBreak();
    hb.addTableCellLink(mmex::getProgramDonateSite()
        , _("Donate"));
    hb.addLineBreak();

    hb.end();
    html = hb.getHTMLText();
    about_text_->SetPage(html);

    wxArrayString data;
    data.Add("");

    int part = 0;
	hb.clear();

    //Read data from file
    wxString filePath = mmex::getPathDoc(mmex::F_CONTRIB);
    wxFileInputStream input(filePath);
    wxTextInputStream text(input);

    while (!input.Eof())
    {
        wxString line = text.ReadLine();
        if (line.StartsWith("============="))
            line = "<hr>\n";
        else if (line.StartsWith("##"))
        {
            line.Replace("##", "<H3>");
            line.Append("</H3>\n");
        }
        else
            line << "<br>\n";

        if (line.StartsWith("-------------"))
        {
            hb.addParaText(data[part]);
            hb.end();
            data[part] = hb.getHTMLText();
            ++part;
			hb.clear();
            data.Add("");
        }
        else
            data[part] << line;
    }

    developers_text_->SetPage(data[0]);
    if (data.GetCount() > 1) artwork_text_->SetPage(data[1]);
    if (data.GetCount() > 2) sponsors_text_->SetPage(data[2]);
	if (data.GetCount() > 3) license_text_->SetPage(data[3]);
}
void mmAboutDialog::CreateControls(const int& TabToOpen)
{
    wxSizerFlags flags;
    flags.Align(wxALIGN_CENTER).Border(wxALL, 5);

    wxBoxSizer* itemBoxSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer);

    wxStaticText* versionStaticText = new wxStaticText( this, wxID_STATIC
		, "Money Manager EX - " + mmex::getTitleProgramVersion());
    int font_size = this->GetFont().GetPointSize() + 2;
    versionStaticText->SetFont(wxFont(wxFontInfo(font_size).Bold()));
    itemBoxSizer->Add(versionStaticText, flags);

    wxStaticText* itemStaticText88 = new wxStaticText(this
        , wxID_STATIC, mmex::getProgramCopyright());

    //Create tabs
    wxNotebook* about_notebook = new wxNotebook(this
        , wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE );

    wxPanel* about_tab = new wxPanel(about_notebook, wxID_ANY);
    about_notebook->AddPage(about_tab, _("About"));
    wxBoxSizer *about_sizer = new wxBoxSizer(wxVERTICAL);
    about_tab->SetSizer(about_sizer);

    wxPanel* developers_tab = new wxPanel(about_notebook, wxID_ANY);
    about_notebook->AddPage(developers_tab, _("Developers"));
    wxBoxSizer *developers_sizer = new wxBoxSizer(wxVERTICAL);
    developers_tab->SetSizer(developers_sizer);

    wxPanel* artwork_tab = new wxPanel(about_notebook, wxID_ANY);
    about_notebook->AddPage(artwork_tab, _("Artwork"));
    wxBoxSizer *artwork_sizer = new wxBoxSizer(wxVERTICAL);
    artwork_tab->SetSizer(artwork_sizer);

    wxPanel* sponsors_tab = new wxPanel(about_notebook, wxID_ANY);
    about_notebook->AddPage(sponsors_tab, _("Sponsors"));
    wxBoxSizer *sponsors_sizer = new wxBoxSizer(wxVERTICAL);
    sponsors_tab->SetSizer(sponsors_sizer);

	wxPanel* license_tab = new wxPanel(about_notebook, wxID_ANY);
	about_notebook->AddPage(license_tab, _("License"));
	wxBoxSizer *license_sizer = new wxBoxSizer(wxVERTICAL);
	license_tab->SetSizer(license_sizer);

    wxSize internal_size = wxSize(400, 400); //developers_tab_->GetBestVirtualSize();

    about_text_ = new wxHtmlWindow(about_tab, wxID_ANY
        , wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    about_sizer->Add(about_text_, 1, wxGROW | wxALL, 1);

    developers_text_ = new wxHtmlWindow(developers_tab
        , wxID_ANY, wxDefaultPosition, internal_size
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    developers_sizer->Add(developers_text_, 1, wxEXPAND);

    artwork_text_ = new wxHtmlWindow(artwork_tab
        , wxID_ANY, wxDefaultPosition, internal_size
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    artwork_sizer->Add(artwork_text_, 1, wxEXPAND);

    sponsors_text_ = new wxHtmlWindow(sponsors_tab
        , wxID_ANY, wxDefaultPosition, internal_size
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    sponsors_sizer->Add(sponsors_text_, 1, wxEXPAND);

	license_text_ = new wxHtmlWindow(license_tab
		, wxID_ANY, wxDefaultPosition, internal_size
		, wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
	license_sizer->Add(license_text_, 1, wxEXPAND);

    itemBoxSizer->Add(about_notebook, flags);
    about_notebook->Layout();

    itemBoxSizer->Add(itemStaticText88, flags);

    wxButton* button_OK = new wxButton(this, wxID_OK, _("&OK "));
    button_OK->SetDefault();
    button_OK->SetFocus();
    itemBoxSizer->Add(button_OK, flags);

	about_notebook->ChangeSelection(TabToOpen);
}

void mmAboutDialog::OnLinkClicked(wxHtmlLinkEvent& event)
{
    wxHtmlLinkInfo link_info = event.GetLinkInfo();
    wxString sURL = link_info.GetHref();
    wxLaunchDefaultBrowser(sURL);
}
