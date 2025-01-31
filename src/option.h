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
    void load(const bool include_infotable = true);

    // m_database_updated
    void setDatabaseUpdated(const bool value);
    bool getDatabaseUpdated() const noexcept;

    // m_language
    wxLanguage getLanguageID(const bool get_db = false);
    // get 2-letter ISO 639-1 code
    const wxString getLanguageCode(const bool get_db = false);
    void setLanguage(const wxLanguage& language);

    // m_locale_name
    void setLocaleName(const wxString& locale);
    const wxString& getLocaleName() const;

    // m_date_format
    void loadDateFormat();
    void setDateFormat(const wxString& date_format);
    const wxString getDateFormat() const;

    // m_user_name
    void loadUserName();
    void setUserName(const wxString& username);
    const wxString& getUserName() const;

    // m_base_currency_id
    void loadBaseCurrencyID();
    void setBaseCurrencyID(const int64 base_currency_id);
    int64 getBaseCurrencyID() const noexcept;

    // m_use_currency_history
    void loadUseCurrencyHistory();
    void setUseCurrencyHistory(const bool value);
    bool getUseCurrencyHistory() const noexcept;

    // m_share_precision
    void loadSharePrecision();
    void setSharePrecision(const int value);
    int getSharePrecision() const noexcept;

    // m_asset_compounding
    void loadAssetCompounding();
    void setAssetCompounding(const int value);
    int getAssetCompounding() const noexcept;

    // m_reporting_first_day: allows the 'first day' in the month to be adjusted
    void loadReportingFirstDay();
    void setReportingFirstDay(const int value);
    int getReportingFirstDay() const noexcept;

    // m_reporting_first_weekday
    void loadReportingFirstWeekday();
    void setReportingFirstWeekday(const wxDateTime::WeekDay value);
    wxDateTime::WeekDay getReportingFirstWeekday() const noexcept;

    // m_financial_first_day
    void loadFinancialFirstDay();
    void setFinancialFirstDay(int value);
    int getFinancialFirstDay() const;

    // m_financial_first_month
    void loadFinancialFirstMonth();
    void setFinancialFirstMonth(const wxDateTime::Month value);
    wxDateTime::Month getFinancialFirstMonth() const;

    // m_budget_days_offset: allows a year to start before or after the 1st of the month.
    void loadBudgetDaysOffset();
    void setBudgetDaysOffset(const int value);
    int getBudgetDaysOffset() const noexcept;
    void addBudgetDateOffset(wxDateTime& date) const;

    // m_homepage_incexp_range: homepage income vs expenses graph range
    void loadHomePageIncExpRange();
    void setHomePageIncExpRange(const int value);
    int getHomePageIncExpRange() const noexcept;

    // m_hide_share_accounts
    void loadHideShareAccounts();
    void setHideShareAccounts(const bool value);
    bool getHideShareAccounts() const noexcept;

    // m_hide_deleted_transactions
    void loadHideDeletedTransactions();
    void setHideDeletedTransactions(const bool value);
    bool getHideDeletedTransactions() const noexcept;

    // m_budget_financial_years
    void loadBudgetFinancialYears();
    void setBudgetFinancialYears(const bool value);
    bool getBudgetFinancialYears() const noexcept;

    // m_budget_include_transfers
    void loadBudgetIncludeTransfers();
    void setBudgetIncludeTransfers(const bool value);
    bool getBudgetIncludeTransfers() const noexcept;

    // m_budget_summary_without_categories
    void loadBudgetSummaryWithoutCategories();
    void setBudgetSummaryWithoutCategories(const bool value);
    bool getBudgetSummaryWithoutCategories() const noexcept;

    // m_budget_override
    void loadBudgetOverride();
    void setBudgetOverride(const bool value);
    bool getBudgetOverride() const noexcept;

    // m_use_trans_datetime
    void loadUseTransDateTime();
    bool UseTransDateTime(const bool value);
    bool UseTransDateTime() const noexcept;

    // m_budget_deduct_monthly: Deduct monthly budget from yearly budget
    void loadBudgetDeductMonthly();
    void setBudgetDeductMonthly(const bool value);
    bool getBudgetDeductMonthly() const noexcept;

    // m_trans_payee_none
    void loadTransPayeeNone();
    void setTransPayeeNone(const int value);
    int getTransPayeeNone() const noexcept;

    // m_trans_category_none
    void loadTransCategoryNone();
    void setTransCategoryNone(const int value);
    int getTransCategoryNone() const noexcept;

    // m_trans_category_transfer_none
    void loadTransCategoryTransferNone();
    void setTransCategoryTransferNone(const int value);
    int getTransCategoryTransferNone() const noexcept;

    // m_bulk_transactions
    void loadBulkTransactions();
    void setBulkTransactions(const bool value);
    bool getBulkTransactions() const noexcept;

    // m_trans_status_reconciled
    void loadTransStatusReconciled();
    void setTransStatusReconciled(const int value);
    int getTransStatusReconciled() const noexcept;

    // m_trans_date_default
    void loadTransDateDefault();
    void setTransDateDefault(const int value);
    int getTransDateDefault() const noexcept;

    // m_send_usage_stats
    void loadSendUsageStats();
    void setSendUsageStats(const bool value);
    bool getSendUsageStats() const noexcept;
    bool doSendUsageStats() const noexcept;

    // m_check_news
    void loadCheckNews();
    void setCheckNews(const bool value);
    bool getCheckNews() const noexcept;

    // m_html_scale: scale factor for html font and other objects, in percantage
    void loadHtmlScale();
    void setHtmlScale(const int value);
    int getHtmlScale() const noexcept;

    // m_theme_mode
    void loadThemeMode();
    void setThemeMode(const int value);
    int getThemeMode() const noexcept;

    // m_font_size
    void loadFontSize();
    void setFontSize(const int value);
    int getFontSize() const noexcept;

    // m_icon_size
    void loadIconSize();
    void setIconSize(const int value);
    int getIconSize() const noexcept;

    // m_toolbar_icon_size
    void loadToolbarIconSize();
    void setToolbarIconSize(const int value);
    int getToolbarIconSize() const noexcept;

    // m_navigation_icon_size
    void loadNavigationIconSize();
    void setNavigationIconSize(const int value);
    int getNavigationIconSize() const noexcept;

    int AccountImageId(const int64 account_id, const bool def, const bool ignoreClosure = false);

    // m_ignore_future_transactions
    void loadIgnoreFutureTransactions();
    void setIgnoreFutureTransactions(const bool value);
    bool getIgnoreFutureTransactions() const noexcept;

    // m_show_tooltips
    void loadShowToolTips();
    void setShowToolTips(const bool value);
    bool getShowToolTips() const noexcept;

    // m_show_moneytips
    void loadShowMoneyTips();
    void setShowMoneyTips(const bool value);
    bool getShowMoneyTips() const noexcept;

    // m_checking_range_a, m_checking_range_m
    void loadCheckingRange();
    void setCheckingRange(const wxArrayString &a, int m);
    const wxArrayString getCheckingRangeA() const noexcept;
    int getCheckingRangeM() const noexcept;

