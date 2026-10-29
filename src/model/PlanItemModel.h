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

#include "base/_defs.h"
#include "base/mmSingleton.h"
#include "table/_TableFactory.h"
#include "data/PlanItemData.h"

class PlanItemModel : public TableFactory<PlanItemTable, PlanItemData>
{
// -- static

public:
    static auto WHERE_KIND(OP op, PlanItemKind kind) -> TableClauseV<wxString>;
    static auto WHERE_STATUS(OP op, PlanStatus status) -> TableClauseV<wxString>;

// -- constructor

public:
    PlanItemModel() :
        TableFactory<PlanItemTable, PlanItemData>() {}
    ~PlanItemModel() {}

    static PlanItemModel& instance(wxSQLite3Database* db);
    static PlanItemModel& instance();

// -- override

public:
    // override TableFactory
    virtual bool purge_id(int64 id) override {
        return unsafe_remove_id(id);
    }

// -- methods

public:
    // Active items of a group, ordered by target date.
    auto find_group_a(int64 group_id) -> DataA;

    // Active items whose target date falls in [date_from, date_to] (ISO yyyy-mm-dd).
    // An empty bound is treated as open-ended. Items with no target date are
    // omitted by default because they cannot be placed on a timeline, but a
    // caller totalling money must include them: set include_undated.
    auto find_between_a(const wxString& date_from, const wxString& date_to,
        bool include_undated = false) -> DataA;

    // Active items that are planned but not scheduled.
    auto find_undated_a() -> DataA;

    // Resolve the unit price for a share-based item.
    // Order: linked price assumption -> the item's own unit price -> live
    // market price from a matching holding -> a share-price assumption whose
    // scope matches the symbol.
    double resolve_unit_price(const PlanItemData& item);

    // Resolve the tax rate (0..1). Order: linked tax assumption -> own rate.
    double resolve_tax_rate(const PlanItemData& item);

    // Resolve the market price used for unit-based (share/RSU) items.
    // Falls back to the item's own unit price, then to 0.
    double get_market_price(const PlanItemData& item);

    // Net value of an item in the base currency, assumptions resolved.
    double net_amount_base(const PlanItemData& item);

    // Net value if every assumption this item depends on were shifted by
    // `pct` percent. Used to show how exposed the plan is to a bad guess.
    double net_amount_base_shifted(const PlanItemData& item, double pct);

    // Sum of net amounts, in base currency, over items matching a predicate.
    // Wishlist items are excluded from obligations by PlanStatus::is_obligation.
    double sum_obligations(const DataA& items);
    double sum_wishlist(const DataA& items);
    double sum_income(const DataA& items);
    double sum_income_shifted(const DataA& items, double pct);
};
