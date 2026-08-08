/*******************************************************
 Copyright (C) 2026

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

#pragma once

// PlanEngine turns the stored plan into numbers.
//
// It has two jobs:
//
// 1. Resolve budget amounts that the user did not type. A budget entry may be
//    marked Auto with an AUTOSOURCE such as "CCBALANCE:52" (what a credit card
//    currently owes) or "AVG:3M" (what this category actually averaged), which
//    removes the "?" placeholders that force manual lookups.
//
// 2. Project a timeline. Opening balances, budget entries expanded per segment,
//    scheduled transactions and long-term plan items are merged into periods
//    with a running cash balance and a running "free assets" figure.
//
// The engine deliberately has no wx UI dependencies so it can be reasoned about
// and unit tested in isolation.

#include <vector>
#include <map>
#include "base/_defs.h"
#include "base/mmDate.h"
#include "data/BudgetData.h"
#include "data/BudgetSegmentData.h"
#include "data/PlanItemData.h"

// How an Auto budget amount is derived.
struct PlanAutoSource
{
    enum KIND {
        e_none = 0,   // not an Auto entry, or an unparseable source
        e_cc_balance, // CCBALANCE:<account_id>  -> amount currently owed
        e_average,    // AVG:<n>M                -> mean actual over n months
        e_last,       // LAST                    -> most recent actual
        e_scheduled   // SCHED:<sched_id>        -> amount of a scheduled trx
    };

    KIND  kind   = e_none;
    int64 ref_id = -1;  // account id or scheduled transaction id
    int   months = 0;   // averaging window

    bool is_valid() const { return kind != e_none; }

    // Parse an AUTOSOURCE string. Never throws; an unrecognised string yields
    // kind == e_none so the caller can flag it instead of silently using 0.
    static PlanAutoSource parse(const wxString& source);
};

// A resolved budget amount, with enough context for the UI to explain itself.
struct PlanResolvedAmount
{
    double   amount      = 0.0;
    bool     is_derived  = false;  // computed rather than typed
    bool     is_estimate = false;  // uncertain, render with a marker
    bool     unresolved  = false;  // Auto entry whose source could not resolve
    wxString explanation;          // e.g. "3-month average of actuals"
};

// One row of the projected timeline.
struct PlanPeriod
{
    wxString label;             // e.g. "2026-03 First half"
    mmDate   start_date = mmDate::invalid();
    mmDate   end_date   = mmDate::invalid();
    int64    segment_id = -1;   // -1 when the period is not segmented

    double income          = 0.0;
    double expense         = 0.0;  // positive magnitude
    double plan_income     = 0.0;  // from long-term plan items
    double plan_expense    = 0.0;
    bool   has_estimates   = false;
    bool   has_unresolved  = false;

    double net() const { return income + plan_income - expense - plan_expense; }

    double closing_balance = 0.0;  // running cash balance after this period
    double free_assets     = 0.0;  // liquid assets less outstanding obligations
};

// A long-term summary mirroring the structure people keep in a notebook.
struct PlanSummary
{
    double cash_assets        = 0.0;  // liquid accounts
    double investment_assets  = 0.0;  // stock / share accounts
    double total_assets       = 0.0;

    double expected_income    = 0.0;  // net of tax, from plan items
    double committed_expense  = 0.0;  // Planned + Committed
    double wishlist_expense   = 0.0;  // Wishlist only; not an obligation

    // How much of the picture is a guess rather than a fact. Investment values
    // and vesting income both move with an assumed share price, so it matters
    // that this is stated rather than buried.
    double assumption_based_income = 0.0;
    int    assumption_count        = 0;

    // What is genuinely uncommitted: assets plus expected income, less the
    // obligations already taken on. Wishlist items are excluded by design.
    double free_assets() const {
        return total_assets + expected_income - committed_expense;
    }
    // The same figure if every wishlist entry were also acted on.
    double free_assets_after_wishlist() const {
        return free_assets() - wishlist_expense;
    }
    // Share of expected income that rests on an assumption (0..1).
    double assumption_exposure() const {
        return (expected_income > 0.0)
            ? (assumption_based_income / expected_income)
            : 0.0;
    }
};

// The plan recomputed with every assumption shifted, so the user can see how
// much of the outcome depends on a guess being right.
struct PlanSensitivity
{
    double shift_pct       = 0.0;  // e.g. -20 for "20% lower than assumed"
    double expected_income = 0.0;
    double free_assets     = 0.0;
    double delta           = 0.0;  // change in free assets vs the base case
};

class PlanEngine
{
public:
    // -- amount resolution

    // Resolve one budget entry to a concrete amount.
    // `as_of` bounds the historical window used by AVG / LAST.
    static PlanResolvedAmount resolve_amount(
        const BudgetData& budget_d,
        const mmDate& as_of
    );

    // Mean absolute amount per month for a category over the last n months.
    // Returns 0 when there is no history, never divides by zero.
    static double average_actual(int64 category_id, int months, const mmDate& as_of);

    // Most recent actual amount for a category on or before `as_of`.
    static double last_actual(int64 category_id, const mmDate& as_of);

    // What a credit-card account currently owes, as a positive number.
    // Returns 0 for an unknown account or an account that is in credit.
    static double credit_card_balance(int64 account_id);

    // -- timeline

    // Expand a budget period into concrete periods. When the period has
    // segments each month yields one row per segment; otherwise one row per
    // month. `months` bounds the horizon.
    static std::vector<PlanPeriod> build_timeline(
        int64 budget_period_id,
        const mmDate& start_date,
        int months
    );

    // Long-term totals across every active plan item.
    static PlanSummary build_summary(const mmDate& as_of);

    // Recompute the summary with every assumption-driven value shifted by
    // `pct` percent. Passing a set of shifts (e.g. -20, -10, +10) produces the
    // sensitivity table shown alongside the plan.
    static PlanSensitivity build_sensitivity(const mmDate& as_of, double pct);
    static std::vector<PlanSensitivity> build_sensitivity_a(
        const mmDate& as_of,
        const std::vector<double>& pct_a
    );

private:
    // Sum of balances over accounts the user can actually spend.
    static double liquid_balance();
    static double investment_balance();

    // Portion of a budget entry's yearly amount that falls inside a window.
    static double amount_for_window(
        const BudgetData& budget_d,
        double resolved_amount,
        int days_in_window,
        int days_in_month
    );
};
