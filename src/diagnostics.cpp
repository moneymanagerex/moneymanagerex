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

#include "constants.h"
#include "images_list.h"
#include "option.h"
#include "paths.h"
#include "diagnostics.h"
#include "util.h"
#include "model/Model_Setting.h"
#include <wx/display.h>


wxIMPLEMENT_DYNAMIC_CLASS(mmDiagnosticsDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmDiagnosticsDialog, wxDialog)
    EVT_BUTTON(wxID_OK, mmDiagnosticsDialog::OnOk)
wxEND_EVENT_TABLE()

const char HTMLPANEL[] = R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8" />
    <meta http - equiv="Content-Type" content="text/html" />
</head>
<body>
%s
</body>
</html>)";

mmDiagnosticsDialog::mmDiagnosticsDialog(wxWindow *parent, const wxString &name)
{
    Create(parent, name);
}

void mmDiagnosticsDialog::Create(wxWindow* parent, const wxString &name)
{
    long style = wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER;
    if (!wxDialog::Create(parent, wxID_ANY, _("Diagnostics Report")
        , wxDefaultPosition, wxDefaultSize, style, name))
    {
        return;
    }

    CreateControls();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    RefreshView();

    Centre();
}

void mmDiagnosticsDialog::CreateControls()
{
    wxBoxSizer* bSizer0 = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* bSizer01 = new wxBoxSizer(wxHORIZONTAL);
    m_diagPanel = new wxHtmlWindow(this, wxID_ANY);
    m_diagPanel->SetMinSize(wxSize(400, 300));
    bSizer01->Add(m_diagPanel, g_flagsExpand);
	bSizer0->Add(bSizer01, g_flagsExpand);

    wxBoxSizer* bSizer02 = new wxBoxSizer(wxHORIZONTAL);
    m_okButton = new wxButton(this, wxID_OK, _("Close"));
    bSizer02->Add(m_okButton, 0, wxALL, 5);
    bSizer0->Add(bSizer02, g_flagsCenter);

    this->SetSizer(bSizer0);
    this->Layout();
    bSizer0->Fit(this);
}

void mmDiagnosticsDialog::RefreshView()
{
    wxString html = _("<b>Developers may ask you to provide information presented here in order to help diagnose issues you may report with the program.</b><p>");
    
    html << "#3222: Screen geometry<p>";

    // Saved dimensions
    int valX = Model_Setting::instance().GetIntSetting("ORIGINX", -1);
    int valY = Model_Setting::instance().GetIntSetting("ORIGINY", -1);
    int valW = Model_Setting::instance().GetIntSetting("SIZEW", -1);
    int valH = Model_Setting::instance().GetIntSetting("SIZEH", -1);
    html += wxString::Format("saved dimensions : x:%i, y:%i, w:%i, h:%i<br>"
                , valX, valY, valW, valH);

    // Check if it fits into any of the windows
    bool itFits = false;
    for (unsigned int i = 0; i < wxDisplay::GetCount(); i++) 
    {
        wxDisplay* display = new wxDisplay(i);
        wxRect rect = display->GetClientArea();
        html += wxString::Format("screen %i: x:%i, y:%i, w:%i, h:%i, fit:%s, primary:%s<br>"
                    , i, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight()
                    , (display->GetGeometry().Contains(wxRect(valX, valY, valW, valH))) ? "true" : "false"
                    , (display->IsPrimary()) ? "true" : "false" );
    }

    wxString displayHtml = wxString::Format(HTMLPANEL, html);
    m_diagPanel->SetPage(displayHtml);
}

void mmDiagnosticsDialog::OnOk(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}
