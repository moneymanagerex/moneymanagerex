/*******************************************************
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

#include "constants.h"
#include "mmtransaction.h"
#include "util.h"
#include "mmOption.h"
#include "mmcoredb.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"
#include "model/Model_Category.h"
#include <algorithm>

mmSplitTransactionEntries::~mmSplitTransactionEntries()
{
    cleanuplist();
}

void mmSplitTransactionEntries::cleanuplist()
{
    for (auto& entry : entries_)
        delete entry;
}

void mmSplitTransactionEntries::addSplit(mmSplitTransactionEntry* split)
{
    entries_.push_back(split);
}

double mmSplitTransactionEntries::getTotalSplits() const
{
    double total = 0.0;
    for (const auto& tran: this->entries_)
        total += tran->splitAmount_;

    return total;
}

void mmSplitTransactionEntries::removeSplit(int splitID)
{
    for (size_t i = 0; i < entries_.size(); ++i)
    {
        if (entries_[i]->splitEntryID_ == splitID)
        {
            delete entries_[i];
            entries_.erase(entries_.begin() + i);
            break;
        }
    }
}

void mmSplitTransactionEntries::removeSplitByIndex(int splitIndex)
{
    delete entries_[splitIndex];
    entries_.erase(entries_.begin() + splitIndex);
}

void mmSplitTransactionEntries::updateToDB(wxSharedPtr<wxSQLite3Database>& db,
    int transID, bool edit)
{
    if (edit)
    {
        wxSQLite3Statement st = db->PrepareStatement(DELETE_TRANSID_SPLITTRANSACTIONS_V1);
        st.Bind(1, transID);
        st.ExecuteUpdate();
        st.Finalize();
    }

    wxSQLite3Statement st = db->PrepareStatement(INSERT_INTO_SPLITTRANSACTIONS_V1);

    for (size_t i = 0; i < entries_.size(); ++i)
    {
        mmSplitTransactionEntry &r = *entries_[i];

        st.Bind(1, transID);
        st.Bind(2, r.categID_);
        st.Bind(3, r.subCategID_);
        st.Bind(4, r.splitAmount_);

        st.ExecuteUpdate();
        r.splitEntryID_ = db->GetLastRowId().ToLong();

        st.Reset();
    }

    st.Finalize();
}

void mmSplitTransactionEntries::loadFromBDDB(mmCoreDB* core, int bdID)
{
    cleanuplist();
    entries_.clear();

    wxSQLite3Statement st = core->db_->PrepareStatement(SELECT_ROW_FROM_BUDGETSPLITTRANSACTIONS_V1);
   st.Bind(1, bdID);

   wxSQLite3ResultSet q1 = st.ExecuteQuery();
   while (q1.NextRow())
   {
      mmSplitTransactionEntry* pSplitEntry(new mmSplitTransactionEntry());
      pSplitEntry->splitEntryID_ = q1.GetInt("SPLITTRANSID");
      pSplitEntry->splitAmount_  = q1.GetDouble("SPLITTRANSAMOUNT");

      int catID = q1.GetInt("CATEGID");
      int subID = q1.GetInt("SUBCATEGID");

      pSplitEntry->categID_ = catID;
      pSplitEntry->subCategID_ = subID;

      addSplit(pSplitEntry);
   }

    st.Finalize();
}
//-----------------------------------------------------------------------------//
mmBankTransaction::mmBankTransaction(mmCoreDB* core) :
    mmTransaction(-1),
    core_(core)
{
    splitEntries_ = new mmSplitTransactionEntries();
    sortby_ = DATE;
}

mmBankTransaction::mmBankTransaction(mmCoreDB* core, wxSQLite3ResultSet& q1)
: mmTransaction(q1.GetInt("TRANSID")),
                core_(core)
{
    date_        = q1.GetDate("TRANSDATE");
    transNum_    = q1.GetString("TRANSACTIONNUMBER");
    status_      = q1.GetString("STATUS");
    notes_       = q1.GetString("NOTES");
    accountID_   = q1.GetInt("ACCOUNTID");
    toAccountID_ = q1.GetInt("TOACCOUNTID");
    payeeID_     = q1.GetInt("PAYEEID");
    transType_   = q1.GetString("TRANSCODE");
    amt_         = q1.GetDouble("TRANSAMOUNT");
    toAmt_       = q1.GetDouble("TOTRANSAMOUNT");
    followupID_  = q1.GetInt("FOLLOWUPID");
    categID_     = q1.GetInt("CATEGID");
    subcategID_  = q1.GetInt("SUBCATEGID");

   // core->accountList_.getCurrencySharedPtr(accountID_)->loadCurrencySettings();

    splitEntries_ = new mmSplitTransactionEntries();
    getSplitTransactions(splitEntries_);
    sortby_ = DATE;
}

mmBankTransaction::~mmBankTransaction()
{
    if(splitEntries_)
        delete splitEntries_;
}

bool mmBankTransaction::operator < (const mmBankTransaction& tran) const
{
    wxString value1, value2;
    const Model_Account::Data* account1 = Model_Account::instance().get(this->accountID_);
    const Model_Account::Data* account2 = Model_Account::instance().get(tran.accountID_);
    switch(sortby_)
    {
    case ACCOUNT:
        value1 = account1->ACCOUNTNAME;
        value2 = account2->ACCOUNTNAME;
        if (value1 < value2) return true; else if (value1 > value2) return false;

        if (this->date_ < tran.date_) return true; else if (this->date_ > tran.date_) return false;
        break;
    case PAYEE:
        value1 = (this->transType_ != TRANS_TYPE_TRANSFER_STR) ? Model_Payee::instance().get(this->payeeID_)->PAYEENAME : "";
        value2 = (tran.transType_ != TRANS_TYPE_TRANSFER_STR ? Model_Payee::instance().get(tran.payeeID_)->PAYEENAME : "");
        if (value1 < value2) return true; else if (value1 > value2) return false;

        if (this->date_ < tran.date_) return true; else if (this->date_ > tran.date_) return false;

        value1 = account1->ACCOUNTNAME;
        value2 = account2->ACCOUNTNAME;
        if (value1 < value2) return true; else if (value1 > value2) return false;
        break;
    case STATUS:
        if (this->status_ < tran.status_) return true; else if (this->status_ > tran.status_) return false;

        if (this->date_ < tran.date_) return true; else if (this->date_ > tran.date_) return false;

        value1 = account1->ACCOUNTNAME;
        value2 = account2->ACCOUNTNAME;
        if (value1 < value2) return true; else if (value1 > value2) return false;
        break;
    case CATEGORY:
        if (this->fullCatStr_ < tran.fullCatStr_) return true; else if (this->fullCatStr_ > tran.fullCatStr_) return false;

        if (this->date_ < tran.date_) return true; else if (this->date_ > tran.date_) return false;

        value1 = account1->ACCOUNTNAME;
        value2 = account2->ACCOUNTNAME;
        if (value1 < value2) return true; else if (value1 > value2) return false;
        break;
    case TYPE:
        if (this->transType_ < tran.transType_) return true; else if (this->transType_ > tran.transType_) return false;

        if (this->date_ < tran.date_) return true; else if (this->date_ > tran.date_) return false;

        value1 = account1->ACCOUNTNAME;
        value2 = account2->ACCOUNTNAME;
        if (value1 < value2) return true; else if (value1 > value2) return false;
        break;
    case AMOUNT:
        if (this->value(-1) < tran.value(-1)) return true; else if (this->value(-1) > tran.value(-1)) return false;

        if (this->date_ < tran.date_) return true; else if (this->date_ > tran.date_) return false;

        value1 = account1->ACCOUNTNAME;
        value2 = account2->ACCOUNTNAME;
        if (value1 < value2) return true; else if (value1 > value2) return false;
        break;
    case NUMBER:
        if (this->transNum_ < tran.transNum_) return true; else if (this->transNum_ > tran.transNum_) return false;

        if (this->date_ < tran.date_) return true; else if (this->date_ > tran.date_) return false;

        value1 = account1->ACCOUNTNAME;
        value2 = account2->ACCOUNTNAME;
        if (value1 < value2) return true; else if (value1 > value2) return false;
        break;
    case NOTE:
        if (this->notes_ < tran.notes_) return true; else if (this->notes_ > tran.notes_) return false;

        if (this->date_ < tran.date_) return true; else if (this->date_ > tran.date_) return false;

        value1 = account1->ACCOUNTNAME;
        value2 = account2->ACCOUNTNAME;
        if (value1 < value2) return true; else if (value1 > value2) return false;
        break;
    default: // DATE
        if (this->date_ < tran.date_) return true; else if (this->date_ > tran.date_) return false;

        value1 = account1->ACCOUNTNAME;
        value2 = account2->ACCOUNTNAME;
        if (value1 < value2) return true; else if (value1 > value2) return false;
    }
    return this->transactionID_ < tran.transactionID_;
}

void mmBankTransaction::updateTransactionData(int accountID, double& balance)
{
    payeeStr_ = "";
    wxASSERT(toAmt_ >= 0);
    wxASSERT(amt_ >= 0);
    if (toAmt_ < 0) toAmt_ = amt_;

    if (transType_ != TRANS_TYPE_TRANSFER_STR)
    {
        Model_Payee::Data* payee = Model_Payee::instance().get(payeeID_);
        if (payee) payeeStr_ = payee->PAYEENAME;
        deposit_amt_ = (transType_ == TRANS_TYPE_DEPOSIT_STR ? amt_ : -amt_);
        withdrawal_amt_ = (transType_ == TRANS_TYPE_WITHDRAWAL_STR ? amt_ : -amt_);
        if (status_ != "V" && status_ != "X")
        {
            balance += (transType_ == TRANS_TYPE_DEPOSIT_STR) ? amt_ : -amt_;
        }
        arrow_ = "   ";
    }
    else
    {
        Model_Account::Data* account = Model_Account::instance().get(accountID_); 
        if (account) fromAccountStr_ = account->ACCOUNTNAME;

        if (accountID_ != toAccountID_)
        {
            if (accountID_ == accountID)
            {
                 balance -= (status_ == "V" || status_ == "X") ? 0.0 : amt_;
                 withdrawal_amt_ = amt_;
                 deposit_amt_ = -amt_;
                 Model_Account::Data* to_account = Model_Account::instance().get(toAccountID_);
                 if (to_account) payeeStr_ = to_account->ACCOUNTNAME;
                 arrow_ = "> ";
            }
            else if (toAccountID_ == accountID)
            {
                 balance += (status_ == "V" || status_ == "X") ? 0.0 : toAmt_;
                 deposit_amt_ = toAmt_;
                 withdrawal_amt_ = -toAmt_;
                 payeeStr_ = fromAccountStr_;
                 arrow_ = "< ";
            }
        }
    }

    balance_ = balance;

    if (splitEntries_->numEntries() == 1)
    {
        categID_ = -1;
        subcategID_ = -1;
        Model_Category::Data* category = Model_Category::instance().get(splitEntries_->entries_[0]->categID_);
        Model_Subcategory::Data* sub_category = (splitEntries_->entries_[0]->subCategID_ != -1 ? Model_Subcategory::instance().get(splitEntries_->entries_[0]->subCategID_) : 0);
        fullCatStr_= Model_Category::full_name(category, sub_category);
    }
    else if (splitEntries_->numEntries() > 1)
    {
        fullCatStr_ = "";
        categID_ = -1;
        subcategID_ = -1;
    }
    else
    {
        Model_Category::Data* category = Model_Category::instance().get(categID_);
        Model_Subcategory::Data* sub_category = (subcategID_ != -1 ? Model_Subcategory::instance().get(subcategID_) : 0);

        fullCatStr_ = Model_Category::full_name(category, sub_category);
    }
}

double mmBankTransaction::value(int accountID) const
{
    double balance = 0.0;
    if (transType_ == TRANS_TYPE_DEPOSIT_STR)
       balance = amt_;
    else if (transType_== TRANS_TYPE_WITHDRAWAL_STR)
       balance -= amt_;
    else if (transType_ == TRANS_TYPE_TRANSFER_STR)
    {
        //Bug fix for broken transactions (as result of wrong import) if account and to account the same
        if (accountID_ != toAccountID_)
        {
            if (accountID_ == accountID)
                 balance -= amt_;
            else if (toAccountID_ == accountID)
                 balance += toAmt_;
        }
    }

    return balance;
}

void mmBankTransaction::getSplitTransactions(mmSplitTransactionEntries* splits) const
{
    for (auto & pEntry: splits->entries_)
        delete pEntry;

    splits->entries_.clear();

    wxSQLite3Statement st = core_->db_->PrepareStatement(SELECT_ROW_FROM_SPLITTRANSACTIONS_V1);
    st.Bind(1, transactionID());

    wxSQLite3ResultSet q1 = st.ExecuteQuery();

    while (q1.NextRow())
    {
        mmSplitTransactionEntry* pSplitEntry(new mmSplitTransactionEntry);

        pSplitEntry->splitEntryID_ = q1.GetInt("SPLITTRANSID");
        pSplitEntry->splitAmount_  = q1.GetDouble("SPLITTRANSAMOUNT");

        int catID = q1.GetInt("CATEGID");
        int subID = q1.GetInt("SUBCATEGID");

        pSplitEntry->categID_ = catID;
        pSplitEntry->subCategID_ = subID;


        splits->addSplit(pSplitEntry);
    }

    st.Finalize();
}

bool mmBankTransaction::containsCategory(int categID, int subcategID, bool ignoreSubCateg) const
{
    bool found = false;
    if (!splitEntries_->numEntries())
    {
        found = (categID_ == categID) && (subcategID_ == subcategID || ignoreSubCateg);
    }
    else
    {
        for (const auto & pSplitEntry: splitEntries_->entries_)
        {
            if (pSplitEntry->categID_ == categID &&
                (ignoreSubCateg || pSplitEntry->subCategID_ == subcategID))
            {
                found = true;
                break;
            }
        }
    }

    return found;
}

double mmBankTransaction::getAmountForSplit(int categID, int subcategID) const
{
    double splitAmount = 0.0;
    if (splitEntries_->numEntries())
    {
        for (const auto & pSplitEntry: splitEntries_->entries_)
        {
            if ((pSplitEntry->categID_ == categID) &&
                (pSplitEntry->subCategID_ == subcategID))
            {
                splitAmount += pSplitEntry->splitAmount_;
            }
        }
    }
    else if ((categID_ == categID) &&
            (subcategID_ == subcategID))
    {
        splitAmount = amt_;
    }

    return splitAmount;
}

//-----------------------------------------------------------------------------//
mmBankTransactionList::mmBankTransactionList(mmCoreDB* core)
: core_(core)
{
   /* Allocate some empty space so loading transactions is faster */
   transactions_.reserve(5000);
}

