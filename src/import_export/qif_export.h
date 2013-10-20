/*******************************************************
  Copyright (C) 2012 Nikolay
 ********************************************************/
#ifndef QIF_EXPORT_H
#define QIF_EXPORT_H

#define SYMBOL_QIFDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_QIFDIALOG_TITLE _("QIF Export")
#define SYMBOL_QIFDIALOG_IDNAME wxID_HIGHEST+1
#define SYMBOL_QIFDIALOG_SIZE wxSize(500, 300)
#define SYMBOL_QIFDIALOG_POSITION wxDefaultPosition

#include "mmcoredb.h"

class wxDatePickerCtrl;

class mmQIFExportDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmQIFExportDialog )
    DECLARE_EVENT_TABLE()

public:
    mmQIFExportDialog() {}

    mmQIFExportDialog(
        mmCoreDB* core,
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

    mmCoreDB* core_; // TODO
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

    void mmExportQIF(wxWindow* parent_, mmCoreDB* core);
};
#endif // 
