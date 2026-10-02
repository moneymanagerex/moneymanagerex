/*******************************************************
 Copyright (C) 2013,2014 James Higley
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)
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

#pragma once

#include <float.h>
#include "base/_defs.h"
#include "base/mmSingleton.h"
#include "util/mmDateRange.h"
#include "table/_TableFactory.h"
#include "data/BudgetData.h"

class BudgetModel : public TableFactory<BudgetTable, BudgetData>
{
// -- static

public:
    static auto WHERE_FREQUENCY(OP op, BudgetFreq freq) -> TableClauseV<wxString>;

// -- constructor

public:
    BudgetModel() :
        TableFactory<BudgetTable, BudgetData>() {}
    ~BudgetModel() {}

    static BudgetModel& instance(wxSQLite3Database* db);
    static BudgetModel& instance();

// -- override

public:
    // override TableFactory
    virtual bool purge_id(int64 id) override {
        return unsafe_remove_id(id);
    }

// -- methods

public:
    // Reading scope for getBudgetEntry. A period split into segments holds
    // entries at two levels -- some belonging to a part, some to the period as
    // a whole -- and which of them to read depends on what is being shown.
    //   SEGMENT_ANY  : every entry, whichever level it sits at
    //   SEGMENT_NONE : only entries that apply to the whole period
    //   >= 0         : only entries belonging to that segment
    static const int64 SEGMENT_ANY;
    static const int64 SEGMENT_NONE;

    void getBudgetEntry(
        int64 bp_id,
        std::map<int64, BudgetFreq>& freq_mCatId,
        std::map<int64, double>& amount_mCatId,
        std::map<int64, wxString>& notes_mCatId,
        // A year and its months are separate periods that do not normally
        // interact. Setting this rolls the months of a year period into it, so
        // a year can be read as the sum of what is planned month by month
        // without either being rewritten.
        bool include_child_periods = false,
        // Applies to bp_id only. Child periods brought in by the flag above are
        // always read whole, since rolling up means totalling everything.
        int64 segment_scope = SEGMENT_ANY
    );

    // The month periods belonging to a year period ("2026" -> "2026-01"...),
    // matched by name in the same way the navigation tree nests them.
    auto find_child_period_id_a(int64 bp_id) -> std::vector<int64>;

    // Where an entry sits inside a period. Moving one between a segment and the
    // period as a whole is a data operation with several outcomes worth telling
    // apart, so it lives here rather than in whichever panel offers it.
    enum MoveOutcome
    {
        MOVE_DONE,          // re-filed
        MOVE_MERGED,        // combined with an entry already in the destination
        MOVE_UNCHANGED,     // already there
        MOVE_NO_SOURCE,     // no entry to move
        MOVE_OCCUPIED,      // destination taken and merging was not permitted
        MOVE_FREQ_MISMATCH  // merging would change the total, so refused
    };

    // Two entries for one category in one place would be silently summed by
    // every reader, so a collision is resolved deliberately: merged only when
    // allow_merge is set and the frequencies agree, otherwise refused.
    auto move_entry_segment(
        int64 bp_id,
        int64 cat_id,
        int64 from_segment_id,
        int64 to_segment_id,
        bool allow_merge
    ) -> MoveOutcome;

    // The entry for a category in one part of a period, or nullptr. A segment
    // id <= 0 means the period as a whole.
    auto find_entry_n(int64 bp_id, int64 cat_id, int64 segment_id) -> BudgetData*;

    void getBudgetStats(
        std::map<int64, std::map<int, double>>& budgetStats,
        mmDateRange* date_range,
        bool groupByMonth
    );
    void copyBudgetYear(int64 dst_bp_id, int64 src_bp_id);
};
