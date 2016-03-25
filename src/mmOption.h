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

#ifndef MM_EX_OPTION_H_
#define MM_EX_OPTION_H_

#include "defs.h"

/*
   mmOptions caches the options for MMEX
   so that we don't hit the DB that often
   for data.
*/
class mmOptions
{
public:
    mmOptions();
    static mmOptions& instance();
    void LoadInfotableOptions();

    // set and save the option: m_dateFormat
    void DateFormat(const wxString& datefornat);
    wxString DateFormat();

    // set and save the option: m_language
    void Language(wxString& language);
    wxString Language();

    // set and save the option: m_userNameString
    void UserName(const wxString& username);
    wxString UserName();

    // set and save the option: m_financialYearStartDayString
    void FinancialYearStartDay(const wxString& setting);
    wxString FinancialYearStartDay();

    // set and save the option: m_financialYearStartMonthString
    void FinancialYearStartMonth(const wxString& setting);
    wxString FinancialYearStartMonth();

    // set and save the option: m_databaseUpdated
    void DatabaseUpdated(bool value);
    bool DatabaseUpdated();

private:
    wxString m_dateFormat;
    wxString m_language;
    wxString m_userNameString;

    wxString m_financialYearStartDayString;
    wxString m_financialYearStartMonthString;
    bool m_databaseUpdated;
};

class mmIniOptions
{
public:
    mmIniOptions();
    static mmIniOptions& instance();
    void loadOptions();

    const int account_image_id(int account_id, bool def = false);
    int html_font_size_;
    int ico_size_;

    bool budgetFinancialYears_;
    bool budgetIncludeTransfers_;
    bool budgetSetupWithoutSummaries_;
    bool budgetReportWithSummaries_;
    bool ignoreFutureTransactions_;

    int transPayeeSelectionNone_;
    int transCategorySelectionNone_;
    int transStatusReconciled_;
    int transDateDefault_;
};

#endif // MM_EX_OPTION_H_
//----------------------------------------------------------------------------
