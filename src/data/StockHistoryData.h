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

#include "util/mmDate.h"
#include "_DataEnum.h"
#include "table/_TableBase.h"
#include "table/StockHistoryTable.h"

// User-friendly representation of a record in table STOCKHISTORY_V1.
struct StockHistoryData
{
    int64      m_id;
    wxString   m_symbol;
    mmDate     m_date;        // non-null
    double     m_price;
    UpdateType m_update_type; // the numeric value instead of the name is written in database

    explicit StockHistoryData();
    explicit StockHistoryData(wxSQLite3ResultSet& q);
    StockHistoryData(const StockHistoryData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    StockHistoryRow to_row() const;
    StockHistoryData& from_row(const StockHistoryRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    StockHistoryData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    StockHistoryData& clone_from(const StockHistoryData& other);
    bool equals(const StockHistoryData* other) const;
    bool operator< (const StockHistoryData& other) const { return id() < other.id(); }
    bool operator< (const StockHistoryData* other) const { return id() < other->id(); }

    struct SorterByHISTID
    {
        bool operator()(const StockHistoryData& x, const StockHistoryData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterBySYMBOL
    {
        bool operator()(const StockHistoryData& x, const StockHistoryData& y)
        {
            return x.m_symbol < y.m_symbol;
        }
    };

    struct SorterByDATE
    {
        bool operator()(const StockHistoryData& x, const StockHistoryData& y)
        {
            return x.m_date < y.m_date;
        }
    };

    struct SorterByVALUE
    {
        bool operator()(const StockHistoryData& x, const StockHistoryData& y)
        {
            return x.m_price < y.m_price;
        }
    };

    struct SorterByUPDTYPE
    {
        bool operator()(const StockHistoryData& x, const StockHistoryData& y)
        {
            return x.m_update_type.id() < y.m_update_type.id();
        }
    };
};

inline StockHistoryData::StockHistoryData(wxSQLite3ResultSet& q) :
    StockHistoryData()
{
    from_select_result(q);
}

inline void StockHistoryData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void StockHistoryData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline StockHistoryData& StockHistoryData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(StockHistoryRow().from_select_result(q));
}

inline wxString StockHistoryData::to_json() const
{
    return to_row().to_json();
}

inline void StockHistoryData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t StockHistoryData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void StockHistoryData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline StockHistoryData& StockHistoryData::clone_from(const StockHistoryData& other)
{
    *this = other;
    id(-1);
    return *this;
}
