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

#ifndef _MM_EX_MMACCOUNT_H_
#define _MM_EX_MMACCOUNT_H_

#include "mmcurrency.h"
#include "mmtransaction.h"

class mmCoreDB;

class mmAccount
{
public:
    mmAccount()
    {}
    mmAccount(wxSQLite3ResultSet& q1);

    /* Scoped Enums */
    enum Status
    {
        MMEX_Open,
        MMEX_Closed
    };

public:
    int      id_;
    wxString name_;
    Status   status_;
    wxString accountNum_;
    wxString notes_;
    wxString heldAt_;
    wxString website_;
    wxString contactInfo_;
    wxString accessInfo_;
    wxString acctType_;
    bool favoriteAcct_;
    double initialBalance_;
    int currencyID_;
    mmCurrency* currency_;
};

class mmAccountList
{
private:
    typedef std::vector< mmAccount* > account_v;
    mmCoreDB* core_;

public:
    mmAccountList(mmCoreDB* core);
    ~mmAccountList();

    /* Account Functions */
    mmAccount* GetAccountSharedPtr(int accountID) const;

    int AddAccount(mmAccount* pAccount);
    /// returns the ACCOUNT_TYPE_xxxx String
    wxString getAccountType(int accountID) const;
    bool getAccountFavorite(int accountID) const;
    /// returns the status of the account (mmAccount::MMEX_Open or mmAccount::MMEX_Closed)\n
    mmAccount::Status getAccountStatus(int accountID) const;
    int getNumAccounts() const
    {
        return accounts_.size();
    }
    int getNumBankAccounts() const;
    int GetAccountId(const wxString& accountName) const;
    bool RemoveAccount(int accountID);
    int UpdateAccount(mmAccount* pAccount);
    bool AccountExists(const wxString& accountName) const;
    bool has_term_account() const;
    bool has_stock_account() const;
    mmCurrency* getCurrencySharedPtr(int accountID) const;
    double getAccountBaseCurrencyConvRate(int accountID) const;
    wxString GetAccountName(int accountID) const;
    wxString getAccountCurrencyDecimalChar(int accountID) const;
    wxString getAccountCurrencyGroupChar(int accountID) const;
    wxString GetAccountCurrencyName(int accountID) const;
    bool currencyInUse(int currencyID) const;
    account_v accounts_;
    typedef std::vector<mmAccount* >::const_iterator const_iterator;

    /// Loads database Accounts list into memory
    void LoadAccounts(const mmCurrencyList& currencyList);
    void getAccountRates(std::map<int, double> &acc_conv_rates);
};

#endif
