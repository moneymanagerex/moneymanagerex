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
#include "attachmentdialog.h"
#include "webapp.h"
#include "paths.h"
#include "constants.h"
#include "Model_Billsdeposits.h"
#include "Model_Checking.h"
#include "Model_Payee.h"
#include "Model_Attachment.h"

wxIMPLEMENT_DYNAMIC_CLASS(relocatePayeeDialog, wxDialog);

wxBEGIN_EVENT_TABLE(relocatePayeeDialog, wxDialog)
    EVT_BUTTON(wxID_OK, relocatePayeeDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, relocatePayeeDialog::OnCancel)
    EVT_TEXT(wxID_ANY, relocatePayeeDialog::OnPayeeChanged)
wxEND_EVENT_TABLE()

relocatePayeeDialog::relocatePayeeDialog( )
{
}

relocatePayeeDialog::relocatePayeeDialog(wxWindow* parent, int source_payee_id)
    : destPayeeID_(-1)
    , cbSourcePayee_(nullptr)
    , cbDestPayee_(nullptr)
    , m_changed_records(0)
    , m_info(nullptr)
{
    sourcePayeeID_  = source_payee_id;

    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;

    Create(parent, wxID_STATIC, _("Relocate Payee Dialog"), wxDefaultPosition, wxSize(500, 300), style);
}

bool relocatePayeeDialog::Create(wxWindow* parent
    , wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    IsOkOk();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    Centre();
    return TRUE;
}

void relocatePayeeDialog::CreateControls()
{
    wxSizerFlags flagsH, flagsV, flagsExpand;
    flagsH.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Center();
	flagsV.Align(wxALIGN_LEFT).Border(wxALL, 5).Center();
	flagsExpand.Align(wxALIGN_LEFT).Border(wxALL, 5).Expand();
    wxSize btnSize = wxSize(180,-1);

    wxStaticText* headerText = new wxStaticText( this, wxID_STATIC
        , _("Relocate all source payee to the destination payee"));
    wxStaticLine* lineTop = new wxStaticLine(this,wxID_STATIC
        , wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);

    cbSourcePayee_ = new wxComboBox(this, wxID_BOTTOM
        , sourcePayeeID_ == -1 ? "" : Model_Payee::get_payee_name(sourcePayeeID_)
        , wxDefaultPosition, btnSize
        , Model_Payee::instance().used_payee_names());
    cbSourcePayee_->AutoComplete(cbSourcePayee_->GetStrings());
    cbSourcePayee_->Enable();

    cbDelete_ = new wxCheckBox(this, wxID_ANY
        , _("Delete source payee after relocation"));

    cbDestPayee_ = new wxComboBox(this, wxID_NEW, ""
        , wxDefaultPosition, btnSize
        , Model_Payee::instance().all_payee_names());
    cbDestPayee_->AutoComplete(cbDestPayee_->GetStrings());

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, flagsV);
    wxFlexGridSizer* request_sizer = new wxFlexGridSizer(0, 2, 0, 0);

    boxSizer->Add(headerText, flagsV);
    boxSizer->Add(lineTop, flagsExpand);

    request_sizer->Add(new wxStaticText( this, wxID_STATIC,_("Relocate:")), flagsH);
    request_sizer->Add(new wxStaticText( this, wxID_STATIC,_("to:")), flagsH);
    request_sizer->Add(cbSourcePayee_, flagsH);
    request_sizer->Add(cbDestPayee_, flagsH);
    
    wxStaticLine* lineMiddle = new wxStaticLine(this, wxID_STATIC
        , wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);

    boxSizer->Add(request_sizer, flagsExpand);
    boxSizer->Add(cbDelete_, flagsExpand);
    boxSizer->Add(lineMiddle, flagsExpand);

    m_info = new wxStaticText(this, wxID_STATIC, "");
    boxSizer->Add(m_info, flagsExpand);

    wxStaticLine* lineBottom = new wxStaticLine(this, wxID_STATIC
        , wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    boxSizer->Add(lineBottom, flagsExpand);

    wxButton* okButton = new wxButton(this, wxID_OK, _("&OK "));
    wxButton* cancelButton = new wxButton(this, wxID_CANCEL, wxGetTranslation(g_CancelLabel));

    wxBoxSizer* buttonBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonBoxSizer->Add(okButton, flagsH);
    buttonBoxSizer->Add(cancelButton, flagsH);
    boxSizer->Add(buttonBoxSizer, flagsV);
}

