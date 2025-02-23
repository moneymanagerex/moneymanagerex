/*************************************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011, 2012 Nikolay Akimov
 Copyright (C) 2011, 2012 Stefano Giorgio

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
 *************************************************************************/

#ifndef MM_EX_HELPPANEL_H_
#define MM_EX_HELPPANEL_H_

#include "mmpanelbase.h"

class wxWebView;
class mmGUIFrame;

class mmHelpPanel : public mmPanelBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmHelpPanel(wxWindow *parent, mmGUIFrame* frame,
                 wxWindowID winid = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                 const wxString& name = "mmHelpPanel");

    wxString BuildPage() const { return ""; }
    void PrintPage();

public:
    mmGUIFrame *m_frame;
private:
    wxWebView* browser_;

    bool Create(wxWindow *parent, wxWindowID winid
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL | wxNO_BORDER
        , const wxString& name = "mmHelpPanel");

    void CreateControls();
    virtual void sortList();

    void OnHelpPageBack(wxCommandEvent& event);
    void OnHelpPageForward(wxCommandEvent& event);
    void OnNewWindow(wxWebViewEvent& evt);
};

#endif

