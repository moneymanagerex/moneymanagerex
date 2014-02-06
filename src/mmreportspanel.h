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

#ifndef _MM_EX_REPORTSPANEL_H_
#define _MM_EX_REPORTSPANEL_H_

#include "mmpanelbase.h"
#include "reports/reportbase.h"

class wxWebView;

class mmReportsPanel : public mmPanelBase
{
    DECLARE_EVENT_TABLE()

public:
    mmReportsPanel(mmPrintableBase* rb,
                    bool cleanupReport,
                    wxWindow *parent,
                    wxWindowID winid = wxID_ANY,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                    const wxString& name = wxPanelNameStr );
    ~mmReportsPanel();

    bool Create( wxWindow *parent, wxWindowID winid,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                 const wxString& name = wxPanelNameStr);

    void CreateControls();
    void sortTable();

    wxString getReportText();

protected:
    wxWebView * htmlWindow_;
    mmPrintableBase* rb_;

    friend class WebViewHandlerStatic;

private:
    bool cleanup_;
};

#endif

