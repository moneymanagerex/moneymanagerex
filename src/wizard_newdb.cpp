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

#include "wizard_newdb.h"
#include "maincurrencydialog.h"
#include "model/Model_Account.h"
#include "../resources/addacctwiz.xpm"
//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(mmNewDatabaseWizard, wxWizard)
    EVT_WIZARD_CANCEL(wxID_ANY, mmNewDatabaseWizard::OnWizardCancel)
END_EVENT_TABLE()

mmNewDatabaseWizard::mmNewDatabaseWizard(wxFrame *frame)
    : wxWizard(frame, wxID_ANY, _("New Database Wizard")
    , wxBitmap(addacctwiz_xpm), wxDefaultPosition, wxDEFAULT_DIALOG_STYLE)
{
/****************** Message to be displayed******************

    The next pages will help you create a new database.

    Your database file is stored with an extension of .mmb.

    As this file contains important financial information,
    we recommended creating daily backups with the Options
    setting: 'Backup before opening', and store your backups
    in a separate location.

    The database can later be encrypted if required, by
    using the option: 'Save database as' and changing the
    file type before saving.
*/
    page1 = new wxWizardPageSimple(this);
    wxString displayMsg;
    displayMsg << _("The next pages will help you create a new database.") << "\n\n"
               << _("Your database file is stored with an extension of .mmb.")<< "\n\n"
               << _("As this file contains important financial information,\nwe recommended creating daily backups with the Options\nsetting: 'Backup before opening', and store your backups\nin a separate location.")<< "\n\n"
               << _("The database can later be encrypted if required, by\nusing the option: 'Save database as' and changing the\nfile type before saving.");
    new wxStaticText(page1, wxID_ANY, displayMsg);

    mmNewDatabaseWizardPage* page2 = new mmNewDatabaseWizardPage(this);

    // set the page order using a convenience function - could also use
    // SetNext/Prev directly as below
    wxWizardPageSimple::Chain(page1, page2);

    // allow the wizard to size itself around the pages
    GetPageAreaSizer()->Add(page1);

    /*wxButton* back = (wxButton*) FindWindow(wxID_BACKWARD);
    if (back) back->SetLabel(_("<- &Back"));
    wxButton* next = (wxButton*) FindWindow(wxID_FORWARD); //FIXME: 
    if (next) next->SetLabel(_("&Next ->"));
    wxButton* ca = (wxButton*) FindWindow(wxID_CANCEL);
    if (ca) ca->SetLabel(_("&Cancel "));*/
}

void mmNewDatabaseWizard::RunIt(bool modal)
{
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
}

void mmNewDatabaseWizard::OnWizardCancel(wxWizardEvent& event)
{
    event.Veto();
}

//----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(mmNewDatabaseWizardPage, wxWizardPageSimple)
    EVT_BUTTON(wxID_ANY, mmNewDatabaseWizardPage::OnCurrency)
END_EVENT_TABLE()

mmNewDatabaseWizardPage::mmNewDatabaseWizardPage(mmNewDatabaseWizard* parent)
    : wxWizardPageSimple(parent)
    , parent_(parent)
    , currencyID_(-1)
{
    wxString currName = _("Set Currency");
    const auto base_currency = Model_Currency::instance().GetBaseCurrency();
    if (base_currency)
    {
        currencyID_ = base_currency->CURRENCYID;
        currName = base_currency->CURRENCYNAME;
    }

    itemButtonCurrency_ = new wxButton(this, wxID_ANY, currName, wxDefaultPosition, wxSize(220, -1), 0);

    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Base Currency for account")), 0, wxALL, 5);
    mainSizer->Add(itemButtonCurrency_, 0 /* No stretching */, wxALL, 5 /* Border size */);

    wxString helpMsg;
/**************************Message to be displayed *************
    Specify the base (or default) currency to be used for the
    database. The base currency can later be changed by using
    the options dialog. New accounts, will use this currency by
    default, and can be changed when editing account details.
***************************************************************/
    helpMsg << _("Specify the base (or default) currency to be used for the\ndatabase. The base currency can later be changed by using\nthe options dialog. New accounts, will use this currency by\ndefault, and can be changed when editing account details.")
            << "\n";
    mainSizer->Add( new wxStaticText(this, wxID_ANY, helpMsg), 0, wxALL, 5);


    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(itemBoxSizer5, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText(this, wxID_STATIC, _("User Name"));
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    itemUserName_ = new wxTextCtrl(this, wxID_ANY);
    itemBoxSizer5->Add(itemUserName_, 1, wxALIGN_CENTER_VERTICAL | wxGROW | wxALL, 5);

    helpMsg.Empty();
    helpMsg << _("(Optional) Specify a title or your name.") << "\n"
            << _("Used as a database title for displayed and printed reports.");
    mainSizer->Add(new wxStaticText(this, wxID_ANY, helpMsg), 0, wxALL, 5);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
}

bool mmNewDatabaseWizardPage::TransferDataFromWindow()
{
    if ( currencyID_ == -1)
    {
        wxMessageBox(_("Base Currency Not Set"), _("New Database"), wxOK | wxICON_WARNING, this);
        return false;
    }
    wxString userName = itemUserName_->GetValue().Trim();
    Model_Infotable::instance().Set("USERNAME", userName);
    mmOptions::instance().userNameString_ = userName;

    return true;
}

void mmNewDatabaseWizardPage::OnCurrency(wxCommandEvent& /*event*/)
{
    currencyID_ = Model_Infotable::instance().GetBaseCurrencyId();

    if (mmMainCurrencyDialog::Execute(this, currencyID_) && currencyID_ != -1)
    {
        Model_Currency::Data* currency = Model_Currency::instance().get(currencyID_);
        if (currency)
        {
            itemButtonCurrency_->SetLabelText(currency->CURRENCYNAME);
            Model_Currency::instance().SetBaseCurrency(currency);
            currencyID_ = currency->CURRENCYID;
        }
    }
}
//----------------------------------------------------------------------------
