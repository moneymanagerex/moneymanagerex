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

#include "images_list.h"
#include "mmpanelbase.h"
#include "model/Model_Setting.h"
#include "mmreportspanel.h"

wxBEGIN_EVENT_TABLE(mmListCtrl, wxListCtrl)
EVT_LIST_COL_END_DRAG(wxID_ANY, mmListCtrl::OnItemResize)
EVT_LIST_COL_CLICK(wxID_ANY, mmListCtrl::OnColClick)
EVT_LIST_COL_RIGHT_CLICK(wxID_ANY, mmListCtrl::OnColRightClick)
EVT_MENU(wxID_ANY, mmListCtrl::PopupSelected)
wxEND_EVENT_TABLE()

mmListCtrl::mmListCtrl(wxWindow *parent, wxWindowID winid)
    : wxListCtrl(parent, winid, wxDefaultPosition, wxDefaultSize
        , wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_VIRTUAL | wxLC_SINGLE_SEL | wxLC_AUTOARRANGE)
    , attr1_(new wxListItemAttr(*bestFontColour(mmThemeMetaColour(meta::COLOR_LISTALT0)), mmThemeMetaColour(meta::COLOR_LISTALT0), wxNullFont))
    , attr2_(new wxListItemAttr(*bestFontColour(mmThemeMetaColour(meta::COLOR_LIST)), mmThemeMetaColour(meta::COLOR_LIST), wxNullFont))
{
}

mmListCtrl::~mmListCtrl()
{
    /*
      Save the column widths of the list control. This will ensure that the
      column widths get set incase the OnItemResize does not work on some systems.
    */
    std::vector<int> columnOrder;
    for (int column_number = 0; column_number < GetColumnCount(); ++column_number)
    {
        int column_width = GetColumnWidth(column_number);
        if (GetColumnWidthSetting(column_number) != column_width)
        {
            SetColumnWidthSetting(column_number, column_width);
        }

#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
        if (m_real_columns.size() > 0)
            columnOrder.push_back(m_real_columns[GetColumnIndexFromOrder(column_number)]);
#endif

    }

    if (!columnOrder.empty())
        SetColumnOrder(columnOrder);
}

wxListItemAttr* mmListCtrl::OnGetItemAttr(long row) const
{
    return (row % 2) ? attr2_.get() : attr1_.get();
}

int mmListCtrl::GetRealColumn(int col)
{
    return (0 == m_real_columns.size()) ? col : m_real_columns[col];
}

wxString mmListCtrl::BuildPage(const wxString &title) const
{
    const wxString eol = wxTextFile::GetEOL();
    wxString text = eol;
    text << "<head>" + eol + "<title>" + title + "</title>" + eol;
    text << "<meta charset = 'utf-8'>" + eol + "</head>" + eol;
    text << "<body>" << eol;
    text << wxString::Format("<table border=%s cellpadding=4 cellspacing=0 >"
        , (GetWindowStyle() & wxLC_HRULES) || (GetWindowStyle() & wxLC_VRULES) ? "1" : "0") + eol;

    text << "<tr>" << eol;
    for (int c = 0; c < GetColumnCount(); c++)
    {
        wxListItem col;
        col.SetMask(wxLIST_MASK_TEXT);
        GetColumn(c, col);
        text << "<th><i>" << col.GetText() << "</i></th>" << eol;
    }
    text << "</tr>" << eol;

    for (int i = 0; i < GetItemCount(); i++)
    {
        text << "<tr>" << eol;
        for (int col = 0; col < GetColumnCount(); col++)
        {
            text << "<td>" << wxListCtrl::GetItemText(i, col) << "</td>" << eol;
        }
        text << eol << "</tr>" << eol;
    }
    text << "</table>" << eol;
    text << "</body>" << eol;
    text = wxString::Format("<!DOCTYPE html>%s<html>%s</html>%s", eol, text, eol);

    return text;
}

