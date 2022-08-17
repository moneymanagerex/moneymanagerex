/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016 - 2021 Nikolay Akimov
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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

//----------------------------------------------------------------------------
Option::Option()
:   m_dateFormat(mmex::DEFDATEFORMAT)
    , m_language(wxLANGUAGE_UNKNOWN)
    , m_databaseUpdated(false)
    , m_budgetFinancialYears(false)
    , m_budgetIncludeTransfers(false)
    , m_budgetReportWithSummaries(true)
    , m_ignoreFutureTransactions(false)
    , m_showToolTips(true)
    , m_showMoneyTips(true)
    , m_currencyHistoryEnabled(false)
    , m_bulk_enter(false)
    , m_transPayeeSelection(Option::NONE)
    , m_transCategorySelectionNonTransfer(Option::NONE)
    , m_transCategorySelectionTransfer(Option::NONE)
    , m_transStatusReconciled(Option::NONE)
    , m_usageStatistics(true)
    , m_transDateDefault(0)
    , m_sharePrecision(4)
    , m_theme_mode(Option::AUTO)
    , m_html_font_size(100)
    , m_ico_size(16)
    , m_font_size(0)
    , m_toolbar_ico_size(32)
    , m_navigation_ico_size(24)
    , m_budget_days_offset(0)
    , m_reporting_firstday(1)
{}

//----------------------------------------------------------------------------
Option& Option::instance()
{
    return Singleton<Option>::instance();
}

//----------------------------------------------------------------------------
void Option::LoadOptions(bool include_infotable)
{
    if (include_infotable)
    {
        m_dateFormat = Model_Infotable::instance().GetStringInfo("DATEFORMAT", mmex::DEFDATEFORMAT);
        m_userNameString = Model_Infotable::instance().GetStringInfo("USERNAME", "");
        m_financialYearStartDayString = Model_Infotable::instance().GetStringInfo("FINANCIAL_YEAR_START_DAY", "1");
        m_financialYearStartMonthString = Model_Infotable::instance().GetStringInfo("FINANCIAL_YEAR_START_MONTH", "7");
        m_sharePrecision = Model_Infotable::instance().GetIntInfo("SHARE_PRECISION", 4);
        m_baseCurrency = Model_Infotable::instance().GetIntInfo("BASECURRENCYID", -1);
        m_currencyHistoryEnabled = Model_Infotable::instance().GetBoolInfo(INIDB_USE_CURRENCY_HISTORY, true);
        m_budget_days_offset = Model_Infotable::instance().GetIntInfo("BUDGET_DAYS_OFFSET", 0);
        m_reporting_firstday = Model_Infotable::instance().GetIntInfo("REPORTING_FIRSTDAY", 1);
        if (m_reporting_firstday > 28) m_reporting_firstday = 28;
        m_homepage_incexp_range = Model_Infotable::instance().GetIntInfo("HOMEPAGE_INCEXP_RANGE", 0);
        // Ensure that base currency is set for the database.
        while (m_baseCurrency < 1)
        {
            if (mmMainCurrencyDialog::Execute(m_baseCurrency))
            {
                setBaseCurrency(m_baseCurrency);
                Model_CurrencyHistory::ResetCurrencyHistory();
                Model_Currency::ResetBaseConversionRates();
            }
        }
    }

    m_language = Option::instance().getLanguageID(true);

    m_hideShareAccounts = Model_Setting::instance().GetBoolSetting(INIDB_HIDE_SHARE_ACCOUNTS, true);
    m_budgetFinancialYears = Model_Setting::instance().GetBoolSetting(INIDB_BUDGET_FINANCIAL_YEARS, false);
    m_budgetIncludeTransfers = Model_Setting::instance().GetBoolSetting(INIDB_BUDGET_INCLUDE_TRANSFERS, false);
    m_budgetReportWithSummaries = Model_Setting::instance().GetBoolSetting(INIDB_BUDGET_SUMMARY_WITHOUT_CATEG, true);
    m_ignoreFutureTransactions = Model_Setting::instance().GetBoolSetting(INIDB_IGNORE_FUTURE_TRANSACTIONS, false);
    m_showToolTips = Model_Setting::instance().GetBoolSetting(INIDB_SHOW_TOOLTIPS, true);
    m_showMoneyTips = Model_Setting::instance().GetBoolSetting(INIDB_SHOW_MONEYTIPS, true);

    // Read the preference as a string and convert to int
    m_transPayeeSelection = Model_Setting::instance().GetIntSetting("TRANSACTION_PAYEE_NONE", Option::NONE);

    // For the category selection, default behavior should remain that the last category used for the payee is selected.
    //  This is item 1 (0-indexed) in the list.
    m_transCategorySelectionNonTransfer = Model_Setting::instance().GetIntSetting("TRANSACTION_CATEGORY_NONE", Option::LASTUSED);
    m_transCategorySelectionTransfer = Model_Setting::instance().GetIntSetting("TRANSACTION_CATEGORY_TRANSFER_NONE", Option::LASTUSED);
    m_transStatusReconciled = Model_Setting::instance().GetIntSetting("TRANSACTION_STATUS_RECONCILED", Option::NONE);
    m_transDateDefault = Model_Setting::instance().GetIntSetting("TRANSACTION_DATE_DEFAULT", 0);
    m_usageStatistics = Model_Setting::instance().GetBoolSetting(INIDB_SEND_USAGE_STATS, true);
    m_newsChecking = Model_Setting::instance().GetBoolSetting(INIDB_CHECK_NEWS, true);
    
    m_theme_mode = Model_Setting::instance().GetIntSetting("THEMEMODE", Option::THEME_MODE::AUTO);
    m_html_font_size = Model_Setting::instance().GetIntSetting("HTMLSCALE", 100);
    m_ico_size = Model_Setting::instance().GetIntSetting("ICONSIZE", 16);
    m_toolbar_ico_size = Model_Setting::instance().GetIntSetting("TOOLBARICONSIZE", 32);
    m_navigation_ico_size = Model_Setting::instance().GetIntSetting("NAVIGATIONICONSIZE", 24);
    m_bulk_enter = Model_Setting::instance().GetBoolSetting("BULK_TRX", false);
    m_font_size = Model_Setting::instance().GetIntSetting("UI_FONT_SIZE", 0);
}

