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
#include "table/TrxLinkTable.h"

// User-friendly representation of a record in table TRANSLINK_V1.
struct TrxLinkData
{
    int64 m_id;
    int64 m_trx_id;      // non-null (> 0) after initialization
    RefTypeN m_ref_type; // one of [e_asset, e_stock] after initialization
    int64 m_ref_id;      // non-null (> 0) after initialization

    explicit TrxLinkData();
    explicit TrxLinkData(wxSQLite3ResultSet& q);
    TrxLinkData(const TrxLinkData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    TrxLinkRow to_row() const;
    TrxLinkData& from_row(const TrxLinkRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    TrxLinkData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    TrxLinkData& clone_from(const TrxLinkData& other);
    bool equals(const TrxLinkData* other) const;
    bool operator< (const TrxLinkData& other) const { return id() < other.id(); }
    bool operator< (const TrxLinkData* other) const { return id() < other->id(); }

    struct SorterByTRANSLINKID
    {
        bool operator()(const TrxLinkData& x, const TrxLinkData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByCHECKINGACCOUNTID
    {
        bool operator()(const TrxLinkData& x, const TrxLinkData& y)
        {
            return x.m_trx_id < y.m_trx_id;
        }
    };

    struct SorterByLINKTYPE
    {
        bool operator()(const TrxLinkData& x, const TrxLinkData& y)
        {
            return x.m_ref_type.id_n() < y.m_ref_type.id_n();
        }
    };

    struct SorterByLINKRECORDID
    {
        bool operator()(const TrxLinkData& x, const TrxLinkData& y)
        {
            return x.m_ref_id < y.m_ref_id;
        }
    };
};

inline TrxLinkData::TrxLinkData(wxSQLite3ResultSet& q) :
    TrxLinkData()
{
    from_select_result(q);
}

inline void TrxLinkData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void TrxLinkData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline TrxLinkData& TrxLinkData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(TrxLinkRow().from_select_result(q));
}

inline wxString TrxLinkData::to_json() const
{
    return to_row().to_json();
}

inline void TrxLinkData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t TrxLinkData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void TrxLinkData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline TrxLinkData& TrxLinkData::clone_from(const TrxLinkData& other)
{
    *this = other;
    id(-1);
    return *this;
}
