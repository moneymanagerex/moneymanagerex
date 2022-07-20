/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013-2022 Nikolay Akimov
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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

#ifdef _MSC_VER
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"crypt32.lib")
#pragma comment(lib,"wldap32.lib")
#endif

#include "build.h"
#include "util.h"
#include "constants.h"
#include "option.h"
#include "platfdep.h"
#include "paths.h"
#include "validators.h"
#include "model/Model_Currency.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include "model/Model_CurrencyHistory.h"
#include <wx/display.h>
#include <wx/sstream.h>
#include <wx/xml/xml.h>
#include <map>
#include <lua.hpp>
#include <wx/fs_mem.h>
#include <fmt/core.h>
#include <cwchar>

using namespace rapidjson;

wxString JSON_PrettyFormated(rapidjson::Document& j_doc)
{
    StringBuffer j_buffer;
    PrettyWriter<StringBuffer> j_writer(j_buffer);
    j_doc.Accept(j_writer);

    return wxString::FromUTF8(j_buffer.GetString());
}

wxString JSON_Formated(rapidjson::Document& j_doc)
{
    StringBuffer j_buffer;
    Writer<StringBuffer> j_writer(j_buffer);
    j_doc.Accept(j_writer);

    return wxString::FromUTF8(j_buffer.GetString());
}

//----------------------------------------------------------------------------

mmTreeItemData::mmTreeItemData(int type, int id)
    : id_(id)
    , type_(type)
    , report_(nullptr)
{
    stringData_ = (wxString::Format("%i", id));
}
mmTreeItemData::mmTreeItemData(const wxString& data, mmPrintableBase* report)
    : id_(-1)
    , type_(mmTreeItemData::REPORT)
    , stringData_(data)
    , report_(report)
{
    const wxString& n = wxString::Format("REPORT_%d", report_->getReportId());
    const wxString& settings = Model_Infotable::instance().GetStringInfo(n, "");
    report_->initReportSettings(settings);
}
mmTreeItemData::mmTreeItemData(mmPrintableBase* report, const wxString& data)
    : id_(-1)
    , type_(mmTreeItemData::GRM)
    , stringData_(data)
    , report_(report)
{}
mmTreeItemData::mmTreeItemData(int type, const wxString& data)
    : id_(-1)
    , type_(type)
    , stringData_(data)
    , report_(nullptr)
{}

//----------------------------------------------------------------------------

int CaseInsensitiveCmp(const wxString &s1, const wxString &s2)
{
    return s1.CmpNoCase(s2);
}

int CaseInsensitiveLocaleCmp(const wxString &s1, const wxString &s2)
{
    return std::wcscoll(s1.Lower().wc_str(),s2.Lower().wc_str());
}

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
        label.Replace("\"", "\"\"", true);
        label = wxString() << "\"" << label << "\"";
    }

    label.Replace("\t", "    ", true);
    label.Replace("\n", " ", true);
    return label;
}

void mmLoadColorsFromDatabase(bool def)
{
    mmColors::userDefColor1 = def ? wxColour(246, 144, 144) : Model_Infotable::instance().GetColourSetting("USER_COLOR1", wxColour(246, 144, 144));
    mmColors::userDefColor2 = def ? wxColour(229, 196, 146) : Model_Infotable::instance().GetColourSetting("USER_COLOR2", wxColour(229, 196, 146));
    mmColors::userDefColor3 = def ? wxColour(245, 237, 149) : Model_Infotable::instance().GetColourSetting("USER_COLOR3", wxColour(245, 237, 149));
    mmColors::userDefColor4 = def ? wxColour(186, 226, 185) : Model_Infotable::instance().GetColourSetting("USER_COLOR4", wxColour(186, 226, 185));
    mmColors::userDefColor5 = def ? wxColour(135, 190, 219) : Model_Infotable::instance().GetColourSetting("USER_COLOR5", wxColour(135, 190, 219));
    mmColors::userDefColor6 = def ? wxColour(172, 167, 239) : Model_Infotable::instance().GetColourSetting("USER_COLOR6", wxColour(172, 167, 239));
    mmColors::userDefColor7 = def ? wxColour(212, 138, 215) : Model_Infotable::instance().GetColourSetting("USER_COLOR7", wxColour(212, 138, 215));
}

wxColour mmColors::userDefColor1;
wxColour mmColors::userDefColor2;
wxColour mmColors::userDefColor3;
wxColour mmColors::userDefColor4;
wxColour mmColors::userDefColor5;
wxColour mmColors::userDefColor6;
wxColour mmColors::userDefColor7;

wxColour getUDColour(int c)
{
    switch (c)
    {
    case 1: return  mmColors::userDefColor1;
    case 2: return  mmColors::userDefColor2;
    case 3: return  mmColors::userDefColor3;
    case 4: return  mmColors::userDefColor4;
    case 5: return  mmColors::userDefColor5;
    case 6: return  mmColors::userDefColor6;
    case 7: return  mmColors::userDefColor7;
    }
    return wxNullColour;
}

//*-------------------------------------------------------------------------*//