void Option::setDateFormat(const wxString& date_format)
{
    m_dateFormat = date_format;
    Model_Infotable::instance().Set("DATEFORMAT", date_format);
}

wxLanguage Option::getLanguageID(bool get_db)
{
    if (get_db)
    {
        auto lang_id = Model_Setting::instance()
            .GetIntSetting(LANGUAGE_PARAMETER, -1);

        if (lang_id == -1)
        {
            auto lang_canonical = Model_Setting::instance()
                .GetStringSetting(LANGUAGE_PARAMETER, wxLocale::GetLanguageCanonicalName(wxLANGUAGE_UNKNOWN));
            int lang_code = wxLANGUAGE_DEFAULT;
            for (lang_code; lang_code < wxLANGUAGE_USER_DEFINED; lang_code++)
            {
                const auto l = wxLocale::GetLanguageCanonicalName(lang_code);
                if (lang_canonical == l) {
                    m_language = static_cast<wxLanguage>(lang_code);
                    break;
                }
            }
        }
        else
        {
            m_language = static_cast<wxLanguage>(lang_id);
        }
    }
   
    return m_language;
}

void Option::UserName(const wxString& username)
{
    m_userNameString = username;
    Model_Infotable::instance().Set("USERNAME", username);
}

void Option::LocaleName(const wxString& locale)
{
    m_localeNameString = locale;
    Model_Infotable::instance().Set("LOCALE", locale);
}

void Option::FinancialYearStartDay(const wxString& setting)
{
    m_financialYearStartDayString = setting;
    Model_Infotable::instance().Set("FINANCIAL_YEAR_START_DAY", setting);
}

wxString Option::FinancialYearStartMonth()
{
    return m_financialYearStartMonthString;
}

void Option::FinancialYearStartMonth(const wxString& setting)
{
    m_financialYearStartMonthString = setting;
    Model_Infotable::instance().Set("FINANCIAL_YEAR_START_MONTH", setting);
}

void Option::setBaseCurrency(int base_currency_id)
{
    m_baseCurrency = base_currency_id;
    Model_Infotable::instance().Set("BASECURRENCYID", base_currency_id);
}

int Option::getBaseCurrencyID()
{
    return m_baseCurrency;
}

void Option::CurrencyHistoryEnabled(bool value)
{
    Model_Infotable::instance().Set(INIDB_USE_CURRENCY_HISTORY, value);
    m_currencyHistoryEnabled = value;
}

