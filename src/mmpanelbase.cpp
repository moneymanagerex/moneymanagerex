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
#include "reports/htmlbuilder.h"
#include "model/Model_Setting.h"
#include "mmreportspanel.h"

const char *panel_template = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8" />
    <meta http - equiv = "Content-Type" content = "text/html" />
    <title><TMPL_VAR REPORTNAME></title>
    <script src = "sorttable.js"></script>
    <link href = "master.css" rel = "stylesheet" />
    <style>
        body {font-size: <TMPL_VAR HTMLSCALE>%;};
    </style>
</head>
<body>

<div class = "container">
<h3><TMPL_VAR REPORTNAME></h3>
<TMPL_VAR TODAY><hr>

<div class = "row">
<div class = "col-xs-1"></div>
<div class = "col-xs-10">

<table class = "table">
    <thead>
        <tr><TMPL_LOOP NAME=HEADERS><th><TMPL_VAR "HEADER"></th></TMPL_LOOP></tr>
    </thead>
    <tbody>
        <TMPL_LOOP NAME=CONTENTS><tr><TMPL_VAR "ROW"><tr></TMPL_LOOP>
    </tbody>
</table>
</div></div></div></div></body>
</html>
)";


wxBEGIN_EVENT_TABLE(mmListCtrl, wxListCtrl)
EVT_LIST_COL_END_DRAG(wxID_ANY, mmListCtrl::OnItemResize)
EVT_LIST_COL_CLICK(wxID_ANY, mmListCtrl::OnColClick)
EVT_LIST_COL_RIGHT_CLICK(wxID_ANY, mmListCtrl::OnColRightClick)
EVT_MENU(wxID_ANY, mmListCtrl::PopupSelected)
wxEND_EVENT_TABLE()

mmListCtrl::mmListCtrl(wxWindow *parent, wxWindowID winid)
    : wxListCtrl(parent, winid, wxDefaultPosition, wxDefaultSize
        , wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_VIRTUAL | wxLC_SINGLE_SEL | wxLC_AUTOARRANGE)
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
    /*
      Save the column widths of the list control. This will ensure that the
      column widths get set incase the OnItemResize does not work on some systems.
    */
    for (int column_number = 0; column_number < GetColumnCount(); ++column_number)
    {
        int column_width = GetColumnWidth(column_number);
        if (GetColumnWidthSetting(column_number) != column_width)
        {
            SetColumnWidthSetting(column_number, column_width);
        }
    }
}

wxListItemAttr* mmListCtrl::OnGetItemAttr(long row) const
{
    return (row % 2) ? attr2_ : attr1_;
}

wxString mmListCtrl::BuildPage(const wxString &title) const
{

    loop_t headers;
    wxString h;
    for (int c = 0; c < GetColumnCount(); c++)
    {
        row_t r;
        wxListItem col;
        col.SetMask(wxLIST_MASK_TEXT);
        GetColumn(c, col);
        h = col.GetText();
        r(L"HEADER") = h;
        headers += r;
    }

    loop_t contents;
    for (int i = 0; i < GetItemCount(); i++)
    {
        wxString row;
        row_t r;
        for (int col = 0; col < GetColumnCount(); col++)
        {
            row += "<td>" + wxListCtrl::GetItemText(i, col) + "</td>";
        }
        r(L"ROW") = row;
        contents += r;
    }

    mm_html_template report(panel_template);
    report(L"REPORTNAME") = title;
    report(L"HEADERS") = headers;
    report(L"CONTENTS") = contents;
    report(L"HTMLSCALE") = wxString::Format("%d", mmIniOptions::instance().html_font_size_);

    wxString out = wxEmptyString;
    try
    {
        out = report.Process();
    }
    catch (const syntax_ex& e)
    {
        out = e.what();
    }
    catch (...)
    {
        // TODO
    }

    Model_Report::outputReportFile(out);
    return out;
}

void mmListCtrl::OnItemResize(wxListEvent& event)
{
    int i = event.GetColumn();
    int width = GetColumnWidth(i);
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
        for (int i = 0; i < (int)m_columns.size(); i++)
        {
            const int id = MENU_HEADER_COLUMN + i;
            submenu->AppendCheckItem(id, m_columns[i].HEADER);
            if (Model_Setting::instance().GetIntSetting(wxString::Format(m_col_width, i), m_columns[i].WIDTH) != 0)
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
    for (int i = 0; i < (int)m_columns.size(); i++)
    {
        SetColumnWidth(i, m_columns[i].WIDTH);
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
        int default_width = m_columns[columnNbr].WIDTH;
        if (default_width == 0)
            default_width = wxLIST_AUTOSIZE_USEHEADER;
        const wxString parameter_name = wxString::Format(m_col_width, columnNbr);
        int cur_width = Model_Setting::instance().GetIntSetting(parameter_name, default_width);
        int new_width = (cur_width != 0 ? 0 : default_width);
        SetColumnWidth(columnNbr, new_width);
        Model_Setting::instance().Set(parameter_name, GetColumnWidth(columnNbr));
    }
}

int mmListCtrl::GetColumnWidthSetting(int column_number, int default_size)
{
    return Model_Setting::instance().GetIntSetting(wxString::Format(m_col_width, column_number), default_size);
}

void mmListCtrl::SetColumnWidthSetting(int column_number, int column_width)
{
    if (!m_col_width.IsEmpty())
        Model_Setting::instance().Set(wxString::Format(m_col_width, column_number), column_width);
}

mmPanelBase::mmPanelBase()
{
}

mmPanelBase::~mmPanelBase()
{
}

wxString mmPanelBase::BuildPage() const
{
    mmReportsPanel* rp = wxStaticCast(this, mmReportsPanel);
    return rp ? rp->getPrintableBase()->getHTMLText() : "TBD";
}

void mmPanelBase::PrintPage()
{
    //this->Freeze();
    wxWebView * htmlWindow = wxWebView::New(this, wxID_ANY);
    htmlWindow->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
    htmlWindow->SetPage(BuildPage(), "");
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