//Get unread news or all news for last year
bool getNewsRSS(std::vector<WebsiteNews>& WebsiteNewsList)
{
    wxString RssContent;
    if (http_get_data(mmex::weblink::NewsRSS, RssContent) != CURLE_OK)
        return false;

    //simple validation to avoid bug #1083
    if (!RssContent.Contains("</rss>"))
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

    wxLogDebug("getNewsRSS: New articles = %i", static_cast<int>(WebsiteNewsList.size()));
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
        if (0 == fmod(static_cast<double>(i), 2))
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

static const wxString MONTHS_SHORT[12] =
{
    wxTRANSLATE("Jan"), wxTRANSLATE("Feb"), wxTRANSLATE("Mar")
    , wxTRANSLATE("Apr"), wxTRANSLATE("May"), wxTRANSLATE("Jun")
    , wxTRANSLATE("Jul"), wxTRANSLATE("Aug"), wxTRANSLATE("Sep")
    , wxTRANSLATE("Oct"), wxTRANSLATE("Nov"), wxTRANSLATE("Dec")
};

static const wxString g_days_of_week[7] =
{
    wxTRANSLATE("Sunday"), wxTRANSLATE("Monday"), wxTRANSLATE("Tuesday")
    , wxTRANSLATE("Wednesday"), wxTRANSLATE("Thursday"), wxTRANSLATE("Friday")
    , wxTRANSLATE("Saturday")
};

static const wxString g_short_days_of_week[7] =
{
    wxTRANSLATE("Sun"), wxTRANSLATE("Mon"), wxTRANSLATE("Tue")
    , wxTRANSLATE("Wed"), wxTRANSLATE("Thu"), wxTRANSLATE("Fri")
    , wxTRANSLATE("Sat")
};


const wxString mmGetDateForDisplay(const wxString &iso_date, const wxString& dateFormat)
{
    //ISO Date to formatted string lookup table.
    static std::unordered_map<wxString, wxString> dateLookup;
    static wxString date_format;
    if (date_format.empty())
        date_format = Option::instance().getDateFormat();

    // If format has been changed, delete all stored strings.
    if (dateFormat != date_format)
    {
        dateLookup.clear();
        if (dateFormat.empty()) {
            return "";
        }
        date_format = dateFormat;
    }

    // If date exists in lookup - return it.
    auto it = dateLookup.find(iso_date);
    if (it != dateLookup.end())
        return it->second; // The stored formatted date.

    wxRegEx pattern(R"([0-9]{4}\-[0-9]{2}\-[0-9]{2})");
    if (!pattern.Matches(iso_date)) {
        return "";
    }

    // Format date, store it and return it.
    wxString date_str = dateFormat;
    if (date_str.Replace("%Y", iso_date.Mid(0, 4)) == 0)
        date_str.Replace("%y", iso_date.Mid(2, 2));

    if (date_str.Contains("%Mon")) {
        const auto mon = wxGetTranslation(MONTHS_SHORT[wxAtoi(iso_date.Mid(5, 2)) - 1]);
        date_str.Replace("%Mon", wxGetTranslation(mon));
    }

    date_str.Replace("%m", iso_date.Mid(5, 2));
    date_str.Replace("%d", iso_date.Mid(8, 2));
    if (date_str.Contains("%w")) {
        wxDateTime d;
        d.ParseISODate(iso_date);
        const auto week = wxGetTranslation(g_short_days_of_week[d.GetWeekDay()]);
        date_str.Replace("%w", wxGetTranslation(week));
    }

    return dateLookup[iso_date] = date_str;
}

bool mmParseDisplayStringToDate(wxDateTime& date, const wxString& str_date, const wxString &sDateMask)
{
    if (date_formats_regex().count(sDateMask) == 0)
        return false;

    wxString date_str = str_date;
    wxString mask_str = sDateMask;

    static std::unordered_map<wxString, wxDate> cache;
    const auto it = cache.find(str_date);
    if (it != cache.end())
    {
        date = it->second;
        return true;
    }

    wxString regex = date_formats_regex().at(mask_str);
    wxRegEx pattern(regex);

    if (pattern.Matches(str_date))
    {
        if (mask_str.Contains("%w")) {
            mask_str.Replace("%w ", "");
            regex = R"(^(\D*))";
            pattern.Compile(regex);
            pattern.ReplaceAll(&date_str, "");
        }

        if (mask_str.Contains("Mon")) {

            static std::map<wxString, wxString> monCache;
            if (monCache.empty())
            {
                int i = 1;
                for (const auto& m : MONTHS_SHORT) {
                    monCache[m] = wxString::Format("%02d", i);
                    monCache[wxGetTranslation(m)] = wxString::Format("%02d", i);
                    i++;
                }
            }

            regex = R"([^\d\s\'\-]{3})";
            pattern.Compile(regex);
            wxString month;
            if (pattern.Matches(date_str)) {
                month = pattern.GetMatch(date_str);
            }

            bool is_month_ok = false;
            for (const auto& i : monCache)
            {
                if (month.CmpNoCase(i.first) == 0) {
                    date_str.Replace(month, i.second);
                    mask_str.Replace("%Mon", "%m");
                    is_month_ok = true;
                    break;
                }
            }

            if (!is_month_ok)
                return false;

            wxRegEx pattern2(R"([^%dmyY])");
            pattern2.ReplaceAll(&mask_str, " ");
            if (date_formats_regex().find(mask_str) != date_formats_regex().end())
                regex = date_formats_regex().at(mask_str);
            else
                return false;

            wxRegEx pattern3(R"([^0-9])");
            pattern3.ReplaceAll(&date_str, " ");
        }

        pattern.Compile(regex);
        if (pattern.Matches(date_str))
        {
            date_str = pattern.GetMatch(date_str);
            date_str.Trim(false);
            const auto& date_formats = g_date_formats_map();
            const auto it2 = std::find_if(date_formats.begin(), date_formats.end(),
                [&mask_str](const std::pair<wxString, wxString>& element) { return element.first == mask_str; });
            if (!it2->second.Contains(" ")) {
                date_str.Replace(" ", "");
            }

            wxString::const_iterator end;
            bool t = date.ParseFormat(date_str, mask_str, &end);
            wxLogDebug("String:%s Mask:%s OK:%s ISO:%s Pattern:%s", str_date, sDateMask, wxString(t ? "true" : "false"), date.FormatISODate(), regex);
            return t;
        }
    }
    return false;
}

bool mmParseISODate(const wxString& in, wxDateTime& out)
{
    if (in.IsEmpty() || !out.ParseDate(in)) {
        out = wxDateTime::Today();
        return false;
    }
    int year = out.GetYear();
    if (year < 50)
        out.Add(wxDateSpan::Years(2000));
    else if (year < 100)
        out.Add(wxDateSpan::Years(1900));
    return true;
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

    if (dayNum <= 0 || dayNum > wxDateTime::GetNumberOfDays(static_cast<wxDateTime::Month>(monthNum), year))
        dayNum = 1;

    wxDateTime financialYear(dayNum, static_cast<wxDateTime::Month>(monthNum), year);
    if (prevDayRequired)
        financialYear.Subtract(wxDateSpan::Day());
    return financialYear;
}

const std::map<wxString, wxString> &date_formats_regex()
{
    static std::map<wxString, wxString> date_regex;

    // If the map was already filled, return it.
    if (!date_regex.empty())
        return date_regex;

    // First time this function is called, fill the map.
    const wxString week = "([^0-9]{3})";
    const wxString mon = "([^0-9]{3})";
    const wxString dd = "((([0 ][1-9])|([1-2][0-9])|(3[0-1]))|([1-9]))";
    const wxString mm = "((([0 ][1-9])|(1[0-2]))|([1-9]))";
    const wxString yy = "(([ ][0-9])|([0-9]{1,2}))";
    const wxString yyyy = "(((19)|([2]([0]{1})))([0-9]{2}))";
    const wxString tail = "($|[^0-9])+";
    const wxString head = "^";

    for (const auto entry : g_date_formats_map())
    {
        wxString regexp = entry.first;
        regexp.Replace(".", R"([.])");
        regexp.Replace("/", R"(\/)");
        regexp.Replace(" ", R"(\s)");
        regexp.Replace("%Mon", mon);
        regexp.Replace("%w", week);
        regexp.Replace("%d", dd);
        regexp.Replace("%m", mm);
        regexp.Replace("%Y", yyyy);
        regexp.Replace("%y", yy);
        regexp.Append(tail);
        regexp.Prepend(head);
        date_regex[entry.first] = regexp;
    }

    return date_regex;
}

bool comp(std::pair<wxString, wxString> a, std::pair<wxString, wxString> b) {

    wxString one = a.second;
    wxRegEx pattern(R"([^DayMonY])");
    pattern.ReplaceAll(&one, wxEmptyString);
    one += a.second;

    wxString two = b.second;
    pattern.ReplaceAll(&two, wxEmptyString);
    two += b.second;

    return one < two;
}

const std::vector<std::pair<wxString, wxString> > g_date_formats_map()
{
    static std::vector<std::pair<wxString, wxString>> df;
    if (!df.empty())
        return df;

    std::vector<wxString> formats = {
        "%d %Mon %Y",
        "%d %Mon %y",
        "%d-%Mon-%Y",
        "%d %Mon'%y",
        "%d %m %y",
        "%d %m %Y",
        "%d,%m,%y",
        "%d.%m.%y",
        "%d.%m.%Y",
        "%d,%m,%Y",
        "%d/%m %Y",
        "%d/%m/%y",
        "%d/%m/%Y",
        "%d/%m'%y",
        "%d/%m'%Y",
        "%d-%m-%y",
        "%d-%m-%Y",
        "%w %d %Mon'%y",
        "%m.%d.%y",
        "%m.%d.%Y",
        "%m/%d/%y",
        "%m/%d/%Y",
        "%m/%d'%y",
        "%m/%d'%Y",
        "%m-%d-%y",
        "%m-%d-%Y",
        "%y/%m/%d",
        "%y-%m-%d",
        "%Y %m %d",
        "%Y.%m.%d",
        "%Y/%m/%d",
        "%Y%d%m",
        "%Y%m%d",
        "%Y-%m-%d"
    };

    const auto local_date_fmt = wxLocale::GetInfo(wxLOCALE_SHORT_DATE_FMT);
    if (std::find(formats.begin(), formats.end(), local_date_fmt) == formats.end())
        formats.push_back(local_date_fmt);

    for (const auto& entry : formats)
    {
        auto local_date_mask = entry;
        local_date_mask.Replace("%Y", "YYYY");
        local_date_mask.Replace("%y", "YY");
        local_date_mask.Replace("%d", "DD");
        local_date_mask.Replace("%Mon", "Mon");
        local_date_mask.Replace("%m", "MM");
        local_date_mask.Replace("%w", "Day");
        df.push_back(std::make_pair(entry, local_date_mask));
    }

    std::sort(df.begin(), df.end(), comp);
    return df;
}

const std::map<int, std::pair<wxConvAuto, wxString> > g_encoding = {
    { 0, { wxConvAuto(wxFONTENCODING_SYSTEM), wxTRANSLATE("Default") } }
    , { 1, { wxConvAuto(wxFONTENCODING_UTF8), "UTF-8" } }
    , { 2, { wxConvAuto(wxFONTENCODING_CP1250), "1250" } }
    , { 3, { wxConvAuto(wxFONTENCODING_CP1251), "1251" } }
    , { 4, { wxConvAuto(wxFONTENCODING_CP1252), "1252" } }
    , { 5, { wxConvAuto(wxFONTENCODING_CP1253), "1253" } }
    , { 6, { wxConvAuto(wxFONTENCODING_CP1254), "1254" } }
    , { 7, { wxConvAuto(wxFONTENCODING_CP1255), "1255" } }
    , { 8, { wxConvAuto(wxFONTENCODING_CP1256), "1256" } }
    , { 9, { wxConvAuto(wxFONTENCODING_CP1257), "1257" } }
};

//
const wxString mmPlatformType()
{
    return wxPlatformInfo::Get().GetOperatingSystemFamilyName().substr(0, 3);
}

void DoWindowsFreezeThaw(wxWindow* w)
{
#ifdef __WXGTK__
    return;
#endif

    if (w->IsFrozen())
        w->Thaw();
    else
        w->Freeze();
}

//--------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(mmCalcValidator, wxTextValidator)
BEGIN_EVENT_TABLE(mmCalcValidator, wxTextValidator)
EVT_CHAR(mmCalcValidator::OnChar)
END_EVENT_TABLE()

//--------------------------------------------------------------------

bool getOnlineCurrencyRates(wxString& msg, int curr_id, bool used_only)
{
    wxString base_currency_symbol;

    if (!Model_Currency::GetBaseCurrencySymbol(base_currency_symbol))
    {
        msg = _("Could not find base currency symbol!");
        return false;
    }

    std::map<wxString, double> fiat;
    const wxDateTime today = wxDateTime::Today();
    const wxString today_str = today.FormatISODate();

    auto currencies = Model_Currency::instance().find(Model_Currency::CURRENCY_SYMBOL(base_currency_symbol, NOT_EQUAL));
    for (const auto& currency : currencies)
    {
        if (curr_id > 0 && currency.CURRENCYID != curr_id)
            continue;
        if (curr_id < 0 && !Model_Account::is_used(currency))
            continue;
        const auto symbol = currency.CURRENCY_SYMBOL;
        if (symbol.IsEmpty())
            continue;

        fiat[symbol] = Model_CurrencyHistory::getDayRate(currency.CURRENCYID, today_str);
    }

    if (fiat.empty())
    {
        msg = _("Nothing to update");
        return false;
    }

    wxString output;
    std::map<wxString, double> currency_data;

    get_yahoo_prices(fiat, currency_data, base_currency_symbol, output, yahoo_price_type::FIAT);

    // fallback to coincap if some currencies were not found
    double usd_conv_rate = -1;
    for (const auto & item : fiat)
    {
        if (currency_data.find(item.first) == currency_data.end() && !g_fiat_curr().Contains(item.first))
        {
            wxString coincap_id;
            wxString coincap_msg;
            double coincap_price_usd;
            if (getCoincapInfoFromSymbol(item.first, coincap_id, coincap_price_usd, coincap_msg) && coincap_price_usd > 0) {
                if (usd_conv_rate < 0) {
                    auto usd = Model_Currency::instance().GetCurrencyRecord("USD");
                    if (usd == nullptr) {
                        break; // can't use coincap without USD, since all prices are in USD so give up
                    }
                    usd_conv_rate = usd->BASECONVRATE;
                }

                currency_data[item.first] = coincap_price_usd * usd_conv_rate;
            }
        }
    }

    const auto b = Model_Currency::GetBaseCurrency();
    msg << _("Currency rates have been updated");
    msg << "\n\n";
    for (const auto & item : fiat)
    {
        const wxString value0_str(fmt::format("{:>{}}", Model_Currency::toString(item.second, b, 4).mb_str(), 20));
        const wxString symbol(fmt::format("{:<{}}", item.first.mb_str(), 10));

        if (currency_data.find(item.first) != currency_data.end())
        {
            auto value1 = currency_data[item.first];
            const wxString value1_str(fmt::format("{:>{}}", Model_Currency::toString(value1, b, 4).mb_str(), 20));
            msg << wxString::Format("%s\t%s\t\t%s\n", symbol, value0_str, value1_str);
        }
        else
        {
            msg << wxString::Format("%s\t%s\t\t%s\n", symbol, value0_str, _("Invalid value"));
        }
    }


    Model_Currency::instance().Savepoint();
    Model_CurrencyHistory::instance().Savepoint();
    for (auto& currency : currencies)
    {
        if (!used_only && !Model_Account::is_used(currency)) continue;

        const wxString currency_symbol = currency.CURRENCY_SYMBOL;
        if (!currency_symbol.IsEmpty() && currency_data.find(currency_symbol) != currency_data.end())
        {
            double new_rate = currency_data[currency_symbol];
            if (new_rate > 0)
            {
                if(Option::instance().getCurrencyHistoryEnabled())
                    Model_CurrencyHistory::instance().addUpdate(currency.CURRENCYID, today, new_rate, Model_CurrencyHistory::ONLINE);
                else
                {
                    currency.BASECONVRATE = new_rate;
                    Model_Currency::instance().save(&currency);
                }
            }
        }
    }
    Model_Currency::instance().ReleaseSavepoint();
    Model_CurrencyHistory::instance().ReleaseSavepoint();

    return true;
}

/* Currencies & stock prices */

bool get_yahoo_prices(std::map<wxString, double>& symbols
    , std::map<wxString, double>& out
    , const wxString& base_currency_symbol
    , wxString& output
    , int type)
{
    double conversion_factor = 1.0;
    wxString buffer;

    wxString base_curr_symbol = base_currency_symbol;
    if (type == yahoo_price_type::FIAT && !wxString("USD|EUR|GBP|CHF").Contains(base_currency_symbol))
    {
        base_curr_symbol = "USD";
        buffer += wxString::Format("%s%s=X,", base_currency_symbol, base_curr_symbol);
    }

    for (const auto& entry : symbols)
    {
        wxRegEx pattern(R"(^([-a-zA-Z0-9_@=\.]+)$)");
        if (!pattern.Matches(entry.first))
            continue;

        if (type == yahoo_price_type::FIAT) {
            if (g_fiat_curr().Contains(entry.first))
                buffer += wxString::Format("%s%s=X,", entry.first, base_curr_symbol);
            else
                buffer += wxString::Format("%s-%s,", entry.first, base_curr_symbol);
        }
        else {
            buffer += entry.first + ",";
        }
    }

    if (buffer.Right(1).Contains(",")) {
        buffer.RemoveLast(1);
    }

    const auto URL = wxString::Format(mmex::weblink::YahooQuotes, buffer);

    wxString json_data;
    auto err_code = http_get_data(URL, json_data);
    if (err_code != CURLE_OK)
    {
        output = json_data;
        return false;
    }

    Document json_doc;
    if (json_doc.Parse(json_data.utf8_str()).HasParseError()) {
        output = _("JSON Parse Error");
        return false;
    }

    /*
    {"finance":{"result":null,"error":{"code":"Bad Request","description":"Missing required query parameter=symbols"}}}
    */

    if (json_doc.HasMember("finance") && json_doc["finance"].IsObject()) {
        Value e = json_doc["finance"].GetObject();
        if (e.HasMember("error") && e["error"].IsObject()) {
            Value err = e["error"].GetObject();
            if (err.HasMember("description") && err["description"].IsString()) {
                output = wxString::FromUTF8(err["description"].GetString());
                return false;
            }
        }
    }


    Value r = json_doc["quoteResponse"].GetObject();
    Value e = r["result"].GetArray();

    if (e.Empty()) {
        output = _("Nothing to update");
        return false;
    }

    if (type == yahoo_price_type::FIAT)
    {
        for (rapidjson::SizeType i = 0; i < e.Size(); i++)
        {
            if (!e[i].IsObject()) continue;
            Value v = e[i].GetObject();

            if (!v.HasMember("symbol") || !v["symbol"].IsString())
                continue;
            auto currency_symbol = wxString::FromUTF8(v["symbol"].GetString());

            double price = 0.0;
            wxRegEx pattern("^([A-Z]{3})[A-Z]{3}=X$");
            if (pattern.Matches(currency_symbol))
            {
                if (!v.HasMember("regularMarketPrice") || !v["regularMarketPrice"].IsFloat())
                    continue;
                price = v["regularMarketPrice"].GetFloat();
                currency_symbol = pattern.GetMatch(currency_symbol, 1);
            }
            wxRegEx crypto_pattern("^([A-Z]{3,})-[A-Z]{3}$");
            if (crypto_pattern.Matches(currency_symbol))
            {
                if (!v.HasMember("regularMarketPrice") || !v["regularMarketPrice"].IsFloat())
                    continue;
                price = v["regularMarketPrice"].GetFloat();
                currency_symbol = crypto_pattern.GetMatch(currency_symbol, 1);
            }

            if (currency_symbol == base_currency_symbol)
                conversion_factor = price;
            else
                out[currency_symbol] = (price <= 0.0 ? 0.0 : price);

        }
    }
    else
    {
        for (rapidjson::SizeType i = 0; i < e.Size(); i++)
        {
            if (!e[i].IsObject()) continue;
            Value v = e[i].GetObject();

            if (!v.HasMember("regularMarketPrice") || !v["regularMarketPrice"].IsFloat())
                continue;
            auto price = v["regularMarketPrice"].GetFloat();

            if (!v.HasMember("symbol") || !v["symbol"].IsString())
                continue;
            const auto symbol = wxString::FromUTF8(v["symbol"].GetString());

            if (!v.HasMember("currency") || !v["currency"].IsString())
                continue;
            const auto currency = wxString::FromUTF8(v["currency"].GetString());
            double k = currency == "GBp" ? 100 : 1;

            wxLogDebug("item: %u %s %f", i, symbol, price);
            out[symbol] = price <= 0 ? 0 : price / k;
        }
    }

    for (auto& item : out)
    {
        item.second /= conversion_factor;
    }

    return true;
}



// coincap.io operates on ascii IDs for currencies, not their symbol
// this method searches coincap using a symbol and gets the ID of the first
// currency found with that symbol.
// this method also tries to get the price in USD, or -1.0 if not found
bool getCoincapInfoFromSymbol(const wxString symbol, wxString& out_id, double& price_usd, wxString& output) {
    wxString url = wxString::Format(mmex::weblink::CoinCapSearch, symbol);

    wxString json_data;
    auto err_code = http_get_data(url, json_data);
    if (err_code != CURLE_OK)
    {
        output = json_data;
        return false;
    }

    Document json_doc;
    if (json_doc.Parse(json_data.utf8_str()).HasParseError()) {
        output = _("JSON Parse Error");
        return false;
    }
    
    if (!json_doc.HasMember("data") || !json_doc["data"].IsArray()) {
        if (json_doc.HasMember("error") && json_doc["error"].IsString()) {
            output = wxString::Format("Error from coincap API: %s", json_doc["error"].GetString());
        } else {
            output = _("Expected response to contain a data or error string");
        }
        
        return false;
    }

    Value assets = json_doc["data"].GetArray();
    for (rapidjson::SizeType i = 0; i < assets.Size(); ++i) {
        if (!assets[i].IsObject()) continue;
        Value asset = assets[i].GetObject();

        if (asset.HasMember("symbol") && asset["symbol"].IsString() && asset.HasMember("id") && asset["id"].IsString()) {
            const char* asset_symbol = asset["symbol"].GetString();
            rapidjson::SizeType asset_symbol_len = asset["symbol"].GetStringLength();

            // verify that the symbol matches the target symbol
            // coincap search searches more than just symbols so the first results may not be the correct currency
            if (symbol.compare(0, asset_symbol_len, asset_symbol) == 0) {
                out_id = wxString::FromAscii(asset["id"].GetString(), asset["id"].GetStringLength());

                price_usd = -1;
                if (asset.HasMember("priceUsd") && asset["priceUsd"].IsString()) {
                    // price is stored as a string, parse it to a double
                    wxString(asset["priceUsd"].GetString(), asset["priceUsd"].GetStringLength()).ToCDouble(&price_usd);
                }

                return true;
            }
        }
    }

    output = _("Could not find asset for symbol");
    return false;
}

bool getCoincapAssetHistory(const wxString asset_id, wxDateTime begin_date, std::map<wxDateTime, double> &historical_rates, wxString &msg) {
    // coincap uses unix time milliseconds
    long long begin_date_unix = static_cast<long long>(begin_date.GetTicks()) * 1000;
    long long end_date_unix = static_cast<long long>(wxDateTime::Today().GetTicks()) * 1000;
    wxString url = wxString::Format(mmex::weblink::CoinCapHistory, asset_id, "d1", begin_date_unix, end_date_unix);

    wxString json_data;
    auto err_code = http_get_data(url, json_data);
    if (err_code != CURLE_OK)
    {
        msg = json_data;
        return false;
    }

    Document json_doc;
    if (json_doc.Parse(json_data.utf8_str()).HasParseError()) {
        msg = _("JSON Parse Error");
        return false;
    }

    if (!json_doc.HasMember("data") || !json_doc["data"].IsArray()) {
        if (json_doc.HasMember("error") && json_doc["error"].IsString()) {
            msg = wxString::Format("Error from coincap API: %s", json_doc["error"].GetString());
        } else {
            msg = _("Expected response to contain a data or error string");
        }
        return false;
    }

    wxString baseCurrencySymbol;
    if (!Model_Currency::GetBaseCurrencySymbol(baseCurrencySymbol)) {
        msg = _("Could not get base currency!");
        return false;
    }

    // prices in USD are multiplied by this value to convert them to the base currency
    double multiplier = 1.0;
    if (baseCurrencySymbol != _("USD")) {
        auto usd = Model_Currency::instance().GetCurrencyRecord("USD");
        if (usd == nullptr) {
            msg = _("Could not find currency 'USD', needed for converting history prices");
            return false;
        }

        multiplier = usd->BASECONVRATE;
    }


    Value history = json_doc["data"].GetArray();
    for (rapidjson::SizeType i = 0; i < history.Size(); ++i) {
        if (!history[i].IsObject()) continue;
        Value entry = history[i].GetObject();

        // if the object has both a symbol and id check if the symbol matches the target symbol
        if (entry.HasMember("priceUsd") && entry["priceUsd"].IsString() && entry.HasMember("time") && entry["time"].IsInt64()) {
            time_t dt = entry["time"].GetInt64() / 1000;
            wxDateTime date = wxDateTime(dt).GetDateOnly();
            double price_usd = -1.0;
            auto priceUSD = wxString::FromUTF8(entry["priceUsd"].GetString());

            if (!priceUSD.ToCDouble(&price_usd)) {
                msg = _("Could not parse price in asset history");
                return false;
            }

            double price_base_currency = price_usd * multiplier;
            historical_rates[date] = price_base_currency;
        }
    }

    return true;
}

//*-------------------------------- CURLcode -----------------------------------------*//

struct curlBuff {
    char *memory;
    size_t size;
};

static size_t curlWriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct curlBuff *mem = static_cast<struct curlBuff *>(userp);

    char *tmp = static_cast<char *>(realloc(mem->memory, mem->size + realsize + 1));
    if (tmp == NULL) {
        /* out of memory! */
        // printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = tmp;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

static size_t curlWriteFileCallback(void *contents, size_t size, size_t nmemb, wxFileOutputStream *stream)
{
    stream->Write(contents, size * nmemb);
    return stream->LastWrite();
}

#ifdef _DEBUG
static int log_libcurl_debug(CURL *handle, curl_infotype type, char *data, size_t size, void *userp)
{
    (void)handle; /* Not used */
    (void)userp; /* Not used */
    static const char * const s_infotype[] = {
        "*", "<", ">", "{", "}", "(", ")"
    };

    wxString text(data, size);
    wxStringTokenizer tokenizer;

    switch (type) {
    case CURLINFO_HEADER_OUT:
    case CURLINFO_TEXT:
    case CURLINFO_HEADER_IN:
        tokenizer.SetString(text, "\n", wxTOKEN_STRTOK);
        while (tokenizer.HasMoreTokens()) {
            wxString line = tokenizer.GetNextToken();
            line.Trim();
            wxLogTrace("libcurl", "%s %s", s_infotype[type], line);
        }
        break;
    case CURLINFO_DATA_OUT:
    case CURLINFO_DATA_IN:
    case CURLINFO_SSL_DATA_IN:
    case CURLINFO_SSL_DATA_OUT:
        if (wxLog::IsAllowedTraceMask("libcurl_data")) {
            tokenizer.SetString(text, "\n", wxTOKEN_STRTOK);
            while (tokenizer.HasMoreTokens()) {
                wxString line = tokenizer.GetNextToken();
                line.Trim();
                wxLogTrace("libcurl_data", "%s %s", s_infotype[type], line);
            }
        }
        else
        {
            wxLogTrace("libcurl", "%s [%zu bytes data]", s_infotype[type], size);
        }
        break;
    case CURLINFO_END:
    default:
        return 0;
    }

    return 0;
}
#endif

void curl_set_common_options(CURL* curl, const wxString& useragent = wxEmptyString) {
    wxString proxyName = Model_Setting::instance().GetStringSetting("PROXYIP", "");
    if (!proxyName.IsEmpty())
    {
        int proxyPort = Model_Setting::instance().GetIntSetting("PROXYPORT", 0);
        const wxString& proxySettings = wxString::Format("%s:%d", proxyName, proxyPort);
        curl_easy_setopt(curl, CURLOPT_PROXY, static_cast<const char*>(proxySettings.mb_str()));
    }

    int networkTimeout = Model_Setting::instance().GetIntSetting("NETWORKTIMEOUT", 10); // default 10 secs
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, networkTimeout);

    if (useragent.IsEmpty())
        curl_easy_setopt(curl, CURLOPT_USERAGENT, static_cast<const char*>(wxString::Format("%s/%s", mmex::getProgramName(), mmex::version::string).mb_str()));
    else
        curl_easy_setopt(curl, CURLOPT_USERAGENT, static_cast<const char*>(useragent.mb_str()));

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

#ifdef _DEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, log_libcurl_debug);
#endif
}

void curl_set_writedata_options(CURL* curl, curlBuff& chunk)
{
    chunk.memory = static_cast<char *>(malloc(1));
    chunk.size = 0;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
}

CURLcode http_get_data(const wxString& sSite, wxString& sOutput, const wxString& useragent)
{
    CURL *curl = curl_easy_init();
    if (!curl) return CURLE_FAILED_INIT;

    curl_set_common_options(curl, useragent);

    struct curlBuff chunk;
    curl_set_writedata_options(curl, chunk);

    curl_easy_setopt(curl, CURLOPT_URL, static_cast<const char*>(sSite.mb_str()));

    CURLcode err_code = curl_easy_perform(curl);
    if (err_code == CURLE_OK)
        sOutput = wxString::FromUTF8(chunk.memory);
    else {
        sOutput = curl_easy_strerror(err_code); //TODO: translation
        wxLogDebug("http_get_data: URL = %s error = %s", sSite, sOutput);
    }

    free(chunk.memory);
    curl_easy_cleanup(curl);
    return err_code;
}

CURLcode http_post_data(const wxString& sSite, const wxString& sData, const wxString& sContentType, wxString& sOutput)
{
    CURL *curl = curl_easy_init();
    if (!curl) return CURLE_FAILED_INIT;

    curl_set_common_options(curl);

    struct curlBuff chunk;
    curl_set_writedata_options(curl, chunk);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, sContentType.mb_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_URL, static_cast<const char*>(sSite.mb_str()));
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, static_cast<const char*>(sData.mb_str()));

    CURLcode err_code = curl_easy_perform(curl);
    if (err_code == CURLE_OK)
        sOutput = wxString::FromUTF8(chunk.memory);
    else {
        sOutput = curl_easy_strerror(err_code); //TODO: translation
        wxLogDebug("http_post_data: URL = %s error = %s", sSite, sOutput);
    }

    free(chunk.memory);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    return err_code;
}

