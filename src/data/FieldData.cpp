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

#include "FieldData.h"

FieldData::FieldData() :
    m_id(-1)
{
}

// Convert FieldData to FieldRow
FieldRow FieldData::to_row() const
{
    FieldRow row;

    row.FIELDID     = m_id;
    row.REFTYPE     = m_ref_type.name_n();
    row.DESCRIPTION = m_description;
    row.TYPE        = m_type_n.name_n();
    row.PROPERTIES  = m_properties;

    return row;
}

// Convert FieldRow to FieldData
FieldData& FieldData::from_row(const FieldRow& row)
{
    m_id          = row.FIELDID;
    m_ref_type    = RefTypeN(row.REFTYPE);
    m_description = row.DESCRIPTION;
    m_type_n      = FieldTypeN(row.TYPE);
    m_properties  = row.PROPERTIES;

    return *this;
}

bool FieldData::equals(const FieldData* other) const
{
    if ( m_id                 != other->m_id)              return false;
    if ( m_ref_type.id_n()    != other->m_ref_type.id_n()) return false;
    if (!m_description.IsSameAs( other->m_description))    return false;
    if ( m_type_n.id_n()      != other->m_type_n.id_n())   return false;
    if (!m_properties.IsSameAs(  other->m_properties))     return false;

    return true;
}
