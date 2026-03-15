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

#include "SchedData.h"

SchedData::SchedData() :
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
    m_due_date(mmDate::today()),
    REPEATS(-1),
    NUMOCCURRENCES(-1)
{
}

// Convert SchedData to SchedRow
SchedRow SchedData::to_row() const
{
    SchedRow row;

    row.BDID               = m_id;
    row.ACCOUNTID          = m_account_id;
    row.TOACCOUNTID        = m_to_account_id_n;
    row.PAYEEID            = m_payee_id_n;
    row.TRANSCODE          = m_type.name();
    row.TRANSAMOUNT        = m_amount;
    row.STATUS             = m_status.key();
    row.TRANSACTIONNUMBER  = m_number;
    row.NOTES              = m_notes;
    row.CATEGID            = m_category_id_n;
    row.TRANSDATE          = m_date_time.isoDateTime();
    row.FOLLOWUPID         = m_followup_id;
    row.TOTRANSAMOUNT      = m_to_amount;
    row.REPEATS            = REPEATS;
    row.NEXTOCCURRENCEDATE = m_due_date.isoDate();
    row.NUMOCCURRENCES     = NUMOCCURRENCES;
    row.COLOR              = m_color;

    return row;
}

// Convert SchedRow to SchedData
SchedData& SchedData::from_row(const SchedRow& row)
{
    m_id               = row.BDID;
    m_date_time        = mmDateTime(row.TRANSDATE);
    m_type             = TrxType(row.TRANSCODE);
    m_status           = TrxStatus(row.STATUS);
    m_account_id       = row.ACCOUNTID;
    m_to_account_id_n  = row.TOACCOUNTID;
    m_payee_id_n       = row.PAYEEID;
    m_category_id_n    = row.CATEGID;
    m_amount           = row.TRANSAMOUNT;
    m_to_amount        = row.TOTRANSAMOUNT;
    m_number           = row.TRANSACTIONNUMBER;
    m_notes            = row.NOTES;
    m_followup_id      = row.FOLLOWUPID;
    m_color            = row.COLOR;
    m_due_date         = mmDate(row.NEXTOCCURRENCEDATE);
    REPEATS            = row.REPEATS;
    NUMOCCURRENCES     = row.NUMOCCURRENCES;

    return *this;
}

bool SchedData::equals(const SchedData* other) const
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
    if ( m_due_date        != other->m_due_date)        return false;
    if ( REPEATS           != other->REPEATS)           return false;
    if ( NUMOCCURRENCES    != other->NUMOCCURRENCES)    return false;

    return true;
}
