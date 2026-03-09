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

#include "CurrencyData.h"

CurrencyData::CurrencyData() :
    m_id(-1),
    m_type(CurrencyType()),
    m_scale(-1),
    m_base_conv_rate(0.0)
{
}

// Convert CurrencyData to CurrencyRow
CurrencyRow CurrencyData::to_row() const
{
    CurrencyRow row;

    row.CURRENCYID      = m_id;
    row.CURRENCYNAME    = m_name;
    row.PFX_SYMBOL      = m_prefix_symbol;
    row.SFX_SYMBOL      = m_suffix_symbol;
    row.DECIMAL_POINT   = m_decimal_point;
    row.GROUP_SEPARATOR = m_group_separator;
    row.UNIT_NAME       = m_unit_name;
    row.CENT_NAME       = m_cent_name;
    row.SCALE           = m_scale;
    row.BASECONVRATE    = m_base_conv_rate;
    row.CURRENCY_SYMBOL = m_symbol;
    row.CURRENCY_TYPE   = m_type.name();

    return row;
}

// Convert CurrencyRow to CurrencyData
CurrencyData& CurrencyData::from_row(const CurrencyRow& row)
{
    m_id              = row.CURRENCYID;
    m_symbol          = row.CURRENCY_SYMBOL;
    m_name            = row.CURRENCYNAME;
    m_type            = CurrencyType(row.CURRENCY_TYPE);
    m_prefix_symbol   = row.PFX_SYMBOL;
    m_suffix_symbol   = row.SFX_SYMBOL;
    m_decimal_point   = row.DECIMAL_POINT;
    m_group_separator = row.GROUP_SEPARATOR;
    m_unit_name       = row.UNIT_NAME;
    m_cent_name       = row.CENT_NAME;
    m_scale           = row.SCALE;
    m_base_conv_rate  = row.BASECONVRATE;

    return *this;
}

bool CurrencyData::equals(const CurrencyData* other) const
{
    if ( m_id != other->m_id) return false;
    if (!m_symbol.IsSameAs(other->m_symbol)) return false;
    if (!m_name.IsSameAs(other->m_name)) return false;
    if ( m_type.id() != other->m_type.id()) return false;
    if (!m_prefix_symbol.IsSameAs(other->m_prefix_symbol)) return false;
    if (!m_suffix_symbol.IsSameAs(other->m_suffix_symbol)) return false;
    if (!m_decimal_point.IsSameAs(other->m_decimal_point)) return false;
    if (!m_group_separator.IsSameAs(other->m_group_separator)) return false;
    if (!m_unit_name.IsSameAs(other->m_unit_name)) return false;
    if (!m_cent_name.IsSameAs(other->m_cent_name)) return false;
    if ( m_scale != other->m_scale) return false;
    if ( m_base_conv_rate != other->m_base_conv_rate) return false;

    return true;
}
