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
#include "table/FieldValueTable.h"

// User-friendly representation of a record in table CUSTOMFIELDDATA_V1.
struct FieldValueData
{
    int64    m_id;
    int64    m_field_id; // non-null (> 0) after initialization
    RefTypeN m_ref_type; // one of [e_trx, e_sched] after initialization
    int64    m_ref_id;   // non-null (> 0) after initialization
    wxString m_content;

    explicit FieldValueData();
    explicit FieldValueData(wxSQLite3ResultSet& q);
    FieldValueData(const FieldValueData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    FieldValueRow to_row() const;
    FieldValueData& from_row(const FieldValueRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    FieldValueData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    FieldValueData& clone_from(const FieldValueData& other);
    bool equals(const FieldValueData* other) const;
    bool operator< (const FieldValueData& other) const { return id() < other.id(); }
    bool operator< (const FieldValueData* other) const { return id() < other->id(); }

    static RefTypeN decode_ref_type(int64 row_REFID) {
        return RefTypeN((row_REFID < 0) ? RefTypeN::e_sched : RefTypeN::e_trx);
    }
    static int64 decode_ref_id(int64 row_REFID) {
        return (row_REFID < 0) ? -row_REFID : row_REFID;
    }
    static int64 encode_REFID(RefTypeN ref_type, int64 ref_id) {
        return (ref_type.id_n() == RefTypeN::e_sched) ? -ref_id : ref_id;
    }

    struct SorterByFIELDATADID
    {
        bool operator()(const FieldValueData& x, const FieldValueData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByFIELDID
    {
        bool operator()(const FieldValueData& x, const FieldValueData& y)
        {
            return x.m_field_id < y.m_field_id;
        }
    };

    struct SorterByREFTYPE
    {
        bool operator()(const FieldValueData& x, const FieldValueData& y)
        {
            return x.m_ref_type.id_n() < y.m_ref_type.id_n();
        }
    };

    struct SorterByREFID
    {
        bool operator()(const FieldValueData& x, const FieldValueData& y)
        {
            return x.m_ref_id < y.m_ref_id;
        }
    };

    struct SorterByCONTENT
    {
        bool operator()(const FieldValueData& x, const FieldValueData& y)
        {
            return x.m_content < y.m_content;
        }
    };
};

inline FieldValueData::FieldValueData(wxSQLite3ResultSet& q) :
    FieldValueData()
{
    from_select_result(q);
}

inline void FieldValueData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void FieldValueData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline FieldValueData& FieldValueData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(FieldValueRow().from_select_result(q));
}

inline wxString FieldValueData::to_json() const
{
    return to_row().to_json();
}

inline void FieldValueData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t FieldValueData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void FieldValueData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline FieldValueData& FieldValueData::clone_from(const FieldValueData& other)
{
    *this = other;
    id(-1);
    return *this;
}
