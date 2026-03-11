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
#include "table/TrxTable.h"

// User-friendly representation of a record in table CHECKINGACCOUNT_V1.
struct TrxData
{
    int64     m_id;
    wxString  TRANSDATE;
    TrxType   m_type;
    TrxStatus m_status;
    int64     m_account_id;      // non-null (> 0) after initialization
    int64     m_to_account_id_n; // optional (can be null)
    int64     m_payee_id_n;      // optional (can be null)
    int64     m_category_id_n;   // optional (can be null)
    double    m_amount;
    double    m_to_amount;
    wxString  m_number;
    wxString  m_notes;
    int64     m_followup_id;     // this is not a database id
    int64     m_color;
    wxString  LASTUPDATEDTIME;
    wxString  DELETEDTIME;

    explicit TrxData();
    explicit TrxData(wxSQLite3ResultSet& q);
    TrxData(const TrxData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    TrxRow to_row() const;
    TrxData& from_row(const TrxRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    TrxData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    TrxData& clone_from(const TrxData& other);
    bool equals(const TrxData* other) const;
    bool operator< (const TrxData& other) const { return id() < other.id(); }
    bool operator< (const TrxData* other) const { return id() < other->id(); }

    bool is_withdrawal() const { return m_type.id() == TrxType::e_withdrawal; }
    bool is_deposit()    const { return m_type.id() == TrxType::e_deposit; }
    bool is_transfer()   const { return m_type.id() == TrxType::e_transfer; }
    bool is_reconciled() const { return m_status.id() == TrxStatus::e_reconciled; }
    bool is_void()       const { return m_status.id() == TrxStatus::e_void; }

    struct SorterByTRANSID
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByACCOUNTID
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.m_account_id < y.m_account_id;
        }
    };

    struct SorterByTOACCOUNTID
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.m_to_account_id_n < y.m_to_account_id_n;
        }
    };

    struct SorterByPAYEEID
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.m_payee_id_n < y.m_payee_id_n;
        }
    };

    struct SorterByTRANSCODE
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.m_type.id() < y.m_type.id();
        }
    };

    struct SorterByTRANSAMOUNT
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.m_amount < y.m_amount;
        }
    };

    struct SorterBySTATUS
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.m_status.id() < y.m_status.id();
        }
    };

    struct SorterByTRANSACTIONNUMBER
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.m_number < y.m_number;
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.m_notes < y.m_notes;
        }
    };

    struct SorterByCATEGID
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.m_category_id_n < y.m_category_id_n;
        }
    };

    struct SorterByTRANSDATE
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.TRANSDATE < y.TRANSDATE;
        }
    };

    struct SorterByLASTUPDATEDTIME
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.LASTUPDATEDTIME < y.LASTUPDATEDTIME;
        }
    };

    struct SorterByDELETEDTIME
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.DELETEDTIME < y.DELETEDTIME;
        }
    };

    struct SorterByFOLLOWUPID
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.m_followup_id < y.m_followup_id;
        }
    };

    struct SorterByTOTRANSAMOUNT
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.m_to_amount < y.m_to_amount;
        }
    };

    struct SorterByCOLOR
    {
        bool operator()(const TrxData& x, const TrxData& y)
        {
            return x.m_color < y.m_color;
        }
    };
};

inline TrxData::TrxData(wxSQLite3ResultSet& q) :
    TrxData()
{
    from_select_result(q);
}

inline void TrxData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void TrxData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline TrxData& TrxData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(TrxRow().from_select_result(q));
}

inline wxString TrxData::to_json() const
{
    return to_row().to_json();
}

inline void TrxData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t TrxData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void TrxData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline TrxData& TrxData::clone_from(const TrxData& other)
{
    *this = other;
    id(-1);
    return *this;
}
