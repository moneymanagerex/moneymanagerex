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
#include "table/AccountTable.h"

// User-friendly representation of a record in table ACCOUNTLIST_V1.
struct AccountData
{
    int64           m_id;
    wxString        m_name;
    wxString        m_type_;              // TODO: restore account types
    int64           m_currency_id;        // non-null (> 0) after initialization
    AccountStatus   m_status;
    AccountFavorite m_favorite;
    wxString        m_num;
    wxString        m_notes;
    wxString        m_held_at;
    wxString        m_website;
    wxString        m_contact_info;
    wxString        m_access_info;
    mmDate          m_open_date;          // non-null
    double          m_open_balance;
    bool            m_stmt_locked;
    mmDateN         m_stmt_date_n;        // optional (can be null)
    double          m_min_balance;
    double          m_credit_limit;
    double          m_interest_rate;
    mmDateN         m_payment_due_date_n; // optional (can be null)
    double          m_min_payment;

    explicit AccountData();
    explicit AccountData(wxSQLite3ResultSet& q);
    AccountData(const AccountData& other) = default;

    int64 id() const { return m_id; }
    void id(const int64 id) { m_id = id; }
    AccountRow to_row() const;
    AccountData& from_row(const AccountRow& row);
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    AccountData& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    AccountData& clone_from(const AccountData& other);
    bool equals(const AccountData* other) const;
    bool operator< (const AccountData& other) const { return id() < other.id(); }
    bool operator< (const AccountData* other) const { return id() < other->id(); }

    bool is_open() const { return m_status.id() == AccountStatus::e_open; }
    bool is_closed() const { return !is_open(); }
    bool is_favorite() const { return m_favorite.id() == AccountFavorite::e_true; }
    bool is_locked_for(const mmDate& date) const {
        // the statement date is inclusive for lock, i.e., new transactions
        // are not allowed within the statement date or before the statement date
        return m_stmt_locked && m_stmt_date_n.has_value() && date <= m_stmt_date_n.value();
    }

    struct SorterById
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_id < y.m_id;
        }
    };

    struct SorterByName
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            // Locale case-insensitive
            return std::wcscoll(x.m_name.Lower().wc_str(), y.m_name.Lower().wc_str()) < 0;
        }
    };

    struct SorterByType
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_type_ < y.m_type_;
        }
    };

    struct SorterByCurrencyId
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_currency_id < y.m_currency_id;
        }
    };

    struct SorterByStatus
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_status.id() < y.m_status.id();
        }
    };

    struct SorterByFavorite
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_favorite.id() < y.m_favorite.id();
        }
    };

    struct SorterByNum
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_num < y.m_num;
        }
    };

    struct SorterByNotes
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_notes < y.m_notes;
        }
    };

    struct SorterByHeldAt
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_held_at < y.m_held_at;
        }
    };

    struct SorterByWebsite
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_website < y.m_website;
        }
    };

    struct SorterByContactInfo
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_contact_info < y.m_contact_info;
        }
    };

    struct SorterByAccessInfo
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_access_info < y.m_access_info;
        }
    };

    struct SorterByOpenDate
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_open_date < y.m_open_date;
        }
    };

    struct SorterByOpenBalance
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_open_balance < y.m_open_balance;
        }
    };

    struct SorterByStmtLocked
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return (x.m_stmt_locked ? 1 : 0) < (y.m_stmt_locked ? 1 : 0);
        }
    };

    struct SorterByStmtDate
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_stmt_date_n.has_value() && (
                !y.m_stmt_date_n.has_value() ||
                x.m_stmt_date_n.value() < y.m_stmt_date_n.value()
            );
        }
    };

    struct SorterByMinBalance
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_min_balance < y.m_min_balance;
        }
    };

    struct SorterByCreditLimit
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_credit_limit < y.m_credit_limit;
        }
    };

    struct SorterByInterestRate
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_interest_rate < y.m_interest_rate;
        }
    };

    struct SorterByPaymentDueDate
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_payment_due_date_n.has_value() && (
                !y.m_payment_due_date_n.has_value() ||
                x.m_payment_due_date_n.value() < y.m_payment_due_date_n.value()
            );
        }
    };

    struct SorterByMinPayment
    {
        bool operator()(const AccountData& x, const AccountData& y)
        {
            return x.m_min_payment < y.m_min_payment;
        }
    };
};

inline AccountData::AccountData(wxSQLite3ResultSet& q) :
    AccountData()
{
    from_select_result(q);
}

inline void AccountData::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    to_row().to_insert_stmt(stmt, id);
}

inline void AccountData::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_row().to_update_stmt(stmt);
}

inline AccountData& AccountData::from_select_result(wxSQLite3ResultSet& q)
{
    return from_row(AccountRow().from_select_result(q));
}

inline wxString AccountData::to_json() const
{
    return to_row().to_json();
}

inline void AccountData::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    to_row().as_json(json_writer);
}

inline row_t AccountData::to_html_row() const
{
    return to_row().to_html_row();
}

inline void AccountData::to_html_template(html_template& t) const
{
    to_row().to_html_template(t);
}

inline AccountData& AccountData::clone_from(const AccountData& other)
{
    *this = other;
    id(-1);
    return *this;
}
