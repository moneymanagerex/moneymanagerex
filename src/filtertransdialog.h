/*******************************************************
Copyright (C) 2013 - 2022 Nikolay Akimov
Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)

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

#include "mmSimpleDialogs.h"
#include "mmcustomdata.h"
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
    ~mmFilterTransactionsDialog();
    mmFilterTransactionsDialog(wxWindow* parent, int64 accountID, bool isReport,  wxString selected = "");
    mmFilterTransactionsDialog(wxWindow* parent, const wxString& json);

    virtual int ShowModal();

    int mmIsRecordMatches(const Model_Checking::Data &tran
        , const Model_Splittransaction::Data_Set& splits);
    int mmIsRecordMatches(const Model_Checking::Data &tran
        , const std::map<int64, Model_Splittransaction::Data_Set>& splits);
    int mmIsRecordMatches(const Model_Billsdeposits::Data &tran
        , const std::map<int64, Model_Budgetsplittransaction::Data_Set>& splits);
    template<class MODEL, class DATA = typename MODEL::DATA>
    bool mmIsRecordMatches(const DATA& tran, bool mergeSplitTags = false);
    template<class MODEL, class DATA = typename MODEL::DATA>
    bool mmIsSplitRecordMatches(const DATA& split);
    const wxString mmGetDescriptionToolTip() const;
    const wxString mmGetCategoryPattern() const;
    void mmGetDescription(mmHTMLBuilder &hb);
    void mmDoResetFilterStatus();

    bool mmIsSomethingChecked() const;
    bool mmIsStatusChecked() const;
    bool mmIsAccountChecked() const;
    bool mmIsCategoryChecked() const;
    bool mmIsCategorySubCatChecked() const;
    bool mmIsRangeChecked() const;
    bool mmIsDateRangeChecked() const;
    bool mmIsHideColumnsChecked() const;
    bool mmIsCombineSplitsChecked() const;
    bool mmIsTagsChecked() const;

public:
    enum groupBy {
        GROUPBY_ACCOUNT,
        GROUPBY_PAYEE,
        GROUPBY_CATEGORY,
        GROUPBY_TYPE,
        GROUPBY_DAY,
        GROUPBY_MONTH,
        GROUPBY_YEAR
    };

    enum columnId
    {
        COL_ID = 0,
        COL_COLOR,
        COL_DATE,
        COL_NUMBER,
        COL_ACCOUNT,
        COL_PAYEE,
        COL_STATUS,
        COL_CATEGORY,
        COL_TYPE,
        COL_AMOUNT,
        COL_NOTES,
        COL_UDFC01,
        COL_UDFC02,
        COL_UDFC03,
        COL_UDFC04,
        COL_UDFC05,
        COL_TAGS,
        COL_RATE,
        COL_TIME
    };

    int mmGetGroupBy() const;

    int mmGetChart() const;

    const wxArrayInt64 mmGetAccountsID() const;
    const wxArrayInt mmGetHideColumnsID() const;

    const wxString mmGetBeginDate() const;
    const wxString mmGetEndDate() const;
    int mmGetStartDay() const;
    bool mmIsFutureIgnored() const;
    const wxString mmGetJsonSettings(bool i18n = false) const;
    const wxString mmGetLabelString() const;

private:
    const wxString mmGetTypes() const;
    void mmDoDataToControls(const wxString& json);

    bool mmIsAmountRangeMinChecked() const;
    bool mmIsAmountRangeMaxChecked() const;
    double mmGetAmountMax() const;
    double mmGetAmountMin() const;

    bool mmIsPayeeMatches(int64 payeeid);
    bool mmIsCategoryMatches(int64 categid);
    bool mmIsNoteMatches(const wxString& note);
    bool mmIsTagMatches(const wxString& refType, int64 refId, bool mergeSplitTags = false);

    void setTransferTypeCheckBoxes();

    const wxString mmGetStatus() const;
    const wxString mmGetNumber() const;
    const wxString mmGetNotes() const;

    bool isMultiAccount_ = false;
    int64 accountID_ = -1;
    bool isReportMode_ = false;

private:
    void OnDateChanged(wxDateEvent& event);
    /// Returns true if Status string matches.
    bool mmIsStatusMatches(const wxString& itemStatus) const;

    bool mmIsTypeChecked() const;
    bool mmIsTypeMaches(const wxString& typeState, int64 accountid, int64 toaccountid) const;
    bool mmIsPayeeChecked() const;
    bool mmIsNumberChecked() const;
    bool mmIsNotesChecked() const;
    bool mmIsColorChecked() const;
    bool mmIsCustomFieldChecked() const;
    bool mmIsCustomFieldMatches(int64 transid) const;

    /// Creation
    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = _t("Customize Transaction Report")
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX
        , const wxString& name = "Transaction Filter"
    );

private:
    /// Creates the controls and sizers
    void mmDoCreateControls();
    void mmDoInitVariables();
    void mmDoInitSettingNameChoice(wxString sel="") const;
    void mmDoUpdateSettings();
    void mmDoSaveSettings(bool is_user_request = false);

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOXACCOUNT
    void OnCheckboxClick( wxCommandEvent& event );
    void OnComboKey(wxKeyEvent& event);
    void OnCategoryChange(wxEvent& event);
    void OnButtonOkClick(wxCommandEvent& event);
    void OnButtonCancelClick(wxCommandEvent& event);
    void OnButtonResetClick(wxCommandEvent& WXUNUSED(event));
    void OnButtonClearClick(wxCommandEvent& event);
    void OnSettingsSelected(wxCommandEvent& event);
    void OnTextEntered(wxCommandEvent& event);
    void OnSaveSettings(wxCommandEvent& event);
    void OnAccountsButton(wxCommandEvent& WXUNUSED(event));
    void OnShowColumnsButton(wxCommandEvent& /*event*/);
    void OnMoreFields(wxCommandEvent& event);
    void OnChoice(wxCommandEvent& event);
    void OnMenuSelected(wxCommandEvent&);
    void OnQuit(wxCloseEvent& event);

