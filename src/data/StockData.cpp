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

#include "StockData.h"

StockData::StockData() :
    m_id(-1),
    m_account_id_n(-1),
    m_num_shares(0.0),
    m_purchase_date(mmDate::today()),
    m_purchase_price(0.0),
    m_current_price(0.0),
    m_purchase_value(0.0),
    m_commission(0.0)
{
}

// Convert StockData to StockRow
StockRow StockData::to_row() const
{
    StockRow row;

    row.STOCKID       = m_id;
    row.HELDAT        = m_account_id_n;
    row.PURCHASEDATE  = m_purchase_date.isoDate();
    row.STOCKNAME     = m_name;
    row.SYMBOL        = m_symbol;
    row.NUMSHARES     = m_num_shares;
    row.PURCHASEPRICE = m_purchase_price;
    row.NOTES         = m_notes;
    row.CURRENTPRICE  = m_current_price;
    row.VALUE         = m_purchase_value;
    row.COMMISSION    = m_commission;

    return row;
}

// Convert StockRow to StockData
StockData& StockData::from_row(const StockRow& row)
{
    m_id             = row.STOCKID;
    m_account_id_n   = row.HELDAT;
    m_name           = row.STOCKNAME;
    m_symbol         = row.SYMBOL;
    m_num_shares     = row.NUMSHARES;
    m_purchase_date  = mmDate(row.PURCHASEDATE);
    m_purchase_price = row.PURCHASEPRICE;
    m_current_price  = row.CURRENTPRICE;
    m_purchase_value = row.VALUE;
    m_commission     = row.COMMISSION;
    m_notes          = row.NOTES;

    return *this;
}

bool StockData::equals(const StockData* other) const
{
    if ( m_id             != other->m_id)             return false;
    if ( m_account_id_n   != other->m_account_id_n)   return false;
    if (!m_name.IsSameAs(    other->m_name))          return false;
    if (!m_symbol.IsSameAs(  other->m_symbol))        return false;
    if ( m_num_shares     != other->m_num_shares)     return false;
    if ( m_purchase_date  != other->m_purchase_date)  return false;
    if ( m_purchase_price != other->m_purchase_price) return false;
    if ( m_current_price  != other->m_current_price)  return false;
    if ( m_purchase_value != other->m_purchase_value) return false;
    if ( m_commission     != other->m_commission)     return false;
    if (!m_notes.IsSameAs(   other->m_notes))         return false;

    return true;
}
