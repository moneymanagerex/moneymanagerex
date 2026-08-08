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

// PlanItemData represents a record in table PLANITEM_V1.
//
// A plan item is a single future income or expense in the long-term plan,
// optionally grouped under a PlanGroup (a trip, a collection, a project).
//
// When m_units > 0 the amount is derived as units * unit price, and m_tax_rate
// gives the net (after-tax) value. That models share/RSU vesting, e.g.
// "9 shares ~= $3,600 ($2,450 after tax)".

#include "_DataEnum.h"
#include "table/PlanItemTable.h"

struct PlanItemData
{
    int64        m_id;
    int64        m_group_id;
    wxString     m_name;
    wxString     m_notes;
    PlanItemKind m_kind;
    PlanStatus   m_status;
    wxString     m_target_date;
    double       m_amount;
    int64        m_currency_id;
    int64        m_category_id;
    int64        m_account_id;
    double       m_units;
    double       m_unit_price;
    double       m_tax_rate;
    wxString     m_stock_symbol;
    double       m_confidence;
    int          m_sort_order;
    bool         m_active;
    // Shared assumptions this item is calculated from. When set they take
    // precedence over the per-item unit price / tax rate, so changing the
    // assumption once updates every item that references it.
    int64        m_price_assumption_id;
    int64        m_tax_assumption_id;

    explicit PlanItemData();
    explicit PlanItemData(wxSQLite3ResultSet& q);
    PlanItemData(const PlanItemData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    PlanItemRow to_row() const;
    PlanItemData& from_row(const PlanItemRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    PlanItemData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    PlanItemData& clone_from(const PlanItemData& other);
    bool equals(const PlanItemData* other) const;
    bool operator< (const PlanItemData& other) const { return id() < other.id(); }
    bool operator< (const PlanItemData* other) const { return id() < other->id(); }

    // -- amount resolution
    //
    // Resolution order for a unit-based item is deliberate:
    //   1. a linked price assumption   (shared, e.g. "MSFT @ 400")
    //   2. the item's own unit price   (a one-off override)
    //   3. a live market price         (supplied by the caller)
    // The same order applies to the tax rate. This keeps a shared guess in one
    // place while still allowing a single row to deviate.

    // True when this item is quantity-based (shares) rather than a flat amount.
    bool is_unit_based() const { return m_units > 0.0; }

    // True when any part of this item's value rests on an assumption.
    bool is_assumption_based() const {
        return m_price_assumption_id > 0 || m_tax_assumption_id > 0;
    }

    // Gross value before tax, given an already-resolved price. Callers should
    // pass the price from PlanItemModel::resolve_unit_price().
    double gross_amount_at(double resolved_price) const {
        return is_unit_based() ? m_units * resolved_price : m_amount;
    }

    // Gross value before tax. For unit-based items an explicit unit price wins;
    // callers that want a live market price should pass it in via
    // gross_amount(market_price).
    double gross_amount() const {
        return is_unit_based() ? m_units * m_unit_price : m_amount;
    }

    // Gross value using a market price when the item has no explicit unit price.
    double gross_amount(double market_price) const {
        if (!is_unit_based())
            return m_amount;
        const double price = (m_unit_price > 0.0) ? m_unit_price : market_price;
        return m_units * price;
    }

    // Net (after-tax) value. Tax only applies to unit-based income such as
    // vesting shares; a flat amount is assumed to be already net.
    //
    // A rate may be stored as a fraction (0.32) or as a percentage (32); both
    // mean the same thing. The threshold is strictly greater than 1 so that a
    // stored 1.0 means 100%, matching PlanAssumptionData::as_rate and
    // PlanItemModel::resolve_tax_rate. A genuine 1% must be stored as 0.01.
    static double apply_tax(double gross, double tax_rate) {
        if (tax_rate <= 0.0) return gross;
        const double rate = (tax_rate > 1.0) ? (tax_rate / 100.0) : tax_rate;
        // guard against nonsensical rates so we never return a negative net
        return (rate >= 1.0) ? 0.0 : gross * (1.0 - rate);
    }

    double net_amount() const {
        return is_unit_based() ? apply_tax(gross_amount(), m_tax_rate) : m_amount;
    }

    double net_amount(double market_price) const {
        return is_unit_based()
            ? apply_tax(gross_amount(market_price), m_tax_rate)
            : m_amount;
    }

    // Net value weighted by confidence (0..1), for risk-adjusted totals.
    double weighted_net_amount(double market_price) const {
        double c = m_confidence;
        if (c <= 0.0) c = 1.0;           // unset == certain
        if (c > 1.0)  c = c / 100.0;     // tolerate 0..100 input
        if (c > 1.0)  c = 1.0;
        return net_amount(market_price) * c;
    }

    // Only Planned/Committed items reduce free assets; a Wishlist entry is an
    // aspiration and must not be treated as an obligation.
    bool is_obligation() const {
        return m_active && m_status.is_obligation();
    }

    struct SorterByPLANITEMID
    {
        bool operator()(const PlanItemData& x, const PlanItemData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByTARGETDATE
    {
        bool operator()(const PlanItemData& x, const PlanItemData& y)
        {
            if (x.m_target_date != y.m_target_date)
                return x.m_target_date < y.m_target_date;
            return x.m_sort_order < y.m_sort_order;
        }
    };

    struct SorterByITEMNAME
    {
        bool operator()(const PlanItemData& x, const PlanItemData& y)
        {
            return x.m_name < y.m_name;
        }
    };
};

inline PlanItemData::PlanItemData(wxSQLite3ResultSet& q) :
    PlanItemData()
{
    from_select_result(q);
}

inline void PlanItemData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void PlanItemData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline PlanItemData& PlanItemData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(PlanItemRow().from_select_result(q));
}

inline wxString PlanItemData::to_json() const
{
    return to_row().to_json();
}

inline void PlanItemData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t PlanItemData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void PlanItemData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline PlanItemData& PlanItemData::clone_from(const PlanItemData& other)
{
    *this = other;
    id(-1);
    return *this;
}
