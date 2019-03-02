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
    void setDateFormat(const wxString& datefornat);
    const wxString getDateFormat() const;

    // set and save the option: m_language
    void setLanguage(wxLanguage& language);
    wxLanguage getLanguageID(bool get_db = false);
    // get 2-letter ISO 639-1 code
    const wxString getLanguageISO6391(bool get_db = false);

    // set and save the option: m_userNameString
    void setUserName(const wxString& username);
    const wxString getUserName() const;

    // set and save the option: m_financialYearStartDayString
    void setFinancialYearStartDay(const wxString& setting);
    const wxString getFinancialYearStartDay() const;

    // set and save the option: m_financialYearStartMonthString
    void setFinancialYearStartMonth(const wxString& setting);
    const wxString getFinancialYearStartMonth() const;

    // set the base currency ID
    void setBaseCurrencyID(int base_currency_id);
    // returns the base currency ID
    int getBaseCurrencyID() const;

    // set and save the option: m_databaseUpdated
    void setDatabaseUpdated(bool value);
    bool getDatabaseUpdated() const;

    void setBudgetFinancialYears(bool value);
    bool getBudgetFinancialYears() const;

    void setBudgetIncludeTransfers(bool value);
    bool getBudgetIncludeTransfers() const;

    void setBudgetSetupWithoutSummaries(bool value);
    bool getBudgetSetupWithoutSummaries() const;

    void setBudgetReportWithSummaries(bool value);
    bool getBudgetReportWithSummaries() const;

    void setIgnoreFutureTransactions(bool value);
    bool getIgnoreFutureTransactions() const;

    void setTransPayeeSelection(int value);
    int getTransPayeeSelection() const;

    void setTransCategorySelection(int value);
    int getTransCategorySelection() const;

    void setTransStatusReconciled(int value);
    int getTransStatusReconciled() const;

    void setTransDateDefault(int value);
    int getTransDateDefault() const;

    void setSendUsageStatistics(bool value);
    bool getSendUsageStatistics() const;

    void setSharePrecision(int value);
    int getSharePrecision() const;

    /* stored value in percentage for scale html font and other objects */
    void setHtmlFontSize(int value);
    int getHtmlFontSize() const;

    // Allows a year or financial year to start before or after the 1st of the month.
    void setBudgetDaysOffset(int value);
    int getBudgetDaysOffset() const;
    /**Re-adjust date by the date offset value*/
    void setBudgetDateOffset(wxDateTime& date) const;

    void setIconSize(int value);
    int getIconSize() const;

    int getAccountImageId(int account_id, bool def = false) const;

    void setHideReport(int report, bool value);
    bool getHideReport(int report) const;
    int getReportCount() const;
    const wxString getReportFullName(int report) const;
    const wxString getReportGroup(int report) const;
    const wxString getReportName(int report) const;
    bool getBudgetReport(int report) const;
    mmPrintableBase* getReportFunction(int report) const;

private:
    bool isReportIDCorrect(int report) const;
    int m_report_count;
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

    const wxString ReportSettings(int id) const;
};

inline const wxString Option::getUserName() const
{
    return m_userNameString;
}

inline const wxString Option::getFinancialYearStartDay() const
{
    return m_financialYearStartDayString;
}

inline const wxString Option::getFinancialYearStartMonth() const
{
    return m_financialYearStartMonthString;
}

inline int Option::getBaseCurrencyID() const
{
    return m_baseCurrency;
}

inline bool Option::getDatabaseUpdated() const
{
    return m_databaseUpdated;
}

inline const wxString Option::getDateFormat() const
{
    return m_dateFormat;
}

inline bool Option::getBudgetFinancialYears() const
{
    return m_budgetFinancialYears;
}

inline bool Option::getBudgetIncludeTransfers() const
{
    return m_budgetIncludeTransfers;
}

inline bool Option::getBudgetSetupWithoutSummaries() const
{
    return m_budgetSetupWithoutSummaries;
}

inline bool Option::getBudgetReportWithSummaries() const
{
    return m_budgetReportWithSummaries;
}

inline bool Option::getIgnoreFutureTransactions() const
{
    return m_ignoreFutureTransactions;
}

inline int Option::getTransPayeeSelection() const
{
    return m_transPayeeSelection;
}

inline int Option::getTransCategorySelection() const
{
    return m_transCategorySelection;
}

inline int Option::getTransStatusReconciled() const
{
    return m_transStatusReconciled;
}

inline int Option::getTransDateDefault() const
{
    return m_transDateDefault;
}

inline int Option::getSharePrecision() const
{
    return m_sharePrecision;
}

inline bool Option::getSendUsageStatistics() const
{
    return m_usageStatistics;
}

inline int Option::getHtmlFontSize() const
{
    return m_html_font_size;
}

inline int Option::getBudgetDaysOffset() const
{
    return m_budget_days_offset;
}

inline int Option::getIconSize() const
{
    return m_ico_size;
}

inline int Option::getReportCount() const
{
    return m_report_count;
}

inline bool Option::isReportIDCorrect(int report) const
{
    return (report >= 0) && (report < getReportCount());
}

#endif // MM_EX_OPTION_H_
//----------------------------------------------------------------------------
