/*******************************************************
 Copyright (C) 2014 Gabriele-V
  Copyright (C) 2020 Nikolay Akimov

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

#include "wizard_update.h"
#include "constants.h"
#include "util.h"
#include "model/Model_Setting.h"
#include "rapidjson/error/en.h"
#include "rapidjson/document.h"
#include "reports/htmlbuilder.h"
#include "paths.h"

const char* update_template = R"(
<!DOCTYPE html>
<html>
<head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8" />
    <title><TMPL_VAR REPORTNAME></title>
    <link href='master.css' rel='stylesheet' />
    <style>
        canvas {min-height: 100px}
        body {font-size: <TMPL_VAR HTMLSCALE>%}
    </style>
</head>
<body>
<div class = "container">
<h3><TMPL_VAR HEADER></h3>
<div class = "col-xs-8">
<TMPL_VAR CONTENTS>
</div>
</body>
</html>
)";


wxBEGIN_EVENT_TABLE(mmUpdateWizard, wxWizard)
    EVT_HTML_LINK_CLICKED(wxID_ANY, mmUpdateWizard::LinkClicked)
    EVT_BUTTON(wxID_CANCEL, mmUpdateWizard::OnCancel)
wxEND_EVENT_TABLE()

mmUpdateWizard::mmUpdateWizard(wxWindow* parent)
    : update_text_(nullptr)
{
    const wxString caption = _("Update Wizard");
    Create(parent, wxID_ANY, caption, wxDefaultPosition
        , wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX);

    checkUpdates();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    SetMinSize(wxSize(400, 580));
    SetIcon(mmex::getProgramIcon());
    Centre();
    ShowModal();
}

void mmUpdateWizard::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString& name)
{
    bool ok = wxDialog::Create(parent, id, caption, pos, size, style, name);

    if (ok)
        CreateControls();

}
void mmUpdateWizard::CreateControls()
{
    wxBoxSizer* itemBoxSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer);

    const auto mmexv =  wxString::Format(_("Your version is %s"), mmex::version::string);
    wxStaticText *verText = new wxStaticText(this, wxID_ANY, mmexv);
    verText->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizer->Add(verText, g_flagsH);

    update_text_ = new wxHtmlWindow(this
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    itemBoxSizer->Add(update_text_, g_flagsExpand);

    wxButton* button_OK = new wxButton(this, wxID_OK, _("&OK "));
    button_OK->SetDefault();
    button_OK->SetFocus();
    itemBoxSizer->Add(button_OK, g_flagsCenter);

}

void mmUpdateWizard::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmUpdateWizard::LinkClicked(wxHtmlLinkEvent& evt)
{
    wxLaunchDefaultBrowser(evt.GetLinkInfo().GetHref());
}

void mmUpdateWizard::checkUpdates()
{
    wxString resp;
    CURLcode err_code = http_get_data(mmex::weblink::Releases, resp);
    if (err_code != CURLE_OK)
    {
        const wxString& msgStr = _("Unable to check for updates!")
            + "\n\n" + _("Error: ") + curl_easy_strerror(err_code);
        wxMessageBox(msgStr, _("MMEX Update Check"));
    }

    // https://developer.github.com/v3/repos/releases/#list-releases-for-a-repository

    Document json_releases;
    ParseResult res = json_releases.Parse(resp.c_str());
    if (!res || !json_releases.IsArray())
    {
        const wxString& msgStr = _("Unable to check for updates!")
            + "\n\n" + _("Error: ")
            + ((!res) ? GetParseError_En(res.Code())
                : json_releases.GetString());
        wxMessageBox(msgStr, _("MMEX Update Check"));
    }

    wxLogDebug("======= mmUpdate::checkUpdates =======");

    const int _stable = Model_Setting::instance().GetIntSetting("UPDATESOURCE", 0) == 0;

    wxString current_tag("v" + mmex::version::string);
    wxString latest_tag = current_tag;
    Value& latest_release = json_releases[0];

    wxLogDebug("Current vertion: = %s", current_tag);

    mmHTMLBuilder hb;
    hb.startTable();
    hb.startThead();
    hb.addTableCell(_("Status"));
    hb.addTableCell(_("Name"));
    hb.addTableCell(_("Date"));
    hb.endThead();

    hb.startTbody();
    for (auto& r : json_releases.GetArray())
    {
        if (r.HasMember("draft") && r["draft"].IsBool()) {
            if (r["draft"].IsTrue()) { 
                continue; 
            }
        }
        else
            wxASSERT(false);

        wxArrayString UpdatesType_;
        UpdatesType_.Add(_("Stable"));
        UpdatesType_.Add(_("Unstable"));

        bool p = (r.HasMember("prerelease") && r["prerelease"].IsBool() && r["prerelease"].GetBool());
        const auto prerelease = !p ? _("Stable") : _("Unstable");

        const auto html_url = (r.HasMember("html_url") && r["html_url"].IsString()) 
            ? r["html_url"].GetString() : "";
        const auto published_at = (r.HasMember("published_at") && r["published_at"].IsString()) 
            ? r["published_at"].GetString() : "";
        const auto tag_name = (r.HasMember("tag_name") && r["tag_name"].IsString()) 
            ?  r["tag_name"].GetString() : "";

        hb.startTableRow();
        hb.addTableCell(prerelease);
        hb.addTableCellLink(html_url, tag_name);
        hb.addTableCell(published_at);
        hb.endTableRow();
    }
    hb.endTbody();
    hb.endTable();

    mm_html_template report(update_template);
    report(L"REPORTNAME") = _("Update Wizard");
    report(L"HEADER") = _("Available Releases:");
    report(L"CONTENTS") = hb.getHTMLText();
    report(L"HTMLSCALE") = wxString::Format("%d", Option::instance().HtmlFontSize());

    wxString out = wxEmptyString;
    try
    {
        out = report.Process();
    }
    catch (const syntax_ex & e)
    {
        wxLogError( e.what());
    }
    catch (...)
    {
        wxLogError(_("Caught exception"));
    }

    update_text_->SetPage(out);
    update_text_->Fit();
}
