/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011 Stefano Giorgio
 Copyright (C) 2021 - 2022 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2016, 2020 - 2022 Nikolay Akimov

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
#include "model/Model_Billsdeposits.h"
#include "model/Model_Checking.h"
#include "model/Model_Payee.h"
#include "model/Model_Attachment.h"

wxIMPLEMENT_DYNAMIC_CLASS(relocatePayeeDialog, wxDialog);

wxBEGIN_EVENT_TABLE(relocatePayeeDialog, wxDialog)
    EVT_CHILD_FOCUS(relocatePayeeDialog::OnFocusChange)
    EVT_BUTTON(wxID_OK, relocatePayeeDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, relocatePayeeDialog::OnCancel)
    EVT_COMBOBOX(wxID_ANY, relocatePayeeDialog::OnTextUpdated)
wxEND_EVENT_TABLE()

relocatePayeeDialog::relocatePayeeDialog( )
{
}

relocatePayeeDialog::relocatePayeeDialog(wxWindow* parent, int source_payee_id)
    : destPayeeID_(-1)
    , m_changed_records(0)
    , m_info(nullptr)
{
    sourcePayeeID_  = source_payee_id;

    this->SetFont(parent->GetFont());
    Create(parent);
}

bool relocatePayeeDialog::Create(wxWindow* parent
    , wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    IsOkOk();

    SetIcon(mmex::getProgramIcon());

    SetMinSize(wxSize(500, 300));
    Fit();
    Centre();
    return TRUE;
}

void relocatePayeeDialog::CreateControls()
{
    wxSizerFlags flagsH, flagsV, flagsExpand;
    flagsH.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Center();
    flagsV.Align(wxALIGN_LEFT).Border(wxALL, 5).Center();
    flagsExpand.Align(wxALIGN_LEFT).Border(wxALL, 5).Expand();

    wxStaticText* headerText = new wxStaticText( this, wxID_STATIC
        , _("Relocate all source payee to the destination payee"));
    wxStaticLine* lineTop = new wxStaticLine(this,wxID_STATIC
        , wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);

    cbSourcePayee_ = new mmComboBoxUsedPayee(this);
    cbSourcePayee_->mmSetId(sourcePayeeID_);
    cbSourcePayee_->SetMinSize(wxSize(200, -1));

    cbDestPayee_ = new mmComboBoxPayee(this, wxID_NEW);
    cbDestPayee_->SetMinSize(wxSize(200, -1));

    cbDeleteSourcePayee_ = new wxCheckBox(this, wxID_ANY
        , _("Delete source payee after relocation"));

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, flagsExpand);
    wxFlexGridSizer* request_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    request_sizer->AddGrowableCol(0, 1);
    request_sizer->AddGrowableCol(1, 1);

    boxSizer->Add(headerText, flagsV);
    boxSizer->Add(lineTop, flagsExpand);

    request_sizer->Add(new wxStaticText( this, wxID_STATIC,_("Relocate:")), flagsH);
    request_sizer->Add(new wxStaticText( this, wxID_STATIC,_("to:")), flagsH);
    request_sizer->Add(cbSourcePayee_, flagsExpand);
    request_sizer->Add(cbDestPayee_, flagsExpand);

    wxStaticLine* lineMiddle = new wxStaticLine(this, wxID_STATIC
        , wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);

    boxSizer->Add(request_sizer, flagsExpand);
    boxSizer->Add(cbDeleteSourcePayee_, flagsExpand);
    boxSizer->Add(lineMiddle, flagsExpand);

    m_info = new wxStaticText(this, wxID_STATIC, "");
    boxSizer->Add(m_info, flagsExpand);

    wxStaticLine* lineBottom = new wxStaticLine(this, wxID_STATIC
        , wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    boxSizer->Add(lineBottom, flagsExpand);

    wxButton* okButton = new wxButton(this, wxID_OK, _("Relocate"));
    wxButton* cancelButton = new wxButton(this, wxID_CANCEL, _("Close"));

    wxBoxSizer* buttonBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonBoxSizer->Add(okButton, flagsH);
    buttonBoxSizer->Add(cancelButton, flagsH);
    boxSizer->Add(buttonBoxSizer, flagsV);

    cancelButton->SetFocus();
}

void relocatePayeeDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(m_changed_records > 0 ? wxID_OK : wxID_CANCEL);
}

void relocatePayeeDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    const auto& source_payee_name = cbSourcePayee_->GetValue();
    const auto& destination_payee_name = cbDestPayee_->GetValue();
    const wxString& info = wxString::Format(_("From %s to %s")
        , source_payee_name
        , destination_payee_name);

    int ans = wxMessageBox(_("Please Confirm:") + "\n" + info
        , _("Payee Relocation Confirmation")
        , wxOK | wxCANCEL | wxICON_INFORMATION);

    if (ans == wxOK)
    {
        Model_Checking::instance().Savepoint();
        auto transactions = Model_Checking::instance().find(Model_Checking::PAYEEID(sourcePayeeID_));
        for (auto &entry : transactions) {
            entry.PAYEEID = destPayeeID_;
        }
        m_changed_records += Model_Checking::instance().save(transactions);
        Model_Checking::instance().ReleaseSavepoint();

        Model_Billsdeposits::instance().Savepoint();
        auto billsdeposits = Model_Billsdeposits::instance().find(Model_Billsdeposits::PAYEEID(sourcePayeeID_));
        for (auto &entry : billsdeposits) {
            entry.PAYEEID = destPayeeID_;
        }
        m_changed_records += Model_Billsdeposits::instance().save(billsdeposits);
        Model_Billsdeposits::instance().ReleaseSavepoint();

        if (cbDeleteSourcePayee_->IsChecked())
        {
            if (Model_Payee::instance().remove(sourcePayeeID_))
            {
                mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::PAYEE), sourcePayeeID_);
                mmWebApp::MMEX_WebApp_UpdatePayee();
            }
            cbSourcePayee_->mmDoReInitialize();
            cbDestPayee_->mmDoReInitialize();
        }

        IsOkOk();
    }
}

void relocatePayeeDialog::IsOkOk()
{
    bool e = true;

    destPayeeID_ = cbDestPayee_->mmGetId();
    sourcePayeeID_ = cbSourcePayee_->mmGetId();
    int trxs_size = (sourcePayeeID_ < 0) ? 0 : Model_Checking::instance().find(Model_Checking::PAYEEID(sourcePayeeID_)).size();
    int bills_size = (sourcePayeeID_ < 0) ? 0 : Model_Billsdeposits::instance().find(Model_Billsdeposits::PAYEEID(sourcePayeeID_)).size();

    if (destPayeeID_ < 0 || sourcePayeeID_ < 0
        || destPayeeID_ == sourcePayeeID_
        || trxs_size + bills_size == 0) {
        e = false;
    }

    wxString msgStr = wxString()
        << wxString::Format(_("Records found in transactions: %i"), trxs_size) << "\n"
        << wxString::Format(_("Records found in recurring transactions: %i"), bills_size);

    m_info->SetLabel(msgStr);

    wxButton* ok = wxStaticCast(FindWindow(wxID_OK), wxButton);
    ok->Enable(e);
}

void relocatePayeeDialog::OnFocusChange(wxChildFocusEvent& event)
{
    cbDestPayee_->ChangeValue(cbDestPayee_->GetValue());
    cbSourcePayee_->ChangeValue(cbSourcePayee_->GetValue());
    IsOkOk();
    event.Skip();
}

void relocatePayeeDialog::OnTextUpdated(wxCommandEvent& event)
{
    IsOkOk();
}