CURLcode http_download_file(const wxString& sSite, const wxString& sPath)
{
    wxLogDebug("http_download_file: URL = %s | Target = %s", sSite, sPath);

    CURL *curl = curl_easy_init();
    if (!curl) return CURLE_FAILED_INIT;

    curl_set_common_options(curl);

    wxFileOutputStream output(sPath);
    if (!output.IsOk()) {
        wxLogDebug("http_download_file: Failed to open output file: %s error = %d", sPath, output.GetLastError());
        return CURLE_WRITE_ERROR;
    }

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFileCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);

    curl_easy_setopt(curl, CURLOPT_URL, static_cast<const char*>(sSite.mb_str()));

    CURLcode err_code = curl_easy_perform(curl);
    output.Close();
    curl_easy_cleanup(curl);

    if (err_code != CURLE_OK)
    {
        wxLogDebug("http_download_file: URL = %s error = %s", sSite, curl_easy_strerror(err_code));
    }

    return err_code;
}

const wxString getProgramDescription(int type)
{
    const wxString bull = L" \u2022 ";
    wxString eol;
    bool simple = true;
    switch (type) {
    case 0: eol = "\n"; break;
    case 1: eol = "<br>\n"; break;
    case 2: eol = "\n"; simple = false;  break;
    }

    wxString build_date = wxString() << BUILD_DATE;
    build_date = wxGetTranslation(build_date.SubString(0, 2)) + build_date.Mid(3);

    wxString description;
    description << wxString::Format(simple ? "Version: %s" : _("Version: %s"), mmex::getTitleProgramVersion()) << eol
        << bull << (simple ? "Build:" : _("Build on")) << " " << build_date << " " BUILD_TIME << eol
        << bull << (simple ? "db " : _("Database version: ")) << mmex::version::getDbLatestVersion()
#if WXSQLITE3_HAVE_CODEC
        << bull << " (" << wxSQLite3Cipher::GetCipherName(wxSQLite3Cipher::GetGlobalCipherDefault()) << ")"
#endif
        << eol

#ifdef GIT_COMMIT_HASH
        << bull << (simple ? "git " : _("Git commit: ")) << GIT_COMMIT_HASH
        << " (" << GIT_COMMIT_DATE << ")" << eol
#endif
#ifdef GIT_BRANCH
        << bull << (simple ? "" : _("Git branch: ")) << GIT_BRANCH << eol
#endif
        << eol

        << (simple ? "Libs:" : _("MMEX is using the following support products:")) << eol
        << bull + wxVERSION_STRING
        << wxString::Format(" (%s %d.%d)",
            wxPlatformInfo::Get().GetPortIdName(),
            wxPlatformInfo::Get().GetToolkitMajorVersion(),
            wxPlatformInfo::Get().GetToolkitMinorVersion())
        << eol

        << bull + wxSQLITE3_VERSION_STRING
        << " (SQLite " << wxSQLite3Database::GetVersion() << ")" << eol
        << bull + "RapidJSON " << RAPIDJSON_VERSION_STRING << eol
        << bull + LUA_RELEASE << eol
        << bull + "lunasvg v2.3.1" << eol
        << bull + curl_version() << eol
        << bull + GETTEXT_VERSION << eol
        << bull + "apexcharts.js" << eol
        << eol

        << (simple ? "Build using:" : _("Build using:")) << eol
        << bull + CMAKE_VERSION << eol
        << bull + MAKE_VERSION << eol

#if defined(_MSC_VER)
#ifdef VS_VERSION
        << bull + (simple ? "MSVS" : "Microsoft Visual Studio ") + VS_VERSION << eol
#endif
        << bull + (simple ? "MSVSC++" : "Microsoft Visual C++ ") + CXX_VERSION << eol
#elif defined(__clang__)
        << bull + "Clang " + __VERSION__ << "\n"
#elif (defined(__GNUC__) || defined(__GNUG__)) && !(defined(__clang__) || defined(__INTEL_COMPILER))
        << bull + "GCC " + __VERSION__
#endif
#ifdef CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION
        << bull + CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION
#endif
#ifdef LINUX_DISTRO_STRING
        << bull + LINUX_DISTRO_STRING
#endif
        << eol << eol
        << (simple ? "OS:" : _("Running on:")) << eol
#ifdef __LINUX__
        << bull + wxGetLinuxDistributionInfo().Description
        << " \"" << wxGetLinuxDistributionInfo().CodeName << "\"\n"
#endif
        << bull + wxGetOsDescription() << eol
        << bull + wxPlatformInfo::Get().GetDesktopEnvironment()
        << " " << wxLocale::GetLanguageName(wxLocale::GetSystemLanguage())
        << " (" << wxLocale::GetSystemEncodingName() << ")" << eol;

    for (unsigned int i = 0; i < wxDisplay::GetCount(); i++)
    {
        wxSharedPtr<wxDisplay> display(new wxDisplay(i));

        if (display->IsPrimary())
        {

            description << wxString::Format(bull + "%ix%i %ibit %ix%ippi\n",
                display->GetCurrentMode().GetWidth(),
                display->GetCurrentMode().GetHeight(),
                display->GetCurrentMode().bpp,
                display->GetPPI().GetWidth(),
                display->GetPPI().GetHeight()
            );
        }
    }

    description.RemoveLast();
    if (simple) {
        description.Replace("#", "&asymp;");
    }

    return description;
}

