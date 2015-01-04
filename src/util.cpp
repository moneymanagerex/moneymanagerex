/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#include <wx/sstream.h>

#include "util.h"
#include "mmex.h"
#include "paths.h"
#include "constants.h"
#include "singleton.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include <wx/richtooltip.h>
#include <wx/sstream.h>

#include "validators.h"
#include "model/Model_Currency.h"
//----------------------------------------------------------------------------

int CaseInsensitiveCmp(const wxString &s1, const wxString &s2)
{
    return s1.CmpNoCase(s2);
}

//----------------------------------------------------------------------------
wxString selectLanguageDlg(wxWindow *parent, const wxString &langPath, bool verbose)
{
    wxString lang;

    wxArrayString lang_files;
    wxFileName fn(langPath, "");
    fn.AppendDir("en");
    size_t cnt = wxDir::GetAllFiles(fn.GetPath(), &lang_files, "*.mo");

    if (!cnt)
    {
        if (verbose)
        {
            wxString s = wxString::Format("Can't find language files (.mo) at \"%s\"", fn.GetPath());

            wxMessageDialog dlg(parent, s, "Error", wxOK|wxICON_ERROR);
            dlg.ShowModal();
        }

        return lang;
    }

    for (size_t i = 0; i < cnt; ++i)
    {
        wxFileName fname(lang_files[i]);
        lang_files[i] = fname.GetName().Left(1).Upper() + fname.GetName().SubString(1,fname.GetName().Len());
    }

    lang_files.Sort(CaseInsensitiveCmp);
    lang = wxGetSingleChoice("Please choose language", "Languages", lang_files, parent);

    return lang.Lower();
}


//----------------------------------------------------------------------------
void correctEmptyFileExt(const wxString& ext, wxString & fileName)
{
    wxFileName tempFileName(fileName);
    if (tempFileName.GetExt().IsEmpty())
        fileName += "." + ext;
}

/*
    locale.AddCatalog(lang) calls wxLogWarning and returns true for corrupted .mo file,
    so I should use locale.IsLoaded(lang) also.
*/
const wxString mmSelectLanguage(mmGUIApp *app, wxWindow* window, bool forced_show_dlg, bool save_setting)
{
    wxString lang;

    const wxString langPath = mmex::getPathShared(mmex::LANG_DIR);
    wxLocale &locale = app->getLocale();

    if (wxDir::Exists(langPath))
    {
        locale.AddCatalogLookupPathPrefix(langPath);
    }
    else
    {
        if (forced_show_dlg)
        {
            wxMessageDialog dlg(window
                , wxString::Format(_("Directory of language files does not exist:\n%s"), langPath)
                , _("Error"), wxOK | wxICON_ERROR);
            dlg.ShowModal();
        }

        return lang;
    }

    if (!forced_show_dlg)
    {
        lang = Model_Setting::instance().GetStringSetting(LANGUAGE_PARAMETER, "english");
        if (!lang.empty() && locale.AddCatalog(lang) && locale.IsLoaded(lang))
        {
            mmOptions::instance().language_ = lang;
            return lang;
        }
    }

    lang = selectLanguageDlg(window, langPath, forced_show_dlg);

    if (save_setting && !lang.empty())
    {
        bool ok = locale.AddCatalog(lang) && locale.IsLoaded(lang);
        if (!ok)  lang.clear(); // bad .mo file
        mmOptions::instance().language_ = lang;
        Model_Setting::instance().Set(LANGUAGE_PARAMETER, lang);
    }

    return lang;
}

const wxString inQuotes(const wxString& l, const wxString& delimiter)
{
    wxString label = l;
    if (label.Contains(delimiter) || label.Contains("\""))
    {
        label.Replace("\"","\"\"", true);
        label = wxString() << "\"" << label << "\"";
    }

    label.Replace("\t","    ", true);
    label.Replace("\n"," ", true);
    return label;
}

