/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013-2014 Nikolay

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
#include "constants.h"
#include "mmtextctrl.h"
#include "validators.h"
#include "model/Model_Currency.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include <wx/sstream.h>
#include <wx/xml/xml.h>
#include <map>
//----------------------------------------------------------------------------

int CaseInsensitiveCmp(const wxString &s1, const wxString &s2)
{
    return s1.CmpNoCase(s2);
}

//----------------------------------------------------------------------------
void correctEmptyFileExt(const wxString& ext, wxString & fileName)
{
    wxFileName tempFileName(fileName);
    if (tempFileName.GetExt().IsEmpty())
        fileName += "." + ext;
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
wxColour mmColors::listAlternativeColor0 = wxColour(240, 245, 235);
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
        const wxString& proxySettings = wxString::Format("%s:%d", proxyName, proxyPort);
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

//Get unread news or all news for last year
const bool getNewsRSS(std::vector<WebsiteNews>& WebsiteNewsList)
{
    wxString RssContent;
    if (site_content(mmex::weblink::NewsRSS, RssContent) != wxURL_NOERR)
        return false;

    wxStringInputStream RssContentStream(RssContent);
    wxXmlDocument RssDocument;
    if (!RssDocument.Load(RssContentStream))
        return false;

    if (RssDocument.GetRoot()->GetName() != "rss")
        return false;

    const wxString news_last_read_date_str = Model_Setting::instance().GetStringSetting(INIDB_NEWS_LAST_READ_DATE, "");
    wxDate news_last_read_date;
    if (!news_last_read_date.ParseISODate(news_last_read_date_str))
        news_last_read_date = wxDateTime::Today().Subtract(wxDateSpan::Year());

    wxXmlNode* RssRoot = RssDocument.GetRoot()->GetChildren()->GetChildren();
    while (RssRoot)
    {
        if (RssRoot->GetName() == "item")
        {
            WebsiteNews website_news;
            wxXmlNode* News = RssRoot->GetChildren();
            while (News)
            {
                wxString ElementName = News->GetName();

                if (ElementName == "title")
                    website_news.Title = News->GetChildren()->GetContent();
                else if (ElementName == "link")
                    website_news.Link = News->GetChildren()->GetContent();
                else if (ElementName == "description")
                    website_news.Description = News->GetChildren()->GetContent();
                else if (ElementName == "pubDate")
                {
                    wxDateTime Date;
                    const wxString DateString = News->GetChildren()->GetContent();
                    if (!DateString.IsEmpty())
                        Date.ParseDate(DateString);
                    if (!Date.IsValid())
                        Date = wxDateTime::Today().Subtract(wxDateSpan::Year()); //Seems invalid date, mark it as 1 year old
                    website_news.Date = Date;
                }
                News = News->GetNext();
            }
            wxLogDebug("%s - %s", news_last_read_date.FormatISODate(), website_news.Date.FormatISODate());
            if (news_last_read_date.IsEarlierThan(website_news.Date))
                WebsiteNewsList.push_back(website_news);
        }
        RssRoot = RssRoot->GetNext();
    }

    if (WebsiteNewsList.size() == 0)
        return false;

    return true;
}

/*--- CSV specific ---------*/
void csv2tab_separated_values(wxString& line, const wxString& delimit)
{
    //csv line example:
    //12.02.2010,Payee,-1105.08,Category,Subcategory,,"Fuel ""95"", 42.31 l (24.20) 212366"
    int i = 0;
    //Single quotes will be used instead double quotes
    //Replace all single quotes first
    line.Replace("'", "\6");
    line.Replace(delimit + "\"\"" + delimit, delimit + delimit);
    if (line.StartsWith("\"\"" + delimit))
        line.Replace("\"\"" + delimit, delimit, false);
    if (line.EndsWith(delimit + "\"\""))
        line.RemoveLast(2);

    //line.Replace(delimit + "\"\"" + "\n", delimit + "\n");
    //Replace double quotes that used twice to replacer
    line.Replace("\"\"\"" + delimit + "\"\"\"", "\5\"" + delimit + "\"\5");
    line.Replace("\"\"\"" + delimit, "\5\"" + delimit);
    line.Replace(delimit + "\"\"\"", delimit + "\"\5");
    line.Replace("\"\"" + delimit, "\5" + delimit);
    line.Replace(delimit + "\"\"", delimit + "\5");

    //replace delimiter to TAB and double quotes to single quotes
    line.Replace("\"" + delimit + "\"", "'\t'");
    line.Replace("\"" + delimit, "'\t");
    line.Replace(delimit + "\"", "\t'");
    line.Replace("\"\"", "\5");
    line.Replace("\"", "'");

    wxString temp_line = wxEmptyString;
    wxString token;
    wxStringTokenizer tkz1(line, "'");

    while (tkz1.HasMoreTokens())
    {
        token = tkz1.GetNextToken();
        if (0 == fmod((double)i, 2))
            token.Replace(delimit, "\t");
        temp_line << token;
        i++;
    };
    //Replace back all replacers to the original value
    temp_line.Replace("\5", "\"");
    temp_line.Replace("\6", "'");
    line = temp_line;
}

//* Date Functions----------------------------------------------------------*//
const wxString mmGetNiceDateSimpleString(const wxDateTime &dt)
{
    wxString dateFmt = Option::instance().DateFormat();
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
    /*
    Since calls to dt.Format() are very expensive, we store previously formatted dates and the resulting formatted
    strings in a lookup table. Next time the same date is queried, the formatted date will be returned automatically.
    If the format is changed, all stored strings are deleted. This provides a considerable performance boost.
    */

    // Remebers previous format. If this changes, all stored string must be deleted.
    static wxString dateFormat = Option::instance().DateFormat();

    // wxDateTime to formatted string lookup table.
    static std::map<wxDateTime, wxString> dateLookup;

    // If format has been changed, delete all stored strings.
    if (dateFormat != Option::instance().DateFormat())
    {
        dateFormat = Option::instance().DateFormat();
        dateLookup.clear();
    }

    // If date exists in lookup- return it.
    auto it = dateLookup.find(dt);
    if (it != dateLookup.end())
        return it->second; // The stored formatted date.

    // Format date, store it and return it.
    return dateLookup[dt] = dt.Format(Option::instance().DateFormat());
}

bool mmParseDisplayStringToDate(wxDateTime& date, wxString sDate, const wxString &sDateMask)
{
    if (date_formats_regex().count(sDateMask) == 0)
        return false;

    const wxString regex = date_formats_regex().at(sDateMask);
    wxRegEx pattern(regex);

    if (pattern.Matches(sDate))
    {
        sDate = pattern.GetMatch(sDate);
        wxString::const_iterator end;
        return date.ParseFormat(sDate, sDateMask, &end);
    }
    return false;
}

const wxDateTime mmGetStorageStringAsDate(const wxString& str)
{
    wxDateTime dt;
    if (str.IsEmpty() || !dt.ParseDate(str))
        dt = wxDateTime::Today();
    int year = dt.GetYear();
    if (year < 50)
        dt.Add(wxDateSpan::Years(2000));
    else if (year < 100)
        dt.Add(wxDateSpan::Years(1900));
    return dt;
}

const wxDateTime getUserDefinedFinancialYear(bool prevDayRequired)
{
    long monthNum;
    Option::instance().FinancialYearStartMonth().ToLong(&monthNum);

    if (monthNum > 0) //Test required for compatability with previous version
        monthNum--;

    int year = wxDate::GetCurrentYear();
    if (wxDate::GetCurrentMonth() < monthNum) year--;

    int dayNum = wxAtoi(Option::instance().FinancialYearStartDay());

    if (dayNum <= 0 || dayNum > wxDateTime::GetNumberOfDays((wxDateTime::Month)monthNum, year))
        dayNum = 1;
    
    wxDateTime financialYear(dayNum, (wxDateTime::Month)monthNum, year);
    if (prevDayRequired)
        financialYear.Subtract(wxDateSpan::Day());
    return financialYear;
}

const std::map<wxString,wxString> &date_formats_regex()
{
    static std::map<wxString, wxString> date_regex;

    // If the map was already filled, return it.
    if (!date_regex.empty())
        return date_regex;

    // First time this function is called, fill the map.
    const wxString dd = "((([0 ][1-9])|([1-2][0-9])|(3[0-1]))|([1-9]))";
    const wxString mm = "((([0 ][1-9])|(1[0-2]))|([1-9]))";
    const wxString yy = "([0-9]{2})";
    const wxString yyyy = "(((19)|([2]([0]{1})))([0-9]{2}))";
    date_regex["%d/%m/%y"] = wxString::Format("^%s/%s/%s*", dd, mm, yy);
    date_regex["%d/%m/%Y"] = wxString::Format("^%s/%s/%s*", dd, mm, yyyy);
    date_regex["%d-%m-%y"] = wxString::Format("^%s-%s-%s*", dd, mm, yy);
    date_regex["%d-%m-%Y"] = wxString::Format("^%s-%s-%s*", dd, mm, yyyy);
    date_regex["%d.%m.%y"] = wxString::Format("^%s\x2E%s\x2E%s*", dd, mm, yy);
    date_regex["%d.%m.%Y"] = wxString::Format("^%s\x2E%s\x2E%s*", dd, mm, yyyy);
    date_regex["%d,%m,%y"] = wxString::Format("^%s,%s,%s*", dd, mm, yy);
    date_regex["%d/%m'%Y"] = wxString::Format("^%s/%s'%s*", dd, mm, yyyy);
    date_regex["%d/%m %Y"] = wxString::Format("^%s/%s %s*", dd, mm, yyyy);
    date_regex["%m/%d/%y"] = wxString::Format("^%s/%s/%s*", mm, dd, yy);
    date_regex["%m/%d/%Y"] = wxString::Format("^%s/%s/%s*", mm, dd, yyyy);
    date_regex["%m-%d-%y"] = wxString::Format("^%s-%s-%s*", mm, dd, yy);
    date_regex["%m-%d-%Y"] = wxString::Format("^%s-%s-%s*", mm, dd, yyyy);
    date_regex["%m/%d'%y"] = wxString::Format("^%s/%s'%s*", mm, dd, yy);
    date_regex["%m/%d'%Y"] = wxString::Format("^%s/%s'%s*", mm, dd, yyyy);
    date_regex["%y/%m/%d"] = wxString::Format("^%s/%s/%s*", yy, mm, dd);
    date_regex["%y-%m-%d"] = wxString::Format("^%s-%s-%s*", yy, mm, dd);
    date_regex["%Y/%m/%d"] = wxString::Format("^%s/%s/%s*", yyyy, mm, dd);
    date_regex["%Y-%m-%d"] = wxString::Format("^%s-%s-%s*", yyyy, mm, dd);
    date_regex["%Y.%m.%d"] = wxString::Format("^%s\x2E%s\x2E%s*", yyyy, mm, dd);
    date_regex["%Y %m %d"] = wxString::Format("^%s %s %s*", yyyy, mm, dd);
    date_regex["%Y%m%d"] = wxString::Format("^%s%s%s*", yyyy, mm, dd);

    return date_regex;
}

const std::map<wxString, wxString> g_date_formats_map = {
    { "%Y-%m-%d", "YYYY-MM-DD" }
    , { "%d/%m/%y", "DD/MM/YY" }
    , { "%d/%m/%Y", "DD/MM/YYYY" }
    , { "%d-%m-%y", "DD-MM-YY" }
    , { "%d-%m-%Y", "DD-MM-YYYY" }
    , { "%d.%m.%y", "DD.MM.YY" }
    , { "%d.%m.%Y", "DD.MM.YYYY" }
    , { "%d,%m,%y", "DD,MM,YY" }
    , { "%d/%m'%Y", "DD/MM'YYYY" }
    , { "%d/%m %Y", "DD/MM YYYY" }
    , { "%m/%d/%y", "MM/DD/YY" }
    , { "%m/%d/%Y", "MM/DD/YYYY" }
    , { "%m-%d-%y", "MM-DD-YY" }
    , { "%m-%d-%Y", "MM-DD-YYYY" }
    , { "%m/%d'%y", "MM/DD'YY" }
    , { "%m/%d'%Y", "MM/DD'YYYY" }
    , { "%y/%m/%d", "YY/MM/DD" }
    , { "%y-%m-%d", "YY-MM-DD" }
    , { "%Y/%m/%d", "YYYY/MM/DD" }
    , { "%Y.%m.%d", "YYYY.MM.DD" }
    , { "%Y %m %d", "YYYY MM DD" }
    , { "%Y%m%d", "YYYYMMDD" }
};

const std::map<int, std::pair<wxConvAuto, wxString> > g_encoding = {
    { 0, { wxConvAuto(wxFONTENCODING_SYSTEM), wxTRANSLATE("Default") } }
    , { 1, { wxConvAuto(wxFONTENCODING_UTF8), wxTRANSLATE("UTF-8") } }
    , { 2, { wxConvAuto(wxFONTENCODING_CP1250), wxTRANSLATE("1250") } }
    , { 3, { wxConvAuto(wxFONTENCODING_CP1251), wxTRANSLATE("1251") } }
    , { 4, { wxConvAuto(wxFONTENCODING_CP1252), wxTRANSLATE("1252") } }
    , { 5, { wxConvAuto(wxFONTENCODING_CP1253), wxTRANSLATE("1253") } }
    , { 6, { wxConvAuto(wxFONTENCODING_CP1254), wxTRANSLATE("1254") } }
    , { 7, { wxConvAuto(wxFONTENCODING_CP1255), wxTRANSLATE("1255") } }
    , { 8, { wxConvAuto(wxFONTENCODING_CP1256), wxTRANSLATE("1256") } }
    , { 9, { wxConvAuto(wxFONTENCODING_CP1257), wxTRANSLATE("1257") } }
};

static const wxString MONTHS[12] =
{
    wxTRANSLATE("January"), wxTRANSLATE("February"), wxTRANSLATE("March")
    , wxTRANSLATE("April"), wxTRANSLATE("May"), wxTRANSLATE("June")
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
    for (const auto& c : " 1234567890.,(/+-*)")
    {
        list.Add(c);
    }
    SetIncludes(list);
}

void mmCalcValidator::OnChar(wxKeyEvent& event)
{
    if (!m_validatorWindow)
        return event.Skip();

    int keyCode = event.GetKeyCode();

    // we don't filter special keys and delete
    if (keyCode < WXK_SPACE || keyCode == WXK_DELETE || keyCode >= WXK_START)
        return event.Skip();


    wxString str((wxUniChar)keyCode, 1);
    if (!(wxIsdigit(str[0]) || wxString("+-.,*/ ()").Contains(str)))
    {
        if ( !wxValidator::IsSilent() )
            wxBell();

        return; // eat message
    }
    // only if it's a wxTextCtrl
    mmTextCtrl* text_field = wxDynamicCast(m_validatorWindow, mmTextCtrl);
    if (!m_validatorWindow || !text_field)
        return event.Skip();

    wxChar decChar = text_field->currency_->DECIMAL_POINT[0];
    bool numpad_dec_swap = (wxGetKeyState(wxKeyCode(WXK_NUMPAD_DECIMAL)) && decChar != str);
    
    if (numpad_dec_swap)
        str = wxString(decChar);

    // if decimal point, check if it's already in the string
    if (str == '.' || str == ',')
    {
        const wxString value = text_field->GetValue();
        size_t ind = value.rfind(decChar);
        if (ind < value.Length())
        {
            // check if after last decimal point there is an operation char (+-/*)
            if (value.find('+', ind + 1) >= value.Length() && value.find('-', ind + 1) >= value.Length() &&
                value.find('*', ind + 1) >= value.Length() && value.find('/', ind + 1) >= value.Length())
                return;
        }
    }

    if (numpad_dec_swap)
        return text_field->WriteText(str);
    else
        event.Skip();

}

