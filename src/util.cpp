/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013-2022 Nikolay Akimov
 Copyright (C) 2021-2024 Mark Whalley (mark@ipx.co.uk)

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

#include <map>
#include <cwchar>
#include <locale>
#include <lua.hpp>
#include <fmt/core.h>
#include <wx/display.h>
#include <wx/sstream.h>
#include <wx/xml/xml.h>
#include <wx/fs_mem.h>

#include "build.h"
#include "util.h"
#include "constants.h"
#include "option.h"
#include "platfdep.h"
#include "paths.h"
#include "validators.h"
#include "model/Model_Setting.h"
#include "model/Model_Infotable.h"
#include "model/Model_Currency.h"
#include "model/Model_CurrencyHistory.h"

using namespace rapidjson;

// Return a JSON formatted string in readable form
wxString JSON_PrettyFormated(rapidjson::Document& j_doc)
{
    StringBuffer j_buffer;
    PrettyWriter<StringBuffer> j_writer(j_buffer);
    j_doc.Accept(j_writer);
    return wxString::FromUTF8(j_buffer.GetString());
}

// Returns a JSON formatted string from RapidJson DOM
wxString JSON_Formated(rapidjson::Document& j_doc)
{
    StringBuffer j_buffer;
    Writer<StringBuffer> j_writer(j_buffer);
    j_doc.Accept(j_writer);
    return wxString::FromUTF8(j_buffer.GetString());
}

// Get a value from RapidJson DOM
Value* JSON_GetValue(Document& j_doc, const MemoryStream::Ch* name)
{
    if (!j_doc.HasMember(name))
        return nullptr;
    return &j_doc[name];
}

// Get a bool value from RapidJson DOM
bool JSON_GetBoolValue(Document& j_doc, const MemoryStream::Ch* name, bool& value)
{
    if (!j_doc.HasMember(name))
        return false;
    Value& j_value = j_doc[name];
    if (!j_value.IsBool())
        return false;
    value = j_value.GetBool();
    return true;
}

// Get an int value from RapidJson DOM
bool JSON_GetIntValue(Document& j_doc, const MemoryStream::Ch* name, int& value)
{
    if (!j_doc.HasMember(name))
        return false;
    Value& j_value = j_doc[name];
    if (!j_value.IsInt())
        return false;
    value = j_value.GetInt();
    return true;
}

// Get a string value from RapidJson DOM
bool JSON_GetStringValue(Document& j_doc, const MemoryStream::Ch* name, wxString& value)
{
    if (!j_doc.HasMember(name))
        return false;
    Value& j_value = j_doc[name];
    if (!j_value.IsString())
        return false;
    value = wxString::FromUTF8(j_value.GetString());
    return true;
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
        label.Replace("\"", "\"\"", true);
        label = wxString() << "\"" << label << "\"";
    }

    label.Replace("\t", "    ", true);
    label.Replace("\n", " ", true);
    return label;
}

void mmLoadColorsFromDatabase(const bool def)
    {
    mmColors::userDefColor1 = def ? wxColour(246, 144, 144) : Model_Infotable::instance().getColour("USER_COLOR1", wxColour(246, 144, 144));
    mmColors::userDefColor2 = def ? wxColour(229, 196, 146) : Model_Infotable::instance().getColour("USER_COLOR2", wxColour(229, 196, 146));
    mmColors::userDefColor3 = def ? wxColour(245, 237, 149) : Model_Infotable::instance().getColour("USER_COLOR3", wxColour(245, 237, 149));
    mmColors::userDefColor4 = def ? wxColour(186, 226, 185) : Model_Infotable::instance().getColour("USER_COLOR4", wxColour(186, 226, 185));
    mmColors::userDefColor5 = def ? wxColour(135, 190, 219) : Model_Infotable::instance().getColour("USER_COLOR5", wxColour(135, 190, 219));
    mmColors::userDefColor6 = def ? wxColour(172, 167, 239) : Model_Infotable::instance().getColour("USER_COLOR6", wxColour(172, 167, 239));
    mmColors::userDefColor7 = def ? wxColour(212, 138, 215) : Model_Infotable::instance().getColour("USER_COLOR7", wxColour(212, 138, 215));
}

