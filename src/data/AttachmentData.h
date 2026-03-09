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
#include "table/AttachmentTable.h"

// User-friendly representation of a record in table ATTACHMENT_V1.
struct AttachmentData
{
    int64    m_id;
    RefTypeN m_ref_type_n;  // non-null after initialization; null value is an error
    int64    m_ref_id;      // non-null (> 0) after initialization
    wxString m_description;
    wxString m_filename;

    explicit AttachmentData();
    explicit AttachmentData(wxSQLite3ResultSet& q);
    AttachmentData(const AttachmentData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    AttachmentRow to_row() const;
    AttachmentData& from_row(const AttachmentRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    AttachmentData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    AttachmentData& clone_from(const AttachmentData& other);
    bool equals(const AttachmentData* other) const;
    bool operator< (const AttachmentData& other) const { return id() < other.id(); }
    bool operator< (const AttachmentData* other) const { return id() < other->id(); }

    struct SorterByATTACHMENTID
    {
        bool operator()(const AttachmentData& x, const AttachmentData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByREFTYPE
    {
        bool operator()(const AttachmentData& x, const AttachmentData& y)
        {
            return x.m_ref_type_n.id_n() < y.m_ref_type_n.id_n();
        }
    };

    struct SorterByREFID
    {
        bool operator()(const AttachmentData& x, const AttachmentData& y)
        {
            return x.m_ref_id < y.m_ref_id;
        }
    };

    struct SorterByDESCRIPTION
    {
        bool operator()(const AttachmentData& x, const AttachmentData& y)
        {
            return x.m_description < y.m_description;
        }
    };

    struct SorterByFILENAME
    {
        bool operator()(const AttachmentData& x, const AttachmentData& y)
        {
            return x.m_filename < y.m_filename;
        }
    };
};

inline AttachmentData::AttachmentData(wxSQLite3ResultSet& q) :
    AttachmentData()
{
    from_select_result(q);
}

inline void AttachmentData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void AttachmentData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline AttachmentData& AttachmentData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(AttachmentRow().from_select_result(q));
}

inline wxString AttachmentData::to_json() const
{
    return to_row().to_json();
}

inline void AttachmentData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t AttachmentData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void AttachmentData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline AttachmentData& AttachmentData::clone_from(const AttachmentData& other)
{
    *this = other;
    id(-1);
    return *this;
}
