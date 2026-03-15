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

#include "util/mmDateTime.h"
#include "util/mmDate.h"
#include "_DataEnum.h"
#include "table/_TableBase.h"
#include "table/SchedTable.h"

// User-friendly representation of a record in table BILLSDEPOSITS_V1.
struct SchedData
{
    int64      m_id;
    mmDateTime m_date_time;
    TrxType    m_type;
    TrxStatus  m_status;
    int64      m_account_id;      // non-null (> 0) after initialization
    int64      m_to_account_id_n; // optional (can be null)
    int64      m_payee_id_n;      // optional (can be null)
    int64      m_category_id_n;   // optional (can be null)
    double     m_amount;
    double     m_to_amount;
    wxString   m_number;
    wxString   m_notes;
    int64      m_followup_id;
    int64      m_color;
    mmDate     m_due_date;
    int64      REPEATS;
    int64      NUMOCCURRENCES;

    explicit SchedData();
    explicit SchedData(wxSQLite3ResultSet& q);
    SchedData(const SchedData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    SchedRow to_row() const;
    SchedData& from_row(const SchedRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    SchedData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    SchedData& clone_from(const SchedData& other);
    bool equals(const SchedData* other) const;
    bool operator< (const SchedData& other) const { return id() < other.id(); }
    bool operator< (const SchedData* other) const { return id() < other->id(); }

    // m_date is a pseudo-member variable, convenient when time is disabled.
    // note: the (unused) time part is set to noon in mmDate constructor and methods
    mmDate m_date() const { return mmDate(m_date_time); }
    void m_date(mmDate date) { m_date_time = mmDateTime(date.getDateTime()); }

    bool is_withdrawal() const { return m_type.id() == TrxType::e_withdrawal; }
    bool is_deposit()    const { return m_type.id() == TrxType::e_deposit; }
    bool is_transfer()   const { return m_type.id() == TrxType::e_transfer; }
    bool is_reconciled() const { return m_status.id() == TrxStatus::e_reconciled; }
    bool is_void()       const { return m_status.id() == TrxStatus::e_void; }

    struct SorterById
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByDateTime
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.m_date_time < y.m_date_time;
        }
    };

    struct SorterByType
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.m_type.id() < y.m_type.id();
        }
    };

    struct SorterByStatus
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.m_status.id() < y.m_status.id();
        }
    };

    struct SorterByAccountId
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.m_account_id < y.m_account_id;
        }
    };

    struct SorterByToAccountId
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.m_to_account_id_n < y.m_to_account_id_n;
        }
    };

    struct SorterByPayeeId
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.m_payee_id_n < y.m_payee_id_n;
        }
    };

    struct SorterByCategoryId
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.m_category_id_n < y.m_category_id_n;
        }
    };

    struct SorterByAmount
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.m_amount < y.m_amount;
        }
    };

    struct SorterByToAmount
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.m_to_amount < y.m_to_amount;
        }
    };

    struct SorterByNumber
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.m_number < y.m_number;
        }
    };

    struct SorterByNotes
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.m_notes < y.m_notes;
        }
    };

    struct SorterByFollowupId
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.m_followup_id < y.m_followup_id;
        }
    };

    struct SorterByColor
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.m_color < y.m_color;
        }
    };

    struct SorterByDueDate
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.m_due_date < y.m_due_date;
        }
    };

    struct SorterByREPEATS
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.REPEATS < y.REPEATS;
        }
    };

    struct SorterByNUMOCCURRENCES
    {
        bool operator()(const SchedData& x, const SchedData& y)
        {
            return x.NUMOCCURRENCES < y.NUMOCCURRENCES;
        }
    };
};

inline SchedData::SchedData(wxSQLite3ResultSet& q) :
    SchedData()
{
    from_select_result(q);
}

inline void SchedData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void SchedData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline SchedData& SchedData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(SchedRow().from_select_result(q));
}

inline wxString SchedData::to_json() const
{
    return to_row().to_json();
}

inline void SchedData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t SchedData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void SchedData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline SchedData& SchedData::clone_from(const SchedData& other)
{
    *this = other;
    id(-1);
    return *this;
}
