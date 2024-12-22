/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 Gabriele-V
 Copyright (C) 2022 Mark WHalley (mark@ipx.co.uk)

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

#ifndef MM_EX_CURRENCYDIALOG_H_
#define MM_EX_CURRENCYDIALOG_H_

#include "model/Model_Currency.h"

class mmTextCtrl;

enum
{
    ID_DIALOG_CURRENCY = wxID_HIGHEST + 400,
    ID_DIALOG_CURRENCY_RATE,
};

class mmCurrencyDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmCurrencyDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmCurrencyDialog();
    ~mmCurrencyDialog();
    mmCurrencyDialog(wxWindow* parent, const Model_Currency::Data * currency);

    int64 getCurrencyID() { return m_currency->CURRENCYID; };

private:
    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& caption = _("Currency Manager"),
        const wxString& name = "Currency Manager",
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX | wxRESIZE_BORDER);

    void CreateControls();
    void fillControls();

    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnDataChanged(wxCommandEvent& event);
    void OnTextEntered(wxCommandEvent& event);

    Model_Currency::Data* m_currency  = nullptr;
    int m_scale = 9;
    bool m_locale_used = false;

    wxTextCtrl* mctrl_name = nullptr;
    wxTextCtrl* mctrl_code = nullptr;
    wxTextCtrl* mctrl_symbol = nullptr;
    wxRadioButton* mctrl_prefix = nullptr;
    wxRadioButton* mctrl_suffix = nullptr;
    wxChoice* mctrl_decimalSep = nullptr;
    wxChoice* mctrl_groupSep = nullptr;
    wxTextCtrl* mctrl_scale = nullptr;
    mmTextCtrl* mctrl_baseConvRate = nullptr;
    wxStaticText* mctrl_sampleText = nullptr;

};

#endif
