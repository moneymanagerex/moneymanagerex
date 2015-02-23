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

#include "mmpanelbase.h"
#include "model/Model_Setting.h"

wxBEGIN_EVENT_TABLE(mmListCtrl, wxListCtrl)
EVT_LIST_COL_END_DRAG(wxID_ANY, mmListCtrl::OnItemResize)
EVT_LIST_COL_CLICK(wxID_ANY, mmListCtrl::OnColClick)
EVT_LIST_COL_RIGHT_CLICK(wxID_ANY, mmListCtrl::OnColRightClick)
EVT_MENU(wxID_ANY, mmListCtrl::PopupSelected)
wxEND_EVENT_TABLE()

mmListCtrl::mmListCtrl(wxWindow *parent, wxWindowID winid)
    : wxListCtrl(parent, winid, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_VIRTUAL | wxLC_SINGLE_SEL)
    , attr1_(new wxListItemAttr(mmColors::listBorderColor, mmColors::listAlternativeColor0, wxNullFont))
    , attr2_(new wxListItemAttr(mmColors::listBorderColor, mmColors::listAlternativeColor1, wxNullFont))
    , m_selected_row(-1)
    , m_selected_col(0)
    , m_asc(true)
    , m_ColumnHeaderNbr(-1)
    , m_default_sort_column(-1)
{
}

mmListCtrl::~mmListCtrl()
{
    if (attr1_) delete attr1_;
    if (attr2_) delete attr2_;
}

wxListItemAttr* mmListCtrl::OnGetItemAttr(long row) const
{
    return (row % 2) ? attr2_ : attr1_;
}

wxString mmListCtrl::BuildPage(const wxString &title) const
{
    wxString text;
    text << "<html>" << wxTextFile::GetEOL();
    text << "<head>" << wxTextFile::GetEOL();
    text << "<title>" << title << "</title>" << wxTextFile::GetEOL();
    text << "</head>" << wxTextFile::GetEOL();
    text << "<body>" << wxTextFile::GetEOL();
    text << "<table ";
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
        col.SetMask(wxLIST_MASK_TEXT);
        GetColumn(c, col);
        text << "<th><i>" << col.GetText() << "</i></th>" << wxTextFile::GetEOL();
    }
    text << "</tr>" << wxTextFile::GetEOL();

    for (int i = 0; i < GetItemCount(); i++)
    {
        text << "<tr>" << wxTextFile::GetEOL();
        for (int col = 0; col < GetColumnCount(); col++)
        {
            text << "<td>" << wxListCtrl::GetItemText(i, col) << "</td>" << wxTextFile::GetEOL();
        }
        text << "</tr>" << wxTextFile::GetEOL();
    }
    text << "</table>" << wxTextFile::GetEOL();
    text << "</body>" << wxTextFile::GetEOL();
    text << "</html>" << wxTextFile::GetEOL();

    return text;
}

void mmListCtrl::OnItemResize(wxListEvent& event)
{
    int i = event.GetColumn();
    int width = event.GetItem().GetWidth();
    if (!m_col_width.IsEmpty())
        Model_Setting::instance().Set(wxString::Format(m_col_width, i), width);
}

void mmListCtrl::OnColClick(wxListEvent& event)
{
    // Default to do nothing and implement in derived class
}

void mmListCtrl::OnColRightClick(wxListEvent& event)
{
    if (m_columns.size() > 0 && !m_col_width.IsEmpty())
    {
        m_ColumnHeaderNbr = event.GetColumn();
        if (0 > m_ColumnHeaderNbr || m_ColumnHeaderNbr >= (int)m_columns.size()) return;
        wxMenu menu;
        wxMenu *submenu = new wxMenu;
        for (size_t i = 0; i < m_columns.size(); i++)
        {
            const int id = MENU_HEADER_COLUMN + i;
            submenu->AppendCheckItem(id, std::get<0>(m_columns[i]));
            if (Model_Setting::instance().GetIntSetting(wxString::Format(m_col_width, i), std::get<1>(m_columns[i])) != 0)
                submenu->Check(id, true);
        }
        menu.AppendSubMenu(submenu, _("Hide/Show Columns"));
        menu.Append(MENU_HEADER_HIDE, _("Hide this column"));
        if (m_default_sort_column >= 0)
            menu.Append(MENU_HEADER_SORT, _("Order by this column"));
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
    case MENU_HEADER_RESET:
        OnHeaderReset(event);
        break;
    default:
        OnHeaderColumn(event);
    }
}

void mmListCtrl::OnHeaderHide(wxCommandEvent& event)
{
    if (m_ColumnHeaderNbr >= 0 && !m_col_width.IsEmpty())
    {
        SetColumnWidth(m_ColumnHeaderNbr, 0);
        const wxString parameter_name = wxString::Format(m_col_width, m_ColumnHeaderNbr);
        Model_Setting::instance().Set(parameter_name, 0);
    }
}

void mmListCtrl::OnHeaderSort(wxCommandEvent& event)
{
    wxListEvent e;
    e.SetId(MENU_HEADER_SORT);
    OnColClick(e);
}

void mmListCtrl::OnHeaderReset(wxCommandEvent& event)
{
    wxString parameter_name;
    for (size_t i = 0; i < m_columns.size(); i++)
    {
        SetColumnWidth(i, std::get<1>(m_columns[i]));
        if (!m_col_width.IsEmpty())
        {
            parameter_name = wxString::Format(m_col_width, i);
            Model_Setting::instance().Set(parameter_name, GetColumnWidth(i));
        }
    }
    wxListEvent e;
    e.SetId(MENU_HEADER_SORT);
    m_ColumnHeaderNbr = m_default_sort_column;
    m_asc = true;
    OnColClick(e);
}

void mmListCtrl::OnHeaderColumn(wxCommandEvent& event)
{
    int id = event.GetId();
    int columnNbr = id - MENU_HEADER_COLUMN;
    if (columnNbr >= 0 && columnNbr < (int)m_columns.size() && !m_col_width.IsEmpty())
    {
        const int default_width = std::get<1>(m_columns[columnNbr]);
        const wxString parameter_name = wxString::Format(m_col_width, columnNbr);
        int cur_width = Model_Setting::instance().GetIntSetting(parameter_name, default_width);
        int new_width = (cur_width != 0 ? 0 : default_width);
        SetColumnWidth(columnNbr, new_width);
        Model_Setting::instance().Set(parameter_name, GetColumnWidth(columnNbr));
    }
}

mmPanelBase::mmPanelBase()
{
}

mmPanelBase::~mmPanelBase()
{
}

wxString mmPanelBase::BuildPage() const
{
    return "TBD";
}

void mmPanelBase::PrintPage()
{
    wxWebView * htmlWindow = wxWebView::New(this, wxID_ANY);
    htmlWindow->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
    htmlWindow->SetPage(BuildPage(), "");
    htmlWindow->Print();
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
