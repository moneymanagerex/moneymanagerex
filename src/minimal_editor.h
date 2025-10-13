/*******************************************************
 Copyright (C) 2008 Troels K
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

#pragma once
#include <wx/stc/stc.h>

class MinimalEditor : public wxStyledTextCtrl
{
    wxDECLARE_DYNAMIC_CLASS(MinimalEditor);
    wxDECLARE_EVENT_TABLE();

public:
    MinimalEditor(){};
    MinimalEditor(wxWindow* parent, wxWindowID id = wxID_ANY);
    void SetLexerSql();
    void SetLexerLua();
    void SetLexerHtml();
    void SetLexerLuaNew();

protected:
    void OnMarginClick(wxStyledTextEvent&);
    wxFont m_font;

private:
    void SetCommonLexarProperties();
};
