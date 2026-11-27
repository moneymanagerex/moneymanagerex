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

// PlanAssumptionData represents a record in table PLANASSUMPTION_V1.
//
// A long-term plan is only as good as the guesses behind it. An assumption
// records one of those guesses once -- "MSFT is worth $400", "vesting is taxed
// at 32%" -- so that every dependent figure is calculated from it. Changing the
// assumption changes the whole plan, and a report can state plainly which
// numbers are facts and which rest on an estimate.

#include "_DataEnum.h"
#include "table/PlanAssumptionTable.h"

struct PlanAssumptionData
{
    int64              m_id;
    wxString           m_name;
    PlanAssumptionKind m_kind;
    double             m_value;
    wxString           m_scope_key;   // e.g. "MSFT" for a share price
    wxString           m_notes;
    wxString           m_as_of_date;
    bool               m_active;
    wxString           m_unit;        // "$", "%", "shares", ... how to read m_value
    int64              m_group_id;    // member of an assumption group, -1 when standalone

    explicit PlanAssumptionData();
    explicit PlanAssumptionData(wxSQLite3ResultSet& q);
    PlanAssumptionData(const PlanAssumptionData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    PlanAssumptionRow to_row() const;
    PlanAssumptionData& from_row(const PlanAssumptionRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    PlanAssumptionData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    PlanAssumptionData& clone_from(const PlanAssumptionData& other);
    bool equals(const PlanAssumptionData* other) const;
    bool operator< (const PlanAssumptionData& other) const { return id() < other.id(); }
    bool operator< (const PlanAssumptionData* other) const { return id() < other->id(); }

    // Rate assumptions are stored however the user typed them; normalise to a
    // 0..1 fraction so 32 and 0.32 both mean 32%.
    double as_rate() const {
        if (m_value <= 0.0) return 0.0;
        const double r = (m_value > 1.0) ? (m_value / 100.0) : m_value;
        return (r >= 1.0) ? 1.0 : r;
    }

    bool is_grouped() const { return m_group_id > 0; }

    // The unit to show when none is recorded: a rate reads as a percentage,
    // anything else is left to the caller (a price is shown in currency).
    const wxString unit_or_default() const {
        if (!m_unit.IsEmpty())
            return m_unit;
        if (m_kind.id() == PlanAssumptionKind::e_tax_rate ||
            m_kind.id() == PlanAssumptionKind::e_inflation)
            return "%";
        return "";
    }

    // Value shifted by a percentage, used for sensitivity analysis
    // ("what if MSFT is 20% lower than I assumed?").
    double value_shifted(double pct) const {
        return m_value * (1.0 + pct / 100.0);
    }

    struct SorterByASSUMPTIONID
    {
        bool operator()(const PlanAssumptionData& x, const PlanAssumptionData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByASSUMPTIONNAME
    {
        bool operator()(const PlanAssumptionData& x, const PlanAssumptionData& y)
        {
            return x.m_name < y.m_name;
        }
    };
};

inline PlanAssumptionData::PlanAssumptionData(wxSQLite3ResultSet& q) :
    PlanAssumptionData()
{
    from_select_result(q);
}

inline void PlanAssumptionData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void PlanAssumptionData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline PlanAssumptionData& PlanAssumptionData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(PlanAssumptionRow().from_select_result(q));
}

inline wxString PlanAssumptionData::to_json() const
{
    return to_row().to_json();
}

inline void PlanAssumptionData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t PlanAssumptionData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void PlanAssumptionData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline PlanAssumptionData& PlanAssumptionData::clone_from(const PlanAssumptionData& other)
{
    *this = other;
    id(-1);
    return *this;
}
