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

#include "relocatecategorydialog.h"
#include "paths.h"
#include "categdialog.h"
#include "constants.h"
#include "webapp.h"

#include "Model_Checking.h"
#include "Model_Budgetsplittransaction.h"
#include "Model_Payee.h"
#include "Model_Budget.h"
#include "Model_Billsdeposits.h"

wxIMPLEMENT_DYNAMIC_CLASS(relocateCategoryDialog, wxDialog);

wxBEGIN_EVENT_TABLE(relocateCategoryDialog, wxDialog)
    EVT_BUTTON(wxID_CLEAR, relocateCategoryDialog::OnSelectSource)
    EVT_BUTTON(wxID_NEW, relocateCategoryDialog::OnSelectDest)
    EVT_BUTTON(wxID_OK, relocateCategoryDialog::OnOk)
wxEND_EVENT_TABLE()

relocateCategoryDialog::relocateCategoryDialog( )
{}

relocateCategoryDialog::relocateCategoryDialog(wxWindow* parent
    , int sourceCatID, int sourceSubCatID)
    : m_buttonSource(nullptr)
    , m_info(nullptr)
    , m_buttonDest(nullptr)
{

    m_sourceCatID    = sourceCatID;
    m_sourceSubCatID = sourceSubCatID;

    m_destCatID      = -1;
    m_destSubCatID   = -1;
    m_changedRecords =  0;

    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("Relocate Category Dialog"), wxDefaultPosition, wxSize(500, 300), style);
}

bool relocateCategoryDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    IsOkOk();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    Centre();
    return TRUE;
}

