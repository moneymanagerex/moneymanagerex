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

#include "currencydialog.h"
#include "constants.h"
#include "util.h"
#include "mmCurrencyFormatter.h"
#include "defs.h"
#include "paths.h"
#include "validators.h"

#include <wx/combobox.h>
#include <wx/valnum.h>

#ifndef __VISUALC__
#define INT_PTR intptr_t
#endif

IMPLEMENT_DYNAMIC_CLASS( mmCurrencyDialog, wxDialog )

BEGIN_EVENT_TABLE( mmCurrencyDialog, wxDialog )
    EVT_BUTTON(ID_DIALOG_CURRENCY_BUTTON_UPDATE, mmCurrencyDialog::OnUpdate)
END_EVENT_TABLE()

mmCurrencyDialog::mmCurrencyDialog( )
{
    core_ = 0;
    currencyID_ = -1;
}

mmCurrencyDialog::~mmCurrencyDialog()
{
    currencyID_ = -1;
}

mmCurrencyDialog::mmCurrencyDialog( mmCoreDB* core, wxWindow* parent,
                                   wxWindowID id, const wxString& caption,
                                   const wxPoint& pos, const wxSize& size, long style )
{
    core_ = core;
    currencyID_ = -1;
    Create(parent, id, caption, pos, size, style);
}

mmCurrencyDialog::mmCurrencyDialog(mmCoreDB* core,  int currencyID, wxWindow* parent,
                                   wxWindowID id, const wxString& caption,
                                   const wxPoint& pos, const wxSize& size, long style )
{
    core_ = core;
    currencyID_ = currencyID;
    Create(parent, id, caption, pos, size, style);
}

bool mmCurrencyDialog::Create( wxWindow* parent, wxWindowID id,
                              const wxString& caption, const wxPoint& pos,
                              const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    fillControls();

    Centre();
    return TRUE;
}

void mmCurrencyDialog::fillControls()
{
    if (!core_)
       return;

    wxSortedArrayString currency_names;
    wxSortedArrayString currency_symbols;
    for(const auto& i : this->currency_map())
    {
        currency_symbols.Add(i.first);
        currency_names.Add(i.second.currencyName_);
        currencySymbolCombo_->Append(i.first);
    }
    
    for (const auto &i : currency_names)
        currencyNameCombo_->Append(i);
    currencyNameCombo_->AutoComplete(currency_names);
    currencySymbolCombo_->AutoComplete(currency_symbols);

    if (currencyID_ != -1)
    {
        wxString name = core_->currencyList_.getCurrencyName(currencyID_);
        currencyNameCombo_->Append(name, (void*)(INT_PTR)currencyID_);
        currencyNameCombo_->SetValue(name);
    }
    else
    {
       currencyNameCombo_->SetValue("");
    }

    currencyNameCombo_->Connect(ID_DIALOG_CURRENCY_CHOICE, wxEVT_COMMAND_COMBOBOX_SELECTED,
        wxCommandEventHandler(mmCurrencyDialog::OnCurrencyNameSelected), NULL, this);
    currencyNameCombo_->Connect(ID_DIALOG_CURRENCY_CHOICE, wxEVT_COMMAND_TEXT_UPDATED,
        wxCommandEventHandler(mmCurrencyDialog::OnCurrencyNameSelected), NULL, this);

    updateControls();
}

