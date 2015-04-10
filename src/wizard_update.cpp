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

mmUpdateWizard::mmUpdateWizard(wxFrame *frame, const wxString& NewVersion)
    : wxWizard(frame, wxID_ANY, _("Update Wizard")
    , wxBitmap(wxNullBitmap), wxDefaultPosition, wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER)
    , NewVersion()
{
    mmUpdateWizard::NewVersion = NewVersion;
    page1 = new wxWizardPageSimple(this);

    const wxString displayMsg = wxString()
        << _("A new version of MMEX is available!") << "\n\n"
        << wxString::Format(_("Your version is %s"), mmex::version::string) << "\n"
        << wxString::Format(_("New version is %s"), NewVersion) << "\n\n\n"
        << _("Click on finish to open our website and download.") << "\n\n";
        //<< _("Click on next to download it now or visit our website to download.") << "\n\n"; //TODO: Download file in wizard page2
    
    wxString URL, Changelog;
    if (Model_Setting::instance().GetIntSetting("UPDATESOURCE", 0) == 1)
        URL = mmex::weblink::Changelog + "unstable";
    else
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
    page1_sizer->Add(whatsnew, wxSizerFlags(g_flags).Border(wxBOTTOM, 0));
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
    , parent_(parent)
{
    wxString strLinks, InstallerType;
    site_content(mmex::weblink::UpdateLinks, strLinks);

    json::Object jsonLinks;
    std::wstringstream jsonLinksStream;
    std::wstring Platform = mmPlatformType().ToStdWstring();
    std::wstring ReleaseType = L"Stable";

    if (parent->NewVersion.Contains("-"))
        ReleaseType = L"Unstable";

    jsonLinksStream << strLinks.ToStdWstring();
    json::Reader::Read(jsonLinks, jsonLinksStream);

    if (Platform != "Win")
        InstallerType = "Setup";
    else
    {
        if (mmex::isPortableMode())
            InstallerType = "Portable";
        else
            InstallerType = "Setup";
        #if _WIN64
            InstallerType.Append("64");
        #else
            InstallerType.Append("32");
        #endif
    }

    DownloadURL = wxString(json::String(jsonLinks[ReleaseType][Platform][InstallerType.ToStdWstring()]));

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
    wxURL url(DownloadURL);
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
const bool mmUpdate::IsUpdateAvailable(const bool bSilent, wxString& NewVersion)
{
    bool isUpdateAvailable = false;
    NewVersion = "error";

    wxString page;
    int err_code = site_content(mmex::weblink::Update, page);
    if (err_code != wxURL_NOERR || page.Find("Unstable") == wxNOT_FOUND)
    {
        if (bSilent)
            return false;
        else
        {
            if (page == wxEmptyString)
                page = "Page not found";

            const wxString& msgStr = wxString::Format("%s\n\n%s"
                , _("Unable to check for updates!")
                , wxString::Format(_("Error: %s"), "\n" + page));
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

    json::Object jsonVersion;
    std::wstringstream jsonVersionStream;
    std::wstring platform = mmPlatformType().ToStdWstring();
    std::wstring ReleaseType = L"Stable";

    int alpha = -1;
    int beta = -1;
    int rc = -1;

    if (!(page.StartsWith("{") && page.EndsWith("}")))
        return false;
    jsonVersionStream << page.ToStdWstring();
    json::Reader::Read(jsonVersion, jsonVersionStream);

    if (Model_Setting::instance().GetIntSetting("UPDATESOURCE", 0) == 1
        || mmex::version::Alpha != -1 || mmex::version::Beta != -1 || mmex::version::RC != -1)
    {
        ReleaseType = L"Unstable";
        alpha = int(json::Number(jsonVersion[ReleaseType][platform][L"Alpha"]));
        beta = int(json::Number(jsonVersion[ReleaseType][platform][L"Beta"]));
        rc = int(json::Number(jsonVersion[ReleaseType][platform][L"RC"]));
    }

    int major = int(json::Number(jsonVersion[ReleaseType][platform][L"Major"]));
    int minor = int(json::Number(jsonVersion[ReleaseType][platform][L"Minor"]));
    int patch = int(json::Number(jsonVersion[ReleaseType][platform][L"Patch"]));

    if (major > mmex::version::Major)
        isUpdateAvailable = true;
    else if (major == mmex::version::Major)
    {
        if (minor > mmex::version::Minor)
            isUpdateAvailable = true;
        else if (minor == mmex::version::Minor)
        {
            if (patch > mmex::version::Patch)
                isUpdateAvailable = true;
            else if (patch == mmex::version::Patch && ReleaseType == L"Unstable")
            {
                if ((mmex::version::Alpha != -1 || mmex::version::Beta != -1 || mmex::version::RC != -1)
                    && alpha == -1 && beta == -1 && rc == -1)
                    isUpdateAvailable = true;
                if (rc > mmex::version::RC)
                    isUpdateAvailable = true;
                else if (rc == mmex::version::RC)
                {
                    if (beta > mmex::version::Beta)
                        isUpdateAvailable = true;
                    else if (beta == mmex::version::Beta && alpha > mmex::version::Alpha)
                        isUpdateAvailable = true;
                }
            }
        }
    }

    // define new version
    if (isUpdateAvailable)
        NewVersion = mmex::version::generateProgramVersion(major, minor, patch, alpha, beta, rc);
    else
        NewVersion = mmex::version::string;

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
