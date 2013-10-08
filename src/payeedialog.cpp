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

#include "payeedialog.h"
#include "relocatepayeedialog.h"
#include "util.h"
#include "mmOption.h"
#include "paths.h"
#include "model/Model_Infotable.h"
#include "model/Model_Payee.h"

IMPLEMENT_DYNAMIC_CLASS( mmPayeeDialog, wxDialog )

BEGIN_EVENT_TABLE( mmPayeeDialog, wxDialog )
    EVT_BUTTON(wxID_ADD, mmPayeeDialog::OnAdd)
    EVT_BUTTON(wxID_REMOVE, mmPayeeDialog::OnDelete)
    EVT_BUTTON(wxID_CANCEL, mmPayeeDialog::OnCancel)
    EVT_BUTTON(wxID_EDIT, mmPayeeDialog::OnEdit)
    EVT_LISTBOX(wxID_ANY, mmPayeeDialog::OnSelChanged)
    EVT_LISTBOX_DCLICK(wxID_ANY, mmPayeeDialog::OnDoubleClicked)
END_EVENT_TABLE()


mmPayeeDialog::mmPayeeDialog(wxWindow *parent, bool showSelectButton) :
    m_payee_id_(-1),
    showSelectButton_(showSelectButton),
    refreshRequested_(false)
{
    do_create(parent);
}

void mmPayeeDialog::do_create(wxWindow* parent)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);

    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX | wxRESIZE_BORDER | wxWANTS_CHARS;
    if (!wxDialog::Create(parent, wxID_ANY, _("Organize Payees"), wxDefaultPosition, wxDefaultSize, style)) {
        return;
    }

    CreateControls();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    fillControls();

    Centre();
}

void mmPayeeDialog::CreateControls()
{
    int border = 5;
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, border);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, border).Expand();

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22);
    wxBitmapButton* payee_relocate = new wxBitmapButton(this,
        wxID_STATIC, wxBitmap(relocate_payees_xpm));
    itemBoxSizer22->Add(payee_relocate, flags);
    payee_relocate->Connect(wxID_STATIC, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(mmPayeeDialog::OnPayeeRelocate), NULL, this);
    payee_relocate->SetToolTip(_("Change all transactions using one Payee to another Payee"));
    itemBoxSizer22->AddSpacer(10);

    Model_Payee::Data_Set filtd = Model_Payee::instance().FilterPayees("");
    int vertical_size_ = (filtd.size()>10 ? 320 : 240);
    listBox_ = new wxListBox( this, wxID_ANY,
       wxDefaultPosition, wxSize(100, vertical_size_), wxArrayString(), wxLB_SINGLE);
    itemBoxSizer2->Add(listBox_, 1, wxEXPAND|wxALL, 1);

    wxNotebook* payee_notebook = new wxNotebook(this,
        wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE );
    itemBoxSizer2->Add(payee_notebook, flagsExpand);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, flagsExpand);

    wxPanel* maintenance_tab = new wxPanel(payee_notebook, wxID_ANY);
    payee_notebook->AddPage(maintenance_tab, _("Tools"));
    wxBoxSizer *maintenance_sizer = new wxBoxSizer(wxHORIZONTAL);
    maintenance_tab->SetSizer(maintenance_sizer);

    addButton_ = new wxButton( maintenance_tab, wxID_ADD);
    editButton_ = new wxButton( maintenance_tab, wxID_EDIT);
    editButton_->Disable();
    deleteButton_ = new wxButton( maintenance_tab, wxID_REMOVE);
    deleteButton_->Disable();

    maintenance_sizer->Add(addButton_, flags.Border(wxALL, 1));
    maintenance_sizer->Add(editButton_, flags);
    maintenance_sizer->Add(deleteButton_, flags);

    //Some interfaces has no any close buttons, it may confuse user. Cancel button added
    btnCancel_ = new wxButton( this, wxID_CANCEL, _("&Cancel "));
    itemBoxSizer9->Add(btnCancel_,  flags.Center());

    addButton_->SetToolTip(_("Add a new payee name"));
    editButton_->SetToolTip(_("Change the name of an existing payee"));
    deleteButton_->SetToolTip(_("Delete the selected payee. The payee cannot be used by an existing transaction."));
}

