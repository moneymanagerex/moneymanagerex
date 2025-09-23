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

#include "option.h"
#include "constants.h"
#include "util.h"
#include "images_list.h"
#include "singleton.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include "model/Model_Account.h"
#include "maincurrencydialog.h"
#include "model/Model_Currency.h"
#include "model/Model_CurrencyHistory.h"

const std::vector<std::pair<Option::COMPOUNDING_ID, wxString> > Option::COMPOUNDING_NAME =
{
    { Option::COMPOUNDING_ID_DAY,   _n("Day") },
    { Option::COMPOUNDING_ID_WEEK,  _n("Week") },
    { Option::COMPOUNDING_ID_MONTH, _n("Month") },
    { Option::COMPOUNDING_ID_YEAR,  _n("Year") },
};
const std::vector<std::pair<Option::COMPOUNDING_ID, int> > Option::COMPOUNDING_N =
{
    { Option::COMPOUNDING_ID_DAY,   365 },
    { Option::COMPOUNDING_ID_WEEK,  52 },
    { Option::COMPOUNDING_ID_MONTH, 12 },
    { Option::COMPOUNDING_ID_YEAR,  1 },
};

const std::vector<std::pair<wxString, wxString> > Option::CHECKING_RANGE_DEFAULT =
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

//----------------------------------------------------------------------------
Option::Option()
:   m_date_format(mmex::DEFDATEFORMAT)
{}

//----------------------------------------------------------------------------
Option& Option::instance()
{
    return Singleton<Option>::instance();
}

//----------------------------------------------------------------------------
void Option::load(bool include_infotable)
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
            if (mmMainCurrencyDialog::Execute(m_base_currency_id)) {
                setBaseCurrencyID(m_base_currency_id);
                Model_CurrencyHistory::ResetCurrencyHistory();
                Model_Currency::ResetBaseConversionRates();
            }
        }
    }

    m_language = Option::instance().getLanguageID(true);

    loadHideShareAccounts();
    loadHideDeletedTransactions();
    loadBudgetFinancialYears();
    loadBudgetIncludeTransfers();
    loadBudgetSummaryWithoutCategories();
    loadBudgetOverride();
    loadBudgetDeductMonthly();
    loadIgnoreFutureTransactions();
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
}

