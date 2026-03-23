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

#include "table/TagTable.h"

// User-friendly representation of a record in table TAG_V1.
struct TagData
{
    int64    m_id;
    wxString m_name;
    bool     m_active;

    explicit TagData();
    explicit TagData(wxSQLite3ResultSet& q);
    TagData(const TagData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    TagRow to_row() const;
    TagData& from_row(const TagRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    TagData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    TagData& clone_from(const TagData& other);
    bool equals(const TagData* other) const;
    bool operator< (const TagData& other) const { return id() < other.id(); }
    bool operator< (const TagData* other) const { return id() < other->id(); }

    struct SorterByTAGID
    {
        bool operator()(const TagData& x, const TagData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByTAGNAME
    {
        bool operator()(const TagData& x, const TagData& y)
        {
            return x.m_name < y.m_name;
        }
    };

    struct SorterByACTIVE
    {
        bool operator()(const TagData& x, const TagData& y)
        {
            return (x.m_active ? 1 : 0) < (y.m_active ? 1 : 0);
        }
    };
};

inline TagData::TagData(wxSQLite3ResultSet& q) :
    TagData()
{
    from_select_result(q);
}

inline void TagData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void TagData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline TagData& TagData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(TagRow().from_select_result(q));
}

inline wxString TagData::to_json() const
{
    return to_row().to_json();
}

inline void TagData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t TagData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void TagData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline TagData& TagData::clone_from(const TagData& other)
{
    *this = other;
    id(-1);
    return *this;
}