void mmLoadColorsFromDatabase()
{
    mmColors::userDefColor1 = Model_Infotable::instance().GetColourSetting("USER_COLOR1", wxColour(255, 0, 0));
    mmColors::userDefColor2 = Model_Infotable::instance().GetColourSetting("USER_COLOR2", wxColour(255, 165, 0));
    mmColors::userDefColor3 = Model_Infotable::instance().GetColourSetting("USER_COLOR3", wxColour(255, 255, 0));
    mmColors::userDefColor4 = Model_Infotable::instance().GetColourSetting("USER_COLOR4", wxColour(0, 255, 0));
    mmColors::userDefColor5 = Model_Infotable::instance().GetColourSetting("USER_COLOR5", wxColour(0, 255, 255));
    mmColors::userDefColor6 = Model_Infotable::instance().GetColourSetting("USER_COLOR6", wxColour(0, 0, 255));
    mmColors::userDefColor7 = Model_Infotable::instance().GetColourSetting("USER_COLOR7", wxColour(0, 0, 128));
}

/* Set the default colors */
wxColour mmColors::listAlternativeColor0 = wxColour(225, 237, 251);
wxColour mmColors::listAlternativeColor1 = wxColour(255, 255, 255);
wxColour mmColors::listBackColor = wxColour(255, 255, 255);
wxColour mmColors::navTreeBkColor = wxColour(255, 255, 255);
wxColour mmColors::listBorderColor = wxColour(0, 0, 0);
wxColour mmColors::listDetailsPanelColor = wxColour(244, 247, 251);
wxColour mmColors::listFutureDateColor = wxColour(116, 134, 168);

wxColour mmColors::userDefColor1;
wxColour mmColors::userDefColor2;
wxColour mmColors::userDefColor3;
wxColour mmColors::userDefColor4;
wxColour mmColors::userDefColor5;
wxColour mmColors::userDefColor6;
wxColour mmColors::userDefColor7;

//*-------------------------------------------------------------------------*//

int site_content(const wxString& sSite, wxString& sOutput)
 {
    wxString proxyName = Model_Setting::instance().GetStringSetting("PROXYIP", "");
    if (!proxyName.empty())
    {
        int proxyPort = Model_Setting::instance().GetIntSetting("PROXYPORT", 0);
        wxString proxySettings = wxString::Format("%s:%d", proxyName, proxyPort);
        wxURL::SetDefaultProxy(proxySettings);
    }
    else
        wxURL::SetDefaultProxy(""); // Remove prior proxy

    wxURL url(sSite);
    int err_code = url.GetError();
    if (err_code == wxURL_NOERR)
    {
        int networkTimeout = Model_Setting::instance().GetIntSetting("NETWORKTIMEOUT", 10); // default 10 secs
        url.GetProtocol().SetTimeout(networkTimeout);
        wxInputStream* in_stream = url.GetInputStream();
        if (in_stream)
        {
            wxStringOutputStream out_stream(&sOutput);
            in_stream->Read(out_stream);
        }
        else
            err_code = -1; //Cannot get data from WWW!
        delete in_stream;
    }

    if (err_code != wxURL_NOERR)
    {
        if      (err_code == wxURL_SNTXERR ) sOutput = _("Syntax error in the URL string");
        else if (err_code == wxURL_NOPROTO ) sOutput = _("Found no protocol which can get this URL");
        else if (err_code == wxURL_NOHOST  ) sOutput = _("A host name is required for this protocol");
        else if (err_code == wxURL_NOPATH  ) sOutput = _("A path is required for this protocol");
        else if (err_code == wxURL_CONNERR ) sOutput = _("Connection error");
        else if (err_code == wxURL_PROTOERR) sOutput = _("An error occurred during negotiation");
        else if (err_code == -1) sOutput = _("Cannot get data from WWW!");
        else sOutput = _("Unknown error");
    }
    return err_code;
}

bool download_file(const wxString& site, const wxString& path)
{
    wxFileSystem fs;
    wxFileSystem::AddHandler(new wxInternetFSHandler());
    wxFSFile *file = fs.OpenFile(site);
    if (file != NULL)
    {
        wxInputStream *in = file->GetStream();
        if (in != NULL)
        {
            wxFileOutputStream output(path);
            output.Write(*file->GetStream());
            output.Close();
            delete in;
            return true;
        }
    }

    return false;
}

//* Date Functions----------------------------------------------------------*//

