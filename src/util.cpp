/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013-2020 Nikolay

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

#include "util.h"
#include "constants.h"
#include "platfdep.h"
#include "paths.h"
#include "validators.h"
#include "model/Model_Currency.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include "model/Model_CurrencyHistory.h"
#include <wx/sstream.h>
#include <wx/xml/xml.h>
#include <map>
#include <lua.hpp>
#include <wx/fs_mem.h>


using namespace rapidjson;

wxString JSON_PrettyFormated(rapidjson::Document& j_doc)
{
    StringBuffer j_buffer;
    PrettyWriter<StringBuffer> j_writer(j_buffer);
    j_doc.Accept(j_writer);

    return j_buffer.GetString();
}

wxString JSON_Formated(rapidjson::Document& j_doc)
{
    StringBuffer j_buffer;
    Writer<StringBuffer> j_writer(j_buffer);
    j_doc.Accept(j_writer);

    return j_buffer.GetString();
}

//----------------------------------------------------------------------------

mmTreeItemData::mmTreeItemData(int id, bool isBudget, bool isReadOnly)
    : id_(id)
    , isString_(false)
    , isBudgetingNode_(isBudget)
    , isReadOnly_(isReadOnly)
    , report_(nullptr)
{
    if (isBudget)
        stringData_ = (wxString::Format("item@Budget_%i", id));
    else
        stringData_ = (wxString::Format("%i", id));
}
mmTreeItemData::mmTreeItemData(const wxString& string, mmPrintableBase* report)
    : id_(0)
    , isString_(true)
    , isBudgetingNode_(false)
    , isReadOnly_(false)
    , stringData_("report@" + string)
    , report_(report)
{
    const wxString& n = wxString::Format("REPORT_%d", report_->getReportId());
    const wxString& settings = Model_Infotable::instance().GetStringInfo(n, "");
    report_->initReportSettings(settings);
}

mmTreeItemData::mmTreeItemData(mmPrintableBase* report)
    : id_(0)
    , isString_(true)
    , isBudgetingNode_(false)
    , isReadOnly_(false)
    , stringData_("report@" + report->getReportTitle())
    , report_(report)
{}
mmTreeItemData::mmTreeItemData(const wxString& string, bool isReadOnly)
    : id_(0)
    , isString_(true)
    , isBudgetingNode_(false)
    , isReadOnly_(isReadOnly)
    , stringData_("item@" + string)
    , report_(nullptr)
{}

//----------------------------------------------------------------------------

int CaseInsensitiveCmp(const wxString &s1, const wxString &s2)
{
    return s1.CmpNoCase(s2);
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

const wxString mmGetDateForDisplay(const wxString &iso_date)
{
    //ISO Date to formatted string lookup table.
    static std::unordered_map<wxString, wxString> dateLookup;

    static wxString dateFormat = Option::instance().getDateFormat();

    // If format has been changed, delete all stored strings.
    if (dateFormat != Option::instance().getDateFormat())
    {
        dateFormat = Option::instance().getDateFormat();
        dateLookup.clear();
    }

    // If date exists in lookup- return it.
    auto it = dateLookup.find(iso_date);
    if (it != dateLookup.end())
        return it->second; // The stored formatted date.

    // Format date, store it and return it.
    wxString date_str = dateFormat;
    if (date_str.Replace("%Y", iso_date.Mid(0, 4)) == 0)
        date_str.Replace("%y", iso_date.Mid(2, 2));
    date_str.Replace("%m", iso_date.Mid(5, 2));
    date_str.Replace("%d", iso_date.Mid(8, 2));
    return dateLookup[iso_date] = date_str;
}

bool mmParseDisplayStringToDate(wxDateTime& date, const wxString& str_date, const wxString &sDateMask)
{
    if (date_formats_regex().count(sDateMask) == 0)
        return false;

    static std::unordered_map<wxString, wxDate> cache;
    const auto it = cache.find(str_date);
    if (it != cache.end())
    {
        date = it->second;
        return true;
    }

    const wxString regex = date_formats_regex().at(sDateMask);
    wxRegEx pattern(regex);

    if (pattern.Matches(str_date))
    {
        const auto date_mask = g_date_formats_map().at(sDateMask);
        wxString date_str = pattern.GetMatch(str_date);
        if (!date_mask.Contains(" ")) {
            date_str.Replace(" ", "");
        }
        wxString::const_iterator end;
        bool t = date.ParseFormat(date_str, sDateMask, &end);
        //wxLogDebug("String:%s Mask:%s OK:%s ISO:%s Pattern:%s", date_str, date_mask, wxString(t ? "true" : "false"), date.FormatISODate(), regex);
        return t;
    }
    return false;
}

const wxDateTime mmParseISODate(const wxString& str)
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
    const wxString dd = "((([0 ][1-9])|([1-2][0-9])|(3[0-1]))|([1-9]))";
    const wxString mm = "((([0 ][1-9])|(1[0-2]))|([1-9]))";
    const wxString yy = "([0-9]{1,2})";
    const wxString yyyy = "(((19)|([2]([0]{1})))([0-9]{2}))";
    const wxString tail = "($|[^0-9])+";

    for (const auto entry : g_date_formats_map())
    {
        wxString regexp = entry.first;
        regexp.Replace("%d", dd);
        regexp.Replace("%m", mm);
        regexp.Replace("%Y", yyyy);
        regexp.Replace("%y", yy);
        regexp.Replace(".", R"(\.)");
        regexp.Append(tail);
        date_regex[entry.first] = regexp;
    }

    return date_regex;
}

