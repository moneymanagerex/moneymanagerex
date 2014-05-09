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

#ifndef MM_EX_NEWACCTDIALOG_H_
#define MM_EX_NEWACCTDIALOG_H_

#include "model/Model_Account.h"
#include "mmtextctrl.h"

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
    void OnTextEntered(wxCommandEvent& event);
	void OnAttachments(wxCommandEvent& event);

    void OnImageButton(wxCommandEvent& event);
    void changeFocus(wxChildFocusEvent& event);

private:
    Model_Account::Data* m_account;
    wxTextCtrl* m_textAccountName;
    wxTextCtrl* m_notesCtrl;
    mmTextCtrl* m_itemInitValue;
    wxImageList* m_imageList;
    wxBitmapButton* m_bitmapButtons;
	wxBitmapButton* bAttachments_;
    wxString m_accessInfo;
    wxString m_notesLabel;
    int m_currencyID;
    bool m_termAccount;
    bool m_accessChanged;

    enum {
        ID_DIALOG_NEWACCT_BUTTON_CURRENCY = wxID_HIGHEST + 1000,
        ID_DIALOG_NEWACCT_TEXTCTRL_ACCTNAME,
        ID_ACCTNUMBER,
        ID_DIALOG_NEWACCT_TEXTCTRL_HELDAT,
        ID_DIALOG_NEWACCT_TEXTCTRL_WEBSITE,
        ID_DIALOG_NEWACCT_TEXTCTRL_CONTACT,
        ID_DIALOG_NEWACCT_TEXTCTRL_ACCESSINFO,
        ID_DIALOG_NEWACCT_TEXTCTRL_NOTES,
        ID_DIALOG_NEWACCT_TEXTCTRL_INITBALANCE,
        ID_DIALOG_NEWACCT_COMBO_ACCTSTATUS,
        ID_DIALOG_NEWACCT_CHKBOX_FAVACCOUNT,
        ID_DIALOG_NEWACCT_COMBO_ACCTTYPE
    };

};

#endif
