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

#include "AttachmentData.h"

AttachmentData::AttachmentData() :
    m_id(-1),
    m_ref_type_n(RefTypeN()),
    m_ref_id(-1)
{
}

// Convert AttachmentData to AttachmentRow
AttachmentRow AttachmentData::to_row() const
{
    AttachmentRow row;

    row.ATTACHMENTID = m_id;
    row.REFTYPE      = m_ref_type_n.name_n();
    row.REFID        = m_ref_id;
    row.DESCRIPTION  = m_description;
    row.FILENAME     = m_filename;

    return row;
}

// Convert AttachmentRow to AttachmentData
AttachmentData& AttachmentData::from_row(const AttachmentRow& row)
{
    m_id          = row.ATTACHMENTID;
    m_ref_type_n  = RefTypeN(row.REFTYPE);
    m_ref_id      = row.REFID;
    m_description = row.DESCRIPTION;
    m_filename    = row.FILENAME;

    return *this;
}

bool AttachmentData::equals(const AttachmentData* other) const
{
    if ( m_id                 != other->m_id)                return false;
    if ( m_ref_type_n.id_n()  != other->m_ref_type_n.id_n()) return false;
    if ( m_ref_id             != other->m_ref_id)            return false;
    if (!m_description.IsSameAs( other->m_description))      return false;
    if (!m_filename.IsSameAs(    other->m_filename))         return false;

    return true;
}
