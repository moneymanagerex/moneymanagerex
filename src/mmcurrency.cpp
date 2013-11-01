/*******************************************************
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

#include "mmcurrency.h"
#include "util.h"
#include "mmCurrencyFormatter.h"
#include <map>
#include "model/Model_Infotable.h"

mmCurrency::mmCurrency(wxSQLite3ResultSet& q1)
{
    currencyID_       = q1.GetInt("CURRENCYID");
    currencyName_     = q1.GetString("CURRENCYNAME");
    pfxSymbol_        = q1.GetString("PFX_SYMBOL");
    sfxSymbol_        = q1.GetString("SFX_SYMBOL");
    dec_              = q1.GetString("DECIMAL_POINT");
    grp_              = q1.GetString("GROUP_SEPARATOR");
    unit_             = q1.GetString("UNIT_NAME");
    cent_             = q1.GetString("CENT_NAME");
    scaleDl_          = q1.GetInt("SCALE");
    baseConv_         = q1.GetDouble("BASECONVRATE", 1.0);
    if(q1.GetColumnCount() < 11) {
       /* no Currency symbol in the table yet */
       currencySymbol_ = wxEmptyString;
    } else {
       currencySymbol_ = q1.GetString("CURRENCY_SYMBOL");
    }
    decChar_ = 0;
    grpChar_ = 0;
    if (!dec_.IsEmpty())
    {
      decChar_ = dec_.GetChar(0);
    }

    if (!grp_.IsEmpty())
    {
      grpChar_ = grp_.GetChar(0);
    }
}

mmCurrency::mmCurrency()
    : currencyID_(-1)
    , currencyName_("US Dollar")
    , pfxSymbol_('$')
    , sfxSymbol_()
    , dec_('.')
    , grp_(',')
    , unit_("dollar")
    , cent_("cent")
    , scaleDl_(100)
    , baseConv_(1)
    , decChar_('\0')
    , grpChar_('\0')
    , currencySymbol_("USD")
{}

mmCurrency::mmCurrency(const wxString& currencySymbol
                       , const wxString& currencyName
                       , const wxString& pfxSymbol
                       , const wxString& sfxSymbol
                       , const wxString& unit
                       , const wxString& cent
                       , double scaleDl
                       , double baseConv
                       )
                       :
currencySymbol_(currencySymbol)
    , currencyName_(currencyName)
    , pfxSymbol_(pfxSymbol)
    , sfxSymbol_(sfxSymbol)
    , unit_(unit)
    , cent_(cent)
    , scaleDl_(scaleDl)
    , baseConv_(baseConv)
{
    currencyID_ = -1;
    dec_ = '.';
    grp_ = ',';
    decChar_ = '\0';
    grpChar_ ='\0';
}

void mmCurrency::loadCurrencySettings()
{
    CurrencyFormatter::instance().loadSettings(
        pfxSymbol_,
        sfxSymbol_,
        decChar_,
        grpChar_,
        unit_,
        cent_,
        scaleDl_
    );
}