private:
    bool m_database_updated = false;
    wxLanguage m_language = wxLANGUAGE_UNKNOWN;

    // stored in Model_Infotable
    wxString m_locale_name;                             // LOCALE
    wxString m_date_format;                             // DATEFORMAT
    wxString m_user_name;                               // USERNAME
    int64 m_base_currency_id = -1;                      // BASECURRENCYID
    bool m_use_currency_history = false;                // USECURRENCYHISTORY
    int m_share_precision = 4;                          // SHARE_PRECISION
    int m_asset_compounding = 0;                        // ASSET_COMPOUNDING
    int m_reporting_first_day = 1;                      // REPORTING_FIRSTDAY
    wxDateTime::WeekDay m_reporting_first_weekday =     // REPORTING_FIRST_WEEKDAY
        wxDateTime::WeekDay::Sun;
    int m_financial_first_day;                          // FINANCIAL_YEAR_START_DAY
    wxDateTime::Month m_financial_first_month;          // FINANCIAL_YEAR_START_MONTH
    int m_budget_days_offset = 0;                       // BUDGET_DAYS_OFFSET
    int m_homepage_incexp_range = 0;                    // HOMEPAGE_INCEXP_RANGE

    // stored in Model_Setting
    bool m_bulk_transactions = false;                   // BULK_TRX
    bool m_hide_share_accounts = true;                  // HIDE_SHARE_ACCOUNTS
    bool m_hide_deleted_transactions = false;           // HIDE_DELETED_TRANSACTIONS
    bool m_budget_financial_years = false;              // BUDGET_FINANCIAL_YEARS
    bool m_budget_include_transfers = false;            // BUDGET_INCLUDE_TRANSFERS
    bool m_budget_summary_without_categories = true;    // BUDGET_SUMMARY_WITHOUT_CATEGORIES
    bool m_budget_override = false;                     // BUDGET_OVERRIDE
    bool m_budget_deduct_monthly = false;               // BUDGET_DEDUCT_MONTH_FROM_YEAR
    bool m_ignore_future_transactions = false;          // IGNORE_FUTURE_TRANSACTIONS
    bool m_show_tooltips = true;                        // IGNORE_SHOW_TOOLTIPS
    bool m_show_moneytips = true;                       // IGNORE_SHOW_MONEYTIPS
    bool m_use_trans_datetime = false;                  // TRANSACTION_USE_DATE_TIME
    int m_trans_payee_none = Option::NONE;              // TRANSACTION_PAYEE_NONE
    int m_trans_category_none = Option::NONE;           // TRANSACTION_CATEGORY_NONE
    int m_trans_category_transfer_none = Option::NONE;  // TRANSACTION_CATEGORY_TRANSFER_NONE
    int m_trans_status_reconciled = Option::NONE;       // TRANSACTION_STATUS_RECONCILED
    int m_trans_date_default = 0;                       // TRANSACTION_DATE_DEFAULT
    bool m_send_usage_stats = true;                     // SENDUSAGESTATS
    bool m_check_news = true;                           // CHECKNEWS
    int m_theme_mode = Option::AUTO;                    // THEMEMODE
    int m_html_scale = 100;                             // HTMLSCALE
    int m_icon_size = 16;                               // ICONSIZE
    int m_font_size = 0;                                // UI_FONT_SIZE
    int m_toolbar_icon_size = 32;                       // TOOLBARICONSIZE
    int m_navigation_icon_size = 24;                    // NAVIGATIONICONSIZE
    wxArrayString m_checking_range_a;                   // CHECKING_RANGE_A
    int m_checking_range_m = 0;                         // CHECKING_RANGE_M
};

