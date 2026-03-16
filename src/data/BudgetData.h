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

#include "_DataEnum.h"
#include "table/_TableBase.h"
#include "table/BudgetTable.h"

// User-friendly representation of a record in table BUDGETTABLE_V1.
struct BudgetData
{
    int64      m_id;
    int64      m_period_id;
    int64      m_category_id;
    BudgetFreq m_freq;
    double     m_amount;
    wxString   m_notes;
    bool       m_active;

    explicit BudgetData();
    explicit BudgetData(wxSQLite3ResultSet& q);
    BudgetData(const BudgetData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    BudgetRow to_row() const;
    BudgetData& from_row(const BudgetRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    BudgetData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    BudgetData& clone_from(const BudgetData& other);
    bool equals(const BudgetData* other) const;
    bool operator< (const BudgetData& other) const { return id() < other.id(); }
    bool operator< (const BudgetData* other) const { return id() < other->id(); }

    double amount_per_year() const {
        return m_amount * m_freq.times_per_year();
    }
    double amount_per_month() const {
        return m_amount * m_freq.times_per_month();
    }

    struct SorterByBUDGETENTRYID
    {
        bool operator()(const BudgetData& x, const BudgetData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByBUDGETYEARID
    {
        bool operator()(const BudgetData& x, const BudgetData& y)
        {
            return x.m_period_id < y.m_period_id;
        }
    };

    struct SorterByCATEGID
    {
        bool operator()(const BudgetData& x, const BudgetData& y)
        {
            return x.m_category_id < y.m_category_id;
        }
    };

    struct SorterByPERIOD
    {
        bool operator()(const BudgetData& x, const BudgetData& y)
        {
            return x.m_freq.id() < y.m_freq.id();
        }
    };

    struct SorterByAMOUNT
    {
        bool operator()(const BudgetData& x, const BudgetData& y)
        {
            return x.m_amount < y.m_amount;
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const BudgetData& x, const BudgetData& y)
        {
            return x.m_notes < y.m_notes;
        }
    };

    struct SorterByACTIVE
    {
        bool operator()(const BudgetData& x, const BudgetData& y)
        {
            return (x.m_active ? 1 : 0) < (y.m_active ? 1 : 0);
        }
    };
};

inline BudgetData::BudgetData(wxSQLite3ResultSet& q) :
    BudgetData()
{
    from_select_result(q);
}

inline void BudgetData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void BudgetData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline BudgetData& BudgetData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(BudgetRow().from_select_result(q));
}

inline wxString BudgetData::to_json() const
{
    return to_row().to_json();
}

inline void BudgetData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t BudgetData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void BudgetData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline BudgetData& BudgetData::clone_from(const BudgetData& other)
{
    *this = other;
    id(-1);
    return *this;
}
