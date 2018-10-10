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

#ifndef MM_EX_WIZARD_NEWDB_H_
#define MM_EX_WIZARD_NEWDB_H_

#include <wx/wizard.h>
#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include "option.h"

//----------------------------------------------------------------------------

class mmNewDatabaseWizard : public wxWizard
{
public:
    mmNewDatabaseWizard(wxFrame *frame);
    void RunIt(bool modal);

private:
    wxWizardPageSimple* page1;
    void OnWizardCancel(wxWizardEvent& event);

    wxDECLARE_EVENT_TABLE();
};
//----------------------------------------------------------------------------

class mmNewDatabaseWizardPage : public wxWizardPageSimple
{
public:
    mmNewDatabaseWizardPage(mmNewDatabaseWizard* parent);

    void OnCurrency(wxCommandEvent& WXUNUSED(event));
    virtual bool TransferDataFromWindow();

private:
    wxButton* itemButtonCurrency_;
    wxTextCtrl* itemUserName_;
    int currencyID_;

    wxDECLARE_EVENT_TABLE();
};

//----------------------------------------------------------------------------
#endif // MM_EX_WIZARD_NEWDB_H_
//----------------------------------------------------------------------------
