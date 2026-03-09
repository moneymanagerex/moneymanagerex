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

#include "ReportData.h"

ReportData::ReportData() :
    m_id(-1),
    m_active(true)
{
}

// Convert ReportData to ReportRow
ReportRow ReportData::to_row() const
{
    ReportRow row;

    row.REPORTID        = m_id;
    row.REPORTNAME      = m_name;
    row.GROUPNAME       = m_group_name;
    row.ACTIVE          = (m_active ? 1 : 0);
    row.SQLCONTENT      = m_sql_content;
    row.LUACONTENT      = m_lua_content;
    row.TEMPLATECONTENT = m_template_content;
    row.DESCRIPTION     = m_description;

    return row;
}

// Convert ReportRow to ReportData
ReportData& ReportData::from_row(const ReportRow& row)
{
    m_id               = row.REPORTID;
    m_name             = row.REPORTNAME;
    m_group_name       = row.GROUPNAME;
    m_active           = (row.ACTIVE != 0);
    m_sql_content      = row.SQLCONTENT;
    m_lua_content      = row.LUACONTENT;
    m_template_content = row.TEMPLATECONTENT;
    m_description      = row.DESCRIPTION;

    return *this;
}

bool ReportData::equals(const ReportData* other) const
{
    if ( m_id != other->m_id) return false;
    if (!m_name.IsSameAs(other->m_name)) return false;
    if (!m_group_name.IsSameAs(other->m_group_name)) return false;
    if ( m_active != other->m_active) return false;
    if (!m_sql_content.IsSameAs(other->m_sql_content)) return false;
    if (!m_lua_content.IsSameAs(other->m_lua_content)) return false;
    if (!m_template_content.IsSameAs(other->m_template_content)) return false;
    if (!m_description.IsSameAs(other->m_description)) return false;

    return true;
}
