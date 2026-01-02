/*******************************************************
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
 ********************************************************/

#include "navigatoreditdialog.h"
#include "constants.h"
#include "images_list.h"
#include "option.h"
#include <wx/wx.h>
#include <wx/filedlg.h>
#include <wx/bitmap.h>
#include <wx/iconbndl.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmNavigatorEditDialog, wxDialog);
wxBEGIN_EVENT_TABLE(mmNavigatorEditDialog, wxDialog)

wxEND_EVENT_TABLE()

mmNavigatorEditDialog::mmNavigatorEditDialog()
{
}

mmNavigatorEditDialog::mmNavigatorEditDialog(wxWindow* parent, NavigatorTypesInfo* info)
{
    this->SetFont(parent->GetFont());

    Create(parent, -1, _t("Edit Navigator entry"), wxDefaultPosition, wxSize(-1, -1), wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX, "");
    CreateControls();
    SetIcon(mmex::getProgramIcon());

    if (info) {
        m_nameTextCtrl->SetValue(info->name);
        if (info->navTyp == NavigatorTypes::NAV_TYP_PANEL) {
            m_activeCheckBox->SetValue(info->active);
        }
        else {
            m_aktivLabel->Show(false);
            m_activeCheckBox->Show(false);
        }
        m_cbIcon->SetSelection(info->imageId);
    }
    else {
        m_cbIcon->SetSelection(img::ACCOUNT_CLOSED_PNG);
        m_saveButton->Enable(false);
        m_aktivLabel->Show(false);
        m_activeCheckBox->Show(false);
    }
    Centre();
    SetMinSize(wxSize(300, 200));
    Fit();
}

void mmNavigatorEditDialog::CreateControls()
{
    wxPanel* panel = new wxPanel(this);
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    wxStaticBox* uiBox = new wxStaticBox(panel, wxID_STATIC, wxString(""));
    wxStaticBoxSizer* uiSizer = new wxStaticBoxSizer(uiBox, wxVERTICAL);
    vbox->Add(uiSizer, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxFlexGridSizer* uiStyleSizer = new wxFlexGridSizer(2, 0, 5);
    uiStyleSizer->AddGrowableCol(1, 0);
    uiSizer->Add(uiStyleSizer);

    wxStaticText* nameLabel = new wxStaticText(uiBox, wxID_ANY, "Name:");
    m_nameTextCtrl = new wxTextCtrl(uiBox, wxID_ANY, "", wxDefaultPosition, wxSize(200, -1));
    m_nameTextCtrl->Bind(wxEVT_TEXT, &mmNavigatorEditDialog::OnNewText, this);

    uiStyleSizer->Add(nameLabel, g_flagsH);
    uiStyleSizer->Add(m_nameTextCtrl, g_flagsExpand);

    m_aktivLabel = new wxStaticText(uiBox, wxID_ANY, "Aktiv:");
    m_activeCheckBox = new wxCheckBox(uiBox, wxID_ANY, "");

    wxStaticText* iconLabel = new wxStaticText(uiBox, wxID_ANY, "Icon:");

    wxVector<wxBitmapBundle> images = navtree_images_list();
    const auto navIconSize = Option::instance().getNavigationIconSize();
    wxImageList* imageList = new wxImageList(navIconSize, navIconSize);
    for (const auto& bundle : navtree_images_list(navIconSize)) {
        wxBitmap bitmap = bundle.GetBitmap(wxSize(navIconSize, navIconSize));
        imageList->Add(bitmap);
    }
    m_cbIcon = new wxBitmapComboBox(uiBox, wxID_ANY, "",
            wxPoint(navIconSize, navIconSize), wxDefaultSize,
            0, nullptr, wxCB_READONLY | wxCB_DROPDOWN);
    m_cbIcon->SetMinSize(wxSize(3 * navIconSize, -1));

    const int imageCount = imageList->GetImageCount();
    for (int i = 0; i < imageCount; ++i) {
        m_cbIcon->Append("", imageList->GetBitmap(i));
    }

    uiStyleSizer->Add(iconLabel, g_flagsH);
    uiStyleSizer->Add(m_cbIcon, g_flagsH);

    m_saveButton = new wxButton(panel, wxID_OK, "Speichern");
    wxButton* cancelButton = new wxButton(panel, wxID_CANCEL, "Abbrechen");

    wxBoxSizer* cSizer = new wxBoxSizer(wxHORIZONTAL);
    cSizer->Add(m_saveButton, g_flagsExpand);
    cSizer->Add(cancelButton, g_flagsExpand);

    uiStyleSizer->Add(m_aktivLabel, g_flagsH);
    uiStyleSizer->Add(m_activeCheckBox, 0, wxALL, 5);
    vbox->Add(cSizer, 0, wxALL | wxALIGN_CENTER, 5);

    panel->SetSizer(vbox);
    vbox->Fit(this);
    this->Centre();
}

void mmNavigatorEditDialog::updateInfo(NavigatorTypesInfo* info)
{
    info->name = m_nameTextCtrl->GetValue();
    info->imageId = m_cbIcon->GetSelection();
    if (info->navTyp == NavigatorTypes::NAV_TYP_PANEL) {
        info->active = m_activeCheckBox->GetValue();
    }
}

void mmNavigatorEditDialog::OnNewText(wxCommandEvent& WXUNUSED(event))
{
    m_saveButton->Enable(m_nameTextCtrl->GetValue().length() > 2);
}