int relocatePayeeDialog::updatedPayeesCount()
{
    return m_changed_records;
}

void relocatePayeeDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(m_changed_records>0 ? wxID_OK : wxID_CANCEL);
}

void relocatePayeeDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    const wxString& info = wxString::Format(_("From %s to %s")
        , Model_Payee::instance().get(sourcePayeeID_)->PAYEENAME
        , Model_Payee::instance().get(destPayeeID_)->PAYEENAME);

    int ans = wxMessageBox(_("Please Confirm:") + "\n" + info
        , _("Payee Relocation Confirmation")
        , wxOK | wxCANCEL | wxICON_INFORMATION);

    if (ans == wxOK)
    {
        auto transactions = Model_Checking::instance().find(Model_Checking::PAYEEID(sourcePayeeID_));
        for (auto &entry : transactions) {
            entry.PAYEEID = destPayeeID_;
        }
        m_changed_records += Model_Checking::instance().save(transactions);

        auto billsdeposits = Model_Billsdeposits::instance().find(Model_Billsdeposits::PAYEEID(sourcePayeeID_));
        for (auto &entry : billsdeposits) {
            entry.PAYEEID = destPayeeID_;
        }
        m_changed_records += Model_Billsdeposits::instance().save(billsdeposits);

        if (cbDelete_->IsChecked())
        {
            if (Model_Payee::instance().remove(sourcePayeeID_))
            {
                mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::PAYEE), sourcePayeeID_);
                mmWebApp::MMEX_WebApp_UpdatePayee();
                int deleted = cbDestPayee_->FindString(cbSourcePayee_->GetValue());
                if (deleted != wxNOT_FOUND)
                {
                    cbDestPayee_->Delete(deleted);
                    cbDestPayee_->AutoComplete(cbDestPayee_->GetStrings());
                }

            }
        }
        int empty = cbSourcePayee_->FindString(cbSourcePayee_->GetValue());
        if (empty != wxNOT_FOUND)
        {
            cbSourcePayee_->Delete(empty);
            cbSourcePayee_->AutoComplete(cbSourcePayee_->GetStrings());
        }
        cbSourcePayee_->SetValue("");
        cbSourcePayee_->SetSelection(wxNOT_FOUND);
        sourcePayeeID_ = -1;
        IsOkOk();
    }
}

void relocatePayeeDialog::IsOkOk()
{
    bool e = true;
    int trxs_size, bills_size;
    const wxString& destPayeeName = cbDestPayee_->GetValue();
    const wxString& sourcePayeeName = cbSourcePayee_->GetValue();

    Model_Payee::Data* source_payee = Model_Payee::instance().get(sourcePayeeName);
    Model_Payee::Data* dest_payee = Model_Payee::instance().get(destPayeeName);
    if (source_payee) {
        sourcePayeeID_ = source_payee->PAYEEID;
        trxs_size = Model_Checking::instance().find(Model_Checking::PAYEEID(sourcePayeeID_)).size();
        bills_size = Model_Billsdeposits::instance().find(Model_Billsdeposits::PAYEEID(sourcePayeeID_)).size();
    }
    else
    {
        trxs_size = 0;
        bills_size = 0;
    }
    if (dest_payee) {
        destPayeeID_ = dest_payee->PAYEEID;
    }

    if (!dest_payee || !source_payee
        || dest_payee == source_payee
        || trxs_size + bills_size == 0) {
        e = false;
    }

    wxString msgStr = wxString()
        << wxString::Format(_("Records found in transactions: %i"), trxs_size) << "\n"
        << wxString::Format(_("Records found in recurring transactions: %i"), bills_size);

    m_info->SetLabel(msgStr);

    wxButton* ok = wxStaticCast(FindWindow(wxID_OK), wxButton);
    ok->Enable(e);
    if (e) {
        ok->SetFocus();
    }
}

void relocatePayeeDialog::OnPayeeChanged(wxCommandEvent& WXUNUSED(event))
{
    IsOkOk();
}
