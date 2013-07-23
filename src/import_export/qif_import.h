
#ifndef QIF_IMPORT_H
#define QIF_IMPORT_H

#define SYMBOL_QIFIMPORT_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_QIFIMPORT_TITLE _("QIF Import")
#define SYMBOL_QIFIMPORT_IDNAME wxID_HIGHEST+1
#define SYMBOL_QIFIMPORT_SIZE wxSize(500, 300)
#define SYMBOL_QIFIMPORT_POSITION wxDefaultPosition

#include "mmcoredb.h"

class wxDatePickerCtrl;

class mmQIFImportDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmQIFImportDialog )
    DECLARE_EVENT_TABLE()

public:
    mmQIFImportDialog() {}

    mmQIFImportDialog(
        mmCoreDB* core,
        wxWindow* parent,
        wxWindowID id = SYMBOL_QIFIMPORT_IDNAME,
        const wxString& caption = SYMBOL_QIFIMPORT_TITLE,
        const wxPoint& pos = SYMBOL_QIFIMPORT_POSITION,
        const wxSize& size = SYMBOL_QIFIMPORT_SIZE,
        long style = SYMBOL_QIFIMPORT_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_QIFIMPORT_IDNAME,
                 const wxString& caption = SYMBOL_QIFIMPORT_TITLE,
                 const wxPoint& pos = SYMBOL_QIFIMPORT_POSITION,
                 const wxSize& size = SYMBOL_QIFIMPORT_SIZE,
                 long style = SYMBOL_QIFIMPORT_STYLE );
    int get_last_imported_acc()
    {
        return last_imported_acc_id_;
    }

private:
    int mmImportQIF();

    void CreateControls();
    void fillControls();

    void OnFileSearch(wxCommandEvent& event);
    bool checkQIFFile(wxString fileName);
    void OnCheckboxClick(wxCommandEvent& /*event*/);
    void OnDateMaskChange(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnOk(wxCommandEvent& /*event*/);

    mmCoreDB* core_;
    wxWindow* parent_;
    wxString dateFormat_;
    wxArrayInt accounts_id_;
    wxArrayInt items_index_;
    wxArrayString accounts_name_;

    bool isLineOK(const wxString& line);
    wxString getLineData(const wxString& line);
    wxString getFileLine(wxTextInputStream& textFile, int& lineNumber);
    wxString getFinancistoProject(wxString& sSubCateg);
    bool warning_message();
    wxString sFileName_;

    wxButton* button_search_;
    wxTextCtrl* file_name_ctrl_;
    wxCheckBox* dateFromCheckBox_;
    wxCheckBox* dateToCheckBox_;
    wxDatePickerCtrl* fromDateCtrl_;
    wxDatePickerCtrl* toDateCtrl_;
    wxComboBox* choiceDateFormat_;
    wxChoice* newAccounts_;
    wxButton* btnOK_;

    wxBitmapButton* bbFile_;
    wxBitmapButton* bbFormat_;
    wxBitmapButton* bbAccounts_;
    int last_imported_acc_id_;
};
#endif // 
