/*******************************************************
 Copyright (C) 2026

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

#include "PlanItemData.h"

PlanItemData::PlanItemData() :
    m_id(-1),
    m_group_id(-1),
    m_kind(PlanItemKind()),
    m_status(PlanStatus()),
    m_amount(0.0),
    m_currency_id(-1),
    m_category_id(-1),
    m_account_id(-1),
    m_units(0.0),
    m_unit_price(0.0),
    m_tax_rate(0.0),
    m_confidence(1.0),
    m_sort_order(0),
    m_active(true),
    m_price_assumption_id(-1),
    m_tax_assumption_id(-1)
{
}

// Convert PlanItemData to PlanItemRow
PlanItemRow PlanItemData::to_row() const
{
    PlanItemRow row;

    row.PLANITEMID  = m_id;
    row.GROUPID     = m_group_id;
    row.ITEMNAME    = m_name;
    row.NOTES       = m_notes;
    row.KIND        = m_kind.key();
    row.STATUS      = m_status.key();
    row.TARGETDATE  = m_target_date;
    row.AMOUNT      = m_amount;
    row.CURRENCYID  = m_currency_id;
    row.CATEGID     = m_category_id;
    row.ACCOUNTID   = m_account_id;
    row.UNITS       = m_units;
    row.UNITPRICE   = m_unit_price;
    row.TAXRATE     = m_tax_rate;
    row.STOCKSYMBOL = m_stock_symbol;
    row.CONFIDENCE  = m_confidence;
    row.SORTORDER   = m_sort_order;
    row.ACTIVE      = (m_active ? 1 : 0);
    row.PRICEASSUMPTIONID = m_price_assumption_id;
    row.TAXASSUMPTIONID   = m_tax_assumption_id;

    return row;
}

// Convert PlanItemRow to PlanItemData
PlanItemData& PlanItemData::from_row(const PlanItemRow& row)
{
    m_id           = row.PLANITEMID;
    m_group_id     = row.GROUPID;
    m_name         = row.ITEMNAME;
    m_notes        = row.NOTES;
    m_kind         = PlanItemKind(row.KIND);
    m_status       = PlanStatus(row.STATUS);
    m_target_date  = row.TARGETDATE;
    m_amount       = row.AMOUNT;
    m_currency_id  = row.CURRENCYID;
    m_category_id  = row.CATEGID;
    m_account_id   = row.ACCOUNTID;
    m_units        = row.UNITS;
    m_unit_price   = row.UNITPRICE;
    m_tax_rate     = row.TAXRATE;
    m_stock_symbol = row.STOCKSYMBOL;
    m_confidence   = row.CONFIDENCE;
    m_sort_order   = static_cast<int>(row.SORTORDER.GetValue());
    m_active       = (row.ACTIVE != 0);
    m_price_assumption_id = row.PRICEASSUMPTIONID;
    m_tax_assumption_id   = row.TAXASSUMPTIONID;

    return *this;
}

bool PlanItemData::equals(const PlanItemData* other) const
{
    if ( m_id          != other->m_id)          return false;
    if ( m_group_id    != other->m_group_id)    return false;
    if (!m_name.IsSameAs(other->m_name))        return false;
    if (!m_notes.IsSameAs(other->m_notes))      return false;
    if ( m_kind.id()   != other->m_kind.id())   return false;
    if ( m_status.id() != other->m_status.id()) return false;
    if (!m_target_date.IsSameAs(other->m_target_date)) return false;
    if ( m_amount      != other->m_amount)      return false;
    if ( m_currency_id != other->m_currency_id) return false;
    if ( m_category_id != other->m_category_id) return false;
    if ( m_account_id  != other->m_account_id)  return false;
    if ( m_units       != other->m_units)       return false;
    if ( m_unit_price  != other->m_unit_price)  return false;
    if ( m_tax_rate    != other->m_tax_rate)    return false;
    if (!m_stock_symbol.IsSameAs(other->m_stock_symbol)) return false;
    if ( m_confidence  != other->m_confidence)  return false;
    if ( m_sort_order  != other->m_sort_order)  return false;
    if ( m_active      != other->m_active)      return false;
    if ( m_price_assumption_id != other->m_price_assumption_id) return false;
    if ( m_tax_assumption_id   != other->m_tax_assumption_id)   return false;

    return true;
}
