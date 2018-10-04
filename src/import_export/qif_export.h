/*******************************************************
Copyright (C) 2012 Nikolay Akimov

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

#ifndef QIF_EXPORT_H
#define QIF_EXPORT_H

#include "defs.h"

class wxDatePickerCtrl;

class mmQIFExportDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmQIFExportDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmQIFExportDialog() {}
    //virtual ~mmQIFExportDialog() {}

    mmQIFExportDialog(wxWindow* parent /*, int gotoAccountID*/);

    bool Create(wxWindow* parent
        , wxWindowID id
        , const wxString&
        , const wxPoint& pos
        , const wxSize& size
        , long style);

private:
    void mmExportQIF();
    void OnAccountsButton(wxCommandEvent& WXUNUSED(event));
    void OnCheckboxClick(wxCommandEvent& WXUNUSED(event));
    void OnFileNameEntered(wxCommandEvent& event);
    void OnFileNameChanged(wxCommandEvent& event);
    void OnFileSearch(wxCommandEvent& event);
    void OnButtonClear(wxCommandEvent& WXUNUSED(event));
    void CreateControls();
    void fillControls();
    void OnQuit(wxCloseEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnOk(wxCommandEvent& WXUNUSED(event));


    wxCheckBox* cCategs_;
    wxCheckBox* accountsCheckBox_;
    wxButton* bSelectedAccounts_;
    wxCheckBox* dateFromCheckBox_;
    wxCheckBox* dateToCheckBox_;
    wxDatePickerCtrl* fromDateCtrl_;
    wxDatePickerCtrl* toDateCtrl_;
    wxComboBox* m_choiceDateFormat;
    wxCheckBox* toFileCheckBox_;
    wxStaticText* file_name_label_;
    wxButton* button_search_;
    wxTextCtrl* m_text_ctrl_;
    wxTextCtrl* log_field_;
    //wxLog *logger_;
    wxRadioBox* m_radio_box_deprecated;
    wxString delimit_deprecated_;

    /* Selected accounts values */
    wxArrayString m_accounts_name;
    wxArrayInt accounts_id_;
    wxArrayInt selected_accounts_id_;
};
#endif // 