const wxRect GetDefaultMonitorRect()
{
    // iterate through each display until the primary is found, default to display 0
    wxSharedPtr<wxDisplay> display;
    for (unsigned int i = 0; i < wxDisplay::GetCount(); ++i) {
        display.reset(new wxDisplay(i));
        if (display->IsPrimary()) {
            break;
        }
    }

    // Get a 'sensible' location on the primary display in case we can't fit it into the window
    return display->GetClientArea();
}

// ----------------------------------------

const wxString mmTrimAmount(const wxString& value, const wxString& decimal, const wxString& replace_decimal)
{
    wxString str;
    wxString valid_strings = "-0123456789" + decimal;
    for (const auto& c : value) {
        if (valid_strings.Contains(c)) {
            str += c;
        }
    }
    if (!replace_decimal.empty()) {
        str.Replace(decimal, replace_decimal);
    }
    return str;
}

mmDates::~mmDates()
{
}

mmDates::mmDates()
    : m_date_formats_temp(g_date_formats_map())
    , m_today(wxDate::Today())
    , m_error_count(0)
{
    m_date_parsing_stat.clear();
    m_month_ago = wxDate::Today().Subtract(wxDateSpan::Months(1));
}


void mmDates::doFinalizeStatistics()
{
    auto result = std::max_element
    (
        m_date_parsing_stat.begin(),
        m_date_parsing_stat.end(),
        [](const std::pair<wxString, int>& p1, const std::pair<wxString, int>& p2) {
            return p1.second < p2.second;
        }
    );

    if (result != m_date_parsing_stat.end()) {
        m_date_format = result->first;
        const auto& date_formats = g_date_formats_map();
        wxString date_format = m_date_format;
        const auto it = std::find_if(date_formats.begin(), date_formats.end(),
            [&date_format](const std::pair<wxString, wxString>& element) { return element.first == date_format; });
        m_date_mask = it->second;
    }
    else
        wxLogDebug("No date string has been handled");
}

