/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#include "base/constants.h"
#include "model/AccountModel.h"
#include "_PanelBase.h"

class mmGUIFrame;
class mmHTMLBuilder;
class mmDateRange;

class DashboardPanel : public PanelBase
{
    wxDECLARE_EVENT_TABLE();

private:
    wxString m_templateText;
    std::map<wxString, wxString> m_htmlText_mLabel;

    mmGUIFrame* w_frame   = nullptr;
    wxWebView*  w_browser = nullptr;

public:
    DashboardPanel(
        wxWindow* parent_win,
        mmGUIFrame* frame,
        wxWindowID win_id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "DashboardPanel"
    );
    ~DashboardPanel();

    // override PanelBase
    virtual auto buildPage() const -> wxString override { return getHomePageText(); }
    virtual void printPage() override { w_browser->Print(); }
    virtual void sortList() override {}

    void createHtml();

private:
    bool create(
        wxWindow* parent_win,
        wxWindowID win_id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "DashboardPanel"
    );
    void createControls();

    wxString getHomePageText() const;
    void insertDataIntoTemplate();
    void fillData();
    const wxString getToggles();

    void onNewWindow(wxWebViewEvent& event);
    void onLinkClicked(wxWebViewEvent& event);
};

