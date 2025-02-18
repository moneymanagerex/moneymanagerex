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

#include "constants.h"
#include "wizard_newdb.h"
#include "maincurrencydialog.h"
#include "model/Model_Account.h"
#include "../resources/addacctwiz.xpm"

BEGIN_EVENT_TABLE(mmNewDatabaseWizard, wxWizard)
    EVT_WIZARD_CANCEL(wxID_ANY, mmNewDatabaseWizard::OnCancel)
END_EVENT_TABLE()

mmNewDatabaseWizard::mmNewDatabaseWizard(wxFrame *frame)
    : wxWizard(frame, wxID_ANY, _("New Database Wizard")
        , wxBitmap(addacctwiz_xpm), wxDefaultPosition, wxDEFAULT_DIALOG_STYLE)
{
    page1 = new wxWizardPageSimple(this);
    wxString displayMsg;
    displayMsg << _tu(
        "The next pages will help create a new database.\n\n"
        "The database file is stored with an extension of .mmb. "
        "As this file contains important financial data, "
        "it is recommended to create daily backups with the setting "
        "“Backup database on startup” and store the backups "
        "in a separate location.\n\n"
        "The database can later be encrypted if required by "
        "using the setting: “Save Database As…” and changing the "
        "filetype before saving.");
    new wxStaticText(page1, wxID_ANY, displayMsg);

    mmNewDatabaseWizardPage* page2 = new mmNewDatabaseWizardPage(this);

    // set the page order using a convenience function - could also use
    // SetNext/Prev directly as below
    wxWizardPageSimple::Chain(page1, page2);

    // allow the wizard to size itself around the pages
    GetPageAreaSizer()->Add(page1);
}

bool mmNewDatabaseWizard::RunIt(bool modal)
{
    success_ = true;
    if (modal)
    {
        if (RunWizard(page1))
        {
            // Success
        }

        Destroy();
    }
    else
    {
        FinishLayout();
        ShowPage(page1);
        Show(true);
    }
    return success_;
}

void mmNewDatabaseWizard::OnCancel(wxWizardEvent&)
{
    success_ = false;
}

BEGIN_EVENT_TABLE(mmNewDatabaseWizardPage, wxWizardPageSimple)
    EVT_BUTTON(wxID_ANY, mmNewDatabaseWizardPage::OnCurrency)
END_EVENT_TABLE()

mmNewDatabaseWizardPage::mmNewDatabaseWizardPage(mmNewDatabaseWizard* parent)
    : wxWizardPageSimple(parent)
    , parent_(parent)
{
    wxString currName = _("Set Currency");
    const auto base_currency = Model_Currency::instance().GetBaseCurrency();
    if (base_currency)
    {
        currencyID_ = base_currency->CURRENCYID;
        currName = base_currency->CURRENCYNAME;
        Option::instance().setBaseCurrencyID(currencyID_);
    }

    itemButtonCurrency_ = new wxButton(this, wxID_ANY, currName, wxDefaultPosition, wxSize(220, -1), 0);

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Base Currency for account")), 0, wxALL, 5);
    mainSizer->Add(itemButtonCurrency_, 0 /* No stretching */, wxALL, 5 /* Border size */);

    wxString helpMsg = _("Specify the base (or default) currency to be used for the\n"
        "database. The base currency can later be changed in\n"
        "Options. New accounts, will use this currency by\n"
        "default, and can be changed when editing account details.");
    helpMsg += "\n";
    mainSizer->Add(new wxStaticText(this, wxID_ANY, helpMsg), 0, wxALL, 5);


    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(itemBoxSizer5, 0, wxALIGN_LEFT | wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText(this, wxID_STATIC, _("User Name"));
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    itemUserName_ = new wxTextCtrl(this, wxID_ANY);
    itemUserName_->SetMinSize(wxSize(200,-1));
    itemBoxSizer5->Add(itemUserName_, g_flagsExpand);

    helpMsg = _("(Optional) Specify a title or name.") + "\n";
    helpMsg += _("Used as a database title for displayed and printed reports.");
    mainSizer->Add(new wxStaticText(this, wxID_ANY, helpMsg), 0, wxALL, 5);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

bool mmNewDatabaseWizardPage::TransferDataFromWindow()
{
    if (currencyID_ == -1)
    {
        wxMessageBox(_("Base Currency Not Set"), _("New Database"), wxOK | wxICON_WARNING, this);
        return false;
    }
    wxString userName = itemUserName_->GetValue().Trim();
    Option::instance().setUserName(userName);

    return true;
}

void mmNewDatabaseWizardPage::OnCurrency(wxCommandEvent& /*event*/)
{
    while (true)
    {
        currencyID_ = Option::instance().getBaseCurrencyID();
        mmMainCurrencyDialog::Execute(this, currencyID_);
        Model_Currency::Data* currency = Model_Currency::instance().get(currencyID_);
        if (currency)
        {
            itemButtonCurrency_->SetLabelText(wxGetTranslation(currency->CURRENCYNAME));
            currencyID_ = currency->CURRENCYID;
            Option::instance().setBaseCurrencyID(currencyID_);
            break;
        }
    }
}
