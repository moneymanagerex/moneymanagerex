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

#include "AssetData.h"

AssetData::AssetData() :
    m_id(-1),
    m_type(AssetType()),
    m_status(AssetStatus()),
    m_start_date(mmDate::today()),
    m_currency_id_n(-1),
    m_value(0.0),
    m_change(AssetChange()),
    m_change_mode(AssetChangeMode()),
    m_change_rate(0.0)
{
}

// Convert AssetData to AssetRow
AssetRow AssetData::to_row() const
{
    AssetRow row;

    row.ASSETID         = m_id;
    row.STARTDATE       = m_start_date.isoDate();
    row.ASSETNAME       = m_name;
    row.ASSETSTATUS     = m_status.name();
    row.CURRENCYID      = m_currency_id_n;
    row.VALUECHANGEMODE = m_change_mode.name();
    row.VALUE           = m_value;
    row.VALUECHANGE     = m_change.name();
    row.NOTES           = m_notes;
    row.VALUECHANGERATE = m_change_rate;
    row.ASSETTYPE       = m_type.name();

    return row;
}

// Convert AssetRow to AssetData
AssetData& AssetData::from_row(const AssetRow& row)
{
    m_id            = row.ASSETID;
    m_type          = AssetType(row.ASSETTYPE);
    m_status        = AssetStatus(row.ASSETSTATUS);
    m_name          = row.ASSETNAME;
    m_start_date    = mmDate(row.STARTDATE);
    m_currency_id_n = row.CURRENCYID;
    m_value         = row.VALUE;
    m_change        = AssetChange(row.VALUECHANGE);
    m_change_mode   = AssetChangeMode(row.VALUECHANGEMODE);
    m_change_rate   = row.VALUECHANGERATE;
    m_notes         = row.NOTES;

    return *this;
}

bool AssetData::equals(const AssetData* other) const
{
    if ( m_id               != other->m_id)               return false;
    if ( m_type.id()        != other->m_type.id())        return false;
    if ( m_status.id()      != other->m_status.id())      return false;
    if (!m_name.IsSameAs(other->m_name))                  return false;
    if ( m_start_date       != other->m_start_date)       return false;
    if ( m_currency_id_n    != other->m_currency_id_n)    return false;
    if ( m_value            != other->m_value)            return false;
    if ( m_change.id()      != other->m_change.id())      return false;
    if ( m_change_mode.id() != other->m_change_mode.id()) return false;
    if ( m_change_rate      != other->m_change_rate)      return false;
    if (!m_notes.IsSameAs(other->m_notes))                return false;

    return true;
}
