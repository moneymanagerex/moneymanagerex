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
#include "paths.h"
#include "util.h"
#include "model/Model_Setting.h"
#include <wx/progdlg.h>
#include <wx/url.h>

wxBEGIN_EVENT_TABLE(mmUpdateWizard, wxWizard)
    EVT_WIZARD_PAGE_CHANGED(wxID_ANY, mmUpdateWizard::PageChanged)
wxEND_EVENT_TABLE()

mmUpdateWizard::mmUpdateWizard(wxFrame *frame, const wxString& new_version)
    : wxWizard(frame, wxID_ANY, _("Update Wizard")
    , wxBitmap(wxNullBitmap), wxDefaultPosition, wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER)
    , m_new_version()
{
    mmUpdateWizard::m_new_version = new_version;
    page1 = new wxWizardPageSimple(this);

    const wxString displayMsg = wxString()
        << _("A new version of MMEX is available!") << "\n\n"
        << wxString::Format(_("Your version is %s"), mmex::version::string) << "\n"
        << wxString::Format(_("New version is %s"), m_new_version) << "\n\n\n"
        << _("Click on finish to open our website and download.") << "\n\n";
        //<< _("Click on next to download it now or visit our website to download.") << "\n\n"; //TODO: Download file in wizard page2
    
    wxString URL, Changelog;
    if (Model_Setting::instance().GetIntSetting("UPDATESOURCE", 0) == 1)
        URL = mmex::weblink::Changelog + "unstable";
    else //TODO: something...
        URL = mmex::weblink::Changelog + "unstable";
    site_content(URL, Changelog);

    wxBoxSizer *page1_sizer = new wxBoxSizer(wxVERTICAL);
    page1->SetSizer(page1_sizer);

    wxStaticText *updateText = new wxStaticText(page1, wxID_ANY, displayMsg);
    wxStaticText *whatsnew = new wxStaticText(page1, wxID_ANY, _("What's new:"));
    wxHtmlWindow *changelog = new wxHtmlWindow(page1
        , wxID_ANY, wxDefaultPosition, wxSize(450, 250)
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);

    page1_sizer->Add(updateText);
    page1_sizer->Add(whatsnew, wxSizerFlags(g_flagsV).Border(wxBOTTOM, 0));
    page1_sizer->Add(changelog, wxSizerFlags(g_flagsExpand).Border(wxTOP,0));
    changelog->LoadPage(URL);

    //mmUpdateWizardPage2* page2 = new mmUpdateWizardPage2(this); //TODO: Download file in wizard page2

    //wxWizardPageSimple::Chain(page1, page2); //TODO: Download file in wizard page2

    GetPageAreaSizer()->Add(page1);
}

