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
#include "util.h"
#include "model/Model_Setting.h"
#include "rapidjson/error/en.h"

wxBEGIN_EVENT_TABLE(mmUpdateWizard, wxWizard)
    EVT_HTML_LINK_CLICKED(wxID_ANY, mmUpdateWizard::LinkClicked)
wxEND_EVENT_TABLE()

mmUpdateWizard::mmUpdateWizard(wxFrame *frame, const Value& new_version)
    : wxWizard(frame, wxID_ANY, _("Update Wizard")
        , wxNullBitmap, wxDefaultPosition, wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER)
    , m_new_version(new_version)
{
    page1 = new wxWizardPageSimple(this);

    wxDateTime pub_date;
    wxString::const_iterator end;
    const wxString pub_date_str = wxString(m_new_version["published_at"].GetString());
    pub_date.ParseFormat(pub_date_str, "%Y-%m-%dT%H:%M:%SZ", &end);
    const wxString displayMsg = wxString()
        << _("A new version of MMEX is available!") << "\n\n"
        << wxString::Format(_("Your version is %s")
            , mmex::version::string) << "\n"
        << wxString::Format(_("New version is %s (published at %s)")
            , m_new_version["tag_name"].GetString() + 1
            , pub_date.Format(Option::instance().DateFormat())) << "\n";

    wxBoxSizer *page1_sizer = new wxBoxSizer(wxVERTICAL);
    page1->SetSizer(page1_sizer);

    wxStaticText *updateText = new wxStaticText(page1, wxID_ANY, displayMsg);
    wxStaticText *whatsnew = new wxStaticText(page1, wxID_ANY, _("What's new:"));
    wxHtmlWindow *changelog = new wxHtmlWindow(page1
        , wxID_ANY, wxDefaultPosition, wxSize(450, 250)
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    wxStaticText *instruction = new wxStaticText(page1, wxID_ANY,
        _("Click on Finish to open our download webpage."));
    // _("Click on next to download it now or visit our website to download."));
    // TODO: Download file in wizard page2

    page1_sizer->Add(updateText);
    page1_sizer->Add(whatsnew, wxSizerFlags(g_flagsV).Border(wxBOTTOM, 0));
    page1_sizer->Add(changelog, wxSizerFlags(g_flagsExpand).Border(wxTOP, 0));
    page1_sizer->Add(instruction, wxSizerFlags(g_flagsV).Border(wxTOP, 10));

    wxString body = m_new_version["body"].GetString();

    // ---- Convert Markup

    // img with link
    // skip images hosted via unsupported https
    wxRegEx re("\\[!\\[([^]]+)\\]\\(([ \t]*https://[^)]+)\\)\\]\\(([^)]+)\\)", wxRE_EXTENDED);
    re.Replace(&body, "<a href=\"\\3\">\\1</a>");
    re.Compile("\\[!\\[([^]]+)\\]\\(([^)]+)\\)\\]\\(([^)]+)\\)", wxRE_EXTENDED);
    re.Replace(&body, "<a href=\"\\3\"><img src=\"\\2\" alt=\"\\1\"></a>");

    // img
    // skip images hosted via unsupported https
    re.Compile("!\\[([^]]+)\\]\\([ \t]*https://[^)]+\\)", wxRE_EXTENDED);
    re.Replace(&body, "\\1");
    re.Compile("!\\[([^]]+)\\]\\(([^)]+)\\)", wxRE_EXTENDED);
    re.Replace(&body, "<img src=\"\\2\" alt=\"\\1\">");

    // link
    re.Compile("\\[([^]]+)\\]\\(([^)]+)\\)", wxRE_EXTENDED);
    re.Replace(&body, "<a href=\"\\2\">\\1</a>");

    body.Replace("\n", "\n<p>");
    wxLogDebug("loading webpage:\n%s", body);
    wxImage::AddHandler(new wxPNGHandler);
    changelog->SetPage("<html><body>" + body + "</body></html>");

    GetPageAreaSizer()->Add(page1);
}

void mmUpdateWizard::RunIt(bool modal)
{
    if (modal)
    {
        if (RunWizard(page1))
        {
            wxLaunchDefaultBrowser(m_new_version["html_url"].GetString()); //TODO: Download file in wizard page2
        }

        Destroy();
    }
    else
    {
        FinishLayout();
        ShowPage(page1);
        Show(true);
    }
}

void mmUpdateWizard::LinkClicked(wxHtmlLinkEvent& evt)
{
    wxLaunchDefaultBrowser(evt.GetLinkInfo().GetHref());
}

struct Version
{
    long v[5];

    explicit Version(const wxString& tag)
    {
        for (int i = 0; i < 5; i++) v[i] = 0;
        wxRegEx re_ver("^v([0-9]+)\\.([0-9]+)(\\.([0-9]+))?(-(alpha|beta|rc)(\\.([0-9]+))?)?$", wxRE_EXTENDED);
        if (re_ver.Matches(tag))
            for (size_t i = 0; i < re_ver.GetMatchCount(); i++)
            {
                wxString val = re_ver.GetMatch(tag, i);
                switch (i)
                {
                case 1: val.ToCLong(&v[0]); break;
                case 2: val.ToCLong(&v[1]); break;
                case 4: val.ToCLong(&v[2]); break;
                case 6: if (val == "alpha") v[3] = -3;
                        else if (val == "beta") v[3] = -2;
                        else v[3] = -1;
                    break;
                case 8: val.ToCLong(&v[4]); break;
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

    const int _stable = Model_Setting::instance().GetIntSetting("UPDATESOURCE", 0) == 0;

    const wxString current_tag = ("v" + mmex::version::string).Lower();
    wxString latest_tag = current_tag;
    Value& latest_release = json_releases[0];
    Version latest(latest_tag);
    wxLogDebug("Current vertion: = %s", current_tag);

    for (auto& r : json_releases.GetArray())
    {
        const auto tag_name = r["tag_name"].GetString();
        if (_stable && r["prerelease"].IsTrue()) {
            wxLogDebug("[X] tag %s", tag_name);
            continue;
        }

        Version check(tag_name);
        if (latest < check)
        {
            latest = check;
            latest_tag = tag_name;
            if (latest_release != r) {
                latest_release = r;
                wxLogDebug("[V] ----> tag %s", tag_name);
            }
        }
        else
        {
            wxLogDebug("[X] tag %s", tag_name);
        }
    }

    if (current_tag != latest_tag)
    {
        mmUpdateWizard* wizard = new mmUpdateWizard(frame, latest_release);
        wizard->CenterOnParent();
        wizard->RunIt(true);
    }
    else if (!bSilent)
    {
        wxMessageBox(_("You already have the latest version"),
            _("MMEX Update Check"), wxICON_INFORMATION);
    }
}
