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

mmListCtrl::mmListCtrl(wxWindow *parent, wxWindowID winid)
    : wxListCtrl(parent, winid, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_VIRTUAL | wxLC_SINGLE_SEL)
    , attr1_(new wxListItemAttr(mmColors::listBorderColor, mmColors::listAlternativeColor0, wxNullFont))
    , attr2_(new wxListItemAttr(mmColors::listBorderColor, mmColors::listAlternativeColor1, wxNullFont))
    , m_selected_row(-1)
    , m_selected_col(0)
    , m_asc(true)
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
