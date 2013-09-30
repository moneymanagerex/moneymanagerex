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

#define SYMBOL_CURRENCYDIALOG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CURRENCYDIALOG_TITLE _("Currency Manager")
#define SYMBOL_CURRENCYDIALOG_IDNAME ID_DIALOG_CURRENCY
#define SYMBOL_CURRENCYDIALOG_SIZE wxSize(500, 300)
#define SYMBOL_CURRENCYDIALOG_POSITION wxDefaultPosition

#include "guiid.h"
#include "defs.h"
#include "mmcoredb.h"
#include "dbwrapper.h"

enum
{
    ID_DIALOG_CURRENCY = wxID_HIGHEST + 1,
    ID_DIALOG_CURRENCY_BUTTON_ADD,
    ID_DIALOG_CURRENCY_BUTTON_SELECT,
    ID_DIALOG_CURRENCY_BUTTON_UPDATE,
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
    mmCurrencyDialog(mmCoreDB* core,
                     int currencyID,
                     wxWindow* parent,
                     wxWindowID id = SYMBOL_CURRENCYDIALOG_IDNAME,
                     const wxString& caption = SYMBOL_CURRENCYDIALOG_TITLE,
                     const wxPoint& pos = SYMBOL_CURRENCYDIALOG_POSITION,
                     const wxSize& size = SYMBOL_CURRENCYDIALOG_SIZE,
                     long style = SYMBOL_CURRENCYDIALOG_STYLE );
    mmCurrencyDialog(mmCoreDB* core, wxWindow* parent,
                     wxWindowID id = SYMBOL_CURRENCYDIALOG_IDNAME,
                     const wxString& caption = SYMBOL_CURRENCYDIALOG_TITLE,
                     const wxPoint& pos = SYMBOL_CURRENCYDIALOG_POSITION,
                     const wxSize& size = SYMBOL_CURRENCYDIALOG_SIZE,
                     long style = SYMBOL_CURRENCYDIALOG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CURRENCYDIALOG_IDNAME,
                 const wxString& caption = SYMBOL_CURRENCYDIALOG_TITLE,
                 const wxPoint& pos = SYMBOL_CURRENCYDIALOG_POSITION,
                 const wxSize& size = SYMBOL_CURRENCYDIALOG_SIZE,
                 long style = SYMBOL_CURRENCYDIALOG_STYLE );

private:
    void CreateControls();

    // utility functions
    void OnUpdate(wxCommandEvent& event);
    void OnCurrencyNameSelected(wxCommandEvent& event);
    void updateControls();
    void fillControls();

private:
    mmCoreDB* core_;

    int currencyID_;
    double convRate_;

    wxComboBox* currencyNameCombo_;
    wxStaticText* baseRateSample_;
    wxStaticText* sampleText_;
    wxComboBox* currencySymbolCombo_;
    wxTextCtrl* baseConvRate_;
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
