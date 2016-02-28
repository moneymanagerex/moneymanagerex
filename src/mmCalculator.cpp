/*******************************************************
Copyright (C) 2013 Nikolay Akimov

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

#include "mmCalculator.h"
#include "LuaGlue/LuaGlue.h"

mmCalculator::mmCalculator()
{
    output_ = 0;
}

const bool mmCalculator::is_ok(const wxString& input)
{
    bool ok = check_syntax(input);
    LuaGlue state;
    state.open().glue();
    std::string lua_f = "function calc() return " + input.ToStdString() + "; end";
    if(!state.doString(lua_f))
    {
        printf("err: %s\n", state.lastError().c_str());
        return false;
    }

    this->output_ = state.invokeFunction<double>("calc");
    return ok;
}

const bool mmCalculator::check_syntax(const wxString& input) const
{
    wxString temp = input;
    int a = temp.Replace("(", "(");
    int b = temp.Replace(")", ")");
    bool ok = (a == b);
    if (ok && a > 0)
    {
        for (size_t i = 0; i < input.Len(); i++)
        {
            if (input[i] == '(') a += i;
            else if (input[i] == ')') b += i;
            if (i > 0 && input[i] == '(') ok = (ok && (wxString("(+-*/").Contains(input[i-1])));
            if (i < input.Len()-1 && input[i] == ')') ok = (ok && wxString(")+-*/").Contains(input[i+1]));
        }
        ok = (a < b);
    }
    return ok;
}
