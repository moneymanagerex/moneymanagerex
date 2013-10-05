
#pragma once
#include <wx/textctrl.h>
#include <wx/string.h>
#include "model/Model_Currency.h"
#include "model/Model_Account.h"

class mmTextCtrl : public wxTextCtrl
{
public:
    using wxTextCtrl::SetValue;
public:
    mmTextCtrl() {}
    mmTextCtrl(wxWindow *parent, wxWindowID id, const wxString &value=wxEmptyString
            , const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize
            , long style=0, const wxValidator &validator=wxDefaultValidator, const wxString &name=wxTextCtrlNameStr)
            : wxTextCtrl(parent, id, value, pos, size, style, validator, name)
            {}
    void SetValue(double value)
    {
        this->SetValue(Model_Currency::toString(value));
    }
    void SetValue(double value, const Model_Account::Data* account)
    {
       this->SetValue(Model_Account::toString(value, account)); 
    }
};


