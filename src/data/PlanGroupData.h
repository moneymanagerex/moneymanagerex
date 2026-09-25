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

// PlanGroupData represents a record in table PLANGROUP_V1.
//
// A plan group collects related plan items so that a project can be planned and
// totalled as a unit -- e.g. a trip with hotels, car rental and food, or a
// collection of intended purchases. Groups may nest via m_parent_id.

#include "_DataEnum.h"
#include "table/PlanGroupTable.h"

struct PlanGroupData
{
    int64      m_id;
    int64      m_parent_id;
    wxString   m_name;
    wxString   m_notes;
    PlanStatus m_status;
    wxString   m_target_date;
    int        m_sort_order;
    bool       m_active;

    explicit PlanGroupData();
    explicit PlanGroupData(wxSQLite3ResultSet& q);
    PlanGroupData(const PlanGroupData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    PlanGroupRow to_row() const;
    PlanGroupData& from_row(const PlanGroupRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    PlanGroupData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    PlanGroupData& clone_from(const PlanGroupData& other);
    bool equals(const PlanGroupData* other) const;
    bool operator< (const PlanGroupData& other) const { return id() < other.id(); }
    bool operator< (const PlanGroupData* other) const { return id() < other->id(); }

    bool is_root() const { return m_parent_id <= 0; }

    struct SorterByGROUPID
    {
        bool operator()(const PlanGroupData& x, const PlanGroupData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterBySORTORDER
    {
        bool operator()(const PlanGroupData& x, const PlanGroupData& y)
        {
            if (x.m_sort_order != y.m_sort_order)
                return x.m_sort_order < y.m_sort_order;
            return x.m_name < y.m_name;
        }
    };

    struct SorterByGROUPNAME
    {
        bool operator()(const PlanGroupData& x, const PlanGroupData& y)
        {
            return x.m_name < y.m_name;
        }
    };
};

inline PlanGroupData::PlanGroupData(wxSQLite3ResultSet& q) :
    PlanGroupData()
{
    from_select_result(q);
}

inline void PlanGroupData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void PlanGroupData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline PlanGroupData& PlanGroupData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(PlanGroupRow().from_select_result(q));
}

inline wxString PlanGroupData::to_json() const
{
    return to_row().to_json();
}

inline void PlanGroupData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t PlanGroupData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void PlanGroupData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline PlanGroupData& PlanGroupData::clone_from(const PlanGroupData& other)
{
    *this = other;
    id(-1);
    return *this;
}
