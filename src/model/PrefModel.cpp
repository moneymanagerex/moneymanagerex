/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016 - 2021 Nikolay Akimov
 Copyright (C) 2021-2025 Mark Whalley (mark@ipx.co.uk)

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

#include "base/constants.h"
#include "base/images_list.h"
#include "util/_util.h"
#include "util/mmSingleton.h"

#include "AccountModel.h"
#include "CurrencyHistoryModel.h"
#include "CurrencyModel.h"
#include "InfoModel.h"
#include "PrefModel.h"
#include "SettingModel.h"

#include "dialog/CurrencyChoiceDialog.h"
#include "uicontrols/navigatortypes.h"

const std::vector<std::pair<PrefModel::COMPOUNDING_ID, wxString> > PrefModel::COMPOUNDING_NAME =
{
    { PrefModel::COMPOUNDING_ID_DAY,   _n("Day") },
    { PrefModel::COMPOUNDING_ID_WEEK,  _n("Week") },
    { PrefModel::COMPOUNDING_ID_MONTH, _n("Month") },
    { PrefModel::COMPOUNDING_ID_YEAR,  _n("Year") },
};
const std::vector<std::pair<PrefModel::COMPOUNDING_ID, int> > PrefModel::COMPOUNDING_N =
{
    { PrefModel::COMPOUNDING_ID_DAY,   365 },
    { PrefModel::COMPOUNDING_ID_WEEK,  52 },
    { PrefModel::COMPOUNDING_ID_MONTH, 12 },
    { PrefModel::COMPOUNDING_ID_YEAR,  1 },
};

const std::vector<std::pair<wxString, wxString> > PrefModel::CHECKING_RANGE_DEFAULT =
{
    { "A",            _n("All") },
    { "A .. W",       _n("All to week") },
    { "A .. M",       _n("All to month") },
    { "A .. Y",       _n("All to year") },
    { "Y",            _n("Current year") },
    { "Y F",          _n("Current financial year") },
    { "Q",            _n("Current quarter") },
    { "M",            _n("Current month") },
    { "W",            _n("Current week") },
    { "T",            _n("Today") },
    { "S .. A",       _n("From statement") },
    { "1 S .. A",     _n("From 1 day after statement") },
    { "",             "" },
    { "Y .. W",       _n("From current year to week") },
    { "Y .. M",       _n("From current year to month") },
    { "M .. W",       _n("From current month to week") },
    { "-1 Y",         _n("Previous year") },
    { "-1 M",         _n("Previous month") },
    { "-1 W",         _n("Previous week") },
    { "-1 Y .. A",    _n("From previous year") },
    { "-1 Y .. M",    _n("From previous year to month") },
    { "-1 M .. A",    _n("From previous month") },
    { "-1 M .. W",    _n("From previous month to week") },
    { "-1 W .. A",    _n("From previous week") },
    { "-1 W .. W",    _n("From previous to current week") },
    { "T .. A, -1 Y", _n("From 1 year ago") },
    { "T .. A, -1 Q", _n("From 1 quarter ago") },
    { "T .. A, -1 M", _n("From 1 month ago") },
    { "T .. A, -1 W", _n("From 1 week ago") },
    { "-2 Y",         _n("Year before last") },
};

