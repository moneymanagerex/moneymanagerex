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

TrxData::TrxData()
{
    m_id              = -1;
    m_account_id_p    = -1;
    m_to_account_id_n = -1;
    m_payee_id_n      = -1;
    m_amount          = 0.0;
    m_category_id_n   = -1;
    m_followup_id     = -1;
    m_to_amount       = 0.0;
    m_color           = -1;
}

// Convert TrxData to TrxRow
TrxRow TrxData::to_row() const
{
    TrxRow row;

    row.TRANSID           = m_id;
    row.ACCOUNTID         = m_account_id_p;
    row.TOACCOUNTID       = m_to_account_id_n;
    row.PAYEEID           = m_payee_id_n;
    row.TRANSCODE         = TRANSCODE;
    row.TRANSAMOUNT       = m_amount;
    row.STATUS            = STATUS;
    row.TRANSACTIONNUMBER = m_number;
    row.NOTES             = NOTES;
    row.CATEGID           = m_category_id_n;
    row.TRANSDATE         = TRANSDATE;
    row.LASTUPDATEDTIME   = LASTUPDATEDTIME;
    row.DELETEDTIME       = DELETEDTIME;
    row.FOLLOWUPID        = m_followup_id;
    row.TOTRANSAMOUNT     = m_to_amount;
    row.COLOR             = m_color;

    return row;
}

// Convert TrxRow to TrxData
TrxData& TrxData::from_row(const TrxRow& row)
{
    m_id              = row.TRANSID;           // int64
    m_account_id_p    = row.ACCOUNTID;         // int64
    m_to_account_id_n = row.TOACCOUNTID;       // int64
    m_payee_id_n      = row.PAYEEID;           // int64
    TRANSCODE         = row.TRANSCODE;         // wxString
    m_amount          = row.TRANSAMOUNT;       // double
    STATUS            = row.STATUS;            // wxString
    m_number          = row.TRANSACTIONNUMBER; // wxString
    NOTES             = row.NOTES;             // wxString
    m_category_id_n   = row.CATEGID;           // int64
    TRANSDATE         = row.TRANSDATE;         // wxString
    LASTUPDATEDTIME   = row.LASTUPDATEDTIME;   // wxString
    DELETEDTIME       = row.DELETEDTIME;       // wxString
    m_followup_id     = row.FOLLOWUPID;        // int64
    m_to_amount       = row.TOTRANSAMOUNT;     // double
    m_color           = row.COLOR;             // int64

    return *this;
}

bool TrxData::equals(const TrxData* other) const
{
    if ( m_id != other->m_id) return false;
    if ( m_account_id_p != other->m_account_id_p) return false;
    if ( m_to_account_id_n != other->m_to_account_id_n) return false;
    if ( m_payee_id_n != other->m_payee_id_n) return false;
    if (!TRANSCODE.IsSameAs(other->TRANSCODE)) return false;
    if ( m_amount != other->m_amount) return false;
    if (!STATUS.IsSameAs(other->STATUS)) return false;
    if (!m_number.IsSameAs(other->m_number)) return false;
    if (!NOTES.IsSameAs(other->NOTES)) return false;
    if ( m_category_id_n != other->m_category_id_n) return false;
    if (!TRANSDATE.IsSameAs(other->TRANSDATE)) return false;
    if (!LASTUPDATEDTIME.IsSameAs(other->LASTUPDATEDTIME)) return false;
    if (!DELETEDTIME.IsSameAs(other->DELETEDTIME)) return false;
    if ( m_followup_id != other->m_followup_id) return false;
    if ( m_to_amount != other->m_to_amount) return false;
    if ( m_color != other->m_color) return false;

    return true;
}
