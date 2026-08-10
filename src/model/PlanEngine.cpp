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

#include <algorithm>
#include <cmath>
#include "base/_defs.h"

#include "PlanEngine.h"
#include "AccountModel.h"
#include "BudgetModel.h"
#include "BudgetSegmentModel.h"
#include "CurrencyModel.h"
#include "PlanAssumptionModel.h"
#include "PlanGroupModel.h"
#include "PlanItemModel.h"
#include "SchedModel.h"
#include "TrxModel.h"

namespace
{
    // mmDate::dateTime() is non-const (it caches), and mmDate has no validity
    // predicate, so these two helpers keep the call sites honest.
    bool date_is_set(const mmDate& d)
    {
        return !d.isoDate().IsEmpty();
    }

    wxDateTime to_datetime(const mmDate& d)
    {
        mmDate copy = d;
        return copy.dateTime();
    }

    // Shift an ISO date by whole months, clamping the day to the target month.
    wxDateTime add_months(const wxDateTime& base, int months)
    {
        wxDateTime d = base;
        d.SetDay(1);
        d += wxDateSpan::Months(months);
        return d;
    }

    int days_in_month_of(const wxDateTime& d)
    {
        return wxDateTime::GetNumberOfDays(d.GetMonth(), d.GetYear());
    }

    bool account_is_liquid(const AccountData& a)
    {
        // Cash the user can actually spend. Credit cards are handled separately
        // because their balance is a debt, not an asset.
        const mmNavigatorItem::TYPE_ID type = AccountModel::type_id(a);
        return type == mmNavigatorItem::TYPE_ID_CASH
            || type == mmNavigatorItem::TYPE_ID_CHECKING
            || type == mmNavigatorItem::TYPE_ID_TERM;
    }
}

// -- PlanAutoSource

PlanAutoSource PlanAutoSource::parse(const wxString& source)
{
    PlanAutoSource out;

    wxString s = source;
    s.Trim(true).Trim(false);
    if (s.IsEmpty())
        return out;

    s.MakeUpper();

    if (s == "LAST") {
        out.kind = e_last;
        return out;
    }

    wxString rest;
    if (s.StartsWith("CCBALANCE:", &rest)) {
        long id = 0;
        if (rest.Trim().ToLong(&id) && id > 0) {
            out.kind   = e_cc_balance;
            out.ref_id = int64(id);
        }
        return out;
    }

    if (s.StartsWith("SCHED:", &rest)) {
        long id = 0;
        if (rest.Trim().ToLong(&id) && id > 0) {
            out.kind   = e_scheduled;
            out.ref_id = int64(id);
        }
        return out;
    }

    if (s.StartsWith("AVG:", &rest)) {
        rest.Trim(true).Trim(false);
        if (rest.EndsWith("M"))
            rest.RemoveLast();
        long n = 0;
        if (rest.ToLong(&n) && n > 0 && n <= 120) {
            out.kind   = e_average;
            out.months = static_cast<int>(n);
        }
        return out;
    }

    // Unrecognised: leave kind == e_none so the caller can flag it.
    return out;
}

// -- amount resolution

double PlanEngine::credit_card_balance(int64 account_id)
{
    const AccountData* acc_n = AccountModel::instance().get_idN_data_n(account_id);
    if (!acc_n)
        return 0.0;

    const double balance = AccountModel::instance().get_data_balance(*acc_n);

    // A card in debt carries a negative balance; the amount to repay is its
    // magnitude. A card in credit owes nothing.
    return (balance < 0.0) ? -balance : 0.0;
}

double PlanEngine::average_actual(int64 category_id, int months, const mmDate& as_of)
{
    if (category_id <= 0 || months <= 0)
        return 0.0;

    wxDateTime end = date_is_set(as_of) ? to_datetime(as_of) : wxDateTime::Today();
    wxDateTime start = add_months(end, -months);

    const mmDate from(start);
    const mmDate to(end);

    double total = 0.0;
    for (const auto& trx_d : TrxModel::instance().find_data_a(
        TrxModel::WHERE_DATE(OP_GE, from),
        TrxModel::WHERE_DATE(OP_LE, to)
    )) {
        if (trx_d.is_transfer())
            continue;
        if (trx_d.m_category_id_n != category_id)
            continue;
        total += std::fabs(trx_d.m_amount);
    }

    return total / static_cast<double>(months);
}

