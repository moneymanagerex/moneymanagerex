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

#include "BudgetData.h"

BudgetData::BudgetData() :
    m_id(-1),
    m_period_id(-1),
    m_category_id(-1),
    m_frequency(BudgetFrequency()),
    m_amount(0.0),
    m_active(true)
{
}

// Convert BudgetData to BudgetRow
BudgetRow BudgetData::to_row() const
{
    BudgetRow row;

    row.BUDGETENTRYID = m_id;
    row.BUDGETYEARID  = m_period_id;
    row.CATEGID       = m_category_id;
    row.PERIOD        = m_frequency.name();
    row.AMOUNT        = m_amount;
    row.NOTES         = m_notes;
    row.ACTIVE        = (m_active ? 1 : 0);

    return row;
}

// Convert BudgetRow to BudgetData
BudgetData& BudgetData::from_row(const BudgetRow& row)
{
    m_id          = row.BUDGETENTRYID;
    m_period_id   = row.BUDGETYEARID;
    m_category_id = row.CATEGID;
    m_frequency   = BudgetFrequency(row.PERIOD);
    m_amount      = row.AMOUNT;
    m_notes       = row.NOTES;
    m_active      = (row.ACTIVE != 0);

    return *this;
}

bool BudgetData::equals(const BudgetData* other) const
{
    if ( m_id             != other->m_id)             return false;
    if ( m_period_id      != other->m_period_id)      return false;
    if ( m_category_id    != other->m_category_id)    return false;
    if ( m_frequency.id() != other->m_frequency.id()) return false;
    if ( m_amount         != other->m_amount)         return false;
    if (!m_notes.IsSameAs(   other->m_notes))         return false;
    if ( m_active         != other->m_active)         return false;

    return true;
}
