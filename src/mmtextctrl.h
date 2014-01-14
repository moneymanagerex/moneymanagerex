/*******************************************************
Copyright (C) 2006-2012

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

    mmTextCtrl() : currency_(0) {}
    mmTextCtrl(wxWindow *parent, wxWindowID id
        , const wxString &value = wxEmptyString
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = 0
        , const wxValidator &validator = wxDefaultValidator
        , const wxString &name = wxTextCtrlNameStr)
        : wxTextCtrl(parent, id, value, pos, size, style, validator, name)
        , currency_(Model_Currency::GetBaseCurrency())
    {}
    void SetValue(double value)
    {
        this->SetValue(Model_Currency::toString(value, currency_));
    }
    void SetValue(double value, int precision)
    {
        this->SetValue(Model_Currency::toString(value, currency_, precision));
    }
    void SetValue(double value, const Model_Account::Data* account, int precision = -1)
    {
        if (account) currency_ = Model_Currency::instance().get(account->CURRENCYID);
        this->SetValue(value, precision > -1 ? precision : log10(currency_->SCALE));
    }
    void SetValue(double value, const Model_Currency::Data* currency, int precision = -1)
    {
        currency_ = (currency ? currency : Model_Currency::GetBaseCurrency());
        this->SetValue(value, precision > -1 ? precision : log10(currency_->SCALE));
    }
    void Calculate(const Model_Currency::Data* currency, int alt_precision = -1)
    {
        mmCalculator calc;
        int precision = alt_precision >= 0 ? alt_precision : log10(currency->SCALE);
        if (calc.is_ok(Model_Currency::fromString2Default(this->GetValue(), currency)))
            this->SetValue(Model_Currency::toString(calc.get_result(), currency, precision));
        this->SetInsertionPoint(this->GetValue().Len());
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
        return Model_Currency::fromString(amountStr, amount, currency_);
    }
    bool checkValue(double &amount)
    {
        if (!GetDouble(amount) || amount < 0)
        {
            wxRichToolTip tip(_("Invalid Amount Entered "),
                _("You might have made an error in the amount entered.")
                + "\n"
                + _("Value should be positive and proper formated.")
                + "\n\n"
                + _("Tip: Enter some amount or math expression like '2+2'\nthen press Enter for proper format"));
            tip.SetIcon(wxICON_WARNING);
            tip.ShowFor(this);
            SetFocus();
            return false;
        }
        return true;
    }
private:
    const Model_Currency::Data* currency_;
};


