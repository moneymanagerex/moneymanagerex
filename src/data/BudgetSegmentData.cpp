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

#include "BudgetSegmentData.h"

BudgetSegmentData::BudgetSegmentData() :
    m_id(-1),
    m_period_id(-1),
    m_start_day(1),
    m_end_day(31),
    m_sort_order(0),
    m_active(true)
{
}

// Convert BudgetSegmentData to BudgetSegmentRow
BudgetSegmentRow BudgetSegmentData::to_row() const
{
    BudgetSegmentRow row;

    row.SEGMENTID    = m_id;
    row.BUDGETYEARID = m_period_id;
    row.SEGMENTNAME  = m_name;
    row.STARTDAY     = m_start_day;
    row.ENDDAY       = m_end_day;
    row.SORTORDER    = m_sort_order;
    row.ACTIVE       = (m_active ? 1 : 0);

    return row;
}

// Convert BudgetSegmentRow to BudgetSegmentData
BudgetSegmentData& BudgetSegmentData::from_row(const BudgetSegmentRow& row)
{
    m_id         = row.SEGMENTID;
    m_period_id  = row.BUDGETYEARID;
    m_name       = row.SEGMENTNAME;
    m_start_day  = static_cast<int>(row.STARTDAY.GetValue());
    m_end_day    = static_cast<int>(row.ENDDAY.GetValue());
    m_sort_order = static_cast<int>(row.SORTORDER.GetValue());
    m_active     = (row.ACTIVE != 0);

    return *this;
}

bool BudgetSegmentData::equals(const BudgetSegmentData* other) const
{
    if ( m_id         != other->m_id)         return false;
    if ( m_period_id  != other->m_period_id)  return false;
    if (!m_name.IsSameAs(other->m_name))      return false;
    if ( m_start_day  != other->m_start_day)  return false;
    if ( m_end_day    != other->m_end_day)    return false;
    if ( m_sort_order != other->m_sort_order) return false;
    if ( m_active     != other->m_active)     return false;

    return true;
}
