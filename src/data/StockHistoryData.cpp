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

#include "StockHistoryData.h"

StockHistoryData::StockHistoryData() :
    m_id(-1),
    m_date(mmDate::today()),
    m_price(0.0),
    m_update_type(UpdateType())
{
}

// Convert StockHistoryData to StockHistoryRow
StockHistoryRow StockHistoryData::to_row() const
{
    StockHistoryRow row;

    row.HISTID  = m_id;
    row.SYMBOL  = m_symbol;
    row.DATE    = m_date.isoDate();
    row.VALUE   = m_price;
    row.UPDTYPE = static_cast<int64>(m_update_type.code());

    return row;
}

// Convert StockHistoryRow to StockHistoryData
StockHistoryData& StockHistoryData::from_row(const StockHistoryRow& row)
{
    m_id          = row.HISTID;
    m_symbol      = row.SYMBOL;
    m_date        = mmDate(row.DATE);
    m_price       = row.VALUE;
    m_update_type = UpdateType::from_code(
        static_cast<int>(row.UPDTYPE.GetValue())
    );

    return *this;
}

bool StockHistoryData::equals(const StockHistoryData* other) const
{
    if ( m_id               != other->m_id)               return false;
    if (!m_symbol.IsSameAs(    other->m_symbol))          return false;
    if ( m_date             != other->m_date)             return false;
    if ( m_price            != other->m_price)            return false;
    if ( m_update_type.id() != other->m_update_type.id()) return false;

    return true;
}