const std::vector<std::pair<wxString, wxString> > PrefModel::REPORTING_RANGE_DEFAULT =
{
    { "A",            _n("All") },
    { "A .. W",       _n("All to week") },
    { "A .. M",       _n("All to month") },
    { "A .. Q",       _n("All to quarter") },
    { "A .. Y",       _n("All to year") },
    { "Y",            _n("Current year") },
    { "Y F",          _n("Current financial year") },
    { "Q",            _n("Current quarter") },
    { "M",            _n("Current month") },
    { "W",            _n("Current week") },
    { "T",            _n("Today") },
    { "",             "" },
    { "Y .. W",       _n("From current year to week") },
    { "Y .. M",       _n("From current year to month") },
    { "M .. W",       _n("From current month to week") },
    { "-1 Y",         _n("Previous year") },
    { "-1 M",         _n("Previous month") },
    { "-1 W",         _n("Previous week") },
    { "-1 Y .. A",    _n("From previous year") },
    { "-1 Y .. M",    _n("From previous year to month") },
    { "-1 M .. A",    _n("From previous month") },
    { "-1 M .. W",    _n("From previous month to week") },
    { "-1 W .. A",    _n("From previous week") },
    { "-1 W .. W",    _n("From previous to current week") },
    { "T .. A, -1 Y", _n("From 1 year ago") },
    { "T .. A, -1 Q", _n("From 1 quarter ago") },
    { "T .. A, -1 M", _n("From 1 month ago") },
    { "T .. A, -1 W", _n("From 1 week ago") },
    { "-2 Y",         _n("Year before last") },
};

//----------------------------------------------------------------------------
PrefModel::PrefModel()
:   m_date_format(mmex::DEFDATEFORMAT)
{}

//----------------------------------------------------------------------------
PrefModel& PrefModel::instance()
{
    return Singleton<PrefModel>::instance();
}

//----------------------------------------------------------------------------
void PrefModel::load(bool include_infotable)
{
    if (include_infotable) {
        loadDateFormat();
        loadUserName();
        loadBaseCurrencyID();
        loadUseCurrencyHistory();
        loadSharePrecision();
        loadAssetCompounding();
        loadReportingFirstDay();
        loadReportingFirstWeekday();
        loadFinancialFirstDay();
        loadFinancialFirstMonth();
        loadBudgetDaysOffset();
        loadHomePageIncExpRange();

        // Ensure that base currency is set for the database.
        while (m_base_currency_id < 1) {
            if (CurrencyChoiceDialog::Execute(m_base_currency_id)) {
                setBaseCurrencyID(m_base_currency_id);
                CurrencyHistoryModel::instance().purge_all();
                CurrencyModel::instance().resetBaseConversionRates();
            }
        }
    }

    m_language = PrefModel::instance().getLanguageID(true);

    loadHideShareAccounts();
    loadHideDeletedTransactions();
    loadBudgetFinancialYears();
    loadBudgetIncludeTransfers();
    loadBudgetSummaryWithoutCategories();
    loadBudgetOverride();
    loadBudgetDeductMonthly();
    loadIgnoreFutureTransactions();
    loadIgnoreFutureTransactionsHomePage();
    loadShowReconciledInHomePage();
    loadUseTransDateTime();
    loadTreatDateAsSN();
    loadDoNotColorFuture();
    loadDoSpecialColorReconciled();
    loadUsePerAccountFilter();
    loadShowToolTips();
    loadShowMoneyTips();
    loadTransPayeeNone();

    // For the category selection, default behavior should remain that the last category used for the payee is selected.
    //  This is item 1 (0-indexed) in the list.
    loadTransCategoryNone();
    loadTransCategoryTransferNone();
    loadTransStatusReconciled();
    loadTransDateDefault();
    loadSendUsageStats();
    loadCheckNews();

    loadThemeMode();
    loadHtmlScale();
    loadIconSize();
    loadToolbarIconSize();
    loadNavigationIconSize();
    loadFontSize();
    loadCheckingRange();
    loadReportingRange();

    loadShowNavigatorCashLedger();
}

wxLanguage PrefModel::getLanguageID(const bool get_db)
{
    if (get_db) {
        auto lang_id = SettingModel::instance().getInt(LANGUAGE_PARAMETER, -1);

        if (lang_id == -1) {
            auto lang_canonical = SettingModel::instance()
                .getString(LANGUAGE_PARAMETER, wxLocale::GetLanguageCanonicalName(wxLANGUAGE_UNKNOWN));

            for (int lang_code = wxLANGUAGE_DEFAULT; lang_code < wxLANGUAGE_USER_DEFINED; lang_code++) {
                const auto l = wxLocale::GetLanguageCanonicalName(lang_code);
                if (lang_canonical == l) {
                    m_language = static_cast<wxLanguage>(lang_code);
                    break;
                }
            }
        }
        else {
            m_language = static_cast<wxLanguage>(lang_id);
        }
    }

    return m_language;
}

