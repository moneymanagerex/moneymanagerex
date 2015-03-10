/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 James Higley

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

#include "util.h"
#include <wx/listctrl.h>
#include <wx/webview.h>
#include <wx/webviewfshandler.h>
//----------------------------------------------------------------------------

class wxListItemAttr;

class mmListCtrl: public wxListCtrl
{
public:
    mmListCtrl(wxWindow *parent, wxWindowID winid);
    virtual ~mmListCtrl();

    wxListItemAttr *attr1_, *attr2_; // style1
    long m_selected_row;
    int m_selected_col;
    bool m_asc;

    virtual wxListItemAttr* OnGetItemAttr(long row) const;
    wxString BuildPage(const wxString &title) const;
};

class mmPanelBase : public wxPanel
{
public:
    mmPanelBase();
    virtual ~mmPanelBase();

    virtual wxString BuildPage() const;
    virtual void PrintPage();
    void windowsFreezeThaw();

    virtual void sortTable() = 0;
};
//----------------------------------------------------------------------------
