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
#include "import_export/univcsvdialog.h"
#include "constants.h"
#include "singleton.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include "model/Model_Account.h"

//----------------------------------------------------------------------------
mmOptions::mmOptions()
:   dateFormat_(mmex::DEFDATEFORMAT)
    , language_("english")
    , databaseUpdated_(false)
{}

//----------------------------------------------------------------------------
mmOptions& mmOptions::instance()
{
    return Singleton<mmOptions>::instance();
}

//----------------------------------------------------------------------------
void mmOptions::LoadInfotableOptions()
{
    dateFormat_ = Model_Infotable::instance().GetStringInfo("DATEFORMAT", mmex::DEFDATEFORMAT);
    userNameString_ = Model_Infotable::instance().GetStringInfo("USERNAME", "");

    financialYearStartDayString_   = Model_Infotable::instance().GetStringInfo("FINANCIAL_YEAR_START_DAY", "1");
    financialYearStartMonthString_ = Model_Infotable::instance().GetStringInfo("FINANCIAL_YEAR_START_MONTH", "7");
}

//----------------------------------------------------------------------------
mmIniOptions::mmIniOptions()
: enableAssets_(true)
, enableBudget_(true)
, enableRepeatingTransactions_(true)
, enableCheckForUpdates_(true)
, enableReportIssues_ (true)
, enableBeNotifiedForNewReleases_(true)
, enableVisitWebsite_(true)
, html_font_size_(3)
, expandBankHome_(true)
, expandTermHome_(false)
, expandStocksHome_(true)
, expandBankTree_(true)
, expandTermTree_(false)
, budgetFinancialYears_(false)
, budgetIncludeTransfers_(false)
, budgetSetupWithoutSummaries_(false)
, budgetSummaryWithoutCategories_(true)
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
    expandStocksHome_ = Model_Setting::instance().GetBoolSetting("ENABLESTOCKS", true);
    enableAssets_     = Model_Setting::instance().GetBoolSetting("ENABLEASSETS", true);
    enableBudget_     = Model_Setting::instance().GetBoolSetting("ENABLEBUDGET", true);

    html_font_size_   = Model_Setting::instance().GetIntSetting("HTMLFONTSIZE", 3);

    expandBankHome_   = Model_Setting::instance().GetBoolSetting("EXPAND_BANK_HOME", true);
    expandTermHome_   = Model_Setting::instance().GetBoolSetting("EXPAND_TERM_HOME", false);
    expandBankTree_   = Model_Setting::instance().GetBoolSetting("EXPAND_BANK_TREE", true);
    expandTermTree_   = Model_Setting::instance().GetBoolSetting("EXPAND_TERM_TREE", false);

    budgetFinancialYears_           = Model_Setting::instance().GetBoolSetting(INIDB_BUDGET_FINANCIAL_YEARS, false);
    budgetIncludeTransfers_         = Model_Setting::instance().GetBoolSetting(INIDB_BUDGET_INCLUDE_TRANSFERS, false);
    budgetSetupWithoutSummaries_    = Model_Setting::instance().GetBoolSetting(INIDB_BUDGET_SETUP_WITHOUT_SUMMARY, false);
    budgetSummaryWithoutCategories_ = Model_Setting::instance().GetBoolSetting(INIDB_BUDGET_SUMMARY_WITHOUT_CATEG, true);
    ignoreFutureTransactions_       = Model_Setting::instance().GetBoolSetting(INIDB_IGNORE_FUTURE_TRANSACTIONS, false);

    // Read the preference as a string and convert to int
    transPayeeSelectionNone_ = Model_Setting::instance().GetIntSetting("TRANSACTION_PAYEE_NONE", 0);

    // For the category selection, default behavior should remain that the last category used for the payee is selected.
    //  This is item 1 (0-indexed) in the list.
    transCategorySelectionNone_ = Model_Setting::instance().GetIntSetting("TRANSACTION_CATEGORY_NONE", 1);
    transStatusReconciled_      = Model_Setting::instance().GetIntSetting("TRANSACTION_STATUS_RECONCILED", 0);
    transDateDefault_           = Model_Setting::instance().GetIntSetting("TRANSACTION_DATE_DEFAULT", 0);
}

int mmIniOptions::account_image_id(int account_id)
{
    int selectedImage = Model_Infotable::instance().GetIntInfo(wxString::Format("ACC_IMAGE_ID_%i", account_id), 99);
    //TODO: What size of the images spool?
    if (selectedImage > 0 && selectedImage < 99)
        return selectedImage;

    selectedImage = 9;
    int t = 0, s = 0;
    wxString acctStatus = "Open";
    Model_Account::TYPE acctType = Model_Account::CHECKING;
    bool favorite = true;

    Model_Account::Data* account = Model_Account::instance().get(account_id);
    if (account)
    {
        acctType = Model_Account::type(account);
        acctStatus = account->STATUS;
        favorite = account->FAVORITEACCT == "TRUE";
    }

    if (acctStatus == "Closed")
        s = 2;
    else if (favorite)
        s = 1;

    if (acctType == Model_Account::TERM) 
        t = 3;
    else if (acctType == Model_Account::INVESTMENT)
        t = 6;

    selectedImage += t + s;

    return selectedImage;
}

bool CommitCallbackHook::CommitCallback()
{
    mmOptions::instance().databaseUpdated_ = true;

    return false;
}
