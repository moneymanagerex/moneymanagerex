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
#include "PreferencesModel.h"
#include "SettingModel.h"

#include "dialog/CurrencyChoiceDialog.h"
#include "uicontrols/navigatortypes.h"

const std::vector<std::pair<PreferencesModel::COMPOUNDING_ID, wxString> > PreferencesModel::COMPOUNDING_NAME =
{
    { PreferencesModel::COMPOUNDING_ID_DAY,   _n("Day") },
    { PreferencesModel::COMPOUNDING_ID_WEEK,  _n("Week") },
    { PreferencesModel::COMPOUNDING_ID_MONTH, _n("Month") },
    { PreferencesModel::COMPOUNDING_ID_YEAR,  _n("Year") },
};
const std::vector<std::pair<PreferencesModel::COMPOUNDING_ID, int> > PreferencesModel::COMPOUNDING_N =
{
    { PreferencesModel::COMPOUNDING_ID_DAY,   365 },
    { PreferencesModel::COMPOUNDING_ID_WEEK,  52 },
    { PreferencesModel::COMPOUNDING_ID_MONTH, 12 },
    { PreferencesModel::COMPOUNDING_ID_YEAR,  1 },
};

const std::vector<std::pair<wxString, wxString> > PreferencesModel::CHECKING_RANGE_DEFAULT =
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

const std::vector<std::pair<wxString, wxString> > PreferencesModel::REPORTING_RANGE_DEFAULT =
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
PreferencesModel::PreferencesModel()
:   m_date_format(mmex::DEFDATEFORMAT)
{}

//----------------------------------------------------------------------------
PreferencesModel& PreferencesModel::instance()
{
    return Singleton<PreferencesModel>::instance();
}

//----------------------------------------------------------------------------
void PreferencesModel::load(bool include_infotable)
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
                CurrencyHistoryModel::ResetCurrencyHistory();
                CurrencyModel::ResetBaseConversionRates();
            }
        }
    }

    m_language = PreferencesModel::instance().getLanguageID(true);

    loadHideShareAccounts();
    loadHideDeletedTransactions();
    loadBudgetFinancialYears();
    loadBudgetIncludeTransfers();
    loadBudgetSummaryWithoutCategories();
    loadBudgetOverride();
    loadBudgetDeductMonthly();
    loadIgnoreFutureTransactions();
    loadIgnoreFutureTransactionsHomePage();
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

wxLanguage PreferencesModel::getLanguageID(const bool get_db)
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

void PreferencesModel::setLocaleName(const wxString& locale)
{
    InfoModel::instance().setString("LOCALE", locale);
    m_locale_name = locale;
}

void PreferencesModel::loadDateFormat()
{
    m_date_format = InfoModel::instance().getString("DATEFORMAT", mmex::DEFDATEFORMAT);
}
void PreferencesModel::setDateFormat(const wxString& date_format)
{
    InfoModel::instance().setString("DATEFORMAT", date_format);
    m_date_format = date_format;
}

void PreferencesModel::loadUserName()
{
    m_user_name = InfoModel::instance().getString("USERNAME", "");
}
void PreferencesModel::setUserName(const wxString& username)
{
    m_user_name = username;
    InfoModel::instance().setString("USERNAME", username);
}

void PreferencesModel::loadBaseCurrencyID()
{
    m_base_currency_id = InfoModel::instance().getInt64("BASECURRENCYID", -1);
}
void PreferencesModel::setBaseCurrencyID(const int64 base_currency_id)
{
    InfoModel::instance().setInt64("BASECURRENCYID", base_currency_id);
    m_base_currency_id = base_currency_id;
}

void PreferencesModel::loadUseCurrencyHistory()
{
    m_use_currency_history = InfoModel::instance().getBool("USECURRENCYHISTORY", true);
}
void PreferencesModel::setUseCurrencyHistory(const bool value)
{
    InfoModel::instance().setBool("USECURRENCYHISTORY", value);
    m_use_currency_history = value;
}

void PreferencesModel::loadSharePrecision()
{
    m_share_precision = InfoModel::instance().getInt("SHARE_PRECISION", 4);
}
void PreferencesModel::setSharePrecision(const int value)
{
    InfoModel::instance().setInt("SHARE_PRECISION", value);
    m_share_precision = value;
}

