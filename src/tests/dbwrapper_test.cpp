/*************************************************************************
 Copyright (C) 2009 VaDiM
 Copyright (C) 2011 Stefano Giorgio

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
 *************************************************************************/

#include <UnitTest++.h>
#include "testing_util.h"

#ifdef DBWRAPPER_TEST_INCLUDED_IN_BUILD
/**************************************************************************
 The test platform will create and test a new database.

 The temporary test databases will be remove on completion.
 *************************************************************************/
SUITE(dbwrapper)
{

TEST(wxSQLite3Exception)
{
    printf("\ndbwrapper_tests: START");
    display_STD_IO_separation_line();
    const wxDateTime start_time(wxDateTime::UNow());
    wxSQLite3Database* db = get_pDb().get();
    displayTimeTaken("wxSQLite3Exception", start_time);
    CHECK_THROW(db->ExecuteUpdate("update bad_table set bad_col = unknown_value"), wxSQLite3Exception);
}
//----------------------------------------------------------------------------

TEST(HasBackupSupport)
{
    bool ok = get_pDb().get()->HasBackupSupport();
    CHECK(ok);
}
//----------------------------------------------------------------------------

TEST(HasEncryptionSupport)
{
    bool ok = get_pDb().get()->HasEncryptionSupport();
    CHECK(ok);
}
//----------------------------------------------------------------------------

TEST(HasMetaDataSupport)
{
    bool ok = get_pDb().get()->HasMetaDataSupport();
    CHECK(ok);
}
//----------------------------------------------------------------------------

TEST(HasSavepointSupport)
{
    bool ok = get_pDb().get()->HasSavepointSupport();
    CHECK(ok);
}
//----------------------------------------------------------------------------

TEST(ViewExists)
{
    wxStopWatch start_time;

    wxSQLite3Database* pDb = get_pDb().get();

    bool ok = mmDBWrapper::ViewExists(pDb, "I'm_not_exists");
    CHECK(!ok);

    pDb->ExecuteUpdate(
        "create view master_view as "
        "select * "
        "from sqlite_master"
    );

    ok = mmDBWrapper::ViewExists(pDb, "master_view");
    CHECK(ok);
    displayTimeTaken("ViewExists", start_time);

//    pDb->ExecuteUpdate("drop view master_view");
}
//----------------------------------------------------------------------------

TEST(init_DB)
{
    wxStopWatch start_time;

    mmCoreDB* pCore = pDb_core().get();
    CHECK(pCore->displayDatabaseError_ == true);

    displayTimeTaken("init_DB", start_time);    
}
//----------------------------------------------------------------------------
#if 0
TEST(getCurrencySymbol)
{
    const wxDateTime start_time(wxDateTime::UNow());
    mmCoreDB* pCore = pDb_core().get();

    int currency_id = 1;
    wxString s = mmDBWrapper::getCurrencySymbol(pCore->db_.get(), currency_id);
    CHECK(s == L"EUR");

    currency_id = pCore->currencyList_.getCurrencyID(g_CurrencyName);
    s = mmDBWrapper::getCurrencySymbol(pCore->db_.get(), currency_id);
    CHECK(s == L"USD");

    s = mmDBWrapper::getCurrencySymbol(pCore->db_.get(), 0);
    CHECK(s.empty());
    displayTimeTaken("getCurrencySymbol", start_time);    
}
#endif
//----------------------------------------------------------------------------

TEST(checkDBVersion)
{
    const wxDateTime start_time(wxDateTime::UNow());
    wxSQLite3Database* pDb = get_pDb().get();
    bool ok = mmDBWrapper::checkDBVersion(pDb);
    CHECK(ok);
    displayTimeTaken("checkDBVersion", start_time);
}
//----------------------------------------------------------------------------

TEST(addBudgetYear)
{
    wxStopWatch start_time;
    wxSQLite3Database* pDb = get_pDb().get();
    
    int year_id = mmDBWrapper::getBudgetYearID(pDb, g_BudgetYear);
    CHECK(year_id == -1);

    mmDBWrapper::addBudgetYear(pDb, g_BudgetYear);

    year_id = mmDBWrapper::getBudgetYearID(pDb, g_BudgetYear);
    CHECK(year_id > 0);
    displayTimeTaken("addBudgetYear", start_time);
}
//----------------------------------------------------------------------------

TEST(getBudgetYearID)
{
    wxStopWatch start_time;
    wxSQLite3Database* pDb = get_pDb().get();

    int year_id = mmDBWrapper::getBudgetYearID(pDb, "unknown_year");
    CHECK(year_id == -1);

    year_id = mmDBWrapper::getBudgetYearID(pDb, g_BudgetYear);
    CHECK(year_id > 0);
    displayTimeTaken("getBudgetYearID", start_time);
}
//----------------------------------------------------------------------------

TEST(getBudgetYearForID)
{
    wxStopWatch start_time;
    wxSQLite3Database* pDb = get_pDb().get();

    int year_id = mmDBWrapper::getBudgetYearID(pDb, g_BudgetYear);
    CHECK(year_id > 0);

    wxString year = mmDBWrapper::getBudgetYearForID(pDb, year_id);
    CHECK(year == g_BudgetYear);
    displayTimeTaken("getBudgetYearForID", start_time);
}
//----------------------------------------------------------------------------

TEST(updateYearForID)
{
    wxStopWatch start_time;
    wxSQLite3Database* pDb = get_pDb().get();

    int year_id = mmDBWrapper::getBudgetYearID(pDb, g_BudgetYear);
    CHECK(year_id > 0);

    wxString new_year = g_BudgetYear + g_BudgetYear;
    mmDBWrapper::updateYearForID(pDb, new_year, year_id);

    wxString year = mmDBWrapper::getBudgetYearForID(pDb, year_id);
    CHECK(year == new_year);

    // restore original value

    mmDBWrapper::updateYearForID(pDb, g_BudgetYear, year_id);
    year = mmDBWrapper::getBudgetYearForID(pDb, year_id);
    CHECK(year == g_BudgetYear);
    displayTimeTaken("updateYearForID", start_time);
}
//----------------------------------------------------------------------------

TEST(copyBudgetYear)
{
    wxStopWatch start_time;
    wxSQLite3Database* pDb = get_pDb().get();

    int base_year_id = mmDBWrapper::getBudgetYearID(pDb, g_BudgetYear);
    CHECK(base_year_id > 0);

    // --

    bool ok = mmDBWrapper::copyBudgetYear(pDb, base_year_id, base_year_id);
    CHECK(!ok);

    // --

    wxString new_year = g_BudgetYear + g_BudgetYear;
    mmDBWrapper::addBudgetYear(pDb, new_year);
    
    int new_year_id = mmDBWrapper::getBudgetYearID(pDb, new_year);
    CHECK(new_year_id > 0);

    // --

    ok = mmDBWrapper::copyBudgetYear(pDb, new_year_id, base_year_id);
    CHECK(ok);
    displayTimeTaken("copyBudgetYear", start_time);
}
//----------------------------------------------------------------------------

TEST(deleteBudgetYear)
{
    wxStopWatch start_time;
    wxSQLite3Database* pDb = get_pDb().get();

    bool deleted = mmDBWrapper::deleteBudgetYear(pDb, "wrong_year");
    CHECK(!deleted);

    deleted = mmDBWrapper::deleteBudgetYear(pDb, g_BudgetYear);
    CHECK(deleted);
    displayTimeTaken("deleteBudgetYear", start_time);
}
//----------------------------------------------------------------------------

TEST(addCategory)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int cat_id = pCore->categoryList_.AddCategory(g_CategName);

    int id = pCore->categoryList_.GetCategoryId(g_CategName);

    CHECK(id == cat_id);
    displayTimeTaken("addCategory", start_time);
}
//----------------------------------------------------------------------------

