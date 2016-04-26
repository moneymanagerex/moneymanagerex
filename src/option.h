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
class Option
{
public:
    Option();
    static Option& instance();
    void LoadOptions(bool include_infotable = true);

    // set and save the option: m_dateFormat
    void DateFormat(const wxString& datefornat);
    wxString DateFormat();

    // set and save the option: m_language
    void Language(wxString& language);
    wxString Language(bool get_db = false);

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

    void TransPayeeSelectionNone(int value);
    int TransPayeeSelectionNone();

    void TransCategorySelectionNone(int value);
    int TransCategorySelectionNone();

    void TransStatusReconciled(int value);
    int TransStatusReconciled();

    void TransDateDefault(int value);
    int TransDateDefault();

    /* stored value in percantage for scale html font and other objects */
    void HtmlFontSize(int value);
    int HtmlFontSize();

    void IconSize(int value);
    int IconSize();

    const int AccountImageId(int account_id, bool def = false);

private:
    wxString m_dateFormat;
    wxString m_language;
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

    int m_transPayeeSelectionNone;
    int m_transCategorySelectionNone;
    int m_transStatusReconciled;
    int m_transDateDefault;

    int m_html_font_size;
    int m_ico_size;
};

#endif // MM_EX_OPTION_H_
//----------------------------------------------------------------------------