void PreferencesModel::loadAssetCompounding()
{
    wxString assetCompounding = InfoModel::instance().getString("ASSET_COMPOUNDING", "Day");
    m_asset_compounding = PreferencesModel::COMPOUNDING_ID_DAY;
    for (const auto& a : PreferencesModel::COMPOUNDING_NAME) if (assetCompounding == a.second) {
        m_asset_compounding = a.first;
        break;
    }
}
void PreferencesModel::setAssetCompounding(const int value)
{
    InfoModel::instance().setString("ASSET_COMPOUNDING", PreferencesModel::COMPOUNDING_NAME[value].second);
    m_asset_compounding = value;
}

void PreferencesModel::loadReportingFirstDay()
{
    int value = InfoModel::instance().getInt("REPORTING_FIRSTDAY", 1);
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    m_reporting_first_day = value;
}
void PreferencesModel::setReportingFirstDay(int value)
{
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    InfoModel::instance().setInt("REPORTING_FIRSTDAY", value);
    m_reporting_first_day = value;
}

void PreferencesModel::loadReportingFirstWeekday()
{
    wxString valueStr = InfoModel::instance().getString("REPORTING_FIRST_WEEKDAY", "");
    m_reporting_first_weekday =
        (valueStr == "Mon") ? wxDateTime::WeekDay::Mon :
        wxDateTime::WeekDay::Sun;
}
void PreferencesModel::setReportingFirstWeekday(wxDateTime::WeekDay value)
{
    if (value != wxDateTime::WeekDay::Mon)
        value = wxDateTime::WeekDay::Sun;
    InfoModel::instance().setString("REPORTING_FIRST_WEEKDAY", g_short_days_of_week[value]);
    m_reporting_first_weekday = value;
}

void PreferencesModel::loadFinancialFirstDay()
{
    int value = InfoModel::instance().getInt("FINANCIAL_YEAR_START_DAY", 1);
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    m_financial_first_day = value;
}
void PreferencesModel::setFinancialFirstDay(int value)
{
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    InfoModel::instance().setInt("FINANCIAL_YEAR_START_DAY", value);
    m_financial_first_day = value;
}

void PreferencesModel::loadFinancialFirstMonth()
{
    int value = InfoModel::instance().getInt("FINANCIAL_YEAR_START_MONTH", 7);
    if (value < 1) value = 1;
    if (value > 12) value = 12;
    m_financial_first_month = wxDateTime::Month(value - 1);
}
void PreferencesModel::setFinancialFirstMonth(const wxDateTime::Month value)
{
    wxString valueStr = wxString::Format("%d", value + 1);
    InfoModel::instance().setString("FINANCIAL_YEAR_START_MONTH", valueStr);
    m_financial_first_month = value;
}

void PreferencesModel::loadBudgetDaysOffset()
{
    m_budget_days_offset = InfoModel::instance().getInt("BUDGET_DAYS_OFFSET", 0);
}
void PreferencesModel::setBudgetDaysOffset(const int value)
{
    InfoModel::instance().setInt("BUDGET_DAYS_OFFSET", value);
    m_budget_days_offset = value;
}
void PreferencesModel::addBudgetDateOffset(wxDateTime& date) const
{
    if (m_budget_days_offset != 0)
        date.Add(wxDateSpan::Days(m_budget_days_offset));
}

void PreferencesModel::loadHomePageIncExpRange()
{
    m_homepage_incexp_range = InfoModel::instance().getInt("HOMEPAGE_INCEXP_RANGE", 0);
}
void PreferencesModel::setHomePageIncExpRange(const int value)
{
    InfoModel::instance().setInt("HOMEPAGE_INCEXP_RANGE", value);
    m_homepage_incexp_range = value;
}

void PreferencesModel::loadHideShareAccounts()
{
    m_hide_share_accounts = SettingModel::instance().getBool("HIDE_SHARE_ACCOUNTS", true);
}
void PreferencesModel::setHideShareAccounts(const bool value)
{
    SettingModel::instance().setBool("HIDE_SHARE_ACCOUNTS", value);
    m_hide_share_accounts = value;
}

void PreferencesModel::loadHideDeletedTransactions()
{
    m_hide_deleted_transactions = SettingModel::instance().getBool("HIDE_DELETED_TRANSACTIONS", false);
}
void PreferencesModel::setHideDeletedTransactions(const bool value)
{
    SettingModel::instance().setBool("HIDE_DELETED_TRANSACTIONS", value);
    m_hide_deleted_transactions = value;
}

