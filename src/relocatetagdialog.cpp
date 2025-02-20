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

#include "relocatetagdialog.h"
#include "attachmentdialog.h"
#include "webapp.h"
#include "paths.h"
#include "constants.h"
#include "util.h"
#include "model/Model_Billsdeposits.h"
#include "model/Model_Checking.h"
#include "model/Model_Payee.h"
#include "model/Model_Attachment.h"
#include "model/Model_Tag.h"

wxIMPLEMENT_DYNAMIC_CLASS(relocateTagDialog, wxDialog);

wxBEGIN_EVENT_TABLE(relocateTagDialog, wxDialog)
    EVT_CHILD_FOCUS(relocateTagDialog::OnFocusChange)
    EVT_BUTTON(wxID_OK, relocateTagDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, relocateTagDialog::OnCancel)
    EVT_COMBOBOX(wxID_ANY, relocateTagDialog::OnTextUpdated)
wxEND_EVENT_TABLE()

relocateTagDialog::relocateTagDialog( )
{
}

relocateTagDialog::~relocateTagDialog()
{
    Model_Infotable::instance().setSize("RELOCATETAG_DIALOG_SIZE", GetSize());
}

relocateTagDialog::relocateTagDialog(wxWindow* parent, int64 source_tag_id)
{
    sourceTagID_  = source_tag_id;

    this->SetFont(parent->GetFont());
    Create(parent);
}

bool relocateTagDialog::Create(wxWindow* parent
    , wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    style |= wxRESIZE_BORDER;
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style, name);

    CreateControls();
    IsOkOk();

    SetIcon(mmex::getProgramIcon());
    Fit();
    wxSize sz = GetSize();
    SetSizeHints(sz.GetWidth(), sz.GetHeight(), -1, sz.GetHeight());
    mmSetSize(this);
    Centre();
    return true;
}

void relocateTagDialog::CreateControls()
{
    wxSizerFlags flagsH, flagsV, flagsExpand;
    flagsH.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Center();
    flagsV.Align(wxALIGN_LEFT).Border(wxALL, 5).Center();
    flagsExpand.Align(wxALIGN_LEFT).Border(wxALL, 5).Expand();

    wxStaticText* headerText = new wxStaticText( this, wxID_STATIC
        , _t("Merge Tags"));
    wxStaticLine* lineTop = new wxStaticLine(this,wxID_STATIC
        , wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);

    choices_.Clear();
    for (const auto& tag : Model_Tag::instance().all())
        choices_.Add(tag.TAGNAME);
    cbSourceTag_ = new wxComboBox(this, wxID_REPLACE, "", wxDefaultPosition, wxDefaultSize, choices_);
    cbSourceTag_->SetMinSize(wxSize(200, -1));

    cbDestTag_ = new wxComboBox(this, wxID_NEW, "", wxDefaultPosition, wxDefaultSize, choices_);
    cbDestTag_->SetMinSize(wxSize(200, -1));

    cbDeleteSourceTag_ = new wxCheckBox(this, wxID_ANY
        , _t("&Delete source tag after merge"));

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
    request_sizer->Add(cbSourceTag_, flagsExpand);
    request_sizer->Add(cbDestTag_, flagsExpand);

    wxStaticLine* lineMiddle = new wxStaticLine(this, wxID_STATIC
        , wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);

    boxSizer->Add(request_sizer, flagsExpand);
    boxSizer->Add(cbDeleteSourceTag_, flagsExpand);
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

void relocateTagDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(m_changed_records > 0 ? wxID_OK : wxID_CANCEL);
}

void relocateTagDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    const auto& source_tag_name = cbSourceTag_->GetValue();
    const auto& destination_tag_name = cbDestTag_->GetValue();
    const wxString& info = wxString::Format(_t("From %1$s to %2$s")
        , source_tag_name
        , destination_tag_name);

    int ans = wxMessageBox(_t("Please Confirm:") + "\n" + info
        , _t("Merge tags confirmation")
        , wxOK | wxCANCEL | wxICON_INFORMATION);

    if (ans == wxOK)
    {
        Model_Taglink::instance().Savepoint();
        Model_Taglink::Data_Set taglinks = Model_Taglink::instance().find(Model_Taglink::TAGID(sourceTagID_));
        for (auto &entry : taglinks) {
            entry.TAGID = destTagID_;
        }
        m_changed_records += Model_Taglink::instance().save(taglinks);
        Model_Taglink::instance().ReleaseSavepoint();

        if (cbDeleteSourceTag_->IsChecked())
        {
            Model_Tag::instance().remove(sourceTagID_);
            choices_.Remove(source_tag_name);
            cbSourceTag_->Set(choices_);
            cbDestTag_->Set(choices_);
        }

        IsOkOk();
    }
}

void relocateTagDialog::IsOkOk()
{
    bool e = true;
    Model_Tag::Data* source = Model_Tag::instance().get(cbSourceTag_->GetValue());
    Model_Tag::Data* dest = Model_Tag::instance().get(cbDestTag_->GetValue());
    if (dest) destTagID_ = dest->TAGID;
    if (source) sourceTagID_ = source->TAGID;
    int trxs_size = (sourceTagID_ < 0) ? 0 : Model_Taglink::instance().find(
        Model_Taglink::REFTYPE(Model_Attachment::REFTYPE_NAME_TRANSACTION),
        Model_Taglink::TAGID(sourceTagID_)
    ).size();
    int split_size = (sourceTagID_ < 0) ? 0 : Model_Taglink::instance().find(
        Model_Taglink::REFTYPE(Model_Attachment::REFTYPE_NAME_TRANSACTIONSPLIT),
        Model_Taglink::TAGID(sourceTagID_)
    ).size();
    int bills_size = (sourceTagID_ < 0) ? 0 : Model_Taglink::instance().find(
        Model_Taglink::REFTYPE(Model_Attachment::REFTYPE_NAME_BILLSDEPOSIT),
        Model_Taglink::TAGID(sourceTagID_)
    ).size();
    int bill_split_size = (sourceTagID_ < 0) ? 0 : Model_Taglink::instance().find(
        Model_Taglink::REFTYPE(Model_Attachment::REFTYPE_NAME_BILLSDEPOSITSPLIT),
        Model_Taglink::TAGID(sourceTagID_)
    ).size();

    if (destTagID_ < 0 || sourceTagID_ < 0
        || destTagID_ == sourceTagID_
        || trxs_size + split_size + bills_size + bill_split_size == 0) {
        e = false;
    }

    wxString msgStr = wxString()
        << wxString::Format(_t("Records found in transactions: %i"), trxs_size) << "\n"
        << wxString::Format(_t("Records found in split transactions: %i"), split_size) << "\n"
        << wxString::Format(_t("Records found in scheduled transactions: %i"), bills_size) << "\n"
        << wxString::Format(_t("Records found in scheduled split transactions: %i"), bill_split_size);

    m_info->SetLabel(msgStr);

    wxButton* ok = wxStaticCast(FindWindow(wxID_OK), wxButton);
    ok->Enable(e);
}

void relocateTagDialog::OnFocusChange(wxChildFocusEvent& event)
{
    IsOkOk();
    event.Skip();
}

void relocateTagDialog::OnTextUpdated(wxCommandEvent& )
{
    IsOkOk();
}
