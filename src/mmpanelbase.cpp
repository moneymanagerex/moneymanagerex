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
    EVT_LIST_COL_END_DRAG(wxID_ANY,    mmListCtrl::onItemResize)
    EVT_LIST_COL_CLICK(wxID_ANY,       mmListCtrl::OnColClick)
    EVT_LIST_COL_RIGHT_CLICK(wxID_ANY, mmListCtrl::onColRightClick)
    EVT_MENU(wxID_ANY,                 mmListCtrl::onHeaderPopup)
wxEND_EVENT_TABLE()

mmListCtrl::mmListCtrl(wxWindow *parent, wxWindowID winid) :
    wxListCtrl(
        parent, winid, wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_VIRTUAL | wxLC_SINGLE_SEL | wxLC_AUTOARRANGE
    ),
    attr1_(new wxListItemAttr(
        *bestFontColour(mmThemeMetaColour(meta::COLOR_LISTALT0)),
        mmThemeMetaColour(meta::COLOR_LISTALT0),
        wxNullFont
    )),
    attr2_(new wxListItemAttr(
        *bestFontColour(mmThemeMetaColour(meta::COLOR_LIST)),
        mmThemeMetaColour(meta::COLOR_LIST),
        wxNullFont
    ))
{
}

mmListCtrl::~mmListCtrl()
{
    // Save the column widths of the list control. This will ensure that the
    // column widths get set incase the onItemResize does not work on some systems.
    std::vector<int> columnOrder;
    for (int column_number = 0; column_number < GetColumnCount(); ++column_number) {
        int column_width = GetColumnWidth(column_number);
        if (loadColumnWidth(column_number) != column_width)
            saveColumnWidth(column_number, column_width);

        #ifdef wxHAS_LISTCTRL_COLUMN_ORDER
        if (m_column_order.size() > 0)
            columnOrder.push_back(m_column_order[GetColumnIndexFromOrder(column_number)]);
        #endif
    }

    if (!columnOrder.empty())
        saveColumnOrder(columnOrder);
}

//----------------------------------------------------------------------------

wxListItemAttr* mmListCtrl::OnGetItemAttr(long row) const
{
    return (row % 2) ? attr2_.get() : attr1_.get();
}

// Set new column order. Called when closing the dialog using the "OK" button
void mmListCtrl::saveColumnOrder(std::vector<int> columnList)
{
    if (columnList.empty())
        columnList = m_column_order;

    wxString columnOrder;
    for (int col_enum : columnList) {
        columnOrder.Append((columnOrder.IsEmpty() ? "" : "|") + wxString::Format("%i", col_enum));
    }
    Model_Setting::instance().setString(m_col_type_str + "_COLUMNORDER", columnOrder);
}

// Get the current column order from the settings, or initialize a default order
std::vector<int> mmListCtrl::loadColumnOrder()
{
    wxString order_str = Model_Setting::instance().getString(m_col_type_str + "_COLUMNORDER", "");
    wxArrayString order_a = wxSplit(order_str, '|');
    if (order_a.IsEmpty())
        return m_column_order;

    std::vector<int> order;
    for (const auto& col_str : order_a) {
        int col_id = wxAtoi(col_str);
        if (std::find(m_column_order.begin(), m_column_order.end(), col_id) !=
            m_column_order.end()
        )
            order.push_back(col_id);
    }

    // add missing columns
    for (int col_id : m_column_order) {
        if (std::find(order.begin(), order.end(), col_id) == order.end())
            order.push_back(col_id);
    }

    return order;
}

void mmListCtrl::saveColumnWidth(int column_number, int column_width)
{
    if (m_col_width_fmt.IsEmpty())
        return;

    wxString key = wxString::Format(m_col_width_fmt, getColumnId(column_number));
    Model_Setting::instance().setInt(key, column_width);
}

int mmListCtrl::loadColumnWidth(int column_number, int default_size)
{
    wxString key = wxString::Format(m_col_width_fmt, getColumnId(column_number));
    return Model_Setting::instance().getInt(key, default_size);
}

wxString mmListCtrl::BuildPage(const wxString &title) const
{
    const wxString eol = wxTextFile::GetEOL();
    wxString text = eol;
    text << "<head>" + eol + "<title>" + title + "</title>" + eol;
    text << "<meta charset = 'utf-8'>" + eol + "</head>" + eol;
    text << "<body>" << eol;
    text << wxString::Format(
        "<table border=%s cellpadding=4 cellspacing=0 >",
        (GetWindowStyle() & wxLC_HRULES) || (GetWindowStyle() & wxLC_VRULES) ? "1" : "0"
    ) + eol;

    text << "<tr>" << eol;
    for (int c = 0; c < GetColumnCount(); c++) {
        wxListItem col;
        col.SetMask(wxLIST_MASK_TEXT);
        GetColumn(c, col);
        text << "<th><i>" << col.GetText() << "</i></th>" << eol;
    }
    text << "</tr>" << eol;

    for (int i = 0; i < GetItemCount(); i++) {
        text << "<tr>" << eol;
        for (int col = 0; col < GetColumnCount(); col++) {
            text << "<td>" << wxListCtrl::GetItemText(i, col) << "</td>" << eol;
        }
        text << eol << "</tr>" << eol;
    }
    text << "</table>" << eol;
    text << "</body>" << eol;
    text = wxString::Format("<!DOCTYPE html>%s<html>%s</html>%s", eol, text, eol);

    return text;
}

