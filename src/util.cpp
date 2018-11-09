/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013-2014 Nikolay Akimov

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
#include "mmTextCtrl.h"
#include "validators.h"
#include "option.h"
#include "reports/reportbase.h"
#include "Model_Infotable.h"
#include "Model_Setting.h"
#include <wx/sstream.h>
#include <wx/xml/xml.h>
#include <map>
//----------------------------------------------------------------------------

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

int CaseInsensitiveCmp(const wxString &s1, const wxString &s2)
{
    return s1.CmpNoCase(s2);
}

//----------------------------------------------------------------------------
mmTreeItemData::mmTreeItemData(int id, bool isBudget)
        : id_(id)
        , isString_(false)
        , isBudgetingNode_(isBudget)
        , report_(0)
    {}
mmTreeItemData::mmTreeItemData(const wxString& string, mmPrintableBase* report)
        : id_(0)
        , isString_(true)
        , isBudgetingNode_(false)
        , stringData_("report@" + string)
        , report_(report)
    {}
mmTreeItemData::mmTreeItemData(mmPrintableBase* report)
        : id_(0)
        , isString_(true)
        , isBudgetingNode_(false)
        , stringData_("report@" + report->title())
        , report_(report)
    {}
mmTreeItemData::mmTreeItemData(const wxString& string)
        : id_(0)
        , isString_(true)
        , isBudgetingNode_(false)
        , stringData_("item@" + string)
        , report_(0)
    {}
mmTreeItemData::~mmTreeItemData()
    {
        if (report_) delete report_;
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

const wxString readPasswordFromUser(const bool confirm)
{
    const wxString& caption = _("Encrypted database password");
    const wxString new_password = wxGetPasswordFromUser(
        _("Enter password for database file"),
        caption);
    wxString msg;
    if (!new_password.IsEmpty())
    {
        if (!confirm) return new_password;
        const wxString confirm_password = wxGetPasswordFromUser(
            _("Re-enter password for database file"),
            caption);
        if (!confirm_password.IsEmpty() && (new_password == confirm_password))
            return new_password;
        msg=_("Confirm password failed.");
    }
    else msg=_("Password must not be empty.");
    wxMessageBox(msg, caption, wxOK | wxICON_WARNING);
    return wxEmptyString;
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

struct curlBuff {
  char *memory;
  size_t size;
};
 
static size_t
curlWriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct curlBuff *mem = (struct curlBuff *)userp;
 
  char *tmp = (char *)realloc(mem->memory, mem->size + realsize + 1);
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

static size_t
curlWriteFileCallback(void *contents, size_t size, size_t nmemb, wxFileOutputStream *stream)
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
    chunk.memory = (char *)malloc(1);
    chunk.size = 0;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
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

    struct curl_slist *headers = NULL;
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
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&output);

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

/* Currencies & stock prices */

bool get_yahoo_prices(std::vector<wxString>& symbols
    , std::map<wxString, double>& out
    , const wxString base_currency_symbol
    , wxString& output
    , int type)
{
    wxString buffer;
    for (const auto& entry : symbols)
    {
        if (type == yahoo_price_type::FIAT) 
        {
            buffer += wxString::Format("%s%s=X,", entry, base_currency_symbol);
        }
        else
            buffer += entry + ",";
    }
    if (buffer.Right(1).Contains(",")) buffer.RemoveLast(1);

    const auto URL = wxString::Format(mmex::weblink::YahooQuotes, buffer);

    wxString json_data;
    auto err_code = http_get_data(URL, json_data);
    if (err_code != CURLE_OK)
    {
        output = json_data;
        return false;
    }

    Document json_doc;
    if (json_doc.Parse(json_data.c_str()).HasParseError())
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

                wxLogDebug("item: %zu %s %.2f", (size_t)i, currency_symbol, price);
                out[currency_symbol] = (price <= 0 ? 1 : price);
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

            wxLogDebug("item: %zu %s %.2f", (size_t)i, symbol, price);
            out[symbol] = price <= 0 ? 1 : price / k;
        }
    }

    return true;
}