double PlanEngine::last_actual(int64 category_id, const mmDate& as_of)
{
    if (category_id <= 0)
        return 0.0;

    const mmDate to = date_is_set(as_of) ? as_of : mmDate::today();

    double amount = 0.0;
    wxString best_date;

    for (const auto& trx_d : TrxModel::instance().find_data_a(
        TrxModel::WHERE_DATE(OP_LE, to)
    )) {
        if (trx_d.is_transfer())
            continue;
        if (trx_d.m_category_id_n != category_id)
            continue;
        const wxString d = trx_d.m_isoDate();
        if (best_date.IsEmpty() || d > best_date) {
            best_date = d;
            amount = std::fabs(trx_d.m_amount);
        }
    }

    return amount;
}

PlanResolvedAmount PlanEngine::resolve_amount(
    const BudgetData& budget_d,
    const mmDate& as_of
) {
    PlanResolvedAmount out;

    // Fixed and Estimated amounts are whatever the user typed; only the
    // presentation differs.
    if (!budget_d.m_amount_type.is_derived()) {
        out.amount      = budget_d.m_amount;
        out.is_estimate = budget_d.m_amount_type.is_uncertain();
        return out;
    }

    out.is_derived  = true;
    out.is_estimate = true;

    const PlanAutoSource src = PlanAutoSource::parse(budget_d.m_auto_source);
    if (!src.is_valid()) {
        // Fall back to the stored amount rather than silently reporting zero,
        // and mark the entry so the UI can point at the broken source.
        out.amount     = budget_d.m_amount;
        out.unresolved = true;
        out.explanation = _t("Automatic amount could not be resolved");
        return out;
    }

    switch (src.kind) {
    case PlanAutoSource::e_cc_balance: {
        out.amount = credit_card_balance(src.ref_id);
        const wxString name = AccountModel::instance().get_id_name(src.ref_id);
        out.explanation = wxString::Format(
            _t("Outstanding balance of %s"), name.IsEmpty() ? _t("account") : name);
        break;
    }
    case PlanAutoSource::e_average: {
        out.amount = average_actual(budget_d.m_category_id, src.months, as_of);
        out.explanation = wxString::Format(
            _t("%d-month average of actual amounts"), src.months);
        break;
    }
    case PlanAutoSource::e_last: {
        out.amount = last_actual(budget_d.m_category_id, as_of);
        out.explanation = _t("Most recent actual amount");
        break;
    }
    case PlanAutoSource::e_scheduled: {
        const SchedData* s_n = SchedModel::instance().get_idN_data_n(src.ref_id);
        out.amount = s_n ? std::fabs(s_n->m_amount) : 0.0;
        if (!s_n) {
            out.unresolved = true;
            out.explanation = _t("Scheduled transaction not found");
        }
        else {
            out.explanation = _t("Amount of a scheduled transaction");
        }
        break;
    }
    default:
        break;
    }

    return out;
}

// -- timeline

double PlanEngine::amount_for_window(
    const BudgetData& budget_d,
    double resolved_amount,
    int days_in_window,
    int days_in_month
) {
    if (days_in_month <= 0)
        return 0.0;

    const BudgetFreq freq = budget_d.m_freq;

    // A frequency of None means the amount is not recurring; treat the stored
    // value as belonging wholly to the period it is attached to.
    if (freq.id() == BudgetFreq::e_none)
        return resolved_amount;

    // Spread the monthly-equivalent amount across the days of the window. This
    // keeps a 1-15 / 16-EOM split correct in every month length.
    const double monthly = resolved_amount * freq.times_per_month();
    if (days_in_window >= days_in_month)
        return monthly;

    return monthly * (static_cast<double>(days_in_window) / static_cast<double>(days_in_month));
}

