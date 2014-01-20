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
    mmSingleChoiceDialog(wxWindow *parent
        , Model_Account::Data_Set accounts)
    {
        wxArrayString choices;
        for (const auto& acc : accounts) choices.Add(acc.ACCOUNTNAME);
        wxSingleChoiceDialog::Create(parent, _("Account name"), _("Select Account"), choices);
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