private:
    bool mmIsValuesCorrect() const;

    wxCheckBox* accountCheckBox_ = nullptr;
    wxButton* bSelectedAccounts_ = nullptr;
    wxCheckBox* datesCheckBox_ = nullptr;
    wxChoice* rangeChoice_ = nullptr;
    wxCheckBox* dateRangeCheckBox_ = nullptr;
    mmDatePickerCtrl* fromDateCtrl_ = nullptr;
    mmDatePickerCtrl* toDateControl_ = nullptr;
    wxCheckBox* payeeCheckBox_ = nullptr;
    mmComboBoxPayee* cbPayee_ = nullptr;
    wxCheckBox* categoryCheckBox_ = nullptr;
    mmComboBoxCategory* categoryComboBox_ = nullptr;
    wxCheckBox* categorySubCatCheckBox_ = nullptr;
    wxCheckBox* statusCheckBox_ = nullptr;

private:
    wxChoice* choiceStatus_ = nullptr;
    wxCheckBox* typeCheckBox_ = nullptr;
    wxCheckBox* cbTypeWithdrawal_ = nullptr;
    wxCheckBox* cbTypeDeposit_ = nullptr;
    wxCheckBox* cbTypeTransferTo_ = nullptr; // Transfer Out
    wxCheckBox* cbTypeTransferFrom_ = nullptr; // Transfer In
    wxCheckBox* amountRangeCheckBox_ = nullptr;
    mmTextCtrl* amountMinEdit_ = nullptr;
    mmTextCtrl* amountMaxEdit_ = nullptr;
    wxCheckBox* notesCheckBox_ = nullptr;
    wxTextCtrl* notesEdit_ = nullptr;
    wxChoice* m_setting_name = nullptr;
    wxBitmapButton* m_btnSaveAs = nullptr;
    wxBitmapButton* m_itemButtonClear = nullptr;
    wxCheckBox* transNumberCheckBox_ = nullptr;
    wxTextCtrl* transNumberEdit_ = nullptr;
    wxCheckBox* tagCheckBox_ = nullptr;
    mmTagTextCtrl* tagTextCtrl_ = nullptr;
    wxCheckBox* colorCheckBox_ = nullptr;
    mmColorButton* colorButton_ = nullptr;
    wxCheckBox* showColumnsCheckBox_ = nullptr;
    wxButton* bHideColumns_ = nullptr;
    wxCheckBox* groupByCheckBox_ = nullptr;
    wxChoice* bGroupBy_ = nullptr;
    wxCheckBox* chartCheckBox_ = nullptr;
    wxChoice* bChart_ = nullptr;
    wxCheckBox* combineSplitsCheckBox_ = nullptr;
    wxButton* buttonReset  = nullptr;

