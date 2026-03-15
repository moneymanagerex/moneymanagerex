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

#include "TrxData.h"

TrxData::TrxData() :
    m_id(-1),
    m_date_time(mmDateTime::now()),
    m_type(TrxType()),
    m_status(TrxStatus()),
    m_account_id(-1),
    m_to_account_id_n(-1),
    m_payee_id_n(-1),
    m_category_id_n(-1),
    m_amount(0.0),
    m_to_amount(0.0),
    m_followup_id(-1),
    m_color(-1),
    m_updated_time_n(mmDateTimeN()),
    m_deleted_time_n(mmDateTimeN())
{
}

// Convert TrxData to TrxRow
TrxRow TrxData::to_row() const
{
    TrxRow row;

    row.TRANSID           = m_id;
    row.ACCOUNTID         = m_account_id;
    row.TOACCOUNTID       = m_to_account_id_n;
    row.PAYEEID           = m_payee_id_n;
    row.TRANSCODE         = m_type.name();
    row.TRANSAMOUNT       = m_amount;
    row.STATUS            = m_status.key();
    row.TRANSACTIONNUMBER = m_number;
    row.NOTES             = m_notes;
    row.CATEGID           = m_category_id_n;
    row.TRANSDATE         = m_date_time.isoDateTime();
    row.LASTUPDATEDTIME   = m_updated_time_n.utcDateTimeN();
    row.DELETEDTIME       = m_deleted_time_n.utcDateTimeN();
    row.FOLLOWUPID        = m_followup_id;
    row.TOTRANSAMOUNT     = m_to_amount;
    row.COLOR             = m_color;

    return row;
}

// Convert TrxRow to TrxData
TrxData& TrxData::from_row(const TrxRow& row)
{
    m_id              = row.TRANSID;
    m_date_time       = mmDateTime(row.TRANSDATE);
    m_type            = TrxType(row.TRANSCODE);
    m_status          = TrxStatus(row.STATUS);
    m_account_id      = row.ACCOUNTID;
    m_to_account_id_n = row.TOACCOUNTID;
    m_payee_id_n      = row.PAYEEID;
    m_category_id_n   = row.CATEGID;
    m_amount          = row.TRANSAMOUNT;
    m_to_amount       = row.TOTRANSAMOUNT;
    m_number          = row.TRANSACTIONNUMBER;
    m_notes           = row.NOTES;
    m_followup_id     = row.FOLLOWUPID;
    m_color           = row.COLOR;
    m_updated_time_n  = mmDateTimeN::from_utc(row.LASTUPDATEDTIME);
    m_deleted_time_n  = mmDateTimeN::from_utc(row.DELETEDTIME);

    return *this;
}

bool TrxData::equals(const TrxData* other) const
{
    if ( m_id              != other->m_id)              return false;
    if ( m_date_time       != other->m_date_time)       return false;
    if ( m_type.id()       != other->m_type.id())       return false;
    if ( m_status.id()     != other->m_status.id())     return false;
    if ( m_account_id      != other->m_account_id)      return false;
    if ( m_to_account_id_n != other->m_to_account_id_n) return false;
    if ( m_payee_id_n      != other->m_payee_id_n)      return false;
    if ( m_category_id_n   != other->m_category_id_n)   return false;
    if ( m_amount          != other->m_amount)          return false;
    if ( m_to_amount       != other->m_to_amount)       return false;
    if (!m_number.IsSameAs(   other->m_number))         return false;
    if (!m_notes.IsSameAs(    other->m_notes))          return false;
    if ( m_followup_id     != other->m_followup_id)     return false;
    if ( m_color           != other->m_color)           return false;
    if ( m_updated_time_n  != other->m_updated_time_n)  return false;
    if ( m_deleted_time_n  != other->m_deleted_time_n)  return false;

    return true;
}

double TrxData::account_flow(int64 account_id) const
{
    if (!is_valid())
        return 0.0;

    switch (m_type.id()) {
    case TrxType::e_withdrawal:
        if (m_account_id == account_id)
            return -(m_amount);
        break;
    case TrxType::e_deposit:
        if (m_account_id == account_id)
            return m_amount;
        break;
    case TrxType::e_transfer:
        // Self Transfer as Revaluation
        if (m_account_id == m_to_account_id_n)
            return 0.0;
        else if (m_account_id == account_id)
            return -(m_amount);
        else if (m_to_account_id_n == account_id)
            return m_to_amount;
        break;
    }

    return 0.0;
}

double TrxData::account_inflow(int64 account_id) const
{
    double flow = account_flow(account_id);
    return flow >= 0.0 ? flow : 0.0;
}

double TrxData::account_outflow(int64 account_id) const
{
    double flow = account_flow(account_id);
    return flow < 0.0 ? -flow : 0.0;
}

double TrxData::account_recflow(int64 account_id) const
{
    return is_reconciled() ? account_flow(account_id) : 0.0;
}
