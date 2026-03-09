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

#include "CategoryData.h"

CategoryData::CategoryData() :
    m_id(-1),
    m_parent_id_n(-1),
    m_active(true)
{
}

// Convert CategoryData to CategoryRow
CategoryRow CategoryData::to_row() const
{
    CategoryRow row;

    row.CATEGID   = m_id;
    row.CATEGNAME = m_name;
    row.ACTIVE    = (m_active ? 1 : 0);
    row.PARENTID  = m_parent_id_n;

    return row;
}

// Convert CategoryRow to CategoryData
CategoryData& CategoryData::from_row(const CategoryRow& row)
{
    m_id          = row.CATEGID;
    m_name        = row.CATEGNAME;
    m_active      = (row.ACTIVE != 0);
    m_parent_id_n = row.PARENTID;

    return *this;
}

bool CategoryData::equals(const CategoryData* other) const
{
    if ( m_id != other->m_id) return false;
    if (!m_name.IsSameAs(other->m_name)) return false;
    if ( m_active != other->m_active) return false;
    if ( m_parent_id_n != other->m_parent_id_n) return false;

    return true;
}
