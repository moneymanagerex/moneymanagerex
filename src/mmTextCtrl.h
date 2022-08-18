/*******************************************************
Copyright (C) 2006-2017 Nikolay Akimov

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

#pragma once

#include <wx/textctrl.h>
#include "model/Model_Currency.h"
#include "model/Model_Account.h"

class mmTextCtrl : public wxTextCtrl
{
public:
    using wxTextCtrl::SetValue;

    mmTextCtrl() : m_currency(0) {}
    mmTextCtrl(wxWindow *parent, wxWindowID id
        , const wxString &value = wxEmptyString
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = wxTE_PROCESS_ENTER | wxALIGN_RIGHT
        , const wxValidator &validator = wxDefaultValidator
        , const Model_Currency::Data* currency = Model_Currency::GetBaseCurrency()
        , const wxString &name = "mmTextCtrl")
    : wxTextCtrl(parent, id, value, pos, size, style, validator, name)
        , m_currency(currency)
        , m_alt_precision(-1)
    {}

    void SetValue(double value);
    void SetValue(double value, int precision);
    //SetValue without generating an event
    void SetValueNoEvent(double value, int precision);
    void SetValue(double value, const Model_Account::Data* account, int precision = -1);
    void SetValue(double value, const Model_Currency::Data* currency, int precision = -1);
    bool Calculate(int alt_precision = -1);
    bool GetDouble(double &amount) const;
    bool checkValue(double &amount, bool positive_value = true);
    wxChar GetDecimalPoint();
    void SetAltPrecision(int precision);

private:
    int m_alt_precision;
    void OnTextEntered(wxCommandEvent& event);
    void OnKillFocus(wxFocusEvent& event);
    const Model_Currency::Data* m_currency;
    wxDECLARE_EVENT_TABLE();
};

inline void mmTextCtrl::SetAltPrecision(int precision) { m_alt_precision = precision; }
