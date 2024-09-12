/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 James Higley
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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
#include "wx/event.h"
#include <wx/webview.h>
#include <wx/webviewfshandler.h>
//----------------------------------------------------------------------------

struct PANEL_COLUMN
{
    PANEL_COLUMN(const wxString & header, int width, int format, bool sortable)
        : HEADER(header), WIDTH(width), FORMAT(format), SORTABLE(sortable)
    {}
    wxString HEADER;
    int WIDTH;
    int FORMAT;
    bool SORTABLE;
};

class mmListCtrl : public wxListCtrl
{
    wxDECLARE_EVENT_TABLE();

public:
    mmListCtrl(wxWindow *parent, wxWindowID winid);
    virtual ~mmListCtrl();

    wxSharedPtr<wxListItemAttr> attr1_, attr2_; // style1, style2
    long m_selected_row = -1;
    int m_selected_col = 0;
    bool m_asc = true;
    std::vector<PANEL_COLUMN> m_columns;
    std::vector<int> m_real_columns; // map from actual column to EColumn when list can have optional columns
    wxString m_col_width;
    wxString m_col_idstr;
    int m_default_sort_column = -1;

    virtual wxListItemAttr* OnGetItemAttr(long row) const;
    wxString BuildPage(const wxString &title) const;
    int GetColumnWidthSetting(int column_number, int default_size = wxLIST_AUTOSIZE);
    void SetColumnWidthSetting(int column_number, int column_width);

    void SetColumnsOrder(wxArrayString columnList);
    wxArrayString GetColumnsOrder();

protected:
    void OnItemResize(wxListEvent& event);
    virtual void OnColClick(wxListEvent& event);
    void OnColRightClick(wxListEvent& event);
    /* Headers Right Click*/
    void PopupSelected(wxCommandEvent& event);
    void OnHeaderColumn(wxCommandEvent& event);
    void OnHeaderHide(wxCommandEvent& WXUNUSED(event));
    void OnHeaderSort(wxCommandEvent& event);
    void OnHeaderReset(wxCommandEvent& WXUNUSED(event));
    void OnHeaderMove(wxCommandEvent& WXUNUSED(event), int direction);
    int GetRealColumn(int col);
    int m_ColumnHeaderNbr = -1;
    enum {
        HEADER = 0,
        WIDTH,
        FORMAT,
        MENU_HEADER_HIDE = wxID_HIGHEST + 2000,
        MENU_HEADER_SORT,
        MENU_HEADER_RESET,
        MENU_HEADER_MOVE_LEFT,
        MENU_HEADER_MOVE_RIGHT,
        MENU_HEADER_COLUMN, // Must be last in list
    };
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
