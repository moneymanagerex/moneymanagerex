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

#ifndef _MM_EX_WIZARD_NEWACCOUNT_H_
#define _MM_EX_WIZARD_NEWACCOUNT_H_

#include <wx/wizard.h>
#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
//----------------------------------------------------------------------------

class mmAddAccountWizard : public wxWizard
{
public:
    mmAddAccountWizard(wxFrame *frame);
    void RunIt(bool modal);
    wxString accountName_;

    int acctID_;

private:
    wxWizardPageSimple* page1;
};
//----------------------------------------------------------------------------

class mmAddAccountPage1 : public wxWizardPageSimple
{
public:
    mmAddAccountPage1(mmAddAccountWizard* parent);
    virtual bool TransferDataFromWindow();

private:
    mmAddAccountWizard* parent_;
    wxTextCtrl* textAccountName_;
};
//----------------------------------------------------------------------------

class mmAddAccountPage2 : public wxWizardPageSimple
{
public:
    mmAddAccountPage2(mmAddAccountWizard *parent);
    virtual bool TransferDataFromWindow();

private:
    wxChoice* itemChoiceType_;
    mmAddAccountWizard* parent_;
};

//----------------------------------------------------------------------------
#endif // _MM_EX_WIZARD_NEWACCOUNT_H_
//----------------------------------------------------------------------------