wxLanguage Option::getLanguageID(const bool get_db)
{
    if (get_db) {
        auto lang_id = Model_Setting::instance().getInt(LANGUAGE_PARAMETER, -1);

        if (lang_id == -1) {
            auto lang_canonical = Model_Setting::instance()
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

void Option::setLocaleName(const wxString& locale)
{
    Model_Infotable::instance().setString("LOCALE", locale);
    m_locale_name = locale;
}

void Option::loadDateFormat()
{
    m_date_format = Model_Infotable::instance().getString("DATEFORMAT", mmex::DEFDATEFORMAT);
}
void Option::setDateFormat(const wxString& date_format)
{
    Model_Infotable::instance().setString("DATEFORMAT", date_format);
    m_date_format = date_format;
}

void Option::loadUserName()
{
    m_user_name = Model_Infotable::instance().getString("USERNAME", "");
}
void Option::setUserName(const wxString& username)
{
    m_user_name = username;
    Model_Infotable::instance().setString("USERNAME", username);
}

void Option::loadBaseCurrencyID()
{
    m_base_currency_id = Model_Infotable::instance().getInt64("BASECURRENCYID", -1);
}
void Option::setBaseCurrencyID(const int64 base_currency_id)
{
    Model_Infotable::instance().setInt64("BASECURRENCYID", base_currency_id);
    m_base_currency_id = base_currency_id;
}

void Option::loadUseCurrencyHistory()
{
    m_use_currency_history = Model_Infotable::instance().getBool("USECURRENCYHISTORY", true);
}
void Option::setUseCurrencyHistory(const bool value)
{
    Model_Infotable::instance().setBool("USECURRENCYHISTORY", value);
    m_use_currency_history = value;
}

void Option::loadSharePrecision()
{
    m_share_precision = Model_Infotable::instance().getInt("SHARE_PRECISION", 4);
}
void Option::setSharePrecision(const int value)
{
    Model_Infotable::instance().setInt("SHARE_PRECISION", value);
    m_share_precision = value;
}

void Option::loadAssetCompounding()
{
    wxString assetCompounding = Model_Infotable::instance().getString("ASSET_COMPOUNDING", "Day");
    m_asset_compounding = Option::COMPOUNDING_ID_DAY;
    for (const auto& a : Option::COMPOUNDING_NAME) if (assetCompounding == a.second) {
        m_asset_compounding = a.first;
        break;
    }
}
void Option::setAssetCompounding(const int value)
{
    Model_Infotable::instance().setString("ASSET_COMPOUNDING", Option::COMPOUNDING_NAME[value].second);
    m_asset_compounding = value;
}

void Option::loadReportingFirstDay()
{
    int value = Model_Infotable::instance().getInt("REPORTING_FIRSTDAY", 1);
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    m_reporting_first_day = value;
}
void Option::setReportingFirstDay(int value)
{
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    Model_Infotable::instance().setInt("REPORTING_FIRSTDAY", value);
    m_reporting_first_day = value;
}

void Option::loadReportingFirstWeekday()
{
    wxString valueStr = Model_Infotable::instance().getString("REPORTING_FIRST_WEEKDAY", "");
    m_reporting_first_weekday =
        (valueStr == "Mon") ? wxDateTime::WeekDay::Mon :
        wxDateTime::WeekDay::Sun;
}
void Option::setReportingFirstWeekday(wxDateTime::WeekDay value)
{
    if (value != wxDateTime::WeekDay::Mon)
        value = wxDateTime::WeekDay::Sun;
    Model_Infotable::instance().setString("REPORTING_FIRST_WEEKDAY", g_short_days_of_week[value]);
    m_reporting_first_weekday = value;
}

void Option::loadFinancialFirstDay()
{
    int value = Model_Infotable::instance().getInt("FINANCIAL_YEAR_START_DAY", 1);
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    m_financial_first_day = value;
}
void Option::setFinancialFirstDay(int value)
{
    if (value < 1) value = 1;
    if (value > 28) value = 28;
    Model_Infotable::instance().setInt("FINANCIAL_YEAR_START_DAY", value);
    m_financial_first_day = value;
}

void Option::loadFinancialFirstMonth()
{
    int value = Model_Infotable::instance().getInt("FINANCIAL_YEAR_START_MONTH", 7);
    if (value < 1) value = 1;
    if (value > 12) value = 12;
    m_financial_first_month = wxDateTime::Month(value - 1);
}
void Option::setFinancialFirstMonth(const wxDateTime::Month value)
{
    wxString valueStr = wxString::Format("%d", value + 1);
    Model_Infotable::instance().setString("FINANCIAL_YEAR_START_MONTH", valueStr);
    m_financial_first_month = value;
}

void Option::loadBudgetDaysOffset()
{
    m_budget_days_offset = Model_Infotable::instance().getInt("BUDGET_DAYS_OFFSET", 0);
}
void Option::setBudgetDaysOffset(const int value)
{
    Model_Infotable::instance().setInt("BUDGET_DAYS_OFFSET", value);
    m_budget_days_offset = value;
}
void Option::addBudgetDateOffset(wxDateTime& date) const
{
    if (m_budget_days_offset != 0)
        date.Add(wxDateSpan::Days(m_budget_days_offset));
}

void Option::loadHomePageIncExpRange()
{
    m_homepage_incexp_range = Model_Infotable::instance().getInt("HOMEPAGE_INCEXP_RANGE", 0);
}
void Option::setHomePageIncExpRange(const int value)
{
    Model_Infotable::instance().setInt("HOMEPAGE_INCEXP_RANGE", value);
    m_homepage_incexp_range = value;
}

void Option::loadHideShareAccounts()
{
    m_hide_share_accounts = Model_Setting::instance().getBool("HIDE_SHARE_ACCOUNTS", true);
}
void Option::setHideShareAccounts(const bool value)
{
    Model_Setting::instance().setBool("HIDE_SHARE_ACCOUNTS", value);
    m_hide_share_accounts = value;
}

void Option::loadHideDeletedTransactions()
{
    m_hide_deleted_transactions = Model_Setting::instance().getBool("HIDE_DELETED_TRANSACTIONS", false);
}
void Option::setHideDeletedTransactions(const bool value)
{
    Model_Setting::instance().setBool("HIDE_DELETED_TRANSACTIONS", value);
    m_hide_deleted_transactions = value;
}

void Option::loadBudgetFinancialYears()
{
    m_budget_financial_years = Model_Setting::instance().getBool("BUDGET_FINANCIAL_YEARS", false);
}
void Option::setBudgetFinancialYears(const bool value)
{
    Model_Setting::instance().setBool("BUDGET_FINANCIAL_YEARS", value);
    m_budget_financial_years = value;
}

void Option::loadBudgetIncludeTransfers()
{
    m_budget_include_transfers = Model_Setting::instance().getBool("BUDGET_INCLUDE_TRANSFERS", false);
}
void Option::setBudgetIncludeTransfers(const bool value)
{
    Model_Setting::instance().setBool("BUDGET_INCLUDE_TRANSFERS", value);
    m_budget_include_transfers = value;
}

void Option::loadBudgetSummaryWithoutCategories()
{
    m_budget_summary_without_categories = Model_Setting::instance().getBool("BUDGET_SUMMARY_WITHOUT_CATEGORIES", true);
}
void Option::setBudgetSummaryWithoutCategories(bool value)
{
    Model_Setting::instance().setBool("BUDGET_SUMMARY_WITHOUT_CATEGORIES", value);
    m_budget_summary_without_categories = value;
}

void Option::loadBudgetOverride()
{
    m_budget_override = Model_Setting::instance().getBool("BUDGET_OVERRIDE", false);
}
void Option::setBudgetOverride(const bool value)
{
    Model_Setting::instance().setBool("BUDGET_OVERRIDE", value);
    m_budget_override = value;
}

void Option::loadBudgetDeductMonthly()
{
    m_budget_deduct_monthly = Model_Setting::instance().getBool("BUDGET_DEDUCT_MONTH_FROM_YEAR", false);
}
void Option::setBudgetDeductMonthly(bool value)
{
    Model_Setting::instance().setBool("BUDGET_DEDUCT_MONTH_FROM_YEAR", value);
    m_budget_deduct_monthly = value;
}

void Option::loadIgnoreFutureTransactions()
{
    m_ignore_future_transactions = Model_Setting::instance().getBool("IGNORE_FUTURE_TRANSACTIONS", false);
}
void Option::setIgnoreFutureTransactions(const bool value)
{
    Model_Setting::instance().setBool("IGNORE_FUTURE_TRANSACTIONS", value);
    m_ignore_future_transactions = value;
}

void Option::loadUseTransDateTime()
{
    m_use_trans_datetime = Model_Setting::instance().getBool("TRANSACTION_USE_DATE_TIME", false);
}

bool Option::UseTransDateTime(const bool value)
{
    if (value != m_use_trans_datetime) {
        Model_Setting::instance().setBool("TRANSACTION_USE_DATE_TIME", value);
        m_use_trans_datetime = value;
        return true;
    }
    return false;
}

void Option::loadTreatDateAsSN()
{
    m_treat_date_as_SN = Model_Setting::instance().getBool("TRANSACTION_TREAT_DATE_AS_SN", true);
}

bool Option::TreatDateAsSN(const bool value)
{
    if (value != m_treat_date_as_SN) {
        Model_Setting::instance().setBool("TRANSACTION_TREAT_DATE_AS_SN", value);
        m_treat_date_as_SN = value;
        return true;
    }
    return false;
}

void Option::loadDoNotColorFuture()
{
    m_do_not_color_future = Model_Setting::instance().getBool("DO_NOT_COLOR_FUTURE_TRANSACTIONS", true);
}
void Option::setDoNotColorFuture(const bool value)
{
    Model_Setting::instance().setBool("DO_NOT_COLOR_FUTURE_TRANSACTIONS", value);
    m_do_not_color_future = value;
}

void Option::loadDoSpecialColorReconciled()
{
    m_do_special_color_reconciled = Model_Setting::instance().getBool("SPECIAL_COLOR_RECONCILED_TRANSACTIONS", true);
}
void Option::setDoSpecialColorReconciled(const bool value)
{
    Model_Setting::instance().setBool("SPECIAL_COLOR_RECONCILED_TRANSACTIONS", value);
    m_do_special_color_reconciled = value;
}

void Option::loadUsePerAccountFilter()
{
    m_store_account_specific_filter = Model_Setting::instance().getBool("USE_PER_ACCOUNT_FILTER", true);
}
void Option::setUsePerAccountFilter(const bool value)
{
    Model_Setting::instance().setBool("USE_PER_ACCOUNT_FILTER", value);
    m_store_account_specific_filter = value;
}


void Option::loadShowToolTips()
{
    m_show_tooltips = Model_Setting::instance().getBool("IGNORE_SHOW_TOOLTIPS", true);
}
void Option::setShowToolTips(const bool value)
{
    Model_Setting::instance().setBool("IGNORE_SHOW_TOOLTIPS", value);
    m_show_tooltips = value;
}

void Option::loadShowMoneyTips()
{
    m_show_moneytips = Model_Setting::instance().getBool("IGNORE_SHOW_MONEYTIPS", true);
}
void Option::setShowMoneyTips(const bool value)
{
    Model_Setting::instance().setBool("IGNORE_SHOW_MONEYTIPS", value);
    m_show_moneytips = value;
}

void Option::loadTransPayeeNone()
{
    // Read the preference as a string and convert to int
    m_trans_payee_none = Model_Setting::instance().getInt("TRANSACTION_PAYEE_NONE", Option::NONE);
}
void Option::setTransPayeeNone(const int value)
{
    Model_Setting::instance().setInt("TRANSACTION_PAYEE_NONE", value);
    m_trans_payee_none = value;
}

void Option::loadTransCategoryNone()
{
    m_trans_category_none = Model_Setting::instance().getInt("TRANSACTION_CATEGORY_NONE", Option::LASTUSED);
}
void Option::setTransCategoryNone(const int value)
{
    Model_Setting::instance().setInt("TRANSACTION_CATEGORY_NONE", value);
    m_trans_category_none = value;
}

void Option::loadTransCategoryTransferNone()
{
    m_trans_category_transfer_none = Model_Setting::instance().getInt("TRANSACTION_CATEGORY_TRANSFER_NONE", Option::LASTUSED);
}
void Option::setTransCategoryTransferNone(const int value)
{
    Model_Setting::instance().setInt("TRANSACTION_CATEGORY_TRANSFER_NONE", value);
    m_trans_category_transfer_none = value;
}

void Option::loadTransStatusReconciled()
{
    m_trans_status_reconciled = Model_Setting::instance().getInt("TRANSACTION_STATUS_RECONCILED", Option::NONE);
}
void Option::setTransStatusReconciled(const int value)
{
    Model_Setting::instance().setInt("TRANSACTION_STATUS_RECONCILED", value);
    m_trans_status_reconciled = value;
}

void Option::loadTransDateDefault()
{
    m_trans_date_default = Model_Setting::instance().getInt("TRANSACTION_DATE_DEFAULT", Option::NONE);
}
void Option::setTransDateDefault(const int value)
{
    Model_Setting::instance().setInt("TRANSACTION_DATE_DEFAULT", value);
    m_trans_date_default = value;
}

void Option::loadSendUsageStats()
{
    m_send_usage_stats = Model_Setting::instance().getBool(INIDB_SEND_USAGE_STATS, true);
}
void Option::setSendUsageStats(const bool value)
{
    Model_Setting::instance().setBool(INIDB_SEND_USAGE_STATS, value);
    m_send_usage_stats = value;
}

void Option::loadCheckNews()
{
    m_check_news = Model_Setting::instance().getBool("CHECKNEWS", true);
}
void Option::setCheckNews(const bool value)
{
    Model_Setting::instance().setBool("CHECKNEWS", value);
    m_check_news = value;
}

void Option::loadThemeMode()
{
    m_theme_mode = Model_Setting::instance().getInt("THEMEMODE", Option::THEME_MODE::AUTO);
}
void Option::setThemeMode(const int value)
{
    Model_Setting::instance().setInt("THEMEMODE", value);
    m_theme_mode = value;
}

void Option::loadHtmlScale()
{
    m_html_scale = Model_Setting::instance().getInt("HTMLSCALE", 100);
}
void Option::setHtmlScale(const int value)
{
    Model_Setting::instance().setInt("HTMLSCALE", value);
    m_html_scale = value;
}

void Option::loadFontSize()
{
    m_font_size = Model_Setting::instance().getInt("UI_FONT_SIZE", 0);
}
void Option::setFontSize(const int value)
{
    Model_Setting::instance().setInt("UI_FONT_SIZE", value);
    m_font_size = value;
}

void Option::loadIconSize()
{
    m_icon_size = Model_Setting::instance().getInt("ICONSIZE", 16);
}
void Option::setIconSize(const int value)
{
    Model_Setting::instance().setInt("ICONSIZE", value);
    m_icon_size = value;
}

void Option::loadToolbarIconSize()
{
    m_toolbar_icon_size = Model_Setting::instance().getInt("TOOLBARICONSIZE", 32);
}
void Option::setToolbarIconSize(const int value)
{
    Model_Setting::instance().setInt("TOOLBARICONSIZE", value);
    m_toolbar_icon_size = value;
}

void Option::loadNavigationIconSize()
{
    m_navigation_icon_size = Model_Setting::instance().getInt("NAVIGATIONICONSIZE", 24);
}
void Option::setNavigationIconSize(const int value)
{
    Model_Setting::instance().setInt("NAVIGATIONICONSIZE", value);
    m_navigation_icon_size = value;
}

void Option::loadCheckingRange()
{
    m_checking_range = Model_Setting::instance().getArrayString("CHECKING_RANGE");
    parseCheckingRange();
}
void Option::setCheckingRange(const wxArrayString &a)
{
    Model_Setting::instance().setArrayString("CHECKING_RANGE", a);
    m_checking_range = a;
    parseCheckingRange();
}
void Option::parseCheckingRange()
{
    wxLogDebug("{{{ Option::parseCheckingRange()");

    m_checking_range_a.clear();
    m_checking_range_m = 0;

    for (wxString &str : m_checking_range) {
        if (str.empty()) {
            if (m_checking_range_m == 0)
                m_checking_range_m = m_checking_range_a.size();
            continue;
        }
        DateRange2::Spec spec;
        if (!spec.parseSpec(str))
            continue;
        m_checking_range_a.push_back(spec);
    }

    if (!m_checking_range_a.empty())
        goto done;

    for (auto &range : CHECKING_RANGE_DEFAULT) {
        wxString label = range.first;
        if (label.empty()) {
            if (m_checking_range_m == 0)
                m_checking_range_m = m_checking_range_a.size();
            continue;
        }
        wxString name = wxGetTranslation(range.second);
        DateRange2::Spec spec;
        if (!spec.parseSpec(label, name))
            continue;
        m_checking_range_a.push_back(spec);
    }

    done:
    if (m_checking_range_m == 0)
        m_checking_range_m = m_checking_range_a.size();

    wxLogDebug("m=[%d], n=[%zu]", m_checking_range_m, m_checking_range_a.size());
    for ([[maybe_unused]] DateRange2::Spec &spec : m_checking_range_a) {
        wxLogDebug("label=[%s], name=[%s]", spec.getLabel(), spec.getName());
    }
    wxLogDebug("}}}");
}

int Option::getHtmlScale() const noexcept
{
    return m_html_scale;
}

int Option::AccountImageId(const int64 account_id, const bool def, const bool ignoreClosure)
{
    wxString acctStatus = VIEW_ACCOUNTS_OPEN_STR;
    Model_Account::TYPE_ID acctType = Model_Account::TYPE_ID_CHECKING;
    int selectedImage = img::SAVINGS_ACC_NORMAL_PNG; //Default value

    Model_Account::Data* account = Model_Account::instance().get(account_id);
    if (account)
    {
        acctType = Model_Account::type_id(account);
        acctStatus = account->STATUS;
    }

    if (!def && !ignoreClosure && (acctStatus == "Closed"))
        return img::ACCOUNT_CLOSED_PNG;

    int max = acc_img::MAX_ACC_ICON - static_cast<int>(img::LAST_NAVTREE_PNG);
    int min = 1;
    int custom_img_id = Model_Infotable::instance().getInt(wxString::Format("ACC_IMAGE_ID_%lld", account_id), 0);
    if (custom_img_id > max) custom_img_id = custom_img_id - 20; //Bug #963 fix
    if (!def && (custom_img_id >= min && custom_img_id <= max))
        return custom_img_id + img::LAST_NAVTREE_PNG - 1;

    switch (acctType)
    {
    case (Model_Account::TYPE_ID_CHECKING) :
        selectedImage = img::SAVINGS_ACC_NORMAL_PNG;
        break;
    case (Model_Account::TYPE_ID_TERM) :
        selectedImage = img::TERMACCOUNT_NORMAL_PNG;
        break;
    case (Model_Account::TYPE_ID_INVESTMENT) :
        selectedImage = img::STOCK_ACC_NORMAL_PNG;
        break;
    case (Model_Account::TYPE_ID_CREDIT_CARD) :
        selectedImage = img::CARD_ACC_NORMAL_PNG;
        break;
    case (Model_Account::TYPE_ID_CASH) :
        selectedImage = img::CASH_ACC_NORMAL_PNG;
        break;
    case (Model_Account::TYPE_ID_LOAN) :
        selectedImage = img::LOAN_ACC_NORMAL_PNG;
        break;
    case (Model_Account::TYPE_ID_ASSET) :
        selectedImage = img::ASSET_NORMAL_PNG;
        break;
    case (Model_Account::TYPE_ID_SHARES) :
        selectedImage = img::LOAN_ACC_NORMAL_PNG;
        break;
    default:
        wxASSERT(false);
    }
    return selectedImage;
}

const wxString Option::getLanguageCode(const bool get_db)
{
    Option::getLanguageID(get_db);
    if (m_language == wxLANGUAGE_UNKNOWN)
        return wxEmptyString;
    if (m_language == wxLANGUAGE_DEFAULT)
        return wxTranslations::Get()->GetBestTranslation("mmex", wxLANGUAGE_ENGLISH_US);

    const auto lang = wxLocale::GetLanguageCanonicalName(m_language);
    return lang;
}

void Option::setLanguage(const wxLanguage& language)
{
    m_language = language;
    Model_Setting::instance().setString(
        LANGUAGE_PARAMETER,
        wxLocale::GetLanguageCanonicalName(language)
    );
}