void PrefModel::setLocaleName(const wxString& locale)
{
    InfoModel::instance().setString("LOCALE", locale);
    m_locale_name = locale;
}

void PrefModel::loadDateFormat()
{
    m_date_format = InfoModel::instance().getString("DATEFORMAT", mmex::DEFDATEFORMAT);
}
void PrefModel::setDateFormat(const wxString& date_format)
{
    InfoModel::instance().setString("DATEFORMAT", date_format);
    m_date_format = date_format;
}

void PrefModel::loadUserName()
{
    m_user_name = InfoModel::instance().getString("USERNAME", "");
}
void PrefModel::setUserName(const wxString& username)
{
    m_user_name = username;
    InfoModel::instance().setString("USERNAME", username);
}

void PrefModel::loadBaseCurrencyID()
{
    m_base_currency_id = InfoModel::instance().getInt64("BASECURRENCYID", -1);
}
void PrefModel::setBaseCurrencyID(const int64 base_currency_id)
{
    InfoModel::instance().setInt64("BASECURRENCYID", base_currency_id);
    m_base_currency_id = base_currency_id;
}

void PrefModel::loadUseCurrencyHistory()
{
    m_use_currency_history = InfoModel::instance().getBool("USECURRENCYHISTORY", true);
}
void PrefModel::setUseCurrencyHistory(const bool value)
{
    InfoModel::instance().setBool("USECURRENCYHISTORY", value);
    m_use_currency_history = value;
}

void PrefModel::loadSharePrecision()
{
    m_share_precision = InfoModel::instance().getInt("SHARE_PRECISION", 4);
}
void PrefModel::setSharePrecision(const int value)
{
    InfoModel::instance().setInt("SHARE_PRECISION", value);
    m_share_precision = value;
}

void PrefModel::loadAssetCompounding()
{
    wxString assetCompounding = InfoModel::instance().getString("ASSET_COMPOUNDING", "Day");
    m_asset_compounding = PrefModel::COMPOUNDING_ID_DAY;
    for (const auto& a : PrefModel::COMPOUNDING_NAME) if (assetCompounding == a.second) {
        m_asset_compounding = a.first;
        break;
    }
}
void PrefModel::setAssetCompounding(const int value)
{
    InfoModel::instance().setString("ASSET_COMPOUNDING", PrefModel::COMPOUNDING_NAME[value].second);
    m_asset_compounding = value;
}

void PrefModel::loadReportingFirstDay()
{
    int value = InfoModel::instance().getInt("REPORTING_FIRSTDAY", 1);
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    m_reporting_first_day = value;
}
void PrefModel::setReportingFirstDay(int value)
{
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    InfoModel::instance().setInt("REPORTING_FIRSTDAY", value);
    m_reporting_first_day = value;
}

void PrefModel::loadReportingFirstWeekday()
{
    wxString valueStr = InfoModel::instance().getString("REPORTING_FIRST_WEEKDAY", "");
    m_reporting_first_weekday =
        (valueStr == "Mon") ? wxDateTime::WeekDay::Mon :
        wxDateTime::WeekDay::Sun;
}
void PrefModel::setReportingFirstWeekday(wxDateTime::WeekDay value)
{
    if (value != wxDateTime::WeekDay::Mon)
        value = wxDateTime::WeekDay::Sun;
    InfoModel::instance().setString("REPORTING_FIRST_WEEKDAY", g_short_days_of_week[value]);
    m_reporting_first_weekday = value;
}

void PrefModel::loadFinancialFirstDay()
{
    int value = InfoModel::instance().getInt("FINANCIAL_YEAR_START_DAY", 1);
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    m_financial_first_day = value;
}
void PrefModel::setFinancialFirstDay(int value)
{
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    InfoModel::instance().setInt("FINANCIAL_YEAR_START_DAY", value);
    m_financial_first_day = value;
}

