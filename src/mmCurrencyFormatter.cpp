/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#include "defs.h"
#include "mmCurrencyFormatter.h"
#include "singleton.h"
#include <wx/numformatter.h>

//----------------------------------------------------------------------------
const wxChar g_def_decimal_point = '.';
int g_def_scale = 100;


CurrencyFormatter::CurrencyFormatter()
{
    loadDefaultSettings();
}

void CurrencyFormatter::loadDefaultSettings()
{
    m_pfx_symbol = "$";
    m_sfx_symbol.clear();

    m_decimal_point = g_def_decimal_point;
    m_group_separator = ',';

    m_unit_name = "dollar";
    m_cent_name = "cent";

    m_scale = g_def_scale;
}

void CurrencyFormatter::loadSettings(
    const wxString &pfx,
    const wxString &sfx,
    wxChar dec,
    wxChar grp,
    const wxString &unit,
    const wxString &cent,
    int scale)

{
    m_pfx_symbol = pfx;
    m_sfx_symbol = sfx;

    m_decimal_point = wxIsprint(dec) ? dec : g_def_decimal_point;
    m_group_separator = grp;

    m_unit_name = unit;
    m_cent_name = cent;

    m_scale = scale > 0 ? scale : g_def_scale;
}

void CurrencyFormatter::loadSettings(const mmCurrency &cur)
{
    wxUniChar dec = cur.dec_.IsEmpty() ? wxUniChar('\0') : cur.dec_.GetChar(0);
    wxUniChar grp = cur.grp_.IsEmpty() ? wxUniChar('\0') : cur.grp_.GetChar(0);

    loadSettings(cur.pfxSymbol_, cur.sfxSymbol_, dec, grp, cur.unit_, cur.cent_, cur.scaleDl_);
}

void CurrencyFormatter::loadSettings(const Model_Currency::Data* currency)
{
    if(currency)
    {
        loadSettings(currency->PFX_SYMBOL, currency->SFX_SYMBOL, currency->DECIMAL_POINT.GetChar(0), currency->GROUP_SEPARATOR.GetChar(0),
            currency->UNIT_NAME, currency->CENT_NAME, currency->SCALE);
    }
}

void CurrencyFormatter::loadSettings(const Model_Account::Data* account)
{
    if(account)
    {
        Model_Currency::Data* currency = Model_Account::instance().currency(account);
        loadSettings(currency);
    }
}

CurrencyFormatter& CurrencyFormatter::instance()
{
    return Singleton<CurrencyFormatter>::instance();
}

wxString CurrencyFormatter::float2String(double val)
{
    wxString d2s = wxNumberFormatter::ToString(val, wxNumberFormatter::Style_NoTrailingZeroes); // Style_WithThousandsSep
    //TODO: Remove it with new wx release
    d2s.Replace("-,", "-");
    if (d2s =="-0.00") d2s = "0.00";
    return d2s;
}

wxString CurrencyFormatter::float2Money(double val)
{
    const CurrencyFormatter &fmt = CurrencyFormatter::instance();
    wxString d2s = float2String(val);
    d2s.Prepend(fmt.getPrefix());
    d2s.Append(fmt.getSuffix());
    return d2s;
}

bool CurrencyFormatter::formatCurrencyToDouble(const wxString& str, double& val)
{
    const CurrencyFormatter &fmt = CurrencyFormatter::instance();

    wxString s2d = str;
    const wxString decimal_separator =  wxString::Format("%c", fmt.getDecimalPoint());
    wxString thousand_separator = wxString::Format("%c", fmt.getGroupSeparator());
    const wxString sys_decimal_separator = wxNumberFormatter::GetDecimalSeparator();
    wxString sys_thousand_separator = "";
    wxChar sep = '.';
    if (wxNumberFormatter::GetThousandsSeparatorIfUsed(&sep))
        sys_thousand_separator = wxString::Format("%c", sep);

    if ((decimal_separator == sys_thousand_separator)
        && (sys_decimal_separator == thousand_separator))
    {
        //Swap separators
        s2d.Replace(decimal_separator, "|");
        s2d.Replace(thousand_separator, sys_thousand_separator);
        s2d.Replace("|", sys_decimal_separator);
    }
    else if (thousand_separator.IsEmpty()
        && (decimal_separator == sys_thousand_separator)
        && !s2d.Contains(sys_decimal_separator))
    {
        s2d.Replace(decimal_separator, sys_decimal_separator);
    }

    //TODO: Very strange 'if'. What about Suffix?
    if (!fmt.getPrefix().IsEmpty() && s2d.StartsWith(fmt.getPrefix(), &s2d))
    {
        return wxNumberFormatter::FromString(s2d , &val);
    }
    else
    {
        return wxNumberFormatter::FromString(s2d , &val);
    }
}
