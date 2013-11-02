
#pragma once
#include <wx/textctrl.h>
#include <wx/string.h>
#include "model/Model_Currency.h"
#include "model/Model_Account.h"

class mmTextCtrl : public wxTextCtrl
{
public:
    using wxTextCtrl::SetValue;

    mmTextCtrl() : currency_(0) {}
    mmTextCtrl(wxWindow *parent, wxWindowID id, const wxString &value=wxEmptyString
            , const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize
            , long style=0, const wxValidator &validator=wxDefaultValidator, const wxString &name=wxTextCtrlNameStr)
            : wxTextCtrl(parent, id, value, pos, size, style, validator, name), currency_(0)
            {}
    void SetValue(double value)
    {
        currency_ = Model_Currency::GetBaseCurrency();
        this->SetValue(Model_Currency::toString(value, currency_));
    }
    void SetValue(double value, const Model_Account::Data* account)
    {
        currency_ = Model_Currency::instance().get(account->CURRENCYID);
        this->SetValue(Model_Account::toString(value, account));
    }
    wxString GetValue() const
    {
        // Remove prefix and suffix characters from value
        wxString prefix = (currency_ ? currency_->PFX_SYMBOL : "");
        wxString suffix = (currency_ ? currency_->SFX_SYMBOL : "");
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
            if (val.EndsWith(suffix, &removed))
                val = removed;
        }
        // Base class handles the thousands seperator
        return val;
    }
private:
    const Model_Currency::Data* currency_;
};


