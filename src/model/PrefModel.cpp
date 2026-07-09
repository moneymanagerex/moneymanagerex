/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016 - 2021 Nikolay Akimov
 Copyright (C) 2021-2025 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2026 George Ef (george.a.ef@gmail.com)
 Copyright (C) 2026 Klaus Wich

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

#include "PrefModel.h"

#include "base/_constants.h"
#include "util/mmImage.h"
#include "util/mmNavigatorList.h"
#include "util/_util.h"
#include "base/mmSingleton.h"

#include "AccountModel.h"
#include "CurrencyHistoryModel.h"
#include "CurrencyModel.h"
#include "InfoModel.h"
#include "SettingModel.h"

#include "dialog/CurrencyChoiceDialog.h"

// -- static

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

// -- constructor

PrefModel::PrefModel() :
    m_date_format(mmex::DEFDATEFORMAT),
    m_financial_first_day(1)
{
}

PrefModel& PrefModel::instance()
{
    return Singleton<PrefModel>::instance();
}

// -- methods

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
                saveBaseCurrencyID(m_base_currency_id);
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
    loadDoPanelResize();
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
    loadTrxStatus();
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
        auto lang_id = SettingModel::instance().getInt("LANGUAGE", -1);

        if (lang_id == -1) {
            auto lang_canonical = SettingModel::instance()
                .getString("LANGUAGE", wxLocale::GetLanguageCanonicalName(wxLANGUAGE_UNKNOWN));

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

void PrefModel::saveLocaleName(const wxString& locale)
{
    InfoModel::instance().saveString("LOCALE", locale);
    m_locale_name = locale;
}

void PrefModel::loadDateFormat()
{
    m_date_format = InfoModel::instance().getString("DATEFORMAT", mmex::DEFDATEFORMAT);
}
void PrefModel::saveDateFormat(const wxString& date_format)
{
    InfoModel::instance().saveString("DATEFORMAT", date_format);
    m_date_format = date_format;
}

void PrefModel::loadUserName()
{
    m_user_name = InfoModel::instance().getString("USERNAME", "");
}
void PrefModel::saveUserName(const wxString& username)
{
    m_user_name = username;
    InfoModel::instance().saveString("USERNAME", username);
}

void PrefModel::loadBaseCurrencyID()
{
    m_base_currency_id = InfoModel::instance().getInt64("BASECURRENCYID", -1);
}
void PrefModel::saveBaseCurrencyID(const int64 base_currency_id)
{
    InfoModel::instance().saveInt64("BASECURRENCYID", base_currency_id);
    m_base_currency_id = base_currency_id;
}

void PrefModel::loadUseCurrencyHistory()
{
    m_use_currency_history = InfoModel::instance().getBool("USECURRENCYHISTORY", true);
}
void PrefModel::saveUseCurrencyHistory(const bool value)
{
    InfoModel::instance().saveBool("USECURRENCYHISTORY", value);
    m_use_currency_history = value;
}

void PrefModel::loadSharePrecision()
{
    m_share_precision = InfoModel::instance().getInt("SHARE_PRECISION", 4);
}
void PrefModel::saveSharePrecision(const int value)
{
    InfoModel::instance().saveInt("SHARE_PRECISION", value);
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
void PrefModel::saveAssetCompounding(const int value)
{
    InfoModel::instance().saveString("ASSET_COMPOUNDING", PrefModel::COMPOUNDING_NAME[value].second);
    m_asset_compounding = value;
}

void PrefModel::loadReportingFirstDay()
{
    int value = InfoModel::instance().getInt("REPORTING_FIRSTDAY", 1);
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    m_reporting_first_day = value;
}
void PrefModel::saveReportingFirstDay(int value)
{
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    InfoModel::instance().saveInt("REPORTING_FIRSTDAY", value);
    m_reporting_first_day = value;
}

void PrefModel::loadReportingFirstWeekday()
{
    wxString valueStr = InfoModel::instance().getString("REPORTING_FIRST_WEEKDAY", "");
    m_reporting_first_weekday =
        (valueStr == "Mon") ? wxDateTime::WeekDay::Mon :
        wxDateTime::WeekDay::Sun;
}
void PrefModel::saveReportingFirstWeekday(wxDateTime::WeekDay value)
{
    if (value != wxDateTime::WeekDay::Mon)
        value = wxDateTime::WeekDay::Sun;
    InfoModel::instance().saveString("REPORTING_FIRST_WEEKDAY", g_short_days_of_week[value]);
    m_reporting_first_weekday = value;
}

void PrefModel::loadFinancialFirstDay()
{
    int value = InfoModel::instance().getInt("FINANCIAL_YEAR_START_DAY", 1);
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    m_financial_first_day = value;
}
void PrefModel::saveFinancialFirstDay(int value)
{
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    InfoModel::instance().saveInt("FINANCIAL_YEAR_START_DAY", value);
    m_financial_first_day = value;
}

void PrefModel::loadFinancialFirstMonth()
{
    int value = InfoModel::instance().getInt("FINANCIAL_YEAR_START_MONTH", 7);
    if (value < 1) value = 1;
    if (value > 12) value = 12;
    m_financial_first_month = wxDateTime::Month(value - 1);
}
void PrefModel::saveFinancialFirstMonth(const wxDateTime::Month value)
{
    wxString valueStr = wxString::Format("%d", value + 1);
    InfoModel::instance().saveString("FINANCIAL_YEAR_START_MONTH", valueStr);
    m_financial_first_month = value;
}

void PrefModel::loadBudgetDaysOffset()
{
    m_budget_days_offset = InfoModel::instance().getInt("BUDGET_DAYS_OFFSET", 0);
}
void PrefModel::saveBudgetDaysOffset(const int value)
{
    InfoModel::instance().saveInt("BUDGET_DAYS_OFFSET", value);
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
void PrefModel::saveHomePageIncExpRange(const int value)
{
    InfoModel::instance().saveInt("HOMEPAGE_INCEXP_RANGE", value);
    m_homepage_incexp_range = value;
}

void PrefModel::loadHideShareAccounts()
{
    m_hide_share_accounts = SettingModel::instance().getBool("HIDE_SHARE_ACCOUNTS", true);
}
void PrefModel::saveHideShareAccounts(const bool value)
{
    SettingModel::instance().saveBool("HIDE_SHARE_ACCOUNTS", value);
    m_hide_share_accounts = value;
}

void PrefModel::loadHideDeletedTransactions()
{
    m_hide_deleted_transactions = SettingModel::instance().getBool("HIDE_DELETED_TRANSACTIONS", false);
}
void PrefModel::saveHideDeletedTransactions(const bool value)
{
    SettingModel::instance().saveBool("HIDE_DELETED_TRANSACTIONS", value);
    m_hide_deleted_transactions = value;
}

void PrefModel::loadBudgetFinancialYears()
{
    m_budget_financial_years = SettingModel::instance().getBool("BUDGET_FINANCIAL_YEARS", false);
}
void PrefModel::saveBudgetFinancialYears(const bool value)
{
    SettingModel::instance().saveBool("BUDGET_FINANCIAL_YEARS", value);
    m_budget_financial_years = value;
}

void PrefModel::loadBudgetIncludeTransfers()
{
    m_budget_include_transfers = SettingModel::instance().getBool("BUDGET_INCLUDE_TRANSFERS", false);
}
void PrefModel::saveBudgetIncludeTransfers(const bool value)
{
    SettingModel::instance().saveBool("BUDGET_INCLUDE_TRANSFERS", value);
    m_budget_include_transfers = value;
}

void PrefModel::loadBudgetSummaryWithoutCategories()
{
    m_budget_summary_without_categories = SettingModel::instance().getBool("BUDGET_SUMMARY_WITHOUT_CATEGORIES", true);
}
void PrefModel::saveBudgetSummaryWithoutCategories(bool value)
{
    SettingModel::instance().saveBool("BUDGET_SUMMARY_WITHOUT_CATEGORIES", value);
    m_budget_summary_without_categories = value;
}

void PrefModel::loadBudgetOverride()
{
    m_budget_override = SettingModel::instance().getBool("BUDGET_OVERRIDE", false);
}
void PrefModel::saveBudgetOverride(const bool value)
{
    SettingModel::instance().saveBool("BUDGET_OVERRIDE", value);
    m_budget_override = value;
}

void PrefModel::loadBudgetDeductMonthly()
{
    m_budget_deduct_monthly = SettingModel::instance().getBool("BUDGET_DEDUCT_MONTH_FROM_YEAR", false);
}
void PrefModel::saveBudgetDeductMonthly(bool value)
{
    SettingModel::instance().saveBool("BUDGET_DEDUCT_MONTH_FROM_YEAR", value);
    m_budget_deduct_monthly = value;
}

void PrefModel::loadIgnoreFutureTransactions()
{
    m_ignore_future_transactions = SettingModel::instance().getBool("IGNORE_FUTURE_TRANSACTIONS", false);
}
void PrefModel::saveIgnoreFutureTransactions(const bool value)
{
    SettingModel::instance().saveBool("IGNORE_FUTURE_TRANSACTIONS", value);
    m_ignore_future_transactions = value;
}

void PrefModel::loadIgnoreFutureTransactionsHomePage()
{
    m_ignore_future_transactions_home = SettingModel::instance().getBool("IGNORE_FUTURE_TRANSACTIONS_HOMEPAGE", true);
}
void PrefModel::saveIgnoreFutureTransactionsHomePage(const bool value)
{
    SettingModel::instance().saveBool("IGNORE_FUTURE_TRANSACTIONS_HOMEPAGE", value);
    m_ignore_future_transactions_home = value;
}

void PrefModel::loadShowReconciledInHomePage()
{
    m_show_reconciled_in_home_page = SettingModel::instance().getBool("SHOW_RECONCILED_IN_HOME_PAGE", true);
}
void PrefModel::saveShowReconciledInHomePage(const bool value)
{
    SettingModel::instance().saveBool("SHOW_RECONCILED_IN_HOME_PAGE", value);
    m_show_reconciled_in_home_page = value;
}

void PrefModel::loadUseTransDateTime()
{
    m_use_trans_datetime = SettingModel::instance().getBool("TRANSACTION_USE_DATE_TIME", false);
}

bool PrefModel::saveUseTransDateTime(const bool value)
{
    if (value != m_use_trans_datetime) {
        SettingModel::instance().saveBool("TRANSACTION_USE_DATE_TIME", value);
        m_use_trans_datetime = value;
        return true;
    }
    return false;
}

void PrefModel::loadTreatDateAsSN()
{
    m_treat_date_as_SN = SettingModel::instance().getBool("TRANSACTION_TREAT_DATE_AS_SN", true);
}

bool PrefModel::saveTreatDateAsSN(const bool value)
{
    if (value != m_treat_date_as_SN) {
        SettingModel::instance().saveBool("TRANSACTION_TREAT_DATE_AS_SN", value);
        m_treat_date_as_SN = value;
        return true;
    }
    return false;
}

void PrefModel::loadDoNotColorFuture()
{
    m_do_not_color_future = SettingModel::instance().getBool("DO_NOT_COLOR_FUTURE_TRANSACTIONS", true);
}
void PrefModel::saveDoNotColorFuture(const bool value)
{
    SettingModel::instance().saveBool("DO_NOT_COLOR_FUTURE_TRANSACTIONS", value);
    m_do_not_color_future = value;
}

void PrefModel::loadDoSpecialColorReconciled()
{
    m_do_special_color_reconciled = SettingModel::instance().getBool("SPECIAL_COLOR_RECONCILED_TRANSACTIONS", true);
}
void PrefModel::saveDoSpecialColorReconciled(const bool value)
{
    SettingModel::instance().saveBool("SPECIAL_COLOR_RECONCILED_TRANSACTIONS", value);
    m_do_special_color_reconciled = value;
}

void PrefModel::loadDoPanelResize()
{
    m_do_panel_resize = SettingModel::instance().getBool("RESIZE_TRANSACTION_PANEL", false);
}
void PrefModel::saveDoPanelResize(const bool value)
{
    SettingModel::instance().saveBool("RESIZE_TRANSACTION_PANEL", value);
    m_do_panel_resize = value;
}


void PrefModel::loadUsePerAccountFilter()
{
    m_store_account_specific_filter = SettingModel::instance().getBool("USE_PER_ACCOUNT_FILTER", true);
}
void PrefModel::saveUsePerAccountFilter(const bool value)
{
    SettingModel::instance().saveBool("USE_PER_ACCOUNT_FILTER", value);
    m_store_account_specific_filter = value;
}


void PrefModel::loadShowToolTips()
{
    m_show_tooltips = SettingModel::instance().getBool("IGNORE_SHOW_TOOLTIPS", true);
}
void PrefModel::saveShowToolTips(const bool value)
{
    SettingModel::instance().saveBool("IGNORE_SHOW_TOOLTIPS", value);
    m_show_tooltips = value;
}

void PrefModel::loadShowMoneyTips()
{
    m_show_moneytips = SettingModel::instance().getBool("IGNORE_SHOW_MONEYTIPS", true);
}
void PrefModel::saveShowMoneyTips(const bool value)
{
    SettingModel::instance().saveBool("IGNORE_SHOW_MONEYTIPS", value);
    m_show_moneytips = value;
}

void PrefModel::loadTransPayeeNone()
{
    // Read the preference as a string and convert to int
    m_trans_payee_none = SettingModel::instance().getInt("TRANSACTION_PAYEE_NONE", PrefModel::NONE);
}
void PrefModel::saveTransPayeeNone(const int value)
{
    SettingModel::instance().saveInt("TRANSACTION_PAYEE_NONE", value);
    m_trans_payee_none = value;
}

void PrefModel::loadTransCategoryNone()
{
    m_trans_category_none = SettingModel::instance().getInt("TRANSACTION_CATEGORY_NONE", PrefModel::LASTUSED);
}
void PrefModel::saveTransCategoryNone(const int value)
{
    SettingModel::instance().saveInt("TRANSACTION_CATEGORY_NONE", value);
    m_trans_category_none = value;
}

void PrefModel::loadTransCategoryTransferNone()
{
    m_trans_category_transfer_none = SettingModel::instance().getInt(
        "TRANSACTION_CATEGORY_TRANSFER_NONE",
        PrefModel::LASTUSED
    );
}
void PrefModel::saveTransCategoryTransferNone(const int value)
{
    SettingModel::instance().saveInt("TRANSACTION_CATEGORY_TRANSFER_NONE", value);
    m_trans_category_transfer_none = value;
}

void PrefModel::loadTrxStatus()
{
    m_trx_status = TrxStatus(SettingModel::instance().getInt(
        "TRANSACTION_STATUS_RECONCILED",
        TrxStatus().id()
    ));
}
void PrefModel::saveTrxStatus(const TrxStatus value)
{
    SettingModel::instance().saveInt("TRANSACTION_STATUS_RECONCILED", value.id());
    m_trx_status = value;
}

void PrefModel::loadTransDateDefault()
{
    m_trans_date_default = SettingModel::instance().getInt(
        "TRANSACTION_DATE_DEFAULT",
        PrefModel::NONE
    );
}
void PrefModel::saveTransDateDefault(const int value)
{
    SettingModel::instance().saveInt("TRANSACTION_DATE_DEFAULT", value);
    m_trans_date_default = value;
}

void PrefModel::loadSendUsageStats()
{
    m_send_usage_stats = SettingModel::instance().getBool(INIDB_SEND_USAGE_STATS, true);
}
void PrefModel::saveSendUsageStats(const bool value)
{
    SettingModel::instance().saveBool(INIDB_SEND_USAGE_STATS, value);
    m_send_usage_stats = value;
}

void PrefModel::loadCheckNews()
{
    m_check_news = SettingModel::instance().getBool("CHECKNEWS", true);
}
void PrefModel::saveCheckNews(const bool value)
{
    SettingModel::instance().saveBool("CHECKNEWS", value);
    m_check_news = value;
}

void PrefModel::loadThemeMode()
{
    m_theme_mode = SettingModel::instance().getInt("THEMEMODE", PrefModel::THEME_MODE::AUTO);
}
void PrefModel::saveThemeMode(const int value)
{
    SettingModel::instance().saveInt("THEMEMODE", value);
    m_theme_mode = value;
}

void PrefModel::loadHtmlScale()
{
    m_html_scale = SettingModel::instance().getInt("HTMLSCALE", 100);
}
void PrefModel::saveHtmlScale(const int value)
{
    SettingModel::instance().saveInt("HTMLSCALE", value);
    m_html_scale = value;
}

void PrefModel::loadFontSize()
{
    m_font_size = SettingModel::instance().getInt("UI_FONT_SIZE", 0);
}
void PrefModel::saveFontSize(const int value)
{
    SettingModel::instance().saveInt("UI_FONT_SIZE", value);
    m_font_size = value;
}

void PrefModel::loadIconSize()
{
    m_icon_size = SettingModel::instance().getInt("ICONSIZE", 16);
}
void PrefModel::saveIconSize(const int value)
{
    SettingModel::instance().saveInt("ICONSIZE", value);
    m_icon_size = value;
}

void PrefModel::loadToolbarIconSize()
{
    m_toolbar_icon_size = SettingModel::instance().getInt("TOOLBARICONSIZE", 32);
}
void PrefModel::saveToolbarIconSize(const int value)
{
    SettingModel::instance().saveInt("TOOLBARICONSIZE", value);
    m_toolbar_icon_size = value;
}

void PrefModel::loadNavigationIconSize()
{
    m_navigation_icon_size = SettingModel::instance().getInt("NAVIGATIONICONSIZE", 24);
}
void PrefModel::saveNavigationIconSize(const int value)
{
    SettingModel::instance().saveInt("NAVIGATIONICONSIZE", value);
    m_navigation_icon_size = value;
}

void PrefModel::loadCheckingRange()
{
    m_checking_range = SettingModel::instance().getArrayString("CHECKING_RANGE");
    parseCheckingRange();
}
void PrefModel::saveCheckingRange(const wxArrayString &a)
{
    SettingModel::instance().saveArrayString("CHECKING_RANGE", a);
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
void PrefModel::saveReportingRange(const wxArrayString &a)
{
    SettingModel::instance().saveArrayString("REPORTING_RANGE", a);
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
    AccountStatus acctStatus = AccountStatus(AccountStatus::e_open);
    mmNavigatorItem::TYPE_ID acctType = mmNavigatorItem::TYPE_ID_CHECKING;

    const AccountData* account_n = AccountModel::instance().get_idN_data_n(account_id);
    if (account_n) {
        acctType = AccountModel::type_id(*account_n);
        acctStatus = account_n->m_status;
    }

    if (!def && !ignoreClosure && acctStatus.id() == AccountStatus::e_closed) {
        return mmImage::img::ACCOUNT_CLOSED_PNG;
    }

    // check for custom id if default is not requested:
    if (!def) {
        wxString timg = InfoModel::instance().getString(wxString::Format("ACC_IMAGE_ID_%lld", account_id), "0");
        int custom_img_id = NavTreeIconImages::instance().getImgIndexFromStorageString(timg);
        int bmListSize = std::max (NavTreeIconImages::instance().getListSize(), static_cast<int>(mmImage::acc_img::MAX_ACC_ICON));
        if (custom_img_id > 0 && custom_img_id < bmListSize) {
            return custom_img_id;
        }
    }

    mmNavigatorItem* info = mmNavigatorList::instance().FindEntry(acctType);
    return info ? info->imageId : mmImage::img::SAVINGS_ACC_NORMAL_PNG;
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

void PrefModel::saveLanguage(const wxLanguage& language)
{
    m_language = language;
    SettingModel::instance().saveString(
        "LANGUAGE",
        wxLocale::GetLanguageCanonicalName(language)
    );
}

void PrefModel::loadShowNavigatorCashLedger()
{
    m_show_navigator_cashLedger = SettingModel::instance().getBool("NAVIGATOR_SHOW_CASHLEDGER", true);
}
void PrefModel::saveShowNavigatorCashLedger(const bool value)
{
    SettingModel::instance().saveBool("NAVIGATOR_SHOW_CASHLEDGER", value);
    m_show_navigator_cashLedger = value;
}
