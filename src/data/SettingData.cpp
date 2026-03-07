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

#include "SettingData.h"

SettingData::SettingData() :
    m_id(-1)
{
}

// Convert SettingData to SettingRow
SettingRow SettingData::to_row() const
{
    SettingRow row;

    row.SETTINGID    = m_id;
    row.SETTINGNAME  = m_name;
    row.SETTINGVALUE = m_value;

    return row;
}

// Convert SettingRow to SettingData
SettingData& SettingData::from_row(const SettingRow& row)
{
    m_id    = row.SETTINGID;
    m_name  = row.SETTINGNAME;
    m_value = row.SETTINGVALUE;

    return *this;
}

bool SettingData::equals(const SettingData* other) const
{
    if ( m_id != other->m_id) return false;
    if (!m_name.IsSameAs(other->m_name)) return false;
    if (!m_value.IsSameAs(other->m_value)) return false;

    return true;
}