void PrefModel::loadFinancialFirstMonth()
{
    int value = InfoModel::instance().getInt("FINANCIAL_YEAR_START_MONTH", 7);
    if (value < 1) value = 1;
    if (value > 12) value = 12;
    m_financial_first_month = wxDateTime::Month(value - 1);
}
void PrefModel::setFinancialFirstMonth(const wxDateTime::Month value)
{
    wxString valueStr = wxString::Format("%d", value + 1);
    InfoModel::instance().setString("FINANCIAL_YEAR_START_MONTH", valueStr);
    m_financial_first_month = value;
}

void PrefModel::loadBudgetDaysOffset()
{
    m_budget_days_offset = InfoModel::instance().getInt("BUDGET_DAYS_OFFSET", 0);
}
void PrefModel::setBudgetDaysOffset(const int value)
{
    InfoModel::instance().setInt("BUDGET_DAYS_OFFSET", value);
    m_budget_days_offset = value;
}
void PrefModel::addBudgetDateOffset(wxDateTime& dateTime) const
{
    if (m_budget_days_offset != 0)
        dateTime.Add(wxDateSpan::Days(m_budget_days_offset));
}
void PrefModel::addBudgetDateOffset(mmDate& date) const
{
    if (m_budget_days_offset != 0)
        date.addDateSpan(wxDateSpan::Days(m_budget_days_offset));
}

void PrefModel::loadHomePageIncExpRange()
{
    m_homepage_incexp_range = InfoModel::instance().getInt("HOMEPAGE_INCEXP_RANGE", 0);
}
void PrefModel::setHomePageIncExpRange(const int value)
{
    InfoModel::instance().setInt("HOMEPAGE_INCEXP_RANGE", value);
    m_homepage_incexp_range = value;
}

void PrefModel::loadHideShareAccounts()
{
    m_hide_share_accounts = SettingModel::instance().getBool("HIDE_SHARE_ACCOUNTS", true);
}
void PrefModel::setHideShareAccounts(const bool value)
{
    SettingModel::instance().setBool("HIDE_SHARE_ACCOUNTS", value);
    m_hide_share_accounts = value;
}

void PrefModel::loadHideDeletedTransactions()
{
    m_hide_deleted_transactions = SettingModel::instance().getBool("HIDE_DELETED_TRANSACTIONS", false);
}
void PrefModel::setHideDeletedTransactions(const bool value)
{
    SettingModel::instance().setBool("HIDE_DELETED_TRANSACTIONS", value);
    m_hide_deleted_transactions = value;
}

void PrefModel::loadBudgetFinancialYears()
{
    m_budget_financial_years = SettingModel::instance().getBool("BUDGET_FINANCIAL_YEARS", false);
}
void PrefModel::setBudgetFinancialYears(const bool value)
{
    SettingModel::instance().setBool("BUDGET_FINANCIAL_YEARS", value);
    m_budget_financial_years = value;
}

void PrefModel::loadBudgetIncludeTransfers()
{
    m_budget_include_transfers = SettingModel::instance().getBool("BUDGET_INCLUDE_TRANSFERS", false);
}
void PrefModel::setBudgetIncludeTransfers(const bool value)
{
    SettingModel::instance().setBool("BUDGET_INCLUDE_TRANSFERS", value);
    m_budget_include_transfers = value;
}

void PrefModel::loadBudgetSummaryWithoutCategories()
{
    m_budget_summary_without_categories = SettingModel::instance().getBool("BUDGET_SUMMARY_WITHOUT_CATEGORIES", true);
}
void PrefModel::setBudgetSummaryWithoutCategories(bool value)
{
    SettingModel::instance().setBool("BUDGET_SUMMARY_WITHOUT_CATEGORIES", value);
    m_budget_summary_without_categories = value;
}