void mmBankTransactionList::cleanuptranslist()
{
    for (const auto& pBankTransaction : transactions_)
        delete pBankTransaction;
}

int mmBankTransactionList::addTransaction(mmBankTransaction* pBankTransaction)
{
    mmBankTransaction &r = *pBankTransaction;
    if (checkForExistingTransaction(pBankTransaction))
    {
       r.status_ = "D";
    }

    Model_Payee::Data* payee = Model_Payee::instance().get(r.payeeID_);
    if (!payee)
    {
        payee = Model_Payee::instance().create();
        payee->PAYEENAME = _("Unknown");
        Model_Payee::instance().save(payee);
    }

    wxSQLite3Statement st = core_->db_.get()->PrepareStatement(INSERT_INTO_CHECKINGACCOUNT_V1);

    int i = 0;
    st.Bind(++i, r.accountID_);
    st.Bind(++i, r.toAccountID_);
    st.Bind(++i, r.payeeID_);
    st.Bind(++i, r.transType_);
    st.Bind(++i, r.amt_);
    st.Bind(++i, r.status_);
    st.Bind(++i, r.transNum_);
    st.Bind(++i, r.notes_);
    st.Bind(++i, r.categID_);
    st.Bind(++i, r.subcategID_);
    st.Bind(++i, r.date_.FormatISODate());
    st.Bind(++i, r.followupID_);
    st.Bind(++i, r.toAmt_);

    wxASSERT(st.GetParamCount() == i);
    st.ExecuteUpdate();

    r.transactionID(core_->db_.get()->GetLastRowId().ToLong());
    st.Finalize();
    mmOptions::instance().databaseUpdated_ = true;

    r.splitEntries_->updateToDB(core_->db_, r.transactionID(), false);
    transactions_.push_back(pBankTransaction);

    return pBankTransaction->transactionID();
}

