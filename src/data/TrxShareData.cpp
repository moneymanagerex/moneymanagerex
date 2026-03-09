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

#include "TrxShareData.h"

TrxShareData::TrxShareData() :
    m_id(-1),
    m_trx_id(-1),
    m_number(0.0),
    m_price(0.0),
    m_commission(0.0)
{
}

// Convert TrxShareData to TrxShareRow
TrxShareRow TrxShareData::to_row() const
{
    TrxShareRow row;

    row.SHAREINFOID       = m_id;
    row.CHECKINGACCOUNTID = m_trx_id;
    row.SHARENUMBER       = m_number;
    row.SHAREPRICE        = m_price;
    row.SHARECOMMISSION   = m_commission;
    row.SHARELOT          = m_lot;

    return row;
}

// Convert TrxShareRow to TrxShareData
TrxShareData& TrxShareData::from_row(const TrxShareRow& row)
{
    m_id         = row.SHAREINFOID;
    m_trx_id     = row.CHECKINGACCOUNTID;
    m_number     = row.SHARENUMBER;
    m_price      = row.SHAREPRICE;
    m_commission = row.SHARECOMMISSION;
    m_lot        = row.SHARELOT;

    return *this;
}

bool TrxShareData::equals(const TrxShareData* other) const
{
    if ( m_id         != other->m_id)         return false;
    if ( m_trx_id     != other->m_trx_id)     return false;
    if ( m_number     != other->m_number)     return false;
    if ( m_price      != other->m_price)      return false;
    if ( m_commission != other->m_commission) return false;
    if (!m_lot.IsSameAs( other->m_lot))       return false;

    return true;
}
