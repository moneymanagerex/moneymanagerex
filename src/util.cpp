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
#include "import_export/univcsvdialog.h"
#include "paths.h"
#include "constants.h"
#include "singleton.h"
#include "model/Model_Setting.h"

#include "../resources/flag.xpm"

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
wxString mmSelectLanguage(wxWindow *parent, bool forced_show_dlg, bool save_setting)
{
    wxString lang;

    const wxString langPath = mmex::getPathShared(mmex::LANG_DIR);
    wxLocale &locale = wxGetApp().getLocale();

    if (wxDir::Exists(langPath))
    {
        locale.AddCatalogLookupPathPrefix(langPath);
    }
    else
    {
        if (forced_show_dlg)
        {
            wxMessageDialog dlg(parent
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

    lang = selectLanguageDlg(parent, langPath, forced_show_dlg);

    if (save_setting && !lang.empty())
    {
        bool ok = locale.AddCatalog(lang) && locale.IsLoaded(lang);
        if (!ok)  lang.clear(); // bad .mo file
        mmOptions::instance().language_ = lang;
        Model_Setting::instance().Set(LANGUAGE_PARAMETER, lang);
    }

    return lang;
}

void mmShowErrorMessage(wxWindow *parent
    , const wxString &message, const wxString &messageheader)
{
    wxMessageDialog msgDlg(parent, message, messageheader, wxOK|wxICON_ERROR);
    msgDlg.ShowModal();
}

void mmShowErrorMessageInvalid(wxWindow *parent, const wxString &message)
{
    wxString msg = wxString::Format(_("Entry %s is invalid"), message);
    mmShowErrorMessage(parent, msg, _("Invalid Entry"));
}

wxString inQuotes(wxString label, wxString& delimiter)
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
    mmColors::listAlternativeColor0 = Model_Setting::instance().GetColourSetting("LISTALT0", wxColour(225,237,251));
    mmColors::listAlternativeColor1 = Model_Setting::instance().GetColourSetting("LISTALT1", wxColour(255,255,255));
    mmColors::listBackColor         = Model_Setting::instance().GetColourSetting("LISTBACK", wxColour(255,255,255));
    mmColors::navTreeBkColor        = Model_Setting::instance().GetColourSetting("NAVTREE", wxColour(255,255,255));
    mmColors::listBorderColor       = Model_Setting::instance().GetColourSetting("LISTBORDER", wxColour(0,0,0));
    mmColors::listDetailsPanelColor = Model_Setting::instance().GetColourSetting("LISTDETAILSPANEL", wxColour(244,247,251));
    mmColors::listFutureDateColor   = Model_Setting::instance().GetColourSetting("LISTFUTUREDATES", wxColour(116,134,168));
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

wxColour mmColors::userDefColor1 = wxColour(255,0,0);
wxColour mmColors::userDefColor2 = wxColour(255,165,0);
wxColour mmColors::userDefColor3 = wxColour(255,255,0);
wxColour mmColors::userDefColor4 = wxColour(0,255,0);
wxColour mmColors::userDefColor5 = wxColour(0,255,255);
wxColour mmColors::userDefColor6 = wxColour(0,0,255);
wxColour mmColors::userDefColor7 = wxColour(0,0,128);

//*-------------------------------------------------------------------------*//

wxString Tips(wxString type)
{
    return wxGetTranslation(TIPS[rand() % sizeof(TIPS)/sizeof(wxString)]);
}

//*--------------------------------------------------------------------------*//

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

wxImageList* navtree_images_list_()
{
    wxImageList* imageList_ = new wxImageList(16, 16);

    imageList_->Add(wxBitmap(wxImage(house_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(moneyaccount_xpm).Scale(16, 16)));  //TODO: remove
    imageList_->Add(wxBitmap(wxImage(schedule_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(calendar_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(chartpiereport_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(help_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(stock_curve_xpm).Scale(16, 16))); //TODO: remove
    imageList_->Add(wxBitmap(wxImage(car_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(customsql_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(moneyaccount_xpm).Scale(16, 16))); // used for: savings_account
    imageList_->Add(wxBitmap(wxImage(savings_acc_favorite_xpm).Scale(16, 16))); //10
    imageList_->Add(wxBitmap(wxImage(savings_acc_closed_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(termaccount_xpm).Scale(16, 16))); // used for: term_account
    imageList_->Add(wxBitmap(wxImage(term_acc_favorite_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(term_acc_closed_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(stock_acc_xpm).Scale(16, 16))); // used for: invest_account
    imageList_->Add(wxBitmap(wxImage(stock_acc_favorite_xpm).Scale(16, 16))); //TODO: more icons
    imageList_->Add(wxBitmap(wxImage(stock_acc_closed_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(money_dollar_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(money_euro_xpm).Scale(16, 16))); //custom icons
    imageList_->Add(wxBitmap(wxImage(flag_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(accounttree_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(about_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(clock_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(cat_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(dog_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(trees_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(hourglass_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(work_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(yandex_money_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(web_money_xpm).Scale(16, 16)));
    imageList_->Add(wxBitmap(wxImage(rubik_cube_xpm).Scale(16, 16)));

    return imageList_;
}

//* Date Functions----------------------------------------------------------*//

wxString mmGetNiceDateSimpleString(const wxDateTime &dt)
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

wxString mmGetDateForDisplay(const wxDateTime &dt)
{
    return dt.Format(mmOptions::instance().dateFormat_);
}

bool mmParseDisplayStringToDate(wxDateTime& date, const wxString& sDate, wxString &sDateMask)
{
    sDateMask.Replace("%Y%m%d", "%Y %m %d");
    if (date_formats_regex().count(sDateMask) == 0) return false;

    wxString regex = date_formats_regex()[sDateMask];
    wxRegEx pattern(regex);
    //wxLogDebug("%s %s %i %s", sDate, sDateMask, pattern.Matches(sDate), regex);
    if (pattern.Matches(sDate))
    {
        date.ParseFormat(sDate, sDateMask, wxDateTime::Today());
        //wxString one = pattern.GetMatch(sDate, 1);
        return true;
    }
    return false;
}

wxDateTime mmGetStorageStringAsDate(const wxString& str)
{
    wxDateTime dt = wxDateTime::Today();
    if (!str.IsEmpty()) dt.ParseDate(str);
    if (!dt.IsValid()) dt = wxDateTime::Today();
    if (dt.GetYear()<100) dt.Add(wxDateSpan::Years(2000));
    return dt;
}

wxDateTime getUserDefinedFinancialYear(bool prevDayRequired)
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

std::map<wxString,wxString> date_formats_map()
{
    std::map<wxString, wxString> date_formats;
    date_formats["%d/%m/%y"]="DD/MM/YY";
    date_formats["%d/%m/%Y"]="DD/MM/YYYY";
    date_formats["%d-%m-%y"]="DD-MM-YY";
    date_formats["%d-%m-%Y"]="DD-MM-YYYY";
    date_formats["%d.%m.%y"]="DD.MM.YY";
    date_formats["%d.%m.%Y"]="DD.MM.YYYY";
    date_formats["%d,%m,%y"]="DD,MM,YY";
    date_formats["%d/%m'%Y"]="DD/MM'YYYY";
    date_formats["%d/%m %Y"]="DD/MM YYYY";
    date_formats["%m/%d/%y"]="MM/DD/YY";
    date_formats["%m/%d/%Y"]="MM/DD/YYYY";
    date_formats["%m-%d-%y"]="MM-DD-YY";
    date_formats["%m-%d-%Y"]="MM-DD-YYYY";
    date_formats["%m/%d'%y"]="MM/DD'YY";
    date_formats["%m/%d'%Y"]="MM/DD'YYYY";
    date_formats["%y/%m/%d"]="YY/MM/DD";
    date_formats["%y-%m-%d"]="YY-MM-DD";
    date_formats["%Y/%m/%d"]="YYYY/MM/DD";
    date_formats["%Y-%m-%d"]="YYYY-MM-DD";
    date_formats["%Y.%m.%d"]="YYYY.MM.DD";
    date_formats["%Y%m%d"]="YYYYMMDD";

    return date_formats;
}

std::map<wxString,wxString> date_formats_regex()
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
    date_regex["%d.%m.%y"] = wxString::Format("^%s\\x2E%s\\x2E%s$", dd, mm, yy);
    date_regex["%d.%m.%Y"] = wxString::Format("^%s\\x2E%s\\x2E%s$", dd, mm, yyyy);
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
    date_regex["%Y.%m.%d"] = wxString::Format("^%s\\x2E%s\\x2E%s$", yyyy, mm, dd);
    date_regex["%Y %m %d"] = wxString::Format("^%s %s %s$", yyyy, mm, dd);

    return date_regex;
}