void Option::DatabaseUpdated(bool value)
{
    m_databaseUpdated = value;
}

bool Option::DatabaseUpdated()
{
    return m_databaseUpdated;
}

void Option::HideShareAccounts(bool value)
{
    Model_Setting::instance().Set(INIDB_HIDE_SHARE_ACCOUNTS, value);
    m_hideShareAccounts = value;
}

bool Option::HideShareAccounts()
{
    return m_hideShareAccounts;
}

void Option::BudgetFinancialYears(bool value)
{
    Model_Setting::instance().Set(INIDB_BUDGET_FINANCIAL_YEARS, value);
    m_budgetFinancialYears = value;
}

bool Option::BudgetFinancialYears()
{
    return m_budgetFinancialYears;
}

void Option::BudgetIncludeTransfers(bool value)
{
    Model_Setting::instance().Set(INIDB_BUDGET_INCLUDE_TRANSFERS, value);
    m_budgetIncludeTransfers = value;

}

bool Option::BudgetIncludeTransfers()
{
    return m_budgetIncludeTransfers;
}

void Option::BudgetReportWithSummaries(bool value)
{
    Model_Setting::instance().Set(INIDB_BUDGET_SUMMARY_WITHOUT_CATEG, value);
    m_budgetReportWithSummaries = value;

}

bool Option::BudgetReportWithSummaries()
{
    return m_budgetReportWithSummaries;
}

void Option::IgnoreFutureTransactions(bool value)
{
    Model_Setting::instance().Set(INIDB_IGNORE_FUTURE_TRANSACTIONS, value);
    m_ignoreFutureTransactions = value;
}

void Option::ShowToolTips(bool value)
{
    Model_Setting::instance().Set(INIDB_SHOW_TOOLTIPS, value);
    m_showToolTips = value;
}

void Option::ShowMoneyTips(bool value)
{
    Model_Setting::instance().Set(INIDB_SHOW_MONEYTIPS, value);
    m_showMoneyTips = value;
}

void Option::TransPayeeSelection(int value)
{
    Model_Setting::instance().Set("TRANSACTION_PAYEE_NONE", value);
    m_transPayeeSelection = value;
}

int Option::TransPayeeSelection()
{
    return m_transPayeeSelection;
}

void Option::TransCategorySelectionNonTransfer(int value)
{
    Model_Setting::instance().Set("TRANSACTION_CATEGORY_NONE", value);
    m_transCategorySelectionNonTransfer = value;
}

void Option::TransCategorySelectionTransfer(int value)
{
    Model_Setting::instance().Set("TRANSACTION_CATEGORY_TRANSFER_NONE", value);
    m_transCategorySelectionTransfer = value;
}

void Option::set_bulk_transactions(bool value)
{
    Model_Setting::instance().Set("BULK_TRX", value);
    m_bulk_enter = value;
}

void Option::TransStatusReconciled(int value)
{
    Model_Setting::instance().Set("TRANSACTION_STATUS_RECONCILED", value);
    m_transStatusReconciled = value;
}

int Option::TransStatusReconciled()
{
    return m_transStatusReconciled;
}

void Option::TransDateDefault(int value)
{
    Model_Setting::instance().Set("TRANSACTION_DATE_DEFAULT", value);
    m_transDateDefault = value;
}

int Option::TransDateDefault()
{
    return m_transDateDefault;
}

void Option::SharePrecision(int value)
{
    Model_Infotable::instance().Set("SHARE_PRECISION", value);
    m_sharePrecision = value;
}

int Option::SharePrecision()
{
    return m_sharePrecision;
}

void Option::SendUsageStatistics(bool value)
{
    m_usageStatistics = value;
    Model_Setting::instance().Set(INIDB_SEND_USAGE_STATS, value);
}

bool Option::SendUsageStatistics()
{
    return m_usageStatistics;
}

void Option::CheckNewsOnStartup(bool value)
{
    m_newsChecking = value;
    Model_Setting::instance().Set(INIDB_CHECK_NEWS, value);
}

bool Option::CheckNewsOnStartup()
{
    return m_newsChecking;
}

void Option::setThemeMode(int value)
{
    Model_Setting::instance().Set("THEMEMODE", value);
    m_theme_mode = value;
}

void Option::setHTMLFontSizes(int value)
{
    Model_Setting::instance().Set("HTMLSCALE", value);
    m_html_font_size = value;
}

