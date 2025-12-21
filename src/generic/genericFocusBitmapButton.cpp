/*******************************************************
 Copyright (C) 2025 Klaus Wich

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

#include "genericFocusBitmapButton.h"

genFocusBitmapButton::genFocusBitmapButton(wxWindow* parent, wxWindowID id, const wxBitmapBundle &bitmap,
                const wxPoint& pos, const wxSize& size)
    : wxBitmapButton(parent, id, bitmap, pos, size)
{
    Bind(wxEVT_SET_FOCUS, &genFocusBitmapButton::OnFocus, this);
    Bind(wxEVT_KILL_FOCUS, &genFocusBitmapButton::OnKillFocus, this);
    Bind(wxEVT_ENTER_WINDOW, &genFocusBitmapButton::OnMouseEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &genFocusBitmapButton::OnMouseLeave, this);
}

void genFocusBitmapButton::OnFocus(wxFocusEvent& event)
{
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
    Refresh();
    event.Skip();
}

void genFocusBitmapButton::OnKillFocus(wxFocusEvent& event)
{
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    Refresh();
    event.Skip();
}

void genFocusBitmapButton::OnMouseEnter(wxMouseEvent& event)
{
    if (this->IsEnabled()) {
        SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
        Refresh();
        event.Skip();
    }
}

void genFocusBitmapButton::OnMouseLeave(wxMouseEvent& event)
{
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    Refresh();
    event.Skip();
}