void PrefModel::loadBudgetOverride()
{
    m_budget_override = SettingModel::instance().getBool("BUDGET_OVERRIDE", false);
}
void PrefModel::setBudgetOverride(const bool value)
{
    SettingModel::instance().setBool("BUDGET_OVERRIDE", value);
    m_budget_override = value;
}

void PrefModel::loadBudgetDeductMonthly()
{
    m_budget_deduct_monthly = SettingModel::instance().getBool("BUDGET_DEDUCT_MONTH_FROM_YEAR", false);
}
void PrefModel::setBudgetDeductMonthly(bool value)
{
    SettingModel::instance().setBool("BUDGET_DEDUCT_MONTH_FROM_YEAR", value);
    m_budget_deduct_monthly = value;
}

void PrefModel::loadIgnoreFutureTransactions()
{
    m_ignore_future_transactions = SettingModel::instance().getBool("IGNORE_FUTURE_TRANSACTIONS", false);
}
void PrefModel::setIgnoreFutureTransactions(const bool value)
{
    SettingModel::instance().setBool("IGNORE_FUTURE_TRANSACTIONS", value);
    m_ignore_future_transactions = value;
}

void PrefModel::loadIgnoreFutureTransactionsHomePage()
{
    m_ignore_future_transactions_home = SettingModel::instance().getBool("IGNORE_FUTURE_TRANSACTIONS_HOMEPAGE", true);
}
void PrefModel::setIgnoreFutureTransactionsHomePage(const bool value)
{
    SettingModel::instance().setBool("IGNORE_FUTURE_TRANSACTIONS_HOMEPAGE", value);
    m_ignore_future_transactions_home = value;
}

void PrefModel::loadShowReconciledInHomePage()
{
    m_show_reconciled_in_home_page = SettingModel::instance().getBool("SHOW_RECONCILED_IN_HOME_PAGE", true);
}
void PrefModel::setShowReconciledInHomePage(const bool value)
{
    SettingModel::instance().setBool("SHOW_RECONCILED_IN_HOME_PAGE", value);
    m_show_reconciled_in_home_page = value;
}

void PrefModel::loadUseTransDateTime()
{
    m_use_trans_datetime = SettingModel::instance().getBool("TRANSACTION_USE_DATE_TIME", false);
}

bool PrefModel::UseTransDateTime(const bool value)
{
    if (value != m_use_trans_datetime) {
        SettingModel::instance().setBool("TRANSACTION_USE_DATE_TIME", value);
        m_use_trans_datetime = value;
        return true;
    }
    return false;
}

void PrefModel::loadTreatDateAsSN()
{
    m_treat_date_as_SN = SettingModel::instance().getBool("TRANSACTION_TREAT_DATE_AS_SN", true);
}

bool PrefModel::TreatDateAsSN(const bool value)
{
    if (value != m_treat_date_as_SN) {
        SettingModel::instance().setBool("TRANSACTION_TREAT_DATE_AS_SN", value);
        m_treat_date_as_SN = value;
        return true;
    }
    return false;
}

void PrefModel::loadDoNotColorFuture()
{
    m_do_not_color_future = SettingModel::instance().getBool("DO_NOT_COLOR_FUTURE_TRANSACTIONS", true);
}
void PrefModel::setDoNotColorFuture(const bool value)
{
    SettingModel::instance().setBool("DO_NOT_COLOR_FUTURE_TRANSACTIONS", value);
    m_do_not_color_future = value;
}

void PrefModel::loadDoSpecialColorReconciled()
{
    m_do_special_color_reconciled = SettingModel::instance().getBool("SPECIAL_COLOR_RECONCILED_TRANSACTIONS", true);
}
void PrefModel::setDoSpecialColorReconciled(const bool value)
{
    SettingModel::instance().setBool("SPECIAL_COLOR_RECONCILED_TRANSACTIONS", value);
    m_do_special_color_reconciled = value;
}

