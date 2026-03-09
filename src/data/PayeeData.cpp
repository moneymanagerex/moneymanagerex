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

#include "PayeeData.h"

PayeeData::PayeeData() :
    m_id(-1),
    m_category_id_n(-1),
    m_active(true)
{
}

// Convert PayeeData to PayeeRow
PayeeRow PayeeData::to_row() const
{
    PayeeRow row;

    row.PAYEEID   = m_id;
    row.PAYEENAME = m_name;
    row.CATEGID   = m_category_id_n;
    row.NUMBER    = m_number;
    row.WEBSITE   = m_website;
    row.NOTES     = m_notes;
    row.ACTIVE    = m_active ? 1 : 0;
    row.PATTERN   = m_pattern;

    return row;
}

// Convert PayeeRow to PayeeData
PayeeData& PayeeData::from_row(const PayeeRow& row)
{
    m_id            = row.PAYEEID;
    m_name          = row.PAYEENAME;
    m_category_id_n = row.CATEGID;
    m_number        = row.NUMBER;
    m_website       = row.WEBSITE;
    m_notes         = row.NOTES;
    m_active        = (row.ACTIVE != 0);
    m_pattern       = row.PATTERN;

    return *this;
}

bool PayeeData::equals(const PayeeData* other) const
{
    if ( m_id             != other->m_id)            return false;
    if (!m_name.IsSameAs(    other->m_name))         return false;
    if ( m_category_id_n  != other->m_category_id_n) return false;
    if (!m_number.IsSameAs(  other->m_number))       return false;
    if (!m_website.IsSameAs( other->m_website))      return false;
    if (!m_notes.IsSameAs(   other->m_notes))        return false;
    if ( m_active         != other->m_active)        return false;
    if (!m_pattern.IsSameAs( other->m_pattern))      return false;

    return true;
}