const wxString mmGetNiceDateSimpleString(const wxDateTime &dt)
{
    wxString dateFmt = mmOptions::instance().dateFormat_;
    dateFmt.Replace("%Y%m%d", "%Y %m %d");
    dateFmt.Replace(".", " ");
    dateFmt.Replace(",", " ");
    dateFmt.Replace("/", " ");
    dateFmt.Replace("-", " ");
    dateFmt.Replace("%d", wxString::Format("%d", dt.GetDay()));
    dateFmt.Replace("%Y", wxString::Format("%d", dt.GetYear()));
    dateFmt.Replace("%y", wxString::Format("%d", dt.GetYear()).Mid(2,2));
    dateFmt.Replace("%m", wxGetTranslation(wxDateTime::GetEnglishMonthName(dt.GetMonth())));

    return dateFmt;
}

const wxString mmGetDateForDisplay(const wxDateTime &dt)
{
    return dt.Format(mmOptions::instance().dateFormat_);
}

bool mmParseDisplayStringToDate(wxDateTime& date, const wxString& sDate, const wxString &sDateMask)
{
    wxString mask = sDateMask;
    const wxDateTime today = date;
    mask.Replace("%Y%m%d", "%Y %m %d");
    if (date_formats_regex().count(mask) == 0) return false;

    const wxString regex = date_formats_regex().at(mask);
    wxRegEx pattern(regex);
    //skip dot if present in pattern but not in date string 
    const wxString separator = mask.Mid(2,1);
    date.ParseFormat(sDate, mask, today);
    if (pattern.Matches(sDate) && sDate.Contains(separator))
        return true;
    else {
        //wxLogDebug("%s %s %i %s", sDate, mask, pattern.Matches(sDate), regex);
        return false;
    }
}

const wxDateTime mmGetStorageStringAsDate(const wxString& str)
{
    wxDateTime dt = wxDateTime::Today();
    if (!str.IsEmpty()) dt.ParseDate(str);
    if (!dt.IsValid()) dt = wxDateTime::Today();
    if (dt.GetYear()<100) dt.Add(wxDateSpan::Years(2000));
    return dt;
}

const wxDateTime getUserDefinedFinancialYear(bool prevDayRequired)
{
    long monthNum;
    mmOptions::instance().financialYearStartMonthString_.ToLong(&monthNum);

    if (monthNum > 0) //Test required for compatability with previous version
        monthNum --;

    wxDateTime today = wxDateTime::Today();
    int year = today.GetYear();
    if (today.GetMonth() < monthNum) year -- ;

    long dayNum;
    wxString dayNumStr = mmOptions::instance().financialYearStartDayString_;
    dayNumStr.ToLong(&dayNum);
    if ((dayNum < 1) || (dayNum > 31 )) {
        dayNum = 1;
    } else if (((monthNum == wxDateTime::Feb) && (dayNum > 28)) ||
        (((monthNum == wxDateTime::Sep) || (monthNum == wxDateTime::Apr) ||
           (monthNum == wxDateTime::Jun) || (monthNum == wxDateTime::Nov)) && (dayNum > 29)))
    {
        dayNum = 1;
    }

    wxDateTime financialYear = wxDateTime(today);
    financialYear.SetDay(dayNum);
    financialYear.SetMonth((wxDateTime::Month)monthNum);
    financialYear.SetYear(year);
    if (prevDayRequired)
        financialYear.Subtract(wxDateSpan::Day());
    return financialYear;
}

const std::map<wxString,wxString> date_formats_map()
{
    std::map<wxString, wxString> date_formats;
    date_formats["%Y-%m-%d"] = "YYYY-MM-DD";
    date_formats["%d/%m/%y"] = "DD/MM/YY";
    date_formats["%d/%m/%Y"] = "DD/MM/YYYY";
    date_formats["%d-%m-%y"] = "DD-MM-YY";
    date_formats["%d-%m-%Y"] = "DD-MM-YYYY";
    date_formats["%d.%m.%y"] = "DD.MM.YY";
    date_formats["%d.%m.%Y"] = "DD.MM.YYYY";
    date_formats["%d,%m,%y"] = "DD,MM,YY";
    date_formats["%d/%m'%Y"] = "DD/MM'YYYY";
    date_formats["%d/%m %Y"] = "DD/MM YYYY";
    date_formats["%m/%d/%y"] = "MM/DD/YY";
    date_formats["%m/%d/%Y"] = "MM/DD/YYYY";
    date_formats["%m-%d-%y"] = "MM-DD-YY";
    date_formats["%m-%d-%Y"] = "MM-DD-YYYY";
    date_formats["%m/%d'%y"] = "MM/DD'YY";
    date_formats["%m/%d'%Y"] = "MM/DD'YYYY";
    date_formats["%y/%m/%d"] = "YY/MM/DD";
    date_formats["%y-%m-%d"] = "YY-MM-DD";
    date_formats["%Y/%m/%d"] = "YYYY/MM/DD";
    date_formats["%Y.%m.%d"] = "YYYY.MM.DD";
    date_formats["%Y%m%d"] = "YYYYMMDD";

    return date_formats;
}

