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

#pragma once

#include "defs.h"
#include "reports/reportbase.h"
#include <wx/valnum.h>
#include <map>
#include <curl/curl.h>
#include <rapidjson/document.h>

class mmGUIApp;

//Returns a JSON formatted string in readable form
wxString JSON_PrettyFormated(rapidjson::Document& j_doc);
//Returns a JSON formatted string from RapidJson DOM
wxString JSON_Formated(rapidjson::Document& j_doc);

struct ValuePair
{
    wxString label;
    double   amount;
};
struct ValueTrio
{
    wxString color;
    wxString label;
    double   amount;
};

struct WebsiteNews
{
    wxDateTime Date;
    wxString Title;
    wxString Link;
    wxString Description;
};

struct LineGraphData
{
    wxString xPos;
    wxString label;
    double   amount;
};

struct BarGraphData
{
    wxString fillColor;
    wxString strokeColor;
    wxString title;
    std::vector<double> data;
};

class mmListBoxItem: public wxClientData
{
public:
    mmListBoxItem(int index, const wxString& name)
        : index_(index), name_(name)
    {}

    int getIndex() const;
    wxString getName() const;

private:
    int index_;
    wxString name_;
};

inline int mmListBoxItem::getIndex() const { return index_; }
inline wxString mmListBoxItem::getName() const { return name_; }

//----------------------------------------------------------------------------

class mmTreeItemData : public wxTreeItemData
{
public:
    mmTreeItemData(int id, bool isBudget)
        : id_(id)
        , isString_(false)
        , isBudgetingNode_(isBudget)
        , stringData_(wxString::Format("%i", id))
        , report_(nullptr)
    {}
    mmTreeItemData(const wxString& string, mmPrintableBase* report)
        : id_(0)
        , isString_(true)
        , isBudgetingNode_(false)
        , stringData_("report@" + string)
        , report_(report)
    {}
    mmTreeItemData(mmPrintableBase* report)
        : id_(0)
        , isString_(true)
        , isBudgetingNode_(false)
        , stringData_("report@" + report->getReportTitle())
        , report_(report)
    {}
    mmTreeItemData(const wxString& string)
        : id_(0)
        , isString_(true)
        , isBudgetingNode_(false)
        , stringData_("item@" + string)
        , report_(nullptr)
    {}
    
    ~mmTreeItemData() {}

    int getData() const;
    const wxString getString() const;
    mmPrintableBase* get_report() const;
    bool isStringData() const;
    bool isBudgetingNode() const;

private:
    int id_;
    bool isString_;
    bool isBudgetingNode_;
    wxString stringData_;
    wxSharedPtr<mmPrintableBase> report_;
};

inline int mmTreeItemData::getData() const { return id_; }
inline const wxString mmTreeItemData::getString() const { return stringData_; }
inline mmPrintableBase* mmTreeItemData::get_report() const { return report_.get(); }
inline bool mmTreeItemData::isStringData() const { return isString_; }
inline bool mmTreeItemData::isBudgetingNode() const { return isBudgetingNode_; }

//----------------------------------------------------------------------------

int CaseInsensitiveCmp(const wxString &s1, const wxString &s2);
const wxString inQuotes(const wxString& label, const wxString& delimiter);
void csv2tab_separated_values(wxString& line, const wxString& delimit);
void correctEmptyFileExt(const wxString& ext, wxString & fileName );

void mmLoadColorsFromDatabase();

class mmColors
{
public:
    static wxColour listAlternativeColor0;
    static wxColour listAlternativeColor1;
    static wxColour listBorderColor;
    static wxColour listBackColor;
    static wxColour navTreeBkColor;
    static wxColour listDetailsPanelColor;
    static wxColour listFutureDateColor;
    static wxColour userDefColor1;
    static wxColour userDefColor2;
    static wxColour userDefColor3;
    static wxColour userDefColor4;
    static wxColour userDefColor5;
    static wxColour userDefColor6;
    static wxColour userDefColor7;
};
//----------------------------------------------------------------------------

