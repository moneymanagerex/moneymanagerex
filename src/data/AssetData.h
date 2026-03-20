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

#include "util/_primitive.h"
#include "util/mmDate.h"
#include "_DataEnum.h"
#include "table/AssetTable.h"

// User-friendly representation of a record in table ASSETS_V1.
struct AssetData
{
    int64           m_id;
    AssetType       m_type;
    AssetStatus     m_status;
    wxString        m_name;
    mmDate          m_start_date;    // non-null
    int64           m_currency_id_n; // -1 means base currency (no conversion)
    double          m_value;
    AssetChange     m_change;
    AssetChangeMode m_change_mode;
    double          m_change_rate;
    wxString        m_notes;

    explicit AssetData();
    explicit AssetData(wxSQLite3ResultSet& q);
    AssetData(const AssetData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    AssetRow to_row() const;
    AssetData& from_row(const AssetRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    AssetData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    AssetData& clone_from(const AssetData& other);
    bool equals(const AssetData* other) const;
    bool operator< (const AssetData& other) const { return id() < other.id(); }
    bool operator< (const AssetData* other) const { return id() < other->id(); }

    struct SorterByASSETID
    {
        bool operator()(const AssetData& x, const AssetData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterBySTARTDATE
    {
        bool operator()(const AssetData& x, const AssetData& y)
        {
            return x.m_start_date < y.m_start_date;
        }
    };

    struct SorterByASSETNAME
    {
        bool operator()(const AssetData& x, const AssetData& y)
        {
            return x.m_name < y.m_name;
        }
    };

    struct SorterByASSETSTATUS
    {
        bool operator()(const AssetData& x, const AssetData& y)
        {
            return x.m_status.id() < y.m_status.id();
        }
    };

    struct SorterByCURRENCYID
    {
        bool operator()(const AssetData& x, const AssetData& y)
        {
            return x.m_currency_id_n < y.m_currency_id_n;
        }
    };

    struct SorterByVALUECHANGEMODE
    {
        bool operator()(const AssetData& x, const AssetData& y)
        {
            return x.m_change_mode.id() < y.m_change_mode.id();
        }
    };

    struct SorterByVALUE
    {
        bool operator()(const AssetData& x, const AssetData& y)
        {
            return x.m_value < y.m_value;
        }
    };

    struct SorterByVALUECHANGE
    {
        bool operator()(const AssetData& x, const AssetData& y)
        {
            return x.m_change.id() < y.m_change.id();
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const AssetData& x, const AssetData& y)
        {
            return x.m_notes < y.m_notes;
        }
    };

    struct SorterByVALUECHANGERATE
    {
        bool operator()(const AssetData& x, const AssetData& y)
        {
            return x.m_change_rate < y.m_change_rate;
        }
    };

    struct SorterByASSETTYPE
    {
        bool operator()(const AssetData& x, const AssetData& y)
        {
            return x.m_type.id() < y.m_type.id();
        }
    };
};

inline AssetData::AssetData(wxSQLite3ResultSet& q) :
    AssetData()
{
    from_select_result(q);
}

inline void AssetData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void AssetData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline AssetData& AssetData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(AssetRow().from_select_result(q));
}

inline wxString AssetData::to_json() const
{
    return to_row().to_json();
}

inline void AssetData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t AssetData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void AssetData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline AssetData& AssetData::clone_from(const AssetData& other)
{
    *this = other;
    id(-1);
    return *this;
}
