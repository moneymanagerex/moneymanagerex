/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)

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

typedef wxLongLong int64;

/*
   mmOptions caches the options for MMEX
   so that we don't hit the DB that often
   for data.
*/
class Option
{
public:
    enum USAGE_TYPE { NONE = 0, LASTUSED, UNUSED, DEFAULT };
    enum THEME_MODE { AUTO = 0, LIGHT, DARK };
    enum COMPOUNDING_ID {
        COMPOUNDING_ID_DAY = 0,
        COMPOUNDING_ID_WEEK,
        COMPOUNDING_ID_MONTH,
        COMPOUNDING_ID_YEAR,
        COMPOUNDING_ID_size
    };
    static const std::vector<std::pair<COMPOUNDING_ID, wxString> > COMPOUNDING_NAME;
    static const std::vector<std::pair<COMPOUNDING_ID, int> > COMPOUNDING_N;

public:
    Option();
    static Option& instance();
    void LoadOptions(const bool include_infotable = true);

    // set and save the option: m_dateFormat
    void setDateFormat(const wxString& date_format);
    const wxString getDateFormat() const;

    // set and save the option: m_language
    wxLanguage getLanguageID(const bool get_db = false);
    // get 2-letter ISO 639-1 code
    const wxString getLanguageCode(const bool get_db = false);
    void setLanguage(const wxLanguage& language);

    // set and save the option: m_userNameString
    void UserName(const wxString& username);
    const wxString& UserName() const;

    // set and save the option: m_localeNameString
    void LocaleName(const wxString& locale);
    const wxString& LocaleName() const;

    // set and save the option: m_financialYearStartDayString
    void FinancialYearStartDay(const wxString& setting);
    const wxString& FinancialYearStartDay() const;

    // set and save the option: m_financialYearStartMonthString
    void FinancialYearStartMonth(const wxString& setting);
    const wxString& FinancialYearStartMonth() const;

    // set the base currency ID
    void setBaseCurrency(const int64 base_currency_id);
    // returns the base currency ID
    int64 getBaseCurrencyID() const noexcept;

    // set and save the option: m_databaseUpdated
    void DatabaseUpdated(const bool value);
    bool DatabaseUpdated() const noexcept;

    void HideShareAccounts(const bool value);
    bool HideShareAccounts() const noexcept;

    void HideDeletedTransactions(const bool value);
    bool HideDeletedTransactions() const noexcept;

    void BudgetFinancialYears(const bool value);
    bool BudgetFinancialYears() const noexcept;

    void BudgetIncludeTransfers(const bool value);
    bool BudgetIncludeTransfers() const noexcept;

    void BudgetReportWithSummaries(const bool value);
    bool BudgetReportWithSummaries() const noexcept;

    void BudgetOverride(const bool value);
    bool BudgetOverride() const noexcept;

    bool UseTransDateTime(const bool value);
    bool UseTransDateTime() const noexcept;

    // Deduct monthly budget from yearly budget
    void BudgetDeductMonthly(const bool value);
    bool BudgetDeductMonthly() const noexcept;

    void TransPayeeSelection(const int value);
    int TransPayeeSelection() const noexcept;

    void TransCategorySelectionNonTransfer(const int value);
    int TransCategorySelectionNonTransfer() const noexcept;

    void TransCategorySelectionTransfer(const int value);
    int TransCategorySelectionTransfer() const noexcept;

    void set_bulk_transactions(const bool value);
    bool get_bulk_transactions() const noexcept;

    void TransStatusReconciled(const int value);
    int TransStatusReconciled() const noexcept;

    void TransDateDefault(const int value);
    int TransDateDefault() const noexcept;

    void SendUsageStatistics(const bool value);
    bool SendUsageStatistics() const noexcept;

    void CheckNewsOnStartup(const bool value);
    bool CheckNewsOnStartup() const noexcept;

    void SharePrecision(const int value);
    int SharePrecision() const noexcept;

    void AssetCompounding(const int value);
    int AssetCompounding() const noexcept;

    // Allows a year or financial year to start before or after the 1st of the month.
    void setBudgetDaysOffset(const int value);
    int getBudgetDaysOffset() const noexcept;
    /**Re-adjust date by the date offset value*/
    void setBudgetDateOffset(wxDateTime& date) const;

    // Allows the 'first day' in the month to be adjusted for reporting purposes
    void setReportingFirstDay(const int value);
    int getReportingFirstDay() const noexcept;

    /* stored value in percantage for scale html font and other objects */
    void setHTMLFontSizes(const int value);
    int getHtmlFontSize() const noexcept;

    void setThemeMode(const int value);
    int getThemeMode() const noexcept;

    void setFontSize(const int value);
    int getFontSize() const noexcept;

    void setIconSize(const int value);
    void setToolbarIconSize(const int value);
    void setNavigationIconSize(const int value);

    int getIconSize() const noexcept;
    int getNavigationIconSize() const noexcept;
    int getToolbarIconSize() const noexcept;

    int AccountImageId(const int64 account_id, const bool def, const bool ignoreClosure = false);
    bool getSendUsageStatistics() const noexcept;

