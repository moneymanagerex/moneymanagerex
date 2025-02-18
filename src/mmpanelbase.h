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

typedef struct {
    wxString header;
    int width;
    int format;
    bool sortable;
} ListColumnInfo;

class mmListCtrl : public wxListCtrl
{
public:
    std::vector<int> m_column_order;       // map: col_nr -> col_id
    std::vector<ListColumnInfo> m_columns; // map: col_nr -> ListColumnInfo
    int m_default_sort_column = -1;
    bool m_asc = true;
    long m_selected_row = -1;
    int m_selected_col = 0;
    wxString m_col_width_fmt;
    wxString m_col_type_str;
    wxSharedPtr<wxListItemAttr> attr1_, attr2_; // style1, style2

public:
    mmListCtrl(wxWindow *parent, wxWindowID winid);
    virtual ~mmListCtrl();

    virtual wxListItemAttr* OnGetItemAttr(long row) const;

    void saveColumnOrder(std::vector<int> columnList);
    std::vector<int> loadColumnOrder();
    void saveColumnWidth(int column_number, int column_width);
    int loadColumnWidth(int column_number, int default_size = wxLIST_AUTOSIZE);
    wxString BuildPage(const wxString &title) const;

protected:
    wxDECLARE_EVENT_TABLE();
    enum {
        MENU_HEADER_HIDE = wxID_HIGHEST + 2000,
        MENU_HEADER_SORT,
        MENU_HEADER_RESET,
        MENU_HEADER_MOVE_LEFT,
        MENU_HEADER_MOVE_RIGHT,
        MENU_HEADER_COLUMN, // Must be last in list
    };

protected:
    int m_ColumnHeaderNbr = -1;

protected:
    void createColumns();
    int getColumnId(int col_nr) const;
    int getColumnNr(int col_id) const;

    virtual void OnColClick(wxListEvent& event);

    void onItemResize(wxListEvent& event);
    void onColRightClick(wxListEvent& event);
    // Headers Right Click
    void onHeaderPopup(wxCommandEvent& event);
    void onHeaderColumn(wxCommandEvent& event);
    void onHeaderHide(wxCommandEvent& WXUNUSED(event));
    void onHeaderSort(wxCommandEvent& event);
    void onHeaderReset(wxCommandEvent& WXUNUSED(event));
    void onHeaderMove(wxCommandEvent& WXUNUSED(event), int direction);
};

//----------------------------------------------------------------------------

class mmPanelBase : public wxPanel
{
public:
    mmPanelBase();
    virtual ~mmPanelBase();

    virtual wxString BuildPage() const;
    virtual void PrintPage();
    virtual void sortTable() = 0;

    void windowsFreezeThaw();
};

