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

#ifndef _MM_EX_NEWACCTDIALOG_H_
#define _MM_EX_NEWACCTDIALOG_H_

#include "model/Model_Account.h"

class mmNewAcctDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmNewAcctDialog )
    DECLARE_EVENT_TABLE()

public:
    mmNewAcctDialog();
    mmNewAcctDialog(Model_Account::Data* account, wxWindow* parent);
    ~mmNewAcctDialog();

    bool Create(wxWindow* parent
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);
    bool termAccountActivated();

private:
    void CreateControls();

    // utility functions
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnCurrency(wxCommandEvent& event);
    void fillControls(void);
    void OnCustonImage(wxCommandEvent& event);

    wxBitmapButton* bitmaps_button_;
    void OnImageButton(wxCommandEvent& event);

    void changeFocus(wxChildFocusEvent& event);
private:
    Model_Account::Data* m_account;
    int currencyID_;
    wxTextCtrl *textAccountName_;
    wxTextCtrl *notesCtrl_;
    bool termAccount_;
    wxString accessInfo_;
    bool access_changed_;
    wxString notesLabel_;
    wxColour notesColour_;
    wxImageList* imageList_;
};

#endif