bool mmBankTransactionList::checkForExistingTransaction(mmBankTransaction* pBankTransaction)
{
    bool found = false;

    wxSQLite3Statement st = core_->db_.get()->PrepareStatement(IS_TRX_IN_CHECKINGACCOUNT_V1);
    const mmBankTransaction &r = *pBankTransaction;

    int i = 0;
    st.Bind(++i, r.accountID_);
    st.Bind(++i, (r.transType_ == TRANS_TYPE_TRANSFER_STR ? r.toAccountID_ : -1));
    st.Bind(++i, (r.transType_ == TRANS_TYPE_TRANSFER_STR ? -1 : r.payeeID_));
    st.Bind(++i, r.transType_);
    st.Bind(++i, r.amt_);
    st.Bind(++i, r.transNum_);
    st.Bind(++i, r.notes_);
    st.Bind(++i, r.categID_);
    st.Bind(++i, r.subcategID_);
    st.Bind(++i, r.date_.FormatISODate());
    st.Bind(++i, r.toAmt_ );

    wxASSERT(st.GetParamCount() == i);

    wxSQLite3ResultSet q1 = st.ExecuteQuery();
    while (q1.NextRow() && !found)
    {
        int transactionID = q1.GetInt("TRANSID");
        if (pBankTransaction->categID_ == -1)
        {
            mmSplitTransactionEntries* splits = pBankTransaction->splitEntries_;

            mmBankTransaction* pTempTransaction = getBankTransactionPtr(transactionID);
            mmSplitTransactionEntries* temp_splits = pTempTransaction->splitEntries_;

            if (splits->entries_.size() != temp_splits->entries_.size())
            {
                delete pTempTransaction;
                continue;
            }

            for (int i = 0; i < (int)splits->entries_.size(); ++i)
            {
                if (splits->entries_[i]->splitAmount_ != temp_splits->entries_[i]->splitAmount_)
                    continue;
                if (splits->entries_[i]->categID_ != temp_splits->entries_[i]->categID_)
                    continue;
                if (splits->entries_[i]->subCategID_ != temp_splits->entries_[i]->subCategID_)
                    continue;
            }

            delete pTempTransaction;
        }
    found = true;
    }
    st.Finalize();

    return found;
}

