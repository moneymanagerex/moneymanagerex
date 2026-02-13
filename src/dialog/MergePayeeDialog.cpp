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

#include "base/constants.h"
#include "base/paths.h"
#include "util/_util.h"

#include "model/ScheduledModel.h"
#include "model/TransactionModel.h"
#include "model/PayeeModel.h"
#include "model/AttachmentModel.h"

#include "AttachmentDialog.h"
#include "MergePayeeDialog.h"
#include "import_export/webapp.h"

wxIMPLEMENT_DYNAMIC_CLASS(MergePayeeDialog, wxDialog);

wxBEGIN_EVENT_TABLE(MergePayeeDialog, wxDialog)
    EVT_CHILD_FOCUS(MergePayeeDialog::OnFocusChange)
    EVT_BUTTON(wxID_OK, MergePayeeDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, MergePayeeDialog::OnCancel)
    EVT_COMBOBOX(wxID_ANY, MergePayeeDialog::OnTextUpdated)
wxEND_EVENT_TABLE()

MergePayeeDialog::MergePayeeDialog( )
{
}

MergePayeeDialog::~MergePayeeDialog()
{
    InfotableModel::instance().setSize("RELOCATEPAYEE_DIALOG_SIZE", GetSize());
}

MergePayeeDialog::MergePayeeDialog(wxWindow* parent, int64 source_payee_id)
{
    sourcePayeeID_  = source_payee_id;

    this->SetFont(parent->GetFont());
    Create(parent);
}

bool MergePayeeDialog::Create(wxWindow* parent
    , wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    style |= wxRESIZE_BORDER;
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style, name);

    CreateControls();
    mmThemeAutoColour(this);
    IsOkOk();

    SetIcon(mmex::getProgramIcon());
    Fit();
    wxSize sz = GetSize();
    SetSizeHints(sz.GetWidth(), sz.GetHeight(), -1, sz.GetHeight());
    mmSetSize(this);
    Centre();
    return true;
}

void MergePayeeDialog::CreateControls()
{
    wxSizerFlags flagsH, flagsV, flagsExpand;
    flagsH.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Center();
    flagsV.Align(wxALIGN_LEFT).Border(wxALL, 5).Center();
    flagsExpand.Align(wxALIGN_LEFT).Border(wxALL, 5).Expand();

    wxStaticText* headerText = new wxStaticText( this, wxID_STATIC
        , _t("Merge Payees"));
    wxStaticLine* lineTop = new wxStaticLine(this,wxID_STATIC
        , wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);

    cbSourcePayee_ = new mmComboBoxUsedPayee(this);
    cbSourcePayee_->mmSetId(sourcePayeeID_);
    cbSourcePayee_->SetMinSize(wxSize(200, -1));

    cbDestPayee_ = new mmComboBoxPayee(this, wxID_NEW, wxDefaultSize, -1, true);
    cbDestPayee_->SetMinSize(wxSize(200, -1));

    cbDeleteSourcePayee_ = new wxCheckBox(this, wxID_ANY
        , _t("&Delete source payee after merge"));

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(boxSizer, flagsExpand);
    wxFlexGridSizer* request_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    request_sizer->AddGrowableCol(0, 1);
    request_sizer->AddGrowableCol(1, 1);

    boxSizer->Add(headerText, g_flagsV);
    boxSizer->Add(lineTop, flagsExpand);

    request_sizer->Add(new wxStaticText( this, wxID_STATIC,_t("Source:")), flagsH);
    request_sizer->Add(new wxStaticText( this, wxID_STATIC,_t("Target:")), flagsH);
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

    wxButton* okButton = new wxButton(this, wxID_OK, _t("&Merge"));
    wxButton* cancelButton = new wxButton(this, wxID_CANCEL, _t("&Close"));

    wxBoxSizer* buttonBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonBoxSizer->Add(okButton, flagsH);
    buttonBoxSizer->Add(cancelButton, flagsH);
    boxSizer->Add(buttonBoxSizer, flagsV);

    cancelButton->SetFocus();
}

void MergePayeeDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(m_changed_records > 0 ? wxID_OK : wxID_CANCEL);
}

void MergePayeeDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    const auto& source_payee_name = cbSourcePayee_->GetValue();
    const auto& destination_payee_name = cbDestPayee_->GetValue();
    const wxString& info = wxString::Format(_t("From %1$s to %2$s")
        , source_payee_name
        , destination_payee_name);

    int ans = wxMessageBox(_t("Please Confirm:") + "\n" + info
        , _t("Merge payees confirmation")
        , wxOK | wxCANCEL | wxICON_INFORMATION);

    if (ans == wxOK)
    {
        TransactionModel::instance().Savepoint();
        auto transactions = TransactionModel::instance().find(TransactionModel::PAYEEID(sourcePayeeID_));
        for (auto &entry : transactions) {
            entry.PAYEEID = destPayeeID_;
        }
        m_changed_records += TransactionModel::instance().save(transactions);
        TransactionModel::instance().ReleaseSavepoint();

        ScheduledModel::instance().Savepoint();
        auto billsdeposits = ScheduledModel::instance().find(ScheduledModel::PAYEEID(sourcePayeeID_));
        for (auto &entry : billsdeposits) {
            entry.PAYEEID = destPayeeID_;
        }
        m_changed_records += ScheduledModel::instance().save(billsdeposits);
        ScheduledModel::instance().ReleaseSavepoint();

        if (cbDeleteSourcePayee_->IsChecked())
        {
            if (PayeeModel::instance().remove(sourcePayeeID_))
            {
                mmAttachmentManage::DeleteAllAttachments(PayeeModel::refTypeName, sourcePayeeID_);
                mmWebApp::MMEX_WebApp_UpdatePayee();
            }
            cbSourcePayee_->mmDoReInitialize();
            cbDestPayee_->mmDoReInitialize();
        }

        IsOkOk();
    }
}

void MergePayeeDialog::IsOkOk()
{
    bool e = true;

    destPayeeID_ = cbDestPayee_->mmGetId();
    sourcePayeeID_ = cbSourcePayee_->mmGetId();
    int trxs_size = (sourcePayeeID_ < 0) ? 0 : TransactionModel::instance().find(TransactionModel::PAYEEID(sourcePayeeID_)).size();
    int bills_size = (sourcePayeeID_ < 0) ? 0 : ScheduledModel::instance().find(ScheduledModel::PAYEEID(sourcePayeeID_)).size();

    if (destPayeeID_ < 0 || sourcePayeeID_ < 0
        || destPayeeID_ == sourcePayeeID_
        || trxs_size + bills_size == 0) {
        e = false;
    }

    wxString msgStr = wxString()
        << wxString::Format(_t("Records found in transactions: %i"), trxs_size) << "\n"
        << wxString::Format(_t("Records found in scheduled transactions: %i"), bills_size);

    m_info->SetLabel(msgStr);

    wxButton* ok = wxStaticCast(FindWindow(wxID_OK), wxButton);
    ok->Enable(e);
}

void MergePayeeDialog::OnFocusChange(wxChildFocusEvent& event)
{
    cbDestPayee_->ChangeValue(cbDestPayee_->GetValue());
    cbSourcePayee_->ChangeValue(cbSourcePayee_->GetValue());
    IsOkOk();
    event.Skip();
}

void MergePayeeDialog::OnTextUpdated(wxCommandEvent&)
{
    IsOkOk();
}