TEST(addSubCategory)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int cat_id = pCore->categoryList_.GetCategoryId(g_CategName);
    CHECK(cat_id > 0);

    int subcat_id = pCore->categoryList_.AddSubCategory(cat_id, g_SubCategName);

    int id = pCore->categoryList_.GetSubCategoryID(cat_id, g_SubCategName);
    CHECK(id == subcat_id);
    displayTimeTaken("addSubCategory", start_time);
}
//----------------------------------------------------------------------------

TEST(getCategoryID)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int cat_id = pCore->categoryList_.GetCategoryId(g_CategName);
    CHECK(cat_id > 0);

    cat_id = pCore->categoryList_.GetCategoryId("unknown category");
    CHECK(cat_id == -1);
    displayTimeTaken("getCategoryID", start_time);
}
//----------------------------------------------------------------------------

TEST(getSubCategoryID)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int cat_id = pCore->categoryList_.GetCategoryId(g_CategName);
    CHECK(cat_id > 0);

    int sc_id = pCore->categoryList_.GetSubCategoryID(cat_id, g_SubCategName);
    CHECK(sc_id > 0);

    sc_id = pCore->categoryList_.GetSubCategoryID(cat_id, "unknown subcategory");
    CHECK(sc_id == -1);
    displayTimeTaken("getSubCategoryID", start_time);
}
//----------------------------------------------------------------------------