mmBankTransaction* mmBankTransactionList::copyTransaction(
   const long transactionID, const long accountID, const bool useOriginalDate)
{
    mmBankTransaction* pBankTransaction = getBankTransactionPtr(transactionID);
    if (!pBankTransaction)
       return NULL;

    mmBankTransaction* pCopyTransaction(new mmBankTransaction(core_));

    if (pBankTransaction->transType_!=TRANS_TYPE_TRANSFER_STR)
        pCopyTransaction->accountID_ = accountID;
    else
    {
        if (accountID != pBankTransaction->accountID_ && accountID != pBankTransaction->toAccountID_)
            pCopyTransaction->accountID_ = accountID;
        else
            pCopyTransaction->accountID_ = pBankTransaction->accountID_;
    }
    pCopyTransaction->date_        = (useOriginalDate ? pBankTransaction->date_ : wxDateTime::Now());
    pCopyTransaction->amt_         = pBankTransaction->amt_;
    pCopyTransaction->toAmt_       = pBankTransaction->toAmt_;
    pCopyTransaction->transType_   = pBankTransaction->transType_;
    pCopyTransaction->status_      = pBankTransaction->status_;
    pCopyTransaction->payeeStr_    = pBankTransaction->payeeStr_;
    pCopyTransaction->payeeID_     = pBankTransaction->payeeID_;
    pCopyTransaction->categID_     = pBankTransaction->categID_;
    pCopyTransaction->subcategID_  = pBankTransaction->subcategID_;
    pCopyTransaction->followupID_  = pBankTransaction->followupID_;
    pCopyTransaction->toAccountID_ = pBankTransaction->toAccountID_;
    pCopyTransaction->transNum_    = pBankTransaction->transNum_;
    pCopyTransaction->notes_       = pBankTransaction->notes_;

    // we need to create a new pointer for Split transactions.
    mmSplitTransactionEntries* splitTransEntries(new mmSplitTransactionEntries());
    pBankTransaction->getSplitTransactions(splitTransEntries);
    pCopyTransaction->splitEntries_->entries_ = splitTransEntries->entries_;

    if (checkForExistingTransaction(pCopyTransaction)) pCopyTransaction->status_ = "D";

    wxSQLite3Statement st = core_->db_.get()->PrepareStatement(INSERT_INTO_CHECKINGACCOUNT_V1);
    const mmBankTransaction &r = *pBankTransaction;

    int i = 0;
    st.Bind(++i, pCopyTransaction->accountID_);
    st.Bind(++i, r.toAccountID_);
    st.Bind(++i, r.payeeID_);
    st.Bind(++i, r.transType_);
    st.Bind(++i, r.amt_);
    st.Bind(++i, r.status_);
    st.Bind(++i, r.transNum_);
    st.Bind(++i, r.notes_);
    st.Bind(++i, r.categID_);
    st.Bind(++i, r.subcategID_);
    st.Bind(++i, pCopyTransaction->date_.FormatISODate());
    st.Bind(++i, r.followupID_ );
    st.Bind(++i, r.toAmt_ );

    wxASSERT(st.GetParamCount() == i);
    st.ExecuteUpdate();

    pCopyTransaction->transactionID(core_->db_.get()->GetLastRowId().ToLong());
    st.Finalize();

    pCopyTransaction->splitEntries_->updateToDB(core_->db_, pCopyTransaction->transactionID(), false);
    transactions_.push_back(pCopyTransaction);

    delete pBankTransaction;

    return pCopyTransaction;
}

