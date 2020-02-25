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

#include "mmTextCtrl.h"
#include <wx/log.h>
#include <wx/richtooltip.h>
#include <LuaGlue/LuaGlue.h>

void mmTextCtrl::SetValue(double value)
{
    this->SetValue(Model_Currency::toString(value, m_currency));
}

void mmTextCtrl::SetValue(double value, int precision)
{
    this->SetValue(Model_Currency::toString(value, m_currency, precision));
}

void mmTextCtrl::SetValueNoEvent(double value, int precision)
{
    this->ChangeValue(Model_Currency::toString(value, m_currency, precision));
}

void mmTextCtrl::SetValue(double value, const Model_Account::Data* account, int precision)
{
    if (account) m_currency = Model_Currency::instance().get(account->CURRENCYID);
    this->SetValue(value, precision > -1 ? precision : log10(m_currency->SCALE));
}

void mmTextCtrl::SetValue(double value, const Model_Currency::Data* currency, int precision)
{
    m_currency = (currency ? currency : Model_Currency::GetBaseCurrency());
    this->SetValue(value, precision > -1 ? precision : log10(m_currency->SCALE));
}

bool mmTextCtrl::Calculate(int alt_precision)
{
    const wxString str = Model_Currency::fromString2Default(this->GetValue(), m_currency);
    if (str.empty()) return false;

    LuaGlue state;
    state.open().glue();
    std::string lua_f = "function calc() return " + str.ToStdString() + "; end";
    if(!state.doString(lua_f))
    {
        wxLogDebug("lua calc() err: %s", state.lastError().c_str());
        return false;
    }

    const double res = state.invokeFunction<double>("calc");
    const int precision = alt_precision >= 0 ? alt_precision : log10(m_currency->SCALE);
    this->ChangeValue(Model_Currency::toString(res, m_currency, precision));
    this->SetInsertionPoint(this->GetValue().Len());
    return true;
}

bool mmTextCtrl::GetDouble(double &amount) const
{
    wxString amountStr = this->GetValue().Trim();
    return Model_Currency::fromString(amountStr, amount, m_currency);
}

bool mmTextCtrl::checkValue(double &amount, bool positive_value)
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

wxChar mmTextCtrl::GetDecimalPoint()
{
    return m_currency->DECIMAL_POINT[0];
}
