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

#include "AccountData.h"

AccountData::AccountData() :
    m_id(-1),
    m_currency_id(-1),
    m_status(AccountStatus()),
    m_favorite(AccountFavorite()),
    m_open_date(mmDate::today()),
    m_open_balance(0.0),
    m_stmt_locked(false),
    m_stmt_date_n(mmDateN()),
    m_min_balance(0.0),
    m_credit_limit(0.0),
    m_interest_rate(0.0),
    m_payment_due_date_n(mmDateN()),
    m_min_payment(0.0)
{
}

// Convert AccountData to AccountRow
AccountRow AccountData::to_row() const
{
    AccountRow row;

    row.ACCOUNTID       = m_id;
    row.ACCOUNTNAME     = m_name;
    row.ACCOUNTTYPE     = m_type_;
    row.ACCOUNTNUM      = m_num;
    row.STATUS          = m_status.name();
    row.NOTES           = m_notes;
    row.HELDAT          = m_held_at;
    row.WEBSITE         = m_website;
    row.CONTACTINFO     = m_contact_info;
    row.ACCESSINFO      = m_access_info;
    row.INITIALBAL      = m_open_balance;
    row.INITIALDATE     = m_open_date.isoDate();
    row.FAVORITEACCT    = m_favorite.name();
    row.CURRENCYID      = m_currency_id;
    row.STATEMENTLOCKED = (m_stmt_locked ? 1 : 0);
    row.STATEMENTDATE   = m_stmt_date_n.isoDateN();
    row.MINIMUMBALANCE  = m_min_balance;
    row.CREDITLIMIT     = m_credit_limit;
    row.INTERESTRATE    = m_interest_rate;
    row.PAYMENTDUEDATE  = m_payment_due_date_n.isoDateN();
    row.MINIMUMPAYMENT  = m_min_payment;

    return row;
}

// Convert AccountRow to AccountData
AccountData& AccountData::from_row(const AccountRow& row)
{
    m_id                 = row.ACCOUNTID;
    m_name               = row.ACCOUNTNAME;
    m_type_              = row.ACCOUNTTYPE;
    m_currency_id        = row.CURRENCYID;
    m_status             = AccountStatus(row.STATUS);
    m_favorite           = AccountFavorite(row.FAVORITEACCT);
    m_num                = row.ACCOUNTNUM;
    m_notes              = row.NOTES;
    m_held_at            = row.HELDAT;
    m_website            = row.WEBSITE;
    m_contact_info       = row.CONTACTINFO;
    m_access_info        = row.ACCESSINFO;
    m_open_date          = mmDate(row.INITIALDATE);
    m_open_balance       = row.INITIALBAL;
    m_stmt_locked        = (row.STATEMENTLOCKED > 0);
    m_stmt_date_n        = mmDateN(row.STATEMENTDATE);
    m_min_balance        = row.MINIMUMBALANCE;
    m_credit_limit       = row.CREDITLIMIT;
    m_interest_rate      = row.INTERESTRATE;
    m_payment_due_date_n = mmDateN(row.PAYMENTDUEDATE);
    m_min_payment        = row.MINIMUMPAYMENT;

    return *this;
}

bool AccountData::equals(const AccountData* other) const
{
    if ( m_id                  != other->m_id)                 return false;
    if (!m_name.IsSameAs(         other->m_name))              return false;
    if (!m_type_.IsSameAs(        other->m_type_))             return false;
    if ( m_currency_id         != other->m_currency_id)        return false;
    if ( m_status.id()         != other->m_status.id())        return false;
    if ( m_favorite.id()       != other->m_favorite.id())      return false;
    if (!m_num.IsSameAs(          other->m_num))               return false;
    if (!m_notes.IsSameAs(        other->m_notes))             return false;
    if (!m_held_at.IsSameAs(      other->m_held_at))           return false;
    if (!m_website.IsSameAs(      other->m_website))           return false;
    if (!m_contact_info.IsSameAs( other->m_contact_info))      return false;
    if (!m_access_info.IsSameAs(  other->m_access_info))       return false;
    if ( m_open_date           != other->m_open_date)          return false;
    if ( m_open_balance        != other->m_open_balance)       return false;
    if ( m_stmt_locked         != other->m_stmt_locked)        return false;
    if ( m_stmt_date_n         != other->m_stmt_date_n)        return false;
    if ( m_min_balance         != other->m_min_balance)        return false;
    if ( m_credit_limit        != other->m_credit_limit)       return false;
    if ( m_interest_rate       != other->m_interest_rate)      return false;
    if ( m_payment_due_date_n  != other->m_payment_due_date_n) return false;
    if ( m_min_payment         != other->m_min_payment)        return false;

    return true;
}
