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

#ifndef _MM_EX_CURRENCYDIALOG_H_
#define _MM_EX_CURRENCYDIALOG_H_

#include "defs.h"
#include "dbwrapper.h"
#include "model/Model_Currency.h"

class mmTextCtrl;

enum
{
    ID_DIALOG_CURRENCY = wxID_HIGHEST + 1,
    ID_DIALOG_CURRENCY_BUTTON_ADD,
    ID_DIALOG_CURRENCY_BUTTON_SELECT,
    ID_DIALOG_CURRENCY_CHOICE,
    ID_DIALOG_CURRENCY_TEXT_PFX,
    ID_DIALOG_CURRENCY_TEXT_SFX,
    ID_DIALOG_CURRENCY_TEXT_DECIMAL,
    ID_DIALOG_CURRENCY_TEXT_GROUP,
    ID_DIALOG_CURRENCY_TEXT_UNIT,
    ID_DIALOG_CURRENCY_TEXT_CENTS,
    ID_DIALOG_CURRENCY_TEXT_SCALE,
    ID_DIALOG_CURRENCY_TEXT_BASECONVRATE,
    ID_DIALOG_CURRENCY_BUTTON_CANCEL,
    ID_DIALOG_CURRENCY_TEXT_SYMBOL,
};

// TODO Model_Currency
class mmCurrencyDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmCurrencyDialog )
    DECLARE_EVENT_TABLE()

public:
    mmCurrencyDialog();
    ~mmCurrencyDialog();
    mmCurrencyDialog(Model_Currency::Data* currency, wxWindow* parent);

    bool Create( wxWindow* parent, wxWindowID id,
                 const wxString& caption,
                 const wxPoint& pos,
                 const wxSize& size,
                 long style);

private:
    void CreateControls();

    // utility functions
    void OnUpdate(wxCommandEvent& event);
    void OnCurrencyNameSelected(wxCommandEvent& event);
    void fillControls();
    void onTextEntered(wxCommandEvent& event);

    Model_Currency::Data* m_currency;
    double convRate_;
    int scale_;

    wxComboBox* currencyNameCombo_;
    wxStaticText* baseRateSample_;
    wxStaticText* sampleText_;
    wxComboBox* currencySymbolCombo_;
    mmTextCtrl* baseConvRate_;
    wxTextCtrl* pfxTx_;
    wxTextCtrl* sfxTx_;
    wxTextCtrl* decTx_;
    wxTextCtrl* grpTx_;
    wxTextCtrl* unitTx_;
    wxTextCtrl* centTx_;
    wxTextCtrl* scaleTx_;
    wxTextCtrl* baseConv_;

};

#endif