std::map<wxString, mmCurrency> mmCurrencyDialog::currency_map() const
{
    std::map<wxString, mmCurrency> currency_map;
	currency_map["USD"] = mmCurrency("USD", "United States dollar", L"US$", L"", L"", L"", 100,1);
	currency_map["EUR"] = mmCurrency("EUR", "European euro", L"€", L"", L"", L"", 100,1);
    currency_map["GBP"] = mmCurrency("GBP", "UK Pound", L"£", "", "Pound", "Pence", 100, 1);
    currency_map["RUB"] = mmCurrency("RUB", "Russian Ruble", "", "р", "руб.", "коп.", 100, 1);
	currency_map["UAH"] = mmCurrency("UAH", "Ukrainian hryvnia", L"₴", L"", L"", L"", 100, 1);
	currency_map["AFN"] = mmCurrency("AFN", "Afghan afghani", L"", L"", L"", L"", 1	, 1);
	currency_map["ALL"] = mmCurrency("ALL", "Albanian lek", L"", L"", L"", L"", 1, 1);
	currency_map["DZD"] = mmCurrency("DZD", "Algerian dinar", L"", L"", L"", L"", 100, 1);
	currency_map["AOA"] = mmCurrency("AOA", "Angolan kwanza", L"", L"", L"", L"", 100, 1);
	currency_map["XCD"] = mmCurrency("XCD", "East Caribbean dollar", L"EC$", L"", L"", L"", 100, 1);
	currency_map["XCD"] = mmCurrency("XCD", "East Caribbean dollar", L"EC$", L"", L"", L"", 100, 1);
	currency_map["ARS"] = mmCurrency("ARS", "Argentine peso", L"", L"", L"", L"", 100, 1);
	currency_map["AMD"] = mmCurrency("AMD", "Armenian dram", L"", L"", L"", L"", 1, 1);
	currency_map["AWG"] = mmCurrency("AWG", "Aruban florin", L"ƒ", L"", L"", L"", 100, 1);
	currency_map["AUD"] = mmCurrency("AUD", "Australian dollar", L"$", L"", L"", L"", 100, 1);
	currency_map["AZN"] = mmCurrency("AZN", "Azerbaijani manat", L"", L"", L"", L"", 100, 1);
	currency_map["BSD"] = mmCurrency("BSD", "Bahamian dollar", L"B$", L"", L"", L"", 100, 1);
	currency_map["BHD"] = mmCurrency("BHD", "Bahraini dinar", L"", L"", L"", L"", 100, 1);
	currency_map["BDT"] = mmCurrency("BDT", "Bangladeshi taka", L"", L"", L"", L"", 100, 1);
	currency_map["BBD"] = mmCurrency("BBD", "Barbadian dollar", L"Bds$", L"", L"", L"", 100, 1);
	currency_map["BYR"] = mmCurrency("BYR", "Belarusian ruble", L"Br", L"", L"", L"",1,1);
	currency_map["BZD"] = mmCurrency("BZD", "Belize dollar", L"BZ$", L"", L"", L"", 100, 1);
	currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
	currency_map["BMD"] = mmCurrency("BMD", "Bermudian dollar", L"BD$", L"", L"", L"", 100, 1);
	currency_map["BTN"] = mmCurrency("BTN", "Bhutanese ngultrum", L"Nu.", L"", L"", L"", 100, 1);
	currency_map["BOB"] = mmCurrency("BOB", "Bolivian boliviano", L"Bs.", L"", L"", L"", 100, 1);
	currency_map["BAM"] = mmCurrency("BAM", "Bosnia and Herzegovina konvertibilna marka", L"KM", L"", L"", L"", 100, 1);
	currency_map["BWP"] = mmCurrency("BWP", "Botswana pula", L"P", L"", L"", L"", 100, 1);
	currency_map["BRL"] = mmCurrency("BRL", "Brazilian real", L"R$", L"", L"", L"", 100, 1);
	currency_map["BND"] = mmCurrency("BND", "Brunei dollar", L"B$", L"", L"", L"", 100, 1);
	currency_map["BGN"] = mmCurrency("BGN", "Bulgarian lev", L"", L"", L"", L"", 100, 1);
	currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
	currency_map["BIF"] = mmCurrency("BIF", "Burundi franc", L"FBu", L"", L"", L"", 1, 1);
	currency_map["KHR"] = mmCurrency("KHR", "Cambodian riel", L"", L"", L"", L"", 100, 1);
	currency_map["XAF"] = mmCurrency("XAF", "Central African CFA franc", L"CFA", L"", L"", L"", 1, 1);
	currency_map["CAD"] = mmCurrency("CAD", "Canadian dollar", L"$", L"", L"", L"", 100, 1);
	currency_map["CVE"] = mmCurrency("CVE", "Cape Verdean escudo", L"Esc", L"", L"", L"", 100, 1);
	currency_map["KYD"] = mmCurrency("KYD", "Cayman Islands dollar", L"KY$", L"", L"", L"", 100, 1);
	currency_map["XAF"] = mmCurrency("XAF", "Central African CFA franc", L"CFA", L"", L"", L"", 100, 1);
	currency_map["XAF"] = mmCurrency("XAF", "Central African CFA franc", L"CFA", L"", L"", L"", 100, 1);
	currency_map["CLP"] = mmCurrency("CLP", "Chilean peso", L"$", L"", L"", L"", 1, 1);
	currency_map["CNY"] = mmCurrency("CNY", "Chinese renminbi", L"¥", L"", L"", L"", 100, 1);
	currency_map["COP"] = mmCurrency("COP", "Colombian peso", L"Col$", L"", L"", L"", 100, 1);
	currency_map["KMF"] = mmCurrency("KMF", "Comorian franc", L"", L"", L"", L"", 1, 1);
	currency_map["XAF"] = mmCurrency("XAF", "Central African CFA franc", L"CFA", L"", L"", L"", 100, 1);
	currency_map["CDF"] = mmCurrency("CDF", "Congolese franc", L"F", L"", L"", L"", 100, 1);
	currency_map["CRC"] = mmCurrency("CRC", "Costa Rican colon", L"₡", L"", L"", L"", 1, 1);
	currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 1, 1);
	currency_map["HRK"] = mmCurrency("HRK", "Croatian kuna", L"kn", L"", L"", L"", 100, 1);
	currency_map["CUC"] = mmCurrency("CUC", "Cuban peso", L"$", L"", L"", L"", 100, 1);
	currency_map["CZK"] = mmCurrency("CZK", "Czech koruna", L"Kč", L"", L"", L"", 100, 1);
	currency_map["DKK"] = mmCurrency("DKK", "Danish krone", L"Kr", L"", L"", L"", 100, 1);
	currency_map["DJF"] = mmCurrency("DJF", "Djiboutian franc", L"Fdj", L"", L"", L"", 1, 1);
	currency_map["XCD"] = mmCurrency("XCD", "East Caribbean dollar", L"EC$", L"", L"", L"", 100, 1);
	currency_map["DOP"] = mmCurrency("DOP", "Dominican peso", L"RD$", L"", L"", L"", 100, 1);
	currency_map["EGP"] = mmCurrency("EGP", "Egyptian pound", L"£", L"", L"", L"", 100, 1);
	currency_map["GQE"] = mmCurrency("GQE", "Central African CFA franc", L"CFA", L"", L"", L"", 100, 1);
	currency_map["ERN"] = mmCurrency("ERN", "Eritrean nakfa", L"Nfa", L"", L"", L"", 100, 1);
	currency_map["EEK"] = mmCurrency("EEK", "Estonian kroon", L"KR", L"", L"", L"", 100, 1);
	currency_map["ETB"] = mmCurrency("ETB", "Ethiopian birr", L"Br", L"", L"", L"", 100, 1);
	currency_map["FKP"] = mmCurrency("FKP", "Falkland Islands pound", L"£", L"", L"", L"", 100, 1);
	currency_map["FJD"] = mmCurrency("FJD", "Fijian dollar", L"FJ$", L"", L"", L"", 100, 1);
	currency_map["XPF"] = mmCurrency("XPF", "CFP franc", L"F", L"", L"", L"", 100, 1);
	currency_map["XAF"] = mmCurrency("XAF", "Central African CFA franc", L"CFA", L"", L"", L"", 100, 1);
	currency_map["GMD"] = mmCurrency("GMD", "Gambian dalasi", L"D", L"", L"", L"", 100, 1);
	currency_map["GEL"] = mmCurrency("GEL", "Georgian lari", L"", L"", L"", L"", 100, 1);
	currency_map["GHS"] = mmCurrency("GHS", "Ghanaian cedi", L"", L"", L"", L"", 100, 1);
	currency_map["GIP"] = mmCurrency("GIP", "Gibraltar pound", L"£", L"", L"", L"", 100, 1);
	currency_map["XCD"] = mmCurrency("XCD", "East Caribbean dollar", L"EC$", L"", L"", L"", 100, 1);
	currency_map["GTQ"] = mmCurrency("GTQ", "Guatemalan quetzal", L"Q", L"", L"", L"", 100, 1);
	currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
	currency_map["GNF"] = mmCurrency("GNF", "Guinean franc", L"FG", L"", L"", L"", 1, 1);
	currency_map["GYD"] = mmCurrency("GYD", "Guyanese dollar", L"GY$", L"", L"", L"", 100, 1);
	currency_map["HTG"] = mmCurrency("HTG", "Haitian gourde", L"G", L"", L"", L"", 100, 1);
	currency_map["HNL"] = mmCurrency("HNL", "Honduran lempira", L"L", L"", L"", L"", 100, 1);
	currency_map["HKD"] = mmCurrency("HKD", "Hong Kong dollar", L"HK$", L"", L"", L"", 100, 1);
	currency_map["HUF"] = mmCurrency("HUF", "Hungarian forint", L"Ft", L"", L"", L"", 1, 1);
	currency_map["ISK"] = mmCurrency("ISK", "Icelandic króna", L"kr", L"", L"", L"", 1, 1);
	currency_map["INR"] = mmCurrency("INR", "Indian rupee", L"", L"", L"", L"", 100, 1);
	currency_map["IDR"] = mmCurrency("IDR", "Indonesian rupiah", L"Rp", L"", L"", L"", 1, 1);
	currency_map["XDR"] = mmCurrency("XDR", "Special Drawing Rights", L"SDR", L"", L"", L"", 100, 1);
	currency_map["IRR"] = mmCurrency("IRR", "Iranian rial", L"", L"", L"", L"", 1, 1);
	currency_map["IQD"] = mmCurrency("IQD", "Iraqi dinar", L"", L"", L"", L"", 1, 1);
	currency_map["ILS"] = mmCurrency("ILS", "Israeli new sheqel", L"", L"", L"", L"", 100, 1);
	currency_map["JMD"] = mmCurrency("JMD", "Jamaican dollar", L"J$", L"", L"", L"", 100, 1);
	currency_map["JPY"] = mmCurrency("JPY", "Japanese yen", L"¥", L"", L"", L"", 1, 1);
	currency_map["JOD"] = mmCurrency("JOD", "Jordanian dinar", L"", L"", L"", L"", 100, 1);
	currency_map["KZT"] = mmCurrency("KZT", "Kazakhstani tenge", L"T", L"", L"", L"", 100, 1);
	currency_map["KES"] = mmCurrency("KES", "Kenyan shilling", L"KSh", L"", L"", L"", 100, 1);
	currency_map["KPW"] = mmCurrency("KPW", "North Korean won", L"W", L"", L"", L"", 100, 1);
	currency_map["KRW"] = mmCurrency("KRW", "South Korean won", L"W", L"", L"", L"", 1, 1);
	currency_map["KWD"] = mmCurrency("KWD", "Kuwaiti dinar", L"", L"", L"", L"", 100, 1);
	currency_map["KGS"] = mmCurrency("KGS", "Kyrgyzstani som", L"", L"", L"", L"", 100, 1);
	currency_map["LAK"] = mmCurrency("LAK", "Lao kip", L"KN", L"", L"", L"", 100, 1);
	currency_map["LVL"] = mmCurrency("LVL", "Latvian lats", L"Ls", L"", L"", L"", 100, 1);
	currency_map["LBP"] = mmCurrency("LBP", "Lebanese lira", L"", L"", L"", L"", 1, 1);
	currency_map["LSL"] = mmCurrency("LSL", "Lesotho loti", L"M", L"", L"", L"", 100, 1);
	currency_map["LRD"] = mmCurrency("LRD", "Liberian dollar", L"L$", L"", L"", L"", 100, 1);
	currency_map["LYD"] = mmCurrency("LYD", "Libyan dinar", L"LD", L"", L"", L"", 100, 1);
	currency_map["LTL"] = mmCurrency("LTL", "Lithuanian litas", L"Lt", L"", L"", L"", 100, 1);
	currency_map["MOP"] = mmCurrency("MOP", "Macanese pataca", L"P", L"", L"", L"", 100, 1);
	currency_map["MKD"] = mmCurrency("MKD", "Macedonian denar", L"", L"", L"", L"", 100, 1);
	currency_map["MGA"] = mmCurrency("MGA", "Malagasy ariary", L"FMG", L"", L"", L"", 100, 1);
	currency_map["MWK"] = mmCurrency("MWK", "Malawian kwacha", L"MK", L"", L"", L"", 1, 1);
	currency_map["MYR"] = mmCurrency("MYR", "Malaysian ringgit", L"RM", L"", L"", L"", 100, 1);
	currency_map["MVR"] = mmCurrency("MVR", "Maldivian rufiyaa", L"Rf", L"", L"", L"", 100, 1);
	currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
	currency_map["MRO"] = mmCurrency("MRO", "Mauritanian ouguiya", L"UM", L"", L"", L"", 100, 1);
	currency_map["MUR"] = mmCurrency("MUR", "Mauritian rupee", L"Rs", L"", L"", L"", 1, 1);
	currency_map["MXN"] = mmCurrency("MXN", "Mexican peso", L"$", L"", L"", L"", 100, 1);
	currency_map["MDL"] = mmCurrency("MDL", "Moldovan leu", L"", L"", L"", L"", 100, 1);
	currency_map["MNT"] = mmCurrency("MNT", "Mongolian tugrik",L"₮", L"", L"", L"", 100, 1);
	currency_map["XCD"] = mmCurrency("XCD", "East Caribbean dollar", L"EC$", L"", L"", L"", 100, 1);
	currency_map["MAD"] = mmCurrency("MAD", "Moroccan dirham", L"", L"", L"", L"", 100, 1);
	currency_map["MZM"] = mmCurrency("MZM", "Mozambican metical", L"MTn", L"", L"", L"", 100, 1);
	currency_map["MMK"] = mmCurrency("MMK", "Myanma kyat", L"K", L"", L"", L"", 1, 1);
	currency_map["NAD"] = mmCurrency("NAD", "Namibian dollar", L"N$", L"", L"", L"", 100, 1);
	currency_map["NPR"] = mmCurrency("NPR", "Nepalese rupee", L"NRs", L"", L"", L"", 100, 1);
	currency_map["ANG"] = mmCurrency("ANG", "Netherlands Antillean gulden", L"NAƒ", L"", L"", L"", 100, 1);
	currency_map["XPF"] = mmCurrency("XPF", "CFP franc", L"F", L"", L"", L"", 100, 1);
	currency_map["NZD"] = mmCurrency("NZD", "New Zealand dollar", L"NZ$", L"", L"", L"", 100, 1);
	currency_map["NIO"] = mmCurrency("NIO", "Nicaraguan córdoba", L"C$", L"", L"", L"", 100, 1);
	currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
	currency_map["NGN"] = mmCurrency("NGN", "Nigerian naira", L"₦", L"", L"", L"", 100, 1);
	currency_map["NOK"] = mmCurrency("NOK", "Norwegian krone", L"kr", L"", L"", L"", 100, 1);
	currency_map["OMR"] = mmCurrency("OMR", "Omani rial", L"", L"", L"", L"", 100, 1);
	currency_map["PKR"] = mmCurrency("PKR", "Pakistani rupee", L"Rs.", L"", L"", L"", 1, 1);
	currency_map["PAB"] = mmCurrency("PAB", "Panamanian balboa", L"B./", L"", L"", L"", 100, 1);
	currency_map["PGK"] = mmCurrency("PGK", "Papua New Guinean kina", L"K", L"", L"", L"", 100, 1);
	currency_map["PYG"] = mmCurrency("PYG", "Paraguayan guarani", L"", L"", L"", L"", 1, 1);
	currency_map["PEN"] = mmCurrency("PEN", "Peruvian nuevo sol", L"S/.", L"", L"", L"", 100, 1);
	currency_map["PHP"] = mmCurrency("PHP", "Philippine peso", L"₱", L"", L"", L"", 100, 1);
	currency_map["PLN"] = mmCurrency("PLN", "Polish zloty", L"", L"", L"", L"", 100, 1);
	currency_map["QAR"] = mmCurrency("QAR", "Qatari riyal", L"QR", L"", L"", L"", 100, 1);
	currency_map["RON"] = mmCurrency("RON", "Romanian leu", L"L", L"", L"", L"", 100, 1);
	currency_map["RWF"] = mmCurrency("RWF", "Rwandan franc", L"RF", L"", L"", L"", 1, 1);
	currency_map["STD"] = mmCurrency("STD", "São Tomé and Príncipe dobra", L"Db", L"", L"", L"", 100, 1);
	currency_map["SAR"] = mmCurrency("SAR", "Saudi riyal", L"SR", L"", L"", L"", 100, 1);
	currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
	currency_map["RSD"] = mmCurrency("RSD", "Serbian dinar", L"din.", L"", L"", L"", 1, 1);
	currency_map["SCR"] = mmCurrency("SCR", "Seychellois rupee", L"SR", L"", L"", L"", 100, 1);
	currency_map["SLL"] = mmCurrency("SLL", "Sierra Leonean leone", L"Le", L"", L"", L"", 100, 1);
	currency_map["SGD"] = mmCurrency("SGD", "Singapore dollar", L"S$", L"", L"", L"", 100, 1);
	currency_map["SBD"] = mmCurrency("SBD", "Solomon Islands dollar", L"SI$", L"", L"", L"", 100, 1);
	currency_map["SOS"] = mmCurrency("SOS", "Somali shilling", L"Sh.", L"", L"", L"", 1, 1);
	currency_map["ZAR"] = mmCurrency("ZAR", "South African rand", L"R", L"", L"", L"", 100, 1);
	currency_map["LKR"] = mmCurrency("LKR", "Sri Lankan rupee", L"Rs", L"", L"", L"", 100, 1);
	currency_map["SHP"] = mmCurrency("SHP", "Saint Helena pound", L"£", L"", L"", L"", 100, 1);
	currency_map["XCD"] = mmCurrency("XCD", "East Caribbean dollar", L"EC$", L"", L"", L"", 100, 1);
	currency_map["SDG"] = mmCurrency("SDG", "Sudanese pound", L"", L"", L"", L"", 100, 1);
	currency_map["SRD"] = mmCurrency("SRD", "Surinamese dollar", L"$", L"", L"", L"", 100, 1);
	currency_map["SZL"] = mmCurrency("SZL", "Swazi lilangeni", L"E", L"", L"", L"", 100, 1);
	currency_map["SEK"] = mmCurrency("SEK", "Swedish krona", L"kr", L"", L"", L"", 100, 1);
	currency_map["CHF"] = mmCurrency("CHF", "Swiss franc", L"Fr.", L"", L"", L"", 100, 1);
	currency_map["SYP"] = mmCurrency("SYP", "Syrian pound", L"", L"", L"", L"", 1, 1);
	currency_map["TWD"] = mmCurrency("TWD", "New Taiwan dollar", L"NT$", L"", L"", L"", 100, 1);
	currency_map["TJS"] = mmCurrency("TJS", "Tajikistani somoni", L"", L"", L"", L"", 100, 1);
	currency_map["TZS"] = mmCurrency("TZS", "Tanzanian shilling", L"", L"", L"", L"", 1, 1);
	currency_map["THB"] = mmCurrency("THB", "Thai baht", L"฿", L"", L"", L"", 100, 1);
	currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
	currency_map["TTD"] = mmCurrency("TTD", "Trinidad and Tobago dollar", L"TT$", L"", L"", L"", 100, 1);
	currency_map["TND"] = mmCurrency("TND", "Tunisian dinar", L"DT", L"", L"", L"", 100, 1);
	currency_map["TRY"] = mmCurrency("TRY", "Turkish new lira", L"YTL", L"", L"", L"", 100, 1);
	currency_map["TMT"] = mmCurrency("TMT", "Turkmen manat", L"m", L"", L"", L"", 100, 1);
	currency_map["UGX"] = mmCurrency("UGX", "Ugandan shilling", L"USh", L"", L"", L"", 1, 1);
	currency_map["AED"] = mmCurrency("AED", "UAE dirham", L"", L"", L"", L"", 100, 1);
	currency_map["UYU"] = mmCurrency("UYU", "Uruguayan peso", L"$U", L"", L"", L"", 100, 1);
	currency_map["UZS"] = mmCurrency("UZS", "Uzbekistani som", L"", L"", L"", L"", 1, 1);
	currency_map["VUV"] = mmCurrency("VUV", "Vanuatu vatu", L"VT", L"", L"", L"", 100, 1);
	currency_map["VEB"] = mmCurrency("VEB", "Venezuelan bolivar", L"Bs", L"", L"", L"", 100, 1);
	currency_map["VND"] = mmCurrency("VND", "Vietnamese dong", L"₫", L"", L"", L"", 1, 1);
	currency_map["XPF"] = mmCurrency("XPF", "CFP franc", L"F", L"", L"", L"", 100, 1);
	currency_map["WST"] = mmCurrency("WST", "Samoan tala", L"WS$", L"", L"", L"", 100, 1);
	currency_map["YER"] = mmCurrency("YER", "Yemeni rial", L"", L"", L"", L"", 1, 1);
	currency_map["ZMK"] = mmCurrency("ZMK", "Zambian kwacha", L"ZK", L"", L"", L"", 1, 1);
	currency_map["ZWR"] = mmCurrency("ZWR", "Zimbabwean dollar", L"Z$", L"", L"", L"", 100, 1);
    
    return currency_map;
}