void mmDates::doHandleStatistics(const wxString &dateStr)
{

    if (m_error_count <= MAX_ATTEMPTS && m_date_formats_temp.size() > 1)
    {
        wxArrayString invalidMask;
        std::vector<std::pair<wxString, wxString> > date_formats = m_date_formats_temp;
        for (const auto& date_mask : date_formats)
        {
            const wxString mask = date_mask.first;
            wxDateTime dtdt = m_today;
            if (mmParseDisplayStringToDate(dtdt, dateStr, mask))
            {
                m_date_parsing_stat[mask] ++;
                //Increase the date mask rating if parse date is recent (1 month ago) date
                if (dtdt <= m_today && dtdt >= m_month_ago)
                    m_date_parsing_stat[mask] ++;

                //Decrease the data mask rating if parsed date is in future
                if (dtdt > m_today)
                    m_date_parsing_stat[mask] -= 2;
            }
            else {
                invalidMask.Add(mask);
                m_date_parsing_stat.erase(mask);
            }
        }

        if (invalidMask.size() < m_date_formats_temp.size())
        {
            for (const auto &i : invalidMask) {
                auto it = std::find_if(m_date_formats_temp.begin(), m_date_formats_temp.end(),
                    [&i](const std::pair<wxString, wxString>& element) { return element.first == i; });
                m_date_formats_temp.erase(it);
            }
        }
        else {
            m_error_count++;
        }
    }
}

