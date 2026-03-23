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

#include "table/TrxShareTable.h"

// User-friendly representation of a record in table SHAREINFO_V1.
struct TrxShareData
{
    int64    m_id;
    int64    m_trx_id;     // non-null (> 0) after initialization
    double   m_number;
    double   m_price;
    double   m_commission;
    wxString m_lot;

    explicit TrxShareData();
    explicit TrxShareData(wxSQLite3ResultSet& q);
    TrxShareData(const TrxShareData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    TrxShareRow to_row() const;
    TrxShareData& from_row(const TrxShareRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    TrxShareData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    TrxShareData& clone_from(const TrxShareData& other);
    bool equals(const TrxShareData* other) const;
    bool operator< (const TrxShareData& other) const { return id() < other.id(); }
    bool operator< (const TrxShareData* other) const { return id() < other->id(); }

    struct SorterBySHAREINFOID
    {
        bool operator()(const TrxShareData& x, const TrxShareData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByCHECKINGACCOUNTID
    {
        bool operator()(const TrxShareData& x, const TrxShareData& y)
        {
            return x.m_trx_id < y.m_trx_id;
        }
    };

    struct SorterBySHARENUMBER
    {
        bool operator()(const TrxShareData& x, const TrxShareData& y)
        {
            return x.m_number < y.m_number;
        }
    };

    struct SorterBySHAREPRICE
    {
        bool operator()(const TrxShareData& x, const TrxShareData& y)
        {
            return x.m_price < y.m_price;
        }
    };

    struct SorterBySHARECOMMISSION
    {
        bool operator()(const TrxShareData& x, const TrxShareData& y)
        {
            return x.m_commission < y.m_commission;
        }
    };

    struct SorterBySHARELOT
    {
        bool operator()(const TrxShareData& x, const TrxShareData& y)
        {
            return x.m_lot < y.m_lot;
        }
    };
};

inline TrxShareData::TrxShareData(wxSQLite3ResultSet& q) :
    TrxShareData()
{
    from_select_result(q);
}

inline void TrxShareData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void TrxShareData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline TrxShareData& TrxShareData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(TrxShareRow().from_select_result(q));
}

inline wxString TrxShareData::to_json() const
{
    return to_row().to_json();
}

inline void TrxShareData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t TrxShareData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void TrxShareData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline TrxShareData& TrxShareData::clone_from(const TrxShareData& other)
{
    *this = other;
    id(-1);
    return *this;
}