void PrefModel::loadUsePerAccountFilter()
{
    m_store_account_specific_filter = SettingModel::instance().getBool("USE_PER_ACCOUNT_FILTER", true);
}
void PrefModel::setUsePerAccountFilter(const bool value)
{
    SettingModel::instance().setBool("USE_PER_ACCOUNT_FILTER", value);
    m_store_account_specific_filter = value;
}


void PrefModel::loadShowToolTips()
{
    m_show_tooltips = SettingModel::instance().getBool("IGNORE_SHOW_TOOLTIPS", true);
}
void PrefModel::setShowToolTips(const bool value)
{
    SettingModel::instance().setBool("IGNORE_SHOW_TOOLTIPS", value);
    m_show_tooltips = value;
}

void PrefModel::loadShowMoneyTips()
{
    m_show_moneytips = SettingModel::instance().getBool("IGNORE_SHOW_MONEYTIPS", true);
}
void PrefModel::setShowMoneyTips(const bool value)
{
    SettingModel::instance().setBool("IGNORE_SHOW_MONEYTIPS", value);
    m_show_moneytips = value;
}

void PrefModel::loadTransPayeeNone()
{
    // Read the preference as a string and convert to int
    m_trans_payee_none = SettingModel::instance().getInt("TRANSACTION_PAYEE_NONE", PrefModel::NONE);
}
void PrefModel::setTransPayeeNone(const int value)
{
    SettingModel::instance().setInt("TRANSACTION_PAYEE_NONE", value);
    m_trans_payee_none = value;
}

void PrefModel::loadTransCategoryNone()
{
    m_trans_category_none = SettingModel::instance().getInt("TRANSACTION_CATEGORY_NONE", PrefModel::LASTUSED);
}
void PrefModel::setTransCategoryNone(const int value)
{
    SettingModel::instance().setInt("TRANSACTION_CATEGORY_NONE", value);
    m_trans_category_none = value;
}

void PrefModel::loadTransCategoryTransferNone()
{
    m_trans_category_transfer_none = SettingModel::instance().getInt("TRANSACTION_CATEGORY_TRANSFER_NONE", PrefModel::LASTUSED);
}
void PrefModel::setTransCategoryTransferNone(const int value)
{
    SettingModel::instance().setInt("TRANSACTION_CATEGORY_TRANSFER_NONE", value);
    m_trans_category_transfer_none = value;
}

void PrefModel::loadTransStatusReconciled()
{
    m_trans_status_reconciled = SettingModel::instance().getInt("TRANSACTION_STATUS_RECONCILED", PrefModel::NONE);
}
void PrefModel::setTransStatusReconciled(const int value)
{
    SettingModel::instance().setInt("TRANSACTION_STATUS_RECONCILED", value);
    m_trans_status_reconciled = value;
}

void PrefModel::loadTransDateDefault()
{
    m_trans_date_default = SettingModel::instance().getInt("TRANSACTION_DATE_DEFAULT", PrefModel::NONE);
}
void PrefModel::setTransDateDefault(const int value)
{
    SettingModel::instance().setInt("TRANSACTION_DATE_DEFAULT", value);
    m_trans_date_default = value;
}

void PrefModel::loadSendUsageStats()
{
    m_send_usage_stats = SettingModel::instance().getBool(INIDB_SEND_USAGE_STATS, true);
}
void PrefModel::setSendUsageStats(const bool value)
{
    SettingModel::instance().setBool(INIDB_SEND_USAGE_STATS, value);
    m_send_usage_stats = value;
}

void PrefModel::loadCheckNews()
{
    m_check_news = SettingModel::instance().getBool("CHECKNEWS", true);
}
void PrefModel::setCheckNews(const bool value)
{
    SettingModel::instance().setBool("CHECKNEWS", value);
    m_check_news = value;
}

void PrefModel::loadThemeMode()
{
    m_theme_mode = SettingModel::instance().getInt("THEMEMODE", PrefModel::THEME_MODE::AUTO);
}
void PrefModel::setThemeMode(const int value)
{
    SettingModel::instance().setInt("THEMEMODE", value);
    m_theme_mode = value;
}

