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

#ifndef _MM_EX_MMCURRENCY_H_
#define _MM_EX_MMCURRENCY_H_

#include <vector>
#include <wx/sharedptr.h>
#include <map>
#include <wx/string.h>

class wxSQLite3Database;
class wxSQLite3ResultSet;

class mmCurrency
{
public:
    mmCurrency();
    mmCurrency(const wxString& currencySymbol
               , const wxString& currencyName
               , const wxString& pfxSymbol
               , const wxString& sfxSymbol
               , const wxString& unit
               , const wxString& cent
               , double scaleDl
               , double baseConv
              );
    mmCurrency(wxSQLite3ResultSet& q1);

    void loadCurrencySettings();

    /* Data */
    int currencyID_;
    wxString currencyName_;
    wxString pfxSymbol_;
    wxString sfxSymbol_;
    wxString dec_;
    wxString grp_;
    wxString unit_;
    wxString cent_;
    int scaleDl_;
    double baseConv_;
    wxChar decChar_;
    wxChar grpChar_;
    wxString currencySymbol_;
    static std::vector<mmCurrency> currency_map();
};

class mmCurrencyList
{
public:
    mmCurrencyList(wxSharedPtr<wxSQLite3Database> db);
    ~mmCurrencyList();

    void LoadCurrencies();

    /* Currency Functions */
    int AddCurrency(mmCurrency* pCurrency);
    bool DeleteCurrency(int currencyID);
    void UpdateCurrency(mmCurrency* pCurrency);
    int getCurrencyID(const wxString& currencyName, bool symbol = false) const;
    wxString getCurrencyName(int currencyID, bool symbol = false) const;
    mmCurrency* getCurrencySharedPtr(int currencyID) const;
    mmCurrency* getCurrencySharedPtr(const wxString& currencyName, bool symbol = false) const;

    void LoadBaseCurrencySettings() const;
    void LoadCurrencySetting(const wxString& currencySymbol);
    int GetBaseCurrencySettings() const;
    void SetBaseCurrencySettings(int currencyID);
    bool OnlineUpdateCurRate(wxString& sError);

    std::vector< mmCurrency* > currencies_;

    typedef std::vector< mmCurrency* >::const_iterator const_iterator;

private:
    wxSharedPtr<wxSQLite3Database> db_;

    void SetCurrencySetting(mmCurrency* pCurrency) const;
};

#endif
