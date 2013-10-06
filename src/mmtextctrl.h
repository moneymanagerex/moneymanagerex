
#pragma once
#include <wx/textctrl.h>
#include <wx/string.h>
#include "model/Model_Currency.h"
#include "model/Model_Account.h"

class mmTextCtrl : public wxTextCtrl
{
public:
    mmTextCtrl() {}
    mmTextCtrl(wxWindow *parent, wxWindowID id, const wxString &value=wxEmptyString
            , const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize
            , long style=0, const wxValidator &validator=wxDefaultValidator, const wxString &name=wxTextCtrlNameStr)
            : wxTextCtrl(parent, id, value, pos, size, style, validator, name)
            {}
    void SetValue(const wxString &value)
    {
        prefix = "";
        suffix = "";
        wxTextCtrl::SetValue(value);
    }
    void SetValue(double value)
    {
        this->SetValue(Model_Currency::toString(value));
        // update after call to SetValue
        prefix = Model_Currency::GetBaseCurrency()->PFX_SYMBOL;
        suffix = Model_Currency::GetBaseCurrency()->SFX_SYMBOL;
    }
    void SetValue(double value, const Model_Account::Data* account)
    {
        this->SetValue(Model_Account::toString(value, account));
        // update after call to SetValue
        Model_Currency::Data* currency = Model_Currency::instance().get(account->CURRENCYID);
        prefix = (currency ? currency->PFX_SYMBOL : "");
        suffix = (currency ? currency->SFX_SYMBOL : "");
    }
    wxString GetValue() const
    {
        // Remove prefix and suffix characters from value
        wxString val = wxTextCtrl::GetValue();
        if (!prefix.IsEmpty())
        {
            wxString removed;
            if (val.StartsWith(prefix, &removed))
                val = removed;
        }
        if (!suffix.IsEmpty())
        {
            wxString removed;
            if (val.EndsWith(prefix, &removed))
                val = removed;
        }
        // Base class handles the thousands seperator
        return val;
    }
private:
    wxString prefix;
    wxString suffix;
};