const std::map<wxString,wxString> date_formats_regex()
{
    const wxString dd = "((([0 ][1-9])|([1-2][0-9])|(3[0-1]))|([1-9]))";
    const wxString mm = "((([0 ][1-9])|(1[0-2]))|([1-9]))";
    const wxString yy = "([0-9]{2})";
    const wxString yyyy = "(((19)|([2]([0]{1})))([0-9]{2}))";
    std::map<wxString, wxString> date_regex;
    date_regex["%d/%m/%y"] = wxString::Format("^%s/%s/%s$", dd, mm, yy);
    date_regex["%d/%m/%Y"] = wxString::Format("^%s/%s/%s$", dd, mm, yyyy);
    date_regex["%d-%m-%y"] = wxString::Format("^%s-%s-%s$", dd, mm, yy);
    date_regex["%d-%m-%Y"] = wxString::Format("^%s-%s-%s$", dd, mm, yyyy);
    date_regex["%d.%m.%y"] = wxString::Format("^%s\x2E%s\x2E%s$", dd, mm, yy);
    date_regex["%d.%m.%Y"] = wxString::Format("^%s\x2E%s\x2E%s$", dd, mm, yyyy);
    date_regex["%d,%m,%y"] = wxString::Format("^%s,%s,%s$", dd, mm, yyyy);
    date_regex["%d/%m'%Y"] = wxString::Format("^%s/%s'%s$", dd, mm, yyyy);
    date_regex["%d/%m %Y"] = wxString::Format("^%s/%s %s$", dd, mm, yyyy);
    date_regex["%m/%d/%y"] = wxString::Format("^%s/%s/%s$", mm, dd, yy);
    date_regex["%m/%d/%Y"] = wxString::Format("^%s/%s/%s$", mm, dd, yyyy);
    date_regex["%m-%d-%y"] = wxString::Format("^%s-%s-%s$", mm, dd, yy);
    date_regex["%m-%d-%Y"] = wxString::Format("^%s-%s-%s$", mm, dd, yyyy);
    date_regex["%m/%d'%y"] = wxString::Format("^%s/%s'%s$", dd, mm, yy);
    date_regex["%m/%d'%Y"] = wxString::Format("^%s/%s-%s$", mm, dd, yyyy);
    date_regex["%y/%m/%d"] = wxString::Format("^%s/%s/%s$", yy, mm, dd);
    date_regex["%y-%m-%d"] = wxString::Format("^%s-%s-%s$", dd, mm, yy);
    date_regex["%Y/%m/%d"] = wxString::Format("^%s/%s/%s$", yyyy, mm, dd);
    date_regex["%Y-%m-%d"] = wxString::Format("^%s-%s-%s$", yyyy, mm, dd);
    date_regex["%Y.%m.%d"] = wxString::Format("^%s\x2E%s\x2E%s$", yyyy, mm, dd);
    date_regex["%Y %m %d"] = wxString::Format("^%s %s %s$", yyyy, mm, dd);

    return date_regex;
}

static const wxString MONTHS[12] =
{
    wxTRANSLATE("January"), wxTRANSLATE("February"), wxTRANSLATE("March")
    , wxTRANSLATE("April"), wxTRANSLATE("May "), wxTRANSLATE("June")
    , wxTRANSLATE("July"), wxTRANSLATE("August"), wxTRANSLATE("September")
    , wxTRANSLATE("October"), wxTRANSLATE("November"), wxTRANSLATE("December")
};