wxColour mmColors::userDefColor1;
wxColour mmColors::userDefColor2;
wxColour mmColors::userDefColor3;
wxColour mmColors::userDefColor4;
wxColour mmColors::userDefColor5;
wxColour mmColors::userDefColor6;
wxColour mmColors::userDefColor7;

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

    wxLogDebug("{{{ getNewsRSS()");

    const wxString news_last_read_date_str = Model_Setting::instance().getString(INIDB_NEWS_LAST_READ_DATE, "");
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

    wxLogDebug("New articles: %i", static_cast<int>(WebsiteNewsList.size()));
    wxLogDebug("}}}");

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
    }
    //Replace back all replacers to the original value
    temp_line.Replace("\5", "\"");
    temp_line.Replace("\6", "'");
    line = temp_line;
}

//* Date Functions----------------------------------------------------------*//

const wxString mmGetDateTimeForDisplay(const wxString &datetime_iso, const wxString& format)
{
    // ISO Date to formatted string lookup table.
    static std::unordered_map<wxString, wxString> cache;
    static wxString cache_format;
    static wxRegEx date_pattern(R"(^[0-9]{4}\-[0-9]{2}\-[0-9]{2})");

    if (format.empty())
        return "";

    if (!date_pattern.Matches(datetime_iso))
        return "";

    // If format has been changed, delete all stored strings.
    if (cache_format != format) {
        cache.clear();
        cache_format = format;
    }

    // Reset cache if it is too big.
    if (cache.size() > 2000) {
        cache.clear();
    }

    // If datetime_iso is in cache, return the stored formatted string.
    if (auto it = cache.find(datetime_iso); it != cache.end())
        return it->second;

    // Format date.
    wxString datetime_str = format;
    datetime_str.Replace("%Y", datetime_iso.Mid(0, 4));
    datetime_str.Replace("%y", datetime_iso.Mid(2, 2));
    if (datetime_str.Contains("%Mon")) {
        const auto mon = wxGetTranslation(MONTHS_SHORT[wxAtoi(datetime_iso.Mid(5, 2)) - 1]);
        datetime_str.Replace("%Mon", mon);
    }
    datetime_str.Replace("%m", datetime_iso.Mid(5, 2));
    datetime_str.Replace("%d", datetime_iso.Mid(8, 2));
    if (datetime_str.Contains("%w")) {
        wxDateTime d;
        d.ParseISODate(datetime_iso);
        const auto weekday = wxGetTranslation(g_short_days_of_week[d.GetWeekDay()]);
        datetime_str.Replace("%w", weekday);
    }

    // Format time.
    if (datetime_iso.Length() == 19) {
        datetime_str.Replace("%H", datetime_iso.Mid(11, 2));
        datetime_str.Replace("%M", datetime_iso.Mid(14, 2));
        datetime_str.Replace("%S", datetime_iso.Mid(17, 2));
    }

    // Store formatted string and return it.
    return cache[datetime_iso] = datetime_str;
}

const wxString mmGetDateForDisplay(const wxString &datetime_iso, const wxString& format)
{
    // ISO Date to formatted string lookup table.
    static std::unordered_map<wxString, wxString> cache;
    static wxString cache_format;
    static wxRegEx date_pattern(R"(^[0-9]{4}\-[0-9]{2}\-[0-9]{2})");

    if (format.empty())
        return "";

    if (!date_pattern.Matches(datetime_iso))
        return "";

    // If format has been changed, delete all stored strings.
    if (cache_format != format) {
        cache.clear();
        cache_format = format;
    }

    // Reset cache if it is too big.
    if (cache.size() > 2000) {
        cache.clear();
    }

    // Get the date part.
    wxString date_iso = datetime_iso.Left(10);

    // If date_iso is in cache, return the stored formatted string.
    if (auto it = cache.find(date_iso); it != cache.end())
        return it->second;

    // Format date.
    wxString date_str = format;
    date_str.Replace("%Y", date_iso.Mid(0, 4));
    date_str.Replace("%y", date_iso.Mid(2, 2));
    if (date_str.Contains("%Mon")) {
        const auto mon = wxGetTranslation(MONTHS_SHORT[wxAtoi(date_iso.Mid(5, 2)) - 1]);
        date_str.Replace("%Mon", mon);
    }
    date_str.Replace("%m", date_iso.Mid(5, 2));
    date_str.Replace("%d", date_iso.Mid(8, 2));
    if (date_str.Contains("%w")) {
        wxDateTime d;
        d.ParseISODate(date_iso);
        const auto weekday = wxGetTranslation(g_short_days_of_week[d.GetWeekDay()]);
        date_str.Replace("%w", weekday);
    }

    // Store formatted string and return it.
    return cache[date_iso] = date_str;
}

