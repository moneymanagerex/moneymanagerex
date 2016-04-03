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
#include "wx/statline.h"
#include "model/Model_Category.h"
#include "model/Model_Payee.h"
#include "model/Model_Budget.h"
#include "model/Model_Billsdeposits.h"
#include "model/Model_Checking.h"

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
    : m_buttonSource()
    , m_buttonDest()
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
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    Centre();
    return TRUE;
}

void relocateCategoryDialog::CreateControls()
{
    wxSize btnSize = wxSize(180,-1);

    wxStaticText* headerText = new wxStaticText(this, wxID_STATIC
        , _("Relocate all source categories to the destination category"));
    wxStaticLine* lineTop = new wxStaticLine(this, wxID_STATIC);

    m_buttonSource = new wxButton(this, wxID_CLEAR, _("Source Category"), wxDefaultPosition, wxSize(200, -1));
    Model_Category::Data* category = Model_Category::instance().get(m_sourceCatID);
    if (category)
        m_buttonSource->SetLabelText(Model_Category::full_name(m_sourceCatID, m_sourceSubCatID));

    m_buttonDest = new wxButton(this, wxID_NEW, _("Destination Category"), wxDefaultPosition, wxSize(200, -1));
    wxStaticLine* lineBottom = new wxStaticLine(this, wxID_STATIC);

    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(topSizer);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* request_sizer = new wxFlexGridSizer(0, 2, 0, 0);

    topSizer->Add(boxSizer, g_flagsV);

    boxSizer->Add(headerText, g_flagsV);
    boxSizer->Add(lineTop, g_flagsExpand);

    request_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Relocate:")), g_flagsH);
    request_sizer->Add(new wxStaticText(this, wxID_STATIC, _("to:")), g_flagsH);
    request_sizer->Add(m_buttonSource, g_flagsH);
    request_sizer->Add(m_buttonDest, g_flagsH);
    boxSizer->Add(request_sizer);

    boxSizer->Add(lineBottom, g_flagsExpand);

    wxButton* okButton = new wxButton(this, wxID_OK, _("&OK "));
    wxButton* cancelButton = new wxButton(this, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    cancelButton-> SetFocus();
    wxBoxSizer* buttonBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonBoxSizer->Add(okButton, g_flagsH);
    buttonBoxSizer->Add(cancelButton, g_flagsH);
    boxSizer->Add(buttonBoxSizer, g_flagsV);

    this->Fit();
}

void relocateCategoryDialog::OnSelectSource(wxCommandEvent& /*event*/)
{
    mmCategDialog sourceCat(this, true, false);
    sourceCat.setTreeSelection(m_sourceCatID,  m_sourceSubCatID);

    if (sourceCat.ShowModal() == wxID_OK)
    {
        m_sourceCatID    = sourceCat.getCategId();
        m_sourceSubCatID = sourceCat.getSubCategId();
        Model_Category::Data* category = Model_Category::instance().get(m_sourceCatID);
        Model_Subcategory::Data* sub_category = Model_Subcategory::instance().get(m_sourceSubCatID);

        m_buttonSource->SetLabelText(Model_Category::full_name(category, sub_category));
    }
    sourceCat.Destroy();
}

void relocateCategoryDialog::OnSelectDest(wxCommandEvent& /*event*/)
{
    mmCategDialog destCat(this, true, false);
    destCat.setTreeSelection(m_destCatID, m_destSubCatID);

    if (destCat.ShowModal() == wxID_OK)
    {
        m_destCatID    = destCat.getCategId();
        m_destSubCatID = destCat.getSubCategId();
        m_buttonDest->SetLabelText(Model_Category::full_name(m_destCatID, m_destSubCatID));
    }
}

int relocateCategoryDialog::updatedCategoriesCount() const
{
    return m_changedRecords;
}

void relocateCategoryDialog::OnOk(wxCommandEvent& /*event*/)
{
    if ((m_sourceCatID > 0) && (m_destCatID > 0) )
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

        wxString msgStr = wxString()
            <<_("Please Confirm:")
            << "\n\n"
            << wxString::Format(_("Records found in transactions: %i"), int(transactions.size())) << "\n"
            << wxString::Format(_("Records found in split transactions: %i"), int(checking_split.size())) << "\n"
            << wxString::Format(_("Records found in recurring transactions: %i"), int(billsdeposits.size())) << "\n"
            << wxString::Format(_("Records found in recurring split transactions: %i"), int(budget_split.size())) << "\n"
            << wxString::Format(_("Records found as Default Payee Category: %i"), int(payees.size())) << "\n"
            << wxString::Format(_("Records found in budget: %i"), int(budget.size())) << "\n\n"
            << wxString::Format(_("Changing all categories of: \n%s to category: %s")
                , m_buttonSource->GetLabelText(), m_buttonDest->GetLabelText());

        int ans = wxMessageBox(msgStr, _("Category Relocation Confirmation"), wxOK|wxCANCEL|wxICON_QUESTION);
        if (ans == wxOK)
        {
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
}
