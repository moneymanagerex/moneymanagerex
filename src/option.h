/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016 - 2017 Stefano Giorgio [stef145g]
 Copyright (C) 2017 James Higley

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
#include <vector>

class mmPrintableBase;

/*
   mmOptions caches the options for MMEX
   so that we don't hit the DB that often
   for data.
*/
class Option
{
public:
    enum USAGE_TYPE { NONE = 0, LASTUSED, UNUSED };

public:
    Option();
    static Option& instance();
    void LoadOptions(bool include_infotable = true);

    // set and save the option: m_dateFormat
    void DateFormat(const wxString& datefornat);
    wxString DateFormat();

    // set and save the option: m_language
    void Language(wxLanguage& language);
    wxLanguage Language(bool get_db = false);
    // get 2-letter ISO 639-1 code
    wxString LanguageISO6391(bool get_db = false);

    // set and save the option: m_userNameString
    void UserName(const wxString& username);
    wxString UserName();

    // set and save the option: m_financialYearStartDayString
    void FinancialYearStartDay(const wxString& setting);
    wxString FinancialYearStartDay();

    // set and save the option: m_financialYearStartMonthString
    void FinancialYearStartMonth(const wxString& setting);
    wxString FinancialYearStartMonth();

    // set the base currency ID
    void BaseCurrency(int base_currency_id);
    // returns the base currency ID
    int BaseCurrency();

    // set and save the option: m_databaseUpdated
    void DatabaseUpdated(bool value);
    bool DatabaseUpdated();

    void BudgetFinancialYears(bool value);
    bool BudgetFinancialYears();

    void BudgetIncludeTransfers(bool value);
    bool BudgetIncludeTransfers();
    
    void BudgetSetupWithoutSummaries(bool value);
    bool BudgetSetupWithoutSummaries();

    void BudgetReportWithSummaries(bool value);
    bool BudgetReportWithSummaries();

    void IgnoreFutureTransactions(bool value);
    bool IgnoreFutureTransactions();

    void TransPayeeSelection(int value);
    int TransPayeeSelection();

    void TransCategorySelection(int value);
    int TransCategorySelection();

    void TransStatusReconciled(int value);
    int TransStatusReconciled();

    void TransDateDefault(int value);
    int TransDateDefault();

    void SendUsageStatistics(bool value);
    bool SendUsageStatistics();

    void SharePrecision(int value);
    int SharePrecision();

    /* stored value in percentage for scale html font and other objects */
    void HtmlFontSize(int value);
    int HtmlFontSize();

    // Allows a year or financial year to start before or after the 1st of the month.
    void BudgetDaysOffset(int value);
    int BudgetDaysOffset();
    /**Re-adjust date by the date offset value*/
    void BudgetDateOffset(wxDateTime& date);

    void IconSize(int value);
    int IconSize();

    int AccountImageId(int account_id, bool def = false);

    void HideReport(int report, bool value);
    bool HideReport(int report);
    int ReportCount();
    wxString ReportFullName(int report);
    wxString ReportGroup(int report);
    wxString ReportName(int report);
    bool BudgetReport(int report);
    mmPrintableBase* ReportFunction(int report);

private:
    wxString m_dateFormat;
    wxLanguage m_language;
    wxString m_userNameString;
    wxString m_financialYearStartDayString;
    wxString m_financialYearStartMonthString;
    int m_baseCurrency;

    bool m_databaseUpdated;
    bool m_budgetFinancialYears;            //INIDB_BUDGET_FINANCIAL_YEARS
    bool m_budgetIncludeTransfers;          //INIDB_BUDGET_INCLUDE_TRANSFERS
    bool m_budgetSetupWithoutSummaries;     //INIDB_BUDGET_SETUP_WITHOUT_SUMMARY
    bool m_budgetReportWithSummaries;       //INIDB_BUDGET_SUMMARY_WITHOUT_CATEG
    bool m_ignoreFutureTransactions;        //INIDB_IGNORE_FUTURE_TRANSACTIONS

    int m_transPayeeSelection;
    int m_transCategorySelection;
    int m_transStatusReconciled;
    int m_transDateDefault;
    bool m_usageStatistics;
    int m_sharePrecision;

    int m_html_font_size;
    int m_ico_size;
    int m_budget_days_offset;
    int m_hideReport;
    struct ReportInfo;
    std::vector<Option::ReportInfo> m_reports;

    const wxString ReportSettings(int id);
};

#endif // MM_EX_OPTION_H_
//----------------------------------------------------------------------------