void mmCurrencyDialog::updateControls()
{
    wxString dispAmount, dispAmount2;
    double amount = 1000;
    core_->currencyList_.LoadBaseCurrencySettings();
    dispAmount = CurrencyFormatter::float2Money(amount);
    dispAmount2 = wxString() << dispAmount << " " << _("Converted to:") << " ";

    wxString currencyName = currencyNameCombo_->GetStringSelection();
    mmCurrency* pCurrency;
    if (currencyID_ > -1)
        pCurrency = core_->currencyList_.getCurrencySharedPtr(currencyID_);
    else
        pCurrency = core_->currencyList_.getCurrencySharedPtr(currencyName);

    if (pCurrency)
    {
        pfxTx_->SetValue(pCurrency->pfxSymbol_);
        sfxTx_->SetValue(pCurrency->sfxSymbol_);
        decTx_->SetValue(pCurrency->dec_);
        grpTx_->SetValue(pCurrency->grp_);
        unitTx_->SetValue(pCurrency->unit_);
        centTx_->SetValue(pCurrency->cent_);
        scaleTx_->SetValue(wxString() << pCurrency->scaleDl_);
        convRate_ = pCurrency->baseConv_;
        baseConvRate_->SetValue(wxString() << convRate_);
        currencySymbolCombo_->SetValue(pCurrency->currencySymbol_);
        mmDBWrapper::loadCurrencySettings(core_->db_.get(), pCurrency->currencyID_);
        if (pCurrency->baseConv_ != 0.0 )
            amount = amount / pCurrency->baseConv_;
        else
            amount = 0.0;
    }
    else
    {
        convRate_ = 1;
        baseConvRate_->SetValue("1");
    }

    dispAmount = CurrencyFormatter::float2Money(amount);
    baseRateSample_->SetLabel(dispAmount2 + dispAmount);

    amount = 123456.78;
    dispAmount = wxString() << "123456.78 " << _("Shown As: ") << CurrencyFormatter::float2Money(amount);
    sampleText_->SetLabel(dispAmount);

    // resize the dialog window
    Fit();
}

