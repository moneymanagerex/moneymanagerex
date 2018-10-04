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

#include "appstartdialog.h"
#include "defs.h"
#include "paths.h"
#include "constants.h"
#include "option.h"
#include "Model_Setting.h"

#include "../resources/money.xpm"

/*******************************************************/

wxIMPLEMENT_DYNAMIC_CLASS(mmAppStartDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmAppStartDialog, wxDialog)
    EVT_BUTTON(wxID_NEW, mmAppStartDialog::OnButtonAppstartNewDatabaseClick)
    EVT_BUTTON(wxID_OPEN, mmAppStartDialog::OnButtonAppstartOpenDatabaseClick)
    EVT_BUTTON(wxID_HELP, mmAppStartDialog::OnButtonAppstartHelpClick)
    EVT_BUTTON(wxID_INDEX, mmAppStartDialog::OnButtonAppstartWebsiteClick)
    EVT_BUTTON(wxID_FILE1, mmAppStartDialog::OnButtonAppstartLastDatabaseClick)
    EVT_BUTTON(wxID_EXIT, mmAppStartDialog::OnQuit)
    EVT_CLOSE(mmAppStartDialog::OnClose)
wxEND_EVENT_TABLE()

mmAppStartDialog::mmAppStartDialog(wxWindow* parent, const wxString& name)
    : itemCheckBox(nullptr)
    , m_buttonClose(nullptr)
    , m_buttonExit(nullptr)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, mmex::getCaption(mmex::getTitleProgramVersion()),
        wxDefaultPosition, wxSize(400, 300), style, name);
}

bool mmAppStartDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    bool ok = wxDialog::Create(parent, id, caption, pos, size, style, name);

    if (ok) {
        SetIcon(mmex::getProgramIcon());
        CreateControls();
        GetSizer()->Fit(this);
        GetSizer()->SetSizeHints(this);
        this->SetInitialSize();
        CentreOnScreen();
    }

    return ok;
}

mmAppStartDialog::~mmAppStartDialog()
{
    try
    {
        bool showBeginApp = itemCheckBox->GetValue();
        Model_Setting::instance().Set("SHOWBEGINAPP", showBeginApp);
    }
    catch (...)
    {
        wxASSERT(false);
    }
}

void mmAppStartDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    wxBitmap itemStaticBitmap4Bitmap(money_xpm);
    wxStaticBitmap* itemStaticBitmap4 = new wxStaticBitmap(this, wxID_STATIC, wxBitmap(money_xpm));

    itemBoxSizer3->Add(itemStaticBitmap4, 0, wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    wxButton* itemButton61 = new wxButton(this, wxID_FILE1, _("Open Last Opened Database"));
    itemBoxSizer5->Add(itemButton61, 0, wxGROW | wxALL, 5);

    wxButton* itemButton6 = new wxButton(this, wxID_NEW, _("Create a New Database"));
    itemButton6->SetToolTip(_("Create a new database file to get started"));
    itemBoxSizer5->Add(itemButton6, 0, wxGROW | wxALL, 5);

    wxButton* itemButton7 = new wxButton(this, wxID_OPEN, _("Open Existing Database"));
    itemButton7->SetToolTip(_("Open an already created database file with extension (*.mmb)"));
    itemBoxSizer5->Add(itemButton7, 0, wxGROW | wxALL, 5);

    wxButton* itemButton8 = new wxButton(this, wxID_HELP, _("Read Documentation"));
    itemButton8->SetToolTip(_("Read the user manual"));
    itemBoxSizer5->Add(itemButton8, 0, wxGROW | wxALL, 5);

    wxButton* itemButton9 = new wxButton(this, wxID_INDEX, _("Visit Website for more information"));
    const wxString s = wxString::Format(_("Open the %s website for latest news, updates etc")
        , mmex::getProgramName());
    itemButton9->SetToolTip(s);
    itemBoxSizer5->Add(itemButton9, 0, wxGROW | wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer10, 0, wxALIGN_LEFT | wxALL, 5);

    const wxString showAppStartString = wxString::Format(_("Show this window next time %s starts")
        , mmex::getProgramName());

    itemCheckBox = new wxCheckBox(this, wxID_STATIC, showAppStartString, wxDefaultPosition,
        wxDefaultSize, wxCHK_2STATE);
    bool showBeginApp = Model_Setting::instance().GetBoolSetting("SHOWBEGINAPP", true);
    itemCheckBox->SetValue(showBeginApp);

    itemBoxSizer10->Add(itemCheckBox, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxStaticLine* line = new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    itemBoxSizer2->Add(line, 0, wxGROW | wxALL, 5);

    m_buttonClose = new wxButton(this, wxID_OK, _("&OK "));
    m_buttonExit = new wxButton(this, wxID_EXIT, _("&Exit "));

    m_buttonClose->Show(true);
    m_buttonExit->Show(false);

    itemBoxSizer2->Add(m_buttonClose, 0, wxALIGN_RIGHT | wxALL, 10);
    itemBoxSizer2->Add(m_buttonExit, 0, wxALIGN_RIGHT | wxALL, 10);

    wxString val = Model_Setting::instance().getLastDbPath();
    wxFileName lastfile(val);
    if (!lastfile.FileExists())
    {
        itemButton61->Disable();
    }
    else
    {
        itemButton61->SetToolTip(wxString::Format(_("Open the previously opened database: %s"), val));
    }
}

void mmAppStartDialog::SetCloseButtonToExit()
{
    m_buttonClose->Show(false);
    m_buttonExit->Show(true);
}

void mmAppStartDialog::OnButtonAppstartHelpClick( wxCommandEvent& WXUNUSED(event) )
{
    int helpFileIndex_ = mmex::HTML_INDEX;
    wxFileName helpIndexFile(mmex::getPathDoc((mmex::EDocFile)helpFileIndex_));
    wxString url = "file://";

    if (Option::instance().LanguageISO6391() != "en")
        helpIndexFile.AppendDir(Option::instance().LanguageISO6391());

    if (helpIndexFile.FileExists()) // Load the help file for the given language
    {
        url << (helpIndexFile.GetPathWithSep() + helpIndexFile.GetFullName());
    }
    else // load the default help file
    {
        url << (mmex::getPathDoc((mmex::EDocFile)helpFileIndex_));
    }
    wxLaunchDefaultBrowser(url);
}

void mmAppStartDialog::OnButtonAppstartWebsiteClick( wxCommandEvent& WXUNUSED(event) )
{
    wxLaunchDefaultBrowser(mmex::weblink::WebSite);
}

void mmAppStartDialog::OnButtonAppstartLastDatabaseClick( wxCommandEvent& WXUNUSED(event) )
{
    EndModal(wxID_FILE1);
}

void mmAppStartDialog::OnButtonAppstartOpenDatabaseClick( wxCommandEvent& WXUNUSED(event) )
{
    EndModal(wxID_OPEN);
}

void mmAppStartDialog::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_EXIT);
}

void mmAppStartDialog::OnClose(wxCloseEvent& WXUNUSED(event))
{
    if (m_buttonExit->IsShown())
        EndModal(wxID_EXIT);
    else
        EndModal(wxID_OK);
}

void mmAppStartDialog::OnButtonAppstartNewDatabaseClick( wxCommandEvent& WXUNUSED(event) )
{
    EndModal(wxID_NEW);
}
