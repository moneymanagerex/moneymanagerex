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

mmChoiceKeyNameA AccountStatus::s_choice_a = mmChoiceKeyNameA({
    { e_open,   "Open",   _n("Open") },
    { e_closed, "Closed", _n("Closed") },
}, e_open, true);

mmChoiceKeyA AccountFavorite::s_choice_a = mmChoiceKeyA({
    { e_false, "FALSE" },
    { e_true,  "TRUE" },
}, e_false, true);

mmChoiceKeyNameA AssetType::s_choice_a = mmChoiceKeyNameA({
    { e_property,   "Property",         _n("Property") },
    { e_automobile, "Automobile",       _n("Automobile") },
    { e_household,  "Household Object", _n("Household Object") },
    { e_art,        "Art",              _n("Art") },
    { e_jewellery,  "Jewellery",        _n("Jewellery") },
    { e_cash,       "Cash",             _n("Cash") },
    { e_other,      "Other",            _n("Other") },
}, e_property, true );

mmChoiceKeyNameA AssetStatus::s_choice_a = mmChoiceKeyNameA({
    { e_open,   "Open",   _n("Open") },
    { e_closed, "Closed", _n("Closed") },
}, e_open, true);

mmChoiceKeyNameA AssetChange::s_choice_a = mmChoiceKeyNameA({
    { e_none,        "None",        _n("None") },
    { e_appreciates, "Appreciates", _n("Appreciates") },
    { e_depreciates, "Depreciates", _n("Depreciates") },
}, e_none, true);

mmChoiceKeyNameA AssetChangeMode::s_choice_a = mmChoiceKeyNameA({
    { e_percentage, "Percentage", _n("Percentage") },
    { e_linear,     "Linear",     _n("Linear") },
}, e_percentage, true);

mmChoiceKeyNameA BudgetFreq::s_choice_a = mmChoiceKeyNameA({
    { e_none,       "None",           _n("None") },
    { e_weekly,     "Weekly",         _n("Weekly") },
    { e_biweekly,   "Fortnightly",    _n("Fortnightly") },
    { e_monthly,    "Monthly",        _n("Monthly") },
    { e_bimonthly,  "Every 2 Months", _n("Every 2 Months") },
    { e_quarterly,  "Quarterly",      _n("Quarterly") },
    { e_halfyearly, "Half-Yearly",    _n("Half-Yearly") },
    { e_yearly,     "Yearly",         _n("Yearly") },
    { e_daily,      "Daily",          _n("Daily") },
}, e_none, true);

mmChoiceKeyNameA BudgetAmountType::s_choice_a = mmChoiceKeyNameA({
    { e_fixed,     "Fixed",     _n("Fixed") },
    { e_estimated, "Estimated", _n("Estimated") },
    { e_auto,      "Auto",      _n("Automatic") },
}, e_fixed, true);

mmChoiceKeyNameA BudgetRollover::s_choice_a = mmChoiceKeyNameA({
    { e_none,    "None",    _n("Resets each period") },
    { e_surplus, "Surplus", _n("Unspent builds a fund") },
}, e_none, true);

mmChoiceKeyNameA PlanAssumptionKind::s_choice_a = mmChoiceKeyNameA({
    { e_share_price,   "SharePrice",   _n("Share price") },
    { e_tax_rate,      "TaxRate",      _n("Tax rate") },
    { e_inflation,     "Inflation",    _n("Inflation") },
    { e_exchange_rate, "ExchangeRate", _n("Exchange rate") },
    { e_generic,       "Generic",      _n("Generic") },
}, e_generic, true);

mmChoiceKeyNameA PlanItemKind::s_choice_a = mmChoiceKeyNameA({
    { e_expense, "Expense", _n("Expense") },
    { e_income,  "Income",  _n("Income") },
}, e_expense, true);

mmChoiceKeyNameA PlanStatus::s_choice_a = mmChoiceKeyNameA({
    { e_planned,   "Planned",   _n("Planned") },
    { e_committed, "Committed", _n("Committed") },
    { e_wishlist,  "Wishlist",  _n("Wishlist") },
    { e_done,      "Done",      _n("Done") },
    { e_cancelled, "Cancelled", _n("Cancelled") },
}, e_planned, true);

