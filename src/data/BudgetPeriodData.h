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

#pragma once

#include "table/BudgetPeriodTable.h"

// User-friendly representation of a record in table BUDGETYEAR_V1.
struct BudgetPeriodData
{
    int64    m_id;
    wxString m_name;

    explicit BudgetPeriodData();
    explicit BudgetPeriodData(wxSQLite3ResultSet& q);
    BudgetPeriodData(const BudgetPeriodData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    BudgetPeriodRow to_row() const;
    BudgetPeriodData& from_row(const BudgetPeriodRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    BudgetPeriodData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    BudgetPeriodData& clone_from(const BudgetPeriodData& other);
    bool equals(const BudgetPeriodData* other) const;
    bool operator< (const BudgetPeriodData& other) const { return id() < other.id(); }
    bool operator< (const BudgetPeriodData* other) const { return id() < other->id(); }

    struct SorterByBUDGETYEARID
    {
        bool operator()(const BudgetPeriodData& x, const BudgetPeriodData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByBUDGETYEARNAME
    {
        bool operator()(const BudgetPeriodData& x, const BudgetPeriodData& y)
        {
            return x.m_name < y.m_name;
        }
    };
};

inline BudgetPeriodData::BudgetPeriodData(wxSQLite3ResultSet& q) :
    BudgetPeriodData()
{
    from_select_result(q);
}

inline void BudgetPeriodData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void BudgetPeriodData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline BudgetPeriodData& BudgetPeriodData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(BudgetPeriodRow().from_select_result(q));
}

inline wxString BudgetPeriodData::to_json() const
{
    return to_row().to_json();
}

inline void BudgetPeriodData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t BudgetPeriodData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void BudgetPeriodData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline BudgetPeriodData& BudgetPeriodData::clone_from(const BudgetPeriodData& other)
{
    *this = other;
    id(-1);
    return *this;
}
