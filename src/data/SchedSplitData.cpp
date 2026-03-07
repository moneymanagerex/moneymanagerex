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

#include "SchedSplitData.h"

SchedSplitData::SchedSplitData() :
    m_id(-1),
    m_sched_id(-1),
    m_category_id(-1),
    m_amount(0.0)
{
}

// Convert SchedSplitData to SchedSplitRow
SchedSplitRow SchedSplitData::to_row() const
{
    SchedSplitRow row;

    row.SPLITTRANSID     = m_id;
    row.TRANSID          = m_sched_id;
    row.CATEGID          = m_category_id;
    row.SPLITTRANSAMOUNT = m_amount;
    row.NOTES            = m_notes;

    return row;
}

// Convert SchedSplitRow to SchedSplitData
SchedSplitData& SchedSplitData::from_row(const SchedSplitRow& row)
{
    m_id          = row.SPLITTRANSID;
    m_sched_id    = row.TRANSID;
    m_category_id = row.CATEGID;
    m_amount      = row.SPLITTRANSAMOUNT;
    m_notes       = row.NOTES;

    return *this;
}

bool SchedSplitData::equals(const SchedSplitData* other) const
{
    if ( m_id != other->m_id) return false;
    if ( m_sched_id != other->m_sched_id) return false;
    if ( m_category_id != other->m_category_id) return false;
    if ( m_amount != other->m_amount) return false;
    if (!m_notes.IsSameAs(other->m_notes)) return false;

    return true;
}