mmBankTransaction* mmBankTransactionList::getBankTransactionPtr(int transactionID) const
{
    for (const auto& pBankTransaction : transactions_)
    {

        if (pBankTransaction->transactionID() == transactionID)
        {
            return pBankTransaction;
        }
    }
    // didn't find the transaction
    wxASSERT(false);
    return NULL;
}

void mmBankTransactionList::LoadTransactions()
{
    wxSQLite3ResultSet q1 = core_->db_.get()->ExecuteQuery(SELECT_ALL_FROM_CHECKINGACCOUNT_V1);
    cleanuptranslist();
    transactions_.clear();
    while (q1.NextRow())
    {
        mmBankTransaction* pTransaction(new mmBankTransaction(core_, q1));
        transactions_.push_back(pTransaction);
    }

    q1.Finalize();
}

void mmBankTransactionList::LoadAccountTransactions(int accountID, double& account_balance, double& reconciled_balance)
{
    wxDateTime today = wxDateTime::Now().GetDateOnly();
    Model_Account::Data* account = Model_Account::instance().get(accountID);
    account_balance = account->INITIALBAL;
    reconciled_balance = account->INITIALBAL;
    double balance = account->INITIALBAL;
    //TODO: get parameter mmIniOptions::instance().ignoreFutureTransactions_;
    bool calculate_future = true;
    accountTransactions_.clear();

    for (const auto& pBankTransaction: transactions_)
    {
        if (pBankTransaction->accountID_ == accountID
            || (pBankTransaction->transType_ == TRANS_TYPE_TRANSFER_STR
                && pBankTransaction->toAccountID_ == accountID))
        {
            accountTransactions_.push_back(pBankTransaction);
        }
    }

    //Default sorting
    std::stable_sort(this->accountTransactions_.begin(), this->accountTransactions_.end()
        , [] (const mmBankTransaction* x, const mmBankTransaction* y)
        {
            if (x->date_ != y->date_) return x->date_ < y->date_;
            else return x->transactionID() < y->transactionID();
        }
    );

    //Balance calculations
    for (const auto& pBankTransaction: accountTransactions_)
    {
        pBankTransaction->updateTransactionData(accountID, balance);
        calculate_future = calculate_future || (pBankTransaction->date_ <= today);
        double amount = pBankTransaction->value(accountID);
        if (pBankTransaction->status_ == "R" && calculate_future) reconciled_balance += amount;
        if (
                pBankTransaction->status_ != "V" 
                && pBankTransaction->status_ != "X" 
                && calculate_future
            )
            account_balance += amount;
    }
}