mmSeparator::~mmSeparator()
{
}

mmSeparator::mmSeparator()
{
    const auto& def_delim = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);
    m_separators[";"] = 0;
    m_separators[","] = 0;
    m_separators["\t"] = 0;
    m_separators["|"] = 0;
    m_separators[def_delim] = 0;
}

const wxString mmSeparator::getSeparator() const
{
    auto x = std::max_element(m_separators.begin(), m_separators.end(),
        [](const std::pair<wxString, int>& p1, const std::pair<wxString, int>& p2) {
            return p1.second < p2.second; });
    return x->first;
}

bool mmSeparator::isStringHasSeparator(const wxString &string)
{
    bool result = false;
    bool skip = false;
    for (const auto& entry : m_separators)
    {
        for (const auto& letter : string) {
            if (letter == '"') {
                skip = !skip;
            }
            if (!skip && letter == entry.first) {
                m_separators[entry.first]++;
                result = true;
            }
        }
    }
    return result;
}

const wxString getVFname4print(const wxString& name, const wxString& data)
{
#if defined(__WXMSW__) || defined(__WXMAC__)

    const wxString file_name = wxString::Format("%s.htm", name);
    wxFileSystem fsys;
    wxSharedPtr<wxFSFile> f(fsys.OpenFile("memory:" + file_name));
    //If the file is in virtual memory, then it must be deleted before use.
    if (f.get()) {
        wxMemoryFSHandler::RemoveFile(file_name);
    }

    wxCharBuffer char_buffer;
    char_buffer = data.ToUTF8();
    wxMemoryFSHandler::AddFile(file_name, char_buffer, char_buffer.length());
    return ("memory:" + file_name);

#else
    wxString txt = data;
    txt.Replace("memory:", "");

    const auto f = wxString::Format("%s%s%shtml", mmex::getTempFolder()
        , name
        , wxString(wxFILE_SEP_EXT));

    wxFileOutputStream index_output(f);
    if (index_output.IsOk())
    {
        wxTextOutputStream index_file(index_output);
        index_file << txt;
        index_output.Close();
    }
    return "file://" + f;

#endif
}

