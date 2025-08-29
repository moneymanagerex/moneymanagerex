/*******************************************************
Copyright (C) 2013-2016 Nikolay Akimov

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

#pragma once

#include <wx/dialog.h>
#include "Model_Checking.h"
#include "mmSimpleDialogs.h"

class mmDatePickerCtrl;
class wxDataViewListCtrl;
class mmQIFImport;
class wxButton;
class wxTextCtrl;
class wxChoice;
class wxCheckBox;
class wxComboBox;

class mmQIFImportDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmQIFImportDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmQIFImportDialog()
    {
    }
    mmQIFImportDialog(wxWindow* parent, int64 account_id, const wxString& file_path = wxEmptyString);

    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& caption = _t("Import from QIF file"), const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize, long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

    wxString OnGetItemText(long item, long column) const;
    int64 get_last_imported_acc();

private:
    mmQIFImport* qif_api = nullptr;
    void CreateControls();
    void fillControls();
    void OnFileSearch(wxCommandEvent& event);
    void OnCheckboxClick(wxCommandEvent& WXUNUSED(event));
    void OnAccountChanged(wxCommandEvent& event);
    void OnDateMaskChange(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);
    void OnCancel(wxCommandEvent& WXUNUSED(event));
    void OnOk(wxCommandEvent& WXUNUSED(event));
    void OnDecimalChange(wxCommandEvent& event);
    void OnFileNameChanged(wxCommandEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    void OnShowPayeeDialog(wxMouseEvent&);
    void OnShowCategDialog(wxMouseEvent&);
    void save_file_name();
    bool mmReadQIFFile();
    int64 getOrCreateAccounts();
    void getOrCreatePayees();
    void getOrCreateCategories();
    bool completeTransaction(std::unordered_map<int, wxString>& trx, const wxString& accName);
    bool completeTransaction(/*in*/ const std::unordered_map<int, wxString>& i, /*out*/ Model_Checking::Data* trx, wxString& msg);
    bool mergeTransferPair(Model_Checking::Cache& to, Model_Checking::Cache& from);
    void appendTransfers(Model_Checking::Cache& destination, Model_Checking::Cache& target);
    void joinSplit(Model_Checking::Cache& destination, std::vector<Model_Splittransaction::Cache>& target);
    void saveSplit();
    void refreshTabs(int tabs);
    void compilePayeeRegEx();
    void validatePayees();

    // QIF paragraphs represented like maps type = data
    std::vector<std::unordered_map<int, wxString>> vQIF_trxs_;
    std::unordered_map<wxString, std::unordered_map<int, wxString>> m_QIFaccounts;
    std::unordered_map<wxString, int64> m_QIFaccountsID;
    std::unordered_map<wxString, std::tuple<int64, wxString, wxString>> m_QIFpayeeNames;
    wxArrayString m_payee_names;
    std::unordered_map<wxString, int64> m_QIFcategoryNames;
    std::vector<Model_Splittransaction::Cache> m_splitDataSets;
    std::map<int, std::map<int, Model_Taglink::Cache>> m_splitTaglinks;
    std::map<std::pair<int, int>, Model_Taglink::Cache> m_txnTaglinks;

    wxString m_accountNameStr;
    wxString m_dateFormatStr;
    wxString decimal_;
    bool m_userDefinedDateMask = false;
    int64 fromAccountID_ = -1;
    wxString m_FileNameStr;
    const wxDateTime m_today;
    const wxDateTime m_fresh;

    wxDataViewListCtrl* dataListBox_ = nullptr;
    wxDataViewListCtrl* accListBox_ = nullptr;
    wxDataViewListCtrl* payeeListBox_ = nullptr;
    wxDataViewListCtrl* categoryListBox_ = nullptr;
    wxButton* button_search_ = nullptr;
    wxComboBox* file_name_ctrl_ = nullptr;
    wxChoice* m_choiceEncoding = nullptr;
    wxTextCtrl* log_field_ = nullptr;
    wxCheckBox* dateFromCheckBox_ = nullptr;
    wxCheckBox* dateToCheckBox_ = nullptr;
    mmDatePickerCtrl* fromDateCtrl_ = nullptr;
    mmDatePickerCtrl* toDateCtrl_ = nullptr;
    wxComboBox* choiceDateFormat_ = nullptr;
    wxCheckBox* accountCheckBox_ = nullptr;
    wxChoice* accountDropDown_ = nullptr;
    wxCheckBox* accountNumberCheckBox_ = nullptr;
    wxCheckBox* payeeIsNotesCheckBox_ = nullptr;
    wxCheckBox* payeeMatchCheckBox_ = nullptr;
    wxCheckBox* payeeMatchAddNotes_ = nullptr;
    wxButton* btnOK_ = nullptr;
    mmChoiceAmountMask* m_choiceDecimalSeparator = nullptr;
    wxCheckBox* colorCheckBox_ = nullptr;
    mmColorButton* mmColorBtn_ = nullptr;
    wxChoice* dupTransMethod_ = nullptr;
    wxChoice* dupTransAction_ = nullptr;
    wxCheckBox* dupTransCheckBox_ = nullptr;

    bool payeeIsNotes_ = false; //Include payee field in notes
    std::map<std::pair <int64, wxString>, std::map<int, std::pair<wxString, wxRegEx>> > payeeMatchPatterns_;
    bool payeeRegExInitialized_ = false;

    std::set<int64> m_duplicateTransactions; // Keep track of matched transaction IDs

    enum LIST_ID
    {
        LIST_ID_ID = 0,
        LIST_ID_ACCOUNT,
        LIST_ID_DATE,
        LIST_ID_NUMBER,
        LIST_ID_PAYEE,
        LIST_ID_TYPE,
        LIST_ID_CATEGORY,
        LIST_ID_TAGS,
        LIST_ID_VALUE,
        LIST_ID_NOTES,
        LIST_ID_size, // number of columns
    };
    enum {
        ID_ACCOUNT = wxID_HIGHEST + 1
    };
    std::map<int, wxString> ColName_;
};
