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

#include "table/_TableBase.h"
#include "table/TrxSplitTable.h"

// User-friendly representation of a record in table SPLITTRANSACTIONS_V1.
struct TrxSplitData
{
    int64    m_id;
    int64    m_trx_id;      // non-null (> 0) after initialization
    int64    m_category_id; // non-null (> 0) after initialization
    double   m_amount;
    wxString m_notes;

    explicit TrxSplitData();
    explicit TrxSplitData(wxSQLite3ResultSet& q);
    TrxSplitData(const TrxSplitData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    TrxSplitRow to_row() const;
    TrxSplitData& from_row(const TrxSplitRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    TrxSplitData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    TrxSplitData& clone_from(const TrxSplitData& other);
    bool equals(const TrxSplitData* other) const;
    bool operator< (const TrxSplitData& other) const { return id() < other.id(); }
    bool operator< (const TrxSplitData* other) const { return id() < other->id(); }

    struct SorterBySPLITTRANSID
    {
        bool operator()(const TrxSplitData& x, const TrxSplitData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByTRANSID
    {
        bool operator()(const TrxSplitData& x, const TrxSplitData& y)
        {
            return x.m_trx_id < y.m_trx_id;
        }
    };

    struct SorterByCATEGID
    {
        bool operator()(const TrxSplitData& x, const TrxSplitData& y)
        {
            return x.m_category_id < y.m_category_id;
        }
    };

    struct SorterBySPLITTRANSAMOUNT
    {
        bool operator()(const TrxSplitData& x, const TrxSplitData& y)
        {
            return x.m_amount < y.m_amount;
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const TrxSplitData& x, const TrxSplitData& y)
        {
            return x.m_notes < y.m_notes;
        }
    };
};

inline TrxSplitData::TrxSplitData(wxSQLite3ResultSet& q) :
    TrxSplitData()
{
    from_select_result(q);
}

inline void TrxSplitData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void TrxSplitData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline TrxSplitData& TrxSplitData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(TrxSplitRow().from_select_result(q));
}

inline wxString TrxSplitData::to_json() const
{
    return to_row().to_json();
}

inline void TrxSplitData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t TrxSplitData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void TrxSplitData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline TrxSplitData& TrxSplitData::clone_from(const TrxSplitData& other)
{
    *this = other;
    id(-1);
    return *this;
}
