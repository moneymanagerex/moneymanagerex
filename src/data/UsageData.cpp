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

#include "UsageData.h"

UsageData::UsageData() :
    m_id(-1)
{
}

// Convert UsageData to UsageRow
UsageRow UsageData::to_row() const
{
    UsageRow row;

    row.USAGEID     = m_id;
    row.USAGEDATE   = m_date;
    row.JSONCONTENT = m_json_content;

    return row;
}

// Convert UsageRow to UsageData
UsageData& UsageData::from_row(const UsageRow& row)
{
    m_id           = row.USAGEID;
    m_date         = row.USAGEDATE;
    m_json_content = row.JSONCONTENT;

    return *this;
}

bool UsageData::equals(const UsageData* other) const
{
    if ( m_id != other->m_id) return false;
    if (!m_date.IsSameAs(other->m_date)) return false;
    if (!m_json_content.IsSameAs(other->m_json_content)) return false;

    return true;
}
