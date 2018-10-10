/*************************************************************************
 Copyright (C) 2006 Madhan Kanagavel
 copyright (C) 2011, 2012 Nikolay & Stefano Giorgio.

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

#include "optiondialog.h"
#include "constants.h"
#include "paths.h"

#include "../resources/main-setup.xpm"
#include "../resources/preferences-attachments.xpm"
#include "../resources/preferences-network.xpm"
#include "../resources/preferences-other.xpm"
#include "../resources/view.xpm"

#include "optionsettingsgeneral.h"
#include "optionsettingsview.h"
#include "optionsettingsattachment.h"
#include "optionsettingsnet.h"
#include "optionsettingsmisc.h"

wxIMPLEMENT_DYNAMIC_CLASS(mmOptionsDialog, wxDialog);

wxBEGIN_EVENT_TABLE( mmOptionsDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmOptionsDialog::OnOk)
    EVT_BUTTON(wxID_APPLY, mmOptionsDialog::OnApply)
wxEND_EVENT_TABLE()

mmOptionsDialog::mmOptionsDialog( )
{
}

mmOptionsDialog::~mmOptionsDialog( )
{
    delete m_imageList;
}

mmOptionsDialog::mmOptionsDialog(wxWindow* parent, mmGUIApp* app): m_app(app)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("MMEX Options"), wxDefaultPosition, wxSize(500, 400), style);
}

bool mmOptionsDialog::Create(wxWindow* parent
    , wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmex::getProgramIcon());
    SetMinSize(wxSize(424, 610));

    Centre();
    return TRUE;
}

void mmOptionsDialog::CreateControls()
{
    wxSize imageSize(48, 48);
    m_imageList = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
    m_imageList->Add(wxBitmap(view_xpm));
    m_imageList->Add(wxBitmap(preferences_attachments_xpm));
    m_imageList->Add(wxBitmap(main_setup_xpm));
    m_imageList->Add(wxBitmap(preferences_other_xpm));
    m_imageList->Add(wxBitmap(preferences_network_xpm));

    wxBoxSizer* mainDialogSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainDialogSizer);

    wxPanel* mainDialogPanel = new wxPanel(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    mainDialogSizer->Add(mainDialogPanel, 1, wxGROW|wxALL, 5);

    wxBoxSizer* mainDialogPanelSizer = new wxBoxSizer(wxVERTICAL);
    mainDialogPanel->SetSizer(mainDialogPanelSizer);

    m_notebook = new wxListbook(mainDialogPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_LEFT);
    m_notebook->SetImageList(m_imageList);

    /*********************************************************************************************
     General Panel
    **********************************************************************************************/
    OptionSettingsGeneral* general_panel = new OptionSettingsGeneral(m_notebook, m_app);
    m_panel_list.push_back(general_panel);

    /*********************************************************************************************
     Views Panel
    **********************************************************************************************/
    OptionSettingsView* views_panel = new OptionSettingsView(m_notebook);
    m_panel_list.push_back(views_panel);

    /*********************************************************************************************
     Attachments Panel
    **********************************************************************************************/
    OptionSettingsAttachment* attachment_panel = new OptionSettingsAttachment(m_notebook);
    m_panel_list.push_back(attachment_panel);

    /*********************************************************************************************
    Network Panel
    **********************************************************************************************/
    OptionSettingsNet* network_panel = new OptionSettingsNet(m_notebook);
    m_panel_list.push_back(network_panel);

    /*********************************************************************************************
    Others Panel
    **********************************************************************************************/
    OptionSettingsMisc* others_panel = new OptionSettingsMisc(m_notebook);
    m_panel_list.push_back(others_panel);

    /**********************************************************************************************
    Add the panels to the notebook
    **********************************************************************************************/
    m_notebook->InsertPage(0, general_panel, _("General"), true, 2);
    m_notebook->InsertPage(1, views_panel, _("View Options"), false, 0);
    m_notebook->InsertPage(2, attachment_panel, _("Attachments"), false, 1);
    m_notebook->InsertPage(3, network_panel, _("Network"), false, 4);
    m_notebook->InsertPage(4, others_panel, _("Others"), false, 3);

    mainDialogPanelSizer->Add(m_notebook, g_flagsExpand);
    mainDialogPanelSizer->Layout();

   /**********************************************************************************************
    Button Panel with OK and Cancel Buttons
    **********************************************************************************************/
    wxStaticLine* panelSeparatorLine = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    mainDialogSizer->Add(panelSeparatorLine, 0, wxGROW | wxLEFT | wxRIGHT, 10);

    wxPanel* buttonPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* buttonPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonPanel->SetSizer(buttonPanelSizer);
    mainDialogSizer->Add(buttonPanel, wxSizerFlags(g_flagsV).Center());

    wxButton* itemButtonOK = new wxButton(buttonPanel, wxID_OK, _("&OK "));
    wxButton* itemButtonApply = new wxButton(buttonPanel, wxID_APPLY, _("&Apply"));
    wxButton* itemButtonCancel = new wxButton(buttonPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    buttonPanelSizer->Add(itemButtonOK, g_flagsH);
    buttonPanelSizer->Add(itemButtonApply, g_flagsH);
    buttonPanelSizer->Add(itemButtonCancel, g_flagsH);
    itemButtonOK->SetFocus();
}

/// Saves the updated System Options to the appropriate databases.
void mmOptionsDialog::SaveNewSystemSettings()
{
    Model_Infotable::instance().Savepoint();
    Model_Setting::instance().Savepoint();

    for (const auto notebook_panel : m_panel_list)
        notebook_panel->SaveSettings();

    Model_Setting::instance().ReleaseSavepoint();
    Model_Infotable::instance().ReleaseSavepoint();
}

void mmOptionsDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    this->SaveNewSystemSettings();
    EndModal(wxID_OK);
}

void mmOptionsDialog::OnApply(wxCommandEvent& WXUNUSED(event))
{
    Model_Infotable::instance().Savepoint();
    Model_Setting::instance().Savepoint();

    int selected_page = m_notebook->GetSelection();
    m_panel_list[selected_page]->SaveSettings();

    Model_Setting::instance().ReleaseSavepoint();
    Model_Infotable::instance().ReleaseSavepoint();

    const wxString& msg = wxString::Format(_("%s page has been saved."), m_notebook->GetPageText(selected_page));
    wxMessageBox(msg, _("MMEX Options"));
}
