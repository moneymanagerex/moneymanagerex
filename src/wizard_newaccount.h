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

#ifndef MM_EX_WIZARD_NEWACCOUNT_H_
#define MM_EX_WIZARD_NEWACCOUNT_H_

#include <wx/wizard.h>
#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
//----------------------------------------------------------------------------

typedef wxLongLong int64;

class mmAddAccountWizard : public wxWizard
{
public:
    mmAddAccountWizard(wxFrame *frame);
    void RunIt();
    wxString accountName_;
    int64 currencyID_ = -1;
    int accountType_ = 0;

    int64 acctID_ = -1;

private:
    wxWizardPageSimple* page1;
};
//----------------------------------------------------------------------------

class mmAddAccountNamePage : public wxWizardPageSimple
{
public:
    mmAddAccountNamePage(mmAddAccountWizard* parent);
    void processPage(wxWizardEvent& event);

private:
    mmAddAccountWizard* parent_ = nullptr;
    wxTextCtrl* textAccountName_ = nullptr;

    wxDECLARE_EVENT_TABLE();
};
//----------------------------------------------------------------------------

class mmAddAccountTypePage : public wxWizardPageSimple
{
public:
    mmAddAccountTypePage(mmAddAccountWizard *parent);
    virtual bool TransferDataFromWindow();

private:
    wxChoice* itemChoiceType_ = nullptr;
    mmAddAccountWizard* parent_ = nullptr;
};

//----------------------------------------------------------------------------
#endif // MM_EX_WIZARD_NEWACCOUNT_H_
//----------------------------------------------------------------------------
