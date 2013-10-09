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

#ifndef _MM_EX_CURRENCY_FORMATEER_H_
#define _MM_EX_CURRENCY_FORMATEER_H_

#include "mmcurrency.h"
#include "model\Model_Account.h"
#include <math.h>

class CurrencyFormatter
{
public:
    CurrencyFormatter();
    static CurrencyFormatter& instance();

    void loadDefaultSettings();
    void loadSettings(const mmCurrency &cur);
    void loadSettings(const Model_Currency::Data* currency);
    void loadSettings(const Model_Account::Data* account);

    void loadSettings(const wxString &pfx, const wxString &sfx,
                      wxChar dec, wxChar grp,
                      const wxString &unit, const wxString &cent,
                      int scale
                     );

    wxString getPrefix() const
    {
        return m_pfx_symbol;
    }
    wxString getSuffix() const
    {
        return m_sfx_symbol;
    }

    wxChar getDecimalPoint() const
    {
        return m_decimal_point;
    }
    wxChar getGroupSeparator() const
    {
        return m_group_separator;
    }

    int getScale() const
    {
        return m_scale;
    }
    int getDec() const
    {
        return log10(m_scale);
    }

private:
    wxString m_pfx_symbol;   // Leading currency symbol
    wxString m_sfx_symbol;   // Trailing currency symbol
    wxChar m_decimal_point;  // Character for 100ths
    wxChar m_group_separator;// Character for 1000nds
    wxString m_unit_name;    // Name of monetary unit
    wxString m_cent_name;    // Name of fraction unit
    int m_scale;

public:
    static wxString float2String(double val);
    static wxString float2Money(double val);
    static bool formatCurrencyToDouble(const wxString& str, double& val );
};

#endif // _MM_EX_CURRENCY_FORMATEER_H_
