/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016 - 2017 Stefano Giorgio [stef145g]
 Copyright (C) 2017 James Higley
 Copyright (C) 2019 Nikolay Akimov

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
    void setLanguageID(wxLanguage& language);
    wxLanguage getLanguage() const;
    // get 2-letter ISO 639-1 code plus Region
    const wxString getBestTranslation() const;

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
    void setBaseCurrency(int base_currency_id);
    // returns the base currency ID
    int getBaseCurrency() const;

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
    int getTransCategorySelection();

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
    void setBudgetDateOffset(wxDateTime& date);

    int getIconSize() const;
    int getAccountImageId(int accountID, bool def = false) const;

    void setHideReport(int reportID, bool value);
    bool getHideReport(int reportID) const;
    int getReportCount() const;
    const wxString getReportFullName(int reportID) const;
    const wxString getReportGroup(int reportID);
    const wxString getReportName(int reportID) const;
    bool getBudgetReport(int reportID) const;
    mmPrintableBase* getReportFunction(int reportID) const;

private:
    bool getReportIndexIsOK(int reportID) const;
    void setIconSize(int html_font_size);

    wxString m_dateFormat;
    wxLanguage m_language;
    wxString m_bestTranslation;
    wxString m_userNameString;
    wxString m_financialYearStartDayString;
    wxString m_financialYearStartMonthString;
    int m_baseCurrency;
    int m_reportsSize;

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

    const wxString getReportSettingsJSON(int id) const;
};

inline wxLanguage Option::getLanguage() const
{
    return m_language;
}

inline const wxString Option::getBestTranslation() const
{
    return m_bestTranslation;
}

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

inline int Option::getBaseCurrency() const
{
    return m_baseCurrency;
}

inline bool Option::getDatabaseUpdated() const
{
    return m_databaseUpdated;
}

inline bool Option::getBudgetReportWithSummaries() const
{
    return m_budgetReportWithSummaries;
}

inline bool Option::getBudgetIncludeTransfers() const
{
    return m_budgetIncludeTransfers;
}

inline bool Option::getBudgetSetupWithoutSummaries() const
{
    return m_budgetSetupWithoutSummaries;
}

inline bool Option::getBudgetFinancialYears() const
{
    return m_budgetFinancialYears;
}

inline const wxString Option::getDateFormat() const
{
    return m_dateFormat;
}

inline int Option::getIconSize() const
{
    return m_ico_size;
}

inline int Option::getBudgetDaysOffset() const
{
    return m_budget_days_offset;
}

inline int Option::getHtmlFontSize() const
{
    return m_html_font_size;
}

inline int Option::getTransPayeeSelection() const
{
    return m_transPayeeSelection;
}

inline bool Option::getIgnoreFutureTransactions() const
{
    return m_ignoreFutureTransactions;
}

inline int Option::getTransStatusReconciled() const
{
    return m_transStatusReconciled;
}

inline bool Option::getSendUsageStatistics() const
{
    return m_usageStatistics;
}


inline int Option::getTransDateDefault() const
{
    return m_transDateDefault;
}


inline int Option::getSharePrecision() const
{
    return m_sharePrecision;
}

inline bool Option::getReportIndexIsOK(int index) const
{
    return (index >= 0) && (index < getReportCount());
}

inline int Option::getReportCount() const
{
    return m_reportsSize;
}
    
#endif // MM_EX_OPTION_H_
//----------------------------------------------------------------------------
