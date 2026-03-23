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

#pragma once

#include "_PanelBase.h"

class wxWebView;
class mmGUIFrame;

class HelpPanel : public PanelBase
{
    wxDECLARE_EVENT_TABLE();

private:
    mmGUIFrame* w_frame;
    wxWebView*  w_browser;

public:
    HelpPanel(
        wxWindow* parent_win,
        mmGUIFrame* frame,
        wxWindowID win_id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "HelpPanel"
    );

    // override PanelBase
    virtual auto buildPage() const -> wxString override { return ""; }
    virtual void printPage() override { w_browser->Print(); }
    virtual void sortList() override {}

private:
    bool create(
        wxWindow* parent_win,
        wxWindowID win_id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "HelpPanel"
    );
    void createControls();

    void onHelpPageBack(wxCommandEvent& event);
    void onHelpPageForward(wxCommandEvent& event);
    void onNewWindow(wxWebViewEvent& evt);
};