void mmBankTransactionList::UpdateTransaction(mmBankTransaction* pBankTransaction)
{
    if (pBankTransaction->transType_ == TRANS_TYPE_TRANSFER_STR)
        pBankTransaction->payeeID_ = -1;

    wxSQLite3Statement st = core_->db_.get()->PrepareStatement(UPDATE_CHECKINGACCOUNT_V1);
    mmBankTransaction &r = *pBankTransaction;

    int i = 0;
    st.Bind(++i, r.accountID_);
    st.Bind(++i, r.toAccountID_);
    st.Bind(++i, r.payeeID_);
    st.Bind(++i, r.transType_);
    st.Bind(++i, r.amt_);
    st.Bind(++i, r.status_);
    st.Bind(++i, r.transNum_);
    st.Bind(++i, r.notes_);
    st.Bind(++i, r.categID_);
    st.Bind(++i, r.subcategID_);
    st.Bind(++i, r.date_.FormatISODate());
    st.Bind(++i, r.followupID_);
    st.Bind(++i, r.toAmt_);
    st.Bind(++i, r.transactionID());

    wxASSERT(st.GetParamCount() == i);
    st.ExecuteUpdate();

    r.splitEntries_->updateToDB(core_->db_, r.transactionID(), true);
    st.Finalize();

    for (auto& i : transactions_)
    {
        if (pBankTransaction->transactionID() == i->transactionID())
        {
            i = pBankTransaction;
            i->payeeStr_ = pBankTransaction->payeeStr_;
            break;
        }
    }
    mmOptions::instance().databaseUpdated_ = true;
}

void mmBankTransactionList::getExpensesIncomeStats
    (std::map<int, std::pair<double, double> > &incomeExpensesStats
    , mmDateRange* date_range
    , int accountID
    , bool group_by_account
    , bool group_by_month) const
{
    //Initialization
    bool ignoreFuture = mmIniOptions::instance().ignoreFutureTransactions_;
    std::pair<double, double> incomeExpensesPair;
    incomeExpensesPair.first = 0;
    incomeExpensesPair.second = 0;
    wxDateTime start_date = wxDateTime(date_range->end_date()).SetDay(1);
    //Store base currency rates for all accounts
    std::map<int, double> acc_conv_rates;
    double convRate = 1;
    for (const auto& account: Model_Account::instance().all())
    {
        Model_Currency::Data* currency = Model_Account::currency(account);
        acc_conv_rates[account.ACCOUNTID] = currency->BASECONVRATE;

        //Set std::map with zerros
        int i = group_by_month ? 12 : 1;
        for (int m = 0; m < i; m++)
        {
            wxDateTime d = wxDateTime(start_date).Subtract(wxDateSpan::Months(m));
            int idx = group_by_account ? (1000000 * account.ACCOUNTID) : 0;
            idx += group_by_month ? (d.GetYear()*100 + (int)d.GetMonth()) : 0;
            incomeExpensesStats[idx] = incomeExpensesPair;
        }
    }

    //Calculations
    for (const auto& pBankTransaction : transactions_)
    {
        if (pBankTransaction->status_ == "V") continue; // skip
        if (accountID != -1)
        {
            if (pBankTransaction->accountID_ != accountID && pBankTransaction->toAccountID_ != accountID)
                continue; // skip
        }
        wxDateTime trxDate = pBankTransaction->date_.GetDateOnly();
        if (ignoreFuture)
        {
            if (trxDate.IsLaterThan(wxDateTime::Now().GetDateOnly()))
                continue; //skip future dated transactions
        }

        if (!trxDate.IsBetween(date_range->start_date(), date_range->end_date()))
            continue; //skip

        // We got this far, get the currency conversion rate for this account
        convRate = acc_conv_rates[pBankTransaction->accountID_];

        int idx = group_by_account ? (1000000 * pBankTransaction->accountID_) : 0;
        idx += group_by_month ? (trxDate.GetYear()*100 + (int)trxDate.GetMonth()) : 0;

        if (pBankTransaction->transType_ == TRANS_TYPE_DEPOSIT_STR)
            incomeExpensesStats[idx].first += pBankTransaction->amt_ * convRate;
        else if (pBankTransaction->transType_ == TRANS_TYPE_WITHDRAWAL_STR)
            incomeExpensesStats[idx].second += pBankTransaction->amt_ * convRate;
        else if (pBankTransaction->transType_ == TRANS_TYPE_TRANSFER_STR)
        {
            // transfers are not considered in income/expenses calculations
        }
    }
}