    void IgnoreFutureTransactions(const bool value);
    bool getIgnoreFutureTransactions() const noexcept;

    void ShowToolTips(const bool value);
    bool getShowToolTips() const noexcept;

    void ShowMoneyTips(const bool value);
    bool getShowMoneyTips() const noexcept;

    void CurrencyHistoryEnabled(const bool value);
    bool getCurrencyHistoryEnabled() const noexcept;

    // Homepage income vs expenses graph range
    void setHomePageIncExpRange(const int value);
    int getHomePageIncExpRange() const noexcept;

private:
    wxString m_dateFormat;
    wxLanguage m_language = wxLANGUAGE_UNKNOWN;
    wxString m_userNameString;
    wxString m_localeNameString;
    wxString m_financialYearStartDayString;
    wxString m_financialYearStartMonthString;
    int64 m_baseCurrency = -1;
    bool m_currencyHistoryEnabled = false;
    bool m_bulk_enter = false;

    bool m_databaseUpdated = false;
    bool m_hideShareAccounts = true;                //INIDB_HIDE_SHARE_ACCOUNTS
    bool m_hideDeletedTransactions = false;         //INIDB_HIDE_DELETED_TRANSACTIONS
    bool m_budgetFinancialYears = false;            //INIDB_BUDGET_FINANCIAL_YEARS
    bool m_budgetIncludeTransfers = false;          //INIDB_BUDGET_INCLUDE_TRANSFERS
    bool m_budgetReportWithSummaries = true;        //INIDB_BUDGET_SUMMARY_WITHOUT_CATEG
    bool m_budgetOverride = false;                  //INIDB_BUDGET_OVERRIDE
    bool m_budgetDeductMonthly = false;             //INIDB_BUDGET_DEDUCT_MONTH_FROM_YEAR
    bool m_ignoreFutureTransactions = false;        //INIDB_IGNORE_FUTURE_TRANSACTIONS
    bool m_showToolTips = true;                     //INIDB_SHOW_TOOLTIPS
    bool m_showMoneyTips = true;                    //INIDB_SHOW_MONEYTIPS
    bool m_useTransDateTime = false;

    int m_transPayeeSelection = Option::NONE;
    int m_transCategorySelectionNonTransfer = Option::NONE;
    int m_transCategorySelectionTransfer = Option::NONE;
    int m_transStatusReconciled = Option::NONE;
    int m_transDateDefault = 0;
    bool m_usageStatistics = true;
    bool m_newsChecking = true;                    //INIDB_CHECK_NEWS
    int m_sharePrecision = 4;
    int m_assetCompounding = Option::COMPOUNDING_ID_DAY;

    int m_theme_mode = Option::AUTO;
    int m_html_font_size = 100;
    int m_ico_size = 16;
    int m_font_size = 0;
    int m_toolbar_ico_size = 32;
    int m_navigation_ico_size = 24;

    int m_budget_days_offset = 0;
    int m_reporting_firstday = 1;

    int m_homepage_incexp_range = 0;
};

inline int Option::getIconSize() const noexcept
{
    return m_ico_size;
}
inline int Option::getNavigationIconSize() const noexcept
{
    return m_navigation_ico_size;
}
inline int Option::getToolbarIconSize() const noexcept{
    return m_toolbar_ico_size;
}
inline const wxString& Option::LocaleName() const { return m_localeNameString; }
inline const wxString& Option::UserName() const { return m_userNameString; }
inline const wxString& Option::FinancialYearStartDay() const { return m_financialYearStartDayString; }
inline int Option::TransCategorySelectionNonTransfer() const noexcept
{
    return m_transCategorySelectionNonTransfer;
}
inline int Option::TransCategorySelectionTransfer() const noexcept
{
    return m_transCategorySelectionTransfer;
}
inline bool Option::get_bulk_transactions() const noexcept
{
    return m_bulk_enter;
}
inline int Option::getThemeMode() const noexcept
{
    return m_theme_mode;
}
inline int Option::getFontSize() const noexcept
{
    return m_font_size;
}

inline const wxString Option::getDateFormat() const
{
    return m_dateFormat;
}

inline bool Option::getSendUsageStatistics() const noexcept
{
#ifdef _DEBUG
    return false;
#else
    return m_usageStatistics;
#endif
}

inline bool Option::getCurrencyHistoryEnabled() const noexcept
{
    return m_currencyHistoryEnabled;
}

inline bool Option::getIgnoreFutureTransactions() const noexcept
{
    return m_ignoreFutureTransactions;
}

inline bool Option::getShowToolTips() const noexcept
{
    return m_showToolTips;
}

inline bool Option::getShowMoneyTips() const noexcept
{
    return m_showMoneyTips;
}

inline int Option::getBudgetDaysOffset() const noexcept
{
    return m_budget_days_offset;
}

inline int Option::getReportingFirstDay() const noexcept
{
    return m_reporting_firstday;
}

inline int Option::getHomePageIncExpRange() const noexcept
{
    return m_homepage_incexp_range;
}
