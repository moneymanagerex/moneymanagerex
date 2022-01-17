/*******************************************************
Copyright (C) 2021 Nikolay Akimov
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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

#ifndef FILTERTRANSDIALOG_H_
#define FILTERTRANSDIALOG_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "filtertransdialog.cpp"
#endif

#include "reports/mmDateRange.h"
#include "reports/htmlbuilder.h"

#include <wx/dialog.h>
#include "mmTextCtrl.h"


class mmFilterTransactionsDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmFilterTransactionsDialog);
    wxDECLARE_EVENT_TABLE();

public:
    /// Constructors
    mmFilterTransactionsDialog();
    mmFilterTransactionsDialog(wxWindow* parent, bool showAccountFilter = true, bool isReportMode = false);

    virtual int ShowModal();

    bool checkAll(const Model_Checking::Data &tran
        , const std::map<int, Model_Splittransaction::Data_Set>& split);
    bool checkAll(const Model_Billsdeposits::Data &tran
        , const std::map<int, Model_Budgetsplittransaction::Data_Set>& split);
    const wxString getDescriptionToolTip();
    void getDescription(mmHTMLBuilder &hb);
    void ResetFilterStatus();

    bool isSomethingSelected();
    bool getStatusCheckBox();
    bool getAccountCheckBox();
    bool getCategoryCheckBox();
    bool getSimilarStatus();
    bool getRangeCheckBox();
    bool getDateRangeCheckBox();
    bool getStartDateCheckBox();
    bool getHideColumnsCheckBox();


    enum groupBy {
        GROUPBY_ACCOUNT,
        GROUPBY_PAYEE,
        GROUPBY_CATEGORY
    };
    int getGroupBy();

    int getCategId();
    int getSubCategId();
    const wxArrayInt getAccountsID() const;
    const wxArrayInt getHideColumnsID() const;
    
    void SetStoredSettings(int id);

    const wxString getBeginDate() { return m_begin_date; };
    const wxString getEndDate() { return m_end_date; };
    const int getStartDay() { return m_startDay; };
    const bool isFutureIgnored() { return m_futureIgnored; };

private:
    void BuildPayeeList();

    bool getAmountRangeCheckBoxMin();
    bool getAmountRangeCheckBoxMax();
    double getAmountMax();
    double getAmountMin();

    template<class MODEL, class DATA = typename MODEL::DATA>
    bool checkPayee(const DATA &tran);
    template<class MODEL, class DATA = typename MODEL::Data>
    bool checkCategory(const DATA& tran, const std::map<int, typename MODEL::Split_Data_Set>& splits);

    wxString getStatus() const;

    wxString getNumber();
    wxString getNotes();

    bool isMultiAccount_;
    bool isReportMode_;

private:
    void OnDateChanged(wxDateEvent& event);
    /// Returns true if Status string matches.
    bool compareStatus(const wxString& itemStatus) const;

    bool getTypeCheckBox();
    bool allowType(const wxString& typeState, int accountid, int toaccountid);
    bool getPayeeCheckBox();
    bool getNumberCheckBox();
    bool getNotesCheckBox();
    bool getColourCheckBox();
    void setPresettings(const wxString& view);
    void clearSettings();

    /// Creation
    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = _("Transaction Filter")
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

    /// Creates the controls and sizers
    void CreateControls();
    void dataToControls();

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOXACCOUNT
    void OnCheckboxClick( wxCommandEvent& event );

    void OnButtonOkClick(wxCommandEvent& event);
    void OnButtonCancelClick(wxCommandEvent& event);
    void OnButtonSaveClick(wxCommandEvent& event);
    void OnButtonClearClick(wxCommandEvent& event);
    void OnSettingsSelected(wxCommandEvent& event);
    void datePresetMenu(wxMouseEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    void OnPayeeUpdated(wxCommandEvent& event);
    void OnTextEntered(wxCommandEvent& event);
    void OnSaveSettings(wxCommandEvent& event);
    void SaveSettings(int menu_item);
    void OnAccountsButton(wxCommandEvent& WXUNUSED(event));
    void OnColourButton(wxCommandEvent& /*event*/);
    void OnShowColumnsButton(wxCommandEvent& /*event*/);

    void OnCategs(wxCommandEvent& event);
    const wxString to_json(bool i18n = false);
    void from_json(const wxString &data);

    bool isValuesCorrect();

    wxString settings_string_;
    wxString prev_value_;
    wxCheckBox* accountCheckBox_;
    wxButton* bSelectedAccounts_;
    std::vector<wxSharedPtr<mmDateRange>> all_date_ranges_;
    wxCheckBox* rangeCheckBox_;
    wxChoice* rangeChoice_;
    wxCheckBox* startDateCheckBox_;
    wxChoice* startDateDropDown_;
    wxCheckBox* dateRangeCheckBox_;
    wxDatePickerCtrl* fromDateCtrl_;
    wxDatePickerCtrl* toDateControl_;
    wxCheckBox* payeeCheckBox_;
    wxComboBox* cbPayee_;
    wxCheckBox* categoryCheckBox_;
    wxButton* btnCategory_;
    wxCheckBox* similarCategCheckBox_;
    wxCheckBox* statusCheckBox_;
    wxChoice* choiceStatus_;
    wxCheckBox* typeCheckBox_;
    wxCheckBox* cbTypeWithdrawal_;
    wxCheckBox* cbTypeDeposit_;
    wxCheckBox* cbTypeTransferTo_;
    wxCheckBox* cbTypeTransferFrom_;
    wxCheckBox* amountRangeCheckBox_;
    mmTextCtrl* amountMinEdit_;
    mmTextCtrl* amountMaxEdit_;
    wxCheckBox* notesCheckBox_;
    wxTextCtrl* notesEdit_;
    wxChoice* m_setting_name;
    wxBitmapButton* m_btnSaveAs;
    wxCheckBox* transNumberCheckBox_;
    wxTextCtrl* transNumberEdit_;
    wxString m_begin_date;
    wxString m_end_date;
    int m_startDay;
    bool m_futureIgnored;

    wxCheckBox* colourCheckBox_;
    wxButton* colourButton_;
    int colourValue_;

    wxCheckBox* showColumnsCheckBox_;
    wxButton* bHideColumns_;

    wxCheckBox* groupByCheckBox_;
    wxChoice* bGroupBy_;

    int categID_;
    int subcategID_;
    int payeeID_;
    bool bSimilarCategoryStatus_;
    wxString payeeStr_;

    /* Selected accounts values */
    //All account names
    wxArrayString m_accounts_name;
    //Selected accountns ID
    wxArrayInt selected_accounts_id_;
    wxArrayInt selected_columns_id_;

    enum
    {
        /* FIlter Dialog */
        ID_DIALOG_DATEPRESET= wxID_HIGHEST + 900,
        ID_DIALOG_COLOUR,
        ID_DIALOG_COLUMNS,
    };
};

#endif
// FILTERTRANSDIALOG_H_
