
#pragma once
#include <wx/textctrl.h>
#include <wx/string.h>
#include "model/Model_Currency.h"

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
        wxString value_str = wxString::Format("%f", value);
        this->SetValue(Model_Currency::toString(value));
    }
};


