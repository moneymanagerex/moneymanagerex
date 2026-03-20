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

#include "table/CategoryTable.h"

// User-friendly representation of a record in table CATEGORY_V1.
struct CategoryData
{
    int64    m_id;
    wxString m_name;
    int64    m_parent_id_n; // -1 means there is no parent
    bool     m_active;

    explicit CategoryData();
    explicit CategoryData(wxSQLite3ResultSet& q);
    CategoryData(const CategoryData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    CategoryRow to_row() const;
    CategoryData& from_row(const CategoryRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    CategoryData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    CategoryData& clone_from(const CategoryData& other);
    bool equals(const CategoryData* other) const;
    bool operator< (const CategoryData& other) const { return id() < other.id(); }
    bool operator< (const CategoryData* other) const { return id() < other->id(); }

    struct SorterByCATEGID
    {
        bool operator()(const CategoryData& x, const CategoryData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByCATEGNAME
    {
        bool operator()(const CategoryData& x, const CategoryData& y)
        {
            // Locale case-insensitive
            return std::wcscoll(x.m_name.Lower().wc_str(), y.m_name.Lower().wc_str()) < 0;
        }
    };

    struct SorterByACTIVE
    {
        bool operator()(const CategoryData& x, const CategoryData& y)
        {
            return (x.m_active ? 1 : 0) < (y.m_active ? 1 : 0);
        }
    };

    struct SorterByPARENTID
    {
        bool operator()(const CategoryData& x, const CategoryData& y)
        {
            return x.m_parent_id_n < y.m_parent_id_n;
        }
    };
};

inline CategoryData::CategoryData(wxSQLite3ResultSet& q) :
    CategoryData()
{
    from_select_result(q);
}

inline void CategoryData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void CategoryData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline CategoryData& CategoryData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(CategoryRow().from_select_result(q));
}

inline wxString CategoryData::to_json() const
{
    return to_row().to_json();
}

inline void CategoryData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t CategoryData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void CategoryData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline CategoryData& CategoryData::clone_from(const CategoryData& other)
{
    *this = other;
    id(-1);
    return *this;
}