void mmListCtrl::OnItemResize(wxListEvent& event)
{
    int i = event.GetColumn();
    int width = GetColumnWidth(i);
    if (!m_col_width.IsEmpty())
        Model_Setting::instance().setInt(wxString::Format(m_col_width, GetRealColumn(i)), width);
}

void mmListCtrl::CreateColumns()
{
    std::vector<int> columnOrder = GetColumnOrder();
    std::vector<PANEL_COLUMN> sortedColumns = {};
    for (unsigned int i = 0; i < columnOrder.size(); i++)
    {
        unsigned int index = std::find(m_real_columns.begin(), m_real_columns.end(), columnOrder[i]) - m_real_columns.begin();
        if (index < m_columns.size())
            sortedColumns.push_back(m_columns[index]);
    }
    
    m_real_columns = columnOrder;
    m_columns = sortedColumns;

    for (const auto& entry : m_columns)
    {
        int count = GetColumnCount();
        InsertColumn(count, entry.HEADER, entry.FORMAT,
                     Model_Setting::instance().getInt(wxString::Format(m_col_width, GetRealColumn(count)), entry.WIDTH));
    }
}

void mmListCtrl::OnColClick(wxListEvent& WXUNUSED(event))
{
    // Default to do nothing and implement in derived class
}

void mmListCtrl::OnColRightClick(wxListEvent& event)
{
    if (m_columns.size() > 0 && !m_col_width.IsEmpty())
    {
        m_ColumnHeaderNbr = event.GetColumn();
        if (0 > m_ColumnHeaderNbr || m_ColumnHeaderNbr >= static_cast<int>(m_columns.size())) return;
        wxMenu menu;
        wxMenu *submenu = new wxMenu;
        for (int i = 0; i < static_cast<int>(m_columns.size()); i++)
        {
            int id = MENU_HEADER_COLUMN + i;
            submenu->AppendCheckItem(id, m_columns[i].HEADER);
            int width = Model_Setting::instance().getInt(wxString::Format(m_col_width, GetRealColumn(i)), m_columns[i].WIDTH);
            submenu->Check(id, width != 0);
        }
        menu.AppendSubMenu(submenu, _("Hide/Show Columns"));
        menu.Append(MENU_HEADER_HIDE, _("Hide this column"));
        if (m_default_sort_column >= 0 && (m_columns[m_ColumnHeaderNbr].SORTABLE == true))
            menu.Append(MENU_HEADER_SORT, _("Order by this column"));
        // Do not show e.g. for Assets root list. Only for sublists.
        if(m_real_columns.size() > 0) {
            menu.Append(MENU_HEADER_MOVE_LEFT, _("Move column left"));
            menu.Append(MENU_HEADER_MOVE_RIGHT, _("Move column right"));
        }
        menu.Append(MENU_HEADER_RESET, _("Reset columns"));
        PopupMenu(&menu);
        this->SetFocus();
    }
}

void mmListCtrl::PopupSelected(wxCommandEvent& event)
{
    switch (event.GetId())
    {
    case MENU_HEADER_HIDE:
        OnHeaderHide(event);
        break;
    case MENU_HEADER_SORT:
        OnHeaderSort(event);
        break;
    case MENU_HEADER_MOVE_LEFT:
        OnHeaderMove(event, -1);
        break;
    case MENU_HEADER_MOVE_RIGHT:
        OnHeaderMove(event, 1);
        break;
    case MENU_HEADER_RESET:
        OnHeaderReset(event);
        break;
    default:
        OnHeaderColumn(event);
    }
}

void mmListCtrl::OnHeaderHide(wxCommandEvent& WXUNUSED(event))
{
    if (m_ColumnHeaderNbr >= 0 && !m_col_width.IsEmpty())
    {
        SetColumnWidth(m_ColumnHeaderNbr, 0);
        const wxString parameter_name = wxString::Format(m_col_width, GetRealColumn(m_ColumnHeaderNbr));
        Model_Setting::instance().setInt(parameter_name, 0);
    }
}

