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

// Validators -----------------------------------------------------
class mmCalcValidator : public wxTextValidator
{
public:
    mmCalcValidator() : wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST)
    {
        wxArrayString list;
        for (const auto& c : "1234567890.,(/+-*)")
        {
            list.Add(c);
        }
        SetIncludes(list);
    }

};

class mmDoubleValidator: public wxFloatingPointValidator<double>
{
public:
    mmDoubleValidator(int precision = 2, int max = 100000000)
        : wxFloatingPointValidator<double>(precision, NULL, wxNUM_VAL_NO_TRAILING_ZEROES)
    {
        this->SetRange(0, max);
    }
};

