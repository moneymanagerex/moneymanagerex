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


#include "mmOption.h"
#include "constants.h"
#include "images_list.h"
#include "singleton.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include "model/Model_Account.h"

//----------------------------------------------------------------------------
mmOptions::mmOptions()
:   m_dateFormat(mmex::DEFDATEFORMAT)
    , m_language("english")
    , m_databaseUpdated(false)
{}

//----------------------------------------------------------------------------
mmOptions& mmOptions::instance()
{
    return Singleton<mmOptions>::instance();
}

//----------------------------------------------------------------------------
void mmOptions::LoadInfotableOptions()
{
    m_dateFormat = Model_Infotable::instance().DateFormat();
    m_userNameString = Model_Infotable::instance().GetStringInfo("USERNAME", "");
    m_language = Model_Setting::instance().GetStringSetting(LANGUAGE_PARAMETER, "english");

    m_financialYearStartDayString   = Model_Infotable::instance().GetStringInfo("FINANCIAL_YEAR_START_DAY", "1");
    m_financialYearStartMonthString = Model_Infotable::instance().GetStringInfo("FINANCIAL_YEAR_START_MONTH", "7");
}

void mmOptions::DateFormat(const wxString& dateformat)
{
    m_dateFormat = dateformat;
    Model_Infotable::instance().SetDateFormat(dateformat);
}

wxString mmOptions::DateFormat()
{
    return m_dateFormat;
}

void mmOptions::Language(wxString& language)
{
    m_language = language;
    Model_Setting::instance().Set(LANGUAGE_PARAMETER, language);
}

wxString mmOptions::Language()
{
    return m_language;
}

void mmOptions::UserName(const wxString& username)
{
    m_userNameString = username;
    Model_Infotable::instance().Set("USERNAME", username);
}

wxString mmOptions::UserName()
{
    return m_userNameString;
}

wxString mmOptions::FinancialYearStartDay()
{
    return m_financialYearStartDayString;
}

void mmOptions::FinancialYearStartDay(const wxString& setting)
{
    m_financialYearStartDayString = setting;
    Model_Infotable::instance().Set("FINANCIAL_YEAR_START_DAY", setting);
}

wxString mmOptions::FinancialYearStartMonth()
{
    return m_financialYearStartMonthString;
}

void mmOptions::FinancialYearStartMonth(const wxString& setting)
{
    m_financialYearStartMonthString = setting;
    Model_Infotable::instance().Set("FINANCIAL_YEAR_START_MONTH", setting);
}

void mmOptions::DatabaseUpdated(bool value)
{
    m_databaseUpdated = value;

}

bool mmOptions::DatabaseUpdated()
{
    return m_databaseUpdated;
}


//----------------------------------------------------------------------------
mmIniOptions::mmIniOptions()
: html_font_size_(100)
, ico_size_(16)
, budgetFinancialYears_(false)
, budgetIncludeTransfers_(false)
, budgetSetupWithoutSummaries_(false)
, budgetReportWithSummaries_(true)
, ignoreFutureTransactions_(false)
, transPayeeSelectionNone_(0)
, transCategorySelectionNone_(0)
, transStatusReconciled_(0)
, transDateDefault_(0)

{}

mmIniOptions& mmIniOptions::instance()
{
    return Singleton<mmIniOptions>::instance();
}

void mmIniOptions::loadOptions()
{
    html_font_size_ = Model_Setting::instance().GetHtmlScaleFactor();

    ico_size_ = 16;
    if (html_font_size_ >= 300) ico_size_ = 48;
    else if (html_font_size_ >= 200) ico_size_ = 32;
    else if (html_font_size_ >= 150) ico_size_ = 24;

    budgetFinancialYears_           = Model_Setting::instance().BudgetFinancialYears();
    budgetIncludeTransfers_         = Model_Setting::instance().BudgetIncludeTransfers();
    budgetSetupWithoutSummaries_    = Model_Setting::instance().BudgetSetupWithoutSummary();
    budgetReportWithSummaries_      = Model_Setting::instance().BudgetSummaryWithoutCategory();
    ignoreFutureTransactions_       = Model_Setting::instance().IgnoreFutureTransactions();

    // Read the preference as a string and convert to int
    transPayeeSelectionNone_ = Model_Setting::instance().GetIntSetting("TRANSACTION_PAYEE_NONE", 0);

    // For the category selection, default behavior should remain that the last category used for the payee is selected.
    //  This is item 1 (0-indexed) in the list.
    transCategorySelectionNone_ = Model_Setting::instance().GetIntSetting("TRANSACTION_CATEGORY_NONE", 1);
    transStatusReconciled_      = Model_Setting::instance().GetIntSetting("TRANSACTION_STATUS_RECONCILED", 0);
    transDateDefault_           = Model_Setting::instance().GetIntSetting("TRANSACTION_DATE_DEFAULT", 0);
}

const int mmIniOptions::account_image_id(int account_id, bool def)
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
    default:
        wxASSERT(false);
    }
    return selectedImage;
}

