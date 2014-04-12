/*******************************************************
Copyright (C) 2006-2012

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

#include "defs.h"
#include <wx/dataview.h>
#include "model/Model_Checking.h"
#include "model/Model_Account.h"

class wxDatePickerCtrl;
class mmQIFImport;

class mmQIFImportDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS(mmQIFImportDialog)
    DECLARE_EVENT_TABLE()

public:
    mmQIFImportDialog() {}
    mmQIFImportDialog(wxWindow* parent);

    bool Create(wxWindow* parent, wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);
    int get_last_imported_acc()
    {
        return m_firstReferencedAccountID;
    }

private:
    bool mmParseQIF();
    bool m_parsedOK;
    bool m_IsFileValid;
    bool m_IsDatesValid;
    bool m_IsAccountsOK;

    void CreateControls();
    void fillControls();

    void OnFileSearch(wxCommandEvent& event);
    bool checkQIFFile();
    void OnCheckboxClick(wxCommandEvent& /*event*/);
    void OnDateMaskChange(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnOk(wxCommandEvent& /*event*/);
    int getOrCreateAccount(const wxString& name, double init_balance, const wxString& currency_name = "");

    mmQIFImport *m_QIFimport;
    std::vector< std::pair<Model_Checking::Data*, Model_Splittransaction::Cache> > vQIF_trxs_;
    wxString dateFormat_;
    bool m_userDefinedFormat;
    wxArrayInt accounts_id_;
    wxArrayInt items_index_;
    wxArrayString accounts_name_;
    int fromAccountID_;

    wxString getLineData(const wxString& line) const;
    wxString getFileLine(wxTextInputStream& textFile, int& lineNumber) const;
    wxString getFinancistoProject(wxString& sSubCateg) const;
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
    int m_firstReferencedAccountID; //The first available account in the QIF file
    int m_numLines;

    struct m_data
    {
        wxDateTime dtdt;
        bool valid;
        bool trxComplited;
        int payeeID;
        int categID;
        int subCategID;
        int to_accountID;
        int from_accountID;
        wxString payeeString, type, amountString, transNum, notes;
        wxString dt, convDate, accountName, dateString, sToAccountName;
        wxString sFullCateg, sCateg, sSubCateg, sSplitCategs, sSplitAmount, sValid, sDescription;
        double val, dSplitAmount;
    } m_data;

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
private:
    std::map<wxString, const Model_Account::Data *> m_account_cache;
};
