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
#include "table/FieldTable.h"

// User-friendly representation of a record in table CUSTOMFIELD_V1.
struct FieldData
{
    int64      m_id;
    RefTypeN   m_ref_type;    // one of [e_trx, e_sched] after initialization
    wxString   m_description;
    FieldTypeN m_type_n;      // can be null
    wxString   m_properties;

    explicit FieldData();
    explicit FieldData(wxSQLite3ResultSet& q);
    FieldData(const FieldData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    FieldRow to_row() const;
    FieldData& from_row(const FieldRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    FieldData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    FieldData& clone_from(const FieldData& other);
    bool equals(const FieldData* other) const;
    bool operator< (const FieldData& other) const { return id() < other.id(); }
    bool operator< (const FieldData* other) const { return id() < other->id(); }

    struct SorterByFIELDID
    {
        bool operator()(const FieldData& x, const FieldData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByREFTYPE
    {
        bool operator()(const FieldData& x, const FieldData& y)
        {
            return x.m_ref_type.id_n() < y.m_ref_type.id_n();
        }
    };

    struct SorterByDESCRIPTION
    {
        bool operator()(const FieldData& x, const FieldData& y)
        {
            return x.m_description < y.m_description;
        }
    };

    struct SorterByTYPE
    {
        bool operator()(const FieldData& x, const FieldData& y)
        {
            return x.m_type_n.id_n() < y.m_type_n.id_n();
        }
    };

    struct SorterByPROPERTIES
    {
        bool operator()(const FieldData& x, const FieldData& y)
        {
            return x.m_properties < y.m_properties;
        }
    };
};

inline FieldData::FieldData(wxSQLite3ResultSet& q) :
    FieldData()
{
    from_select_result(q);
}

inline void FieldData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void FieldData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline FieldData& FieldData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(FieldRow().from_select_result(q));
}

inline wxString FieldData::to_json() const
{
    return to_row().to_json();
}

inline void FieldData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t FieldData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void FieldData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline FieldData& FieldData::clone_from(const FieldData& other)
{
    *this = other;
    id(-1);
    return *this;
}