double PlanEngine::liquid_balance()
{
    double total = 0.0;
    for (const auto& acc_d : AccountModel::instance().find_data_a()) {
        if (acc_d.m_status.key() != "Open")
            continue;
        if (!account_is_liquid(acc_d))
            continue;

        double bal = AccountModel::instance().get_data_balance(acc_d);

        const CurrencyData* curr_n =
            CurrencyModel::instance().get_idN_data_n(acc_d.m_currency_id);
        if (curr_n && curr_n->m_base_conv_rate > 0.0)
            bal *= curr_n->m_base_conv_rate;

        total += bal;
    }
    return total;
}

double PlanEngine::investment_balance()
{
    double total = 0.0;
    for (const auto& acc_d : AccountModel::instance().find_data_a()) {
        if (acc_d.m_status.key() != "Open")
            continue;
        const mmNavigatorItem::TYPE_ID type = AccountModel::type_id(acc_d);
        if (type != mmNavigatorItem::TYPE_ID_INVESTMENT
            && type != mmNavigatorItem::TYPE_ID_SHARES)
            continue;

        double bal = AccountModel::instance().get_data_balance(acc_d);

        const CurrencyData* curr_n =
            CurrencyModel::instance().get_idN_data_n(acc_d.m_currency_id);
        if (curr_n && curr_n->m_base_conv_rate > 0.0)
            bal *= curr_n->m_base_conv_rate;

        total += bal;
    }
    return total;
}

std::vector<PlanPeriod> PlanEngine::build_timeline(
    int64 budget_period_id,
    const mmDate& start_date,
    int months
) {
    std::vector<PlanPeriod> out;
    if (months <= 0)
        return out;

    // Cap the horizon so a bad argument cannot spin for ever.
    months = std::min(months, 120);

    const BudgetModel::DataA budget_a = BudgetModel::instance().find_data_a(
        BudgetCol::WHERE_BUDGETYEARID(OP_EQ, budget_period_id)
    );
    const BudgetSegmentModel::DataA segment_a =
        BudgetSegmentModel::instance().find_period_a(budget_period_id);

    wxDateTime cursor = date_is_set(start_date)
        ? to_datetime(start_date)
        : wxDateTime::Today();
    cursor.SetDay(1);

    double running_balance = liquid_balance();

    for (int m = 0; m < months; ++m) {
        const wxDateTime month_start = add_months(cursor, m);
        const int dim = days_in_month_of(month_start);
        const mmDate as_of(month_start);

        // One row per segment, or a single row when the period is unsegmented.
        const size_t rows = segment_a.empty() ? 1 : segment_a.size();

        for (size_t s = 0; s < rows; ++s) {
            PlanPeriod p;

            int win_start = 1;
            int win_end   = dim;

            if (!segment_a.empty()) {
                const BudgetSegmentData& seg = segment_a[s];
                win_start = seg.start_day_in_month(dim);
                win_end   = seg.end_day_in_month(dim);
                p.segment_id = seg.m_id;
                p.label = wxString::Format("%04d-%02d %s",
                    month_start.GetYear(),
                    int(month_start.GetMonth()) + 1,
                    seg.m_name);
            }
            else {
                p.label = wxString::Format("%04d-%02d",
                    month_start.GetYear(),
                    int(month_start.GetMonth()) + 1);
            }

            const int win_days = (win_end >= win_start) ? (win_end - win_start + 1) : 0;

            wxDateTime d_start = month_start; d_start.SetDay(win_start);
            wxDateTime d_end   = month_start; d_end.SetDay(win_end < 1 ? 1 : win_end);
            p.start_date = mmDate(d_start);
            p.end_date   = mmDate(d_end);

            // Budget entries
            for (const auto& b_d : budget_a) {
                if (!b_d.m_active)
                    continue;
                // A NULL segment applies to the whole period, so it is spread
                // across every segment rather than dropped.
                const bool entry_is_global = (b_d.m_segment_id <= 0);
                if (!segment_a.empty() && !entry_is_global &&
                    b_d.m_segment_id != p.segment_id)
                    continue;

                const PlanResolvedAmount r = resolve_amount(b_d, as_of);
                if (r.is_estimate)  p.has_estimates  = true;
                if (r.unresolved)   p.has_unresolved = true;

                // A global entry is apportioned to this window; a segment-bound
                // entry belongs to the window in full.
                const double amount = entry_is_global
                    ? amount_for_window(b_d, r.amount, win_days, dim)
                    : amount_for_window(b_d, r.amount, dim, dim);

                // Budget amounts are magnitudes; the category decides the sign.
                // Income categories are recorded as negative budget amounts in
                // MMEX, so a negative value means money coming in.
                if (amount < 0.0) p.income  += -amount;
                else              p.expense += amount;
            }

            // Long-term plan items landing inside this window
            const PlanItemModel::DataA items = PlanItemModel::instance().find_between_a(
                p.start_date.isoDate(), p.end_date.isoDate()
            );
            for (const auto& item : items) {
                if (!item.m_status.is_active_plan())
                    continue;
                // A wishlist entry is an aspiration, not a cash-flow commitment.
                if (item.m_status.is_wishlist())
                    continue;

                const double amt = PlanItemModel::instance().net_amount_base(item);
                if (item.m_kind.is_income()) p.plan_income  += amt;
                else                         p.plan_expense += amt;
            }

            running_balance += p.net();
            p.closing_balance = running_balance;
            out.push_back(p);
        }
    }

    // Free assets decline as obligations are met, so compute it once per row
    // from the summary rather than re-deriving it inside the loop.
    const PlanSummary summary = build_summary(start_date);
    double consumed = 0.0;
    for (auto& p : out) {
        consumed += p.plan_expense;
        p.free_assets = summary.free_assets() + consumed;
    }

    return out;
}

