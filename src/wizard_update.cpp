/*******************************************************
 Copyright (C) 2014 Gabriele-V
 Copyright (C) 2020, 2021 Nikolay Akimov

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
#include "mmTips.h"
#include "util.h"
#include "paths.h"
#include "reports/htmlbuilder.h"
#include "model/Model_Setting.h"
#include "rapidjson/error/en.h"

#include <wx/webview.h>
#include <wx/webviewfshandler.h>
#include <wx/fs_mem.h>

wxBEGIN_EVENT_TABLE(mmUpdateWizard, wxDialog)
wxEND_EVENT_TABLE()

const char* update_template = R"(
<!DOCTYPE html>
<html>
<head>
  <meta http-equiv="content-type" content="text/html; charset=utf-8" />
    <link href="memory:master.css" rel="stylesheet" />
<style>
.header .image,
.header .text {
    display: inline-block;
    vertical-align: middle;
    border:10px;
}
</style>
</head>
<body>
<header class="header"><div class="image"><img src="memory:mmex.png" title="moneymanagerex.org" width="64" height="64" /></div>
<div class="text"><h2>%s</h2><h3>%s</h3></div></header>
%s
</body>
</html>
)";

mmUpdateWizard::~mmUpdateWizard()
{
    clearVFprintedFiles("rep");
    bool isActive = showUpdateCheckBox_->GetValue();
    Model_Setting::instance().setString(
        "UPDATE_LAST_CHECKED_VERSION",
        (!isActive) ? top_version_ : ("v" + mmex::version::string).Lower()
    );
}

mmUpdateWizard::mmUpdateWizard(wxWindow* parent, const Document& json_releases, wxArrayInt new_releases, const wxString& top_version)
    : top_version_(top_version)
{

    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    bool isDialogCreated = wxDialog::Create(parent, wxID_ANY, _("Update Wizard")
        , wxDefaultPosition, wxDefaultSize
        , wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX, "mmUpdateWizard");

    if (isDialogCreated) 
    {
        SetMinSize(wxSize(600, 400));

        CreateControls(json_releases, new_releases);

        this->SetIcon(mmex::getProgramIcon());
        this->Centre();
        this->Layout();
    }
}


void mmUpdateWizard::CreateControls(const Document& json_releases, wxArrayInt new_releases)
{

    int i = 0;
    bool isHistory = false;
    wxString html, separator = " ", new_html_url, new_tag;

    for (auto& r : json_releases.GetArray())
    {
        if (!isHistory && new_releases.Index(i) == wxNOT_FOUND) {
            isHistory = true;
            separator = wxString::Format("<h3> %s </h3>", _("Historical releases:"));
        }

        bool is_prerelease = (r.HasMember("prerelease") && r["prerelease"].IsBool() && r["prerelease"].GetBool());
        bool update_stable = Model_Setting::instance().getInt("UPDATESOURCE", 0) == 0;
        if (update_stable && is_prerelease)
            continue;

        const auto prerelease = !is_prerelease ? _("Stable") : _("Unstable");

        const auto html_url = (r.HasMember("html_url") && r["html_url"].IsString())
            ? wxString::FromUTF8(r["html_url"].GetString()) : "";

        const auto tag = (r.HasMember("tag_name") && r["tag_name"].IsString())
            ? wxString::FromUTF8(r["tag_name"].GetString()) : "";

        if (new_tag.empty()) {
            new_tag = tag;
            new_html_url = html_url;
        }

        const auto published_at = (r.HasMember("published_at") && r["published_at"].IsString())
            ? wxString::FromUTF8(r["published_at"].GetString()) : "";
        wxDateTime pub_date;
        wxString::const_iterator end;
        pub_date.ParseFormat(published_at, "%Y-%m-%dT%H:%M:%SZ", &end);
        const wxString pd = pub_date.FormatISODate();
        const wxString time = pub_date.FormatISOTime();

        const auto body = md2html((r.HasMember("body") && r["body"].IsString())
            ? wxString::FromUTF8(r["body"].GetString()) : "");

        const wxString link = wxString::Format(R"(<a href="%s" target="_blank">%s</a>)", html_url, tag);

        html += wxString::Format("%s<table class='table'><thead><tr><th>%s</th><th>%s</th><th>%s</th><th>%s</th></tr></thead>\n"
            , separator, _("Version"), _("Status"), _("Date"), _("Time"));
        html += wxString::Format("<tbody><tr class='success'><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>"
            "<tr class='active'><td colspan='4'>%s</td></tr><tbody></table>\n\n"
            , link, prerelease, pd, time, body);
        separator = "<hr>\n";
        i++;
    }

    auto version = new_releases.empty() ? _("MMEX is up to date.") : _("A new version of MMEX is available.");
    if (!new_releases.empty()) {

        const auto ver_num = new_tag.Mid(1);

#if defined(__APPLE__)
        new_html_url.Replace("/tag/", "/download/");
        new_html_url.Append(wxString::Format("/mmex-%s-Darwin.dmg", ver_num));
        version = wxString::Format(R"(<a href="%s" target="_blank">%s</a>)", new_html_url, version);
#endif


#if defined(__WXGTK__)
        version = wxString::Format(R"(<a href="%s" target="_blank">%s</a>)", new_html_url, version);
#endif


#if defined(__WXMSW__)
        const auto win_ver =
#if defined(_WIN64) || defined(__x86_64__)
            "-win64";
#else
            "-win32";
#endif
        new_html_url.Replace("/tag/", "/download/");
        const auto portable = mmex::isPortableMode() ? "-portable" : "";
        const auto extension = mmex::isPortableMode() ? ".zip" : ".exe";
        new_html_url.Append(wxString::Format("/mmex-%s%s%s%s", ver_num, win_ver, portable, extension));
        version = wxString::Format(R"(<a href="%s" target="_blank">%s</a>)", new_html_url, version);
#endif
    }

    wxString header = wxString::Format(_("Version: %s"), mmex::version::string);
    html = wxString::Format(update_template, header, version, html);

    wxBoxSizer *page1_sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(page1_sizer);

    wxWebView* browser = wxWebView::New();
#ifdef __WXMAC__
    // With WKWebView handlers need to be registered before creation
    browser->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
    browser->Create(this, wxID_CONTEXT_HELP, wxWebViewDefaultURLStr);
#else
    browser->Create(this, wxID_CONTEXT_HELP, wxWebViewDefaultURLStr);
    browser->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
#endif
#ifndef _DEBUG
    browser->EnableContextMenu(false);
#endif
    Bind(wxEVT_WEBVIEW_NEWWINDOW, &mmUpdateWizard::OnNewWindow, this, browser->GetId());

    wxStaticText *tipsText = new wxStaticText(this, wxID_ANY, wxGetTranslation(TIPS[1]));

    page1_sizer->Add(browser, wxSizerFlags(g_flagsExpand).Border(wxTOP, 0));
    page1_sizer->Add(tipsText, g_flagsCenter);

    const auto name = getVFname4print("rep", html);
    browser->LoadURL(name);

    const wxString showAppStartString = wxString::Format(_("&Show this dialog box at startup"));
    showUpdateCheckBox_ = new wxCheckBox(this, wxID_ANY, showAppStartString, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    showUpdateCheckBox_->SetValue(true);
    page1_sizer->Add(showUpdateCheckBox_, g_flagsV);


    wxButton* buttonOk = new wxButton(this, wxID_OK, _("&OK "));
    page1_sizer->Add(buttonOk, g_flagsCenter);

    buttonOk->SetDefault();
    buttonOk->SetFocus();

    Fit();
}

void mmUpdateWizard::OnNewWindow(wxWebViewEvent& evt)
{
    const wxString uri = evt.GetURL();
    wxLaunchDefaultBrowser(uri);

    evt.Skip();
}

struct Version
{
    enum version { MAJOR = 0, MINOR, PATCH, TYPE, NUM, MAX };
    enum parcer {MAJ = 1, MIN, PAT, SKIP, RTYPE, SKIP2, UNUM};
    long v[MAX];

    explicit Version(const wxString& tag)
    {
        for (int i = 0; i < 5; i++) v[i] = 0;
        wxRegEx re_ver(R"(^v([0-9]+)\.([0-9]+)\.(-?[0-9]+)?(-(alpha|beta|rc)(\.([0-9]+))?)?$)", wxRE_EXTENDED);
        if (re_ver.Matches(tag)) {
            for (size_t i = 1; i < re_ver.GetMatchCount(); ++i)
            {
                wxString val = re_ver.GetMatch(tag, i).Lower();
                switch (i)
                {
                case MAJ: val.ToCLong(&v[MAJOR]); break;
                case MIN: val.ToCLong(&v[MINOR]); break;
                case PAT: val.ToCLong(&v[PATCH]); break;
                case RTYPE: if (val == "alpha") v[TYPE] = -3;
                        else if (val == "beta") v[TYPE] = -2;
                        else if (val == "rc") v[TYPE] = -1;
                        else v[TYPE] = 0;
                    break;
                case parcer::UNUM: val.ToCLong(&v[NUM]); break;
                }
            }
        }
    }

    bool operator < (const Version& other)
    {
        for (int i = 0; i < 5; i++)
            if (v[i] < other.v[i]) return true;
            else if (v[i] > other.v[i]) return false;
        return false;
    }

    bool operator > (const Version& other)
    {
        for (int i = 0; i < 5; i++)
            if (v[i] > other.v[i]) return true;
            else if (v[i] < other.v[i]) return false;
        return false;
    }

    Version& operator = (const Version& other)
    {
        if (*this != other)
            for (int i = 0; i < 5; i++) v[i] = other.v[i];
        return *this;
    }

    bool operator == (const Version& other)
    {
        for (int i = 0; i < 5; i++) if (v[i] != other.v[i]) return false;
        return true;
    }

    bool operator != (const Version& other)
    {
        return !(*this == other);
    }

    friend wxString& operator << (wxString& str, const Version& ver)
    {
        str << 'v' << ver.v[0] << '.' << ver.v[1] << '.' << ver.v[2];
        if (ver.v[3] < 0)
        {
            switch (ver.v[3])
            {
            case -3: str << "-alpha"; break;
            case -2: str << "-beta"; break;
            case -1: str << "-rc"; break;
            }
            if (ver.v[4] > 0)
                str << '.' << ver.v[4];
        }
        return str;
    }
};

//--------------
//mmUpdate Class
//--------------
void mmUpdate::checkUpdates(wxFrame *frame, bool bSilent)
{
    bool is_stable = mmex::version::isStable();
    const auto url = is_stable ? mmex::weblink::Latest : mmex::weblink::Releases;

    wxString resp;
    CURLcode err_code = http_get_data(url, resp);
    if (err_code != CURLE_OK) {
        if (!bSilent) {
            const wxString& msgStr = _("Unable to check for updates!")
                + "\n\n" + _("Error: ") + curl_easy_strerror(err_code);
            wxMessageBox(msgStr, _("MMEX Update Check"));
        }
        return;
    }

    // https://developer.github.com/v3/repos/releases/#list-releases-for-a-repository

    resp = is_stable ? wxString::Format("[%s]", resp) : resp;
    Document json_releases;
    ParseResult res = json_releases.Parse(resp.utf8_str());
    if (!res || !json_releases.IsArray()) {
        if (!bSilent) {
            const wxString& msgStr = _("Unable to check for updates!")
                + "\n\n" + _("Error: ")
                + wxString::FromUTF8(!res ? GetParseError_En(res.Code()) : json_releases.GetString());
            wxMessageBox(msgStr, _("MMEX Update Check"));
        }
        return;
    }

    wxLogDebug("{{{ mmUpdate::checkUpdates()");
    bool update_stable = Model_Setting::instance().getInt("UPDATESOURCE", 0) == 0;
    const int _stable = is_stable ? update_stable : 0;
    const wxString current_tag = ("v" + mmex::version::string).Lower();
    wxString last_checked = Model_Setting::instance().getString("UPDATE_LAST_CHECKED_VERSION", current_tag);

    bool is_update_available = false;
    Version current(current_tag);
    Version top(current_tag);
    wxString top_version;
    Version last(last_checked);
    wxLogDebug("Current vertion: %s", current_tag);
    wxArrayInt new_releases;
    int i = 0;
    for (auto& r : json_releases.GetArray()) {
        const auto tag_name = wxString::FromUTF8(r["tag_name"].GetString());
        bool prerelease = r["prerelease"].GetBool();
        if (_stable && prerelease) {
            wxLogDebug("[Skip] tag %s", tag_name);
            continue;
        }

        Version check(tag_name);

        if (current < check) {
            wxLogDebug("[New] tag %s", tag_name);
            new_releases.Add(i);
            if (top < check) {
                top = check;
                top_version = tag_name;
                if (last < top) {
                    is_update_available = true;
                }
            }
        } else {
            wxLogDebug("[Outdated] tag %s", tag_name);
        }
        i++;
    }
    wxLogDebug("}}}");

    if (!bSilent || (is_update_available && !new_releases.empty())) {
        mmUpdateWizard* wizard = new mmUpdateWizard(frame, json_releases, new_releases, top_version);
        wizard->CenterOnParent();
        wizard->ShowModal();
    }
}
