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
class wxDatePickerCtrl;
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
    mmQIFImportDialog() {}
    mmQIFImportDialog(wxWindow* parent, int account_id);

    bool Create(wxWindow* parent, wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);

    wxString OnGetItemText(long item, long column) const;
    int get_last_imported_acc();

private:
    mmQIFImport* qif_api;
    void CreateControls();
    void fillControls();
    void OnFileSearch(wxCommandEvent& event);
    void OnCheckboxClick(wxCommandEvent& WXUNUSED(event));
    void OnAccountChanged(wxCommandEvent& WXUNUSED(event));
    void OnDateMaskChange(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnOk(wxCommandEvent& WXUNUSED(event));

    bool mmReadQIFFile();
    void clear_transaction_data();
    int getOrCreateAccounts();
    void getOrCreatePayees();
    void getOrCreateCategories();
    void completeTransaction(std::unordered_map <int, wxString> &trx, const wxString &accName);
    bool completeTransaction(/*in*/ const std::unordered_map <int, wxString> &i
        , /*out*/ Model_Checking::Data* trx, wxString& msg);
    bool mergeTransferPair(Model_Checking::Cache& to, Model_Checking::Cache& from);
    void appendTransfers(Model_Checking::Cache &destination, Model_Checking::Cache &target);
    void joinSplit(Model_Checking::Cache &destination, std::vector <Model_Splittransaction::Cache> &target);
    void saveSplit();
    void getDateMask();
    void refreshTabs(int tabs);
    void parseDate(const wxString &dateStr, std::map<wxString, wxString> &date_formats_temp);

    //QIF paragraphs represented like maps type = data
    std::vector <std::unordered_map <int, wxString> > vQIF_trxs_;
    std::unordered_map<wxString, int> m_date_parsing_stat; //it counts successfully parsed dates with selected date mask
    std::unordered_map <wxString, std::unordered_map <int, wxString> > m_QIFaccounts;
    std::unordered_map <wxString, int> m_QIFaccountsID;
    std::unordered_map <wxString, int> m_QIFpayeeNames;
    wxArrayString m_payee_names;
    std::unordered_map <wxString, std::pair<int, int> > m_QIFcategoryNames;
    std::vector <Model_Splittransaction::Cache> m_splitDataSets;

    int m_init_account_id;
    wxString m_accountNameStr;
    wxString m_dateFormatStr;
    bool m_userDefinedDateMask;
    int fromAccountID_;
    wxString m_FileNameStr;
    const wxDateTime m_today;
    const wxDateTime m_fresh;

    wxDataViewListCtrl* dataListBox_;
    wxDataViewListCtrl* accListBox_;
    wxDataViewListCtrl* payeeListBox_;
    wxDataViewListCtrl* categoryListBox_;
    wxButton* button_search_;
    wxTextCtrl* file_name_ctrl_;
    wxChoice* m_choiceEncoding;
    wxTextCtrl* log_field_;
    wxCheckBox* dateFromCheckBox_;
    wxCheckBox* dateToCheckBox_;
    wxDatePickerCtrl* fromDateCtrl_;
    wxDatePickerCtrl* toDateCtrl_;
    wxComboBox* choiceDateFormat_;
    wxCheckBox* accountCheckBox_;
    wxChoice* accountDropDown_;
    wxCheckBox* accountNumberCheckBox_;     
    wxCheckBox* payeeIsNotesCheckBox_;
    wxButton* btnOK_;
    
    bool payeeIsNotes_; //Include payee field in notes

    enum EColumn
    {
        COL_ID = 0,
        COL_ACCOUNT,
        COL_DATE,
        COL_NUMBER,
        COL_PAYEE,
        COL_TYPE,
        COL_CATEGORY,
        COL_VALUE,
        COL_NOTES,
        COL_MAX, // number of columns
    };
    std::map<int, wxString> ColName_;
};
