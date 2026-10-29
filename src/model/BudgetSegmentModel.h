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
#include "data/BudgetSegmentData.h"

class BudgetSegmentModel : public TableFactory<BudgetSegmentTable, BudgetSegmentData>
{
// -- constructor

public:
    BudgetSegmentModel() :
        TableFactory<BudgetSegmentTable, BudgetSegmentData>() {}
    ~BudgetSegmentModel() {}

    static BudgetSegmentModel& instance(wxSQLite3Database* db);
    static BudgetSegmentModel& instance();

// -- override

public:
    // override TableFactory
    virtual bool purge_id(int64 id) override;

// -- methods

public:
    // All active segments of a budget period, ordered for display.
    auto find_period_a(int64 bp_id) -> DataA;

    // Create the default paycheck-aligned split (1-15, 16-end of month).
    // Returns false if the period already has segments, so it is safe to call
    // more than once.
    bool create_default_halves(int64 bp_id);

    // Detach every budget entry from a segment before the segment is removed,
    // so entries fall back to "whole period" instead of dangling.
    void detach_entries(int64 segment_id);
};
