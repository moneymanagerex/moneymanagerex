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

#include "model/AccountModel.h"
#include "model/AttachmentModel.h"
#include "model/ScheduledModel.h"
#include "model/CategoryModel.h"
#include "model/PayeeModel.h"
#include "model/StockModel.h"
#include "uicontrols/navigatortypes.h"

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
    const auto &transactions = TransactionModel::instance().get_all();
    for (const auto& trx : transactions)
        if (!AccountModel::instance().cache_id(trx.ACCOUNTID) || (TransactionModel::type_id(trx) == TransactionModel::TYPE_ID_TRANSFER && !AccountModel::instance().cache_id(trx.TOACCOUNTID)))
        {
            result = false;
        }

    // BillsDeposits
    const auto &bills = ScheduledModel::instance().get_all();
    for (const auto& bill : bills)
        if (!AccountModel::instance().cache_id(bill.ACCOUNTID) || (ScheduledModel::type_id(bill) == TransactionModel::TYPE_ID_TRANSFER && !AccountModel::instance().cache_id(bill.TOACCOUNTID)))
        {
            result = false;
        }

    // Stocks
    const auto &stocks = StockModel::instance().get_all();
    for (const auto& stock : stocks)
        if (!AccountModel::instance().cache_id(stock.HELDAT) || (AccountModel::type_id(AccountModel::instance().cache_id(stock.HELDAT)) != NavigatorTypes::TYPE_ID_INVESTMENT)) {
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