void mmListCtrl::OnHeaderSort(wxCommandEvent& WXUNUSED(event))
{
    wxListEvent e;
    e.SetId(MENU_HEADER_SORT);
    OnColClick(e);
}

void mmListCtrl::OnHeaderMove(wxCommandEvent& WXUNUSED(event), int direction)
{
    Freeze();
#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
    // on Windows the visual order can differ from the array order due to drag/drop
    // so we need to realign them before adjusting the column orders programatically
    std::vector<int> realColumns, widths;

    wxArrayInt columnorder;

    std::vector<PANEL_COLUMN> columns;
    bool reindexSelection = false;
    for (int i = 0; i < m_columns.size(); i++)
    {
        // we will reset the visual indices in sequential order
        columnorder.push_back(i);

        // get the true index from the visual column position
        int index = GetColumnIndexFromOrder(i);

        // update the selected column index
        if (index == m_ColumnHeaderNbr && !reindexSelection)
        {
            m_ColumnHeaderNbr = i;
            reindexSelection = true;
        }

        realColumns.push_back(m_real_columns[index]);
        columns.push_back(m_columns[index]);
        int width = GetColumnWidth(i);
        wxListItem column;
        column.SetText(m_columns[index].HEADER);
        column.SetAlign(static_cast<wxListColumnFormat>(m_columns[index].FORMAT));
        SetColumn(i, column);
        SetColumnWidth(i, width);
    }

    SetColumnsOrder(columnorder);
    m_real_columns = realColumns;
    m_columns = columns;
#endif

    // find the next visible column
    int distance = direction;
    while (m_ColumnHeaderNbr + distance > 0
        && m_ColumnHeaderNbr + distance < static_cast<int>(m_columns.size()) - 1
        && GetColumnWidth(m_ColumnHeaderNbr + distance) == 0)
    {
        distance += direction;
    }
    wxLogDebug("Moving column %d (%s) %d", m_ColumnHeaderNbr, m_columns[m_ColumnHeaderNbr].HEADER.c_str(), distance);
    if (0 <= m_ColumnHeaderNbr + distance
        && static_cast<int>(m_columns.size()) > m_ColumnHeaderNbr + distance
        && static_cast<int>(m_real_columns.size()) > m_ColumnHeaderNbr + distance)
    {
        // swap order of column data
        std::swap(m_real_columns[m_ColumnHeaderNbr + distance], m_real_columns[m_ColumnHeaderNbr]);
        std::swap(m_columns[m_ColumnHeaderNbr + distance], m_columns[m_ColumnHeaderNbr]);
        SetColumnOrder(m_real_columns);
    
        // swap column headers & widths
        wxListItem col1, col2;
        col1.SetText(m_columns[m_ColumnHeaderNbr].HEADER);
        col1.SetAlign(static_cast<wxListColumnFormat>(m_columns[m_ColumnHeaderNbr].FORMAT));
        col2.SetText(m_columns[m_ColumnHeaderNbr + distance].HEADER);
        col2.SetAlign(static_cast<wxListColumnFormat>(m_columns[m_ColumnHeaderNbr + distance].FORMAT));
        int width = GetColumnWidth(m_ColumnHeaderNbr);
        SetColumn(m_ColumnHeaderNbr, col1);
        SetColumnWidth(m_ColumnHeaderNbr, GetColumnWidth(m_ColumnHeaderNbr + distance));
        SetColumn(m_ColumnHeaderNbr + distance, col2);
        SetColumnWidth(m_ColumnHeaderNbr + distance, width);
        Thaw();
    }
}

void mmListCtrl::OnHeaderReset(wxCommandEvent& WXUNUSED(event))
{
    wxString parameter_name;
    Freeze();
    for (int i = 0; i < static_cast<int>(m_columns.size()); i++)
    {
        SetColumnWidth(i, m_columns[i].WIDTH);
        if (!m_col_width.IsEmpty())
        {
            parameter_name = wxString::Format(m_col_width, GetRealColumn(i));
            Model_Setting::instance().setInt(parameter_name, GetColumnWidth(i));
        }
    }
    wxListEvent e;
    e.SetId(MENU_HEADER_SORT);
    m_ColumnHeaderNbr = m_default_sort_column;
    m_asc = true;
    OnColClick(e);
    Thaw();
}

