/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

//----------------------------------------------------------------------------
Option::Option()
:   m_dateFormat(mmex::DEFDATEFORMAT)
    , m_language("english")
    , m_databaseUpdated(false)
    , m_budgetFinancialYears(false)
    , m_budgetIncludeTransfers(false)
    , m_budgetSetupWithoutSummaries(false)
    , m_budgetReportWithSummaries(true)
    , m_ignoreFutureTransactions(false)
    , m_transPayeeSelection(Option::NONE)
    , m_transCategorySelection(Option::NONE)
    , m_transStatusReconciled(Option::NONE)
    , m_usageStatistics(true)
    , m_transDateDefault(0)
    , m_sharePrecision(4)
    , m_html_font_size(100)
    , m_ico_size(16)
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
        m_baseCurrency = Model_Infotable::instance().GetIntInfo("BASECURRENCYID", -1);
        m_sharePrecision = Model_Infotable::instance().GetIntInfo("SHARE_PRECISION", 4);
    }

    m_language = Model_Setting::instance().GetStringSetting(LANGUAGE_PARAMETER, "english");

    m_budgetFinancialYears = Model_Setting::instance().GetBoolSetting(INIDB_BUDGET_FINANCIAL_YEARS, false);
    m_budgetIncludeTransfers = Model_Setting::instance().GetBoolSetting(INIDB_BUDGET_INCLUDE_TRANSFERS, false);
    m_budgetSetupWithoutSummaries = Model_Setting::instance().GetBoolSetting(INIDB_BUDGET_SETUP_WITHOUT_SUMMARY, false);
    m_budgetReportWithSummaries = Model_Setting::instance().GetBoolSetting(INIDB_BUDGET_SUMMARY_WITHOUT_CATEG, true);
    m_ignoreFutureTransactions = Model_Setting::instance().GetBoolSetting(INIDB_IGNORE_FUTURE_TRANSACTIONS, false);

    // Read the preference as a string and convert to int
    m_transPayeeSelection = Model_Setting::instance().GetIntSetting("TRANSACTION_PAYEE_NONE", Option::NONE);

    // For the category selection, default behavior should remain that the last category used for the payee is selected.
    //  This is item 1 (0-indexed) in the list.
    m_transCategorySelection = Model_Setting::instance().GetIntSetting("TRANSACTION_CATEGORY_NONE", Option::LASTUSED);
    m_transStatusReconciled = Model_Setting::instance().GetIntSetting("TRANSACTION_STATUS_RECONCILED", Option::NONE);
    m_transDateDefault = Model_Setting::instance().GetIntSetting("TRANSACTION_DATE_DEFAULT", 0);
    m_usageStatistics = Model_Setting::instance().GetBoolSetting(INIDB_SEND_USAGE_STATS, true);

    m_html_font_size = Model_Setting::instance().GetIntSetting("HTMLSCALE", 100);
    m_ico_size = 16;
    if (m_html_font_size >= 300)
    {
        m_ico_size = 48;
    }
    else if (m_html_font_size >= 200)
    {
        m_ico_size = 32;
    }
    else if (m_html_font_size >= 150)
    {
        m_ico_size = 24;
    }
}

void Option::DateFormat(const wxString& dateformat)
{
    m_dateFormat = dateformat;
    Model_Infotable::instance().Set("DATEFORMAT", dateformat);;
}

wxString Option::DateFormat()
{
    return m_dateFormat;
}

void Option::Language(wxString& language)
{
    m_language = language;
    Model_Setting::instance().Set(LANGUAGE_PARAMETER, language);
}

wxString Option::Language(bool get_db)
{
    if (get_db)
    {
        m_language = Model_Setting::instance().GetStringSetting(LANGUAGE_PARAMETER, "english");
    }

    return m_language;
}

void Option::UserName(const wxString& username)
{
    m_userNameString = username;
    Model_Infotable::instance().Set("USERNAME", username);
}

wxString Option::UserName()
{
    return m_userNameString;
}

