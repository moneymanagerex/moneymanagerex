/*******************************************************
 Copyright (C) 2013 Stefano Giorgio

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

#include "currency.h"
#include "../constants.h"
#include "../mmCurrencyFormatter.h"

/************************************************************************************
 TCurrencyEntry Methods
 ***********************************************************************************/
/// Constructor used when loading currencies from the database
TCurrencyEntry::TCurrencyEntry(wxSQLite3ResultSet& q1)
: TEntryBase()
{
    id_         = q1.GetInt("CURRENCYID");
    name_       = q1.GetString("CURRENCYNAME");
    pfxSymbol_  = q1.GetString("PFX_SYMBOL");
    sfxSymbol_  = q1.GetString("SFX_SYMBOL");
    dec_        = q1.GetString("DECIMAL_POINT");
    grp_        = q1.GetString("GROUP_SEPARATOR");
    unit_       = q1.GetString("UNIT_NAME");
    cent_       = q1.GetString("CENT_NAME");
    scaleDl_    = q1.GetInt("SCALE");
    baseConv_   = q1.GetDouble("BASECONVRATE", 1.0);
    if(q1.GetColumnCount() < 11)
    {
        /* no Currency symbol in the table yet */
        currencySymbol_ = wxEmptyString;
    }
    else
    {
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

/// Constructor for creating a new currency entry
TCurrencyEntry::TCurrencyEntry()
: TEntryBase()
, name_(CURRENCIES[TCurrencyEntry::NAME_USD])
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
, currencySymbol_(CURRENCIES[TCurrencyEntry::SYMBOL_USD])
{}

void TCurrencyEntry::SetDatabaseValues(wxSQLite3Statement& st, int& db_index)
{
    st.Bind(++db_index, name_);
    st.Bind(++db_index, pfxSymbol_);
    st.Bind(++db_index, sfxSymbol_);
    st.Bind(++db_index, dec_);
    st.Bind(++db_index, grp_);
    st.Bind(++db_index, unit_);
    st.Bind(++db_index, cent_);
    st.Bind(++db_index, scaleDl_);
    st.Bind(++db_index, baseConv_);
    st.Bind(++db_index, currencySymbol_);
}

int TCurrencyEntry::Add(wxSQLite3Database* db)
{
    try
    {
        const char SQL_STATEMENT[] =
        "insert into CURRENCYFORMATS_V1 "
        "(CURRENCYNAME, PFX_SYMBOL, SFX_SYMBOL, DECIMAL_POINT, GROUP_SEPARATOR,"
        " UNIT_NAME, CENT_NAME, SCALE, BASECONVRATE, CURRENCY_SYMBOL) "
        "values ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )";

        wxSQLite3Statement st = db->PrepareStatement(SQL_STATEMENT);
        int db_index = 0;
        SetDatabaseValues(st, db_index);

        FinaliseAdd(db, st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TCurrencyEntry:Add: %s", e.GetMessage().c_str());
    }

    return id_;
}

void TCurrencyEntry::Delete(wxSQLite3Database* db)
{
    DeleteEntry(db, "delete from CURRENCYFORMATS_V1 where CURRENCYID = ?");
}

void TCurrencyEntry::Update(wxSQLite3Database* db)
{
    try
    {
        const char SQL_STATEMENT[] =
        "update CURRENCYFORMATS_V1 set"
        " CURRENCYNAME = ?, PFX_SYMBOL = ?, SFX_SYMBOL = ?, DECIMAL_POINT = ?, GROUP_SEPARATOR = ?,"
        " UNIT_NAME = ?, CENT_NAME = ?, SCALE = ?, BASECONVRATE = ?, CURRENCY_SYMBOL = ? "
        "where CURRENCYID = ?";
        
        wxSQLite3Statement st = db->PrepareStatement(SQL_STATEMENT);
        int db_index = 0;
        SetDatabaseValues(st, db_index);
        st.Bind(++db_index, id_);

        FinaliseStatement(st);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError("TCurrencyEntry:Update: %s", e.GetMessage().c_str());
    }
}

void TCurrencyEntry::SetCurrencySettings()
{
    CurrencyFormatter::instance().loadSettings(pfxSymbol_, sfxSymbol_,
        decChar_, grpChar_, unit_, cent_, scaleDl_);
}

std::map<wxString, wxString> TCurrencyEntry::CURRENCIES_MAP() 
{
    std::map<wxString, wxString> curr;
    curr["AFN"] = "Afghanistan Afghani";
    curr["ALL"] = "Albania Lek";
    curr["DZD"] = "Algeria Dinar";
    curr["AOA"] = "Angola Kwanza";
    curr["ARS"] = "Argentina Peso";
    curr["AMD"] = "Armenia Dram";
    curr["AWG"] = "Aruba Guilder";
    curr["AUD"] = "Australia Dollar";
    curr["AZN"] = "Azerbaijan New Manat";
    curr["BSD"] = "Bahamas Dollar";
    curr["BHD"] = "Bahrain Dinar";
    curr["BDT"] = "Bangladesh Taka";
    curr["BBD"] = "Barbados Dollar";
    curr["BYR"] = "Belarus Ruble";
    curr["BZD"] = "Belize Dollar";
    curr["BMD"] = "Bermuda Dollar";
    curr["BTN"] = "Bhutan Ngultrum";
    curr["BOB"] = "Bolivia Boliviano";
    curr["BAM"] = "Bosnia and Herzegovina Convertible Marka";
    curr["BWP"] = "Botswana Pula";
    curr["BRL"] = "Brazil Real";
    curr["BND"] = "Brunei Darussalam Dollar";
    curr["BGN"] = "Bulgaria Lev";
    curr["BIF"] = "Burundi Franc";
    curr["KHR"] = "Cambodia Riel";
    curr["CAD"] = "Canada Dollar";
    curr["CVE"] = "Cape Verde Escudo";
    curr["KYD"] = "Cayman Islands Dollar";
    curr["CLP"] = "Chile Peso";
    curr["CNY"] = "China Yuan Renminbi";
    curr["XOF"] = L"Communaut\u00E9 Financi\u00E8re Africaine (BCEAO) Franc";
    curr["COP"] = "Colombia Peso";
    curr["KMF"] = "Comoros Franc";
    curr["XPF"] = "Comptoirs Francais du Pacifique (CFP) Franc";
    curr["XPF"] = L"Comptoirs Fran\u00E7ais du Pacifique (CFP) Franc";
    curr["CDF"] = "Congo/Kinshasa Franc";
    curr["CRC"] = "Costa Rica Colon";
    curr["HRK"] = "Croatia Kuna";
    curr["CUC"] = "Cuba Convertible Peso";
    curr["CUP"] = "Cuba Peso";
    curr["CZK"] = "Czech Republic Koruna";
    curr["DKK"] = "Denmark Krone";
    curr["DJF"] = "Djibouti Franc";
    curr["DOP"] = "Dominican Republic Peso";
    curr["XCD"] = "East Caribbean Dollar";
    curr["EGP"] = "Egypt Pound";
    curr["SVC"] = "El Salvador Colon";
    curr["ERN"] = "Eritrea Nakfa";
    curr["ETB"] = "Ethiopia Birr";
    curr["EUR"] = "Euro Member Countries";
    curr["FKP"] = "Falkland Islands (Malvinas) Pound";
    curr["FJD"] = "Fiji Dollar";
    curr["GMD"] = "Gambia Dalasi";
    curr["GEL"] = "Georgia Lari";
    curr["GHS"] = "Ghana Cedi";
    curr["GIP"] = "Gibraltar Pound";
    curr["GTQ"] = "Guatemala Quetzal";
    curr["GGP"] = "Guernsey Pound";
    curr["GNF"] = "Guinea Franc";
    curr["GYD"] = "Guyana Dollar";
    curr["HTG"] = "Haiti Gourde";
    curr["HNL"] = "Honduras Lempira";
    curr["HKD"] = "Hong Kong Dollar";
    curr["HUF"] = "Hungary Forint";
    curr["ISK"] = "Iceland Krona";
    curr["INR"] = "India Rupee";
    curr["IDR"] = "Indonesia Rupiah";
    curr["XDR"] = "International Monetary Fund (IMF) Special Drawing Rights";
    curr["IRR"] = "Iran Rial";
    curr["IQD"] = "Iraq Dinar";
    curr["IMP"] = "Isle of Man Pound";
    curr["ILS"] = "Israel Shekel";
    curr["JMD"] = "Jamaica Dollar";
    curr["JPY"] = "Japan Yen";
    curr["JEP"] = "Jersey Pound";
    curr["JOD"] = "Jordan Dinar";
    curr["KZT"] = "Kazakhstan Tenge";
    curr["KES"] = "Kenya Shilling";
    curr["KPW"] = "Korea (North) Won";
    curr["KRW"] = "Korea (South) Won";
    curr["KWD"] = "Kuwait Dinar";
    curr["KGS"] = "Kyrgyzstan Som";
    curr["LAK"] = "Laos Kip";
    curr["LVL"] = "Latvia Lat";
    curr["LBP"] = "Lebanon Pound";
    curr["LSL"] = "Lesotho Loti";
    curr["LRD"] = "Liberia Dollar";
    curr["LYD"] = "Libya Dinar";
    curr["LTL"] = "Lithuania Litas";
    curr["MOP"] = "Macau Pataca";
    curr["MKD"] = "Macedonia Denar";
    curr["MGA"] = "Madagascar Ariary";
    curr["MWK"] = "Malawi Kwacha";
    curr["MYR"] = "Malaysia Ringgit";
    curr["MVR"] = "Maldives (Maldive Islands) Rufiyaa";
    curr["MRO"] = "Mauritania Ouguiya";
    curr["MUR"] = "Mauritius Rupee";
    curr["MXN"] = "Mexico Peso";
    curr["MDL"] = "Moldova Leu";
    curr["MNT"] = "Mongolia Tughrik";
    curr["MAD"] = "Morocco Dirham";
    curr["MZN"] = "Mozambique Metical";
    curr["MMK"] = "Myanmar (Burma) Kyat";
    curr["NAD"] = "Namibia Dollar";
    curr["NPR"] = "Nepal Rupee";
    curr["ANG"] = "Netherlands Antilles Guilder";
    curr["NZD"] = "New Zealand Dollar";
    curr["NIO"] = "Nicaragua Cordoba";
    curr["NGN"] = "Nigeria Naira";
    curr["NOK"] = "Norway Krone";
    curr["OMR"] = "Oman Rial";
    curr["PKR"] = "Pakistan Rupee";
    curr["PAB"] = "Panama Balboa";
    curr["PGK"] = "Papua New Guinea Kina";
    curr["PYG"] = "Paraguay Guarani";
    curr["PEN"] = "Peru Nuevo Sol";
    curr["PHP"] = "Philippines Peso";
    curr["PLN"] = "Poland Zloty";
    curr["QAR"] = "Qatar Riyal";
    curr["RON"] = "Romania New Leu";
    curr["RUB"] = "Russia Ruble";
    curr["RWF"] = "Rwanda Franc";
    curr["SHP"] = "Saint Helena Pound";
    curr["WST"] = "Samoa Tala";
    curr["STD"] = L"S\u00E3o Tom\u00E9 and Pr\u00EDncipe Dobra";
    curr["SAR"] = "Saudi Arabia Riyal";
    curr["RSD"] = "Serbia Dinar";
    curr["SCR"] = "Seychelles Rupee";
    curr["SLL"] = "Sierra Leone Leone";
    curr["SGD"] = "Singapore Dollar";
    curr["SBD"] = "Solomon Islands Dollar";
    curr["SOS"] = "Somalia Shilling";
    curr["ZAR"] = "South Africa Rand";
    curr["LKR"] = "Sri Lanka Rupee";
    curr["SDG"] = "Sudan Pound";
    curr["SRD"] = "Suriname Dollar";
    curr["SZL"] = "Swaziland Lilangeni";
    curr["SEK"] = "Sweden Krona";
    curr["CHF"] = "Switzerland Franc";
    curr["SYP"] = "Syria Pound";
    curr["TWD"] = "Taiwan New Dollar";
    curr["TJS"] = "Tajikistan Somoni";
    curr["TZS"] = "Tanzania Shilling";
    curr["THB"] = "Thailand Baht";
    curr["TOP"] = "Tonga Pa'anga";
    curr["TTD"] = "Trinidad and Tobago Dollar";
    curr["TND"] = "Tunisia Dinar";
    curr["TRY"] = "Turkey Lira";
    curr["TMT"] = "Turkmenistan Manat";
    curr["TVD"] = "Tuvalu Dollar";
    curr["UGX"] = "Uganda Shilling";
    curr["UAH"] = "Ukraine Hryvna";
    curr["AED"] = "United Arab Emirates Dirham";
    curr["GBP"] = "United Kingdom Pound";
    curr["USD"] = "United States Dollar";
    curr["UYU"] = "Uruguay Peso";
    curr["UZS"] = "Uzbekistan Som";
    curr["VUV"] = "Vanuatu Vatu";
    curr["VEF"] = "Venezuela Bolivar";
    curr["VND"] = "Viet Nam Dong";
    curr["YER"] = "Yemen Rial";
    curr["ZMK"] = "Zambia Kwacha";
    curr["ZWD"] = "Zimbabwe Dollar";
	return curr;
};

/************************************************************************************
 TCurrencyList Methods
 ***********************************************************************************/
/// Constructor
TCurrencyList::TCurrencyList(wxSQLite3Database* db)
: TListBase(db)
, basecurrency_id_(-1)
{
    CurrencyFormatter::instance().loadDefaultSettings();
    LoadEntries();
}

TCurrencyList::~TCurrencyList()
{
    DestroyEntryList();
}

void TCurrencyList::LoadEntries(bool load_entries)
{
    try
    {
        if (!ListDatabase()->TableExists("CURRENCYFORMATS_V1"))
        {
            const char SQL_CREATE_TABLE_CURRENCYFORMATS_V1[] =
            "create table CURRENCYFORMATS_V1 "
            "(CURRENCYID integer primary key, CURRENCYNAME TEXT NOT NULL UNIQUE,"
            " PFX_SYMBOL TEXT, SFX_SYMBOL TEXT, DECIMAL_POINT TEXT, GROUP_SEPARATOR TEXT,"
            " UNIT_NAME TEXT, CENT_NAME TEXT, SCALE numeric, BASECONVRATE numeric, CURRENCY_SYMBOL TEXT)";
            
            ListDatabase()->ExecuteUpdate(SQL_CREATE_TABLE_CURRENCYFORMATS_V1);
        }

        if (load_entries)
        {
            LoadEntriesUsing("select * from CURRENCYFORMATS_V1 order by CURRENCYNAME");
        }
    }
    catch (const wxSQLite3Exception& e)
    {
        wxLogError("TCurrencyList:LoadEntries: %s", e.GetMessage().c_str());
    }
}

void TCurrencyList::LoadEntriesUsing(const wxString& sql_statement)
{
    DestroyEntryList();
    wxSQLite3ResultSet q1 = ListDatabase()->ExecuteQuery(sql_statement);
    while (q1.NextRow())
    {
        entrylist_.push_back(new TCurrencyEntry(q1));
    }
    q1.Finalize();
}

void TCurrencyList::DestroyEntryList()
{
    for (size_t i = 0; i < entrylist_.size(); ++i)
    {
        delete entrylist_[i];
    }
    entrylist_.clear();
}

int TCurrencyList::AddEntry(TCurrencyEntry* pCurrencyEntry)
{
    int currency_id = -1;
    if (CurrencyExists(pCurrencyEntry->name_) || CurrencyExists(pCurrencyEntry->currencySymbol_, true))
    {
        currency_id = entrylist_[current_index_]->id_;
    }
    else
    {
        entrylist_.push_back(pCurrencyEntry);
        pCurrencyEntry->Add(ListDatabase());

        currency_id = pCurrencyEntry->id_;
    }

    return currency_id;
}

void TCurrencyList::SetBaseCurrency(int currency_id)
{
    TCurrencyEntry* pEntry = GetEntryPtr(currency_id);
    if (pEntry)
    {
        basecurrency_id_ = currency_id;
        pEntry->SetCurrencySettings();
    }
    else
    {
        basecurrency_id_ = -1;
        CurrencyFormatter::instance().loadDefaultSettings();
    }
}

void TCurrencyList::DeleteEntry(int currency_id)
{
    TCurrencyEntry* pEntry = GetEntryPtr(currency_id);
    if (pEntry)
    {
        pEntry->Delete(ListDatabase());
        entrylist_.erase(entrylist_.begin() + current_index_);
        delete pEntry;
    }
}

void TCurrencyList::DeleteEntry(const wxString& name, bool is_symbol)
{
    DeleteEntry(GetCurrencyId(name, is_symbol));
}

//-----------------------------------------------------------------------------

TCurrencyEntry* TCurrencyList::GetEntryPtr(int currency_id)
{
    TCurrencyEntry* pEntry = 0;
    size_t index = 0;

    while (index < entrylist_.size())
    {
        if (entrylist_[index]->id_ == currency_id)
        {
            pEntry = entrylist_[index];
            current_index_ = index;
            break;
        }
        ++ index;
    }

    return pEntry;
}

TCurrencyEntry* TCurrencyList::GetEntryPtr(const wxString& name, bool is_symbol)
{
    TCurrencyEntry* pEntry = 0;
    size_t index = 0;
    bool found = false;
    while (index < entrylist_.size())
    {
        if (is_symbol)
        {
            if (entrylist_[index]->currencySymbol_ == name)
                found = true;
        }
        else
        {
            if (entrylist_[index]->name_ == name)
                found = true;
        }

        if (found)
        {
            pEntry = entrylist_[index];
            current_index_ = index;
            break;
        }
        ++ index;
    }

    return pEntry;
}

TCurrencyEntry* TCurrencyList::GetIndexedEntryPtr(int index)
{
    return entrylist_[index];
}

int TCurrencyList::GetCurrencyId(const wxString& name, bool is_symbol)
{
    int currency_id = -1;
    TCurrencyEntry* pEntry = GetEntryPtr(name, is_symbol);
    if (pEntry)
    {
        currency_id = pEntry->GetId();
    }

    return currency_id;
}

wxString TCurrencyList::GetCurrencyName(int currency_id)
{
    wxString currency_name;
    TCurrencyEntry* pEntry = GetEntryPtr(currency_id);
    if (pEntry)
    {
        currency_name = pEntry->name_;
    }

    return currency_name;
}

bool TCurrencyList::CurrencyExists(const wxString& name, bool is_symbol)
{
    bool result = false;
    if (GetEntryPtr(name, is_symbol))
    {
        result = true;
    }
    return result;
}