bool get_crypto_currency_prices(std::vector<wxString>& symbols, double& usd_rate
    , std::map<wxString, double>& out
    , wxString& output)
{
    if (!usd_rate || usd_rate == 0)
    {
        output = _("Wrong base currency to USD rate provided");
        return false;
    }

    bool ok = false;
    const auto URL = mmex::weblink::CoinCap;

    wxString json_data;
    auto err_code = http_get_data(URL, json_data);
    if (err_code != CURLE_OK)
    {
        output = json_data;
        return false;
    }

    Document json_doc;
    if (json_doc.Parse(json_data.c_str()).HasParseError())
        return false;

    if (!json_doc.IsArray())
        return false;
    Value e = json_doc.GetArray();

    std::map<wxString, float> all_crypto_data;

    for (rapidjson::SizeType i = 0; i < e.Size(); i++)
    {
        if (!e[i].IsObject())
            continue;
        Value v = e[i].GetObject();

        if (!v["short"].IsString())  continue;
        auto currency_symbol = wxString::FromUTF8(v["short"].GetString());
        if (!v["price"].IsFloat()) continue;
        auto price = v["price"].GetFloat();
        all_crypto_data[currency_symbol] = price;
        wxLogDebug("item: %zu %s %.8f", (size_t)i, currency_symbol, price);
    }

    for (auto& entry : symbols)
    {
        if (all_crypto_data.find(entry) != all_crypto_data.end())
        {
            out[entry] = all_crypto_data.at(entry) * usd_rate;
            ok = true;
        }
        else
            output << entry << "\t: " << _("Invalid value") << "\n";
    }

    return ok;
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

const wxString mmGetDateForDisplay(const wxString &iso_date)
{
    //ISO Date to formatted string lookup table.
    static std::unordered_map<wxString, wxString> dateLookup;

    static wxString dateFormat = Option::instance().DateFormat();

    // If format has been changed, delete all stored strings.
    if (dateFormat != Option::instance().DateFormat())
    {
        dateFormat = Option::instance().DateFormat();
        dateLookup.clear();
    }

    // If date exists in lookup- return it.
    auto it = dateLookup.find(iso_date);
    if (it != dateLookup.end())
        return it->second; // The stored formatted date.

    // Format date, store it and return it.
    wxString date_str = dateFormat;
    if (date_str.Replace("%Y", iso_date.Mid(0, 4))==0)
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

    const wxString& regex = date_formats_regex().at(sDateMask);
    wxRegEx pattern(regex);

    if (pattern.Matches(str_date))
    {
        const auto& date_mask = g_date_formats_map.at(sDateMask);
        wxString date_str = pattern.GetMatch(str_date);
        if (!date_mask.Contains(" ")) {
            date_str.Replace(" ", "");
        }
        wxString::const_iterator end;
        bool t = date.ParseFormat(date_str, sDateMask, &end);
        wxLogDebug("String:%s Mask:%s OK:%s ISO:%s Pattern:%s", date_str, date_mask, wxString(t ? "true" : "false"), date.FormatISODate(), regex);
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

    if (monthNum > 0) //Test required for compatibility with previous version
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

const std::unordered_map<wxString, wxString> &date_formats_regex()
{
    static std::unordered_map<wxString, wxString> date_regex;

    // If the map was already filled, return it.
    if (!date_regex.empty())
        return date_regex;

    // First time this function is called, fill the map.
    const wxString dd = "((([0 ][1-9])|([1-2][0-9])|(3[0-1]))|([1-9]))";
    const wxString mm = "((([0 ][1-9])|(1[0-2]))|([1-9]))";
    const wxString yy = "([0-9]{1,2})";
    const wxString yyyy = "(((19)|([2]([0]{1})))([0-9]{2}))";
    const wxString tail = "($|[^0-9])+";
    date_regex["%d/%m/%y"] = wxString::Format("^%s/%s/%s%s", dd, mm, yy, tail);
    date_regex["%d/%m/%Y"] = wxString::Format("^%s/%s/%s%s", dd, mm, yyyy, tail);
    date_regex["%d-%m-%y"] = wxString::Format("^%s-%s-%s%s", dd, mm, yy, tail);
    date_regex["%d-%m-%Y"] = wxString::Format("^%s-%s-%s%s", dd, mm, yyyy, tail);
    date_regex["%d.%m.%y"] = wxString::Format("^%s\x2E%s\x2E%s%s", dd, mm, yy, tail);
    date_regex["%d.%m.%Y"] = wxString::Format("^%s\x2E%s\x2E%s%s", dd, mm, yyyy, tail);
    date_regex["%d,%m,%y"] = wxString::Format("^%s,%s,%s%s", dd, mm, yy, tail);
    date_regex["%d/%m'%Y"] = wxString::Format("^%s/%s'%s%s", dd, mm, yyyy, tail);
    date_regex["%d/%m'%y"] = wxString::Format("^%s/%s'%s%s", dd, mm, yy, tail);
    date_regex["%d/%m %Y"] = wxString::Format("^%s/%s %s%s", dd, mm, yyyy, tail);
    date_regex["%m/%d/%y"] = wxString::Format("^%s/%s/%s%s", mm, dd, yy, tail);
    date_regex["%m/%d/%Y"] = wxString::Format("^%s/%s/%s%s", mm, dd, yyyy, tail);
    date_regex["%m-%d-%y"] = wxString::Format("^%s-%s-%s%s", mm, dd, yy, tail);
    date_regex["%m-%d-%Y"] = wxString::Format("^%s-%s-%s%s", mm, dd, yyyy, tail);
    date_regex["%m/%d'%y"] = wxString::Format("^%s/%s'%s%s", mm, dd, yy, tail);
    date_regex["%m/%d'%Y"] = wxString::Format("^%s/%s'%s%s", mm, dd, yyyy, tail);
    date_regex["%y/%m/%d"] = wxString::Format("^%s/%s/%s%s", yy, mm, dd, tail);
    date_regex["%y-%m-%d"] = wxString::Format("^%s-%s-%s%s", yy, mm, dd, tail);
    date_regex["%Y/%m/%d"] = wxString::Format("^%s/%s/%s%s", yyyy, mm, dd, tail);
    date_regex["%Y-%m-%d"] = wxString::Format("^%s-%s-%s%s", yyyy, mm, dd, tail);
    date_regex["%Y.%m.%d"] = wxString::Format("^%s\x2E%s\x2E%s%s", yyyy, mm, dd, tail);
    date_regex["%Y %m %d"] = wxString::Format("^%s %s %s%s", yyyy, mm, dd, tail);
    date_regex["%Y%m%d"] = wxString::Format("^%s%s%s%s", yyyy, mm, dd, tail);
    date_regex["%Y%d%m"] = wxString::Format("^%s%s%s%s", yyyy, mm, dd, tail);

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
    , { "%d/%m'%y", "DD/MM'YY" }
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
    , { "%Y%d%m", "YYYYDDMM" }
};

const std::map<int, std::pair<wxConvAuto, wxString> > g_encoding = {
    { 0, { wxConvAuto(wxFONTENCODING_SYSTEM), wxTRANSLATE("Default") } }
    , { 1, { wxConvAuto(wxFONTENCODING_UTF8), wxTRANSLATE("UTF-8") } }
    , { 2, { wxConvAuto(wxFONTENCODING_CP1250), "1250" } }
    , { 3, { wxConvAuto(wxFONTENCODING_CP1251), "1251" } }
    , { 4, { wxConvAuto(wxFONTENCODING_CP1252), "1252" } }
    , { 5, { wxConvAuto(wxFONTENCODING_CP1253), "1253" } }
    , { 6, { wxConvAuto(wxFONTENCODING_CP1254), "1254" } }
    , { 7, { wxConvAuto(wxFONTENCODING_CP1255), "1255" } }
    , { 8, { wxConvAuto(wxFONTENCODING_CP1256), "1256" } }
    , { 9, { wxConvAuto(wxFONTENCODING_CP1257), "1257" } }
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

#ifdef __WXGTK__
void windowsFreezeThaw(wxWindow*)
{
    return;
#else
void windowsFreezeThaw(wxWindow* w)
{
    if (w->IsFrozen())
        w->Thaw();
    else
        w->Freeze();
#endif
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

    wxChar decChar = text_field->GetDecimalPoint();
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
