/*******************************************************
Copyright (C) 2014 Stefano Giorgio

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
Foundation, Inc., 59 Temple Placeuite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#include "cpu_timer.h"
#include <iostream>
//----------------------------------------------------------------------------

CpuTimer::CpuTimer(const wxString& msg)
{
    m_message = msg;
}

CpuTimer::~CpuTimer()
{
    std::cout << wxString::Format("\n    %.5d ms for: %s", m_start.Time(), m_message.c_str()).char_str();
}