wxString Option::FinancialYearStartDay()
{
    return m_financialYearStartDayString;
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

void Option::BaseCurrency(int base_currency_id)
{
    m_baseCurrency = base_currency_id;
    Model_Infotable::instance().Set("BASECURRENCYID", base_currency_id);
}

int Option::BaseCurrency()
{
    return m_baseCurrency;
}

void Option::DatabaseUpdated(bool value)
{
    m_databaseUpdated = value;
}

bool Option::DatabaseUpdated()
{
    return m_databaseUpdated;
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

void Option::BudgetSetupWithoutSummaries(bool value)
{
    Model_Setting::instance().Set(INIDB_BUDGET_SETUP_WITHOUT_SUMMARY, value);
    m_budgetSetupWithoutSummaries = value;
}

bool Option::BudgetSetupWithoutSummaries()
{
    return m_budgetSetupWithoutSummaries;
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

bool Option::IgnoreFutureTransactions()
{
    return m_ignoreFutureTransactions;
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


void Option::TransCategorySelection(int value)
{
    Model_Setting::instance().Set("TRANSACTION_CATEGORY_NONE", value);
    m_transCategorySelection = value;
}

int Option::TransCategorySelection()
{
    return m_transCategorySelection;
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

void Option::HtmlFontSize(int value)
{
    Model_Setting::instance().Set("HTMLSCALE", value);
    m_html_font_size = value;
}

int Option::HtmlFontSize()
{
    return m_html_font_size;
}

void Option::IconSize(int value)
{
    m_ico_size = value;
}

int Option::IconSize()
{
    return m_ico_size;
}

const int Option::AccountImageId(int account_id, bool def)
{
    int max = acc_img::MAX_XPM - img::LAST_NAVTREE_PNG;
    int min = 1;
    int custom_img_id = Model_Infotable::instance().GetIntInfo(wxString::Format("ACC_IMAGE_ID_%i", account_id), 0);
    if (!def && (custom_img_id >= min && custom_img_id <= max))
        return custom_img_id + img::LAST_NAVTREE_PNG - 1;

    int selectedImage = img::SAVINGS_ACC_NORMAL_PNG; //Default value
    wxString acctStatus = VIEW_ACCOUNTS_OPEN_STR;
    Model_Account::TYPE acctType = Model_Account::CHECKING;
    bool favorite = true;

    Model_Account::Data* account = Model_Account::instance().get(account_id);
    if (account)
    {
        acctType = Model_Account::type(account);
        acctStatus = account->STATUS;
        favorite = Model_Account::FAVORITEACCT(account);
    }
    bool closed = acctStatus == "Closed";

    switch (acctType)
    {
    case (Model_Account::CHECKING) :
        if (closed) selectedImage = img::SAVINGS_ACC_CLOSED_PNG;
        else if (favorite) selectedImage = img::SAVINGS_ACC_FAVORITE_PNG;
        else selectedImage = img::SAVINGS_ACC_NORMAL_PNG;
        break;
    case (Model_Account::TERM) :
        if (closed) selectedImage = img::TERM_ACC_CLOSED_PNG;
        else if (favorite) selectedImage = img::TERM_ACC_FAVORITE_PNG;
        else  selectedImage = img::TERMACCOUNT_PNG;
        break;
    case (Model_Account::INVESTMENT) :
        if (closed) selectedImage = img::STOCK_ACC_CLOSED_PNG;
        else if (favorite) selectedImage = img::STOCK_ACC_FAVORITE_PNG;
        else  selectedImage = img::STOCK_ACC_PNG;
        break;
    case (Model_Account::CREDIT_CARD) :
        if (closed) selectedImage = img::CARD_ACC_CLOSED_PNG;
        else if (favorite)   selectedImage = img::CARD_ACC_FAVORITE_PNG;
        else   selectedImage = img::CARD_ACC_PNG;
        break;
    case (Model_Account::CASH) :
        if (closed) selectedImage = img::CASH_ACC_CLOSED_PNG;
        else if (favorite) selectedImage = img::CASH_ACC_FAVORITE_PNG;
        else selectedImage = img::CASH_ACC_NORMAL_PNG;
        break;
    case (Model_Account::LOAN) :
        if (closed) selectedImage = img::LOAN_ACC_CLOSED_PNG;
        else if (favorite) selectedImage = img::LOAN_ACC_FAVORITE_PNG;
        else selectedImage = img::LOAN_ACC_NORMAL_PNG;
        break;
    case (Model_Account::ASSET) :
        if (closed) selectedImage = img::CASH_ACC_CLOSED_PNG;
        else if (favorite) selectedImage = img::CASH_ACC_FAVORITE_PNG;
        else selectedImage = img::CASH_ACC_NORMAL_PNG;
        break;
    case (Model_Account::SHARES) :
        if (closed) selectedImage = img::LOAN_ACC_CLOSED_PNG;
        else if (favorite) selectedImage = img::LOAN_ACC_FAVORITE_PNG;
        else selectedImage = img::LOAN_ACC_NORMAL_PNG;
        break;
    default:
        wxASSERT(false);
    }
    return selectedImage;
}
