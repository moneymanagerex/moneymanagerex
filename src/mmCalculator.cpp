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
#include <wx/log.h>

mmCalculator::mmCalculator()
{
    output_ = 0;
}

const double mmCalculator::get_result()
{
    return output_;
}

const bool mmCalculator::is_ok(const wxString& input)
{
    LuaGlue state;
    state.open().glue();
    std::string lua_f = "function calc() return " + input.ToStdString() + "; end";
    if(!state.doString(lua_f))
    {
        wxLogDebug("lua calc() err: %s", state.lastError().c_str());
        return false;
    }

    this->output_ = state.invokeFunction<double>("calc");
    return true;
}
