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

// BudgetSegmentData represents a record in table BUDGETSEGMENT_V1.
//
// A segment splits a budget period into intra-period windows, so a monthly
// budget can be planned against paycheck-aligned halves (e.g. day 1-15 and
// day 16-end of month) instead of a single lump sum.

#include "_DataEnum.h"
#include "table/BudgetSegmentTable.h"

struct BudgetSegmentData
{
    int64    m_id;
    int64    m_period_id;
    wxString m_name;
    int      m_start_day;
    int      m_end_day;
    int      m_sort_order;
    bool     m_active;

    explicit BudgetSegmentData();
    explicit BudgetSegmentData(wxSQLite3ResultSet& q);
    BudgetSegmentData(const BudgetSegmentData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    BudgetSegmentRow to_row() const;
    BudgetSegmentData& from_row(const BudgetSegmentRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    BudgetSegmentData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    BudgetSegmentData& clone_from(const BudgetSegmentData& other);
    bool equals(const BudgetSegmentData* other) const;
    bool operator< (const BudgetSegmentData& other) const { return id() < other.id(); }
    bool operator< (const BudgetSegmentData* other) const { return id() < other->id(); }

    // Clamp the configured window to a concrete month.
    // ENDDAY 31 (or any day past the end of the month) means "end of month",
    // which keeps a 16..31 segment correct in February.
    int start_day_in_month(int days_in_month) const {
        int d = m_start_day < 1 ? 1 : m_start_day;
        return d > days_in_month ? days_in_month : d;
    }
    int end_day_in_month(int days_in_month) const {
        int d = m_end_day < 1 ? days_in_month : m_end_day;
        return d > days_in_month ? days_in_month : d;
    }
    // Number of days covered; never negative even if the user inverts the days.
    int day_count(int days_in_month) const {
        const int lo = start_day_in_month(days_in_month);
        const int hi = end_day_in_month(days_in_month);
        return hi >= lo ? (hi - lo + 1) : 0;
    }
    bool contains_day(int day, int days_in_month) const {
        return day >= start_day_in_month(days_in_month) &&
               day <= end_day_in_month(days_in_month);
    }

    struct SorterBySEGMENTID
    {
        bool operator()(const BudgetSegmentData& x, const BudgetSegmentData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterBySORTORDER
    {
        bool operator()(const BudgetSegmentData& x, const BudgetSegmentData& y)
        {
            if (x.m_sort_order != y.m_sort_order)
                return x.m_sort_order < y.m_sort_order;
            return x.m_start_day < y.m_start_day;
        }
    };

    struct SorterBySEGMENTNAME
    {
        bool operator()(const BudgetSegmentData& x, const BudgetSegmentData& y)
        {
            return x.m_name < y.m_name;
        }
    };
};

inline BudgetSegmentData::BudgetSegmentData(wxSQLite3ResultSet& q) :
    BudgetSegmentData()
{
    from_select_result(q);
}

inline void BudgetSegmentData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void BudgetSegmentData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline BudgetSegmentData& BudgetSegmentData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(BudgetSegmentRow().from_select_result(q));
}

inline wxString BudgetSegmentData::to_json() const
{
    return to_row().to_json();
}

inline void BudgetSegmentData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t BudgetSegmentData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void BudgetSegmentData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline BudgetSegmentData& BudgetSegmentData::clone_from(const BudgetSegmentData& other)
{
    *this = other;
    id(-1);
    return *this;
}
