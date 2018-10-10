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

#include "wizard_newaccount.h"
#include "mmhomepagepanel.h"
#include "../resources/addacctwiz.xpm"
#include "option.h"
//----------------------------------------------------------------------------

mmAddAccountWizard::mmAddAccountWizard(wxFrame *frame)
    : wxWizard(frame,wxID_ANY,_("Add Account Wizard")
    , wxBitmap(addacctwiz_xpm), wxDefaultPosition
    , wxDEFAULT_DIALOG_STYLE), currencyID_(-1)
    , accountType_(0), acctID_(-1)
{
    // a wizard page may be either an object of predefined class
    page1 = new wxWizardPageSimple(this);

    wxString noteString = wxString::Format(
        _("%s models all transactions as belonging to accounts."), mmex::getProgramName()) + "\n\n"
        + _("The next pages will help you create a new account.\n"
            "To help you get started, begin by making a list of all\n"
            "financial institutions where you hold an account.");

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
        account->STATUS = Model_Account::all_status()[Model_Account::OPEN];
        account->ACCOUNTTYPE = Model_Account::all_type()[accountType_];
        account->ACCOUNTNAME = accountName_;
        account->INITIALBAL = 0;
        account->CURRENCYID = currencyID_;

        Model_Account::instance().save(account);
        acctID_ = account->ACCOUNTID;
    }
    Destroy();
}

bool mmAddAccountNamePage::TransferDataFromWindow()
{
    bool result = true;
    const wxString account_name = textAccountName_->GetValue().Trim();
    if ( account_name.IsEmpty())
    {
        wxMessageBox(_("Account Name Invalid"), _("New Account"), wxOK|wxICON_ERROR, this);
        result = false;
    }
    else
    {
        if (Model_Account::instance().get(account_name))
        {
            wxMessageBox(_("Account Name already exists"), _("New Account"), wxOK|wxICON_ERROR, this);
            result = false;
        }
    }
    parent_->accountName_ = account_name;
    return result;
}

mmAddAccountNamePage::mmAddAccountNamePage(mmAddAccountWizard* parent)
    : wxWizardPageSimple(parent), parent_(parent)
{
    textAccountName_ = new wxTextCtrl(this, wxID_ANY, wxGetEmptyString(), wxDefaultPosition, wxSize(130,-1), 0 );

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Name of the Account")), 0, wxALL, 5 );
    mainSizer->Add( textAccountName_, 0 /* No stretching */, wxALL, 5 /* Border Size */);

    wxString helpMsg;
    helpMsg  << "\n" << _("Specify a descriptive name for the account.") << "\n"
            << _("This is generally the name of a financial institution\n"
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
    for (const auto& type: Model_Account::all_type())
        itemChoiceType_->Append(wxGetTranslation(type), new wxStringClientData(type));
    itemChoiceType_->SetToolTip(_("Specify the type of account to be created."));
    itemChoiceType_->SetSelection(Model_Account::CHECKING);

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    mainSizer->Add( new wxStaticText(this, wxID_ANY, _("Type of Account")), 0, wxALL, 5 );
    mainSizer->Add( itemChoiceType_, 0 /* No stretching*/, wxALL, 5 /* Border Size */);

    wxString textMsg = "\n";
    textMsg << _("Select the type of account you want to create:") << "\n\n"
            << _("General bank accounts cover a wide variety of account\n"
            "types like Checking, Savings etc.");
    mainSizer->Add(new wxStaticText(this, wxID_ANY, textMsg), 0, wxALL, 5);

    textMsg = "\n";
    textMsg << _("Investment accounts are specialized accounts that only\n"
        "have stock/mutual fund investments associated with them.");
    mainSizer->Add( new wxStaticText(this, wxID_ANY,textMsg), 0, wxALL, 5);

    textMsg = "\n";
    textMsg << _("Term accounts are specialized bank accounts. Intended for asset\n"
        "type accounts such as Term Deposits and Bonds. These accounts\n"
        "can have regular money coming in and out, being outside the\n"
        "general income stream.");
    mainSizer->Add( new wxStaticText(this, wxID_ANY,textMsg), 0, wxALL, 5);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

bool mmAddAccountTypePage::TransferDataFromWindow()
{
    int currencyID = Option::instance().BaseCurrency();
    if (currencyID == -1)
    {
        wxString errorMsg;
        errorMsg << _("Base Account Currency Not set.") << "\n"
                 << _("Set that first using Tools->Options menu and then add a new account.");
        wxMessageBox( errorMsg, _("New Account"), wxOK|wxICON_WARNING, this);
        return false;
    }

    parent_->currencyID_ = currencyID;
    parent_->accountType_ = itemChoiceType_->GetSelection();

    return true;
}
