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

#include "mmaccount.h"
#include "util.h"
#include "mmOption.h"
#include "constants.h"
#include "mmcoredb.h"
#include "model/Model_Account.h"

mmAccount::mmAccount(wxSQLite3ResultSet& q1)
{
    id_ = q1.GetInt("ACCOUNTID");
    name_ = q1.GetString("ACCOUNTNAME");
    accountNum_  = q1.GetString("ACCOUNTNUM");
    heldAt_ = q1.GetString("HELDAT");
    website_ = q1.GetString("WEBSITE");
    contactInfo_ = q1.GetString("CONTACTINFO");
    accessInfo_ = q1.GetString("ACCESSINFO");
    notes_ = q1.GetString("NOTES");
    acctType_ = q1.GetString("ACCOUNTTYPE");

    status_ =  mmAccount::MMEX_Open;
    if (q1.GetString("STATUS") == "Closed")
        status_ = mmAccount::MMEX_Closed;

    wxString retVal = q1.GetString("FAVORITEACCT");
    if (retVal == "TRUE")
        favoriteAcct_ = true;
    else
        favoriteAcct_ = false;

    initialBalance_ = q1.GetDouble("INITIALBAL");
    currencyID_ = q1.GetDouble("CURRENCYID");
}

mmAccountList::mmAccountList(mmCoreDB* core)
: core_(core)
{}

double mmAccountList::getAccountBaseCurrencyConvRate(int accountID) const
{
    if (accountID > 0)
    {
        mmCurrency* pCurrency = getCurrencySharedPtr(accountID);

        wxASSERT(pCurrency);

        if (pCurrency)
        return pCurrency->baseConv_;
    }

    return 1.0;
}

mmAccount* mmAccountList::GetAccountSharedPtr(int accountID) const
{
    for (const auto& r : accounts_)
    {
        if (r->id_ == accountID)
        {
            return r;
        }
    }
    wxASSERT(false);
    account_v::value_type res = 0;
    return res;
}

bool mmAccountList::AccountExists(const wxString& accountName) const
{
    for (const auto& it : accounts_)
    {
        if (!accountName.CmpNoCase(it->name_)) return true;
    }

    return false;
}

bool mmAccountList::has_term_account() const
{
    for (const auto& it : accounts_)
    {
        if (it->acctType_ == ACCOUNT_TYPE_TERM) return true;
    }

    return false;
}

bool mmAccountList::has_stock_account() const
{
    for (const auto& it : accounts_)
    {
        if (it->acctType_ == ACCOUNT_TYPE_STOCK) return true;
    }

    return false;
}

int mmAccountList::getNumBankAccounts() const
{
    int num = 0;
    for (const auto& it : accounts_)
    {
        if (it->acctType_ != ACCOUNT_TYPE_STOCK) ++ num;
    }

    return num;
}

int mmAccountList::GetAccountId(const wxString& accountName) const
{
    for (const auto& it : accounts_)
    {
        if (it->name_ == accountName) return it->id_;
    }

   return -1;
}

wxString mmAccountList::GetAccountName(int accountID) const
{
    for (const auto& it : accounts_)
    {
        if (it->id_ == accountID) return it->name_;
    }

    return wxEmptyString;
}

wxString mmAccountList::getAccountType(int accountID) const
{
    for (const auto& account : accounts_)
    {
        if (account->id_ == accountID) return account->acctType_;
    }

    wxASSERT(false);
    return "";
}

bool mmAccountList::getAccountFavorite(int accountID) const
{
    for (const auto& account : accounts_)
    {
        if (account->id_ == accountID) return account->favoriteAcct_;
    }

    wxASSERT(false);
    return false;
}

mmAccount::Status mmAccountList::getAccountStatus(int accountID) const
{
    for (const auto& account : accounts_)
    {
        if (account->id_ == accountID) return account->status_;
    }

    wxASSERT(false);
    return mmAccount::MMEX_Open;
}

bool mmAccountList::currencyInUse(int currencyID) const
{
    for (const auto& account : accounts_)
    {
        if (account->currencyID_ == currencyID) return true;
    }

    return false;
}

wxString mmAccountList::getAccountCurrencyDecimalChar(int accountID) const
{
    mmCurrency* pCurrency = getCurrencySharedPtr(accountID); 

    if (pCurrency)
        return pCurrency->decChar_;

    return ".";
}

wxString mmAccountList::getAccountCurrencyGroupChar(int accountID) const
{
    mmCurrency* pCurrency = getCurrencySharedPtr(accountID);
    wxASSERT(pCurrency);

    if (pCurrency)
        return pCurrency->grpChar_;

    return ",";
}

wxString mmAccountList::GetAccountCurrencyName(int accountID) const
{
    mmCurrency* pCurrency = getCurrencySharedPtr(accountID);
    wxASSERT(pCurrency);

    if (pCurrency)
        return pCurrency->currencyName_;

    return "";
}

mmCurrency* mmAccountList::getCurrencySharedPtr(int accountID) const
{
    for (const auto& idx : accounts_)
    {
        if (idx->id_ == accountID)
            return idx->currency_;
    }
    wxASSERT(false);
    return NULL;
}