TEST(getCategoryName)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int cat_id = pCore->categoryList_.GetCategoryId(g_CategName);
    CHECK(cat_id > 0);

    wxString name = pCore->categoryList_.GetCategoryName(cat_id);
    CHECK(name == g_CategName);

    name = pCore->categoryList_.GetCategoryName(0);
    CHECK(name.empty());
    displayTimeTaken("getCategoryName", start_time);
}
//----------------------------------------------------------------------------

TEST(getSubCategoryName)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int cat_id = pCore->categoryList_.GetCategoryId(g_CategName);
    CHECK(cat_id > 0);

    int sc_id = pCore->categoryList_.GetSubCategoryID(cat_id, g_SubCategName);
    CHECK(sc_id > 0);
    
    wxString name = pCore->categoryList_.GetSubCategoryName(cat_id, sc_id);
    CHECK(name == g_SubCategName);

    name = pCore->categoryList_.GetSubCategoryName(cat_id, 0);
    CHECK(name.empty());
    displayTimeTaken("getSubCategoryName", start_time);
}
//----------------------------------------------------------------------------

TEST(updateCategory)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int cat_id = pCore->categoryList_.GetCategoryId(g_CategName);
    CHECK(cat_id > 0);

    int sc_id = pCore->categoryList_.GetSubCategoryID(cat_id, g_SubCategName);
    CHECK(sc_id > 0);

    // --

    const wxString new_name = "new name";

    bool ok = pCore->categoryList_.UpdateCategory(cat_id, -1, new_name);
    CHECK(ok);

    wxString name = pCore->categoryList_.GetCategoryName(cat_id);
    CHECK(name == new_name);

    ok = pCore->categoryList_.UpdateCategory(cat_id, -1, g_CategName); // restore name
    CHECK(ok);

    // --

    ok = pCore->categoryList_.UpdateCategory(cat_id, sc_id, new_name);
    CHECK(ok);

    name = pCore->categoryList_.GetSubCategoryString(cat_id, sc_id);
    CHECK(name == new_name);

    ok = pCore->categoryList_.UpdateCategory(cat_id, sc_id, g_SubCategName); // restore
    CHECK(ok);
    displayTimeTaken("updateCategory", start_time);
}
//----------------------------------------------------------------------------

TEST(deleteSubCategoryWithConstraints)
{
    wxStopWatch start_time;
    wxSQLite3Database* pDb = get_pDb().get();
    mmCoreDB* pCore = pDb_core().get();

    int cat_id = pCore->categoryList_.GetCategoryId(g_CategName);
    CHECK(cat_id > 0);

    int sc_id = pCore->categoryList_.GetSubCategoryID(cat_id, g_SubCategName);
    CHECK(sc_id > 0);

    bool ok = mmDBWrapper::deleteSubCategoryWithConstraints(pDb, cat_id, sc_id);
    CHECK(ok);
    displayTimeTaken("deleteSubCategoryWithConstraints", start_time);
}
//----------------------------------------------------------------------------

TEST(deleteCategoryWithConstraints)
{
    wxStopWatch start_time;
    wxSQLite3Database* pDb = get_pDb().get();
    mmCoreDB* pCore = pDb_core().get();

    int cat_id = pCore->categoryList_.GetCategoryId(g_CategName);
    CHECK(cat_id > 0);

    bool ok = mmDBWrapper::deleteCategoryWithConstraints(pDb, cat_id);
    CHECK(ok);
    displayTimeTaken("deleteCategoryWithConstraints", start_time);
}
//----------------------------------------------------------------------------

TEST(getCurrencyID)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int id = pCore->currencyList_.getCurrencyID(g_CurrencyName);
    CHECK(id > 0);

    id = pCore->currencyList_.getCurrencyID("unknown currency");
    CHECK(id == -1);
    displayTimeTaken("getCurrencyID", start_time);
}
//----------------------------------------------------------------------------

TEST(getCurrencyName)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int id = pCore->currencyList_.getCurrencyID(g_CurrencyName);
    CHECK(id > 0);

    wxString name = pCore->currencyList_.getCurrencyName(id);
    CHECK(name == g_CurrencyName);

    // --

    name = pCore->currencyList_.getCurrencyName(-1);
    CHECK(name.empty());
    displayTimeTaken("getCurrencyName", start_time);
}
//----------------------------------------------------------------------------

TEST(setBaseCurrencySettings)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int id = pCore->currencyList_.getCurrencyID(g_CurrencyName);
    CHECK(id > 0);
    pCore->currencyList_.SetBaseCurrencySettings(id);

    int base_id = pCore->currencyList_.GetBaseCurrencySettings();

    CHECK(base_id == id);
    displayTimeTaken("setBaseCurrencySettings", start_time);
}
//----------------------------------------------------------------------------

