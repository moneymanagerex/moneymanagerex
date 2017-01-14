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

#ifndef MM_EX_UTIL_H_
#define MM_EX_UTIL_H_

#include "defs.h"
#include "reports/reportbase.h"
#include <wx/valnum.h>
#include <map>

class mmGUIApp;
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

class mmListBoxItem: public wxClientData
{
public:
    mmListBoxItem(int index, const wxString& name)
        : index_(index), name_(name)
    {}

    int getIndex() const
    {
        return index_;
    }
    wxString getName() const
    {
        return name_;
    }

private:
    int index_;
    wxString name_;
};

class mmTreeItemData : public wxTreeItemData
{
public:
    mmTreeItemData(int id, bool isBudget)
        : id_(id)
        , isString_(false)
        , isBudgetingNode_(isBudget)
        , report_(0)
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
        , stringData_("report@" + report->title())
        , report_(report)
    {}
    mmTreeItemData(const wxString& string)
        : id_(0)
        , isString_(true)
        , isBudgetingNode_(false)
        , stringData_("item@" + string)
        , report_(0)
    {}
    ~mmTreeItemData()
    {
        if (report_) delete report_;
    }

    int getData() const
    {
        return id_;
    }
    const wxString getString() const
    {
        return stringData_;
    }
    mmPrintableBase* get_report() const
    {
        return report_;
    }
    bool isStringData() const
    {
        return isString_;
    }
    bool isBudgetingNode() const
    {
        return isBudgetingNode_;
    }

private:
    int id_;
    bool isString_;
    bool isBudgetingNode_;
    wxString stringData_;
    mmPrintableBase* report_;
};

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

const bool getNewsRSS(std::vector<WebsiteNews>& WebsiteNewsList);
int site_content(const wxString& site, wxString& output);
bool download_file(const wxString& site, const wxString& path);
const wxString getURL(const wxString& file);

const wxString mmPlatformType();
void windowsFreezeThaw(wxWindow* w);

//* Date Functions----------------------------------------------------------*//
const wxDateTime getUserDefinedFinancialYear(bool prevDayRequired = false);
const std::unordered_map<wxString, wxString> &date_formats_regex();
const wxDateTime mmParseISODate(const wxString& str);
const wxString mmGetDateForDisplay(const wxString &iso_date);
bool mmParseDisplayStringToDate(wxDateTime& date, const wxString& sDate, const wxString& sDateMask);
const wxString mmGetNiceDateSimpleString(const wxDateTime &dt);
extern const std::unordered_map<wxString, wxString> g_date_formats_map;
extern const std::map<int, std::pair<wxConvAuto, wxString> > g_encoding;

#endif // MM_EX_UTIL_H_
//----------------------------------------------------------------------------
