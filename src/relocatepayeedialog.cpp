/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011 Stefano Giorgio

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

#include "relocatepayeedialog.h"
#include "model/Model_Payee.h"
#include "paths.h"
#include "wx/statline.h"

IMPLEMENT_DYNAMIC_CLASS( relocatePayeeDialog, wxDialog )

BEGIN_EVENT_TABLE( relocatePayeeDialog, wxDialog )
    EVT_BUTTON(wxID_OK, relocatePayeeDialog::OnOk)
END_EVENT_TABLE()

relocatePayeeDialog::relocatePayeeDialog( )
{
    changedRecords_  = 0;
}

relocatePayeeDialog::relocatePayeeDialog(wxWindow* parent, int source_payee_id)
{
    sourcePayeeID_  = source_payee_id;
    destPayeeID_    = -1;
    long style = wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX;

    Create(parent, wxID_STATIC, _("Relocate Payee Dialog"), wxDefaultPosition, wxSize(500, 300), style);
}

bool relocatePayeeDialog::Create( wxWindow* parent, wxWindowID id,
                           const wxString& caption, const wxPoint& pos,
                           const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    Centre();
    return TRUE;
}

void relocatePayeeDialog::CreateControls()
{
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Center();
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Expand().Center();
    wxSize btnSize = wxSize(180,-1);

    wxStaticText* headerText = new wxStaticText( this, wxID_STATIC,
        _("Relocate all source payees to the destination payee"));
    wxStaticLine* lineTop = new wxStaticLine(this,wxID_STATIC,
        wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);

    cbSourcePayee_ = new wxComboBox(this, wxID_ANY, ""
        , wxDefaultPosition, btnSize);
    for (const auto payee: Model_Payee::instance().all(Model_Payee::COL_PAYEENAME))
    {
        //wxStringClientData* data = (wxStringClientData*)payee.PAYEEID;
        cbSourcePayee_->Append(payee.PAYEENAME/*, data*/);
        if (payee.PAYEEID == sourcePayeeID_)
            cbSourcePayee_->SetStringSelection(payee.PAYEENAME);
    }

    cbSourcePayee_->AutoComplete(Model_Payee::instance().all_payee_names());
    cbSourcePayee_->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(relocatePayeeDialog::OnPayeeUpdated), NULL, this);

    cbDestPayee_ = new wxComboBox(this, wxID_NEW, ""
        , wxDefaultPosition, btnSize);
    for (const auto payee: Model_Payee::instance().all(Model_Payee::COL_PAYEENAME))
    {
        //wxStringClientData* data = (wxStringClientData*)payee.PAYEEID;
        cbDestPayee_->Append(payee.PAYEENAME/*, data*/);
    }
    cbDestPayee_->AutoComplete(Model_Payee::instance().all_payee_names());
    cbDestPayee_->Connect(wxID_NEW, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(relocatePayeeDialog::OnPayeeUpdated), NULL, this);

    wxStaticLine* lineBottom = new wxStaticLine(this,wxID_STATIC
        , wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, flags);
    wxFlexGridSizer* request_sizer = new wxFlexGridSizer(0, 2, 0, 0);

    boxSizer->Add(headerText, flags);
    boxSizer->Add(lineTop, flagsExpand);

    request_sizer->Add(new wxStaticText( this, wxID_STATIC,_("Relocate:")), flags);
    request_sizer->Add(new wxStaticText( this, wxID_STATIC,_("to:")), flags);
    request_sizer->Add(cbSourcePayee_, flags);
    request_sizer->Add(cbDestPayee_, flags);

    boxSizer->Add(request_sizer, flagsExpand);
    boxSizer->Add(lineBottom, flagsExpand);

    wxButton* okButton = new wxButton(this,wxID_OK, _("&OK "));
    wxButton* cancelButton = new wxButton(this,wxID_CANCEL, _("&Cancel "));
    cancelButton-> SetFocus () ;
    wxBoxSizer* buttonBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonBoxSizer->Add(okButton, flags);
    buttonBoxSizer->Add(cancelButton, flags);
    boxSizer->Add(buttonBoxSizer, flags);
}

int relocatePayeeDialog::updatedPayeesCount()
{
    return changedRecords_;
}

void relocatePayeeDialog::OnOk(wxCommandEvent& /*event*/)
{
    wxString destPayeeName, sourcePayeeName;
    destPayeeName = cbDestPayee_->GetValue();
    sourcePayeeName = cbSourcePayee_->GetValue();

    Model_Payee::Data* dest_payee = Model_Payee::instance().get(destPayeeName);
    Model_Payee::Data* source_payee = Model_Payee::instance().get(sourcePayeeName);

    if ( dest_payee &&  source_payee )
    {
        wxString msgStr = _("Please Confirm:") ;
        msgStr << "\n\n";
        msgStr << wxString::Format( _("Changing all payees of: %s \n\n  to payee: %s"),
             sourcePayeeName, destPayeeName);

        int ans = wxMessageBox(msgStr,_("Payee Relocation Confirmation"), wxOK|wxCANCEL|wxICON_QUESTION);
        if (ans == wxOK)
        {
            Model_Checking::Data_Set transactions = Model_Checking::instance().find(Model_Checking::COL_PAYEEID, source_payee->PAYEEID);
            for (auto &entry : transactions)
                entry.PAYEEID = dest_payee->PAYEEID;
            changedRecords_ += Model_Checking::instance().save(transactions);

            Model_Billsdeposits::Data_Set billsdeposits = Model_Billsdeposits::instance().find(Model_Billsdeposits::COL_PAYEEID, source_payee->PAYEEID);
            for (auto &entry : billsdeposits)
                entry.PAYEEID = dest_payee->PAYEEID;
            changedRecords_ += Model_Billsdeposits::instance().save(billsdeposits);

            EndModal(wxID_OK);
        }
    }
}

void relocatePayeeDialog::OnPayeeUpdated(wxCommandEvent& event)
{
    wxWindow *w = FindFocus();
    bool source_payee = (w && w->GetId() == wxID_NEW);

    wxComboBox *cbPayeeInFocus = cbSourcePayee_;
    if (source_payee)
        cbPayeeInFocus = cbDestPayee_;

    wxString value = cbPayeeInFocus->GetValue();

    //wxStringClientData* data_obj = (wxStringClientData *)cbPayeeInFocus->GetClientObject(cbPayeeInFocus->GetSelection());
    //if (data_obj) wxLogDebug("%s", data_obj->GetData());

    Model_Payee::Data* payee = Model_Payee::instance().get(value);
    if (payee)
    {
        if (source_payee)
            sourcePayeeID_ = payee->PAYEEID;
        else
            destPayeeID_ = payee->PAYEEID;
    }
    event.Skip();
}
