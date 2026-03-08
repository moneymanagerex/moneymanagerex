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

#include "CurrencyHistoryData.h"

CurrencyHistoryData::CurrencyHistoryData() :
    m_id(-1),
    m_currency_id(-1),
    m_date(mmDate::today()),
    m_base_conv_rate(0.0),
    m_update_type(UpdateType())
{
}

// Convert CurrencyHistoryData to CurrencyHistoryRow
CurrencyHistoryRow CurrencyHistoryData::to_row() const
{
    CurrencyHistoryRow row;

    row.CURRHISTID  = m_id;
    row.CURRENCYID  = m_currency_id;
    row.CURRDATE    = m_date.isoDate();
    row.CURRVALUE   = m_base_conv_rate;
    row.CURRUPDTYPE = static_cast<int64>(m_update_type.code());

    return row;
}

// Convert CurrencyHistoryRow to CurrencyHistoryData
CurrencyHistoryData& CurrencyHistoryData::from_row(const CurrencyHistoryRow& row)
{
    m_id             = row.CURRHISTID;
    m_currency_id    = row.CURRENCYID;
    m_date           = mmDate(row.CURRDATE);
    m_base_conv_rate = row.CURRVALUE;
    m_update_type    = UpdateType::from_code(
        static_cast<int>(row.CURRUPDTYPE.GetValue())
    );

    return *this;
}

bool CurrencyHistoryData::equals(const CurrencyHistoryData* other) const
{
    if ( m_id               != other->m_id)               return false;
    if ( m_currency_id      != other->m_currency_id)      return false;
    if ( m_date             != other->m_date)             return false;
    if ( m_base_conv_rate   != other->m_base_conv_rate)   return false;
    if ( m_update_type.id() != other->m_update_type.id()) return false;

    return true;
}
