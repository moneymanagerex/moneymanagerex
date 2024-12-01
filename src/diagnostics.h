/*******************************************************
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

#ifndef MM_EX_DIAGNOSTICS_H_
#define MM_EX_DIAGNOSTICS_H_

#include "defs.h"
#include <vector>

class mmDiagnosticsDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmDiagnosticsDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmDiagnosticsDialog(wxWindow* parent, bool is_maximized);
    ~mmDiagnosticsDialog() {};
private:
    bool createWindow(wxWindow* parent
        , const wxString& caption
    );
    mmDiagnosticsDialog() {};
    wxSharedPtr<wxHtmlWindow> m_diagPanel;
    wxWindow* m_parent = nullptr;
    wxButton* m_okButton = nullptr;
    bool m_is_max = false;

private:

    void CreateControls();
    void RefreshView();
 
    void OnOk(wxCommandEvent&);
};

#endif // MM_EX_DIAGNOSTICS_H_
