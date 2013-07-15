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

#ifndef _MM_EX_MMTRANSACTION_H_
#define _MM_EX_MMTRANSACTION_H_

#include "dbwrapper.h"
#include "mmcategory.h"
#include "mmpayee.h"
#include "mmcurrency.h"
#include "reports/mmDateRange.h"
#include <map>

class mmCoreDB;

class mmTransaction
{
public:
    mmTransaction() {}
    mmTransaction(int transactionID) : transactionID_(transactionID) {}
    virtual ~mmTransaction() {}

    const virtual int transactionID() const
    {
        return transactionID_;
    }
    void transactionID(int transactionID)
    {
        transactionID_ = transactionID;
    }

    virtual double value(int accountCtx) const = 0;

protected:
    int transactionID_;
};

class mmSplitTransactionEntry
{
public:
    int splitEntryID_;
    int categID_;
    int subCategID_;
    wxString sFullCategoryName_;

    double splitAmount_;
};

class mmSplitTransactionEntries
{
public:
    mmSplitTransactionEntries() {}

    size_t numEntries() const
    {
        return entries_.size();
    }
    double getTotalSplits() const;
    double getUpdatedTotalSplits() const;

    void addSplit(mmSplitTransactionEntry* split);
    void removeSplit(int splitID);
    void removeSplitByIndex(int splitIndex);

    void updateToDB(std::shared_ptr<wxSQLite3Database>& db, int transID, bool edit);
    void loadFromBDDB(mmCoreDB* core, int bdID);

    std::vector<mmSplitTransactionEntry* > entries_;
};

class mmBankTransaction : public mmTransaction
{
public:
    mmBankTransaction(mmCoreDB* core, wxSQLite3ResultSet& q1);
    mmBankTransaction(mmCoreDB* core);
    virtual ~mmBankTransaction() {}
    bool operator < (const mmBankTransaction& tran) const;
    bool containsCategory(int categID, int subcategID, bool ignoreSubCateg = false) const;
    double getAmountForSplit(int categID, int subcategID) const;

    double value(int accountID) const;
    void updateTransactionData(int accountID, double& balance);

    void getSplitTransactions(mmSplitTransactionEntries* splits) const;
    mmCoreDB* core_;

    /* Core Data */
    wxDateTime date_;
    wxString transNum_;
    wxString transType_;
    wxString status_;
    int payeeID_;
    wxString payeeStr_;
    wxString arrow_;
    int categID_;
    int subcategID_;
    wxString fullCatStr_;
    double amt_;
    double toAmt_;

    int accountID_;
    wxString fromAccountStr_;
    int toAccountID_;
    int followupID_;
    wxString notes_;

    mmSplitTransactionEntries* splitEntries_;

    /* Derived Data */
    double withdrawal_amt_;
    double deposit_amt_;
    double balance_;

    //bool updateRequired_;

    // used for transaction reports
    double reportCategAmount_;
    wxString reportCategAmountStr_;
};

class mmBankTransactionList
{
public:
    mmBankTransactionList(mmCoreDB* core);
    ~mmBankTransactionList() {}

    mmBankTransaction* getBankTransactionPtr(int transactionID) const;
    int addTransaction(mmBankTransaction* pTransaction);
    bool checkForExistingTransaction(mmBankTransaction* pTransaction);
    mmBankTransaction* copyTransaction(/*mmCoreDB* pCore,*/
        const long transactionID, const long accountID, const bool useOriginalDate);

    /// Loads database primary Transactions into memory.
    void LoadTransactions();
    void LoadAccountTransactions(int accountID, double& account_balance, double& reconciled_balance);

    /* Update Transactions */
    void UpdateTransaction(mmBankTransaction* pTransaction);

    bool removeTransaction(int accountID, int transactionID);
    bool deleteTransaction(int accountID, int transactionID);
    void deleteTransactions(int accountID);
    int RelocatePayee(mmCoreDB* core, int destPayeeID, int sourcePayeeID, int& changedPayees_);
    int RelocateCategory(mmCoreDB* core,
                         int destCatID, int destSubCatID, int sourceCatID, int sourceSubCatID,
                         int& changedCats, int& changedSubCats);

    /* Query Functions */
    void getTransactionStats(std::map<wxDateTime::Month, std::map<int, int> > &stats, int start_year) const;

    void getExpensesIncomeStats(std::map<int, std::pair<double, double> > &incomeExpensesStats
                                , mmDateRange* date_range
                                , int accountID
                                , bool group_by_account = false
                                        , bool group_by_month = false) const;

    void getTopCategoryStats(
        std::vector<std::pair<wxString, double> > &categoryStats
        , mmDateRange* date_range) const;

    void getCategoryStats(
        std::map<int, std::map<int, std::map<int, double> > > &categoryStats
        , mmDateRange* date_range, bool ignoreFuture
        , bool group_by_month = true, bool with_date = true) const;

    // The setting asDeposit is only valid if evaluateTransfer is true
    double getAmountForCategory(int categID, int subcategID, bool ignoreDate, const wxDateTime &dtBegin, const wxDateTime &dtEnd, bool evaluateTransfer = false, bool asDeposit = false, bool ignoreFuture = false) const;
    void getPayeeStats(std::map<int, std::pair<double, double> > &payeeStats
        , mmDateRange* date_range, bool ignoreFuture) const;
    wxDateTime getLastDate(int accountID) const;

    double getBalance(int accountID, bool ignoreFuture = false) const;
    double getReconciledBalance(int accountID, bool ignoreFuture = false) const;
    int countFollowupTransactions() const;
    int getLastUsedCategoryID(int accountID, int payeeID, const wxString& sType, int& subcategID) const;
    int getLastUsedPayeeID(int accountID, const wxString& sType, int& categID, int& subcategID) const;
    wxArrayString getTransactionNumber(int accountID, const wxDateTime& transaction_date) const;
    bool IsCategoryUsed(int iCatID, int iSubCatID, bool& bIncome, bool bIgnor_subcat = true) const;
    bool IsPayeeUsed(int iPayeeID) const;
    bool getDailyBalance(const mmCoreDB* core, int accountID, std::map<wxDateTime, double>& daily_balance, bool ignoreFuture = false) const;


    /* Data */
    //typedef std::vector< std::shared_ptr<mmBankTransaction> >::const_iterator const_iterator;
    std::vector<mmBankTransaction*> transactions_;
    std::vector<mmBankTransaction*> accountTransactions_;

private:
    mmCoreDB* core_;
};

#endif