void mmPayeeDialog::fillControls()
{
    Model_Payee::Data_Set filtd = Model_Payee::instance().all(Model_Payee::COL_PAYEENAME);

    listBox_->Clear();

    for (const auto& payee: filtd)
    {
        wxStringClientData *data = new wxStringClientData((wxStringClientData)wxString::Format("%i", payee.PAYEEID));
        listBox_->Append(payee.PAYEENAME, data);
    }
}


void mmPayeeDialog::OnSelChanged(wxCommandEvent& event)
{
    wxStringClientData *client_object = (wxStringClientData *)listBox_->GetClientObject(event.GetSelection());
    if (client_object) m_payee_id_ = wxAtoi(client_object->GetData());
    else return;
    editButton_->Enable(true);
    deleteButton_->Enable(!Model_Payee::is_used(m_payee_id_));
}

void mmPayeeDialog::OnAdd(wxCommandEvent& event)
{
    wxString text = wxGetTextFromUser(_("Enter the name for the new payee:")
        , _("Organize Payees: Add Payee"), "");
    if (text.IsEmpty()) {
        return;
    }

    Model_Payee::Data* payee = Model_Payee::instance().get(text);
    if (payee)
    {
        wxMessageBox(_("Payee with same name exists"), _("Organize Payees: Add Payee"), wxOK|wxICON_ERROR);
    }
    else
    {
        payee = Model_Payee::instance().create();
        payee->PAYEENAME = text;
        int payeeID = Model_Payee::instance().save(payee);
        if (payeeID < 0) return;
        wxASSERT(payeeID > 0);

        fillControls();

        listBox_->SetStringSelection(text);

        // SetStringSelection does not emit event, so we need to do it manually.
        // This is important because it is where m_payee_id_ gets set
        OnSelChanged(event);
    }
}

void mmPayeeDialog::OnDelete(wxCommandEvent& event)
{
    if (!Model_Payee::instance().remove(m_payee_id_))
    {
        wxString deletePayeeErrMsg = _("Payee in use.");
        deletePayeeErrMsg
            << "\n\n"
            << _("Tip: Change all transactions using this Payee to another Payee\nusing the relocate command:")
            << "\n\n" << _("Tools -> Relocation of -> Payees");
        wxMessageBox(deletePayeeErrMsg,_("Organize Payees: Delete Error"), wxOK|wxICON_ERROR);
        return;
    }

    fillControls();
    if (!listBox_->IsEmpty()) {
        listBox_->Select(0);
    }
    OnSelChanged(event);
}

void mmPayeeDialog::OnDoubleClicked(wxCommandEvent& event)
{
        OnEdit(event);
}

void mmPayeeDialog::OnEdit(wxCommandEvent& event)
{
    wxString old_name = listBox_->GetStringSelection();

    wxString mesg;
    mesg.Printf(_("Enter a new name for %s"), old_name);
    wxString new_name = wxGetTextFromUser(mesg, _("Edit Payee Name"), old_name);
    if (new_name != wxGetEmptyString())
    {
        Model_Payee::Data* payee = Model_Payee::instance().get(m_payee_id_);
        if (payee)
        {
            payee->PAYEENAME = new_name;
            Model_Payee::instance().save(payee);
        }
        refreshRequested_ = true;
        editButton_->Disable();
        fillControls();

        listBox_->SetStringSelection(new_name);
        OnSelChanged(event);
    }
}

void mmPayeeDialog::OnPayeeRelocate(wxCommandEvent& event)
{
    relocatePayeeDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msgStr;
        msgStr << _("Payee Relocation Completed.") << "\n\n"
            << wxString::Format(_("Records have been updated in the database: %i")
                , dlg.updatedPayeesCount())
            << "\n\n";
        wxMessageBox(msgStr, _("Payee Relocation Result"));
        mmOptions::instance().databaseUpdated_ = true;
        refreshRequested_ = true;
        OnSelChanged(event);
    }
}

void mmPayeeDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}