bool getNewsRSS(std::vector<WebsiteNews>& WebsiteNewsList);
const wxString getURL(const wxString& file);
enum yahoo_price_type { FIAT = 0, SHARES };
bool getOnlineCurrencyRates(wxString& msg, int curr_id = -1, bool used_only = true);
bool get_yahoo_prices(std::map<wxString, double>& symbols
    , std::map<wxString, double>& out
    , const wxString base_currency_symbol
    , wxString& output
    , int type);

const wxString mmPlatformType();
const wxString getProgramDescription(int type = 0);
void windowsFreezeThaw(wxWindow* w);
const wxString md2html(const wxString& md);

//* Date Functions----------------------------------------------------------*//
static const wxString MONTHS[12] =
{
    wxTRANSLATE("January"), wxTRANSLATE("February"), wxTRANSLATE("March")
    , wxTRANSLATE("April"), wxTRANSLATE("May"), wxTRANSLATE("June")
    , wxTRANSLATE("July"), wxTRANSLATE("August"), wxTRANSLATE("September")
    , wxTRANSLATE("October"), wxTRANSLATE("November"), wxTRANSLATE("December")
};

const wxDateTime getUserDefinedFinancialYear(bool prevDayRequired = false);
const wxString mmGetMonthName(wxDateTime::Month month);
const std::map<wxString, wxString> &date_formats_regex();
const wxDateTime mmParseISODate(const wxString& str);
const wxString mmGetDateForDisplay(const wxString &iso_date);
bool mmParseDisplayStringToDate(wxDateTime& date, const wxString& sDate, const wxString& sDateMask);
extern const std::map<wxString, wxString> g_date_formats_map();
extern const std::map<int, std::pair<wxConvAuto, wxString> > g_encoding;

inline const wxString mmGetMonthName(wxDateTime::Month month) { return MONTHS[static_cast<int>(month)]; }
//----------------------------------------------------------------------------

CURLcode http_get_data(const wxString& site, wxString& output, const wxString& useragent = wxEmptyString);
CURLcode http_post_data(const wxString& site, const wxString& data, const wxString& contentType, wxString& output);
CURLcode http_download_file(const wxString& site, const wxString& path);

//----------------------------------------------------------------------------

const wxString mmTrimAmount(const wxString& value, const wxString& decimal, const wxString& replace_decimal ="");

class mmDates
{
public:
    mmDates();
    ~mmDates();
    void doHandleStatistics(const wxString &dateStr);
    const wxString getDateMask() const;
    const wxString getDateFormat() const;
    void doFinalizeStatistics();
    int getErrorCount() const;
    bool isDateFormatFound() const;
private:
    std::map<wxString, wxString> m_date_formats_temp;

    //Numbers of successfully applied Format Specifier for every handled date string
    std::map<wxString, int> m_date_parsing_stat;
    wxDateTime m_today;
    wxDateTime m_month_ago;
    wxString m_date_mask; //Human readable date format like DD/MM/YYYY
    wxString m_date_format; //Date Format Specifier like %d/%m/%Y
    int m_error_count;
    int MAX_ATTEMPTS = 3;
};

inline bool mmDates::isDateFormatFound() const
{
    return m_date_formats_temp.size() < g_date_formats_map().size();
}

//Get the most appropriate human readable date mask.
inline const wxString mmDates::getDateMask() const
{
    return m_date_mask;
}

//Get the most appropriate date Format Specifier.
inline const wxString mmDates::getDateFormat() const
{
    return m_date_format;
}
inline int mmDates::getErrorCount() const
{
    return m_error_count;
}

class mmSeparator
{
public:
    mmSeparator();
    ~mmSeparator();
    bool isStringHasSeparator(const wxString &string);
    const wxString getSeparator() const;
private:
    std::map<wxString, int> m_separators;

};