void relocateCategoryDialog::CreateControls()
{
    wxSizerFlags flagsH, flagsV, flagsExpand;
    flagsH.Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Center();
    flagsV.Align(wxALIGN_LEFT).Border(wxALL, 5).Center();
    flagsExpand.Align(wxALIGN_LEFT).Border(wxALL, 5).Expand();

    wxStaticText* headerText = new wxStaticText(this, wxID_STATIC
        , _("Relocate source category to the destination category"));
    wxStaticLine* lineTop = new wxStaticLine(this, wxID_STATIC);

    m_buttonSource = new wxButton(this, wxID_CLEAR, _("Select Source Category"), wxDefaultPosition, wxSize(200, -1));
    Model_Category::Data* category = Model_Category::instance().get(m_sourceCatID);
    if (category)
        m_buttonSource->SetLabelText(Model_Category::full_name(m_sourceCatID, m_sourceSubCatID));

    m_buttonDest = new wxButton(this, wxID_NEW, _("Select Destination Category"), wxDefaultPosition, wxSize(200, -1));
    wxStaticLine* lineBottom = new wxStaticLine(this, wxID_STATIC);

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* request_sizer = new wxFlexGridSizer(0, 2, 0, 0);

    topSizer->Add(boxSizer, g_flagsV);

    boxSizer->Add(headerText, g_flagsV);
    boxSizer->Add(lineTop, g_flagsExpand);

    request_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Relocate:")), flagsH);
    request_sizer->Add(new wxStaticText(this, wxID_STATIC, _("to:")), flagsH);
    request_sizer->Add(m_buttonSource, flagsH);
    request_sizer->Add(m_buttonDest, flagsH);
    boxSizer->Add(request_sizer);

    boxSizer->Add(lineBottom, flagsExpand);

    m_info = new wxStaticText(this, wxID_STATIC, "");
    boxSizer->Add(m_info, flagsExpand);

    wxStaticLine* lineBottom2 = new wxStaticLine(this, wxID_STATIC);
    boxSizer->Add(lineBottom2, flagsExpand);

    wxButton* okButton = new wxButton(this, wxID_OK, _("&OK "));
    wxButton* cancelButton = new wxButton(this, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    cancelButton-> SetFocus();
    wxBoxSizer* buttonBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonBoxSizer->Add(okButton, flagsH);
    buttonBoxSizer->Add(cancelButton, flagsH);
    boxSizer->Add(buttonBoxSizer, flagsV);

    this->Fit();
}

void relocateCategoryDialog::OnSelectSource(wxCommandEvent& WXUNUSED(event))
{
    mmCategDialog sourceCat(this, m_sourceCatID, m_sourceSubCatID, false);

    if (sourceCat.ShowModal() == wxID_OK)
    {
        m_sourceCatID    = sourceCat.getCategId();
        m_sourceSubCatID = sourceCat.getSubCategId();
        Model_Category::Data* category = Model_Category::instance().get(m_sourceCatID);
        Model_Subcategory::Data* sub_category = Model_Subcategory::instance().get(m_sourceSubCatID);

        m_buttonSource->SetLabelText(Model_Category::full_name(category, sub_category));
        IsOkOk();
    }
}

void relocateCategoryDialog::OnSelectDest(wxCommandEvent& WXUNUSED(event))
{
    mmCategDialog destCat(this, m_destCatID, m_destSubCatID, false);

    if (destCat.ShowModal() == wxID_OK)
    {
        m_destCatID    = destCat.getCategId();
        m_destSubCatID = destCat.getSubCategId();
        m_buttonDest->SetLabelText(Model_Category::full_name(m_destCatID, m_destSubCatID));
        IsOkOk();
    }
}

int relocateCategoryDialog::updatedCategoriesCount() const
{
    return m_changedRecords;
}

void relocateCategoryDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{

    if (wxMessageBox(_("Please Confirm:"), _("Category Relocation Confirmation"), wxOK | wxCANCEL) == wxOK)
    {
        auto transactions = Model_Checking::instance()
            .find(Model_Checking::CATEGID(m_sourceCatID)
                , Model_Checking::SUBCATEGID(m_sourceSubCatID));
        auto checking_split = Model_Splittransaction::instance()
            .find(Model_Splittransaction::CATEGID(m_sourceCatID)
                , Model_Splittransaction::SUBCATEGID(m_sourceSubCatID));
        auto billsdeposits = Model_Billsdeposits::instance()
            .find(Model_Billsdeposits::CATEGID(m_sourceCatID)
                , Model_Billsdeposits::SUBCATEGID(m_sourceSubCatID));
        auto budget = Model_Budget::instance()
            .find(Model_Budget::CATEGID(m_sourceCatID)
                , Model_Budget::SUBCATEGID(m_sourceSubCatID));
        auto budget_split = Model_Budgetsplittransaction::instance()
            .find(Model_Budgetsplittransaction::CATEGID(m_sourceCatID)
                , Model_Budgetsplittransaction::SUBCATEGID(m_sourceSubCatID));
        auto payees = Model_Payee::instance()
            .find(Model_Payee::CATEGID(m_sourceCatID)
                , Model_Payee::SUBCATEGID(m_sourceSubCatID));

        for (auto &entry : transactions)
        {
            entry.CATEGID = m_destCatID;
            entry.SUBCATEGID = m_destSubCatID;
        }
        m_changedRecords += Model_Checking::instance().save(transactions);

        for (auto &entry : billsdeposits)
        {
            entry.CATEGID = m_destCatID;
            entry.SUBCATEGID = m_destSubCatID;
        }
        m_changedRecords += Model_Billsdeposits::instance().save(billsdeposits);

        for (auto &entry : checking_split)
        {
            entry.CATEGID = m_destCatID;
            entry.SUBCATEGID = m_destSubCatID;
        }
        m_changedRecords += Model_Splittransaction::instance().save(checking_split);

        for (auto &entry : payees)
        {
            entry.CATEGID = m_destCatID;
            entry.SUBCATEGID = m_destSubCatID;
        }
        m_changedRecords += Model_Payee::instance().save(payees);
        mmWebApp::MMEX_WebApp_UpdatePayee();

        for (auto &entry : budget_split)
        {
            entry.CATEGID = m_destCatID;
            entry.SUBCATEGID = m_destSubCatID;
        }
        m_changedRecords += Model_Budgetsplittransaction::instance().save(budget_split);

        for (auto &entry : budget)
        {
            Model_Budget::instance().remove(entry.BUDGETENTRYID);
            m_changedRecords++;
        }

        EndModal(wxID_OK);
    }
}

void relocateCategoryDialog::IsOkOk()
{
    auto transactions = Model_Checking::instance()
        .find(Model_Checking::CATEGID(m_sourceCatID)
            , Model_Checking::SUBCATEGID(m_sourceSubCatID));
    auto checking_split = Model_Splittransaction::instance()
        .find(Model_Splittransaction::CATEGID(m_sourceCatID)
            , Model_Splittransaction::SUBCATEGID(m_sourceSubCatID));
    auto billsdeposits = Model_Billsdeposits::instance()
        .find(Model_Billsdeposits::CATEGID(m_sourceCatID)
            , Model_Billsdeposits::SUBCATEGID(m_sourceSubCatID));
    auto budget = Model_Budget::instance()
        .find(Model_Budget::CATEGID(m_sourceCatID)
            , Model_Budget::SUBCATEGID(m_sourceSubCatID));
    auto budget_split = Model_Budgetsplittransaction::instance()
        .find(Model_Budgetsplittransaction::CATEGID(m_sourceCatID)
            , Model_Budgetsplittransaction::SUBCATEGID(m_sourceSubCatID));
    auto payees = Model_Payee::instance()
        .find(Model_Payee::CATEGID(m_sourceCatID)
            , Model_Payee::SUBCATEGID(m_sourceSubCatID));

    int trxs_size = (m_sourceCatID < 0 && m_sourceSubCatID < 0) ? 0 : int(transactions.size());
    int checks_size = int(checking_split.size());
    int bills_size = int(billsdeposits.size());
    int budget_split_size = int(budget_split.size());
    int payees_size = (m_sourceCatID < 0 && m_sourceSubCatID < 0) ? 0 : int(payees.size());
    int budget_size = int(budget.size());

    int total = trxs_size + checks_size + bills_size + budget_split_size + payees_size + budget_size;

    wxString msgStr = wxString()
        << wxString::Format(_("Records found in transactions: %i"), trxs_size) << "\n"
        << wxString::Format(_("Records found in split transactions: %i"), checks_size) << "\n"
        << wxString::Format(_("Records found in recurring transactions: %i"), bills_size) << "\n"
        << wxString::Format(_("Records found in recurring split transactions: %i"), budget_split_size) << "\n"
        << wxString::Format(_("Records found as Default Payee Category: %i"), payees_size) << "\n"
        << wxString::Format(_("Records found in budget: %i"), budget_size);

    m_info->SetLabel(msgStr);

    bool e = true;
    if (total == 0)
        e = false;
    else if (m_sourceCatID == m_destCatID && m_sourceSubCatID == m_destSubCatID)
        e = false;
    else if (m_destCatID < 0 || m_sourceCatID < 0)
        e = false;
    wxButton* ok = wxStaticCast(FindWindow(wxID_OK), wxButton);
    ok->Enable(e);
}