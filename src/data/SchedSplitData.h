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

#include "table/_TableBase.h"
#include "table/SchedSplitTable.h"

// User-friendly representation of a record in table BUDGETSPLITTRANSACTIONS_V1.
struct SchedSplitData
{
    int64    m_id;
    int64    m_sched_id;    // non-null (> 0) after initialization
    int64    m_category_id; // non-null (> 0) after initialization
    double   m_amount;
    wxString m_notes;

    explicit SchedSplitData();
    explicit SchedSplitData(wxSQLite3ResultSet& q);
    SchedSplitData(const SchedSplitData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    SchedSplitRow to_row() const;
    SchedSplitData& from_row(const SchedSplitRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    SchedSplitData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    SchedSplitData& clone_from(const SchedSplitData& other);
    bool equals(const SchedSplitData* other) const;
    bool operator< (const SchedSplitData& other) const { return id() < other.id(); }
    bool operator< (const SchedSplitData* other) const { return id() < other->id(); }

    struct SorterBySPLITTRANSID
    {
        bool operator()(const SchedSplitData& x, const SchedSplitData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByTRANSID
    {
        bool operator()(const SchedSplitData& x, const SchedSplitData& y)
        {
            return x.m_sched_id < y.m_sched_id;
        }
    };

    struct SorterByCATEGID
    {
        bool operator()(const SchedSplitData& x, const SchedSplitData& y)
        {
            return x.m_category_id < y.m_category_id;
        }
    };

    struct SorterBySPLITTRANSAMOUNT
    {
        bool operator()(const SchedSplitData& x, const SchedSplitData& y)
        {
            return x.m_amount < y.m_amount;
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const SchedSplitData& x, const SchedSplitData& y)
        {
            return x.m_notes < y.m_notes;
        }
    };
};

inline SchedSplitData::SchedSplitData(wxSQLite3ResultSet& q) :
    SchedSplitData()
{
    from_select_result(q);
}

inline void SchedSplitData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void SchedSplitData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline SchedSplitData& SchedSplitData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(SchedSplitRow().from_select_result(q));
}

inline wxString SchedSplitData::to_json() const
{
    return to_row().to_json();
}

inline void SchedSplitData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t SchedSplitData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void SchedSplitData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline SchedSplitData& SchedSplitData::clone_from(const SchedSplitData& other)
{
    *this = other;
    id(-1);
    return *this;
}
