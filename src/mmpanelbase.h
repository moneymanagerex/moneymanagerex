/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 James Higley
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 George Ef (george.a.ef@gmail.com)

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
#include "util.h"
#include <wx/event.h>
#include <wx/webview.h>
#include <wx/webviewfshandler.h>

// short names for wx macros
#define _WA wxLIST_AUTOSIZE
#define _WH wxLIST_AUTOSIZE_USEHEADER
#define _FL wxLIST_FORMAT_LEFT
#define _FR wxLIST_FORMAT_RIGHT
#define _FC wxLIST_FORMAT_CENTER

struct ListColumnInfo
{
    int id;
    bool translate;
    wxString header;
    int default_width;
    int format;
    bool sortable;

    static const std::vector<int> getListId(const std::vector<ListColumnInfo>& a_info);
};

//----------------------------------------------------------------------------

class mmListCtrl : public wxListCtrl
{
protected:
    wxDECLARE_EVENT_TABLE();
    enum {
        MENU_HEADER_SORT = wxID_HIGHEST + 2000,
        MENU_HEADER_TOGGLE_MIN,
        MENU_HEADER_TOGGLE_MAX = MENU_HEADER_TOGGLE_MIN + 99,
        MENU_HEADER_HIDE,
        MENU_HEADER_SHOW_MIN,
        MENU_HEADER_SHOW_MAX = MENU_HEADER_SHOW_MIN + 99,
        MENU_HEADER_MOVE_LEFT,
        MENU_HEADER_MOVE_RIGHT,
        MENU_HEADER_RESET,
    };

public:
    // configured by constructor (cannot be updated)
    const wxSharedPtr<wxListItemAttr> attr1_, attr2_; // style1, style2

    // configured by constructor (but may be updated)
    wxString m_setting_name;                   // name for settings
    wxString o_col_order_prefix;               // v1.9.0 prefix for column order
    wxString o_col_width_prefix;               // v1.9.0 prefix for column width
    wxString o_sort_prefix;                    // v1.9.0 prefix for sort
    std::vector<ListColumnInfo> m_col_info_id; // map: col_id -> col_info
    std::unordered_set<int> m_col_disabled_id; // set: col_id -> isDisabled

    // dynamic
    std::vector<int> m_col_id_nr;              // map: col_nr -> col_id; or empty
    std::vector<int> m_col_width_id;           // map: col_id -> col_width (lazy)
    std::unordered_set<int> m_col_hidden_id;   // map (set): col_id -> isHidden
    std::vector<int> m_sort_col_id;            // sorting col_id; can be empty
    std::vector<bool> m_sort_asc;              // sorting direction
    int m_sel_col_nr = -1;                     // set by onColRightClick()

private:
    std::vector<int> c_sort_col_nr;            // sorting col_nr (cache)
    int c_icon_col_nr = -1;                    // sort icon col_nr (cache)

public:
    mmListCtrl(wxWindow *parent, wxWindowID winid);
    virtual ~mmListCtrl();

    void createColumns();
    wxString BuildPage(const wxString &title) const;

public:
    int getColIdSize() const;
    int getColNrSize() const;
    bool isValidColId(int col_id) const;
    bool isValidColNr(int col_nr) const;
    int getColId_Nr(int col_nr) const;
    int getColNr_Id(int col_id) const;
    int getColNr_Vo(int col_vo) const;
    int getColVo_Nr(int col_nr) const;
    int getColId_Vo(int col_vo) const;
    int getColVo_Id(int col_id) const;
    const wxString getColHeader(int col_id, bool show_icon = false) const;
    bool isDisabledColId(int col_id) const;
    bool isDisabledColNr(int col_nr) const;
    bool isHiddenColId(int col_id) const;
    bool isHiddenColNr(int col_nr) const;
    bool isHiddenColVo(int col_vo) const;
    int getSortColId(int i = 0) const;
    int getSortColNr(int i = 0);
    bool getSortAsc(int i = 0) const;
    void savePreferences();
    void loadPreferences();

private:
    // backwards compatibility
    const wxString getColOrderKey_v190() const;
    const wxString getColWidthKey_v190(int col_id) const;
    const wxString getSortColKey_v190(int i = 0) const;
    const wxString getSortAscKey_v190(int i = 0) const;
    void savePreferences_v190();
    void loadPreferences_v190();
    void saveBoolInt(const wxString& key, bool value, bool isInt);
    bool loadBoolInt(const wxString& key, bool default_value, bool isInt) const;

private:
    int cacheSortColNr(int i);

protected:
    virtual wxListItemAttr* OnGetItemAttr(long row) const;
    virtual void OnColClick(wxListEvent& event);
    virtual int getSortIcon(bool asc) const;
    void updateSortIcon();

private:
    void shiftColumn(int col_vo, int offset);