TEST(getBaseCurrencySettings)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int id = pCore->currencyList_.getCurrencyID(g_CurrencyName);
    CHECK(id > 0);

    int base_id = pCore->currencyList_.GetBaseCurrencySettings();
    CHECK(base_id == id);
    displayTimeTaken("getBaseCurrencySettings", start_time);
}
//----------------------------------------------------------------------------

TEST(loadBaseCurrencySettings)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    pCore->currencyList_.LoadBaseCurrencySettings();

    CHECK(true);
    displayTimeTaken("loadBaseCurrencySettings", start_time);
}
//----------------------------------------------------------------------------

#if 0
TEST(getCurrencyBaseConvRate)
{
    const wxDateTime start_time(wxDateTime::UNow());
    wxSQLite3Database* pDb = get_pDb().get();
    mmCoreDB* pCore = pDb_core().get();

    int id = pCore->currencyList_.getCurrencyID(g_CurrencyName);
    CHECK(id > 0);

    double rate = mmDBWrapper::getCurrencyBaseConvRateForId(pDb, id);
    CHECK(rate != 0.0);
    displayTimeTaken("getCurrencyBaseConvRate", start_time);
}
#endif
//----------------------------------------------------------------------------

TEST(load_Currency_Settings)
{
    wxStopWatch start_time;
    wxSQLite3Database* pDb = get_pDb().get();
    mmCoreDB* pCore = pDb_core().get();

    int id = pCore->currencyList_.getCurrencyID(g_CurrencyName);
    CHECK(id > 0);

    mmDBWrapper::loadCurrencySettings(pDb, id);
    displayTimeTaken("load_Currency_Settings", start_time);
}
//----------------------------------------------------------------------------

TEST(setInfoSettingValue)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    const wxString name = "settings name";
    const wxString val  = "settings value";

    pCore->dbInfoSettings_->SetStringSetting(name, val);

    wxString s = pCore->dbInfoSettings_->GetStringSetting(name, "");
    CHECK(s == val);
    pCore->dbInfoSettings_->Save(); // ensure they end up in the database
    displayTimeTaken("setInfoSettingValue", start_time);
}
//----------------------------------------------------------------------------

TEST(getInfoSettingValue)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    const wxString name   = "wrong name";
    const wxString defVal = "default value";

    wxString s = pCore->dbInfoSettings_->GetStringSetting(name, defVal);
    CHECK(s == defVal);

    // --

    pCore->dbInfoSettings_->SetStringSetting(name, defVal);
    s = pCore->dbInfoSettings_->GetStringSetting(name, "");
    CHECK(s == defVal);
    pCore->dbInfoSettings_->Save(); // ensure they end up in the database
    displayTimeTaken("getInfoSettingValue", start_time);
}
//----------------------------------------------------------------------------

TEST(addPayee)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int added = pCore->categoryList_.AddCategory(g_CategName);
    CHECK(added > 0);

    int cat_id = pCore->categoryList_.GetCategoryId(g_CategName);
    CHECK(cat_id > 0);

    added = pCore->categoryList_.AddSubCategory(cat_id, g_SubCategName);
    CHECK(added > 0);

    int sc_id = pCore->categoryList_.GetSubCategoryID(cat_id, g_SubCategName);
    CHECK(sc_id > 0);

    // --

    int payeeID = pCore->payeeList_.AddPayee(g_PayeeName);
    std::shared_ptr<mmPayee> pPayee = pCore->payeeList_.GetPayeeSharedPtr(payeeID);
    pPayee->categoryId_ = cat_id;
    pPayee->subcategoryId_ = sc_id;
    int errCode = pCore->payeeList_.UpdatePayee(pPayee->id_, pPayee->name_);
    CHECK(errCode == 0);

    displayTimeTaken("addPayee", start_time);
}
//----------------------------------------------------------------------------

TEST(getPayeeID)
{
    wxStopWatch start_time;

    mmCoreDB* pCore = pDb_core().get();

    int cat_id = pCore->categoryList_.GetCategoryId(g_CategName);
    CHECK(cat_id > 0);

    int sc_id = pCore->categoryList_.GetSubCategoryID(cat_id, g_SubCategName);
    CHECK(sc_id > 0);

    int id = pCore->payeeList_.GetPayeeId(g_PayeeName);
    CHECK(id > 0);

    wxString name = pCore->payeeList_.GetPayeeName(id);
    CHECK(name == g_PayeeName);

    id = pCore->payeeList_.GetPayeeId("bad payee name");
    CHECK(id < 0);

    displayTimeTaken("getPayeeID", start_time);
}
//----------------------------------------------------------------------------

