/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016 - 2021 Nikolay Akimov
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


#include "option.h"
#include "constants.h"
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
    { Option::COMPOUNDING_ID_DAY,   wxString(wxTRANSLATE("Day")) },
    { Option::COMPOUNDING_ID_WEEK,  wxString(wxTRANSLATE("Week")) },
    { Option::COMPOUNDING_ID_MONTH, wxString(wxTRANSLATE("Month")) },
    { Option::COMPOUNDING_ID_YEAR,  wxString(wxTRANSLATE("Year")) },
};
const std::vector<std::pair<Option::COMPOUNDING_ID, int> > Option::COMPOUNDING_N =
{
    { Option::COMPOUNDING_ID_DAY,   365 },
    { Option::COMPOUNDING_ID_WEEK,  52 },
    { Option::COMPOUNDING_ID_MONTH, 12 },
    { Option::COMPOUNDING_ID_YEAR,  1 },
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
    loadBulkTransactions();
    loadFontSize();
}

wxLanguage Option::getLanguageID(const bool get_db)
{
    if (get_db) {
        auto lang_id = Model_Setting::instance()
            .GetIntSetting(LANGUAGE_PARAMETER, -1);

        if (lang_id == -1) {
            auto lang_canonical = Model_Setting::instance()
                .GetStringSetting(LANGUAGE_PARAMETER, wxLocale::GetLanguageCanonicalName(wxLANGUAGE_UNKNOWN));

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
    Model_Infotable::instance().Set("LOCALE", locale);
    m_locale_name = locale;
}

void Option::loadDateFormat()
{
    m_date_format = Model_Infotable::instance().GetStringInfo("DATEFORMAT", mmex::DEFDATEFORMAT);
}
void Option::setDateFormat(const wxString& date_format)
{
    Model_Infotable::instance().Set("DATEFORMAT", date_format);
    m_date_format = date_format;
}

void Option::loadUserName()
{
    m_user_name = Model_Infotable::instance().GetStringInfo("USERNAME", "");
}
void Option::setUserName(const wxString& username)
{
    m_user_name = username;
    Model_Infotable::instance().Set("USERNAME", username);
}

void Option::loadBaseCurrencyID()
{
    m_base_currency_id = Model_Infotable::instance().GetInt64Info("BASECURRENCYID", -1);
}
void Option::setBaseCurrencyID(const int64 base_currency_id)
{
    Model_Infotable::instance().Set("BASECURRENCYID", base_currency_id);
    m_base_currency_id = base_currency_id;
}

void Option::loadUseCurrencyHistory()
{
    m_use_currency_history = Model_Infotable::instance().GetBoolInfo("USECURRENCYHISTORY", true);
}
void Option::setUseCurrencyHistory(const bool value)
{
    Model_Infotable::instance().Set("USECURRENCYHISTORY", value);
    m_use_currency_history = value;
}

void Option::loadSharePrecision()
{
    m_share_precision = Model_Infotable::instance().GetIntInfo("SHARE_PRECISION", 4);
}
void Option::setSharePrecision(const int value)
{
    Model_Infotable::instance().Set("SHARE_PRECISION", value);
    m_share_precision = value;
}

void Option::loadAssetCompounding()
{
    wxString assetCompounding = Model_Infotable::instance().GetStringInfo("ASSET_COMPOUNDING", "Day");
    m_asset_compounding = Option::COMPOUNDING_ID_DAY;
    for (const auto& a : Option::COMPOUNDING_NAME) if (assetCompounding == a.second) {
        m_asset_compounding = a.first;
        break;
    }
}
void Option::setAssetCompounding(const int value)
{
    Model_Infotable::instance().Set("ASSET_COMPOUNDING", Option::COMPOUNDING_NAME[value].second);
    m_asset_compounding = value;
}

void Option::loadReportingFirstDay()
{
    m_reporting_firstday = Model_Infotable::instance().GetIntInfo("REPORTING_FIRSTDAY", 1);
    if (m_reporting_firstday < 1) m_reporting_firstday = 1;
    if (m_reporting_firstday > 28) m_reporting_firstday = 28;
}
void Option::setReportingFirstDay(const int value)
{
    Model_Infotable::instance().Set("REPORTING_FIRSTDAY", value);
    m_reporting_firstday = value;
}

void Option::loadFinancialFirstDay()
{
    m_financial_first_day = Model_Infotable::instance().GetStringInfo("FINANCIAL_YEAR_START_DAY", "1");
}
void Option::setFinancialFirstDay(const wxString& setting)
{
    m_financial_first_day = setting;
    Model_Infotable::instance().Set("FINANCIAL_YEAR_START_DAY", setting);
}

void Option::loadFinancialFirstMonth()
{
    m_financial_first_month = Model_Infotable::instance().GetStringInfo("FINANCIAL_YEAR_START_MONTH", "7");
}
void Option::setFinancialFirstMonth(const wxString& setting)
{
    m_financial_first_month = setting;
    Model_Infotable::instance().Set("FINANCIAL_YEAR_START_MONTH", setting);
}

void Option::loadBudgetDaysOffset()
{
    m_budget_days_offset = Model_Infotable::instance().GetIntInfo("BUDGET_DAYS_OFFSET", 0);
}
void Option::setBudgetDaysOffset(const int value)
{
    Model_Infotable::instance().Set("BUDGET_DAYS_OFFSET", value);
    m_budget_days_offset = value;
}
void Option::addBudgetDateOffset(wxDateTime& date) const
{
    if (m_budget_days_offset != 0)
        date.Add(wxDateSpan::Days(m_budget_days_offset));
}

void Option::loadHomePageIncExpRange()
{
    m_homepage_incexp_range = Model_Infotable::instance().GetIntInfo("HOMEPAGE_INCEXP_RANGE", 0);
}
void Option::setHomePageIncExpRange(const int value)
{
    Model_Infotable::instance().Set("HOMEPAGE_INCEXP_RANGE", value);
    m_homepage_incexp_range = value;
}

void Option::loadHideShareAccounts()
{
    m_hide_share_accounts = Model_Setting::instance().GetBoolSetting("HIDE_SHARE_ACCOUNTS", true);
}
void Option::setHideShareAccounts(const bool value)
{
    Model_Setting::instance().Set("HIDE_SHARE_ACCOUNTS", value);
    m_hide_share_accounts = value;
}

void Option::loadHideDeletedTransactions()
{
    m_hide_deleted_transactions = Model_Setting::instance().GetBoolSetting("HIDE_DELETED_TRANSACTIONS", false);
}
void Option::setHideDeletedTransactions(const bool value)
{
    Model_Setting::instance().Set("HIDE_DELETED_TRANSACTIONS", value);
    m_hide_deleted_transactions = value;
}

void Option::loadBudgetFinancialYears()
{
    m_budget_financial_years = Model_Setting::instance().GetBoolSetting("BUDGET_FINANCIAL_YEARS", false);
}
void Option::setBudgetFinancialYears(const bool value)
{
    Model_Setting::instance().Set("BUDGET_FINANCIAL_YEARS", value);
    m_budget_financial_years = value;
}

void Option::loadBudgetIncludeTransfers()
{
    m_budget_include_transfers = Model_Setting::instance().GetBoolSetting("BUDGET_INCLUDE_TRANSFERS", false);
}
void Option::setBudgetIncludeTransfers(const bool value)
{
    Model_Setting::instance().Set("BUDGET_INCLUDE_TRANSFERS", value);
    m_budget_include_transfers = value;
}

void Option::loadBudgetSummaryWithoutCategories()
{
    m_budget_summary_without_categories = Model_Setting::instance().GetBoolSetting("BUDGET_SUMMARY_WITHOUT_CATEGORIES", true);
}
void Option::setBudgetSummaryWithoutCategories(bool value)
{
    Model_Setting::instance().Set("BUDGET_SUMMARY_WITHOUT_CATEGORIES", value);
    m_budget_summary_without_categories = value;
}

void Option::loadBudgetOverride()
{
    m_budget_override = Model_Setting::instance().GetBoolSetting("BUDGET_OVERRIDE", false);
}
void Option::setBudgetOverride(const bool value)
{
    Model_Setting::instance().Set("BUDGET_OVERRIDE", value);
    m_budget_override = value;
}

void Option::loadBudgetDeductMonthly()
{
    m_budget_deduct_monthly = Model_Setting::instance().GetBoolSetting("BUDGET_DEDUCT_MONTH_FROM_YEAR", false);
}
void Option::setBudgetDeductMonthly(bool value)
{
    Model_Setting::instance().Set("BUDGET_DEDUCT_MONTH_FROM_YEAR", value);
    m_budget_deduct_monthly = value;
}

void Option::loadIgnoreFutureTransactions()
{
    m_ignore_future_transactions = Model_Setting::instance().GetBoolSetting("IGNORE_FUTURE_TRANSACTIONS", false);
}
void Option::setIgnoreFutureTransactions(const bool value)
{
    Model_Setting::instance().Set("IGNORE_FUTURE_TRANSACTIONS", value);
    m_ignore_future_transactions = value;
}

void Option::loadUseTransDateTime()
{
    m_use_trans_datetime = Model_Setting::instance().GetBoolSetting("TRANSACTION_USE_DATE_TIME", false);
}
bool Option::UseTransDateTime(const bool value)
{
    if (value != m_use_trans_datetime) {
        Model_Setting::instance().Set("TRANSACTION_USE_DATE_TIME", value);
        m_use_trans_datetime = value;
        return true;
    }
    return false;
}

void Option::loadShowToolTips()
{
    m_show_tooltips = Model_Setting::instance().GetBoolSetting("IGNORE_SHOW_TOOLTIPS", true);
}
void Option::setShowToolTips(const bool value)
{
    Model_Setting::instance().Set("IGNORE_SHOW_TOOLTIPS", value);
    m_show_tooltips = value;
}

void Option::loadShowMoneyTips()
{
    m_show_moneytips = Model_Setting::instance().GetBoolSetting("IGNORE_SHOW_MONEYTIPS", true);
}
void Option::setShowMoneyTips(const bool value)
{
    Model_Setting::instance().Set("IGNORE_SHOW_MONEYTIPS", value);
    m_show_moneytips = value;
}

void Option::loadTransPayeeNone()
{
    // Read the preference as a string and convert to int
    m_trans_payee_none = Model_Setting::instance().GetIntSetting("TRANSACTION_PAYEE_NONE", Option::NONE);
}
void Option::setTransPayeeNone(const int value)
{
    Model_Setting::instance().Set("TRANSACTION_PAYEE_NONE", value);
    m_trans_payee_none = value;
}

void Option::loadTransCategoryNone()
{
    m_trans_category_none = Model_Setting::instance().GetIntSetting("TRANSACTION_CATEGORY_NONE", Option::LASTUSED);
}
void Option::setTransCategoryNone(const int value)
{
    Model_Setting::instance().Set("TRANSACTION_CATEGORY_NONE", value);
    m_trans_category_none = value;
}

void Option::loadTransCategoryTransferNone()
{
    m_trans_category_transfer_none = Model_Setting::instance().GetIntSetting("TRANSACTION_CATEGORY_TRANSFER_NONE", Option::LASTUSED);
}
void Option::setTransCategoryTransferNone(const int value)
{
    Model_Setting::instance().Set("TRANSACTION_CATEGORY_TRANSFER_NONE", value);
    m_trans_category_transfer_none = value;
}

void Option::loadBulkTransactions()
{
    m_bulk_transactions = Model_Setting::instance().GetBoolSetting("BULK_TRX", false);
}
void Option::setBulkTransactions(const bool value)
{
    Model_Setting::instance().Set("BULK_TRX", value);
    m_bulk_transactions = value;
}

void Option::loadTransStatusReconciled()
{
    m_trans_status_reconciled = Model_Setting::instance().GetIntSetting("TRANSACTION_STATUS_RECONCILED", Option::NONE);
}
void Option::setTransStatusReconciled(const int value)
{
    Model_Setting::instance().Set("TRANSACTION_STATUS_RECONCILED", value);
    m_trans_status_reconciled = value;
}

void Option::loadTransDateDefault()
{
    m_trans_date_default = Model_Setting::instance().GetIntSetting("TRANSACTION_DATE_DEFAULT", Option::NONE);
}
void Option::setTransDateDefault(const int value)
{
    Model_Setting::instance().Set("TRANSACTION_DATE_DEFAULT", value);
    m_trans_date_default = value;
}

void Option::loadSendUsageStats()
{
    m_send_usage_stats = Model_Setting::instance().GetBoolSetting(INIDB_SEND_USAGE_STATS, true);
}
void Option::setSendUsageStats(const bool value)
{
    Model_Setting::instance().Set(INIDB_SEND_USAGE_STATS, value);
    m_send_usage_stats = value;
}

void Option::loadCheckNews()
{
    m_check_news = Model_Setting::instance().GetBoolSetting("CHECKNEWS", true);
}
void Option::setCheckNews(const bool value)
{
    Model_Setting::instance().Set("CHECKNEWS", value);
    m_check_news = value;
}

void Option::loadThemeMode()
{
    m_theme_mode = Model_Setting::instance().GetIntSetting("THEMEMODE", Option::THEME_MODE::AUTO);
}
void Option::setThemeMode(const int value)
{
    Model_Setting::instance().Set("THEMEMODE", value);
    m_theme_mode = value;
}

void Option::loadHtmlScale()
{
    m_html_scale = Model_Setting::instance().GetIntSetting("HTMLSCALE", 100);
}
void Option::setHtmlScale(const int value)
{
    Model_Setting::instance().Set("HTMLSCALE", value);
    m_html_scale = value;
}

void Option::loadFontSize()
{
    m_font_size = Model_Setting::instance().GetIntSetting("UI_FONT_SIZE", 0);
}
void Option::setFontSize(const int value)
{
    Model_Setting::instance().Set("UI_FONT_SIZE", value);
    m_font_size = value;
}

void Option::loadIconSize()
{
    m_icon_size = Model_Setting::instance().GetIntSetting("ICONSIZE", 16);
}
void Option::setIconSize(const int value)
{
    Model_Setting::instance().Set("ICONSIZE", value);
    m_icon_size = value;
}

void Option::loadToolbarIconSize()
{
    m_toolbar_icon_size = Model_Setting::instance().GetIntSetting("TOOLBARICONSIZE", 32);
}
void Option::setToolbarIconSize(const int value)
{
    Model_Setting::instance().Set("TOOLBARICONSIZE", value);
    m_toolbar_icon_size = value;
}

void Option::loadNavigationIconSize()
{
    m_navigation_icon_size = Model_Setting::instance().GetIntSetting("NAVIGATIONICONSIZE", 24);
}
void Option::setNavigationIconSize(const int value)
{
    Model_Setting::instance().Set("NAVIGATIONICONSIZE", value);
    m_navigation_icon_size = value;
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

    int max = acc_img::MAX_ACC_ICON - img::LAST_NAVTREE_PNG;
    int min = 1;
    int custom_img_id = Model_Infotable::instance().GetIntInfo(wxString::Format("ACC_IMAGE_ID_%lld", account_id), 0);
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
    Model_Setting::instance().Set(LANGUAGE_PARAMETER, wxLocale::GetLanguageCanonicalName(language));
}
