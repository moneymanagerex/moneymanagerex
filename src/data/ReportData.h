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

#include "table/ReportTable.h"

// User-friendly representation of a record in table REPORT_V1.
struct ReportData
{
    int64    m_id;
    wxString m_name;
    wxString m_group_name;
    bool     m_active;
    wxString m_sql_content;
    wxString m_lua_content;
    wxString m_template_content;
    wxString m_description;

    explicit ReportData();
    explicit ReportData(wxSQLite3ResultSet& q);
    ReportData(const ReportData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    ReportRow to_row() const;
    ReportData& from_row(const ReportRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    ReportData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    ReportData& clone_from(const ReportData& other);
    bool equals(const ReportData* other) const;
    bool operator< (const ReportData& other) const { return id() < other.id(); }
    bool operator< (const ReportData* other) const { return id() < other->id(); }

    struct SorterByREPORTID
    {
        bool operator()(const ReportData& x, const ReportData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByREPORTNAME
    {
        bool operator()(const ReportData& x, const ReportData& y)
        {
            return x.m_name < y.m_name;
        }
    };

    struct SorterByGROUPNAME
    {
        bool operator()(const ReportData& x, const ReportData& y)
        {
            return x.m_group_name < y.m_group_name;
        }
    };

    struct SorterByACTIVE
    {
        bool operator()(const ReportData& x, const ReportData& y)
        {
            return (x.m_active ? 1 : 0) < (y.m_active ? 1 : 0);
        }
    };

    struct SorterBySQLCONTENT
    {
        bool operator()(const ReportData& x, const ReportData& y)
        {
            return x.m_sql_content < y.m_sql_content;
        }
    };

    struct SorterByLUACONTENT
    {
        bool operator()(const ReportData& x, const ReportData& y)
        {
            return x.m_lua_content < y.m_lua_content;
        }
    };

    struct SorterByTEMPLATECONTENT
    {
        bool operator()(const ReportData& x, const ReportData& y)
        {
            return x.m_template_content < y.m_template_content;
        }
    };

    struct SorterByDESCRIPTION
    {
        bool operator()(const ReportData& x, const ReportData& y)
        {
            return x.m_description < y.m_description;
        }
    };
};

inline ReportData::ReportData(wxSQLite3ResultSet& q) :
    ReportData()
{
    from_select_result(q);
}

inline void ReportData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void ReportData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline ReportData& ReportData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(ReportRow().from_select_result(q));
}

inline wxString ReportData::to_json() const
{
    return to_row().to_json();
}

inline void ReportData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t ReportData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void ReportData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline ReportData& ReportData::clone_from(const ReportData& other)
{
    *this = other;
    id(-1);
    return *this;
}
