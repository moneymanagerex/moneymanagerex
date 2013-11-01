
#ifndef QIF_IMPORT_H
#define QIF_IMPORT_H

#define SYMBOL_QIFIMPORT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_QIFIMPORT_TITLE _("QIF Import")
#define SYMBOL_QIFIMPORT_IDNAME wxID_HIGHEST+1
#define SYMBOL_QIFIMPORT_SIZE wxSize(500, 300)
#define SYMBOL_QIFIMPORT_POSITION wxDefaultPosition

#include "defs.h"
#include <wx/dataview.h>
#include "model/Model_Checking.h"

class wxDatePickerCtrl;

class mmQIFImportDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmQIFImportDialog )
    DECLARE_EVENT_TABLE()

public:
    mmQIFImportDialog() {}

    mmQIFImportDialog(
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
    int mmImportQIF(wxTextFile& tFile);

    void CreateControls();
    void fillControls();

    void OnFileSearch(wxCommandEvent& event);
    bool checkQIFFile(wxTextFile& tFile);
    void OnCheckboxClick(wxCommandEvent& /*event*/);
    void OnDateMaskChange(wxCommandEvent& event);
    void OnButtonClear(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnOk(wxCommandEvent& /*event*/);

    std::vector<Model_Checking::Data*> vQIF_trxs_;
    wxWindow* parent_;
    wxString dateFormat_;
    wxArrayInt accounts_id_;
    wxArrayInt items_index_;
    wxArrayString accounts_name_;
    int fromAccountID_;

    bool isLineOK(const wxString& line);
    wxString getLineData(const wxString& line);
    wxString getFileLine(wxTextInputStream& textFile, int& lineNumber);
    wxString getFinancistoProject(wxString& sSubCateg);
    bool warning_message();
    wxString sFileName_;

    wxDataViewListCtrl* dataListBox_;
    wxButton* button_search_;
    wxTextCtrl* file_name_ctrl_;
    wxTextCtrl* log_field_;
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

    enum EColumn
    {
        COL_ACCOUNT = 0,
        COL_DATE,
        COL_NUMBER,
        COL_PAYEE,
        COL_STATUS,
        COL_CATEGORY,
        COL_VALUE,
        COL_NOTES,
        COL_MAX, // number of columns
    };
    std::map<int, wxString> ColName_;
};
#endif // 
