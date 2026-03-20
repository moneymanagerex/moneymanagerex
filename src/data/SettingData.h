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

#include "table/SettingTable.h"

// User-friendly representation of a record in table SETTING_V1.
struct SettingData
{
    int64    m_id;
    wxString m_name;
    wxString m_value;

    explicit SettingData();
    explicit SettingData(wxSQLite3ResultSet& q);
    SettingData(const SettingData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    SettingRow to_row() const;
    SettingData& from_row(const SettingRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    SettingData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    SettingData& clone_from(const SettingData& other);
    bool equals(const SettingData* other) const;
    bool operator< (const SettingData& other) const { return id() < other.id(); }
    bool operator< (const SettingData* other) const { return id() < other->id(); }

    struct SorterBySETTINGID
    {
        bool operator()(const SettingData& x, const SettingData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterBySETTINGNAME
    {
        bool operator()(const SettingData& x, const SettingData& y)
        {
            return x.m_name < y.m_name;
        }
    };

    struct SorterBySETTINGVALUE
    {
        bool operator()(const SettingData& x, const SettingData& y)
        {
            return x.m_value < y.m_value;
        }
    };
};

inline SettingData::SettingData(wxSQLite3ResultSet& q) :
    SettingData()
{
    from_select_result(q);
}

inline void SettingData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void SettingData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline SettingData& SettingData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(SettingRow().from_select_result(q));
}

inline wxString SettingData::to_json() const
{
    return to_row().to_json();
}

inline void SettingData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t SettingData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void SettingData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline SettingData& SettingData::clone_from(const SettingData& other)
{
    *this = other;
    id(-1);
    return *this;
}