void PrefModel::loadHtmlScale()
{
    m_html_scale = SettingModel::instance().getInt("HTMLSCALE", 100);
}
void PrefModel::setHtmlScale(const int value)
{
    SettingModel::instance().setInt("HTMLSCALE", value);
    m_html_scale = value;
}

void PrefModel::loadFontSize()
{
    m_font_size = SettingModel::instance().getInt("UI_FONT_SIZE", 0);
}
void PrefModel::setFontSize(const int value)
{
    SettingModel::instance().setInt("UI_FONT_SIZE", value);
    m_font_size = value;
}

void PrefModel::loadIconSize()
{
    m_icon_size = SettingModel::instance().getInt("ICONSIZE", 16);
}
void PrefModel::setIconSize(const int value)
{
    SettingModel::instance().setInt("ICONSIZE", value);
    m_icon_size = value;
}

void PrefModel::loadToolbarIconSize()
{
    m_toolbar_icon_size = SettingModel::instance().getInt("TOOLBARICONSIZE", 32);
}
void PrefModel::setToolbarIconSize(const int value)
{
    SettingModel::instance().setInt("TOOLBARICONSIZE", value);
    m_toolbar_icon_size = value;
}

void PrefModel::loadNavigationIconSize()
{
    m_navigation_icon_size = SettingModel::instance().getInt("NAVIGATIONICONSIZE", 24);
}
void PrefModel::setNavigationIconSize(const int value)
{
    SettingModel::instance().setInt("NAVIGATIONICONSIZE", value);
    m_navigation_icon_size = value;
}

void PrefModel::loadCheckingRange()
{
    m_checking_range = SettingModel::instance().getArrayString("CHECKING_RANGE");
    parseCheckingRange();
}
void PrefModel::setCheckingRange(const wxArrayString &a)
{
    SettingModel::instance().setArrayString("CHECKING_RANGE", a);
    m_checking_range = a;
    parseCheckingRange();
}
void PrefModel::parseCheckingRange()
{
    //wxLogDebug("{{{ PrefModel::parseCheckingRange()");

    m_checking_range_a.clear();
    m_checking_range_m = 0;

    for (wxString &str : m_checking_range) {
        if (str.empty()) {
            if (m_checking_range_m == 0)
                m_checking_range_m = m_checking_range_a.size();
            continue;
        }
        mmDateRange2::Range range;
        if (!range.parseLabelName(str))
            continue;
        m_checking_range_a.push_back(range);
    }

    if (!m_checking_range_a.empty())
        goto done;

    for (auto &default_range : CHECKING_RANGE_DEFAULT) {
        wxString label = default_range.first;
        if (label.empty()) {
            if (m_checking_range_m == 0)
                m_checking_range_m = m_checking_range_a.size();
            continue;
        }
        wxString name = wxGetTranslation(default_range.second);
        mmDateRange2::Range range;
        if (!range.parseLabelName(label, name))
            continue;
        m_checking_range_a.push_back(range);
    }

    done:
    if (m_checking_range_m == 0)
        m_checking_range_m = m_checking_range_a.size();

    /*wxLogDebug("m=[%d], n=[%zu]", m_checking_range_m, m_checking_range_a.size());
    for ([[maybe_unused]] mmDateRange2::Range &range : m_checking_range_a) {
        wxLogDebug("label=[%s], name=[%s]", range.getLabel(), range.getName());
    }
    wxLogDebug("}}}");*/
}

