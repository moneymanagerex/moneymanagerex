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
    m_account_id(-1),
    m_to_account_id_n(-1),
    m_payee_id_n(-1),
    m_category_id_n(-1),
    m_amount(0.0),
    m_to_amount(0.0),
    m_followup_id(-1),
    m_color(-1),
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
    row.TRANSCODE          = TRANSCODE;
    row.TRANSAMOUNT        = m_amount;
    row.STATUS             = STATUS;
    row.TRANSACTIONNUMBER  = m_number;
    row.NOTES              = m_notes;
    row.CATEGID            = m_category_id_n;
    row.TRANSDATE          = TRANSDATE;
    row.FOLLOWUPID         = m_followup_id;
    row.TOTRANSAMOUNT      = m_to_amount;
    row.REPEATS            = REPEATS;
    row.NEXTOCCURRENCEDATE = NEXTOCCURRENCEDATE;
    row.NUMOCCURRENCES     = NUMOCCURRENCES;
    row.COLOR              = m_color;

    return row;
}

// Convert SchedRow to SchedData
SchedData& SchedData::from_row(const SchedRow& row)
{
    m_id               = row.BDID;
    m_account_id       = row.ACCOUNTID;
    m_to_account_id_n  = row.TOACCOUNTID;
    m_payee_id_n       = row.PAYEEID;
    TRANSCODE          = row.TRANSCODE;
    m_amount           = row.TRANSAMOUNT;
    STATUS             = row.STATUS;
    m_number           = row.TRANSACTIONNUMBER;
    m_notes            = row.NOTES;
    m_category_id_n    = row.CATEGID;
    TRANSDATE          = row.TRANSDATE;
    m_followup_id      = row.FOLLOWUPID;
    m_to_amount        = row.TOTRANSAMOUNT;
    REPEATS            = row.REPEATS;
    NEXTOCCURRENCEDATE = row.NEXTOCCURRENCEDATE;
    NUMOCCURRENCES     = row.NUMOCCURRENCES;
    m_color            = row.COLOR;

    return *this;
}

bool SchedData::equals(const SchedData* other) const
{
    if ( m_id != other->m_id) return false;
    if ( m_account_id != other->m_account_id) return false;
    if ( m_to_account_id_n != other->m_to_account_id_n) return false;
    if ( m_payee_id_n != other->m_payee_id_n) return false;
    if (!TRANSCODE.IsSameAs(other->TRANSCODE)) return false;
    if ( m_amount != other->m_amount) return false;
    if (!STATUS.IsSameAs(other->STATUS)) return false;
    if (!m_number.IsSameAs(other->m_number)) return false;
    if (!m_notes.IsSameAs(other->m_notes)) return false;
    if ( m_category_id_n != other->m_category_id_n) return false;
    if (!TRANSDATE.IsSameAs(other->TRANSDATE)) return false;
    if ( m_followup_id != other->m_followup_id) return false;
    if ( m_to_amount != other->m_to_amount) return false;
    if ( REPEATS != other->REPEATS) return false;
    if (!NEXTOCCURRENCEDATE.IsSameAs(other->NEXTOCCURRENCEDATE)) return false;
    if ( NUMOCCURRENCES != other->NUMOCCURRENCES) return false;
    if ( m_color != other->m_color) return false;

    return true;
}