const std::map<wxString, wxString> g_date_formats_map()
{
    static std::map<wxString, wxString> df;
    if (!df.empty())
        return df;

    const auto local_date_fmt = wxLocale::GetInfo(wxLOCALE_SHORT_DATE_FMT);
    const wxString formats[] = {
        local_date_fmt,
        "%Y-%m-%d", "%d/%m/%y", "%d/%m/%Y",
        "%d-%m-%y", "%d-%m-%Y", "%d.%m.%y",
        "%d.%m.%Y", "%d,%m,%y", "%d/%m'%Y",
        "%d/%m'%y", "%d/%m %Y", "%m/%d/%y",
        "%m/%d/%Y", "%m-%d-%y", "%m-%d-%Y",
        "%m/%d'%y", "%m/%d'%Y", "%y/%m/%d",
        "%y-%m-%d", "%Y/%m/%d", "%Y.%m.%d",
        "%Y %m %d", "%Y%m%d",   "%Y%d%m"
    };

    for (const auto& entry : formats)
    {
        auto local_date_mask = entry;
        local_date_mask.Replace("%Y", "YYYY");
        local_date_mask.Replace("%y", "YY");
        local_date_mask.Replace("%d", "DD");
        local_date_mask.Replace("%m", "MM");
        df[entry] = local_date_mask;
    }

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

    if (get_yahoo_prices(fiat, currency_data, base_currency_symbol, output, yahoo_price_type::FIAT))
    {

        msg << _("Currency rates have been updated");
        msg << "\n\n";
        for (const auto & item : fiat)
        {
            auto value0 = item.second;
            if (currency_data.find(item.first) != currency_data.end())
            {
                auto value1 = currency_data[item.first];
                msg << wxString::Format("%s %f -> %f\n", item.first, value0, value1);
            }
            else
            {
                msg << wxString::Format("%s %f -> %s\n", item.first, value0, _("Invalid value"));
            }
        }
    }
    else
    {
        msg = output;
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
    , const wxString base_currency_symbol
    , wxString& output
    , int type)
{
    wxString buffer;
    for (const auto& entry : symbols)
    {
        if (type == yahoo_price_type::FIAT) {
            buffer += wxString::Format("%s%s=X,", entry.first, base_currency_symbol);
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
    if (json_doc.Parse(json_data.utf8_str()).HasParseError())
        return false;


    Value r = json_doc["quoteResponse"].GetObject();
    //if (!r.HasMember("error") || !r["error"].IsNull())
    //    return false;

    Value e = r["result"].GetArray();

    if (e.Empty()) {
        output = _("Nothing to update");
        return false;
    }

    if (type == yahoo_price_type::FIAT)
    {
        wxRegEx pattern("^(...)...=X$");
        for (rapidjson::SizeType i = 0; i < e.Size(); i++)
        {
            if (!e[i].IsObject()) continue;
            Value v = e[i].GetObject();

            if (!v.HasMember("symbol") || !v["symbol"].IsString())
                continue;
            auto currency_symbol = wxString::FromUTF8(v["symbol"].GetString());
            if (pattern.Matches(currency_symbol))
            {
                if (!v.HasMember("regularMarketPrice") || !v["regularMarketPrice"].IsFloat())
                    continue;
                const auto price = v["regularMarketPrice"].GetFloat();
                currency_symbol = pattern.GetMatch(currency_symbol, 1);

                wxLogDebug("item: %u %s %f", i, currency_symbol, price);
                out[currency_symbol] = (price <= 0 ? 0 : price);
            }
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

//All components version in TXT, HTML, ABOUT
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

    wxString description;

    description << wxString::Format(simple ? "Version: %s" : _("Version: %s"), mmex::getTitleProgramVersion()) << eol
        << bull << (simple ? "db " : _("Database version: ")) << mmex::version::getDbLatestVersion()
#if WXSQLITE3_HAVE_CODEC
        << bull << " (" << wxSQLite3Cipher::GetCipherName(wxSQLite3Cipher::GetGlobalCipherDefault()) << ")"
#endif
        << eol

#ifdef GIT_COMMIT_HASH
        << bull << (simple ? "git " : _("Git commit: ")) << GIT_COMMIT_HASH
        << " (" << GIT_COMMIT_DATE << ")"
#endif
#ifdef GIT_BRANCH
        << bull << (simple ? "" : _("Git branch: ")) << GIT_BRANCH
#endif
        << eol << eol

        << (simple ? "Libs:" : _("MMEX is using the following support products:")) << eol
        << bull + wxVERSION_STRING
        << wxString::Format(" (%s %d.%d)",
            wxPlatformInfo::Get().GetPortIdName(),
            wxPlatformInfo::Get().GetToolkitMajorVersion(),
            wxPlatformInfo::Get().GetToolkitMinorVersion())
        << eol

        << bull + wxSQLITE3_VERSION_STRING
        << " (SQLite " << wxSQLite3Database::GetVersion() << ")"
        << eol

        << bull + "RapidJSON " << RAPIDJSON_VERSION_STRING << eol

        << bull + LUA_RELEASE << eol

        << bull + curl_version() << eol << eol

        << (simple ? "Build:" : _("Build on")) << " " << __DATE__ << " " << __TIME__ << " "
        << (simple ? "" : _("with:")) << eol

        << bull + CMAKE_VERSION << eol
        << bull + MAKE_VERSION << eol
        << bull + GETTEXT_VERSION << eol
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
        << " (" << wxLocale::GetSystemEncodingName() << ")" << eol
        << wxString::Format(bull + "%ix%i %ibit %ix%ippi\n",
            wxGetDisplaySize().GetX(),
            wxGetDisplaySize().GetY(),
            wxDisplayDepth(),
            wxGetDisplayPPI().GetX(),
            wxGetDisplayPPI().GetY())
        ;

    description.RemoveLast();
    if (simple) {
        description.Replace("#", "&asymp;");
    }

    return description;
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
    m_month_ago = m_today.Subtract(wxDateSpan::Months(1));
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
        m_date_mask = g_date_formats_map().at(m_date_format);
    }
    else
        wxLogDebug("No date string has been handled");
}

void mmDates::doHandleStatistics(const wxString &dateStr)
{

    if (m_error_count <= MAX_ATTEMPTS && m_date_formats_temp.size() > 1)
    {
        wxArrayString invalidMask;
        const std::map<wxString, wxString> date_formats = m_date_formats_temp;
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
            }
            else {
                invalidMask.Add(mask);
            }
        }

        if (invalidMask.size() < m_date_formats_temp.size())
        {
            for (const auto &i : invalidMask)
                m_date_formats_temp.erase(i);
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
#ifndef __WXGTK__

    int fid = 0;
    wxFileSystem fsys;
    wxFSFile *f0 = fsys.OpenFile(wxString::Format("memory:%s0.htm", name));
    if (f0) {
        delete f0;
        wxMemoryFSHandler::RemoveFile(wxString::Format("%s0.htm", name));
        fid = 1;
    }
    wxFSFile *f1 = fsys.OpenFile(wxString::Format("memory:%s1.htm", name));
    if (f1) {
        delete f1;
        wxMemoryFSHandler::RemoveFile(wxString::Format("%s1.htm", name));
        fid = 0;
    }

    wxCharBuffer char_buffer;
    char_buffer = data.ToUTF8();

    wxMemoryFSHandler::AddFile(wxString::Format("%s%i.htm", name, fid), char_buffer, char_buffer.length());
    return wxString::Format("memory:%s%i.htm", name, fid);

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