int mmAccountList::UpdateAccount(mmAccount* pAccount)
{
    wxString statusStr = pAccount->status_ == mmAccount::MMEX_Closed ? "Closed" : "Open";
    wxString favStr = pAccount->favoriteAcct_ ? "TRUE" : "FALSE";

    mmCurrency* pCurrency = pAccount->currency_;
    wxASSERT(pCurrency);
    int currencyID = pCurrency->currencyID_;

    wxSQLite3Statement st = core_->db_.get()->PrepareStatement(UPDATE_ACCOUNTLIST_V1);
    const mmAccount &r = *pAccount;

    std::vector<wxString> data;
    data.push_back(r.name_);
    data.push_back(r.acctType_);
    data.push_back(r.accountNum_);
    data.push_back(statusStr);
    data.push_back(r.notes_);
    data.push_back(r.heldAt_);
    data.push_back(r.website_);
    data.push_back(r.contactInfo_);
    data.push_back(r.accessInfo_);
    data.push_back(wxString()<<r.initialBalance_);
    data.push_back(favStr);
    data.push_back(wxString()<<currencyID);
    data.push_back(wxString()<<r.id_);

    long accountId = -1;
    wxString sql = wxString::FromUTF8(UPDATE_ACCOUNTLIST_V1);
    int iError = mmDBWrapper::mmSQLiteExecuteUpdate(core_->db_.get(), data, sql, accountId);
    if (iError == 0)
        mmOptions::instance().databaseUpdated_ = true;

    return iError;
}

int mmAccountList::AddAccount(mmAccount* pAccount)
{
    wxString statusStr = "Open";
    if (pAccount->status_ == mmAccount::MMEX_Closed)
        statusStr = "Closed";

    wxString favStr = "TRUE";
    if (!pAccount->favoriteAcct_)
        favStr = "FALSE";

    mmCurrency* pCurrency = pAccount->currency_;
    wxASSERT(pCurrency);
    int currencyID = pCurrency->currencyID_;

    wxSQLite3Statement st = core_->db_.get()->PrepareStatement(INSERT_INTO_ACCOUNTLIST_V1);
    const mmAccount &r = *pAccount;

    int i = 0;
    st.Bind(++i, r.name_);
    st.Bind(++i, r.acctType_);
    st.Bind(++i, r.accountNum_);
    st.Bind(++i, statusStr);
    st.Bind(++i, r.notes_);
    st.Bind(++i, r.heldAt_);
    st.Bind(++i, r.website_);
    st.Bind(++i, r.contactInfo_);
    st.Bind(++i, r.accessInfo_);
    st.Bind(++i, r.initialBalance_);
    st.Bind(++i, favStr);
    st.Bind(++i, currencyID);

    wxASSERT(st.GetParamCount() == i);
    st.ExecuteUpdate();

    pAccount->id_ = core_->db_.get()->GetLastRowId().ToLong();
    accounts_.push_back(pAccount);

    st.Finalize();
    mmOptions::instance().databaseUpdated_ = true;

    return pAccount->id_;
}

bool mmAccountList::RemoveAccount(int accountID)
{
    wxString acctType = getAccountType(accountID);
    core_->db_.get()->Begin();
    wxSQLite3Statement st;

    if (acctType != ACCOUNT_TYPE_STOCK)
    {
        mmDBWrapper::removeSplitsForAccount(core_->db_.get(), accountID);

        st = core_->db_.get()->PrepareStatement("DELETE FROM CHECKINGACCOUNT_V1 WHERE ACCOUNTID = ? OR TOACCOUNTID = ?");
        st.Bind(1, accountID);
        st.Bind(2, accountID);
        st.ExecuteUpdate();
        st.Finalize();

        // --
        st = core_->db_.get()->PrepareStatement("DELETE FROM BILLSDEPOSITS_V1 WHERE ACCOUNTID=? OR TOACCOUNTID = ?");
        st.Bind(1, accountID);
        st.Bind(2, accountID);
        st.ExecuteUpdate();
        st.Finalize();
    }
    else
    {
        st = core_->db_.get()->PrepareStatement("DELETE FROM STOCK_V1 WHERE HELDAT = ?");
        st.Bind(1, accountID);
        st.ExecuteUpdate();
        st.Finalize();
    }

    st = core_->db_.get()->PrepareStatement("DELETE FROM INFOTABLE_V1 WHERE INFONAME = ?");
    st.Bind(1, wxString::Format("ACC_IMAGE_ID_%i", accountID));
    st.ExecuteUpdate();
    st.Finalize();

    st = core_->db_.get()->PrepareStatement("DELETE FROM ACCOUNTLIST_V1 WHERE ACCOUNTID = ?");
    st.Bind(1, accountID);

    st.ExecuteUpdate();
    st.Finalize();

    core_->db_.get()->Commit();

    mmOptions::instance().databaseUpdated_ = true;

    std::vector<mmAccount* >::iterator iter;
    for (iter = accounts_.begin(); iter != accounts_.end(); )
    {
        mmAccount* pAccount = (*iter);
        if (pAccount->id_ == accountID)
        {
            iter = accounts_.erase(iter);
            break;
        }
        else
            ++iter;
    }

    return true;
}

void mmAccountList::LoadAccounts(const mmCurrencyList& currencyList)
{
    wxSQLite3ResultSet q1 = core_->db_.get()->ExecuteQuery(SELECT_ALL_FROM_ACCOUNTLIST_V1);

    while (q1.NextRow())
    {
        mmAccount* pAccount(new mmAccount(q1));

        mmCurrency* pCurrency =
            currencyList.getCurrencySharedPtr(q1.GetInt("CURRENCYID"));
        pAccount->currency_ = pCurrency;

        accounts_.push_back(pAccount);
    }

    q1.Finalize();
}

void mmAccountList::getAccountRates(std::map<int, double> &acc_conv_rates)
{
    Model_Account::Data_Set accounts = Model_Account::instance().all();
    for (const auto& account: accounts)
    {
        mmCurrency* pCurrencyPtr = core_->accountList_.getCurrencySharedPtr(account.ACCOUNTID);
        wxASSERT(pCurrencyPtr);
        double rate = pCurrencyPtr->baseConv_;
        acc_conv_rates[account.ACCOUNTID] = rate;
    }
}
//----------------------------------------------------------------------------
