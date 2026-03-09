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
#include "table/_TableBase.h"
#include "table/StockTable.h"

// User-friendly representation of a record in table STOCK_V1.
struct StockData
{
    int64    m_id;
    int64    m_account_id_n;   // optional (can be null)
    wxString m_name;
    wxString m_symbol;
    double   m_num_shares;
    mmDate   m_purchase_date;  // non-null
    double   m_purchase_price;
    double   m_current_price;
    double   m_purchase_value;
    double   m_commission;
    wxString m_notes;

    explicit StockData();
    explicit StockData(wxSQLite3ResultSet& q);
    StockData(const StockData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    StockRow to_row() const;
    StockData& from_row(const StockRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    StockData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    StockData& clone_from(const StockData& other);
    bool equals(const StockData* other) const;
    bool operator< (const StockData& other) const { return id() < other.id(); }
    bool operator< (const StockData* other) const { return id() < other->id(); }

    double current_value() const { return m_num_shares * m_current_price; }

    struct SorterBySTOCKID
    {
        bool operator()(const StockData& x, const StockData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByHELDAT
    {
        bool operator()(const StockData& x, const StockData& y)
        {
            return x.m_account_id_n < y.m_account_id_n;
        }
    };

    struct SorterByPURCHASEDATE
    {
        bool operator()(const StockData& x, const StockData& y)
        {
            return x.m_purchase_date < y.m_purchase_date;
        }
    };

    struct SorterBySTOCKNAME
    {
        bool operator()(const StockData& x, const StockData& y)
        {
            return x.m_name < y.m_name;
        }
    };

    struct SorterBySYMBOL
    {
        bool operator()(const StockData& x, const StockData& y)
        {
            return x.m_symbol < y.m_symbol;
        }
    };

    struct SorterByNUMSHARES
    {
        bool operator()(const StockData& x, const StockData& y)
        {
            return x.m_num_shares < y.m_num_shares;
        }
    };

    struct SorterByPURCHASEPRICE
    {
        bool operator()(const StockData& x, const StockData& y)
        {
            return x.m_purchase_price < y.m_purchase_price;
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const StockData& x, const StockData& y)
        {
            return x.m_notes < y.m_notes;
        }
    };

    struct SorterByCURRENTPRICE
    {
        bool operator()(const StockData& x, const StockData& y)
        {
            return x.m_current_price < y.m_current_price;
        }
    };

    struct SorterByVALUE
    {
        bool operator()(const StockData& x, const StockData& y)
        {
            return x.m_purchase_value < y.m_purchase_value;
        }
    };

    struct SorterByCOMMISSION
    {
        bool operator()(const StockData& x, const StockData& y)
        {
            return x.m_commission < y.m_commission;
        }
    };
};

inline StockData::StockData(wxSQLite3ResultSet& q) :
    StockData()
{
    from_select_result(q);
}

inline void StockData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void StockData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline StockData& StockData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(StockRow().from_select_result(q));
}

inline wxString StockData::to_json() const
{
    return to_row().to_json();
}

inline void StockData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t StockData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void StockData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline StockData& StockData::clone_from(const StockData& other)
{
    *this = other;
    id(-1);
    return *this;
}