void clearVFprintedFiles(const wxString& name)
{
    wxFileSystem fsys;
    wxFSFile *f0 = fsys.OpenFile(wxString::Format("memory:%s0.htm", name));
    if (f0) {
        delete f0;
        wxMemoryFSHandler::RemoveFile(wxString::Format("%s0.htm", name));
    }
    wxFSFile *f1 = fsys.OpenFile(wxString::Format("memory:%s1.htm", name));
    if (f1) {
        delete f1;
        wxMemoryFSHandler::RemoveFile(wxString::Format("%s1.htm", name));
    }
}

const wxString md2html(const wxString& md)
{
    wxString body = md;
    // ---- Convert Markup

    // img with link
    // skip images hosted via unsupported https
    wxRegEx re(R"(\[!\[([^]]+)\]\(([ \t]*https:\/\/[^)]+)\)\]\(([^)]+)\))", wxRE_EXTENDED);
    re.Replace(&body, R"(<a href="\3" target="_blank">\1</a>)");
    re.Compile(R"(\[!\[([^]]+)\]\(([^)]+)\)\]\(([^)]+)\))", wxRE_EXTENDED);
    re.Replace(&body, R"(<a href="\3" target="_blank"><img src="\2" alt="\1"></a>)");

    // img
    // skip images hosted via unsupported https
    re.Compile(R"(!\[([^]]+)\]\([ \t]*https:\/\/[^)]+\))", wxRE_EXTENDED);
    re.Replace(&body, R"(\1)");
    re.Compile(R"(!\[([^]]+)\]\(([^)]+)\))", wxRE_EXTENDED);
    re.Replace(&body, R"(<img src="\2" alt="\1">)");

    // link
    re.Compile(R"(\[([^]]+)\]\(([^)]+)\))", wxRE_EXTENDED);
    re.Replace(&body, R"(<a href="\2" target="_blank">\1</a>)");

    // github issues #XXXX
    re.Compile(R"(#([0-9]{4,5}))", wxRE_EXTENDED);
    re.Replace(&body, R"(<a href="https://github.com/moneymanagerex/moneymanagerex/issues/\1" target="_blank">#\1</a>)");

    body.Replace("\n", "\n<p>");

    return body;
}

wxImageList* createImageList(int size)
{
    int x = (size > 0) ? size : Option::instance().getIconSize();
    return(new wxImageList(x, x, false));   // No mask creation, not needed and causes image correuption on Mac

}

const wxColor* bestFontColour(wxColour background)
{
    // http://stackoverflow.com/a/3943023/112731

    int r = static_cast<int>(background.Red());
    int g = static_cast<int>(background.Green());
    int b = static_cast<int>(background.Blue());
    int k = (r * 299 + g * 587 + b * 114);

    wxLogDebug("best FontColour: [%s] -> r=%d, g=%d, b=%d | k: %d"
        , background.GetAsString(wxC2S_HTML_SYNTAX), r, g, b, k);

    return (k > 149000) ? wxBLACK : wxWHITE;
}

// Ideally we would use wxToolTip::Enable() to enable or disable tooltips globally.
// but this only works on some platforms! 
void mmToolTip(wxWindow* widget, wxString tip)
{
    if (Option::instance().getShowToolTips()) widget->SetToolTip(tip);
}

int pow10(int y)
{
    switch (y)
    {
    case 0: return 1;
    case 1: return 10;
    case 2: return 100;
    case 3: return 1000;
    case 4: return 10000;
    case 5: return 100000;
    case 6: return 1000000;
    case 7: return 10000000;
    case 8: return 100000000;
    case 9: return 1000000000;
    default: return 10;
    }
}

wxString HTMLEncode(wxString input)
{
    wxString output;
    for(int pos = 0; pos < input.Len(); ++pos) 
    {
        wxUniChar c = input.GetChar(pos);
        if (c.IsAscii())
            switch(static_cast<char>(c)) 
            {
                case '&':  output.Append("&amp;");      break;
                case '\"': output.Append("&quot;");     break;
                case '\'': output.Append("&apos;");     break;
                case '<':  output.Append("&lt;");       break;
                case '>':  output.Append("&gt;");       break;
                default:   output.Append(c);            break;
            }
        else
            output.Append(c);
    }
    return output;
}

const wxString __(const char* c)
{
    wxString mystring = wxGetTranslation(wxString::FromUTF8(c));
    if (mystring.Contains("\t"))
        mystring.Replace("\t", "...\t", false);
    else
        mystring.Append("...");
    return mystring;
}

void mmSetSize(wxWindow* w)
{
    auto name = w->GetName();
    wxSize my_size;

    if (name == "Split Transaction Dialog") {
        my_size = Model_Infotable::instance().GetSizeSetting("SPLITTRANSACTION_DIALOG_SIZE");
    }
    else if (name == "Organize Categories") {
        my_size = Model_Infotable::instance().GetSizeSetting("CATEGORIES_DIALOG_SIZE");
    }
    else if (name == "mmPayeeDialog") {
        my_size = Model_Infotable::instance().GetSizeSetting("PAYEES_DIALOG_SIZE");
    }
    else if (name == "Currency Dialog") {
        my_size = Model_Infotable::instance().GetSizeSetting("CURRENCY_DIALOG_SIZE");
    }
    else if (name == "Themes Dialog") {
        my_size = Model_Infotable::instance().GetSizeSetting("THEMES_DIALOG_SIZE");
    }
    else if (name == "General Reports Manager") {
        my_size = Model_Infotable::instance().GetSizeSetting("GRM_DIALOG_SIZE");
    }

    wxSharedPtr<wxDisplay> display(new wxDisplay(w->GetParent()));
    wxRect display_rect = display.get()->GetGeometry();
    display_rect.SetX(0);
    display_rect.SetY(0);

    if (display_rect.Contains(my_size)) {
        w->SetSize(my_size);
    }
    else {
        w->Fit();
    }
}

void mmFontSize(wxWindow* widget)
{
    int x = Option::instance().getFontSize();
    for (int i = 0; i < x; i++)
    {
        widget->SetFont(widget->GetFont().Larger());
    }
}