void mmListCtrl::OnHeaderColumn(wxCommandEvent& event)
{
    int id = event.GetId();
    int columnNbr = id - MENU_HEADER_COLUMN;
    if (columnNbr >= 0 && columnNbr < static_cast<int>(m_columns.size()) && !m_col_width.IsEmpty())
    {
        int default_width = m_columns[columnNbr].WIDTH;
        if (default_width == 0)
            default_width = wxLIST_AUTOSIZE_USEHEADER;
        const wxString parameter_name = wxString::Format(m_col_width, GetRealColumn(columnNbr));
        int cur_width = Model_Setting::instance().getInt(parameter_name, default_width);
        int new_width = (cur_width != 0 ? 0 : default_width);
        SetColumnWidth(columnNbr, new_width);
        Model_Setting::instance().setInt(parameter_name, GetColumnWidth(columnNbr));
    }
}

int mmListCtrl::GetColumnWidthSetting(int column_number, int default_size)
{
    return Model_Setting::instance().getInt(
        wxString::Format(m_col_width, GetRealColumn(column_number)),
        default_size
    );
}

void mmListCtrl::SetColumnWidthSetting(int column_number, int column_width)
{
    if (!m_col_width.IsEmpty())
        Model_Setting::instance().setInt(
            wxString::Format(m_col_width, GetRealColumn(column_number)),
            column_width
        );
}

// Set new column order. Called when closing the dialog using the "OK" button
void mmListCtrl::SetColumnOrder(std::vector<int> columnList)
{
    if (columnList.empty())
        columnList = m_real_columns;

    wxString columnOrder;
    for (int col_enum : columnList) {
        columnOrder.Append((columnOrder.IsEmpty() ? "" : "|") + wxString::Format("%i", col_enum));
    }
    Model_Setting::instance().setString(m_col_idstr + "_COLUMNORDER", columnOrder);
}


// Get the current column order from the settings, or initialize a default order
std::vector<int> mmListCtrl::GetColumnOrder()
{
    wxArrayString columnStringList = wxSplit(Model_Setting::instance().getString(m_col_idstr + "_COLUMNORDER", ""), '|');

    // if there is no defined setting, use default order of the listctrl
    if(columnStringList.IsEmpty())
        return m_real_columns;

    // otherwise, read order from settings db
    std::vector<int> columnOrder;
    for (const auto& col_enum : columnStringList)
    {
        if (std::find(m_real_columns.begin(), m_real_columns.end(), wxAtoi(col_enum)) != m_real_columns.end())
            columnOrder.push_back(wxAtoi(col_enum));
    }

    // add missing column enums
    for (int i : m_real_columns)
    {
        if (std::find(columnOrder.begin(), columnOrder.end(), i) == columnOrder.end())
            columnOrder.push_back(i);
    }

    return columnOrder;
}


mmPanelBase::mmPanelBase()
{
}

mmPanelBase::~mmPanelBase()
{
}

wxString mmPanelBase::BuildPage() const
{
    mmReportsPanel* rp = wxDynamicCast(this, mmReportsPanel);
    return rp ? rp->getPrintableBase()->getHTMLText() : "TBD";
}

void mmPanelBase::PrintPage()
{
   //this->Freeze();
    wxWebView * htmlWindow = wxWebView::New(this, wxID_ANY);
    htmlWindow->SetPage(BuildPage(), "");
    htmlWindow->GetPageSource();    // Needed to generate the page - at least on Mac anyway!
    htmlWindow->Print();
    htmlWindow->Destroy();
    //this->Thaw();
}

void mmPanelBase::windowsFreezeThaw()
{
#ifdef __WXGTK__
    return;
#endif

    if (this->IsFrozen())
        this->Thaw();
    else
        this->Freeze();
}
