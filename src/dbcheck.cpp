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

#include "model/Model_Account.h"
#include "model/Model_Attachment.h"
#include "model/Model_Billsdeposits.h"
#include "model/Model_Category.h"
#include "model/Model_Payee.h"
#include "model/Model_Stock.h"

bool dbCheck::checkDB()
{
    bool result = true;
    result = checkAccounts();
    
    return result;
}

bool dbCheck::checkAccounts()
{
    bool result = true;

    // Transactions
    const auto &transactions = Model_Checking::instance().all();
    for (const auto& trx : transactions)
        if (!Model_Account::instance().get(trx.ACCOUNTID) || (Model_Checking::type_id(trx) == Model_Checking::TYPE_ID_TRANSFER && !Model_Account::instance().get(trx.TOACCOUNTID)))
        {
            result = false;
        }

    // BillsDeposits
    const auto &bills = Model_Billsdeposits::instance().all();
    for (const auto& bill : bills)
        if (!Model_Account::instance().get(bill.ACCOUNTID) || (Model_Billsdeposits::type_id(bill) == Model_Checking::TYPE_ID_TRANSFER && !Model_Account::instance().get(bill.TOACCOUNTID)))
        {
            result = false;
        }

    // Stocks
    const auto &stocks = Model_Stock::instance().all();
    for (const auto& stock : stocks)
        if (!Model_Account::instance().get(stock.HELDAT) || (Model_Account::type_id(Model_Account::instance().get(stock.HELDAT)) != Model_Account::TYPE_ID_INVESTMENT))
        {
            result = false;
        }

    return result;
}

bool dbCheck::checkAttachments()
{
    return true;
}

bool dbCheck::checkBudgets()
{
    return true;
}

bool dbCheck::checkBudgetYears()
{
    return true;
}

bool dbCheck::checkCategories()
{
    return true;
}

bool dbCheck::checkCurrencies()
{
    return true;
}

bool dbCheck::checkPayees()
{
    return true;
}

bool dbCheck::checkStocks()
{
    return true;
}

bool dbCheck::checkSubcategories()
{
    return true;
}

bool dbCheck::checkTransactions()
{
    return true;
}