void PreferencesModel::loadBudgetFinancialYears()
{
    m_budget_financial_years = SettingModel::instance().getBool("BUDGET_FINANCIAL_YEARS", false);
}
void PreferencesModel::setBudgetFinancialYears(const bool value)
{
    SettingModel::instance().setBool("BUDGET_FINANCIAL_YEARS", value);
    m_budget_financial_years = value;
}

void PreferencesModel::loadBudgetIncludeTransfers()
{
    m_budget_include_transfers = SettingModel::instance().getBool("BUDGET_INCLUDE_TRANSFERS", false);
}
void PreferencesModel::setBudgetIncludeTransfers(const bool value)
{
    SettingModel::instance().setBool("BUDGET_INCLUDE_TRANSFERS", value);
    m_budget_include_transfers = value;
}

void PreferencesModel::loadBudgetSummaryWithoutCategories()
{
    m_budget_summary_without_categories = SettingModel::instance().getBool("BUDGET_SUMMARY_WITHOUT_CATEGORIES", true);
}
void PreferencesModel::setBudgetSummaryWithoutCategories(bool value)
{
    SettingModel::instance().setBool("BUDGET_SUMMARY_WITHOUT_CATEGORIES", value);
    m_budget_summary_without_categories = value;
}

void PreferencesModel::loadBudgetOverride()
{
    m_budget_override = SettingModel::instance().getBool("BUDGET_OVERRIDE", false);
}
void PreferencesModel::setBudgetOverride(const bool value)
{
    SettingModel::instance().setBool("BUDGET_OVERRIDE", value);
    m_budget_override = value;
}

void PreferencesModel::loadBudgetDeductMonthly()
{
    m_budget_deduct_monthly = SettingModel::instance().getBool("BUDGET_DEDUCT_MONTH_FROM_YEAR", false);
}
void PreferencesModel::setBudgetDeductMonthly(bool value)
{
    SettingModel::instance().setBool("BUDGET_DEDUCT_MONTH_FROM_YEAR", value);
    m_budget_deduct_monthly = value;
}

void PreferencesModel::loadIgnoreFutureTransactions()
{
    m_ignore_future_transactions = SettingModel::instance().getBool("IGNORE_FUTURE_TRANSACTIONS", false);
}
void PreferencesModel::setIgnoreFutureTransactions(const bool value)
{
    SettingModel::instance().setBool("IGNORE_FUTURE_TRANSACTIONS", value);
    m_ignore_future_transactions = value;
}

void PreferencesModel::loadIgnoreFutureTransactionsHomePage()
{
    m_ignore_future_transactions_home = SettingModel::instance().getBool("IGNORE_FUTURE_TRANSACTIONS_HOMEPAGE", true);
}
void PreferencesModel::setIgnoreFutureTransactionsHomePage(const bool value)
{
    SettingModel::instance().setBool("IGNORE_FUTURE_TRANSACTIONS_HOMEPAGE", value);
    m_ignore_future_transactions_home = value;
}

void PreferencesModel::loadUseTransDateTime()
{
    m_use_trans_datetime = SettingModel::instance().getBool("TRANSACTION_USE_DATE_TIME", false);
}

bool PreferencesModel::UseTransDateTime(const bool value)
{
    if (value != m_use_trans_datetime) {
        SettingModel::instance().setBool("TRANSACTION_USE_DATE_TIME", value);
        m_use_trans_datetime = value;
        return true;
    }
    return false;
}

void PreferencesModel::loadTreatDateAsSN()
{
    m_treat_date_as_SN = SettingModel::instance().getBool("TRANSACTION_TREAT_DATE_AS_SN", true);
}

bool PreferencesModel::TreatDateAsSN(const bool value)
{
    if (value != m_treat_date_as_SN) {
        SettingModel::instance().setBool("TRANSACTION_TREAT_DATE_AS_SN", value);
        m_treat_date_as_SN = value;
        return true;
    }
    return false;
}

void PreferencesModel::loadDoNotColorFuture()
{
    m_do_not_color_future = SettingModel::instance().getBool("DO_NOT_COLOR_FUTURE_TRANSACTIONS", true);
}
void PreferencesModel::setDoNotColorFuture(const bool value)
{
    SettingModel::instance().setBool("DO_NOT_COLOR_FUTURE_TRANSACTIONS", value);
    m_do_not_color_future = value;
}

void PreferencesModel::loadDoSpecialColorReconciled()
{
    m_do_special_color_reconciled = SettingModel::instance().getBool("SPECIAL_COLOR_RECONCILED_TRANSACTIONS", true);
}
void PreferencesModel::setDoSpecialColorReconciled(const bool value)
{
    SettingModel::instance().setBool("SPECIAL_COLOR_RECONCILED_TRANSACTIONS", value);
    m_do_special_color_reconciled = value;
}

