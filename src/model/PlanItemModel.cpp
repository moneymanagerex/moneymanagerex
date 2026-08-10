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

#include "base/_defs.h"

#include "PlanItemModel.h"
#include "PlanAssumptionModel.h"
#include "PlanAssumptionGroupModel.h"
#include "CurrencyModel.h"
#include "StockModel.h"

// -- static

TableClauseV<wxString> PlanItemModel::WHERE_KIND(OP op, PlanItemKind kind)
{
    return PlanItemCol::WHERE_KIND(op, kind.key());
}

TableClauseV<wxString> PlanItemModel::WHERE_STATUS(OP op, PlanStatus status)
{
    return PlanItemCol::WHERE_STATUS(op, status.key());
}

// -- constructor

// Initialize the global PlanItemModel table.
PlanItemModel& PlanItemModel::instance(wxSQLite3Database* db)
{
    PlanItemModel& ins = Singleton<PlanItemModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of PlanItemModel table
PlanItemModel& PlanItemModel::instance()
{
    return Singleton<PlanItemModel>::instance();
}

// -- methods

PlanItemModel::DataA PlanItemModel::find_group_a(int64 group_id)
{
    DataA a = find_data_a(
        PlanItemCol::WHERE_GROUPID(OP_EQ, group_id)
    );

    DataA active;
    for (const auto& item : a) {
        if (item.m_active)
            active.push_back(item);
    }
    std::sort(active.begin(), active.end(), Data::SorterByTARGETDATE());

    return active;
}

PlanItemModel::DataA PlanItemModel::find_between_a(
    const wxString& date_from,
    const wxString& date_to
) {
    DataA a = find_data_a();

    DataA out;
    for (const auto& item : a) {
        if (!item.m_active)
            continue;
        // Items with no target date cannot be placed on a timeline; the caller
        // still sees them through find_group_a.
        if (item.m_target_date.IsEmpty())
            continue;
        if (!date_from.IsEmpty() && item.m_target_date < date_from)
            continue;
        if (!date_to.IsEmpty() && item.m_target_date > date_to)
            continue;
        out.push_back(item);
    }
    std::sort(out.begin(), out.end(), Data::SorterByTARGETDATE());

    return out;
}

double PlanItemModel::resolve_unit_price(const PlanItemData& item)
{
    if (!item.is_unit_based())
        return 0.0;

    PlanAssumptionModel& pam = PlanAssumptionModel::instance();
    PlanAssumptionGroupModel& pagm = PlanAssumptionGroupModel::instance();
    const PlanAssumptionKind kind(PlanAssumptionKind::e_share_price);

    // 1. A pinned member is a deliberate exception: this one row is held at one
    //    particular value regardless of what the group is currently set to.
    //    It is only honoured while it still answers the right question.
    if (item.m_price_assumption_id > 0 &&
        pam.id_applies_to(item.m_price_assumption_id, kind, item.m_stock_symbol)) {
        const double v = pam.get_value(item.m_price_assumption_id, 0.0);
        if (v > 0.0)
            return v;
    }

    // 2. The usual path: follow whichever member the assigned group has active,
    //    so switching that single choice moves every item using the group.
    if (item.m_price_assumption_group_id > 0 &&
        pagm.accepts(item.m_price_assumption_group_id, kind, item.m_stock_symbol)) {
        const double v = pagm.get_active_value(item.m_price_assumption_group_id, 0.0);
        if (v > 0.0)
            return v;
    }

    // 3. A per-item override.
    if (item.m_unit_price > 0.0)
        return item.m_unit_price;

    // 4. A standalone share-price assumption scoped to the same symbol.
    if (!item.m_stock_symbol.IsEmpty()) {
        const PlanAssumptionData* a_n = pam.get_scope_data_n(kind, item.m_stock_symbol);
        if (a_n && a_n->m_value > 0.0)
            return a_n->m_value;
    }

    // 5. The live price of a holding with the same symbol.
    return get_market_price(item);
}

double PlanItemModel::resolve_tax_rate(const PlanItemData& item)
{
    PlanAssumptionModel& pam = PlanAssumptionModel::instance();
    PlanAssumptionGroupModel& pagm = PlanAssumptionGroupModel::instance();
    const PlanAssumptionKind kind(PlanAssumptionKind::e_tax_rate);

    // A tax rate is not scoped to a ticker, so only the kind has to agree.
    if (item.m_tax_assumption_id > 0 &&
        pam.id_applies_to(item.m_tax_assumption_id, kind, wxEmptyString)) {
        const double r = pam.get_rate(item.m_tax_assumption_id, -1.0);
        if (r >= 0.0)
            return r;
    }

    if (item.m_tax_assumption_group_id > 0 &&
        pagm.accepts(item.m_tax_assumption_group_id, kind, wxEmptyString)) {
        const double r = pagm.get_active_rate(item.m_tax_assumption_group_id, -1.0);
        if (r >= 0.0)
            return r;
    }

    // Normalise a per-item rate the same way, so 32 and 0.32 agree.
    const double raw = item.m_tax_rate;
    if (raw <= 0.0) return 0.0;
    const double r = (raw > 1.0) ? (raw / 100.0) : raw;
    return (r >= 1.0) ? 1.0 : r;
}

double PlanItemModel::get_market_price(const PlanItemData& item)
{
    if (!item.is_unit_based())
        return 0.0;

    if (item.m_stock_symbol.IsEmpty())
        return 0.0;

    // Fall back to the current price of any holding with the same symbol.
    for (const auto& stock_d : StockModel::instance().find_data_a()) {
        if (stock_d.m_symbol.IsSameAs(item.m_stock_symbol, false))
            return stock_d.m_current_price;
    }

    return 0.0;
}

double PlanItemModel::net_amount_base(const PlanItemData& item)
{
    double net;

    if (item.is_unit_based()) {
        const double gross = item.gross_amount_at(resolve_unit_price(item));
        net = PlanItemData::apply_tax(gross, resolve_tax_rate(item));
    }
    else {
        net = item.m_amount;
    }

    // Convert to the base currency when the item names a foreign one.
    if (item.m_currency_id > 0) {
        const CurrencyData* curr_n =
            CurrencyModel::instance().get_idN_data_n(item.m_currency_id);
        if (curr_n && curr_n->m_base_conv_rate > 0.0)
            net *= curr_n->m_base_conv_rate;
    }

    return net;
}

double PlanItemModel::net_amount_base_shifted(const PlanItemData& item, double pct)
{
    // Only assumption-driven values move; a flat amount the user typed is a
    // decision, not a guess, so it stays put.
    if (!item.is_unit_based())
        return net_amount_base(item);

    const double price = resolve_unit_price(item) * (1.0 + pct / 100.0);
    const double gross = item.gross_amount_at(price);
    double net = PlanItemData::apply_tax(gross, resolve_tax_rate(item));

    if (item.m_currency_id > 0) {
        const CurrencyData* curr_n =
            CurrencyModel::instance().get_idN_data_n(item.m_currency_id);
        if (curr_n && curr_n->m_base_conv_rate > 0.0)
            net *= curr_n->m_base_conv_rate;
    }

    return net;
}

double PlanItemModel::sum_obligations(const DataA& items)
{
    double total = 0.0;
    for (const auto& item : items) {
        if (item.m_kind.is_income())
            continue;
        if (!item.is_obligation())
            continue;
        total += net_amount_base(item);
    }
    return total;
}

double PlanItemModel::sum_wishlist(const DataA& items)
{
    double total = 0.0;
    for (const auto& item : items) {
        if (item.m_kind.is_income())
            continue;
        if (!item.m_active || !item.m_status.is_wishlist())
            continue;
        total += net_amount_base(item);
    }
    return total;
}

double PlanItemModel::sum_income(const DataA& items)
{
    double total = 0.0;
    for (const auto& item : items) {
        if (!item.m_kind.is_income())
            continue;
        if (!item.m_active || !item.m_status.is_active_plan())
            continue;
        total += net_amount_base(item);
    }
    return total;
}

double PlanItemModel::sum_income_shifted(const DataA& items, double pct)
{
    double total = 0.0;
    for (const auto& item : items) {
        if (!item.m_kind.is_income())
            continue;
        if (!item.m_active || !item.m_status.is_active_plan())
            continue;
        total += net_amount_base_shifted(item, pct);
    }
    return total;
}