PlanSummary PlanEngine::build_summary(const mmDate& as_of)
{
    PlanSummary s;

    s.cash_assets       = liquid_balance();
    s.investment_assets = investment_balance();
    s.total_assets      = s.cash_assets + s.investment_assets;

    // Everything still ahead of us, including items that are planned but not
    // yet scheduled: an undated obligation is still money owed.
    const wxString from = date_is_set(as_of) ? as_of.isoDate() : mmDate::today().isoDate();
    const PlanItemModel::DataA items =
        PlanItemModel::instance().find_between_a(from, "", true);

    PlanItemModel& pim = PlanItemModel::instance();
    s.expected_income   = pim.sum_income(items);
    s.committed_expense = pim.sum_obligations(items);
    s.wishlist_expense  = pim.sum_wishlist(items);

    // Record how much of the expected income is only as good as its assumptions.
    for (const auto& item : items) {
        if (!item.m_kind.is_income())
            continue;
        if (!item.m_active || !item.m_status.is_active_plan())
            continue;
        if (item.is_unit_based() || item.is_assumption_based())
            s.assumption_based_income += pim.net_amount_base(item);
    }

    s.assumption_count = static_cast<int>(
        PlanAssumptionModel::instance().find_active_a().size());

    return s;
}

PlanSensitivity PlanEngine::build_sensitivity(const mmDate& as_of, double pct)
{
    PlanSensitivity out;
    out.shift_pct = pct;

    const PlanSummary base = build_summary(as_of);

    const wxString from = date_is_set(as_of) ? as_of.isoDate() : mmDate::today().isoDate();
    const PlanItemModel::DataA items =
        PlanItemModel::instance().find_between_a(from, "", true);

    out.expected_income = PlanItemModel::instance().sum_income_shifted(items, pct);

    // Investments move with the same assumed prices, so shift them too;
    // committed expenses are cash commitments and stay fixed.
    const double shifted_investments = base.investment_assets * (1.0 + pct / 100.0);
    const double shifted_assets = base.cash_assets + shifted_investments;

    out.free_assets = shifted_assets + out.expected_income - base.committed_expense;
    out.delta       = out.free_assets - base.free_assets();

    return out;
}

std::vector<PlanSensitivity> PlanEngine::build_sensitivity_a(
    const mmDate& as_of,
    const std::vector<double>& pct_a
) {
    std::vector<PlanSensitivity> out;
    for (double pct : pct_a)
        out.push_back(build_sensitivity(as_of, pct));
    return out;
}
