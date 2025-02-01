/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013-2022 Nikolay Akimov
 Copyright (C) 2021,2024 Mark Whalley (mark@ipx.co.uk)

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

#include <wx/clipbrd.h>
#include <wx/valnum.h>
#include <map>
#include <curl/curl.h>
#include <rapidjson/document.h>

#include "defs.h"
#include "primitive.h"
#include "option.h"

class mmGUIApp;

//Returns a JSON formatted string in readable form
wxString JSON_PrettyFormated(rapidjson::Document& j_doc);
//Returns a JSON formatted string from RapidJson DOM
wxString JSON_Formated(rapidjson::Document& j_doc);

struct ValuePair
{
    wxString label;
    double amount;
};

struct ValueTrio
{
    wxString color;
    wxString label;
    double amount;
};

struct WebsiteNews
{
    wxDateTime Date;
    wxString Title;
    wxString Link;
    wxString Description;
};

class mmListBoxItem: public wxClientData
{
public:
    mmListBoxItem(int64 index, const wxString& name)
        : index_(index), name_(name)
    {}

    int64 getIndex() const;
    wxString getName() const;

private:
    int64 index_;
    wxString name_;
};

inline int64 mmListBoxItem::getIndex() const { return index_; }
inline wxString mmListBoxItem::getName() const { return name_; }

//----------------------------------------------------------------------------

const wxString inQuotes(const wxString& label, const wxString& delimiter);
void csv2tab_separated_values(wxString& line, const wxString& delimit);
void correctEmptyFileExt(const wxString& ext, wxString & fileName );

void mmLoadColorsFromDatabase(const bool def = false);

//----------------------------------------------------------------------------

bool getNewsRSS(std::vector<WebsiteNews>& WebsiteNewsList);
enum yahoo_price_type { FIAT = 0, SHARES };
bool getOnlineCurrencyRates(wxString& msg, const int64 curr_id = -1, const bool used_only = true);
bool get_yahoo_prices(
    std::map<wxString, double>& symbols,
    std::map<wxString, double>& out,
    const wxString& base_currency_symbol,
    wxString& output,
    int type
);
bool getCoincapInfoFromSymbol(const wxString& symbol, wxString& out_id, double& price_usd, wxString& output);
bool getCoincapAssetHistory(
    const wxString& asset_id, wxDateTime begin_date,
    std::map<wxDateTime, double> &historical_rates, wxString &msg
);

wxString cleanseNumberString(const wxString& str, const bool decimal);
double cleanseNumberStringToDouble(const wxString& str, const bool decimal);
const wxString mmPlatformType();

//All components version in TXT, HTML, ABOUT
const wxString getProgramDescription(const int type = 0);
void DoWindowsFreezeThaw(wxWindow* w);
const wxString md2html(const wxString& md);
const wxString getVFname4print(const wxString& name, const wxString& data);
void clearVFprintedFiles(const wxString& name);
const wxRect GetDefaultMonitorRect();

//* Date Functions----------------------------------------------------------*//

const wxDateTime getUserDefinedFinancialYear(bool prevDayRequired = false);
const std::map<wxString, wxString> &date_formats_regex();
const wxString mmGetDateTimeForDisplay(const wxString &datetime_iso, const wxString& format = Option::instance().getDateFormat());
const wxString mmGetDateForDisplay(const wxString &datetime_iso, const wxString& format = Option::instance().getDateFormat());
const wxString mmGetTimeForDisplay(const wxString& datetime_iso);
bool mmParseDisplayStringToDate(wxDateTime& date, const wxString& sDate, const wxString& sDateMask);
extern const std::vector<std::pair<wxString, wxString> > g_date_formats_map();
extern const std::map<int, std::pair<wxConvAuto, wxString> > g_encoding;

//----------------------------------------------------------------------------

CURLcode http_get_data(const wxString& site, wxString& output, const wxString& useragent = wxEmptyString);
CURLcode http_post_data(const wxString& site, const wxString& data, const wxString& contentType, wxString& output);
CURLcode http_download_file(const wxString& site, const wxString& path);
CURLcode getYahooFinanceQuotes(const wxString& URL, wxString& json_data);

//----------------------------------------------------------------------------

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
    std::vector<std::pair<wxString, wxString> > m_date_formats_temp;

    //Numbers of successfully applied Format Specifier for every handled date string
    std::map<wxString, int> m_date_parsing_stat;
    wxDateTime m_today;
    wxDateTime m_month_ago;
    wxString m_date_mask; //Human readable date format like DD/MM/YYYY
    wxString m_date_format; //Date Format Specifier like %d/%m/%Y
    int m_error_count = 0;
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

// used where differences occur between platforms
wxImageList* createImageList(const int size = 0);

void mmToolTip(wxWindow* widget,const wxString& tip);

// escape HTML characters
wxString HTMLEncode(const wxString& input);

void mmSetSize(wxWindow* w);
void mmFontSize(wxWindow* widget);

class mmHtmlWindow : public wxHtmlWindow
{
public:
    mmHtmlWindow(
        wxWindow *parent,
        wxWindowID id=wxID_ANY,
        const wxPoint &pos=wxDefaultPosition,
        const wxSize &size=wxDefaultSize,
        long style=wxHW_DEFAULT_STYLE,
        const wxString &name="htmlWindow"
    );
private:
    void OnMouseRightClick(wxMouseEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
};
