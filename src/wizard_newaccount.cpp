/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2022  Mark Whalley (mark@ipx.co.uk)

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

#include "wizard_newaccount.h"
#include "mmhomepagepanel.h"
#include "../resources/addacctwiz.xpm"

wxBEGIN_EVENT_TABLE(mmAddAccountNamePage, wxWizardPageSimple)
    EVT_WIZARD_PAGE_CHANGING(wxID_ANY, mmAddAccountNamePage::processPage)
wxEND_EVENT_TABLE()

//----------------------------------------------------------------------------

mmAddAccountWizard::mmAddAccountWizard(wxFrame *frame)
    : wxWizard(frame,wxID_ANY,_t("Add Account Wizard")
    , wxBitmap(addacctwiz_xpm), wxDefaultPosition
    , wxDEFAULT_DIALOG_STYLE)
{
    // a wizard page may be either an object of predefined class
    page1 = new wxWizardPageSimple(this);

    wxString noteString = wxString::Format(
        _t("%s models all transactions as belonging to accounts."), mmex::getProgramName()) + "\n\n"
        + _t("The next pages will help create a new account. "
            "To help get started, begin by making a list of all the "
            "financial institutions where the accounts are held.");

    new wxStaticText(page1, wxID_ANY, noteString);

    mmAddAccountTypePage* page2 = new mmAddAccountTypePage(this);
    mmAddAccountNamePage* page3 = new mmAddAccountNamePage(this);

    // set the page order using a convenience function - could also use
    // SetNext/Prev directly as below
    wxWizardPageSimple::Chain(page1, page2);
    wxWizardPageSimple::Chain(page2, page3);

    // allow the wizard to size itself around the pages
    GetPageAreaSizer()->Add(page1);
    this->CentreOnParent();
}

void mmAddAccountWizard::RunIt()
{
    if (RunWizard(page1)) {
        // Success
        Model_Account::Data* account = Model_Account::instance().create();

        account->FAVORITEACCT = "TRUE";
        account->STATUS = Model_Account::STATUS_NAME_OPEN;
        account->ACCOUNTTYPE = Model_Account::type_name(accountType_);
        account->ACCOUNTNAME = accountName_;
        account->INITIALBAL = 0;
        account->INITIALDATE = wxDate::Today().FormatISODate();
        account->CURRENCYID = currencyID_;

        Model_Account::instance().save(account);
        acctID_ = account->ACCOUNTID;
    }
    Destroy();
}

void mmAddAccountNamePage::processPage(wxWizardEvent& event)
{
    const wxString account_name = textAccountName_->GetValue().Trim();
    parent_->accountName_ = account_name;
    if (event.GetDirection())
    {
        if ( account_name.IsEmpty())
        {
            wxMessageBox(_t("Account Name Invalid"), _t("New Account"), wxOK|wxICON_ERROR, this);
            event.Veto();
        }
        else
        {
            if (Model_Account::instance().get(account_name))
            {
                wxMessageBox(_t("An account with this name already exists"), _t("New Account"), wxOK|wxICON_ERROR, this);
                event.Veto();
            }
        }
    }
}

mmAddAccountNamePage::mmAddAccountNamePage(mmAddAccountWizard* parent)
    : wxWizardPageSimple(parent), parent_(parent)
{
    textAccountName_ = new wxTextCtrl(this, wxID_ANY);
    textAccountName_->SetMinSize(wxSize(200,-1));

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, _t("Name of the Account")), 0, wxALL, 5 );
    mainSizer->Add( textAccountName_, 0 /* No stretching */, wxALL, 5 /* Border Size */);

    wxString helpMsg;
    helpMsg  << "\n" << _t("Specify a descriptive name for the account.") << "\n"
            << _t("This is generally the name of a financial institution\n"
            "where the account is held. For example: 'ABC Bank'.");
    mainSizer->Add(new wxStaticText(this, wxID_ANY, helpMsg ), 0, wxALL, 5);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

mmAddAccountTypePage::mmAddAccountTypePage(mmAddAccountWizard *parent)
    : wxWizardPageSimple(parent)
    , parent_(parent)
{
    itemChoiceType_ = new wxChoice(this, wxID_ANY);
    for (int i = 0; i < Model_Account::TYPE_ID_size; ++i) {
        wxString type = Model_Account::type_name(i);
        itemChoiceType_->Append(wxGetTranslation(type), new wxStringClientData(type));
    }
    mmToolTip(itemChoiceType_, _t("Specify the type of account to be created."));
    itemChoiceType_->SetSelection(Model_Account::TYPE_ID_CHECKING);

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    mainSizer->Add( new wxStaticText(this, wxID_ANY, _t("Type of Account")), 0, wxALL, 5 );
    mainSizer->Add( itemChoiceType_, 0 /* No stretching*/, wxALL, 5 /* Border Size */);

    wxString textMsg = "\n";
    textMsg << _t("Select the type of account to create:") << "\n\n"
            << _t("General bank accounts cover a wide variety of account\n"
            "types like Cash, Checking, Loans, and Credit cards.");
    mainSizer->Add(new wxStaticText(this, wxID_ANY, textMsg), 0, wxALL, 5);

    textMsg = "\n";
    textMsg << _t("Investment and Share accounts are specialized accounts that\n"
        "have stock/mutual fund investments associated with them.");
    mainSizer->Add( new wxStaticText(this, wxID_ANY,textMsg), 0, wxALL, 5);

    textMsg = "\n";
    textMsg << _t("Term and asset accounts are specialized bank accounts. "
        "They are intended for monitoring assets or term deposits and bonds "
        "where typically regular money goes in and comes out, outside "
        "the general income stream.");
    mainSizer->Add( new wxStaticText(this, wxID_ANY,textMsg), 0, wxALL, 5);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

bool mmAddAccountTypePage::TransferDataFromWindow()
{
    int64 currencyID = Option::instance().getBaseCurrencyID();
    if (currencyID == -1)
    {
        wxString errorMsg;
        errorMsg << _t("Base Account Currency Not set.") << "\n"
                 << _tu("Set that first using Tools → Settings… menu and then add a new account.");
        wxMessageBox( errorMsg, _t("New Account"), wxOK|wxICON_WARNING, this);
        return false;
    }

    parent_->currencyID_ = currencyID;
    parent_->accountType_ = itemChoiceType_->GetSelection();

    return true;
}