private:
    wxString m_settings_json;
    std::vector<wxSharedPtr<mmDateRange>> m_all_date_ranges;
    wxString m_begin_date;
    wxString m_end_date;
    wxString m_filter_key;
    int m_startDay = 0;
    bool m_futureIgnored = false;
    bool m_use_date_filter;
    int m_color_value = -1;
    wxString m_payee_str;

    /* Selected accounts values */
    //All account names
    wxArrayString m_accounts_name;
    //Selected accountns ID
    wxArrayInt64 m_selected_accounts_id;
    wxArrayInt m_selected_columns_id;
    wxArrayInt64 m_selected_categories_id;
    wxSharedPtr<mmCustomData> m_custom_fields;

    enum
    {
        /* Filter Dialog */
        ID_DIALOG_COLUMNS = wxID_HIGHEST + 897,
        ID_BTN_CUSTOMFIELDS,
        ID_DATE_RANGE,
        ID_PERIOD_CB,
        ID_ACCOUNT_CB,
        ID_DATE_RANGE_CB,
        ID_CUSTOMFIELDS
    };
};

inline const wxString mmFilterTransactionsDialog::mmGetBeginDate() const { return m_begin_date; }
inline const wxString mmFilterTransactionsDialog::mmGetEndDate() const { return m_end_date; }
inline int mmFilterTransactionsDialog::mmGetStartDay() const { return m_startDay; }
inline bool mmFilterTransactionsDialog::mmIsFutureIgnored() const { return m_futureIgnored; }
inline const wxArrayInt64 mmFilterTransactionsDialog::mmGetAccountsID() const { return m_selected_accounts_id; }
inline const wxArrayInt mmFilterTransactionsDialog::mmGetHideColumnsID() const { return m_selected_columns_id; }
inline bool mmFilterTransactionsDialog::mmIsDateRangeChecked() const { return m_use_date_filter && dateRangeCheckBox_->GetValue(); }
inline bool mmFilterTransactionsDialog::mmIsRangeChecked() const { return m_use_date_filter && datesCheckBox_->IsChecked(); }
inline const wxString mmFilterTransactionsDialog::mmGetNumber() const { return transNumberEdit_->GetValue(); }
inline const wxString mmFilterTransactionsDialog::mmGetNotes() const { return notesEdit_->GetValue(); }
inline bool mmFilterTransactionsDialog::mmIsHideColumnsChecked() const { return showColumnsCheckBox_->IsChecked(); }
inline bool mmFilterTransactionsDialog::mmIsTypeChecked() const { return typeCheckBox_->IsChecked(); }
inline bool mmFilterTransactionsDialog::mmIsPayeeChecked() const { return payeeCheckBox_->IsChecked(); }
inline bool mmFilterTransactionsDialog::mmIsNumberChecked() const { return transNumberCheckBox_->IsChecked(); }
inline bool mmFilterTransactionsDialog::mmIsNotesChecked() const { return notesCheckBox_->IsChecked(); }
inline bool mmFilterTransactionsDialog::mmIsColorChecked() const { return colorCheckBox_->IsChecked(); }
inline bool mmFilterTransactionsDialog::mmIsCategoryChecked() const { return categoryCheckBox_->IsChecked(); }
inline bool mmFilterTransactionsDialog::mmIsCategorySubCatChecked() const { return categorySubCatCheckBox_->IsChecked(); }
inline bool mmFilterTransactionsDialog::mmIsStatusChecked() const { return statusCheckBox_->IsChecked(); }
inline const wxString mmFilterTransactionsDialog::mmGetLabelString() const { return  m_setting_name->GetStringSelection(); }
inline const wxString mmFilterTransactionsDialog::mmGetCategoryPattern() const { return categoryComboBox_->mmGetPattern(); }
inline bool mmFilterTransactionsDialog::mmIsCombineSplitsChecked() const { return combineSplitsCheckBox_->IsChecked(); }
inline bool mmFilterTransactionsDialog::mmIsTagsChecked() const { return tagCheckBox_->IsChecked(); }

#endif
// FILTERTRANSDIALOG_H_
