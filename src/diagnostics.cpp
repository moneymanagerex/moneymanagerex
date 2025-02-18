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
#include "reports/htmlbuilder.h"
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

mmDiagnosticsDialog::mmDiagnosticsDialog(wxWindow* parent, bool is_maximized)
    : m_parent(parent)
    , m_is_max(is_maximized)
{

    createWindow(parent, _t("Diagnostics"));
}

bool mmDiagnosticsDialog::createWindow(wxWindow* parent
    , const wxString& caption
)
{
    const wxString name = "mmDiagnosticsDialog";
    long style = wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX;
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    bool ok = wxDialog::Create(parent, wxID_ANY, caption, wxDefaultPosition, wxDefaultSize, style, name);

    if (ok)
    {
        SetIcon(mmex::getProgramIcon());
        CreateControls();
        RefreshView();

        SetMinSize(wxSize(500, 350));
        Fit();
        Centre();
    }

    return ok;
}

void mmDiagnosticsDialog::CreateControls()
{
    wxBoxSizer* bSizer0 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* bSizer01 = new wxBoxSizer(wxHORIZONTAL);
    m_diagPanel = new wxHtmlWindow(this, wxID_ANY);
    bSizer01->Add(m_diagPanel.get(), g_flagsExpand);
    bSizer0->Add(bSizer01, g_flagsExpand);

    wxBoxSizer* bSizer02 = new wxBoxSizer(wxHORIZONTAL);
    m_okButton = new wxButton(this, wxID_OK, _t("&Close"));
    bSizer02->Add(m_okButton, 0, wxALL, 5);
    bSizer0->Add(bSizer02, g_flagsCenter);

    this->SetSizer(bSizer0);
    this->Layout();
}

void mmDiagnosticsDialog::RefreshView()
{
    wxString html;

    html << "<p><b>";
    html << _t("Developers may ask you to provide information presented here in order to help diagnose issues you may report with the program.");
    html << "</b></p>";

    html << "<p>";
    html << "#3222: Screen geometry";
    html << "<br>";

    // Saved dimensions
    int valX = Model_Setting::instance().getInt("ORIGINX", -1);
    int valY = Model_Setting::instance().getInt("ORIGINY", -1);
    int valW = Model_Setting::instance().getInt("SIZEW", -1);
    int valH = Model_Setting::instance().getInt("SIZEH", -1);
    bool is_max = Model_Setting::instance().getBool("ISMAXIMIZED", "?");
    html << wxString::Format("saved dimensions : x:%i, y:%i, w:%i, h:%i, maximized:%s"
        , valX, valY, valW, valH
        , is_max ? "true" : "false");
    html << "</p>";

    // Check if it fits into any of the windows
    for (unsigned int i = 0; i < wxDisplay::GetCount(); i++)
    {
        wxSharedPtr<wxDisplay> display(new wxDisplay(i));
        wxRect rect = display->GetClientArea();
        html << "<p>";
        html << wxString::Format("screen %i: x:%i, y:%i, w:%i, h:%i, fit:%s, primary:%s<br>"
            , i, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight()
            , (display->GetGeometry().Contains(wxRect(valX, valY, valW, valH))) ? "true" : "false"
            , (display->IsPrimary()) ? "true" : "false");
        html << "</p>";
    }

    //
    int value_x = 0, value_y = 0;
    m_parent->GetPosition(&value_x, &value_y);
    int value_w = 0, value_h = 0;
    m_parent->GetSize(&value_w, &value_h);

    html << "<p>";
    html << wxString::Format("current dimensions : x:%i, y:%i, w:%i, h:%i, maximized:%s"
        , value_x, value_y, value_w, value_h
        , m_is_max ? "true" : "false");
    html << "</p>";

    mmHTMLBuilder hb;
    hb.init(true);
    const wxString displayHtml = wxString::Format(HTMLPANEL, html);
    hb.addText(displayHtml);
    hb.end(true);
    m_diagPanel->SetPage(hb.getHTMLText());
}

void mmDiagnosticsDialog::OnOk(wxCommandEvent&)
{
    EndModal(wxID_OK);
}