void mmCurrencyDialog::CreateControls()
{
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Expand();
    const wxSize size = wxSize(220, -1);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer3->AddGrowableCol(1);
    itemBoxSizer2->Add(itemFlexGridSizer3, flags);

    //--------------------------
    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Currency Name")), flags);

    currencyNameCombo_ = new wxComboBox( this, ID_DIALOG_CURRENCY_CHOICE
        , "", wxDefaultPosition, wxSize(220, -1));
    itemFlexGridSizer3->Add(currencyNameCombo_, flags);
    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Currency Symbol")), flags);

    currencySymbolCombo_ = new wxComboBox( this, wxID_ANY);
    itemFlexGridSizer3->Add(currencySymbolCombo_, flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Unit Name")), flags);
    unitTx_ = new wxTextCtrl( this, ID_DIALOG_CURRENCY_TEXT_UNIT, "");
    itemFlexGridSizer3->Add(unitTx_, flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Cents Name")), flags);
    centTx_ = new wxTextCtrl( this, ID_DIALOG_CURRENCY_TEXT_CENTS, "");
    itemFlexGridSizer3->Add(centTx_, flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Prefix Symbol")), flags);
    pfxTx_ = new wxTextCtrl( this, ID_DIALOG_CURRENCY_TEXT_PFX, "");
    itemFlexGridSizer3->Add(pfxTx_, flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Suffix Symbol")), flags);
    sfxTx_ = new wxTextCtrl( this, ID_DIALOG_CURRENCY_TEXT_SFX, "");
    itemFlexGridSizer3->Add(sfxTx_, flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Decimal Char")), flags);
    decTx_ = new wxTextCtrl( this, ID_DIALOG_CURRENCY_TEXT_DECIMAL, "");
    itemFlexGridSizer3->Add(decTx_, flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Grouping Char")), flags);
    grpTx_ = new wxTextCtrl( this, ID_DIALOG_CURRENCY_TEXT_GROUP, "");
    itemFlexGridSizer3->Add(grpTx_, flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Scale")), flags);
    scaleTx_ = new wxTextCtrl( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize
        , wxALIGN_RIGHT|wxTE_PROCESS_ENTER , mmDoubleValidator4() );
    itemFlexGridSizer3->Add(scaleTx_, flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Conversion to Base Rate")), flags);
    baseConvRate_ = new wxTextCtrl( this, ID_DIALOG_CURRENCY_TEXT_BASECONVRATE, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmDoubleValidator4() );
    itemFlexGridSizer3->Add(baseConvRate_, flagsExpand);
    baseConvRate_ ->SetToolTip(_("Other currency conversion rate. Set Base Currency to 1."));

    //--------------------------
    wxStaticBox* itemStaticBox_02 = new wxStaticBox(this, wxID_ANY, _("Base Rate Conversion Sample:"));
    wxStaticBoxSizer* itemStaticBoxSizer_02 = new wxStaticBoxSizer(itemStaticBox_02, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer_02, flagsExpand);

    baseRateSample_ = new wxStaticText( this, wxID_STATIC, "");
    itemStaticBoxSizer_02->Add(baseRateSample_, flags);

    //--------------------------
    wxStaticBox* itemStaticBox_01 = new wxStaticBox(this, wxID_STATIC, _("Value Display Sample:"));
    wxStaticBoxSizer* itemStaticBoxSizer_01 = new wxStaticBoxSizer(itemStaticBox_01, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer_01, flagsExpand);

    sampleText_ = new wxStaticText( this, wxID_STATIC, "");
    itemStaticBoxSizer_01->Add(sampleText_, flags);

    //--------------------------
    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22, flags.Centre());

    wxButton* itemButton24 = new wxButton( this, ID_DIALOG_CURRENCY_BUTTON_UPDATE, _("&Update"));
    itemBoxSizer22->Add(itemButton24, flags);
    itemButton24->SetToolTip(_("Save any changes made"));

    wxButton* itemButton25 = new wxButton( this, wxID_CANCEL, _("&Close"));
    itemBoxSizer22->Add(itemButton25, flags);
    itemButton25->SetToolTip(_("Any changes will be lost without update"));
}

void mmCurrencyDialog::OnUpdate(wxCommandEvent& /*event*/)
{
    wxString currencyName = currencyNameCombo_->GetValue();
    mmCurrency* pCurrency;
    if (currencyID_ > -1)
        pCurrency = core_->currencyList_.getCurrencySharedPtr(currencyID_);
    else
        pCurrency = new mmCurrency();

    long scal = 0;
    scaleTx_->GetValue().ToLong(&scal);
    baseConvRate_->GetValue().ToDouble(&convRate_);

    pCurrency->pfxSymbol_ = pfxTx_->GetValue();
    pCurrency->sfxSymbol_ = sfxTx_->GetValue();
    pCurrency->dec_ = decTx_->GetValue();
    pCurrency->grp_ =  grpTx_->GetValue();
    pCurrency->unit_ = unitTx_->GetValue();
    pCurrency->cent_ = centTx_->GetValue();
    pCurrency->scaleDl_ = static_cast<int>(scal);
    pCurrency->baseConv_ = convRate_;
    pCurrency->currencySymbol_ = currencySymbolCombo_->GetValue();
    pCurrency->currencyName_ = currencyName;

    if (currencyID_ > -1)
        core_->currencyList_.UpdateCurrency(pCurrency);
    else
        currencyID_ = core_->currencyList_.AddCurrency(pCurrency);

    fillControls();
}

void mmCurrencyDialog::OnCurrencyNameSelected(wxCommandEvent& /*event*/)
{
    if (currencyID_ > -1) return;

    for (const auto& i : this->currency_map())
    {
        if (i.second.currencyName_ == currencyNameCombo_->GetValue())
        {
            currencySymbolCombo_->SetValue(i.first);
            pfxTx_->SetValue(i.second.pfxSymbol_);
            sfxTx_->SetValue(i.second.sfxSymbol_);
            decTx_->SetValue(i.second.dec_);
            grpTx_->SetValue(i.second.grp_);
            unitTx_->SetValue(i.second.unit_);
            centTx_->SetValue(i.second.cent_);
            scaleTx_->SetValue(wxString() << i.second.scaleDl_);
            convRate_ = i.second.baseConv_;
            baseConvRate_->SetValue(wxString() << convRate_);
            currencySymbolCombo_->SetValue(i.second.currencySymbol_);
            break;
        }
    }
}