//----------------------------------------------------------------------------

void mmListCtrl::createColumns()
{
    std::vector<int> columnOrder = loadColumnOrder();
    std::vector<ListColumnInfo> columns = {};
    for (unsigned int i = 0; i < columnOrder.size(); i++) {
        unsigned int index = std::find(m_column_order.begin(), m_column_order.end(), columnOrder[i]) - m_column_order.begin();
        if (index < m_columns.size())
            columns.push_back(m_columns[index]);
    }

    m_column_order = columnOrder;
    m_columns = columns;

    for (const auto& entry : m_columns) {
        int count = GetColumnCount();
        wxString key = wxString::Format(m_col_width_fmt, getColumnId(count));
        int width = Model_Setting::instance().getInt(key, entry.width);
        InsertColumn(count, entry.header, entry.format, width);
    }
}

int mmListCtrl::getColumnId(int col_nr) const
{
    return m_column_order.empty() ? col_nr : m_column_order[col_nr];
}

int mmListCtrl::getColumnNr(int col_id) const
{
    return m_column_order.empty() ? col_id :
        std::find(m_column_order.begin(), m_column_order.end(), col_id) - m_column_order.begin();
}

//----------------------------------------------------------------------------

void mmListCtrl::OnColClick(wxListEvent& WXUNUSED(event))
{
    // Default to do nothing and implement in derived class
}

//----------------------------------------------------------------------------

void mmListCtrl::onItemResize(wxListEvent& event)
{
    int i = event.GetColumn();
    int width = GetColumnWidth(i);
    if (!m_col_width_fmt.IsEmpty()) {
        wxString key = wxString::Format(m_col_width_fmt, getColumnId(i));
        Model_Setting::instance().setInt(key, width);
    }
}

void mmListCtrl::onColRightClick(wxListEvent& event)
{
    if (m_columns.size() == 0 || m_col_width_fmt.IsEmpty())
        return;

    m_ColumnHeaderNbr = event.GetColumn();
    if (m_ColumnHeaderNbr < 0 || m_ColumnHeaderNbr >= static_cast<int>(m_columns.size()))
        return;

    wxMenu menu;
    wxMenu *submenu = new wxMenu;
    for (int i = 0; i < static_cast<int>(m_columns.size()); i++) {
        int id = MENU_HEADER_COLUMN + i;
        submenu->AppendCheckItem(id, m_columns[i].header);
        wxString key = wxString::Format(m_col_width_fmt, getColumnId(i));
        int width = Model_Setting::instance().getInt(key, m_columns[i].width);
        submenu->Check(id, width != 0);
    }
    menu.AppendSubMenu(submenu, _t("Hide/Show Columns"));
    menu.Append(MENU_HEADER_HIDE, _t("Hide this column"));
    if (m_default_sort_column >= 0 && m_columns[m_ColumnHeaderNbr].sortable)
        menu.Append(MENU_HEADER_SORT, _t("Order by this column"));
    // Do not show e.g. for Assets root list. Only for sublists.
    if (m_column_order.size() > 0) {
        menu.Append(MENU_HEADER_MOVE_LEFT, _t("Move column left"));
        menu.Append(MENU_HEADER_MOVE_RIGHT, _t("Move column right"));
    }
    menu.Append(MENU_HEADER_RESET, _t("Reset columns"));
    PopupMenu(&menu);
    this->SetFocus();
}

void mmListCtrl::onHeaderPopup(wxCommandEvent& event)
{
    switch (event.GetId()) {
    case MENU_HEADER_HIDE:
        onHeaderHide(event);
        break;
    case MENU_HEADER_SORT:
        onHeaderSort(event);
        break;
    case MENU_HEADER_MOVE_LEFT:
        onHeaderMove(event, -1);
        break;
    case MENU_HEADER_MOVE_RIGHT:
        onHeaderMove(event, 1);
        break;
    case MENU_HEADER_RESET:
        onHeaderReset(event);
        break;
    default:
        onHeaderColumn(event);
    }
}

void mmListCtrl::onHeaderColumn(wxCommandEvent& event)
{
    int id = event.GetId();
    int columnNbr = id - MENU_HEADER_COLUMN;
    if (columnNbr < 0 || columnNbr >= static_cast<int>(m_columns.size()) || m_col_width_fmt.IsEmpty())
        return;

    int default_width = m_columns[columnNbr].width;
    if (default_width == 0)
        default_width = wxLIST_AUTOSIZE_USEHEADER;
    const wxString parameter_name = wxString::Format(m_col_width_fmt, getColumnId(columnNbr));
    int cur_width = Model_Setting::instance().getInt(parameter_name, default_width);
    int new_width = (cur_width != 0 ? 0 : default_width);
    SetColumnWidth(columnNbr, new_width);
    Model_Setting::instance().setInt(parameter_name, GetColumnWidth(columnNbr));
}

