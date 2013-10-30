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

#ifndef _MM_EX_UTIL_H_
#define _MM_EX_UTIL_H_

#include "defs.h"
#include "reports/reportbase.h"
#include <wx/valnum.h>
#include <map>

struct ValuePair
{
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
    mmTreeItemData(const wxString& string, mmPrintableBase* report = 0)
        : id_(0)
        , isString_(true)
        , isBudgetingNode_(false)
        , stringData_(string)
        , report_(report)
    {}
    ~mmTreeItemData()
    {
        if (report_) delete report_;
    }

    int getData() const
    {
        return id_;
    }
    wxString getString() const
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

void mmShowErrorMessageInvalid( wxWindow *parent, const wxString &message );
void mmShowErrorMessage( wxWindow *parent, const wxString &message, const wxString &messageheader );

wxString mmSelectLanguage(wxWindow *parent, bool forced_show_dlg, bool save_setting = true);

wxDateTime mmGetStorageStringAsDate( const wxString& str );
wxString mmGetDateForDisplay( const wxDateTime &dt );
bool mmParseDisplayStringToDate(wxDateTime& date, const wxString sDate, wxString sDateMask );
wxString mmGetNiceDateSimpleString( const wxDateTime &dt );
std::map<wxString,wxString> date_formats_map();

wxString inQuotes(wxString label, wxString& delimiter);
wxString adjustedExportAmount(const wxString& amtSeparator, const wxString& strValue);

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

wxString Tips(wxString type);
int site_content(const wxString& site, wxString& output);

wxDateTime getUserDefinedFinancialYear(bool prevDayRequired = false);

wxImageList* navtree_images_list_();

wxSharedPtr<wxSQLite3Database> static_db_ptr();

#endif // _MM_EX_UTIL_H_
//----------------------------------------------------------------------------