TEST(getPayee)
{
    wxStopWatch start_time;

    mmCoreDB* pCore = pDb_core().get();

    int payee_id = pCore->payeeList_.GetPayeeId(g_PayeeName);
    CHECK(payee_id != -1);

    std::shared_ptr<mmPayee> pPayee = pCore->payeeList_.GetPayeeSharedPtr(payee_id);
    CHECK(pPayee->name_ == g_PayeeName);
    CHECK(pPayee->categoryId_ != 0);
    CHECK(pPayee->subcategoryId_ != 0);

    wxString name = pCore->payeeList_.GetPayeeName(0);
    CHECK(name.empty());

    displayTimeTaken("getPayee", start_time);
}
//----------------------------------------------------------------------------

TEST(updatePayee)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int payee_id = pCore->payeeList_.GetPayeeId(g_PayeeName);
    CHECK(payee_id != -1);

    std::shared_ptr<mmPayee> pPayee = pCore->payeeList_.GetPayeeSharedPtr(payee_id);
    int cat    = pPayee->categoryId_;
    int subcat = pPayee->subcategoryId_;

    const wxString new_name = "new payee name";

    pPayee->name_ = new_name;
    pPayee->categoryId_ = 0;
    pPayee->subcategoryId_ = -1;
    int errCode = pCore->payeeList_.UpdatePayee(pPayee->id_, pPayee->name_);
    CHECK(errCode == 0);

    // Reset the payee list to match the database
    pPayee.reset();
    pCore->payeeList_.entries_.clear();
    pCore->payeeList_.LoadPayees();

    pPayee = pCore->payeeList_.GetPayeeSharedPtr(payee_id);
    wxString name2 = pCore->payeeList_.GetPayeeName(payee_id);
    CHECK_EQUAL(name2, new_name);
    CHECK(pPayee->categoryId_ != cat);
    CHECK(pPayee->subcategoryId_ != subcat);

    // restore
    pPayee->name_ = g_PayeeName;
    pPayee->categoryId_ = cat;
    pPayee->subcategoryId_ = subcat;
    errCode = pCore->payeeList_.UpdatePayee(pPayee->id_, pPayee->name_);
    CHECK(errCode == 0);

    displayTimeTaken("updatePayee", start_time);
}
//----------------------------------------------------------------------------

TEST(getAmountForPayee)
{
    wxStopWatch start_time;

    mmCoreDB* pCore = pDb_core().get();

    int id = pCore->payeeList_.GetPayeeId(g_PayeeName);
    CHECK(id != -1);

    wxString name = pCore->payeeList_.GetPayeeName(id);
    CHECK(name == g_PayeeName);

    const wxDateTime dt = wxDateTime::Now();
    double amt = pCore->bTransactionList_.getAmountForPayee(id, true, dt, dt);
    CHECK(amt == 0.0);

    displayTimeTaken("getAmountForPayee", start_time);
}
//----------------------------------------------------------------------------

TEST(delete_Payee)
{
    wxStopWatch start_time;

    mmCoreDB* pCore = pDb_core().get();

    int id = pCore->payeeList_.GetPayeeId(g_PayeeName);
    CHECK(id > 0);

    wxString name = pCore->payeeList_.GetPayeeName(id);
    CHECK(name == g_PayeeName);

    bool ok = pCore->payeeList_.RemovePayee(id);

    CHECK(ok);

    ok = pCore->payeeList_.RemovePayee(0);
    CHECK(!ok); // returns true even for wrong id

    displayTimeTaken("delete_Payee", start_time);
}
//----------------------------------------------------------------------------

TEST(getNumAccounts)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int cnt = pCore->accountList_.getNumAccounts();
    CHECK(!cnt);
    displayTimeTaken("getNumAccounts", start_time);
}
//----------------------------------------------------------------------------