void Option::setFontSize(int value)
{
    Model_Setting::instance().Set("UI_FONT_SIZE", value);
    m_font_size = value;
}

void Option::setIconSize(int value)
{
    Model_Setting::instance().Set("ICONSIZE", value);
    m_ico_size = value;
}

void Option::setToolbarIconSize(int value)
{
    Model_Setting::instance().Set("TOOLBARICONSIZE", value);
    m_toolbar_ico_size = value;
}

void Option::setNavigationIconSize(int value)
{
    Model_Setting::instance().Set("NAVIGATIONICONSIZE", value);
    m_navigation_ico_size = value;
}


int Option::getHtmlFontSize()
{
    return m_html_font_size;
}

void Option::setBudgetDaysOffset(int value)
{
    Model_Infotable::instance().Set("BUDGET_DAYS_OFFSET", value);
    m_budget_days_offset = value;
}

void Option::setBudgetDateOffset(wxDateTime& date) const
{
    if (m_budget_days_offset != 0)
        date.Add(wxDateSpan::Days(m_budget_days_offset));
}

void Option::setReportingFirstDay(int value)
{
    Model_Infotable::instance().Set("REPORTING_FIRSTDAY", value);
    m_reporting_firstday = value;
}

void Option::setHomePageIncExpRange(int value)
{
    Model_Infotable::instance().Set("HOMEPAGE_INCEXP_RANGE", value);
    m_homepage_incexp_range = value;
}

int Option::AccountImageId(int account_id, bool def, bool ignoreClosure)
{
    wxString acctStatus = VIEW_ACCOUNTS_OPEN_STR;
    Model_Account::TYPE acctType = Model_Account::CHECKING;
    int selectedImage = img::SAVINGS_ACC_NORMAL_PNG; //Default value

    Model_Account::Data* account = Model_Account::instance().get(account_id);
    if (account)
    {
        acctType = Model_Account::type(account);
        acctStatus = account->STATUS;
    }

    if (!def && !ignoreClosure && (acctStatus == "Closed"))
        return img::ACCOUNT_CLOSED_PNG;

    int max = acc_img::MAX_ACC_ICON - img::LAST_NAVTREE_PNG;
    int min = 1;
    int custom_img_id = Model_Infotable::instance().GetIntInfo(wxString::Format("ACC_IMAGE_ID_%i", account_id), 0);
    if (custom_img_id > max) custom_img_id = custom_img_id - 20; //Bug #963 fix 
    if (!def && (custom_img_id >= min && custom_img_id <= max))
        return custom_img_id + img::LAST_NAVTREE_PNG - 1;

    switch (acctType)
    {
    case (Model_Account::CHECKING) :
        selectedImage = img::SAVINGS_ACC_NORMAL_PNG;
        break;
    case (Model_Account::TERM) :
        selectedImage = img::TERMACCOUNT_NORMAL_PNG;
        break;
    case (Model_Account::INVESTMENT) :
        selectedImage = img::STOCK_ACC_NORMAL_PNG;
        break;
    case (Model_Account::CREDIT_CARD) :
        selectedImage = img::CARD_ACC_NORMAL_PNG;
        break;
    case (Model_Account::CASH) :
        selectedImage = img::CASH_ACC_NORMAL_PNG;
        break;
    case (Model_Account::LOAN) :
        selectedImage = img::LOAN_ACC_NORMAL_PNG;
        break;
    case (Model_Account::ASSET) :
        selectedImage = img::ASSET_NORMAL_PNG;
        break;
    case (Model_Account::SHARES) :
        selectedImage = img::LOAN_ACC_NORMAL_PNG;
        break;
    default:
        wxASSERT(false);
    }
    return selectedImage;
}

const wxString Option::getLanguageCode(bool get_db)
{
    Option::getLanguageID(get_db);
    if (m_language == wxLANGUAGE_UNKNOWN)
        return wxEmptyString;
    if (m_language == wxLANGUAGE_DEFAULT)
        return wxTranslations::Get()->GetBestTranslation("mmex", wxLANGUAGE_ENGLISH_US);

    const auto lang = wxLocale::GetLanguageCanonicalName(m_language);
    return lang;
}

void Option::setLanguage(wxLanguage& language)
{
    m_language = language;
    Model_Setting::instance().Set(LANGUAGE_PARAMETER, wxLocale::GetLanguageCanonicalName(language));
}
