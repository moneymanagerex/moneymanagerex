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

void mmShowErrorMessageInvalid(wxWindow *parent, const wxString &message);
void mmShowErrorMessage(wxWindow *parent, const wxString &message, const wxString &messageheader);
void mmShowWarningMessage(wxWindow *parent, const wxString &message, const wxString &messageheader);
void mmMessageCategoryInvalid(wxWindow *button);
void mmMessageAccountInvalid(wxWindow *object, bool transfer = false);
void mmMessageFileInvalid(wxWindow *object, bool open = false);
void mmMessagePayeeInvalid(wxWindow *object);
void mmMessageNameInvalid(wxTextCtrl *textBox);

const wxString mmSelectLanguage(mmGUIApp *app, wxWindow* window, bool forced_show_dlg, bool save_setting = true);

const wxDateTime mmGetStorageStringAsDate(const wxString& str);
const wxString mmGetDateForDisplay(const wxDateTime &dt);
bool mmParseDisplayStringToDate(wxDateTime& date, const wxString& sDate, const wxString& sDateMask);
const wxString mmGetNiceDateSimpleString(const wxDateTime &dt);
const std::map<wxString,wxString> date_formats_map();
const std::map<wxString,wxString> date_formats_regex();

const wxString inQuotes(const wxString& label, const wxString& delimiter);

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

int site_content(const wxString& site, wxString& output);

const wxDateTime getUserDefinedFinancialYear(bool prevDayRequired = false);

const wxString mmPlatformType();
const wxString getURL(const wxString& file);
const bool IsUpdateAvailable(const bool& bSilent, wxString& NewVersion);

/*  Checks for new updates and return message boxes.
    Set bSilent to false to don't show messages for version already updated */
void checkUpdates(const bool& bSilent);

#endif // MM_EX_UTIL_H_
//----------------------------------------------------------------------------