void PreferencesModel::loadUsePerAccountFilter()
{
    m_store_account_specific_filter = SettingModel::instance().getBool("USE_PER_ACCOUNT_FILTER", true);
}
void PreferencesModel::setUsePerAccountFilter(const bool value)
{
    SettingModel::instance().setBool("USE_PER_ACCOUNT_FILTER", value);
    m_store_account_specific_filter = value;
}


void PreferencesModel::loadShowToolTips()
{
    m_show_tooltips = SettingModel::instance().getBool("IGNORE_SHOW_TOOLTIPS", true);
}
void PreferencesModel::setShowToolTips(const bool value)
{
    SettingModel::instance().setBool("IGNORE_SHOW_TOOLTIPS", value);
    m_show_tooltips = value;
}

void PreferencesModel::loadShowMoneyTips()
{
    m_show_moneytips = SettingModel::instance().getBool("IGNORE_SHOW_MONEYTIPS", true);
}
void PreferencesModel::setShowMoneyTips(const bool value)
{
    SettingModel::instance().setBool("IGNORE_SHOW_MONEYTIPS", value);
    m_show_moneytips = value;
}

void PreferencesModel::loadTransPayeeNone()
{
    // Read the preference as a string and convert to int
    m_trans_payee_none = SettingModel::instance().getInt("TRANSACTION_PAYEE_NONE", PreferencesModel::NONE);
}
void PreferencesModel::setTransPayeeNone(const int value)
{
    SettingModel::instance().setInt("TRANSACTION_PAYEE_NONE", value);
    m_trans_payee_none = value;
}

void PreferencesModel::loadTransCategoryNone()
{
    m_trans_category_none = SettingModel::instance().getInt("TRANSACTION_CATEGORY_NONE", PreferencesModel::LASTUSED);
}
void PreferencesModel::setTransCategoryNone(const int value)
{
    SettingModel::instance().setInt("TRANSACTION_CATEGORY_NONE", value);
    m_trans_category_none = value;
}

void PreferencesModel::loadTransCategoryTransferNone()
{
    m_trans_category_transfer_none = SettingModel::instance().getInt("TRANSACTION_CATEGORY_TRANSFER_NONE", PreferencesModel::LASTUSED);
}
void PreferencesModel::setTransCategoryTransferNone(const int value)
{
    SettingModel::instance().setInt("TRANSACTION_CATEGORY_TRANSFER_NONE", value);
    m_trans_category_transfer_none = value;
}

void PreferencesModel::loadTransStatusReconciled()
{
    m_trans_status_reconciled = SettingModel::instance().getInt("TRANSACTION_STATUS_RECONCILED", PreferencesModel::NONE);
}
void PreferencesModel::setTransStatusReconciled(const int value)
{
    SettingModel::instance().setInt("TRANSACTION_STATUS_RECONCILED", value);
    m_trans_status_reconciled = value;
}

void PreferencesModel::loadTransDateDefault()
{
    m_trans_date_default = SettingModel::instance().getInt("TRANSACTION_DATE_DEFAULT", PreferencesModel::NONE);
}
void PreferencesModel::setTransDateDefault(const int value)
{
    SettingModel::instance().setInt("TRANSACTION_DATE_DEFAULT", value);
    m_trans_date_default = value;
}

void PreferencesModel::loadSendUsageStats()
{
    m_send_usage_stats = SettingModel::instance().getBool(INIDB_SEND_USAGE_STATS, true);
}
void PreferencesModel::setSendUsageStats(const bool value)
{
    SettingModel::instance().setBool(INIDB_SEND_USAGE_STATS, value);
    m_send_usage_stats = value;
}

void PreferencesModel::loadCheckNews()
{
    m_check_news = SettingModel::instance().getBool("CHECKNEWS", true);
}
void PreferencesModel::setCheckNews(const bool value)
{
    SettingModel::instance().setBool("CHECKNEWS", value);
    m_check_news = value;
}

void PreferencesModel::loadThemeMode()
{
    m_theme_mode = SettingModel::instance().getInt("THEMEMODE", PreferencesModel::THEME_MODE::AUTO);
}
void PreferencesModel::setThemeMode(const int value)
{
    SettingModel::instance().setInt("THEMEMODE", value);
    m_theme_mode = value;
}

