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
#include "constants.h"
#include "validators.h"
#include "model/Model_Currency.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include <wx/sstream.h>
#include <wx/xml/xml.h>
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
    //skip dot if present in pattern but not in date string 
    const wxString separator = mask.Mid(2,1);
    date.ParseFormat(sDate, mask, date);
    if (pattern.Matches(sDate) && sDate.Contains(separator))
        return true;
    else
    {
        //wxLogDebug("%s %s %i %s", sDate, mask, pattern.Matches(sDate), regex);
        return false;
    }
}

const wxDateTime mmGetStorageStringAsDate(const wxString& str)
{
    wxDateTime dt = wxDateTime::Today();
    if (!str.IsEmpty()) dt.ParseDate(str);
    if (!dt.IsValid()) dt = wxDateTime::Today();
    if (dt.GetYear() < 100) dt.Add(wxDateSpan::Years(2000));
    return dt;
}

const wxDateTime getUserDefinedFinancialYear(bool prevDayRequired)
{
    long monthNum;
    mmOptions::instance().financialYearStartMonthString_.ToLong(&monthNum);

    if (monthNum > 0) //Test required for compatability with previous version
        monthNum--;

    const wxDateTime today = wxDateTime::Today();
    int year = today.GetYear();
    if (today.GetMonth() < monthNum) year--;

    int dayNum = wxAtoi(mmOptions::instance().financialYearStartDayString_);
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
    , { "%Y%m%d", "YYYYMMDD" }
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
    for (const auto& c : " 1234567890.,(/+-*)")
    {
        list.Add(c);
    }
    SetIncludes(list);
    const Model_Currency::Data *base_currency = Model_Currency::instance().GetBaseCurrency();
    if (base_currency)
        m_decChar = base_currency->DECIMAL_POINT[0];
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

    const wxString str((wxUniChar)keyCode, 1);
    if (!(wxIsdigit(str[0]) || wxString("+-.,*/ ()").Contains(str)))
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
        const wxString value = ((wxTextCtrl*)m_validatorWindow)->GetValue();
        size_t ind = value.rfind(m_decChar);
        if (ind < value.Length())
        {
            // check if after last decimal point there is an operation char (+-/*)
            if (value.find('+', ind+1) >= value.Length() && value.find('-', ind+1) >= value.Length() &&
                value.find('*', ind+1) >= value.Length() && value.find('/', ind+1) >= value.Length())
                return;
        }
        if (str != m_decChar)
        {
#ifdef _MSC_VER
            const wxChar vk = m_decChar == '.' ? 0x6e : 0xbc;
            keybd_event(vk, 0xb3, 0, 0);
            keybd_event(vk, 0xb3, KEYEVENTF_KEYUP, 0);
            return;
#endif
        }
    }
    event.Skip();
}
#endif
