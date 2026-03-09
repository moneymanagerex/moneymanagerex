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

#include "TagLinkData.h"

TagLinkData::TagLinkData() :
    m_id(-1),
    m_tag_id(-1),
    m_ref_type(RefTypeN()),
    m_ref_id(-1)
{
}

// Convert TagLinkData to TagLinkRow
TagLinkRow TagLinkData::to_row() const
{
    TagLinkRow row;

    row.TAGLINKID = m_id;
    row.REFTYPE   = m_ref_type.name_n();
    row.REFID     = m_ref_id;
    row.TAGID     = m_tag_id;

    return row;
}

// Convert TagLinkRow to TagLinkData
TagLinkData& TagLinkData::from_row(const TagLinkRow& row)
{
    m_id       = row.TAGLINKID;
    m_tag_id   = row.TAGID;
    m_ref_type = RefTypeN(row.REFTYPE);
    m_ref_id   = row.REFID;

    return *this;
}

bool TagLinkData::equals(const TagLinkData* other) const
{
    if ( m_id              != other->m_id)              return false;
    if ( m_tag_id          != other->m_tag_id)          return false;
    if ( m_ref_type.id_n() != other->m_ref_type.id_n()) return false;
    if ( m_ref_id          != other->m_ref_id)          return false;

    return true;
}
