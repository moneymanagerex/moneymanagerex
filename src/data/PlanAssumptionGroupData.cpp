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

#include "PlanAssumptionGroupData.h"

PlanAssumptionGroupData::PlanAssumptionGroupData() :
    m_id(-1),
    m_kind(PlanAssumptionKind()),
    m_active_assumption_id(-1),
    m_active(true)
{
}

// Convert PlanAssumptionGroupData to PlanAssumptionGroupRow
PlanAssumptionGroupRow PlanAssumptionGroupData::to_row() const
{
    PlanAssumptionGroupRow row;

    row.GROUPID            = m_id;
    row.GROUPNAME          = m_name;
    row.KIND               = m_kind.key();
    row.SCOPEKEY           = m_scope_key;
    row.UNIT               = m_unit;
    row.ACTIVEASSUMPTIONID = m_active_assumption_id;
    row.NOTES              = m_notes;
    row.ACTIVE             = (m_active ? 1 : 0);

    return row;
}

// Convert PlanAssumptionGroupRow to PlanAssumptionGroupData
PlanAssumptionGroupData& PlanAssumptionGroupData::from_row(const PlanAssumptionGroupRow& row)
{
    m_id                   = row.GROUPID;
    m_name                 = row.GROUPNAME;
    m_kind                 = PlanAssumptionKind(row.KIND);
    m_scope_key            = row.SCOPEKEY;
    m_unit                 = row.UNIT;
    m_active_assumption_id = row.ACTIVEASSUMPTIONID;
    m_notes                = row.NOTES;
    m_active               = (row.ACTIVE != 0);

    return *this;
}

bool PlanAssumptionGroupData::equals(const PlanAssumptionGroupData* other) const
{
    if ( m_id        != other->m_id)        return false;
    if (!m_name.IsSameAs(other->m_name))    return false;
    if ( m_kind.id() != other->m_kind.id()) return false;
    if (!m_scope_key.IsSameAs(other->m_scope_key)) return false;
    if (!m_unit.IsSameAs(other->m_unit))    return false;
    if ( m_active_assumption_id != other->m_active_assumption_id) return false;
    if (!m_notes.IsSameAs(other->m_notes))  return false;
    if ( m_active    != other->m_active)    return false;

    return true;
}
