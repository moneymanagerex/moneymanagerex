/*******************************************************
Copyright (C) 2015 Gabriele-V

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
********************************************************/


#include "dbcheck.h"

#include "Model_Account.h"
#include "Model_Checking.h"
#include "Model_Billsdeposits.h"
#include "Model_Stock.h"

bool dbCheck::checkDB()
{
    bool result = true;
    result = result && checkAccounts();
    result = result && checkAttachments();
    result = result && checkBudgets();
    result = result && checkBudgetYears();
    result = result && checkCategories();
    result = result && checkCurrencies();
    result = result && checkPayees();
    result = result && checkStocks();
    result = result && checkSubcategories();
    result = result && checkTransactions();
    return result;
}

bool dbCheck::checkAccounts()
{
    // Transactions
    const auto &transactions = Model_Checking::instance().all();
    for (const auto& trx : transactions)
        if (!Model_Account::instance().get(trx.ACCOUNTID) || (Model_Checking::type(trx) == Model_Checking::TRANSFER && !Model_Account::instance().get(trx.TOACCOUNTID)))
        {
            return false;
        }

    // BillsDeposits
    const auto &bills = Model_Billsdeposits::instance().all();
    for (const auto& bill : bills)
        if (!Model_Account::instance().get(bill.ACCOUNTID) || (Model_Billsdeposits::type(bill) == Model_Billsdeposits::TRANSFER && !Model_Account::instance().get(bill.TOACCOUNTID)))
        {
            return false;
        }

    // Stocks
    const auto &stocks = Model_Stock::instance().all();
    for (const auto& stock : stocks)
        if (!Model_Account::instance().get(stock.HELDAT) || (Model_Account::type(Model_Account::instance().get(stock.HELDAT)) != Model_Account::INVESTMENT))
        {
            return false;
        }

    return true;
}

bool dbCheck::checkAttachments()
{
    return true; // FIXME
}

bool dbCheck::checkBudgets()
{
    return true; // FIXME
}

bool dbCheck::checkBudgetYears()
{
    return true; // FIXME
}

bool dbCheck::checkCategories()
{
    return true; // FIXME
}

bool dbCheck::checkCurrencies()
{
    return true; // FIXME
}

bool dbCheck::checkPayees()
{
    return true; // FIXME
}

bool dbCheck::checkStocks()
{
    return true; // FIXME
}

bool dbCheck::checkSubcategories()
{
    return true; // FIXME
}

bool dbCheck::checkTransactions()
{
    return true; // FIXME
}