    void onItemResize(wxListEvent& event);
    void onColRightClick(wxListEvent& event);
    // Headers Right Click
    void onHeaderPopup(wxCommandEvent& event);
    void onHeaderSort(wxCommandEvent& event);
    void onHeaderToggle(wxCommandEvent& event);
    void onHeaderHide(wxCommandEvent& WXUNUSED(event));
    void onHeaderShow(wxCommandEvent& WXUNUSED(event));
    void onHeaderMove(wxCommandEvent& WXUNUSED(event), int dir);
    void onHeaderReset(wxCommandEvent& WXUNUSED(event));
};

inline int mmListCtrl::getColIdSize() const
{
    return static_cast<int>(m_col_info_id.size());
}

inline int mmListCtrl::getColNrSize() const
{
    return !m_col_id_nr.empty() ?
        static_cast<int>(m_col_id_nr.size()) :
        static_cast<int>(m_col_info_id.size());
}

inline bool mmListCtrl::isValidColId(int col_id) const
{
    return (col_id >= 0 && col_id < getColIdSize());
}

inline bool mmListCtrl::isValidColNr(int col_nr) const
{
    return (col_nr >= 0 && col_nr < getColNrSize());
}

inline int mmListCtrl::getColId_Nr(int col_nr) const
{
    return m_col_id_nr.empty() ? col_nr : m_col_id_nr[col_nr];
}

inline int mmListCtrl::getColNr_Id(int col_id) const
{
    return m_col_id_nr.empty() ? col_id :
        std::find(m_col_id_nr.begin(), m_col_id_nr.end(), col_id) - m_col_id_nr.begin();
}

inline int mmListCtrl::getColNr_Vo(int col_vo) const
{
    // Return the column number (inedx) from the visual order.
    // The column number and the visual order are always equal on Linux/macOS,
    // but on Windows they may differ due to drag/drop actions.
    #ifdef wxHAS_LISTCTRL_COLUMN_ORDER
        return GetColumnIndexFromOrder(col_vo);
    #else
        return col_vo;
    #endif
}

inline int mmListCtrl::getColVo_Nr(int col_nr) const
{
    // Return the visual order from the column number (inedx).
    #ifdef wxHAS_LISTCTRL_COLUMN_ORDER
        return GetColumnOrder(col_nr);
    #else
        return col_nr;
    #endif
}

inline int mmListCtrl::getColId_Vo(int col_vo) const
{
    return getColId_Nr(getColNr_Vo(col_vo));
}

inline int mmListCtrl::getColVo_Id(int col_id) const
{
    return getColVo_Nr(getColNr_Id(col_id));
}

inline bool mmListCtrl::isDisabledColId(int col_id) const
{
    return m_col_disabled_id.find(col_id) != m_col_disabled_id.end();
}

inline bool mmListCtrl::isDisabledColNr(int col_nr) const
{
    return isDisabledColId(getColId_Nr(col_nr));
}

inline bool mmListCtrl::isHiddenColId(int col_id) const
{
    return m_col_hidden_id.find(col_id) != m_col_hidden_id.end();
}

inline bool mmListCtrl::isHiddenColNr(int col_nr) const
{
    return isHiddenColId(getColId_Nr(col_nr));
}

inline bool mmListCtrl::isHiddenColVo(int col_vo) const
{
    return isHiddenColId(getColId_Vo(col_vo));
}

inline int mmListCtrl::getSortColId(int i) const
{
    return m_sort_col_id[i];
}

inline int mmListCtrl::getSortColNr(int i)
{
    int col_id = m_sort_col_id[i];
    int col_nr = c_sort_col_nr[i];
    return (isValidColNr(col_nr) && getColId_Nr(col_nr) == col_id) ? col_nr :
        cacheSortColNr(i);
}

inline bool mmListCtrl::getSortAsc(int i) const
{
    return m_sort_asc[i];
}

inline const wxString mmListCtrl::getColOrderKey_v190() const
{
    return o_col_order_prefix + "_COLUMNORDER";
}

inline const wxString mmListCtrl::getColWidthKey_v190(int col_id) const
{
    return wxString::Format(o_col_width_prefix + "%d_WIDTH", col_id);
}

inline const wxString mmListCtrl::getSortColKey_v190(int i) const
{
    return wxString::Format("%s_SORT_COL%s", o_sort_prefix, (i == 1 ? "2" : ""));
}

inline const wxString mmListCtrl::getSortAscKey_v190(int i) const
{
    return wxString::Format("%s_ASC%s", o_sort_prefix, (i == 1 ? "2" : ""));
}

//----------------------------------------------------------------------------

class mmPanelBase : public wxPanel
{
public:
    mmPanelBase();
    virtual ~mmPanelBase();

    virtual wxString BuildPage() const;
    virtual void PrintPage();
    virtual void sortList() = 0;

    void windowsFreezeThaw();
};

