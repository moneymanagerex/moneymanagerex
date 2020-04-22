/*******************************************************
 Copyright (C) 2014 Gabriele-V

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

wxBEGIN_EVENT_TABLE(mmUpdateWizard, wxWizard)
    EVT_HTML_LINK_CLICKED(wxID_ANY, mmUpdateWizard::LinkClicked)
wxEND_EVENT_TABLE()

const char* update_template = R"(
<!DOCTYPE html>
<html>
<head>
  <meta http-equiv="content-type" content="text/html; charset=utf-8" />
    <link href="memory:master.css" rel="stylesheet" />
</head>
<body>
<h2>%s</h2>
<h3>%s</h3>
%s
</body>
</html>
)";

mmUpdateWizard::mmUpdateWizard(wxFrame *frame, const Document& json_releases, wxArrayInt new_releases)
    : wxWizard(frame, wxID_ANY, _("Update Wizard")
        , wxNullBitmap, wxDefaultPosition, wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER)
{
    this->SetIcon(mmex::getProgramIcon());
    page1 = new wxWizardPageSimple(this);
    
    int i = 0;

    wxString html;

    for (auto& r : json_releases.GetArray())
    {
        if (new_releases.Index(i) != wxNOT_FOUND) 
        {
            bool p = (r.HasMember("prerelease") && r["prerelease"].IsBool() && r["prerelease"].GetBool());
            const auto prerelease = !p ? _("Stable") : _("Unstable");

            const auto html_url = (r.HasMember("html_url") && r["html_url"].IsString())
                ? r["html_url"].GetString() : "";

            const auto tag = (r.HasMember("tag_name") && r["tag_name"].IsString())
                ? r["tag_name"].GetString() : "";

            const auto published_at = (r.HasMember("published_at") && r["published_at"].IsString())
                ? r["published_at"].GetString() : "";
            wxDateTime pub_date;
            wxString::const_iterator end;
            pub_date.ParseFormat(published_at, "%Y-%m-%dT%H:%M:%SZ", &end);
            const wxString pd = pub_date.FormatISODate();
            const wxString time = pub_date.FormatISOTime();

            const auto body = md2html((r.HasMember("body") && r["body"].IsString())
                ? r["body"].GetString() : "");

            wxString link = wxString::Format(R"(<a href="%s">%s</a>)", html_url, tag);
            const wxString github = "https://github.com/moneymanagerex/moneymanagerex/releases/tag/";
            const wxString sf = "https://sourceforge.net/projects/moneymanagerex/files/";
            if (link.Contains(github)) link.Replace(github, sf);
            html += wxString::Format("<table class='table'><thead><tr><th>%s</th><th>%s</th><th>%s</th><th>%s</th></tr></thead>\n"
                , _("Version"), _("Status"), _("Date"), _("Time"));
            html += wxString::Format("<tbody><tr class='success'><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr><tr class='active'><td colspan='4'>%s</td></tr><tbody></table><hr>\n\n"
                , link, prerelease, pd, time, body);
        }
        i++;
    }

    wxString ver = wxString::Format(_("Your version is %s"), mmex::version::string);
    wxString header = _("A new version of MMEX is available!");
    html = wxString::Format(update_template, header, ver, html);

    wxBoxSizer *page1_sizer = new wxBoxSizer(wxVERTICAL);
    page1->SetSizer(page1_sizer);

    wxHtmlWindow* browser = new wxHtmlWindow(page1
        , wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    browser->SetMinSize(wxSize(350, 250));

    wxStaticText *tipsText = new wxStaticText(page1, wxID_ANY, wxGetTranslation(TIPS[1]));

    page1_sizer->Add(browser, wxSizerFlags(g_flagsExpand).Border(wxTOP, 0));
    page1_sizer->Add(tipsText, g_flagsCenter);

    GetPageAreaSizer()->Add(page1);
    setControlEnable(wxID_CANCEL);
    setControlEnable(wxID_BACKWARD);

    browser->SetPage(html);
}

void mmUpdateWizard::RunIt(bool modal)
{
    if (modal)
    {
        if (RunWizard(page1))
        {
            //wxLaunchDefaultBrowser(m_new_version["html_url"].GetString()); //TODO: Download file in wizard page2
        }

        Destroy();
    }
}

void mmUpdateWizard::LinkClicked(wxHtmlLinkEvent& evt)
{
    wxLaunchDefaultBrowser(evt.GetLinkInfo().GetHref());
}

struct Version
{
    enum version { MAJOR = 0, MINOR, PATCH, TYPE, NUM, MAX };
    enum parcer {MAJ = 1, MIN, PAT, SKIP, RTYPE, SKIP2, UNUM};
    long v[MAX];

    explicit Version(const wxString& tag)
    {
        for (int i = 0; i < 5; i++) v[i] = 0;
        wxRegEx re_ver(R"(^v([0-9]+)\.([0-9]+)\.([0-9]+)?(-(alpha|beta|rc)(\.([0-9]+))?)?$)", wxRE_EXTENDED);
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
void mmUpdate::checkUpdates(bool bSilent, wxFrame *frame)
{
    wxString resp;
    CURLcode err_code = http_get_data(mmex::weblink::Releases, resp);
    if (err_code != CURLE_OK)
    {
        if (!bSilent)
        {
            const wxString& msgStr = _("Unable to check for updates!")
                + "\n\n" + _("Error: ") + curl_easy_strerror(err_code);
            wxMessageBox(msgStr, _("MMEX Update Check"));
        }
        return;
    }

    // https://developer.github.com/v3/repos/releases/#list-releases-for-a-repository

    Document json_releases;
    ParseResult res = json_releases.Parse(resp.c_str());
    if (!res || !json_releases.IsArray())
    {
        if (!bSilent)
        {
            const wxString& msgStr = _("Unable to check for updates!")
                + "\n\n" + _("Error: ")
                + ((!res) ? GetParseError_En(res.Code())
                    : json_releases.GetString());
            wxMessageBox(msgStr, _("MMEX Update Check"));
        }
        return;
    }

    wxLogDebug("======= mmUpdate::checkUpdates =======");

    bool s = mmex::version::isStable();
    const int _stable = s ?
        Model_Setting::instance().GetIntSetting("UPDATESOURCE", 0) == 0
        : 0;

    const wxString current_tag = ("v" + mmex::version::string).Lower();
    Version current(current_tag);
    wxLogDebug("Current vertion: = %s", current_tag);
    wxArrayInt new_releas;
    int i = 0;
    for (auto& r : json_releases.GetArray())
    {
        const auto tag_name = r["tag_name"].GetString();
        if (_stable && r["prerelease"].IsTrue()) {
            wxLogDebug("[S] tag %s", tag_name);
            continue;
        }

        Version check(tag_name);
        if (current < check) {
            wxLogDebug("[V] tag %s", tag_name);
            new_releas.Add(i);
        } else {
            wxLogDebug("[X] tag %s", tag_name);
        }
        i++;
    }

    if (!new_releas.empty())
    {
        mmUpdateWizard* wizard = new mmUpdateWizard(frame, json_releases, new_releas);
        wizard->CenterOnParent();
        wizard->RunIt(true);
    }
    else if (!bSilent)
    {
        wxMessageBox(_("You already have the latest version"),
            _("MMEX Update Check"), wxICON_INFORMATION);
    }
}
