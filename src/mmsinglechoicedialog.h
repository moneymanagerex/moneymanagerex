/*******************************************************
Copyright (C) 2014 Nikolay

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
#include <wx/string.h>
#include <wx/choicdlg.h>
#include <wx/button.h>
#include "model/Model_Account.h"

class mmSingleChoiceDialog : public wxSingleChoiceDialog
{
public:
    using wxSingleChoiceDialog::ShowModal;

    mmSingleChoiceDialog(){}
    mmSingleChoiceDialog(wxWindow *parent
        , const wxString& message
        , const wxString& caption
        , const wxArrayString& choices)
    {
        wxSingleChoiceDialog::Create(parent, message, caption, choices);
        fix_translation();
    }
    mmSingleChoiceDialog(wxWindow* parent
        , const wxString& message
        , const wxString& caption
        , const Model_Account::Data_Set& accounts)
    {
        wxArrayString choices;
        for (const auto & item: accounts) choices.Add(item.ACCOUNTNAME);
        wxSingleChoiceDialog::Create(parent, message, caption, choices);
        fix_translation();
    }
    int ShowModal()
    {
        return wxSingleChoiceDialog::ShowModal();
    }
private:
    void fix_translation()
    {
        wxButton* ok = (wxButton*) FindWindow(wxID_OK);
        if (ok) ok->SetLabel(_("&OK "));
        wxButton* ca = (wxButton*) FindWindow(wxID_CANCEL);
        if (ca) ca->SetLabel(_("&Cancel "));
    }
};

class mmDialogComboBoxAutocomplete : public wxDialog
{
public:
    mmDialogComboBoxAutocomplete(){}
    mmDialogComboBoxAutocomplete(wxWindow *parent, const wxString& message, const wxString& caption, const wxString default, const wxArrayString& choices)
    {
        long style = wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX;
        Default = default;
        Choices = choices;
        Message = message;
        Create(parent, wxID_STATIC, caption, wxDefaultPosition, wxSize(300, 100), style);
    }

    wxString getText()
    {
        return cbText_->GetValue();
    };

private:
    bool Create(wxWindow* parent, wxWindowID id
        , const wxString& caption, const wxPoint& pos
        , const wxSize& size, long style)
    {
        wxDialog::Create(parent, id, caption, pos, size, style);
        const wxSizerFlags flags = wxSizerFlags().Align(wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL).Border(wxLEFT | wxRIGHT, 15);
        
        wxBoxSizer* Sizer = new wxBoxSizer(wxVERTICAL);
        this->SetSizer(Sizer);

        Sizer->AddSpacer(10);
        wxStaticText* headerText = new wxStaticText(this, wxID_STATIC, Message);
        Sizer->Add(headerText, flags);
        Sizer->AddSpacer(15);
        cbText_ = new wxComboBox(this, wxID_STATIC,Default,wxDefaultPosition,wxSize(150,-1),Choices);
        cbText_->AutoComplete(Choices);
        Sizer->Add(cbText_, wxSizerFlags(flags).Expand());
        Sizer->AddSpacer(20);
        wxSizer* Button = CreateButtonSizer(wxOK | wxCANCEL);
        Sizer->Add(Button, flags);
        Sizer->AddSpacer(10);

        GetSizer()->Fit(this);
        GetSizer()->SetSizeHints(this);
        Centre();
        return true;
    }

    wxString Message, Default;
    wxArrayString Choices;
    wxComboBox* cbText_;
};