TEST(add_new_transactions)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int trans_list_size = pCore->bTransactionList_.transactions_.size();
    CHECK(trans_list_size == 0);

    int account_id = 1;

    // Add a deposit transaction Previous month
    std::shared_ptr<mmBankTransaction> pBankTransaction_1(new mmBankTransaction(pCore->db_));
    pBankTransaction_1->date_      = wxDateTime::Now().Subtract(wxDateSpan::Days(32));
    pBankTransaction_1->accountID_ = account_id;
    pBankTransaction_1->amt_       = 500;
    pBankTransaction_1->toAmt_     = 500;
    pBankTransaction_1->status_    = g_status_reconciled;
    pBankTransaction_1->transType_ = g_TransType_deposit;
    int trans_id_1 = pCore->bTransactionList_.addTransaction(pBankTransaction_1);
    CHECK(trans_id_1 == 1);

    // Add a withdrawal transaction - Today
    std::shared_ptr<mmBankTransaction> pBankTransaction_2(new mmBankTransaction(pCore->db_));
    pBankTransaction_2->date_      = wxDateTime::Now();
    pBankTransaction_2->accountID_ = account_id;
    pBankTransaction_2->amt_       = 200;
    pBankTransaction_2->toAmt_     = 200;
    pBankTransaction_2->status_    = g_status_reconciled;
    pBankTransaction_2->transType_ = g_TransType_withdrawal;
    int trans_id_2 = pCore->bTransactionList_.addTransaction(pBankTransaction_2);
    CHECK(trans_id_2 == 2);

    // Add a void deposit transaction - 2 days before today;
    std::shared_ptr<mmBankTransaction> pBankTransaction_3(new mmBankTransaction(pCore->db_));
    pBankTransaction_3->date_      = wxDateTime::Now().Subtract(wxDateSpan::Days(2));
    pBankTransaction_3->accountID_ = account_id;
    pBankTransaction_3->amt_       = 1200;
    pBankTransaction_3->toAmt_     = 1200;
    pBankTransaction_3->status_    = g_status_void;
    pBankTransaction_3->transType_ = g_TransType_deposit;
    int trans_id_3 = pCore->bTransactionList_.addTransaction(pBankTransaction_3);
    CHECK(trans_id_3 == 3);

    // Checks that the transaction is in the database.
    bool trans_exist = pCore->bTransactionList_.checkForExistingTransaction(pBankTransaction_2);
    CHECK(trans_exist);

    double balance = pCore->bTransactionList_.getBalance(account_id);
    CHECK(balance == 300);

    trans_list_size = pCore->bTransactionList_.transactions_.size();
    CHECK(trans_list_size == 3);

    // Delete the void transaction
    bool trans_deleted_ok = pCore->bTransactionList_.deleteTransaction(account_id, trans_id_3);
    CHECK(trans_deleted_ok);

    // checks that the transaction is not in the database.
    trans_exist = pCore->bTransactionList_.checkForExistingTransaction(pBankTransaction_3);
    CHECK(!trans_exist);

    // This method deletes the transactions from memory but not the database
    // Database transactions need to be deleted individually
    // pCore->bTransactionList_.deleteTransactions(account_id);

    pCore->bTransactionList_.deleteTransaction(account_id, trans_id_1);
    pCore->bTransactionList_.deleteTransaction(account_id, trans_id_2);

    balance = pCore->bTransactionList_.getBalance(account_id);
    CHECK(balance == 0);

    // Check that no transactions are left im memory
    trans_list_size = pCore->bTransactionList_.transactions_.size();
    CHECK(trans_list_size == 0);

    // Check that the transactions are not in the database.
    trans_exist = pCore->bTransactionList_.checkForExistingTransaction(pBankTransaction_1);
    CHECK(!trans_exist);  // 
    trans_exist = pCore->bTransactionList_.checkForExistingTransaction(pBankTransaction_2);
    CHECK(!trans_exist);  // 

    displayTimeTaken("add_new_transactions", start_time);

    //remove empty spaces from the database
    //pCore->db_.get()->Vacuum();
}

TEST(copy_paste_transactions)
{
    wxStopWatch start_time;
    mmCoreDB* pCore = pDb_core().get();

    int account_id = 1;

    std::shared_ptr<mmBankTransaction> pBankTransaction_1(new mmBankTransaction(pCore->db_));
    pBankTransaction_1->date_      = wxDateTime::Now();
    pBankTransaction_1->accountID_ = account_id;
    pBankTransaction_1->amt_       = 100;
    pBankTransaction_1->toAmt_     = 100;
    pBankTransaction_1->status_    = g_status_reconciled;
    pBankTransaction_1->transType_ = g_TransType_withdrawal;
    int trans_id_1 = pCore->bTransactionList_.addTransaction(pBankTransaction_1);
    CHECK(trans_id_1 == 1); // The transaction in previous test was not deleted from the database.

    bool new_trans_exist = pCore->bTransactionList_.checkForExistingTransaction(pBankTransaction_1);
    CHECK(new_trans_exist);

    std::shared_ptr<mmBankTransaction> pCopiedTrans = pCore->bTransactionList_.copyTransaction(trans_id_1, account_id, true);
    int trans_id_2 = pCopiedTrans->transactionID();
    CHECK(trans_id_2 == 2);

    bool copied_trans_exist = pCore->bTransactionList_.checkForExistingTransaction(pCopiedTrans);
    CHECK(copied_trans_exist);

    pCopiedTrans->amt_   = 200;
    pCopiedTrans->toAmt_ = 200;
    pCore->bTransactionList_.UpdateTransaction(pCopiedTrans);

    double balance = pCore->bTransactionList_.getBalance(account_id);
    CHECK(balance == -300);

    int trans_list_size = pCore->bTransactionList_.transactions_.size();
    CHECK(trans_list_size == 2);
    displayTimeTaken("copy_paste_transactions", start_time);
}

} // SUITE

