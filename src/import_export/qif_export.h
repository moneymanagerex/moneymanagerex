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

class mmDatePickerCtrl;

class mmQIFExportDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmQIFExportDialog);
    wxDECLARE_EVENT_TABLE();

public:
    enum type { CSV = 0, JSON, QIF };
    mmQIFExportDialog() {}
    //virtual ~mmQIFExportDialog() {}

    mmQIFExportDialog(wxWindow* parent, int type, int account_id);

    bool Create(wxWindow* parent
        , const wxString& caption
        , wxWindowID id = wxID_ANY
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

private:

    int m_type = type::CSV;
    int m_account_id = -1;
    void mmExportQIF();
    void OnAccountsButton(wxCommandEvent& WXUNUSED(event));
    void OnCheckboxClick(wxCommandEvent& WXUNUSED(event));
    void OnChoiceType(wxCommandEvent& event);
    void OnFileNameEntered(wxCommandEvent& event);
    void OnFileNameChanged(wxCommandEvent& event);
    void OnFileSearch(wxCommandEvent& event);
    void OnButtonClear(wxCommandEvent& WXUNUSED(event));
    void CreateControls();
    void fillControls();
    void OnQuit(wxCloseEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnOk(wxCommandEvent& WXUNUSED(event));


    wxCheckBox* cCategs_ = nullptr;
    wxCheckBox* accountsCheckBox_ = nullptr;
    wxButton* bSelectedAccounts_ = nullptr;
    wxCheckBox* dateFromCheckBox_ = nullptr;
    wxCheckBox* dateToCheckBox_ = nullptr;
    mmDatePickerCtrl* fromDateCtrl_ = nullptr;
    mmDatePickerCtrl* toDateCtrl_ = nullptr;
    wxStaticText* choiceDateFormat_label_ = nullptr;
    wxComboBox* m_choiceDateFormat = nullptr;
    wxCheckBox* toFileCheckBox_ = nullptr;
    wxStaticText* file_name_label_ = nullptr;
    wxButton* button_search_ = nullptr;
    wxTextCtrl* m_text_ctrl_ = nullptr;
    wxTextCtrl* log_field_ = nullptr;
    //wxLog *logger_;

    /* Selected accounts values */
    wxArrayString m_accounts_name;
    wxArrayInt accounts_id_;
    wxArrayInt selected_accounts_id_;
};
#endif