void mmBankTransactionList::getCategoryStats(
    std::map<int, std::map<int, std::map<int, double> > > &categoryStats
    , mmDateRange* date_range, bool ignoreFuture
    , bool group_by_month, bool with_date) const
{
    //Initialization
    //Get base currency rates for all accounts
    std::map<int, double> acc_conv_rates;
    for (const auto& account: Model_Account::instance().all())
    {
        Model_Currency::Data* currency = Model_Account::currency(account);
        acc_conv_rates[account.ACCOUNTID] = currency->BASECONVRATE;
    }
    //Set std::map with zerros
    double value = 0;
    for (const auto& category: Model_Category::instance().all())
    {
        int columns = group_by_month ? 12 : 1;
        wxDateTime start_date = wxDateTime(date_range->end_date()).SetDay(1);
        for (int m = 0; m < columns; m++)
        {
            wxDateTime d = wxDateTime(start_date).Subtract(wxDateSpan::Months(m));
            int idx = group_by_month ? (d.GetYear()*100 + (int)d.GetMonth()) : 0;
            categoryStats[category.CATEGID][-1][idx] = value;
            for (const auto & sub_category: Model_Category::sub_category(category))
            {
                categoryStats[category.CATEGID][sub_category.SUBCATEGID][idx] = value;
            }
        }
    }
    //Calculations
    for (const auto& pBankTransaction : transactions_)
    {
        if (pBankTransaction->status_ == "V") continue; // skip

        if (ignoreFuture)
        {
            if (pBankTransaction->date_.GetDateOnly().IsLaterThan(wxDateTime::Now().GetDateOnly()))
                continue; //skip future dated transactions
        }

        if (with_date)
        {
            if (!pBankTransaction->date_.IsBetween(date_range->start_date(), date_range->end_date()))
                continue; //skip
        }

        // We got this far, get the currency conversion rate for this account
        double convRate = acc_conv_rates[pBankTransaction->accountID_];

        wxDateTime d = pBankTransaction->date_;
        int idx = group_by_month ? (d.GetYear()*100 + (int)d.GetMonth()) : 0;
        int categID = pBankTransaction->categID_;

        if (categID > -1)
        {
            if (pBankTransaction->transType_ != TRANS_TYPE_TRANSFER_STR)
                categoryStats[categID][pBankTransaction->subcategID_][idx] += pBankTransaction->value(-1) * convRate;
        }
        else
        {
            mmSplitTransactionEntries* splits = pBankTransaction->splitEntries_;
            pBankTransaction->getSplitTransactions(splits);
            for (const auto& entry : splits->entries_)
            {
                categoryStats[entry->categID_][entry->subCategID_][idx] += entry->splitAmount_ 
                    * convRate * (pBankTransaction->value(-1)< 0 ? -1 : 1);
            }
        }
    }
}

wxDateTime mmBankTransactionList::getLastDate(int accountID) const
{
    // Set initial date a long time in past
    wxDateTime dt(wxDateTime::Now().Subtract(wxDateSpan::Years(20)));
    bool same_initial_date = true;

    for (auto const& pBankTransaction : transactions_)
    {
        if (accountID != -1)
        {
            if (pBankTransaction->accountID_ != accountID && pBankTransaction->toAccountID_ != accountID)
                continue; // skip

            if (pBankTransaction->date_.IsLaterThan(wxDateTime::Now()))
                continue; //skip future dated transactions
        }

        if (!dt.IsLaterThan(pBankTransaction->date_))
        {
            dt = pBankTransaction->date_;
            same_initial_date = false;
        }
    }

    if (same_initial_date) // then set date to present day.
    {
        dt = wxDateTime::Now();
    }

    return dt;
}

int mmBankTransactionList::getLastUsedCategoryID(int accountID
    , int payeeID, const wxString& sType, int& subcategID) const
{
    int categ_id = -1;
    subcategID = -1;

    for (const auto& pTransaction : transactions_)
    {
        if ((pTransaction->accountID_ == accountID || pTransaction->toAccountID_ == accountID)
            && pTransaction->transType_ == sType
            && pTransaction->payeeID_ == payeeID)
        {
            categ_id = pTransaction->categID_;
            if (categ_id > -1)
            {
                subcategID = pTransaction->subcategID_;
                break;
            }
        }
    }
    return categ_id;
}

int mmBankTransactionList::getLastUsedPayeeID(int accountID, const wxString& sType, int& categID, int& subcategID) const
{
    int payee_id = -1;

    for (const auto& pBankTransaction : transactions_)
    {
        if (pBankTransaction->accountID_ == accountID
            && pBankTransaction->transType_ == sType)
        {
            payee_id   = pBankTransaction->payeeID_;
            categID    = pBankTransaction->categID_;
            subcategID = pBankTransaction->subcategID_;
            break;
        }
    }

    return payee_id;
}