/*****************************************************************************************
 The tests for testing mmex_settings - in a database.
 *****************************************************************************************/
SUITE(Inidb_test)
{

#if 1
TEST(New_INIDB_TEST_1)
{
    wxStopWatch start_time;

	const wxString TEST_NAME_BOOL = "Test 1 BOOL";
	const wxString TEST_NAME_INT  = "Test 1 INTEGER";
	const wxString TEST_NAME_STR  = "Test 1 STRING";
	const wxString TEST_VALUE_STR = "String Value Test 1";

    std::shared_ptr<wxSQLite3Database> pInidb = get_pInidb();
    MMEX_IniSettings* pSettings =  pSettingsList().get();

    bool b = pSettings->GetBoolSetting(TEST_NAME_BOOL, true);
    CHECK_EQUAL(b, true);

    int i = pSettings->GetIntSetting(TEST_NAME_INT, 150);
    CHECK(i == 150);

    wxString s = pSettings->GetStringSetting(TEST_NAME_STR, TEST_VALUE_STR);
    CHECK_EQUAL(s, TEST_VALUE_STR);

    displayTimeTaken("New_INIDB_TEST_1", start_time);
}
#endif

#if 1
TEST(New_INIDB_TEST_2)
{
    wxStopWatch start_time;

    const wxString TEST_NAME_BOOL = "Test 2 BOOL";
	const wxString TEST_NAME_INT  = "Test 2 INTEGER";
	const wxString TEST_NAME_STR  = "Test 2 STRING";
	const wxString TEST_VALUE_STR = "String Value Test 2";

    std::shared_ptr<wxSQLite3Database> pInidb = get_pInidb();
    MMEX_IniSettings* pSettings =  pSettingsList().get();

    pSettings->SetBoolSetting(TEST_NAME_BOOL, true);
    bool b = pSettings->GetBoolSetting(TEST_NAME_BOOL, true);
    CHECK_EQUAL(b, true);

    pSettings->SetIntSetting(TEST_NAME_INT, 200);
    int i = pSettings->GetIntSetting(TEST_NAME_INT, 150);
    CHECK_EQUAL(i, 200);

    pSettings->SetStringSetting(TEST_NAME_STR, TEST_VALUE_STR);
    wxString s = pSettings->GetStringSetting(TEST_NAME_STR, TEST_VALUE_STR);
    CHECK_EQUAL(s, TEST_VALUE_STR);

    displayTimeTaken("New_INIDB_TEST_2", start_time);
}
#endif

#if 1
TEST(New_INIDB_TEST_3)
{
    wxStopWatch start_time;

	const wxString TEST_NAME_BOOL = "Test 3 BOOL";
	const wxString TEST_NAME_INT  = "Test 3 INTEGER";
	const wxString TEST_NAME_STR  = "Test 3 STRING";

    const wxString TEST_VALUE_STR_NEW = "String Value Test 3";
	const wxString TEST_VALUE_STR_ADJ = "String Value Test 3_adjusted";

    std::shared_ptr<wxSQLite3Database> pInidb = get_pInidb();
    MMEX_IniSettings* pSettings =  pSettingsList().get();

    // New settings being initialised
    pSettings->SetBoolSetting(TEST_NAME_BOOL, true);
    bool b = pSettings->GetBoolSetting(TEST_NAME_BOOL, false);
    CHECK_EQUAL(b, true);

    pSettings->SetIntSetting(TEST_NAME_INT, 300);
    int i = pSettings->GetIntSetting(TEST_NAME_INT, 150);
    CHECK_EQUAL(i, 300);

    pSettings->SetStringSetting(TEST_NAME_STR, TEST_VALUE_STR_NEW);
    wxString s = pSettings->GetStringSetting(TEST_NAME_STR, TEST_VALUE_STR_NEW);
    CHECK_EQUAL(s, TEST_VALUE_STR_NEW);

    // Existing settings being changed
    pSettings->SetBoolSetting(TEST_NAME_BOOL, false);
    b = pSettings->GetBoolSetting(TEST_NAME_BOOL, true);
    CHECK_EQUAL(b, false);

    pSettings->SetIntSetting(TEST_NAME_INT, 600);
    i = pSettings->GetIntSetting(TEST_NAME_INT, 150);
    CHECK_EQUAL(i, 600);

    pSettings->SetStringSetting(TEST_NAME_STR, TEST_VALUE_STR_ADJ);
    s = pSettings->GetStringSetting(TEST_NAME_STR, TEST_VALUE_STR_NEW);
    CHECK_EQUAL(s, TEST_VALUE_STR_ADJ);

    displayTimeTaken("New_INIDB_TEST_3", start_time);
}
#endif

#if 1
TEST(New_INIDB_TEST_4)
{
    wxStopWatch start_time;

    std::shared_ptr<wxSQLite3Database> pInidb = get_pInidb();
    MMEX_IniSettings* pSettings =  pSettingsList().get();

    wxString test_name; 
    for(int i = 0; i < 100; ++i)
    {
        test_name = "Inidb Test ";
        test_name << i;
        pSettings->SetIntSetting(test_name, i);
    }
    int value = pSettings->GetIntSetting(test_name, 50);
    CHECK_EQUAL(99, value);
    pSettings->Save();

    displayTimeTaken("New_INIDB_TEST_4", start_time);
}
#endif

#if 1
TEST(New_INIDB_TEST_5)
{
    wxStopWatch start_time;

    MMEX_IniSettings* pSettings =  pSettingsList().get();
    pSettings->Load();

    wxString test_name; 
    for(int i = 0; i < 100; ++i)
    {
        test_name = "Inidb Test ";
        test_name << i;
        int value = pSettings->GetIntSetting(test_name, 50);
        CHECK_EQUAL(i, value);
    }

    displayTimeTaken("New_INIDB_TEST_5", start_time);
}
#endif

#if 1
TEST(New_INFO_DB_TEST_1)
{
    wxStopWatch start_time;

    //  Using the details from the master table database
    std::shared_ptr<wxSQLite3Database> pdb = get_pDb();

    MMEX_IniSettings pdb_settings(pdb, true);

    const wxString name    = "New_info_test_1: setting name";
    const wxString val     = "New_info_test_1: setting value";
    const wxString new_val = "New_info_test_1: new setting value";

    wxString test_name =  pdb_settings.GetStringSetting(name, val);
    CHECK_EQUAL(test_name, val);

    pdb_settings.SetStringSetting(name, new_val);
    test_name =  pdb_settings.GetStringSetting(name, val);
    CHECK_EQUAL(test_name, new_val);
    pdb_settings.Save();

    displayTimeTaken("New_INFO_DB_TEST_1", start_time);
}
#endif

#if 1
TEST(New_INFO_DB_TEST_2)
{
    wxStopWatch start_time;

    // Using the details from the global ini database
    std::shared_ptr<wxSQLite3Database> pdb = get_pInidb();

    MMEX_IniSettings pdb_settings(pdb, true);

    const wxString name    = "New_info_test_2: setting name";
    const wxString val     = "New_info_test_2: setting value";
    const wxString new_val = "New_info_test_2: new setting value";

    wxString test_name =  pdb_settings.GetStringSetting(name, val);
    CHECK_EQUAL(test_name, val);

    pdb_settings.SetStringSetting(name, new_val);
    test_name =  pdb_settings.GetStringSetting(name, val);
    CHECK_EQUAL(test_name, new_val);
    pdb_settings.Save();

    displayTimeTaken("New_INFO_DB_TEST_2", start_time);
}
#endif

#if 1
TEST(New_INFO_DB_TEST_3)
{
    wxStopWatch start_time;

    //  Save the details in the main database
    std::shared_ptr<wxSQLite3Database> pdb = get_pDb();

    MMEX_IniSettings pdb_settings(pdb, true);

    const wxString name    = "New_info_test_3: setting name";
    const wxString val     = "New_info_test_3: setting value";
    const wxString new_val = "New_info_test_3: new setting value";

    wxString test_name =  pdb_settings.GetStringSetting(name, val);
    CHECK_EQUAL(test_name, val);

    pdb_settings.SetStringSetting(name, new_val);
    test_name =  pdb_settings.GetStringSetting(name, val);
    CHECK_EQUAL(test_name, new_val);

    const wxString prev_name = "settings name";
    const wxString prev_val  = "settings value";

    test_name =  pdb_settings.GetStringSetting(prev_name, "");
    CHECK_EQUAL(test_name, prev_val);
    pdb_settings.Save();

    displayTimeTaken("New_INFO_DB_TEST_3", start_time);
}
#endif

} // SUITE end Inidb_test

#endif // DBWRAPPER_TEST_INCLUDED_IN_BUILD
//----------------------------------------------------------------------------
