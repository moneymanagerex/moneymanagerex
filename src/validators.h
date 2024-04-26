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

#include "mmTextCtrl.h"

// Validators -----------------------------------------------------
class mmCalcValidator : public wxTextValidator
{
public:

    mmCalcValidator();
    virtual ~mmCalcValidator(){}

    virtual wxObject *Clone() const;

    // Filter keystrokes
    void OnChar(wxKeyEvent& event);

private:
    wxDECLARE_NO_ASSIGN_CLASS(mmCalcValidator);
    DECLARE_DYNAMIC_CLASS(mmCalcValidator)
    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// mmCalcValidator
// Same as previous, but substitute dec char according to currency configuration
// ----------------------------------------------------------------------------
inline mmCalcValidator::mmCalcValidator() : wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST)
{
    wxArrayString list;
    for (const auto& c : " 1234567890.,(/+-*)")
    {
        list.Add(c);
    }
    SetIncludes(list);
}

inline void mmCalcValidator::OnChar(wxKeyEvent& event)
{
    if (!m_validatorWindow)
        return event.Skip();

    int keyCode = event.GetKeyCode();

    // we don't filter special keys and delete
    if (keyCode < WXK_SPACE || keyCode == WXK_DELETE || keyCode >= WXK_START)
        return event.Skip();


    wxString str(static_cast<wxUniChar>(keyCode), 1);
    if (!(wxIsdigit(str[0]) || wxString("+-.,*/ ()").Contains(str)))
    {
        if (!wxValidator::IsSilent())
            wxBell();

        return; // eat message
    }
    // only if it's a wxTextCtrl
    mmTextCtrl* text_field = wxDynamicCast(m_validatorWindow, mmTextCtrl);
    if (!m_validatorWindow || !text_field)
        return event.Skip();

    wxChar decChar = text_field->GetDecimalPoint();

    bool numpad_dec_swap = false;

    try
    {
        numpad_dec_swap = (wxGetKeyState(WXK_NUMPAD_DECIMAL) && decChar != str);
    }
    catch(...)
    {

    }

    if (numpad_dec_swap)
        str = wxString(decChar);

    // if decimal point, check if it's already in the string
    if (str == decChar)
    {
        const wxString value = text_field->GetValue();
        size_t ind = value.rfind(decChar);
        // Determine selection start/end to allow overwrite of decimal
        long selStart, selEnd;
        text_field->GetSelection(&selStart, &selEnd);
        if (ind < value.Length() && (ind < size_t(selStart) || ind >= size_t(selEnd)))
        {
            // check if after last decimal point there is an operation char (+-/*)
            if (value.find('+', ind + 1) >= value.Length() && value.find('-', ind + 1) >= value.Length() &&
                value.find('*', ind + 1) >= value.Length() && value.find('/', ind + 1) >= value.Length())
                return;
        }
    }

    if (numpad_dec_swap)
        return text_field->WriteText(str);
    else
        event.Skip();
}

inline wxObject* mmCalcValidator::Clone() const { return new mmCalcValidator(); }
