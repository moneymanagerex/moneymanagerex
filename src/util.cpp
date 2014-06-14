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

#include "util.h"
#include "mmex.h"
#include "paths.h"
#include "constants.h"
#include "singleton.h"
#include "model/Model_Setting.h"
#include <wx/richtooltip.h>
#include <wx/sstream.h>

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
        fileName << "." << ext;
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

const wxString inQuotes(wxString label, wxString& delimiter)
{
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
    mmColors::userDefColor1   = Model_Setting::instance().GetColourSetting("USER_COLOR1", wxColour(255,0,0));
    mmColors::userDefColor2   = Model_Setting::instance().GetColourSetting("USER_COLOR2", wxColour(255,165,0));
    mmColors::userDefColor3   = Model_Setting::instance().GetColourSetting("USER_COLOR3", wxColour(255,255,0));
    mmColors::userDefColor4   = Model_Setting::instance().GetColourSetting("USER_COLOR4", wxColour(0,255,0));
    mmColors::userDefColor5   = Model_Setting::instance().GetColourSetting("USER_COLOR5", wxColour(0,255,255));
    mmColors::userDefColor6   = Model_Setting::instance().GetColourSetting("USER_COLOR6", wxColour(0,0,255));
    mmColors::userDefColor7   = Model_Setting::instance().GetColourSetting("USER_COLOR7", wxColour(0,0,128));
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
        url.GetProtocol().SetTimeout(10); // 10 secs
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
    mask.Replace("%Y%m%d", "%Y %m %d");
    if (date_formats_regex().count(mask) == 0) return false;

    const wxString regex = date_formats_regex().at(mask);
    wxRegEx pattern(regex);
    //wxLogDebug("%s %s %i %s", sDate, mask, pattern.Matches(sDate), regex);
    //skip dot if present in pattern but not in date string 
    const wxString separator = mask.Mid(2,1);
    if (pattern.Matches(sDate) && sDate.Contains(separator))
    {
        date.ParseFormat(sDate, mask, wxDateTime::Today());
        return date.IsValid();
    }
    return false;
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
    const wxString dd = "(((0[1-9])|([1-2][0-9])|(3[0-1]))|([1-9]))";
    const wxString mm = "(((0[1-9])|(1[0-2]))|([1-9]))";
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

void mmShowErrorMessageInvalid(wxWindow *parent, const wxString &message)
{
    wxString msg = wxString::Format(_("Entry %s is invalid"), message);
    mmShowErrorMessage(parent, msg, _("Invalid Entry"));
}

void mmMessageCategoryInvalid(wxButton *button)
{
    wxRichToolTip tip(_("Invalid Category"),
        _("Please use this button for category selection\nor use the 'Split' checkbox for multiple categories.")
        + "\n");
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(button);
}

void mmMessageAccountInvalid(wxComboBox *comboBox, bool transfer)
{
    const wxString errorHeader = _("Invalid Account");
    wxString errorMessage;
    if (!transfer)
    {
        errorMessage = _("Please select the account for this transaction.");
    }
    else
    {
        errorMessage = _("Please specify which account the transfer is going to.");
    }
    wxString errorTips = _("Selection can be made by using the dropdown button.");
    errorMessage = errorMessage + "\n\n" + errorTips + "\n";

    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor((wxWindow*) comboBox);
}

void mmMessagePayeeInvalid(wxComboBox *comboBox)
{
    const wxString errorHeader = _("Invalid Payee");
    const wxString errorMessage = (_("Please type in a new payee,\nor make a selection using the dropdown button.")
        + "\n");
    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor((wxWindow*)comboBox);
}

void mmMessageNameInvalid(wxTextCtrl *textBox)
{
    const wxString errorHeader = _("Invalid Name");
    const wxString errorMessage = (_("Please type in a non empty name.")
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

    // Access current version details page
    wxString site = mmex::getProgramWebSite() + "/version.html";

    wxString page;
    int err_code = site_content(site, page);
    if (!bSilent && (err_code != wxURL_NOERR || page.Find("Unix:") == wxNOT_FOUND))
    {
        if (page == wxEmptyString)
            page = "Page not found";

        wxString msgStr = wxString() << _("Unable to check for updates!") << "\n\n"
            << _("Error code:") << "\n"
            << page;
        wxMessageBox(msgStr, _("MMEX Update Check"));
        return false;
    }

    /*************************************************************************
    Note: To allow larger digit counters and maintain backward compatability,
    the leading counters before the character [ is ignored by the version
    checking routines.

    Expected string format from the internet up to Version: 0.9.9.0
    page = "x.x.x.x - Win: w.w.w.w - Unix: u.u.u.u - Mac: m.m.m.m";
    string length = 53 characters
    **************************************************************************/
    // Included for future testing
    // Old format of counters
    // page = "x.x.x.x - Win: w.w.w.w - Unix: u.u.u.u - Mac: m.m.m.m";
    // page = "9.9.9.9 - Win: 0.9.9.0 - Unix: 0.9.9.0 - Mac: 0.9.9.0";

    // New format to allow counters greater than 9
    // page = "9.9.9.9 - Win: 9.9.9.9 - Unix: 9.9.9.9 - Mac: 9.9.9.9 -[ Win: 1.1.0.12 - Unix: 0.9.10.0 - Mac: 0.9.9.10";
    // page = "9.9.9.9 - Win: 9.9.9.9 - Unix: 9.9.9.9 - Mac: 9.9.9.9 -[ Win: 1.1.0 - Unix: 0.9.10.0 - Mac: 0.9.9.10";
    // page = "9.9.9.9 - Win: 0.9.9.0 - Unix: 0.9.9.0 - Mac: 0.9.9.0 -[ Win: 0.9.9.2 - Unix: 0.9.9.2 - Mac: 0.9.9.2";
    // page = "9.9.9.9 - Win: 9.9.9.9 - Unix: 9.9.9.9 - Mac: 9.9.9.9 -[ Mac: 0.9.9.3 - Unix: 0.9.9.3 - Win: 2.10.19";

    wxStringTokenizer versionTokens(page, ("["));
    versionTokens.GetNextToken(); // ignore old counters
    page = versionTokens.GetNextToken(); // substrtute new counters

    page = page.SubString(page.find(mmPlatformType()), 53);
    wxStringTokenizer mySysToken(page, ":");
    mySysToken.GetNextToken().Trim(false).Trim();           // skip Operating System. Already accounted for.
    page = mySysToken.GetNextToken().Trim(false).Trim();    // Get version for OS

    wxStringTokenizer tkz(page, '.', wxTOKEN_RET_EMPTY_ALL);
    if (tkz.CountTokens() < 3)
        return true;

    int major = wxAtoi(tkz.GetNextToken());
    int minor = wxAtoi(tkz.GetNextToken());
    int patch = wxAtoi(tkz.GetNextToken());
    int rc = wxAtoi(tkz.GetNextToken());

    // get current version
    wxString currentV = mmex::getProgramVersion();

    // get release candidate version if it exists.
    int rc_C = rc;
    if (currentV.find("RC") != -1)
    {
        wxString rc_ver = currentV.SubString(currentV.find("RC") + 2, currentV.Length()).Trim();
        rc_C = wxAtoi(rc_ver);
    }

    currentV = currentV.SubString(0, currentV.Find("RC") - 1).Trim();
    wxStringTokenizer tkz1(currentV, ('.'), wxTOKEN_RET_EMPTY_ALL);

    int majorC = wxAtoi(tkz1.GetNextToken());
    int minorC = wxAtoi(tkz1.GetNextToken());
    int patchC = wxAtoi(tkz1.GetNextToken());

    if (major > majorC)
        isUpdateAvailable = true;
    else if (major == majorC)
    {
        if (minor > minorC)
        {
            isUpdateAvailable = true;
        }
        else if (minor == minorC)
        {
            if (patch > patchC)
            {
                isUpdateAvailable = true;
            }
            else if (patch == patchC)
            {
                if ((rc == 0) || (rc > rc_C))
                    isUpdateAvailable = true;
            }
        }
    }

    // define new version
    NewVersion = wxString() << majorC << "." << minorC << "." << patchC;
    if (rc > 0)
        NewVersion << "-RC" << rc_C;

    return isUpdateAvailable;
}

void checkUpdates(const bool& bSilent)
{
    wxString NewVersion = wxEmptyString;

    if (IsUpdateAvailable(bSilent, NewVersion) && NewVersion != "error")
    {
        wxString urlDownload = mmex::getProgramWebSite() + "/download";
        wxString msgStr = wxString() << _("New version of MMEX is available") << "\n\n"
            << _("Your current version is: ") << mmex::getProgramVersion() << "\n"
            << _("New version is: ") << NewVersion << "\n\n"
            << _("Would you like to download it now ?");
        int DowloadResponse = wxMessageBox(msgStr,
            _("MMEX Update Check"), wxICON_EXCLAMATION | wxYES | wxNO);
        if (DowloadResponse == wxYES)
            wxLaunchDefaultBrowser(urlDownload);
    }
    else if (!bSilent && NewVersion != "error")
    {
        wxString msgStr = wxString() << _("You already have the latest version") << " " << NewVersion;
        wxMessageBox(msgStr, _("MMEX Update Check"), wxICON_INFORMATION);
    }
}