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

#include "PlanGroupData.h"

PlanGroupData::PlanGroupData() :
    m_id(-1),
    m_parent_id(-1),
    m_status(PlanStatus()),
    m_sort_order(0),
    m_active(true)
{
}

// Convert PlanGroupData to PlanGroupRow
PlanGroupRow PlanGroupData::to_row() const
{
    PlanGroupRow row;

    row.GROUPID    = m_id;
    row.PARENTID   = m_parent_id;
    row.GROUPNAME  = m_name;
    row.NOTES      = m_notes;
    row.STATUS     = m_status.key();
    row.TARGETDATE = m_target_date;
    row.SORTORDER  = m_sort_order;
    row.ACTIVE     = (m_active ? 1 : 0);

    return row;
}

// Convert PlanGroupRow to PlanGroupData
PlanGroupData& PlanGroupData::from_row(const PlanGroupRow& row)
{
    m_id          = row.GROUPID;
    m_parent_id   = row.PARENTID;
    m_name        = row.GROUPNAME;
    m_notes       = row.NOTES;
    m_status      = PlanStatus(row.STATUS);
    m_target_date = row.TARGETDATE;
    m_sort_order  = static_cast<int>(row.SORTORDER.GetValue());
    m_active      = (row.ACTIVE != 0);

    return *this;
}

bool PlanGroupData::equals(const PlanGroupData* other) const
{
    if ( m_id          != other->m_id)          return false;
    if ( m_parent_id   != other->m_parent_id)   return false;
    if (!m_name.IsSameAs(other->m_name))        return false;
    if (!m_notes.IsSameAs(other->m_notes))      return false;
    if ( m_status.id() != other->m_status.id()) return false;
    if (!m_target_date.IsSameAs(other->m_target_date)) return false;
    if ( m_sort_order  != other->m_sort_order)  return false;
    if ( m_active      != other->m_active)      return false;

    return true;
}
