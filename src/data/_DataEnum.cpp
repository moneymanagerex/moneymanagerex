/*******************************************************
 Copyright (C) 2026 George Ef (george.a.ef@gmail.com)

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

// The enumeration strings listed below are written in the database.
// DO NOT CHANGE THEM!

#include "_DataEnum.h"

mmChoiceNameA AccountStatus::s_choice_a = mmChoiceNameA({
    { e_open,   _n("Open") },
    { e_closed, _n("Closed") },
}, e_open, true);

mmChoiceNameA AccountFavorite::s_choice_a = mmChoiceNameA({
    { e_false, _n("FALSE") },
    { e_true,  _n("TRUE") },
}, e_false, true);

mmChoiceNameA AssetType::s_choice_a = mmChoiceNameA({
    { e_property,   _n("Property") },
    { e_automobile, _n("Automobile") },
    { e_household,  _n("Household Object") },
    { e_art,        _n("Art") },
    { e_jewellery,  _n("Jewellery") },
    { e_cash,       _n("Cash") },
    { e_other,      _n("Other") },
}, e_property, true );

mmChoiceNameA AssetStatus::s_choice_a = mmChoiceNameA({
    { e_closed, _n("Closed") },
    { e_open,   _n("Open") },
}, e_open, true);

mmChoiceNameA AssetChange::s_choice_a = mmChoiceNameA({
    { e_none,        _n("None") },
    { e_appreciates, _n("Appreciates") },
    { e_depreciates, _n("Depreciates") },
}, e_none, true);

mmChoiceNameA AssetChangeMode::s_choice_a = mmChoiceNameA({
    { e_percentage, _n("Percentage") },
    { e_linear,     _n("Linear") },
}, e_percentage, true);

mmChoiceNameA BudgetFrequency::s_choice_a = mmChoiceNameA({
    { e_none,       _n("None") },
    { e_weekly,     _n("Weekly") },
    { e_biweekly,   _n("Fortnightly") },
    { e_monthly,    _n("Monthly") },
    { e_bimonthly,  _n("Every 2 Months") },
    { e_quarterly,  _n("Quarterly") },
    { e_halfyearly, _n("Half-Yearly") },
    { e_yearly,     _n("Yearly") },
    { e_daily,      _n("Daily") },
}, e_none, true);

mmChoiceNameA CurrencyType::s_choice_a = mmChoiceNameA({
    { e_fiat,   _n("Fiat") },
    { e_crypto, _n("Crypto") },
}, e_fiat, true);

mmChoiceNameA FieldTypeN::s_choice_a = mmChoiceNameA({
    { e_string,        _n("String") },
    { e_integer,       _n("Integer") },
    { e_decimal,       _n("Decimal") },
    { e_boolean,       _n("Boolean") },
    { e_date,          _n("Date") },
    { e_time,          _n("Time") },
    { e_single_choice, _n("SingleChoice") },
    { e_multi_choice,  _n("MultiChoice") },
}, -1, true);

mmChoiceNameA TrxType::s_choice_a = mmChoiceNameA({
    { e_withdrawal, _n("Withdrawal") },
    { e_deposit,    _n("Deposit") },
    { e_transfer,   _n("Transfer") }
}, e_withdrawal, true);

mmChoiceKeyNameA TrxStatus::s_choice_a = mmChoiceKeyNameA({
    { e_unreconciled, "",  _n("Unreconciled") },
    { e_reconciled,   "R", _n("Reconciled") },
    { e_void,         "V", _n("Void") },
    { e_followup,     "F", _n("Follow Up") },
    { e_duplicate,    "D", _n("Duplicate") }
}, e_unreconciled, true);

mmChoiceNameA RefTypeN::s_choice_a = mmChoiceNameA({
    { e_trx,         _n("Transaction") },
    { e_stock,       _n("Stock") },
    { e_asset,       _n("Asset") },
    { e_account,     _n("BankAccount") },
    { e_sched,       _n("RecurringTransaction") },
    { e_payee,       _n("Payee") },
    { e_trx_split,   _n("TransactionSplit") },
    { e_sched_split, _n("RecurringTransactionSplit") },
}, -1, true);

mmChoiceCodeNameA UpdateType::s_choice_a = mmChoiceCodeNameA({
    { e_none,   0, _n("None") },
    { e_online, 1, _n("Online") },
    { e_manual, 2, _n("Manual") },
}, e_none, true);

