/*************************************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011, 2012 Nikolay & Stefano Giorgio.
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 Klaus Wich

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
#include "option.h"
#include "constants.h"
#include "paths.h"
#include "images_list.h"
#include "util.h"
#include <wx/bookctrl.h>

#include "optionsettingsgeneral.h"
#include "optionsettingsview.h"
#include "optionsettingshome.h"
#include "optionsettingsattachment.h"
#include "optionsettingsnet.h"
#include "optionsettingsmisc.h"
#include "optionsettingstransaction.h"


static const wxArrayString s_pagetitle = {
    _n("General"), _n("View"), _n("Dashboard"), _n("Transactions"), _n("Attachments"), _n("Network"), _n("Other")
};

wxIMPLEMENT_DYNAMIC_CLASS(mmOptionsDialog, wxDialog)

wxBEGIN_EVENT_TABLE( mmOptionsDialog, wxDialog )
EVT_BUTTON(wxID_OK, mmOptionsDialog::OnOk)
EVT_BUTTON(wxID_APPLY, mmOptionsDialog::OnApply)
EVT_LISTBOOK_PAGE_CHANGED(wxID_ANY, mmOptionsDialog::OnPageChange)
wxEND_EVENT_TABLE()

mmOptionsDialog::mmOptionsDialog( )
{
}

mmOptionsDialog::~mmOptionsDialog( )
{
}

mmOptionsDialog::mmOptionsDialog(wxWindow* parent, mmGUIApp* app): m_app(app)
{
    this->SetFont(parent->GetFont());
    Create(parent);
    SetMinSize(wxSize(500, 400));
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
    return true;
}

void mmOptionsDialog::CreateControls()
{
    const int iconSize = 32;
    wxVector<wxBitmapBundle> images;
    images.push_back(mmBitmapBundle(png::GENERAL, iconSize));
    images.push_back(mmBitmapBundle(png::VIEW, iconSize));
    images.push_back(mmBitmapBundle(png::HOME, iconSize));
    images.push_back(mmBitmapBundle(png::NEW_TRX, iconSize));
    images.push_back(mmBitmapBundle(png::ATTACHMENTS, iconSize));
    images.push_back(mmBitmapBundle(png::NETWORK, iconSize));
    images.push_back(mmBitmapBundle(png::OTHERS, iconSize));

    wxBoxSizer* mainDialogSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainDialogSizer);

    wxPanel* mainDialogPanel = new wxPanel(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    mainDialogSizer->Add(mainDialogPanel, 1, wxGROW|wxALL, 5);

    wxBoxSizer* mainDialogPanelSizer = new wxBoxSizer(wxVERTICAL);
    mainDialogPanel->SetSizer(mainDialogPanelSizer);

    m_listbook = new wxListbook(mainDialogPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_LEFT);
    m_listbook->SetImages(images);

    /*********************************************************************************************
     General Panel
    **********************************************************************************************/
    m_panel_list.push_back(new OptionSettingsGeneral(m_listbook, m_app));

    /*********************************************************************************************
     Views Panel
    **********************************************************************************************/
    m_panel_list.push_back(new OptionSettingsView(m_listbook));

    /*********************************************************************************************
     Home Panel
    **********************************************************************************************/
    m_panel_list.push_back(new OptionSettingsHome(m_listbook));

    /*********************************************************************************************
     Transaction Panel
    **********************************************************************************************/
    m_panel_list.push_back(new OptionSettingsTransaction(m_listbook));

    /*********************************************************************************************
     Attachments Panel
    **********************************************************************************************/
    m_panel_list.push_back(new OptionSettingsAttachment(m_listbook));

    /*********************************************************************************************
    Network Panel
    **********************************************************************************************/
    m_panel_list.push_back(new OptionSettingsNet(m_listbook));

    /*********************************************************************************************
    Others Panel
    **********************************************************************************************/
    m_panel_list.push_back(new OptionSettingsMisc(m_listbook));

    /**********************************************************************************************
    Add the panels to the notebook
    **********************************************************************************************/
    for (unsigned int i = 0; i < m_panel_list.size(); i++) {
        m_listbook->InsertPage(i, m_panel_list.at(i), wxGetTranslation(s_pagetitle[i]), i ? false : true, i);
    }

    mainDialogPanelSizer->Add(m_listbook, g_flagsExpand);
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

    wxButton* itemButtonOK = new wxButton(buttonPanel, wxID_OK, _t("&OK "));
    buttonPanelSizer->Add(itemButtonOK, g_flagsH);
    itemButtonOK->SetFocus();
    buttonPanelSizer->Add(new wxButton(buttonPanel, wxID_APPLY, _t("&Apply ")), g_flagsH);
    buttonPanelSizer->Add(new wxButton(buttonPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel)), g_flagsH);
}

/// Saves the updated System Options to the appropriate databases.
bool mmOptionsDialog::SaveNewSystemSettings()
{
    bool bResult = true;

    Model_Infotable::instance().Savepoint();
    Model_Setting::instance().Savepoint();

    for (const auto notebook_panel : m_panel_list)
    {
        if (!notebook_panel->SaveSettings())
            bResult = false;
    }

    Model_Setting::instance().ReleaseSavepoint();
    Model_Infotable::instance().ReleaseSavepoint();

    return bResult;
}

void mmOptionsDialog::OnOk(wxCommandEvent& /*event*/)
{
    if(this->SaveNewSystemSettings())
        EndModal(wxID_OK);
}

void mmOptionsDialog::OnPageChange(wxBookCtrlEvent& event)
{
    int old_page = event.GetOldSelection();
    if (old_page == wxNOT_FOUND)
        return;

    if (old_page == 0 && !m_panel_list[old_page]->SaveSettings())
    {
        SetEvtHandlerEnabled(false);
        m_listbook->GetEventHandler()->Disconnect();
        m_listbook->SetSelection(old_page);
        SetEvtHandlerEnabled(true);
        event.Veto();
    }
}

void mmOptionsDialog::OnApply(wxCommandEvent& /*event*/)
{
    Model_Infotable::instance().Savepoint();
    Model_Setting::instance().Savepoint();

    int selected_page = m_listbook->GetSelection();
    if (m_panel_list[selected_page]->SaveSettings())
    {
        const wxString& msg = wxString::Format(_t("%s page has been saved."), wxGetTranslation(s_pagetitle[selected_page]));
        wxMessageBox(msg, _t("Settings"));
    }

    Model_Setting::instance().ReleaseSavepoint();
    Model_Infotable::instance().ReleaseSavepoint();
}