const wxString mmGetTimeForDisplay(const wxString& datetime_iso)
{
    return (datetime_iso.Length() == 19) ? datetime_iso.Mid(11, 8) : wxString("00:00:00");
}

bool mmParseDisplayStringToDate(wxDateTime& date, const wxString& str_date, const wxString &sDateMask)
{
    if (date_formats_regex().count(sDateMask) == 0)
        return false;

    wxString date_str = str_date;
    wxString mask_str = sDateMask;

    static std::unordered_map<wxString, wxDate> cache;
    if (const auto it = cache.find(str_date); it != cache.end())
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

const wxDateTime getUserDefinedFinancialYear(const bool prevDayRequired)
{
    int day = Option::instance().getFinancialFirstDay();
    wxDateTime::Month month = Option::instance().getFinancialFirstMonth();
    int year = wxDate::GetCurrentYear();

    if (wxDate::GetCurrentMonth() < month) year--;
    if (day < 1 || day > wxDateTime::GetNumberOfDays(month, year))
        day = 1;

    wxDateTime financialYear(day, month, year);
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

    for (const auto &entry : g_date_formats_map())
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

bool comp(const std::pair<wxString, wxString>& a, const std::pair<wxString, wxString>& b)
{

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

    std::vector<wxString> date_formats = {
        "%d %Mon %Y",
        "%d %Mon %y",
        "%d-%Mon-%Y",
        "%d-%Mon-%y",
        "%d %Mon'%y",
        "%d %m %y",
        "%d %m %Y",
        "%d,%m,%y",
        "%d.%m.%y",
        "%d.%m.%Y",
        "%d.%m'%Y",
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
    if (std::find(date_formats.begin(), date_formats.end(), local_date_fmt) == date_formats.end())
        date_formats.push_back(local_date_fmt);

    for (const auto& entry : date_formats)
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
    { 0, { wxConvAuto(wxFONTENCODING_SYSTEM), _n("Default") } },
    { 1, { wxConvAuto(wxFONTENCODING_UTF8),   "UTF-8" } },
    { 2, { wxConvAuto(wxFONTENCODING_CP1250), "1250" } },
    { 3, { wxConvAuto(wxFONTENCODING_CP1251), "1251" } },
    { 4, { wxConvAuto(wxFONTENCODING_CP1252), "1252" } },
    { 5, { wxConvAuto(wxFONTENCODING_CP1253), "1253" } },
    { 6, { wxConvAuto(wxFONTENCODING_CP1254), "1254" } },
    { 7, { wxConvAuto(wxFONTENCODING_CP1255), "1255" } },
    { 8, { wxConvAuto(wxFONTENCODING_CP1256), "1256" } },
    { 9, { wxConvAuto(wxFONTENCODING_CP1257), "1257" } }
};

wxString cleanseNumberString(const wxString& str,const bool decimal)
{
    // Strip any thousands separators and make sure decimal is "." (if present)
    wxString content = str;
    if (decimal)
    {
        wxRegEx pattern(R"([\., ](?=\d*[\., ]))");  // Leave the decimal seperator
        pattern.ReplaceAll(&content, wxEmptyString);
        content.Replace(",",".");
    } else
    {
        wxRegEx pattern(R"([\., ])");
        pattern.ReplaceAll(&content, wxEmptyString);           
    }
    return content;
}

double cleanseNumberStringToDouble(const wxString& str, const bool decimal)
{
    double v;
    if (!cleanseNumberString(str, decimal).ToCDouble(&v))
        v = 0;
    return v;
}


//
const wxString mmPlatformType()
{
    return wxPlatformInfo::Get().GetOperatingSystemFamilyName().substr(0, 3).MakeLower();
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

bool getOnlineCurrencyRates(wxString& msg,const int64 curr_id, const bool used_only)
{
    wxString base_currency_symbol;

    if (!Model_Currency::GetBaseCurrencySymbol(base_currency_symbol))
    {
        msg = _t("Unable to find base currency symbol!");
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
        msg = _t("Nothing to update");
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
    msg << _t("Currency rates have been updated");
    msg << "\n\n";
    for (const auto & item : fiat)
    {
        const wxString value0_str(fmt::format("{:>{}}", fmt::string_view(Model_Currency::toString(item.second, b, 4).mb_str()), 20));
        const wxString symbol(fmt::format("{:<{}}", fmt::string_view(item.first.mb_str()), 10));

        if (currency_data.find(item.first) != currency_data.end())
        {
            auto value1 = currency_data[item.first];
            const wxString value1_str(fmt::format("{:>{}}", fmt::string_view(Model_Currency::toString(value1, b, 4).mb_str()), 20));
            msg << wxString::Format("%s\t%s\t\t%s\n", symbol, value0_str, value1_str);
        }
        else
        {
            msg << wxString::Format("%s\t%s\t\t%s\n", symbol, value0_str, _t("Invalid value"));
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
                if(Option::instance().getUseCurrencyHistory())
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
    if (type == yahoo_price_type::FIAT && !wxString("USD|EUR|GBP").Contains(base_currency_symbol))
    {
        base_curr_symbol = "USD";
        buffer += wxString::Format("%s%s=X,", base_currency_symbol, base_curr_symbol);
    }

    for (const auto& entry : symbols)
    {
        wxRegEx pattern(R"(^([\^-a-zA-Z0-9_@=\.]+)$)");
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
    auto err_code = getYahooFinanceQuotes(URL, json_data);
    if (err_code != CURLE_OK)
    {
        output = json_data;
        return false;
    }

    Document json_doc;
    if (json_doc.Parse(json_data.utf8_str()).HasParseError()) {
        output = _t("JSON Parse Error");
        return false;
    }

    /*
    {"finance":{"result":null,"error":{"code":"Bad Request","description":"Missing required query parameter=symbols"}}}
    */

    if (json_doc.HasMember("finance") && json_doc["finance"].IsObject()) {
        Value e = json_doc["finance"].GetObject();
        if (e.HasMember("error")) {
            if (e["error"].IsObject()) {
                Value err = e["error"].GetObject();
                if (err.HasMember("description") && err["description"].IsString()) {
                    output = wxString::FromUTF8(err["description"].GetString());
                    return false;
                }
            }
        }
    }

    /*{"quoteResponse":{"result":[{"currency":"USD","regularMarketPrice":173.57,"shortName":"Apple Inc.","regularMarketTime":1683316804,"symbol":"AAPL"}],"error":null}}*/

    if (json_doc.HasMember("quoteResponse") && json_doc["quoteResponse"].IsObject()) {
        Value r = json_doc["quoteResponse"].GetObject();
        if (r.HasMember("result") && r["result"].IsArray()) {
            Value e = r["result"].GetArray();

            if (e.Empty()) {
                output = _t("Nothing to update");
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
        }
        else {
            output = _t("JSON Parse Error");
            return false;
        }
    }
    else
    {
        output = _t("JSON Parse Error");
        return false;
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
bool getCoincapInfoFromSymbol(const wxString& symbol, wxString& out_id, double& price_usd, wxString& output) {
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
        output = _t("JSON Parse Error");
        return false;
    }
    
    if (!json_doc.HasMember("data") || !json_doc["data"].IsArray()) {
        if (json_doc.HasMember("error") && json_doc["error"].IsString()) {
            output = wxString::Format("Error from coincap API: %s", json_doc["error"].GetString());
        } else {
            output = _t("Expected response to contain a data or error string");
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

    output = _t("Unable to find asset for symbol.");
    return false;
}

bool getCoincapAssetHistory(const wxString& asset_id, wxDateTime begin_date, std::map<wxDateTime, double> &historical_rates, wxString &msg) {
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
        msg = _t("JSON Parse Error");
        return false;
    }

    if (!json_doc.HasMember("data") || !json_doc["data"].IsArray()) {
        if (json_doc.HasMember("error") && json_doc["error"].IsString()) {
            msg = wxString::Format("Error from coincap API: %s", json_doc["error"].GetString());
        } else {
            msg = _t("Expected response to contain a data or error string");
        }
        return false;
    }

    wxString baseCurrencySymbol;
    if (!Model_Currency::GetBaseCurrencySymbol(baseCurrencySymbol)) {
        msg = _t("Unable to get base currency!");
        return false;
    }

    // prices in USD are multiplied by this value to convert them to the base currency
    double multiplier = 1.0;
    if (baseCurrencySymbol != _t("USD")) {
        auto usd = Model_Currency::instance().GetCurrencyRecord("USD");
        if (usd == nullptr) {
            msg = _t("Unable to find currency 'USD', required for converting historical prices");
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
                msg = _t("Unable to parse price in asset history");
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

static size_t curlWriteMemoryCallback(void *contents,const size_t size,const size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct curlBuff *mem = static_cast<struct curlBuff *>(userp);

    char *tmp = static_cast<char *>(realloc(mem->memory, mem->size + realsize + 1));
    if (tmp == nullptr) {
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

static size_t curlWriteFileCallback(void *contents,const size_t size,const size_t nmemb, wxFileOutputStream *stream)
{
    stream->Write(contents, size * nmemb);
    return stream->LastWrite();
}

#ifdef _DEBUG
static int log_libcurl_debug(CURL *handle,const curl_infotype type, char *data,const size_t size, void *userp)
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
    wxString proxyName = Model_Setting::instance().getString("PROXYIP", "");
    if (!proxyName.IsEmpty())
    {
        int proxyPort = Model_Setting::instance().getInt("PROXYPORT", 0);
        const wxString& proxySettings = wxString::Format("%s:%d", proxyName, proxyPort);
        curl_easy_setopt(curl, CURLOPT_PROXY, static_cast<const char*>(proxySettings.mb_str()));
    }

    int networkTimeout = Model_Setting::instance().getInt("NETWORKTIMEOUT", 10); // default 10 secs
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
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); //#5870

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

CURLcode getYahooFinanceQuotes(const wxString& URL, wxString& output) {
    CURL* curl = curl_easy_init();
    if (!curl) return CURLE_FAILED_INIT;

    struct curlBuff cookie{nullptr, 0};
    struct curlBuff crumb{nullptr, 0};
    struct curlBuff quote{nullptr, 0};

    wxString savedCookie = Model_Setting::instance().getString("YAHOO_FINANCE_COOKIE", "");
    wxString savedCrumb = Model_Setting::instance().getString("YAHOO_FINANCE_CRUMB", "");

    // Request to get cookies and save them to the cookie buffer
    curl_set_common_options(curl);
    curl_set_writedata_options(curl, quote);
    wxString crumb_url = URL + "&crumb=" + savedCrumb;
    curl_easy_setopt(curl, CURLOPT_URL, static_cast<const char*>(crumb_url.mb_str()));
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "");

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8");
    headers = curl_slist_append(headers, "user-agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/136.0.0.0 Safari/537.36");
    if (!savedCookie.IsEmpty()) headers = curl_slist_append(headers, static_cast<const char*>(("Cookie: " + savedCookie).mb_str()));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = CURLE_OK;
    if(!savedCookie.IsEmpty() && !savedCrumb.IsEmpty())
        res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        if (wxString::FromUTF8(quote.memory).Contains("Unauthorized") || savedCookie.IsEmpty() || savedCrumb.IsEmpty())
        {
            curl_set_writedata_options(curl, cookie);
            curl_easy_setopt(curl, CURLOPT_URL, "https://finance.yahoo.com");
            res = curl_easy_perform(curl);
            if (res == CURLE_OK)
            {
                wxString response = wxString::FromUTF8(cookie.memory);
                wxRegEx csrfTokenPattern("csrfToken\" value=\"([^\"]+)\">");
                if (csrfTokenPattern.Matches(response))
                {
                    // Get the csrfToken
                    wxString csrfToken = csrfTokenPattern.GetMatch(response, 1);

                    wxRegEx sessionIdPattern("sessionId\" value=\"([^\"]+)\">");
                    if (sessionIdPattern.Matches(response))
                    {
                        wxString sessionId = sessionIdPattern.GetMatch(response, 1);

                        wxString postData = "csrfToken=" + csrfToken + "&sessionId=" + sessionId + "&originalDoneUrl=https%3A%2F%2Ffinance.yahoo.com%2F%3Fguccounter%3D1&namespace=yahoo&reject=reject&reject=reject";
                        curl_easy_setopt(curl, CURLOPT_URL, static_cast<const char*>(("https://consent.yahoo.com/v2/collectConsent?sessionId=" + sessionId).mb_str()));
                        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, static_cast<const char*>(postData.mb_str()));
                        res = curl_easy_perform(curl);
                        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
                    }
                }

                if (res == CURLE_OK) {
                    // Request to get crumb using the saved cookies
                    curl_set_writedata_options(curl, crumb);
                    curl_easy_setopt(curl, CURLOPT_URL, "https://query1.finance.yahoo.com/v1/test/getcrumb");
                    res = curl_easy_perform(curl);
                    if (res == CURLE_OK) {
                        struct curlBuff newQuote { nullptr, 0 };
                        // Request to get the quotes using the crumb and saved cookies
                        curl_set_writedata_options(curl, newQuote);
                        crumb_url = URL + "&crumb=" + wxString::FromUTF8(crumb.memory);
                        curl_easy_setopt(curl, CURLOPT_URL, static_cast<const char*>(crumb_url.mb_str()));
                        res = curl_easy_perform(curl);
                        if (res == CURLE_OK) {
                            output = wxString::FromUTF8(newQuote.memory);
                            wxString cookieJar;
                            // Get the cookies from the cookie jar
                            struct curl_slist* cookies = nullptr;
                            curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
                            if (cookies) {
                                for (struct curl_slist* item = cookies; item; item = item->next) {
                                    wxStringTokenizer tokenizer(wxString::FromUTF8(item->data), "\t");
                                    int count = 0;
                                    while (tokenizer.HasMoreTokens()) {
                                        wxString token = tokenizer.GetNextToken();
                                        if (count++ < 5) continue;
                                        cookieJar += token;
                                        (count < 7) ? cookieJar += "=" : cookieJar += "; ";
                                    }
                                }
                                curl_slist_free_all(cookies);
                            }

                            Model_Setting::instance().setString("YAHOO_FINANCE_COOKIE", cookieJar);
                            Model_Setting::instance().setString("YAHOO_FINANCE_CRUMB", wxString::FromUTF8(crumb.memory));
                        }
                        free(newQuote.memory);
                    }
                }
            }
        }
        else {
            output = wxString::FromUTF8(quote.memory);
        }
    }
    if (res != CURLE_OK) {
        output = curl_easy_strerror(res); //TODO: translation
        wxLogDebug("http_get_data: URL = %s error = %s", URL, output);
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(cookie.memory);
    free(crumb.memory);
    free(quote.memory);
    return res;

}

const wxString getProgramDescription(const int type)
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

#ifdef __LINUX__
    wxString desktop_environment;

    if (wxPlatformInfo::Get().GetDesktopEnvironment().Len() > 0) {
        desktop_environment = wxPlatformInfo::Get().GetDesktopEnvironment();
    } else if (wxGetenv("XDG_CURRENT_DESKTOP")) {
        desktop_environment = wxGetenv("XDG_CURRENT_DESKTOP");
    } else if (wxGetenv("XDG_SESSION_DESKTOP")) {
        desktop_environment = wxGetenv("XDG_SESSION_DESKTOP");
    } else if (wxGetenv("DESKTOP_SESSION")) {
        desktop_environment = wxGetenv("DESKTOP_SESSION");
    } else {
        desktop_environment = wxGetenv("GDMSESSION");
    }

    wxString session_type;

    switch(wxGetDisplayInfo().type) {
        case wxDisplayX11:
            session_type = "X11";
            break;
        case wxDisplayWayland:
            session_type = "Wayland";
            break;
        case wxDisplayNone:
        default:
            session_type = "unknown display protocol";
    }
#endif

    wxString description;
    description << bull << wxString::Format(simple ? "Version: %s" : _t("Version: %s"), mmex::getTitleProgramVersion()) << eol
        << bull << wxString::Format(simple ? "Built: %1$s %2$s" : _t("Built on: %1$s %2$s"), build_date, BUILD_TIME) << eol
        << bull << wxString::Format(simple ? "db %d" : _t("Database version: %d"), mmex::version::getDbLatestVersion())
#if WXSQLITE3_HAVE_CODEC
        << " (aes256cbc-hmac-sha512)"
#endif
        << eol

#ifdef GIT_COMMIT_HASH
        << bull << wxString::Format(simple ? "git %1$s (%2$s)" : _t("Git commit: %1$s (%2$s)"), GIT_COMMIT_HASH, GIT_COMMIT_DATE) << eol
#endif
#ifdef GIT_BRANCH
        << bull << wxString::Format(simple ? "%s" : _t("Git branch: %s"), GIT_BRANCH) << eol
#endif
        << eol

        << (simple ? "Libs:" : _t("MMEX is using the following support products:")) << eol
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
        << bull + curl_version() << eol
        << bull + GETTEXT_VERSION << eol
        << bull + "apexcharts.js" << eol
        << eol

        << (simple ? "Built with:" : _t("Built with:")) << eol
        << bull + CMAKE_VERSION << eol
        << bull + MAKE_VERSION << eol

#if defined(_MSC_VER)
#ifdef VS_VERSION
        << bull + (simple ? "MSVS" : "Microsoft Visual Studio ") + VS_VERSION << eol
#endif
        << bull + (simple ? "MSVSC++" : "Microsoft Visual C++ ") + CXX_VERSION << eol
#elif defined(__clang__)
        << bull + "Clang " + __VERSION__ << eol
#elif (defined(__GNUC__) || defined(__GNUG__)) && !(defined(__clang__) || defined(__INTEL_COMPILER))
        << bull + "GCC " + __VERSION__ << eol
#endif

#ifdef CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION
        << bull + CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION << eol
#endif
        << eol
        << (simple ? "OS:" : _t("Running on:")) << eol

#ifdef LINUX_DISTRO_STRING
        << bull + LINUX_DISTRO_STRING << eol
#endif

#ifdef __LINUX__
        << bull + wxGetLinuxDistributionInfo().Description
        << " \"" << wxGetLinuxDistributionInfo().CodeName << "\"" << eol
#endif
        << bull + wxGetOsDescription() << eol
#ifdef __LINUX__
        << bull + desktop_environment << " (" << session_type << ")" << eol
#endif
        << bull << wxString::Format(simple ? "System Locale: %s" : _t("System Locale: %s"), std::locale("").name()) << eol
        << bull << wxString::Format(simple ? "User Interface Language: %s" : _t("User Interface Language: %s"), wxTranslations::Get()->GetBestTranslation("mmex") + "." + wxLocale::GetSystemEncodingName()) << eol;

    for (unsigned int i = 0; i < wxDisplay::GetCount(); i++)
    {
        wxSharedPtr<wxDisplay> display(new wxDisplay(i));

        if (display->IsPrimary())
        {

            description << wxString::Format(bull + "%1$ix%2$i %3$i-bit %4$ix%5$ippi" + "\n",
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
mmDates::~mmDates()
{
}

mmDates::mmDates()
    : m_date_formats_temp(g_date_formats_map())
    , m_today(wxDate::Today())
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
    const auto& def_delim = Model_Infotable::instance().getString("DELIMITER", mmex::DEFDELIMTER);
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

    for (const auto& entry : m_separators)
    {
        bool skip = false;
        for (const auto& letter : string)
        {
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

    wxRegEx re(R"(https:\/\/github\.com\/moneymanagerex\/moneymanagerex\/milestone\/(\d+)\?closed=1)", wxRE_EXTENDED);
    re.Replace(&body, R"(<a href="https://github.com/moneymanagerex/moneymanagerex/milestone/\1?closed=1" target="_blank">The complete list of closed issues is available at this link</a>)");
    body.Replace("The complete list of closed issues is available at this link", _t("The complete list of closed issues is available at this link"));

    // img with link
    // skip images hosted via unsupported https
    re.Compile(R"(\[!\[([^]]+)\]\(([ \t]*https:\/\/[^)]+)\)\]\(([^)]+)\))", wxRE_EXTENDED);
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

    body.Replace("\n", "\n<br>");

    return body;
}

wxImageList* createImageList(const int size)
{
    int x = (size > 0) ? size : Option::instance().getIconSize();
    return(new wxImageList(x, x, false));   // No mask creation, not needed and causes image correuption on Mac

}

// Ideally we would use wxToolTip::Enable() to enable or disable tooltips globally.
// but this only works on some platforms! 
void mmToolTip(wxWindow* widget, const wxString& tip)
{
    if (Option::instance().getShowToolTips()) widget->SetToolTip(tip);
}

wxString HTMLEncode(const wxString& input)
{
    wxString output;
    for(size_t pos = 0; pos < input.Len(); ++pos)
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

void mmSetSize(wxWindow* w)
{
    auto name = w->GetName();
    wxSize my_size;

    if (name == "Split Transaction Dialog") {
        my_size = Model_Infotable::instance().getSize("SPLITTRANSACTION_DIALOG_SIZE");
        my_size.SetHeight(w->GetSize().GetHeight());  // Do not touch the height
    }
    else if (name == "Organize Categories") {
        my_size = Model_Infotable::instance().getSize("CATEGORIES_DIALOG_SIZE");
    }
    else if (name == "mmPayeeDialog") {
        my_size = Model_Infotable::instance().getSize("PAYEES_DIALOG_SIZE");
    }
    else if (name == "Organize Currencies") {
        my_size = Model_Infotable::instance().getSize("CURRENCY_DIALOG_SIZE");
    }
    else if (name == "Column Order Dialog") {
        my_size = Model_Infotable::instance().getSize("COLUMNORDER_DIALOG_SIZE");
    }
    else if (name == "Themes Dialog") {
        my_size = Model_Infotable::instance().getSize("THEMES_DIALOG_SIZE");
    }
    else if (name == "General Reports Manager") {
        my_size = Model_Infotable::instance().getSize("GRM_DIALOG_SIZE");
    } 
    else if (name == "mmEditPayeeDialog") {
        my_size = Model_Infotable::instance().getSize("EDITPAYEE_DIALOG_SIZE"); 
    }
    else if (name == "mmEditSplitOther") {
        my_size = Model_Infotable::instance().getSize("EDITSPLITOTHER_DIALOG_SIZE"); 
    }
    else if (name == "Transactions Dialog") {
        my_size = Model_Infotable::instance().getSize("TRANSACTION_DIALOG_SIZE");
    }
    else if (name == "Merge categories") {
        my_size = Model_Infotable::instance().getSize("RELOCATECATEG_DIALOG_SIZE");
    }
    else if (name == "Merge payees") {
        my_size = Model_Infotable::instance().getSize("RELOCATEPAYEE_DIALOG_SIZE");
    }
    else if (name == "Scheduled Transaction Dialog") {
        my_size = Model_Infotable::instance().getSize("RECURRINGTRANS_DIALOG_SIZE");
    }
    else if (name == "Transaction Filter") {
        my_size = Model_Infotable::instance().getSize("TRANSACTION_FILTER_SIZE");
    }
    else if (name == "Organize Tags") {
        my_size = Model_Infotable::instance().getSize("TAG_DIALOG_SIZE");
    }
    else if (name == "Merge tags") {
        my_size = Model_Infotable::instance().getSize("RELOCATETAG_DIALOG_SIZE");
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

//
// mmHtmlWindow just adds a right click menu to save text to the system clipboard
//

mmHtmlWindow::mmHtmlWindow( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size
                    , long style, const wxString &name)
            : wxHtmlWindow(parent, id, pos, size, style, name)
{
    this->Bind(wxEVT_RIGHT_DOWN, &mmHtmlWindow::OnMouseRightClick, this);
    this->Bind(wxEVT_MENU, &mmHtmlWindow::OnMenuSelected, this);
}

void mmHtmlWindow::OnMouseRightClick(wxMouseEvent& /*event*/)
{
    wxMenu menu;
    menu.Append(wxID_HIGHEST + 1, _t("Copy all text to clipboard"));
    PopupMenu(&menu);

}

void mmHtmlWindow::OnMenuSelected(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_HIGHEST;
    if (i == 1) // There is only one anyway
    {
        if (wxTheClipboard->Open())
        {
            wxTheClipboard->SetData(new wxTextDataObject(this->ToText()));
            wxTheClipboard->Close();
        }
    }
}
