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

#define SYMBOL_NEWCHKGACCTDIALOG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_NEWCHKGACCTDIALOG_TITLE _("New Account")
#define SYMBOL_NEWCHKGACCTDIALOG_IDNAME ID_DIALOG_NEWACCOUNT
#define SYMBOL_NEWCHKGACCTDIALOG_SIZE wxSize(550, 300)
#define SYMBOL_NEWCHKGACCTDIALOG_POSITION wxDefaultPosition

#include "guiid.h"
#include "mmcoredb.h"

class mmNewAcctDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmNewAcctDialog )
    DECLARE_EVENT_TABLE()

public:
    mmNewAcctDialog();
    mmNewAcctDialog(mmCoreDB* core,
                    bool edit,
                    int acctId,
                    wxWindow* parent, wxWindowID id = SYMBOL_NEWCHKGACCTDIALOG_IDNAME,
                    const wxString& caption = SYMBOL_NEWCHKGACCTDIALOG_TITLE,
                    const wxPoint& pos = SYMBOL_NEWCHKGACCTDIALOG_POSITION,
                    const wxSize& size = SYMBOL_NEWCHKGACCTDIALOG_SIZE,
                    long style = SYMBOL_NEWCHKGACCTDIALOG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_NEWCHKGACCTDIALOG_IDNAME,
                 const wxString& caption = SYMBOL_NEWCHKGACCTDIALOG_TITLE,
                 const wxPoint& pos = SYMBOL_NEWCHKGACCTDIALOG_POSITION,
                 const wxSize& size = SYMBOL_NEWCHKGACCTDIALOG_SIZE,
                 long style = SYMBOL_NEWCHKGACCTDIALOG_STYLE );
    bool termAccountActivated();

private:
    void CreateControls();

    // utility functions
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnCurrency(wxCommandEvent& event);
    void fillControlsWithData(void);
    void OnCustonImage(wxCommandEvent& event);

    wxBitmapButton* bitmaps_button_;
    void OnImageButton(wxCommandEvent& event);

    void changeFocus(wxChildFocusEvent& event);
    mmCoreDB* core_;
    wxTextCtrl *textAccountName_;
    wxTextCtrl *notesCtrl_;
    bool edit_;
    int accountID_;
    int currencyID_;
    bool termAccount_;
    wxString accessInfo_;
    bool access_changed_;
    wxString notesLabel_;
    wxColour notesColour_;
};

#endif