mmChoiceKeyNameA CurrencyType::s_choice_a = mmChoiceKeyNameA({
    { e_fiat,   "Fiat",   _n("Fiat") },
    { e_crypto, "Crypto", _n("Crypto") },
}, e_fiat, true);

mmChoiceKeyNameA FieldTypeN::s_choice_a = mmChoiceKeyNameA({
    { e_string,        "String",       _n("String") },
    { e_integer,       "Integer",      _n("Integer") },
    { e_decimal,       "Decimal",      _n("Decimal") },
    { e_boolean,       "Boolean",      _n("Boolean") },
    { e_date,          "Date",         _n("Date") },
    { e_time,          "Time",         _n("Time") },
    { e_single_choice, "SingleChoice", _n("Single Choice") },
    { e_multi_choice,  "MultiChoice",  _n("Multi Choice") },
}, -1, true);

mmChoiceKeyNameA TrxType::s_choice_a = mmChoiceKeyNameA({
    { e_withdrawal, "Withdrawal", _n("Withdrawal") },
    { e_deposit,    "Deposit",    _n("Deposit") },
    { e_transfer,   "Transfer",   _n("Transfer") },
}, e_withdrawal, true);

mmChoiceKeyKeyNameA TrxStatus::s_choice_a = mmChoiceKeyKeyNameA({
    { e_unreconciled, "",  "Unreconciled", _n("Unreconciled") },
    { e_reconciled,   "R", "Reconciled",   _n("Reconciled") },
    { e_void,         "V", "Void",         _n("Void") },
    { e_followup,     "F", "Follow Up",    _n("Follow Up") },
    { e_duplicate,    "D", "Duplicate",    _n("Duplicate") },
}, e_unreconciled, true);

mmChoiceCodeNameA RepeatMode::s_choice_a = mmChoiceCodeNameA({
    { e_none,      0, _n("Manual") },
    { e_suggested, 1, _n("Suggested") },
    { e_automated, 2, _n("Automated") },
}, e_none, true);

mmChoiceCodeNameA RepeatFreq::s_choice_a = mmChoiceCodeNameA({
    { e_once,                     0, _n("Once") },
    { e_1_day,                   10, _n("Daily") },
    { e_1_week,                   1, _n("Weekly") },
    { e_2_weeks,                  2, _n("Fortnightly") },
    { e_4_weeks,                  9, _n("Four Weeks") },
    { e_1_month,                  3, _n("Monthly") },
    { e_month_last_day,          15, _n("Monthly (last day)") },
    { e_month_last_business_day, 16, _n("Monthly (last business day)") },
    { e_2_months,                 4, _n("Every 2 Months") },
    { e_3_months,                 5, _n("Quarterly") },
    { e_4_months,                 8, _n("Four Months") },
    { e_6_months,                 6, _n("Half-Yearly") },
    { e_1_year,                   7, _n("Yearly") },
    { e_in_x_days,               11, _n("In %s Days") },
    { e_in_x_months,             12, _n("In %s Months") },
    { e_every_x_days,            13, _n("Every %s Days") },
    { e_every_x_months,          14, _n("Every %s Months") },
}, e_once, true);

mmChoiceKeyNameA RefTypeN::s_choice_a = mmChoiceKeyNameA({
    { e_trx,         "Transaction",               _n("Transaction") },
    { e_stock,       "Stock",                     _n("Stock") },
    { e_asset,       "Asset",                     _n("Asset") },
    { e_account,     "BankAccount",               _n("Account") },
    { e_sched,       "RecurringTransaction",      _n("Schedule") },
    { e_payee,       "Payee",                     _n("Payee") },
    { e_trx_split,   "TransactionSplit",          _n("Transaction Split") },
    { e_sched_split, "RecurringTransactionSplit", _n("Schedule Split") },
}, -1, true);

mmChoiceCodeNameA UpdateType::s_choice_a = mmChoiceCodeNameA({
    { e_none,   0, _n("None") },
    { e_online, 1, _n("Online") },
    { e_manual, 2, _n("Manual") },
}, e_none, true);

