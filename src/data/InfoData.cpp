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

#include "InfoData.h"

InfoData::InfoData() :
    m_id(-1)
{
}

// Convert InfoData to InfoRow
InfoRow InfoData::to_row() const
{
    InfoRow row;

    row.INFOID    = m_id;
    row.INFONAME  = m_name;
    row.INFOVALUE = m_value;

    return row;
}

// Convert InfoRow to InfoData
InfoData& InfoData::from_row(const InfoRow& row)
{
    m_id    = row.INFOID;
    m_name  = row.INFONAME;
    m_value = row.INFOVALUE;

    return *this;
}

bool InfoData::equals(const InfoData* other) const
{
    if ( m_id != other->m_id) return false;
    if (!m_name.IsSameAs(other->m_name)) return false;
    if (!m_value.IsSameAs(other->m_value)) return false;

    return true;
}