void PreferencesModel::loadHtmlScale()
{
    m_html_scale = SettingModel::instance().getInt("HTMLSCALE", 100);
}
void PreferencesModel::setHtmlScale(const int value)
{
    SettingModel::instance().setInt("HTMLSCALE", value);
    m_html_scale = value;
}

void PreferencesModel::loadFontSize()
{
    m_font_size = SettingModel::instance().getInt("UI_FONT_SIZE", 0);
}
void PreferencesModel::setFontSize(const int value)
{
    SettingModel::instance().setInt("UI_FONT_SIZE", value);
    m_font_size = value;
}

void PreferencesModel::loadIconSize()
{
    m_icon_size = SettingModel::instance().getInt("ICONSIZE", 16);
}
void PreferencesModel::setIconSize(const int value)
{
    SettingModel::instance().setInt("ICONSIZE", value);
    m_icon_size = value;
}

void PreferencesModel::loadToolbarIconSize()
{
    m_toolbar_icon_size = SettingModel::instance().getInt("TOOLBARICONSIZE", 32);
}
void PreferencesModel::setToolbarIconSize(const int value)
{
    SettingModel::instance().setInt("TOOLBARICONSIZE", value);
    m_toolbar_icon_size = value;
}

void PreferencesModel::loadNavigationIconSize()
{
    m_navigation_icon_size = SettingModel::instance().getInt("NAVIGATIONICONSIZE", 24);
}
void PreferencesModel::setNavigationIconSize(const int value)
{
    SettingModel::instance().setInt("NAVIGATIONICONSIZE", value);
    m_navigation_icon_size = value;
}

void PreferencesModel::loadCheckingRange()
{
    m_checking_range = SettingModel::instance().getArrayString("CHECKING_RANGE");
    parseCheckingRange();
}
void PreferencesModel::setCheckingRange(const wxArrayString &a)
{
    SettingModel::instance().setArrayString("CHECKING_RANGE", a);
    m_checking_range = a;
    parseCheckingRange();
}
void PreferencesModel::parseCheckingRange()
{
    //wxLogDebug("{{{ PreferencesModel::parseCheckingRange()");

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

void PreferencesModel::loadReportingRange()
{
    m_reporting_range = SettingModel::instance().getArrayString("REPORTING_RANGE");
    parseReportingRange();
}
void PreferencesModel::setReportingRange(const wxArrayString &a)
{
    SettingModel::instance().setArrayString("REPORTING_RANGE", a);
    m_reporting_range = a;
    parseReportingRange();
}
void PreferencesModel::parseReportingRange()
{
    //wxLogDebug("{{{ PreferencesModel::parseReportingRange()");

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

int PreferencesModel::getHtmlScale() const noexcept
{
    return m_html_scale;
}

int PreferencesModel::AccountImageId(const int64 account_id, const bool def, const bool ignoreClosure)
{
    wxString acctStatus = VIEW_ACCOUNTS_OPEN_STR;
    NavigatorTypes::TYPE_ID acctType = NavigatorTypes::TYPE_ID_CHECKING;
    int selectedImage = img::SAVINGS_ACC_NORMAL_PNG; //Default value

    AccountModel::Data* account = AccountModel::instance().cache_id(account_id);
    if (account)
    {
        acctType = AccountModel::type_id(account);
        acctStatus = account->STATUS;
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

const wxString PreferencesModel::getLanguageCode(const bool get_db)
{
    PreferencesModel::getLanguageID(get_db);
    if (m_language == wxLANGUAGE_UNKNOWN)
        return wxEmptyString;
    if (m_language == wxLANGUAGE_DEFAULT)
        return wxTranslations::Get()->GetBestTranslation("mmex", wxLANGUAGE_ENGLISH_US);

    const auto lang = wxLocale::GetLanguageCanonicalName(m_language);
    return lang;
}

void PreferencesModel::setLanguage(const wxLanguage& language)
{
    m_language = language;
    SettingModel::instance().setString(
        LANGUAGE_PARAMETER,
        wxLocale::GetLanguageCanonicalName(language)
    );
}

void PreferencesModel::loadShowNavigatorCashLedger()
{
    m_show_navigator_cashLedger = SettingModel::instance().getBool("NAVIGATOR_SHOW_CASHLEDGER", true);
}
void PreferencesModel::setShowNavigatorCashLedger(const bool value)
{
    SettingModel::instance().setBool("NAVIGATOR_SHOW_CASHLEDGER", value);
    m_show_navigator_cashLedger = value;
}
