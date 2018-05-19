/*******************************************************
Copyright (C) 2006-2012 Nikolay Akimov

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
#include <wx/valtext.h>

// Validators -----------------------------------------------------
class mmCalcValidator : public wxTextValidator
{
public:
    mmCalcValidator();
    virtual ~mmCalcValidator(){}

    virtual wxObject *Clone() const { return new mmCalcValidator(*this); }

    // Filter keystrokes
    void OnChar(wxKeyEvent& event);

private:
    wxDECLARE_NO_ASSIGN_CLASS(mmCalcValidator);
    DECLARE_DYNAMIC_CLASS(mmCalcValidator)
    DECLARE_EVENT_TABLE()
};
