/*******************************************************
 Copyright (C) 2025 George Ef (george.a.ef@gmail.com)

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

#include "model/Model.h"
#include "model/allmodel.h"

ChoicesName ModelBase::REFTYPE_CHOICES = ChoicesName({
    { REFTYPE_ID_TRANSACTION,       _n("Transaction") },
    { REFTYPE_ID_STOCK,             _n("Stock") },
    { REFTYPE_ID_ASSET,             _n("Asset") },
    { REFTYPE_ID_BANKACCOUNT,       _n("BankAccount") },
    { REFTYPE_ID_BILLSDEPOSIT,      _n("RecurringTransaction") },
    { REFTYPE_ID_PAYEE,             _n("Payee") },
    { REFTYPE_ID_TRANSACTIONSPLIT,  _n("TransactionSplit") },
    { REFTYPE_ID_BILLSDEPOSITSPLIT, _n("RecurringTransactionSplit") },
});

const wxString ModelBase::REFTYPE_NAME_TRANSACTION       = reftype_name(REFTYPE_ID_TRANSACTION);
const wxString ModelBase::REFTYPE_NAME_STOCK             = reftype_name(REFTYPE_ID_STOCK);
const wxString ModelBase::REFTYPE_NAME_ASSET             = reftype_name(REFTYPE_ID_ASSET);
const wxString ModelBase::REFTYPE_NAME_BANKACCOUNT       = reftype_name(REFTYPE_ID_BANKACCOUNT);
const wxString ModelBase::REFTYPE_NAME_BILLSDEPOSIT      = reftype_name(REFTYPE_ID_BILLSDEPOSIT);
const wxString ModelBase::REFTYPE_NAME_PAYEE             = reftype_name(REFTYPE_ID_PAYEE);
const wxString ModelBase::REFTYPE_NAME_TRANSACTIONSPLIT  = reftype_name(REFTYPE_ID_TRANSACTIONSPLIT);
const wxString ModelBase::REFTYPE_NAME_BILLSDEPOSITSPLIT = reftype_name(REFTYPE_ID_BILLSDEPOSITSPLIT);

// Model_*::refTypeName are initialized here because they depend on REFTYPE_NAME_,
// which depend on REFTYPE_CHOICES.
const wxString Model_Account::refTypeName                = REFTYPE_NAME_BANKACCOUNT;
const wxString Model_Asset::refTypeName                  = REFTYPE_NAME_ASSET;
const wxString Model_Stock::refTypeName                  = REFTYPE_NAME_STOCK;
const wxString Model_Payee::refTypeName                  = REFTYPE_NAME_PAYEE;
const wxString Model_Checking::refTypeName               = REFTYPE_NAME_TRANSACTION;
const wxString Model_Splittransaction::refTypeName       = REFTYPE_NAME_TRANSACTIONSPLIT;
const wxString Model_Billsdeposits::refTypeName          = REFTYPE_NAME_BILLSDEPOSIT;
const wxString Model_Budgetsplittransaction::refTypeName = REFTYPE_NAME_BILLSDEPOSITSPLIT;