void mmUpdateWizard::RunIt(bool modal)
{
    if (modal)
    {
        if (RunWizard(page1))
        {
            wxLaunchDefaultBrowser(mmex::weblink::Download); //TODO: Download file in wizard page2
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

void mmUpdateWizard::PageChanged(wxWizardEvent& evt)
{
    //if (evt.GetDirection && evt.GetPage == mmUpdateWizard.pages[1]) //TODO: Download file in wizard page2
    //self.pages[1].timer.Start(1000) //TODO
}

//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(mmUpdateWizardPage2, wxWizardPageSimple)
wxEND_EVENT_TABLE()

mmUpdateWizardPage2::mmUpdateWizardPage2(mmUpdateWizard* parent)
    : wxWizardPageSimple(parent)
{
    wxString json_links, installer_type;
    site_content(mmex::weblink::UpdateLinks, json_links);

    Document json_doc;
    json_doc.Parse(json_links);

    wxLogDebug("======= mmUpdateWizardPage2::mmUpdateWizardPage2 =======");
    wxLogDebug("RapidJson\n%s", JSON_PrettyFormated(json_doc));

    wxString platform = mmPlatformType();
    wxString release_type = "Stable";

    if (parent->m_new_version.Contains("-"))
    {
        release_type = "Unstable";
    }

    if (platform != "Win")
        installer_type = "Setup";
    else
    {
        if (mmex::isPortableMode())
            installer_type = "Portable";
        else
            installer_type = "Setup";
        #if _WIN64
            installer_type.Append("64");
        #else
            installer_type.Append("32");
        #endif
    }

    auto& json_doc_allocator = json_doc.GetAllocator();
    Value v_release_type(release_type, json_doc_allocator);
    Value v_platform(platform, json_doc_allocator);
    Value v_installer_type(installer_type, json_doc_allocator);

    m_download_url = json_doc[v_release_type][v_platform][v_installer_type].GetString();

    wxBoxSizer *page2_sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText *downloadText = new wxStaticText(this, wxID_ANY, _("Download in progress"));
    wxGauge *progressBar = new wxGauge(this, wxID_SAVE, 100);

    page2_sizer->Add(downloadText);
    page2_sizer->Add(progressBar);

    SetSizer(page2_sizer);
    page2_sizer->Fit(this);
}

void mmUpdateWizardPage2::OnDownload()
{
    wxGauge* progressBar = (wxGauge*)FindWindow(wxID_SAVE);
    wxURL url(m_download_url);
    wxInputStream *in = url.GetInputStream();
    if (in && in->IsOk())
    {
#define BUFSIZE 0x10000
        unsigned char tempbuf[BUFSIZE];
        wxMemoryBuffer buf;
        wxFileOutputStream* localFile = new wxFileOutputStream("aaa.zip");

        size_t total_len = in->GetSize();
        size_t data_loaded = 0;
        bool abort = false;

        while (in->CanRead() && !in->Eof() && !abort)
        {
            in->Read(tempbuf, BUFSIZE);
            size_t readlen = in->LastRead();
            localFile->WriteAll(tempbuf, readlen);
            progressBar->SetRange(total_len);

            if (readlen > 0)   {
                buf.AppendData(tempbuf, readlen);
                data_loaded += readlen;
            }

            if (total_len > 0)
            {
                // if we know the length of the file, display correct progress
                progressBar->SetValue(data_loaded);
            }
            else
            {
                // if we don't know the length of the file, just Pulse
                progressBar->Pulse();
            }
        }

        if (abort)
        {
            wxLogMessage("Download was cancelled.");
        }
        else
        {
            // wxMemoryBuffer buf now contains the downloaded data
            wxLogMessage("Downloaded %ld bytes", buf.GetDataLen());
        }
    }
    delete in;
}

//--------------
//mmUpdate Class
//--------------
const bool mmUpdate::IsUpdateAvailable(const bool bSilent, wxString& new_version)
{
    bool isUpdateAvailable = false;
    new_version = "error";

    wxString json_links;
    CURLcode err_code = site_content(mmex::weblink::Update, json_links);
    if (err_code != CURLE_OK || json_links.Find("Unstable") == wxNOT_FOUND)
    {
        if (bSilent)
            return false;
        else
        {
            if (json_links == wxEmptyString)
                json_links = "Page not found";

            const wxString& msgStr = wxString::Format("%s\n\n%s"
                , _("Unable to check for updates!")
                , wxString::Format(_("Error: %s"), "\n" + json_links));
            wxMessageBox(msgStr, _("MMEX Update Check"));
            return false;
        }
    }

    /*************************************************************************
    Sample JSON:
    {
      "Stable": {
        "Win": {"Major": 1, "Minor": 2, "Patch": 0},
        "Uni": {"Major": 1, "Minor": 2, "Patch": 0},
        "Mac": {"Major": 1, "Minor": 2, "Patch": 0}
      },
      "Unstable": {
        "Win": {"Major": 1, "Minor": 3, "Patch": 0, "Alpha": 0, "Beta": 0, "RC": 1},
        "Uni": {"Major": 1, "Minor": 3, "Patch": 0, "Alpha": 0, "Beta": 0, "RC": 1},
        "Mac": {"Major": 1, "Minor": 3, "Patch": 0, "Alpha": 0, "Beta": 0, "RC": 1}
      }
    }

    ************
    Alpha, Beta, RC = -1 means no Alpha\Beta\RC Available
    Alpha, Beta, RC = 0 means Alpha\Beta\RC without version
    When a stable is released an no unstable is available yet,
    insert in unstable the same version number of stable with Alpha\Beta\RC= -1
    **************************************************************************/

    wxString platform_type = mmPlatformType();

    int alpha = 0, beta = 0, rc = 0;

    Document json_doc;
    if (json_doc.Parse(json_links).HasParseError())
    {
        return false;
    }

    wxLogDebug("======= mmUpdate::IsUpdateAvailable =======");
    wxLogDebug("RapidJson\n%s", JSON_PrettyFormated(json_doc));

    bool unstable = mmex::version::Alpha != -1 || mmex::version::Beta != -1 || mmex::version::RC != -1;
    const wxString release_type = unstable ? "Unstable" : "Stable";
    auto& json_doc_allocator = json_doc.GetAllocator();
    Value key_platform_type(platform_type, json_doc_allocator);
    Value key_release_type(release_type, json_doc_allocator);
    Value k = json_doc[key_release_type][key_platform_type].GetObject();

    if (Model_Setting::instance().GetIntSetting("UPDATESOURCE", 0) == 1 || unstable)
    {
        alpha = k["Alpha"].GetInt();
        beta = k["Beta"].GetInt();
        rc = k["RC"].GetInt();
    }

    // Value needs to be redefined due to release_type value change.
    int major = k["Major"].GetInt();
    int minor = k["Minor"].GetInt();
    int patch = k["Patch"].GetInt();

    const auto out = wxString::Format("%02d%02d%02d%02d%02d%02d"
        , major, minor, patch
        , rc + 1, beta + 1, alpha + 1);
    const auto in = wxString::Format("%02d%02d%02d%02d%02d%02d"
        , mmex::version::Major, mmex::version::Minor, mmex::version::Patch
        , mmex::version::RC + 1, mmex::version::Beta + 1, mmex::version::Alpha + 1);

    isUpdateAvailable = in < out;
 
    // define new version
    if (isUpdateAvailable)
        new_version = mmex::version::generateProgramVersion(major, minor, patch, alpha, beta, rc);
    else
        new_version = mmex::version::string;

    return isUpdateAvailable;
}

void mmUpdate::checkUpdates(const bool bSilent, wxFrame *frame)
{
    wxString NewVersion = wxEmptyString;
    if (IsUpdateAvailable(bSilent, NewVersion) && NewVersion != "error")
    {
        mmUpdateWizard* wizard = new mmUpdateWizard(frame, NewVersion);
        wizard->CenterOnParent();
        wizard->RunIt(true);
    }
    else if (!bSilent && NewVersion != "error")
    {
        const wxString& msgStr = wxString::Format(_("You already have the latest version %s"), NewVersion);
        wxMessageBox(msgStr, _("MMEX Update Check"), wxICON_INFORMATION);
    }
}
