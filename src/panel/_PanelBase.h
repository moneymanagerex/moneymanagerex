/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 James Higley
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 George Ef (george.a.ef@gmail.com)
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

#include <unordered_set>
#include <wx/event.h>
#include <wx/webview.h>
#include <wx/webviewfshandler.h>
#include "base/defs.h"

#include "util/_util.h"

class PanelBase : public wxPanel
{
public:
    PanelBase();
    virtual ~PanelBase();

    // virtual
    virtual wxString buildPage() const;
    virtual void printPage();
    virtual void sortList() = 0;

    void windowsFreezeThaw();
};
