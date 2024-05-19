/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011 Stefano Giorgio
 Copyright (C) 2016, 2020, 2022 Nikolay Akimov
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "relocatecategorydialog.h"
#include "paths.h"
#include "categdialog.h"
#include "constants.h"
#include "webapp.h"
#include "util.h"
#include "model/allmodel.h"

wxIMPLEMENT_DYNAMIC_CLASS(relocateCategoryDialog, wxDialog);

wxBEGIN_EVENT_TABLE(relocateCategoryDialog, wxDialog)
    EVT_CHAR_HOOK(relocateCategoryDialog::OnComboKey)
    EVT_CHILD_FOCUS(relocateCategoryDialog::OnFocusChange)
    EVT_COMBOBOX(wxID_ANY, relocateCategoryDialog::OnTextUpdated)
    EVT_BUTTON(wxID_OK, relocateCategoryDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, relocateCategoryDialog::OnCancel)
wxEND_EVENT_TABLE()

relocateCategoryDialog::relocateCategoryDialog()
{}

relocateCategoryDialog::~relocateCategoryDialog()
{
    Model_Infotable::instance().Set("RELOCATECATEG_DIALOG_SIZE", GetSize());
}

relocateCategoryDialog::relocateCategoryDialog(wxWindow* parent
    , int sourceCatID, int sourceSubCatID)
    : m_sourceCatID(sourceCatID)
    , m_sourceSubCatID(sourceSubCatID)
{
    this->SetFont(parent->GetFont());
    Create(parent);
}

bool relocateCategoryDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString& name)
{
    style |= wxRESIZE_BORDER;
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    CreateControls();
    IsOkOk();

    SetIcon(mmex::getProgramIcon());
    Fit();
    wxSize sz = GetSize();
    SetSizeHints(sz.GetWidth(), sz.GetHeight(), -1, sz.GetHeight());
    Centre();
    mmSetSize(this);
    return true;
}

void relocateCategoryDialog::CreateControls()
{
    wxSizerFlags flagsH, flagsV, flagsExpand;
    flagsH.Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Center();
    flagsV.Align(wxALIGN_LEFT).Border(wxALL, 5).Center();
    flagsExpand.Align(wxALIGN_LEFT).Border(wxALL, 5).Expand();

    wxStaticText* headerText = new wxStaticText(this, wxID_STATIC
        , _("Merge Categories"));
    wxStaticLine* lineTop = new wxStaticLine(this, wxID_STATIC);

    cbSourceCategory_ = new mmComboBoxCategory(this, wxID_LAST);
    cbSourceCategory_->SetMinSize(wxSize(200, -1));
    Model_Category::Data* category = Model_Category::instance().get(m_sourceCatID);
    if (category)
        cbSourceCategory_->SetValue(Model_Category::full_name(m_sourceCatID));

    cbDestCategory_ = new mmComboBoxCategory(this, wxID_NEW, wxDefaultSize, -1, true);
    cbDestCategory_->SetMinSize(wxSize(200, -1));

    cbDeleteSourceCategory_ = new wxCheckBox(this, wxID_ANY
        , _("&Delete source category after merge (if source category has no subcategories)"));

    wxStaticLine* lineBottom = new wxStaticLine(this, wxID_STATIC);

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* request_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    request_sizer->AddGrowableCol(0, 1);
    request_sizer->AddGrowableCol(1, 1);

    topSizer->Add(boxSizer, flagsExpand);

    boxSizer->Add(headerText, g_flagsV);
    boxSizer->Add(lineTop, g_flagsExpand);

    request_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Source:")), flagsH);
    request_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Target:")), flagsH);
    request_sizer->Add(cbSourceCategory_, flagsExpand);
    request_sizer->Add(cbDestCategory_, flagsExpand);

    boxSizer->Add(request_sizer, flagsExpand);
    boxSizer->Add(cbDeleteSourceCategory_, flagsExpand);
    boxSizer->Add(lineBottom, flagsExpand);

    m_info = new wxStaticText(this, wxID_STATIC, "");
    boxSizer->Add(m_info, flagsExpand);

    wxStaticLine* lineBottom2 = new wxStaticLine(this, wxID_STATIC);
    boxSizer->Add(lineBottom2, flagsExpand);

    wxButton* okButton = new wxButton(this, wxID_OK, _("&Merge"));
    wxButton* cancelButton = new wxButton(this, wxID_CANCEL, _("&Close"));
    cancelButton-> SetFocus();
    wxBoxSizer* buttonBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonBoxSizer->Add(okButton, flagsH);
    buttonBoxSizer->Add(cancelButton, flagsH);
    boxSizer->Add(buttonBoxSizer, flagsV);
}

void relocateCategoryDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(m_changedRecords > 0 ? wxID_OK : wxID_CANCEL);
}

void relocateCategoryDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    int m_destCatID = cbDestCategory_->mmGetCategoryId();

    const auto& source_category_name = cbSourceCategory_->GetValue();
    const auto& destination_category_name = cbDestCategory_->GetValue();
    const wxString& info = wxString::Format(_("From %1$s to %2$s")
        , source_category_name
        , destination_category_name);

    if (wxMessageBox(_("Please Confirm:") + "\n" + info
        , _("Merge categories confirmation"), wxOK | wxCANCEL | wxICON_INFORMATION) == wxOK)
    {
        auto transactions = Model_Checking::instance()
            .find(Model_Checking::CATEGID(m_sourceCatID));
        auto checking_split = Model_Splittransaction::instance()
            .find(Model_Splittransaction::CATEGID(m_sourceCatID));
        auto billsdeposits = Model_Billsdeposits::instance()
            .find(Model_Billsdeposits::CATEGID(m_sourceCatID));
        auto budget = Model_Budget::instance()
            .find(Model_Budget::CATEGID(m_sourceCatID));
        auto budget_split = Model_Budgetsplittransaction::instance()
            .find(Model_Budgetsplittransaction::CATEGID(m_sourceCatID));
        auto payees = Model_Payee::instance()
            .find(Model_Payee::CATEGID(m_sourceCatID));

        for (auto &entry : transactions)
        {
            entry.CATEGID = m_destCatID;
        }
        m_changedRecords += Model_Checking::instance().save(transactions);

        for (auto &entry : billsdeposits)
        {
            entry.CATEGID = m_destCatID;
        }
        m_changedRecords += Model_Billsdeposits::instance().save(billsdeposits);

        for (auto &entry : checking_split)
        {
            entry.CATEGID = m_destCatID;
        }
        m_changedRecords += Model_Splittransaction::instance().save(checking_split);

        for (auto &entry : payees)
        {
            entry.CATEGID = m_destCatID;
        }
        m_changedRecords += Model_Payee::instance().save(payees);
        mmWebApp::MMEX_WebApp_UpdatePayee();

        for (auto &entry : budget_split)
        {
            entry.CATEGID = m_destCatID;
        }
        m_changedRecords += Model_Budgetsplittransaction::instance().save(budget_split);

        for (auto &entry : budget)
        {
            Model_Budget::instance().remove(entry.BUDGETENTRYID);
            m_changedRecords++;
        }

        if (cbDeleteSourceCategory_->IsChecked())
        {
            if (m_sourceSubCatID == -1)
            {
                if (Model_Category::sub_category(Model_Category::instance().get(m_sourceCatID)).empty())
                    Model_Category::instance().remove(m_sourceCatID);
            }

            cbSourceCategory_->mmDoReInitialize();
            cbDestCategory_->mmDoReInitialize();
            mmWebApp::MMEX_WebApp_UpdateCategory();
        }

        IsOkOk();
    }
}