static const wxString MONTHS_SHORT[12] =
{
    wxTRANSLATE("Jan"), wxTRANSLATE("Feb"), wxTRANSLATE("Mar")
    , wxTRANSLATE("Apr"), wxTRANSLATE("May"), wxTRANSLATE("Jun")
    , wxTRANSLATE("Jul"), wxTRANSLATE("Aug"), wxTRANSLATE("Sep")
    , wxTRANSLATE("Oct"), wxTRANSLATE("Nov"), wxTRANSLATE("Dec")
};

static const wxString gDaysInWeek[7] =
{
    wxTRANSLATE("Sunday"), wxTRANSLATE("Monday"), wxTRANSLATE("Tuesday")
    , wxTRANSLATE("Wednesday"), wxTRANSLATE("Thursday"), wxTRANSLATE("Friday")
    , wxTRANSLATE("Saturday")
};

/* Error Messages --------------------------------------------------------*/
void mmShowErrorMessage(wxWindow *parent
    , const wxString &message, const wxString &messageheader)
{
    wxMessageDialog msgDlg(parent, message, messageheader, wxOK | wxICON_ERROR);
    msgDlg.ShowModal();
}

void mmShowWarningMessage(wxWindow *parent
    , const wxString &message, const wxString &messageheader)
{
    wxMessageDialog msgDlg(parent, message, messageheader, wxOK | wxICON_WARNING);
    msgDlg.ShowModal();
}

void mmShowErrorMessageInvalid(wxWindow *parent, const wxString &message)
{
    const wxString& msg = wxString::Format(_("Entry %s is invalid"), message);
    mmShowErrorMessage(parent, msg, _("Invalid Entry"));
}

void mmMessageCategoryInvalid(wxWindow *button)
{
    wxRichToolTip tip(_("Invalid Category"),
        _("Please use this button for category selection\nor use the 'Split' checkbox for multiple categories.")
        + "\n");
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(button);
}

void mmMessageFileInvalid(wxWindow *object, bool open)
{
    const wxString& errorHeader = open ? _("Unable to open file.") : _("File name is empty.");
    wxString errorMessage = _("Please select the file for this operation.");

    const wxString errorTips = _("Selection can be made by using Search button.");
    errorMessage = errorMessage + "\n\n" + errorTips + "\n";

    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(object);
}

void mmMessageAccountInvalid(wxWindow *object, bool transfer)
{
    const wxString& errorHeader = _("Invalid Account");
    wxString errorMessage;
    if (!transfer)
        errorMessage = _("Please select the account for this transaction.");
    else
        errorMessage = _("Please specify which account the transfer is going to.");

    wxString errorTips = _("Selection can be made by using the dropdown button.");
    errorMessage = errorMessage + "\n\n" + errorTips + "\n";

    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(object);
}

void mmMessagePayeeInvalid(wxWindow *object)
{
    const wxString& errorHeader = _("Invalid Payee");
    const wxString& errorMessage = (_("Please type in a new payee,\nor make a selection using the dropdown button.")
        + "\n");
    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(object);
}

void mmMessageNameInvalid(wxTextCtrl *textBox)
{
    const wxString& errorHeader = _("Invalid Name");
    const wxString& errorMessage = (_("Please type in a non empty name.")
        + "\n");
    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor((wxWindow*) textBox);
}

//
const wxString mmPlatformType()
{
	return wxPlatformInfo::Get().GetOperatingSystemFamilyName().substr(0, 3);
}

const wxString getURL(const wxString& file)
{
	wxString index = file;
#ifdef __WXGTK__
    index.Prepend("file://");
#endif
    return index;
}

const bool IsUpdateAvailable(const bool& bSilent, wxString& NewVersion)
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

            const wxString msgStr = wxString::Format("%s\n\n%s"
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
                if (alpha > mmex::version::Alpha)
                    isUpdateAvailable = true;
                else if (alpha == mmex::version::Alpha)
                {
                    if (beta > mmex::version::Beta)
                        isUpdateAvailable = true;
                    else if (beta == mmex::version::Beta && rc > mmex::version::RC)
                        isUpdateAvailable = true;
                }
            }
        }
    }

    // define new version
    if (isUpdateAvailable)
        NewVersion = mmex::version::generateProgramVersion(major, minor, patch, alpha, beta, rc);
    else
        NewVersion = mmex::getProgramVersion();

    return isUpdateAvailable;
}

