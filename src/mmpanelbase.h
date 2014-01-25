/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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
#include "reports/mmgraphgenerator.h"
#include <wx/listctrl.h>
//----------------------------------------------------------------------------

class wxSQLite3Database;
class wxListItemAttr;

class mmListCtrl: public wxListCtrl
{
public:
    mmListCtrl(wxWindow *parent, wxWindowID winid)
        : wxListCtrl(parent, winid, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_HRULES|wxLC_VRULES|wxLC_VIRTUAL|wxLC_SINGLE_SEL|wxLC_EDIT_LABELS)
        , attr1_(new wxListItemAttr(mmColors::listBorderColor, mmColors::listAlternativeColor0, wxNullFont))
        , attr2_(new wxListItemAttr(mmColors::listBorderColor, mmColors::listAlternativeColor1, wxNullFont))
        , m_selected_row(-1)
        , m_selected_col(0)
        , m_asc(true)
    {}
    virtual ~mmListCtrl()
    {
        if (attr1_) delete attr1_;
        if (attr2_) delete attr2_;
    }
public:
    wxListItemAttr *attr1_, *attr2_; // style1
    long m_selected_row;
    int m_selected_col;
    bool m_asc;

public:
    virtual wxListItemAttr* OnGetItemAttr(long row) const
    {
        return (row % 2) ? attr2_ : attr1_;
    }
    wxString BuildPage() const
    {
        wxString text;
        text << "<TABLE ";

        if ((GetWindowStyle() & wxLC_HRULES) ||
          (GetWindowStyle() & wxLC_VRULES))
            text << "border=1";
        else
            text << "border=0";

            text << " cellpadding=4 cellspacing=0 >" << wxTextFile::GetEOL();

            text << "<tr>" << wxTextFile::GetEOL();

        for (int c = 0; c < GetColumnCount(); c++)
        { 
            wxListItem col;
            GetColumn(c, col);
            text << "<td><i>" << col.GetText() << "</i>" << wxTextFile::GetEOL();
        }

        for (int i = 0; i < GetItemCount(); i++)
        { 
            text << "<tr>" << wxTextFile::GetEOL();

            for (int col = 0; col < GetColumnCount(); col++)
            {
                text << "<td>" << wxListCtrl::GetItemText(i, col) << wxTextFile::GetEOL();
            }
        }

        text << "</TABLE>" << wxTextFile::GetEOL();

        return text;
    }
};

class mmPanelBase : public wxPanel
{
public:
    mmPanelBase() {}
    virtual ~mmPanelBase() {mmGraphGenerator::cleanup();}

public:
    wxString BuildPage() const { return "TBD"; }
    void windowsFreezeThaw()
    {
#ifdef __WXGTK__
        return;
#endif

        if (this->IsFrozen())
            this->Thaw();
        else
            this->Freeze();
    }
public:
    virtual void sortTable() = 0;
};
//----------------------------------------------------------------------------
