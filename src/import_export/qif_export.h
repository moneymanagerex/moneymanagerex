/*******************************************************
Copyright (C) 2012 Nikolay

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

#define SYMBOL_QIFDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_QIFDIALOG_TITLE _("QIF Export")
#define SYMBOL_QIFDIALOG_IDNAME wxID_HIGHEST+1
#define SYMBOL_QIFDIALOG_SIZE wxSize(500, 300)
#define SYMBOL_QIFDIALOG_POSITION wxDefaultPosition

#include "defs.h"

class wxDatePickerCtrl;

class mmQIFExportDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmQIFExportDialog )
    DECLARE_EVENT_TABLE()

public:
    mmQIFExportDialog() {}

    mmQIFExportDialog(
        wxWindow* parent,
        wxWindowID id = SYMBOL_QIFDIALOG_IDNAME,
        const wxString& caption = SYMBOL_QIFDIALOG_TITLE,
        const wxPoint& pos = SYMBOL_QIFDIALOG_POSITION,
        const wxSize& size = SYMBOL_QIFDIALOG_SIZE,
        long style = SYMBOL_QIFDIALOG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_QIFDIALOG_IDNAME,
                 const wxString& caption = SYMBOL_QIFDIALOG_TITLE,
                 const wxPoint& pos = SYMBOL_QIFDIALOG_POSITION,
                 const wxSize& size = SYMBOL_QIFDIALOG_SIZE,
                 long style = SYMBOL_QIFDIALOG_STYLE );

private:
    void OnQuit(wxCloseEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnOk(wxCommandEvent& /*event*/);
    void mmExportQIF();
    void OnAccountsButton(wxCommandEvent& /*event*/);
    void OnCheckboxClick(wxCommandEvent& /*event*/);
    void OnFileNameEntered(wxCommandEvent& event);
    void OnFileNameChanged(wxCommandEvent& event);
    void OnFileSearch(wxCommandEvent& event);
    void OnButtonClear(wxCommandEvent& /*event*/);
    void CreateControls();
    void fillControls();

    wxWindow* parent_;
    wxArrayInt accounts_id_;
    /* Selected accounts id */
    wxArrayInt selected_accounts_id_;

    wxCheckBox* cCategs_;
    wxCheckBox* accountsCheckBox_;
    wxButton* bSelectedAccounts_;
    wxCheckBox* dateFromCheckBox_;
    wxCheckBox* dateToCheckBox_;
    wxDatePickerCtrl* fromDateCtrl_;
    wxDatePickerCtrl* toDateCtrl_;
    wxCheckBox* toFileCheckBox_;
    wxStaticText* file_name_label_;
    wxButton* button_search_;
    wxTextCtrl* m_text_ctrl_;
    wxTextCtrl* log_field_;
    //wxLog *logger_;
    wxRadioBox* m_radio_box_;
    wxString delimit_;
    wxArrayString accounts_name_;
};
#endif // 