void checkUpdates(const bool& bSilent)
{
    wxString NewVersion = wxEmptyString;
    if (IsUpdateAvailable(bSilent, NewVersion) && NewVersion != "error")
    {
        const wxString msgStr = wxString() << _("New version of MMEX is available") << "\n\n"
            << wxString::Format(_("Your current version is: %s"), mmex::getProgramVersion()) << "\n"
            << wxString::Format(_("New version is: %s"), NewVersion) << "\n\n"
            << _("Would you like to download it now ?");
        int DowloadResponse = wxMessageBox(msgStr,
            _("MMEX Update Check"), wxICON_EXCLAMATION | wxYES | wxNO);
        if (DowloadResponse == wxYES)
        {
            wxString DownloadURL = mmex::weblink::Download;
            if (NewVersion.Contains("-"))
                DownloadURL.Append("#Unstable");

            wxLaunchDefaultBrowser(DownloadURL);
        }
    }
    else if (!bSilent && NewVersion != "error")
    {
        const wxString msgStr = wxString::Format(_("You already have the latest version %s"), NewVersion);
        wxMessageBox(msgStr, _("MMEX Update Check"), wxICON_INFORMATION);
    }
}

void windowsFreezeThaw(wxWindow* w)
{
#ifdef __WXGTK__
    return;
#endif

    if (w->IsFrozen())
        w->Thaw();
    else
        w->Freeze();
}

#if 1
// ----------------------------------------------------------------------------
// mmCalcValidator
// Same as previous, but substitute dec char according to currency configuration
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(mmCalcValidator, wxTextValidator)
BEGIN_EVENT_TABLE(mmCalcValidator, wxTextValidator)
EVT_CHAR(mmCalcValidator::OnChar)
END_EVENT_TABLE()

mmCalcValidator::mmCalcValidator() : wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST)
{
    wxArrayString list;
    wxString valid_chars(" 1234567890.,(/+-*)");
    size_t len = valid_chars.Length();
    for (size_t i=0; i<len; i++) {
        list.Add(wxString(valid_chars.GetChar(i)));
    }
    SetIncludes(list);
    const Model_Currency::Data *base_currency = Model_Currency::instance().GetBaseCurrency();
    decChar = base_currency->DECIMAL_POINT[0];
}

void mmCalcValidator::OnChar(wxKeyEvent& event)
{
    if (!m_validatorWindow)
    {
        event.Skip();
        return;
    }

    int keyCode = event.GetKeyCode();

    // we don't filter special keys and delete
    if (keyCode < WXK_SPACE || keyCode == WXK_DELETE || keyCode >= WXK_START)
    {
        event.Skip();
        return;
    }

    wxString str((wxUniChar)keyCode, 1);
    if (!wxIsdigit(str[0]) && str != '+' && str != '-' && str != '.' && str != ',')
    {
        if ( !wxValidator::IsSilent() )
            wxBell();

        // eat message
        return;
    }
    // only if it's a wxTextCtrl
    if (!m_validatorWindow || !wxDynamicCast(m_validatorWindow, wxTextCtrl))
    {
        event.Skip();
        return;
    }
    // if decimal point, check if it's already in the string
    if (str == '.' || str == ',')
    {
        wxString value = ((wxTextCtrl*)m_validatorWindow)->GetValue();
        size_t ind = value.rfind(decChar);
        if (ind < value.Length())
        {
            // check if after last decimal point there is an operation char (+-/*)
            if (value.find('+', ind+1) >= value.Length() && value.find('-', ind+1) >= value.Length() &&
                value.find('*', ind+1) >= value.Length() && value.find('/', ind+1) >= value.Length())
                return;
        }
        if (str != decChar)
        {
#ifdef _MSC_VER
            wxChar vk = decChar == '.' ? 0x6e : 0xbc;
            keybd_event(vk,0xb3,0 , 0);
            keybd_event(vk,0xb3, KEYEVENTF_KEYUP,0);
            return;
#endif
        }
    }
    event.Skip();
}
#endif