double mmBankTransactionList::getAmountForCategory(
    int categID,
    int subcategID,
    bool ignoreDate,
    const wxDateTime &dtBegin,
    const wxDateTime &dtEnd,
    bool evaluateTransfer,      // activates the asDeposit parameter.
    bool asDeposit,             // No effect when evaluateTransfer is false.
    bool ignoreFuture
) const
{
    double amt = 0.0;
    const wxDateTime dtNow = wxDateTime::Now().GetDateOnly();

    for (const auto & pBankTransaction: transactions_)
    {
        if (!pBankTransaction->containsCategory(categID, subcategID)) continue;
        if (pBankTransaction->status_ == "V") continue;
        if (!ignoreDate)
        {
            if (!pBankTransaction->date_.GetDateOnly().IsBetween(dtBegin, dtEnd)) continue;
        }
        if (ignoreFuture)
        {
            //skip future dated transactions
            if (pBankTransaction->date_.GetDateOnly().IsLaterThan(dtNow)) continue;
        }

        Model_Account::Data* account = Model_Account::instance().get(pBankTransaction->accountID_);
        Model_Currency::Data* currency = Model_Account::currency(account);
        double convRate = currency->BASECONVRATE;
        if (pBankTransaction->transType_ == TRANS_TYPE_TRANSFER_STR)
        {
            if (evaluateTransfer)
            {
                if (asDeposit)
                {
                    amt += pBankTransaction->getAmountForSplit(categID, subcategID) * convRate;
                }
                else
                {
                    amt -= pBankTransaction->getAmountForSplit(categID, subcategID) * convRate;
                }
            }
            continue;  //skip
        }
        if (pBankTransaction->transType_ == TRANS_TYPE_WITHDRAWAL_STR)
        {
            amt -= pBankTransaction->getAmountForSplit(categID, subcategID) * convRate;
        }
        else if (pBankTransaction->transType_ == TRANS_TYPE_DEPOSIT_STR)
        {
            amt += pBankTransaction->getAmountForSplit(categID, subcategID) * convRate;
        }
    }

    return amt;
}

/** removes the transaction from memory */
bool mmBankTransactionList::removeTransaction(int accountID, int transactionID)
{
    std::vector<mmBankTransaction* >::iterator i;
    for (i = transactions_.begin(); i!= transactions_.end(); ++i)
    {
        mmBankTransaction* pBankTransaction = *i;
        if (pBankTransaction)
        {
            if ((pBankTransaction->accountID_ == accountID) || (pBankTransaction->toAccountID_ == accountID))
            {
                if (pBankTransaction->transactionID() == transactionID)
                {
                    delete (*i);
                    i = transactions_.erase(i);
                    return true;
                }
            }
        }
    }
    // didn't find the transaction
    wxASSERT(false);
    return false;
}

/** removes the transaction from memory and the database */
bool mmBankTransactionList::deleteTransaction(int accountID, int transactionID)
{
    if (mmDBWrapper::deleteTransaction(core_->db_.get(), transactionID))
    {
        removeTransaction(accountID, transactionID);
        return true;
    }
    return false;
}

void mmBankTransactionList::deleteTransactions(int accountID)
{
    for (const auto& pBankTransaction : transactions_ )
    {
        if ((pBankTransaction->accountID_ == accountID) ||
            (pBankTransaction->toAccountID_ == accountID))
        {
            cleanuptranslist();
            transactions_.clear();
        }
    }
}

wxArrayString mmBankTransactionList::getTransactionNumber(int accountID, const wxDateTime& transaction_date) const
{
    double trx_number, today_number = 1, max_number = 1;
    wxArrayString number_strings;
    for (const auto & pBankTransaction: transactions_)
    {
        if (pBankTransaction)
        {
            if (pBankTransaction->accountID_ != accountID && pBankTransaction->toAccountID_ != accountID)
                continue; // skip
            wxRegEx pattern("([0-9.]+)");
            if (!pattern.Matches(pBankTransaction->transNum_))
                continue;

            trx_number = 1;
            if ((pBankTransaction->transNum_).ToDouble(&trx_number))
            {
                if (max_number <= trx_number) max_number = trx_number+1;
                if (pBankTransaction->date_.FormatISODate() == transaction_date.FormatISODate())
                    if (today_number <= trx_number) today_number = trx_number+1;
            }
        }
    }
    number_strings.Add(wxString()<<(int)today_number);
    number_strings.Add(wxString()<<(int)max_number);

    return number_strings;
}

void mmBankTransactionList::UpdateCategory(int catID, int subCatID, wxString &fullCatStr)
{
    for (const auto & pBankTransaction: transactions_)
    {
        if ((pBankTransaction->categID_ == catID)
            && pBankTransaction->subcategID_== subCatID)
        {
            pBankTransaction->fullCatStr_ = fullCatStr;
        }
    }
}

