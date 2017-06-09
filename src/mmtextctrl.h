/*******************************************************
Copyright (C) 2006-2017

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
#include <wx/string.h>
#include "mmCalculator.h"
#include "model/Model_Currency.h"
#include "model/Model_Account.h"
#include <wx/richtooltip.h>

class mmTextCtrl : public wxTextCtrl
{
public:
    using wxTextCtrl::SetValue;

    mmTextCtrl() : m_currency(0) {}
    mmTextCtrl(wxWindow *parent, wxWindowID id
        , const wxString &value = wxEmptyString
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = 0
        , const wxValidator &validator = wxDefaultValidator
        , const Model_Currency::Data* currency = Model_Currency::GetBaseCurrency()
        , const wxString &name = wxTextCtrlNameStr)
    : wxTextCtrl(parent, id, value, pos, size, style, validator, name)
        , m_currency(currency)
    {}

    void SetValue(double value)
    {
        this->SetValue(Model_Currency::toString(value, m_currency));
    }

    void SetValue(double value, int precision)
    {
        this->SetValue(Model_Currency::toString(value, m_currency, precision));
    }

    //SetValue without generating an event
    void SetValueNoEvent(double value, int precision)
    {
        this->ChangeValue(Model_Currency::toString(value, m_currency, precision));
    }

    void SetValue(double value, const Model_Account::Data* account, int precision = -1)
    {
        if (account) m_currency = Model_Currency::instance().get(account->CURRENCYID);
        this->SetValue(value, precision > -1 ? precision : log10(m_currency->SCALE));
    }

    void SetValue(double value, const Model_Currency::Data* currency, int precision = -1)
    {
        m_currency = (currency ? currency : Model_Currency::GetBaseCurrency());
        this->SetValue(value, precision > -1 ? precision : log10(m_currency->SCALE));
    }

    bool Calculate(int alt_precision = -1)
    {
        if (this->GetValue().empty()) return false;
        mmCalculator calc;
        int precision = alt_precision >= 0 ? alt_precision : log10(m_currency->SCALE);
        const wxString str = Model_Currency::fromString2Default(this->GetValue(), m_currency);
        if (calc.is_ok(str))
        {
            this->ChangeValue(Model_Currency::toString(calc.get_result(), m_currency, precision));
            this->SetInsertionPoint(this->GetValue().Len());
            return true;
        }
        return false;
    }

    wxString GetValue() const
    {
        // Remove prefix and suffix characters from value
        // Base class handles the thousands seperator
        return /*Model_Currency::fromString(*/wxTextCtrl::GetValue()/*, currency_)*/;
    }

    bool GetDouble(double &amount) const
    {
        wxString amountStr = this->GetValue().Trim();
        return Model_Currency::fromString(amountStr, amount, m_currency);
    }

    bool checkValue(double &amount, bool positive_value = true)
    {
        if (!GetDouble(amount) || (positive_value && amount < 0))
        {
            wxRichToolTip tip(_("Invalid Amount."),
                wxString(positive_value ? _("Please enter a positive or calculated value.") : _("Please enter a calculated value."))
                + "\n\n"
                + _("Tip: For calculations, enter expressions like (2+2)*(2+2)\nCalculations will be evaluated and the result used as the entry."));
            tip.SetIcon(wxICON_WARNING);
            tip.ShowFor(this);
            SetFocus();
            return false;
        }
        return true;
    }

    const Model_Currency::Data* m_currency;
};