void PrefModel::loadReportingRange()
{
    m_reporting_range = SettingModel::instance().getArrayString("REPORTING_RANGE");
    parseReportingRange();
}
void PrefModel::setReportingRange(const wxArrayString &a)
{
    SettingModel::instance().setArrayString("REPORTING_RANGE", a);
    m_reporting_range = a;
    parseReportingRange();
}
void PrefModel::parseReportingRange()
{
    //wxLogDebug("{{{ PrefModel::parseReportingRange()");

    m_reporting_range_a.clear();
    m_reporting_range_m = 0;

    for (wxString &str : m_reporting_range) {
        if (str.empty()) {
            if (m_reporting_range_m == 0)
                m_reporting_range_m = m_reporting_range_a.size();
            continue;
        }
        mmDateRange2::Range range;
        if (!range.parseLabelName(str))
            continue;
        m_reporting_range_a.push_back(range);
    }

    if (!m_reporting_range_a.empty())
        goto done;

    for (auto &default_range : REPORTING_RANGE_DEFAULT) {
        wxString label = default_range.first;
        if (label.empty()) {
            if (m_reporting_range_m == 0)
                m_reporting_range_m = m_reporting_range_a.size();
            continue;
        }
        wxString name = wxGetTranslation(default_range.second);
        mmDateRange2::Range range;
        if (!range.parseLabelName(label, name))
            continue;
        m_reporting_range_a.push_back(range);
    }

    done:
    if (m_reporting_range_m == 0)
        m_reporting_range_m = m_reporting_range_a.size();

    /*wxLogDebug("m=[%d], n=[%zu]", m_reporting_range_m, m_reporting_range_a.size());
    for ([[maybe_unused]] mmDateRange2::Range &range : m_reporting_range_a) {
        wxLogDebug("label=[%s], name=[%s]", range.getLabel(), range.getName());
    }
    wxLogDebug("}}}");*/
}

int PrefModel::getHtmlScale() const noexcept
{
    return m_html_scale;
}

int PrefModel::AccountImageId(const int64 account_id, const bool def, const bool ignoreClosure)
{
    wxString acctStatus = VIEW_ACCOUNTS_OPEN_STR;
    NavigatorTypes::TYPE_ID acctType = NavigatorTypes::TYPE_ID_CHECKING;
    int selectedImage = img::SAVINGS_ACC_NORMAL_PNG; //Default value

    const AccountData* account_n = AccountModel::instance().get_id_data_n(account_id);
    if (account_n) {
        acctType = AccountModel::type_id(*account_n);
        acctStatus = account_n->m_status.name();
    }

    if (!def && !ignoreClosure && (acctStatus == "Closed"))
        return img::ACCOUNT_CLOSED_PNG;

    int max = acc_img::MAX_ACC_ICON - static_cast<int>(img::LAST_NAVTREE_PNG);
    int min = 1;
    int custom_img_id = InfoModel::instance().getInt(wxString::Format("ACC_IMAGE_ID_%lld", account_id), 0);
    if (custom_img_id > max) custom_img_id = custom_img_id - 20; //Bug #963 fix
    if (!def && (custom_img_id >= min && custom_img_id <= max))
        return custom_img_id + img::LAST_NAVTREE_PNG - 1;

    NavigatorTypesInfo* info = NavigatorTypes::instance().FindEntry(acctType);
    if (info) {
        selectedImage = info->imageId;
    }
    return selectedImage;
}

const wxString PrefModel::getLanguageCode(const bool get_db)
{
    PrefModel::getLanguageID(get_db);
    if (m_language == wxLANGUAGE_UNKNOWN)
        return wxEmptyString;
    if (m_language == wxLANGUAGE_DEFAULT)
        return wxTranslations::Get()->GetBestTranslation("mmex", wxLANGUAGE_ENGLISH_US);

    const auto lang = wxLocale::GetLanguageCanonicalName(m_language);
    return lang;
}

void PrefModel::setLanguage(const wxLanguage& language)
{
    m_language = language;
    SettingModel::instance().setString(
        LANGUAGE_PARAMETER,
        wxLocale::GetLanguageCanonicalName(language)
    );
}

void PrefModel::loadShowNavigatorCashLedger()
{
    m_show_navigator_cashLedger = SettingModel::instance().getBool("NAVIGATOR_SHOW_CASHLEDGER", true);
}
void PrefModel::setShowNavigatorCashLedger(const bool value)
{
    SettingModel::instance().setBool("NAVIGATOR_SHOW_CASHLEDGER", value);
    m_show_navigator_cashLedger = value;
}