inline void Option::setDatabaseUpdated(const bool value)
{
    m_database_updated = value;
}
inline bool Option::getDatabaseUpdated() const noexcept
{
    return m_database_updated;
}

inline const wxString& Option::getLocaleName() const
{
    return m_locale_name;
}

inline const wxString Option::getDateFormat() const
{
    return m_date_format;
}

inline const wxString& Option::getUserName() const
{
    return m_user_name;
}

inline int64 Option::getBaseCurrencyID() const noexcept
{
    return m_base_currency_id;
}

inline bool Option::getUseCurrencyHistory() const noexcept
{
    return m_use_currency_history;
}

inline int Option::getSharePrecision() const noexcept
{
    return m_share_precision;
}

inline int Option::getAssetCompounding() const noexcept
{
    return m_asset_compounding;
}

inline int Option::getReportingFirstDay() const noexcept
{
    return m_reporting_first_day;
}

inline wxDateTime::WeekDay Option::getReportingFirstWeekday() const noexcept
{
    return m_reporting_first_weekday;
}

inline int Option::getFinancialFirstDay() const
{
    return m_financial_first_day;
}

inline wxDateTime::Month Option::getFinancialFirstMonth() const {
    return m_financial_first_month;
}

inline int Option::getBudgetDaysOffset() const noexcept
{
    return m_budget_days_offset;
}

inline int Option::getHomePageIncExpRange() const noexcept
{
    return m_homepage_incexp_range;
}

inline int Option::getIconSize() const noexcept
{
    return m_icon_size;
}

inline int Option::getNavigationIconSize() const noexcept
{
    return m_navigation_icon_size;
}

inline int Option::getToolbarIconSize() const noexcept
{
    return m_toolbar_icon_size;
}

inline int Option::getTransCategoryNone() const noexcept
{
    return m_trans_category_none;
}

inline int Option::getTransCategoryTransferNone() const noexcept
{
    return m_trans_category_transfer_none;
}

inline bool Option::getBulkTransactions() const noexcept
{
    return m_bulk_transactions;
}

inline int Option::getTransPayeeNone() const noexcept
{
    return m_trans_payee_none;
}

inline int Option::getTransStatusReconciled() const noexcept
{
    return m_trans_status_reconciled;
}

inline int Option::getTransDateDefault() const noexcept
{
    return m_trans_date_default;
}

inline bool Option::getCheckNews() const noexcept
{
    return m_check_news;
}

inline bool Option::getHideShareAccounts() const noexcept
{
    return m_hide_share_accounts;
}

inline bool Option::getHideDeletedTransactions() const noexcept
{
    return m_hide_deleted_transactions;
}

inline bool Option::getBudgetFinancialYears() const noexcept
{
    return m_budget_financial_years;
}

inline bool Option::getBudgetIncludeTransfers() const noexcept
{
    return m_budget_include_transfers;
}

inline bool Option::getBudgetSummaryWithoutCategories() const noexcept
{
    return m_budget_summary_without_categories;
}

inline bool Option::UseTransDateTime() const noexcept
{
    return m_use_trans_datetime;
}

inline bool Option::getBudgetOverride() const noexcept
{
    return m_budget_override;
}

inline bool Option::getBudgetDeductMonthly() const noexcept
{
    return m_budget_deduct_monthly;
}

inline int Option::getThemeMode() const noexcept
{
    return m_theme_mode;
}

inline int Option::getFontSize() const noexcept
{
    return m_font_size;
}

inline bool Option::getSendUsageStats() const noexcept
{
    return m_send_usage_stats;
}

inline bool Option::doSendUsageStats() const noexcept
{
#ifdef _DEBUG
    return false;
#else
    return m_send_usage_stats;
#endif
}

inline bool Option::getIgnoreFutureTransactions() const noexcept
{
    return m_ignore_future_transactions;
}

inline bool Option::getShowToolTips() const noexcept
{
    return m_show_tooltips;
}

inline bool Option::getShowMoneyTips() const noexcept
{
    return m_show_moneytips;
}

inline const wxArrayString Option::getCheckingRangeA() const noexcept
{
    return m_checking_range_a;
}

inline int Option::getCheckingRangeM() const noexcept
{
    return m_checking_range_m;
}
