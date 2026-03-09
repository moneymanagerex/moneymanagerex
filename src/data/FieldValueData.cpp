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

#include "FieldValueData.h"

FieldValueData::FieldValueData() :
    m_id(-1),
    m_field_id(-1),
    m_ref_type(RefTypeN()),
    m_ref_id(-1)
{
}

// Convert FieldValueData to FieldValueRow
FieldValueRow FieldValueData::to_row() const
{
    FieldValueRow row;


    row.FIELDATADID = m_id;
    row.FIELDID     = m_field_id;
    row.REFID       = FieldValueData::encode_REFID(m_ref_type, m_ref_id);
    row.CONTENT     = m_content;

    return row;
}

// Convert FieldValueRow to FieldValueData
FieldValueData& FieldValueData::from_row(const FieldValueRow& row)
{
    m_id       = row.FIELDATADID;
    m_field_id = row.FIELDID;
    m_ref_type = FieldValueData::decode_ref_type(row.REFID);
    m_ref_id   = FieldValueData::decode_ref_id(row.REFID);
    m_content  = row.CONTENT;

    return *this;
}

bool FieldValueData::equals(const FieldValueData* other) const
{
    if ( m_id              != other->m_id)              return false;
    if ( m_field_id        != other->m_field_id)        return false;
    if ( m_ref_type.id_n() != other->m_ref_type.id_n()) return false;
    if ( m_ref_id          != other->m_ref_id)          return false;
    if (!m_content.IsSameAs(  other->m_content))        return false;

    return true;
}