void relocateCategoryDialog::IsOkOk()
{
    m_sourceCatID = cbSourceCategory_->mmGetCategoryId();
    int m_destCatID = cbDestCategory_->mmGetCategoryId();

    auto transactions = Model_Checking::instance()
        .find(Model_Checking::CATEGID(m_sourceCatID));
    auto checking_split = Model_Splittransaction::instance()
        .find(Model_Splittransaction::CATEGID(m_sourceCatID));
    auto billsdeposits = Model_Billsdeposits::instance()
        .find(Model_Billsdeposits::CATEGID(m_sourceCatID));
    auto budget = Model_Budget::instance()
        .find(Model_Budget::CATEGID(m_sourceCatID));
    auto budget_split = Model_Budgetsplittransaction::instance()
        .find(Model_Budgetsplittransaction::CATEGID(m_sourceCatID));
    auto payees = Model_Payee::instance()
        .find(Model_Payee::CATEGID(m_sourceCatID));

    const int trxs_size = (m_sourceCatID < 0 && m_sourceSubCatID < 0) ? 0 : static_cast<int>(transactions.size());
    const int checks_size = static_cast<int>(checking_split.size());
    const int bills_size = (m_sourceCatID < 0 && m_sourceSubCatID < 0) ? 0 : static_cast<int>(billsdeposits.size());
    const int budget_split_size = static_cast<int>(budget_split.size());
    const int payees_size = (m_sourceCatID < 0 && m_sourceSubCatID < 0) ? 0 : static_cast<int>(payees.size());
    const int budget_size = static_cast<int>(budget.size());

    const int total = trxs_size + checks_size + bills_size + budget_split_size + payees_size + budget_size;

    wxString msgStr = wxString()
        << wxString::Format(_("Records found in transactions: %i"), trxs_size) << "\n"
        << wxString::Format(_("Records found in split transactions: %i"), checks_size) << "\n"
        << wxString::Format(_("Records found in scheduled transactions: %i"), bills_size) << "\n"
        << wxString::Format(_("Records found in scheduled split transactions: %i"), budget_split_size) << "\n"
        << wxString::Format(_("Records found as default payee category: %i"), payees_size) << "\n"
        << wxString::Format(_("Records found in budget: %i"), budget_size);

    m_info->SetLabel(msgStr);

    bool e = true;
    if (total == 0)
        e = false;
    else if (m_sourceCatID == m_destCatID)
        e = false;
    else if (m_destCatID < 0 || m_sourceCatID < 0)
        e = false;
    wxButton* ok = wxStaticCast(FindWindow(wxID_OK), wxButton);
    ok->Enable(e);
}

void relocateCategoryDialog::OnComboKey(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_RETURN)
    {
        auto id = event.GetId();
        switch (id)
        {
        case wxID_LAST:
        {
            auto category = cbSourceCategory_->GetValue();
            if (category.empty())
            {
                mmCategDialog dlg(this, true, -1);
                dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    cbSourceCategory_->mmDoReInitialize();
                category = Model_Category::full_name(dlg.getCategId());
                cbSourceCategory_->ChangeValue(category);
                return;
            }
        }

        break;
        case wxID_NEW:
        {
            auto category = cbDestCategory_->GetValue();
            if (category.empty())
            {
                mmCategDialog dlg(this, true, -1);
                dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    cbDestCategory_->mmDoReInitialize();
                category = Model_Category::full_name(dlg.getCategId());
                cbDestCategory_->ChangeValue(category);
                return;
            }
        }
        break;
        default:
            break;
        }
    }

    event.Skip();
}

void relocateCategoryDialog::OnFocusChange(wxChildFocusEvent& event)
{
    cbDestCategory_->ChangeValue(cbDestCategory_->GetValue());
    cbSourceCategory_->ChangeValue(cbSourceCategory_->GetValue());
    IsOkOk();
    event.Skip();
}

void relocateCategoryDialog::OnTextUpdated(wxCommandEvent& event)
{
    IsOkOk();
}
