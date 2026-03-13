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
#include "table/CurrencyTable.h"

// User-friendly representation of a record in table CURRENCYFORMATS_V1.
struct CurrencyData
{
    int64        m_id;
    wxString     m_symbol;
    wxString     m_name;
    CurrencyType m_type;
    wxString     m_prefix_symbol;
    wxString     m_suffix_symbol;
    wxString     m_decimal_point;
    wxString     m_group_separator;
    wxString     m_unit_name;
    wxString     m_cent_name;
    int64        m_scale;
    double       m_base_conv_rate;

    explicit CurrencyData();
    explicit CurrencyData(wxSQLite3ResultSet& q);
    CurrencyData(const CurrencyData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    CurrencyRow to_row() const;
    CurrencyData& from_row(const CurrencyRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    CurrencyData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    CurrencyData& clone_from(const CurrencyData& other);
    bool equals(const CurrencyData* other) const;
    bool operator< (const CurrencyData& other) const { return id() < other.id(); }
    bool operator< (const CurrencyData* other) const { return id() < other->id(); }

    int precision() const {
        return static_cast<int>(log10(static_cast<double>(m_scale.GetValue())));
    }

    struct SorterByCURRENCYID
    {
        bool operator()(const CurrencyData& x, const CurrencyData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByCURRENCYNAME
    {
        bool operator()(const CurrencyData& x, const CurrencyData& y)
        {
            return wxGetTranslation(x.m_name) < wxGetTranslation(y.m_name);
        }
    };

    struct SorterByPFX_SYMBOL
    {
        bool operator()(const CurrencyData& x, const CurrencyData& y)
        {
            return x.m_prefix_symbol < y.m_prefix_symbol;
        }
    };

    struct SorterBySFX_SYMBOL
    {
        bool operator()(const CurrencyData& x, const CurrencyData& y)
        {
            return x.m_suffix_symbol < y.m_suffix_symbol;
        }
    };

    struct SorterByDECIMAL_POINT
    {
        bool operator()(const CurrencyData& x, const CurrencyData& y)
        {
            return x.m_decimal_point < y.m_decimal_point;
        }
    };

    struct SorterByGROUP_SEPARATOR
    {
        bool operator()(const CurrencyData& x, const CurrencyData& y)
        {
            return x.m_group_separator < y.m_group_separator;
        }
    };

    struct SorterByUNIT_NAME
    {
        bool operator()(const CurrencyData& x, const CurrencyData& y)
        {
            return x.m_unit_name < y.m_unit_name;
        }
    };

    struct SorterByCENT_NAME
    {
        bool operator()(const CurrencyData& x, const CurrencyData& y)
        {
            return x.m_cent_name < y.m_cent_name;
        }
    };

    struct SorterBySCALE
    {
        bool operator()(const CurrencyData& x, const CurrencyData& y)
        {
            return x.m_scale < y.m_scale;
        }
    };

    struct SorterByBASECONVRATE
    {
        bool operator()(const CurrencyData& x, const CurrencyData& y)
        {
            return x.m_base_conv_rate < y.m_base_conv_rate;
        }
    };

    struct SorterByCURRENCY_SYMBOL
    {
        bool operator()(const CurrencyData& x, const CurrencyData& y)
        {
            return x.m_symbol < y.m_symbol;
        }
    };

    struct SorterByCURRENCY_TYPE
    {
        bool operator()(const CurrencyData& x, const CurrencyData& y)
        {
            return x.m_type.id() < y.m_type.id();
        }
    };
};

inline CurrencyData::CurrencyData(wxSQLite3ResultSet& q) :
    CurrencyData()
{
    from_select_result(q);
}

inline void CurrencyData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void CurrencyData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline CurrencyData& CurrencyData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(CurrencyRow().from_select_result(q));
}

inline wxString CurrencyData::to_json() const
{
    return to_row().to_json();
}

inline void CurrencyData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t CurrencyData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void CurrencyData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline CurrencyData& CurrencyData::clone_from(const CurrencyData& other)
{
    *this = other;
    id(-1);
    return *this;
}