void mmListCtrl::onHeaderHide(wxCommandEvent& WXUNUSED(event))
{
    if (m_ColumnHeaderNbr < 0 || m_col_width_fmt.IsEmpty())
        return;

    SetColumnWidth(m_ColumnHeaderNbr, 0);
    const wxString key = wxString::Format(m_col_width_fmt, getColumnId(m_ColumnHeaderNbr));
    Model_Setting::instance().setInt(key, 0);
}

void mmListCtrl::onHeaderSort(wxCommandEvent& WXUNUSED(event))
{
    wxListEvent e;
    e.SetId(MENU_HEADER_SORT);
    OnColClick(e);
}

void mmListCtrl::onHeaderReset(wxCommandEvent& WXUNUSED(event))
{
    wxString parameter_name;
    Freeze();
    for (int i = 0; i < static_cast<int>(m_columns.size()); i++) {
        SetColumnWidth(i, m_columns[i].width);
        if (!m_col_width_fmt.IsEmpty()) {
            parameter_name = wxString::Format(m_col_width_fmt, getColumnId(i));
            Model_Setting::instance().setInt(parameter_name, GetColumnWidth(i));
        }
    }
    wxListEvent e;
    e.SetId(MENU_HEADER_RESET);
    m_ColumnHeaderNbr = m_default_sort_column;
    m_asc = true;
    OnColClick(e);
    Thaw();
}

void mmListCtrl::onHeaderMove(wxCommandEvent& WXUNUSED(event), int direction)
{
    Freeze();

    #ifdef wxHAS_LISTCTRL_COLUMN_ORDER
    // on Windows the visual order can differ from the array order due to drag/drop
    // so we need to realign them before adjusting the column orders programatically
    std::vector<int> realColumns, widths;

    wxArrayInt columnorder;

    std::vector<ListColumnInfo> columns;
    bool reindexSelection = false;
    for (int i = 0; i < m_columns.size(); i++) {
        // we will reset the visual indices in sequential order
        columnorder.push_back(i);

        // get the true index from the visual column position
        int index = GetColumnIndexFromOrder(i);

        // update the selected column index
        if (index == m_ColumnHeaderNbr && !reindexSelection) {
            m_ColumnHeaderNbr = i;
            reindexSelection = true;
        }

        realColumns.push_back(m_column_order[index]);
        columns.push_back(m_columns[index]);
        int width = GetColumnWidth(i);
        wxListItem column;
        column.SetText(m_columns[index].header);
        column.SetAlign(static_cast<wxListColumnFormat>(m_columns[index].format));
        SetColumn(i, column);
        SetColumnWidth(i, width);
    }

    SetColumnsOrder(columnorder);
    m_column_order = realColumns;
    m_columns = columns;
    #endif

    // find the next visible column
    int distance = direction;
    while (m_ColumnHeaderNbr + distance > 0 &&
        m_ColumnHeaderNbr + distance < static_cast<int>(m_columns.size()) - 1 &&
        GetColumnWidth(m_ColumnHeaderNbr + distance) == 0
    ) {
        distance += direction;
    }
    wxLogDebug("Moving column %d (%s) %d", m_ColumnHeaderNbr, m_columns[m_ColumnHeaderNbr].header.c_str(), distance);
    if (m_ColumnHeaderNbr + distance >= 0 &&
        static_cast<int>(m_columns.size()) > m_ColumnHeaderNbr + distance &&
        static_cast<int>(m_column_order.size()) > m_ColumnHeaderNbr + distance
    ) {
        // swap order of column data
        std::swap(m_column_order[m_ColumnHeaderNbr + distance], m_column_order[m_ColumnHeaderNbr]);
        std::swap(m_columns[m_ColumnHeaderNbr + distance], m_columns[m_ColumnHeaderNbr]);
        saveColumnOrder(m_column_order);
    
        // swap column headers & widths
        wxListItem col1, col2;
        col1.SetText(m_columns[m_ColumnHeaderNbr].header);
        col1.SetAlign(static_cast<wxListColumnFormat>(m_columns[m_ColumnHeaderNbr].format));
        col2.SetText(m_columns[m_ColumnHeaderNbr + distance].header);
        col2.SetAlign(static_cast<wxListColumnFormat>(m_columns[m_ColumnHeaderNbr + distance].format));
        int width = GetColumnWidth(m_ColumnHeaderNbr);
        SetColumn(m_ColumnHeaderNbr, col1);
        SetColumnWidth(m_ColumnHeaderNbr, GetColumnWidth(m_ColumnHeaderNbr + distance));
        SetColumn(m_ColumnHeaderNbr + distance, col2);
        SetColumnWidth(m_ColumnHeaderNbr + distance, width);
        Thaw();
    }
}

//----------------------------------------------------------------------------

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
    htmlWindow->GetPageSource(); // Needed to generate the page - at least on Mac anyway!
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

