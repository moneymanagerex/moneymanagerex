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
#include "util.h"
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/string.h>
#include "Model_Account.h"
#include "Model_Payee.h"
#include <wx/richtooltip.h>


class mmComboBox : public wxTextCtrl
{
public:
    using wxTextCtrl::Connect;

    mmComboBox() {}
    mmComboBox(wxWindow *parent, wxWindowID id
        , const wxString &value
        , bool payee
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = 0
        , const wxValidator &validator = wxDefaultValidator
        , const wxString &name = "mmComboBox")
        : wxTextCtrl(parent, id, value, pos, size, style, validator, name)
        , m_payee(payee)
    {
        if (m_payee)
            this->AutoComplete(Model_Payee::instance().all_payee_names());
        else
            this->AutoComplete(Model_Account::instance().all_checking_account_names());

    }
    wxString GetValue() const
    {
        return wxTextCtrl::GetValue();
    }

    void setSelection(int &id)
    {
        if (m_payee) {
            for (const auto payee : Model_Payee::instance().all(Model_Payee::COL_PAYEENAME))
                if (payee.PAYEEID == id) this->ChangeValue(payee.PAYEENAME);
        }
        else
        {
            for (const auto acc : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
                if (acc.ACCOUNTID == id) this->ChangeValue(acc.ACCOUNTNAME);
        }
    }

    int getID()
    {
        int id = -1;
        if (m_payee) {
            Model_Payee::Data * p = Model_Payee::instance().get(this->GetValue());
            if (p) id = p->PAYEEID;
            else {
                const wxString errorHeader = _("Invalid Payee");
                const wxString errorMessage = (_("Please type in an exiting payee name,\nor make a selection using the dropdown button.")
                    + "\n");
                wxRichToolTip tip(errorHeader, errorMessage);
                tip.SetIcon(wxICON_WARNING);
                tip.ShowFor((wxWindow*) this);
                this->SetFocus();
            }
        }
        else {
            Model_Account::Data* a = Model_Account::instance().get(this->GetValue());
            if (a) id = a->ACCOUNTID;
            else {
                //TODO:
            }
        }
        return id;
    }

private:
    bool m_payee;
};
