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
#include "paths.h"
#include "reports/htmlbuilder.h"
#include <wx/statline.h>
#include <wx/version.h>
#include <wx/regex.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmAboutDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmAboutDialog, wxDialog)
    EVT_HTML_LINK_CLICKED(wxID_ANY, mmAboutDialog::OnLinkClicked)
wxEND_EVENT_TABLE()

mmAboutDialog::mmAboutDialog(wxWindow* parent, int TabToOpen, const wxString &name)
: about_text_(nullptr)
, authors_text_(nullptr)
, sponsors_text_(nullptr)
, license_text_(nullptr)
, privacy_text_(nullptr)
{
    const wxString caption = (TabToOpen == 4)
        ? _("License agreement") : wxString::Format(_("About %s"), mmex::getProgramName());
    Create(parent, wxID_ANY, caption, wxDefaultPosition
        , wxSize(390, 550), wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX, TabToOpen, name);
    SetMinClientSize(wxSize(300, 400));
}

bool mmAboutDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , int TabToOpen
    , const wxString &name
    )
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    bool ok = wxDialog::Create(parent, id, caption, pos, size, style, name);

    if (ok)
    {
        CreateControls(TabToOpen);
        InitControls();
        //GetSizer()->Fit(this);
        //GetSizer()->SetSizeHints(this);
        //this->SetInitialSize();
        this->SetIcon(mmex::getProgramIcon());
        this->Centre();
    }

    return ok;
}

void mmAboutDialog::InitControls()
{
    mmHTMLBuilder hb;
    wxString html = mmex::getProgramDescription();
    html.Replace("\n", "<br>");
    hb.addHeader(1, mmex::getProgramName());
    hb.addText(html);
    hb.end();
    html = hb.getHTMLText();
    about_text_->SetPage(html);

    wxArrayString data;
    data.Add("");

    int part = 0;
    hb.clear();

    //Read data from file
    wxString filePath = mmex::getPathDoc(mmex::F_CONTRIB);
    if (wxFileName::FileExists(filePath))
    {
        wxFileInputStream input(filePath);
        wxTextInputStream text(input);
        wxRegEx link (R"(\[([^][]+)\]\(([^\(\)]+)\))", wxRE_EXTENDED);

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
                link.Replace(&line,"<a href='\\2'>\\1</a>");
                data[part] << line;
            }
        }
    }

    authors_text_->SetPage(data[0]);
    if (data.GetCount() > 1) sponsors_text_->SetPage(data[1]);
    if (data.GetCount() > 2) license_text_->SetPage(data[2]);
    if (data.GetCount() > 3) privacy_text_->SetPage(data[3]);
}

void mmAboutDialog::CreateControls(int TabToOpen)
{
    wxBoxSizer* itemBoxSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer);

    //Create tabs
    wxNotebook* about_notebook = new wxNotebook(this
        , wxID_ANY, wxDefaultPosition, wxSize(400, 500), wxNB_MULTILINE);

    wxPanel* about_tab = new wxPanel(about_notebook, wxID_ANY);
    about_notebook->AddPage(about_tab, _("About"));
    wxBoxSizer *about_sizer = new wxBoxSizer(wxVERTICAL);
    about_tab->SetSizer(about_sizer);

    wxPanel* authors_tab = new wxPanel(about_notebook, wxID_ANY);
    about_notebook->AddPage(authors_tab, _("Authors"));
    wxBoxSizer *authors_sizer = new wxBoxSizer(wxVERTICAL);
    authors_tab->SetSizer(authors_sizer);

    wxPanel* sponsors_tab = new wxPanel(about_notebook, wxID_ANY);
    about_notebook->AddPage(sponsors_tab, _("Sponsors"));
    wxBoxSizer *sponsors_sizer = new wxBoxSizer(wxVERTICAL);
    sponsors_tab->SetSizer(sponsors_sizer);

    wxPanel* license_tab = new wxPanel(about_notebook, wxID_ANY);
    about_notebook->AddPage(license_tab, _("License"));
    wxBoxSizer *license_sizer = new wxBoxSizer(wxVERTICAL);
    license_tab->SetSizer(license_sizer);

    wxPanel* privacy_tab = new wxPanel(about_notebook, wxID_ANY);
    about_notebook->AddPage(privacy_tab, _("Privacy"));
    wxBoxSizer *privacy_sizer = new wxBoxSizer(wxVERTICAL);
    privacy_tab->SetSizer(privacy_sizer);

    about_text_ = new wxHtmlWindow(about_tab
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    about_sizer->Add(about_text_, g_flagsExpand);

    authors_text_ = new wxHtmlWindow(authors_tab
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    authors_sizer->Add(authors_text_, g_flagsExpand);

    sponsors_text_ = new wxHtmlWindow(sponsors_tab
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    sponsors_sizer->Add(sponsors_text_, g_flagsExpand);

    license_text_ = new wxHtmlWindow(license_tab
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    license_sizer->Add(license_text_, g_flagsExpand);

    privacy_text_ = new wxHtmlWindow(privacy_tab
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    privacy_sizer->Add(privacy_text_, g_flagsExpand);

    itemBoxSizer->Add(about_notebook, g_flagsExpand);

    wxButton* button_OK = new wxButton(this, wxID_OK, _("&OK "));
    button_OK->SetDefault();
    button_OK->SetFocus();
    itemBoxSizer->Add(button_OK, g_flagsCenter);

    about_notebook->ChangeSelection(TabToOpen);
}

void mmAboutDialog::OnLinkClicked(wxHtmlLinkEvent& event)
{
    wxHtmlLinkInfo link_info = event.GetLinkInfo();
    wxString sURL = link_info.GetHref();
    wxLaunchDefaultBrowser(sURL);
}
