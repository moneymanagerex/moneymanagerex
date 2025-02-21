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

#ifndef MM_EX_NEWACCTDIALOG_H_
#define MM_EX_NEWACCTDIALOG_H_

#include "model/Model_Account.h"
#include "mmSimpleDialogs.h"
#include "mmTextCtrl.h"

#include <memory>

class mmNewAcctDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmNewAcctDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmNewAcctDialog();
    mmNewAcctDialog(Model_Account::Data* account, wxWindow* parent);
    ~mmNewAcctDialog();

    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = _t("New Account")
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX);

private:
    void CreateControls();

    // utility functions
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnCurrency(wxCommandEvent& event);
    void fillControls(void);
    void OnCustonImage(wxCommandEvent& event);
    void OnAttachments(wxCommandEvent& event);
    void OnAccountStatus(void);
    void OnAccountStatus(wxCommandEvent& event);

    void OnImageButton(wxCommandEvent& event);
    void OnChangeFocus(wxChildFocusEvent& event);

private:
    Model_Account::Data* m_account = nullptr;
    wxNotebook* m_notebook = nullptr;
    wxTextCtrl* m_textAccountName = nullptr;
    wxTextCtrl* m_notesCtrl = nullptr;
    mmTextCtrl* m_initbalance_ctrl = nullptr;
    mmDatePickerCtrl* m_initdate_ctrl = nullptr;

    wxVector<wxBitmapBundle> m_images;
    wxBitmapButton* m_bitmapButtons = nullptr;
    wxBitmapButton* bAttachments_ = nullptr;
    wxString m_accessInfo;

    wxCheckBox* m_statement_lock_ctrl = nullptr;
    mmDatePickerCtrl* m_statement_date_ctrl = nullptr;
    mmTextCtrl* m_minimum_balance_ctrl = nullptr;

    mmTextCtrl* m_credit_limit_ctrl = nullptr;
    mmTextCtrl* m_interest_rate_ctrl = nullptr;
    mmDatePickerCtrl* m_payment_due_date_ctrl = nullptr;
    mmTextCtrl* m_minimum_payment_ctrl = nullptr;

    int64 m_currencyID = 0;
    bool m_accessinfo_infocus = false;
};

#endif
