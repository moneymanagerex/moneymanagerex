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
    mmFilterTransactionsDialog(wxWindow* parent, int accountID, bool isReport,  wxString selected = "");
    mmFilterTransactionsDialog(wxWindow* parent, const wxString& json);

    virtual int ShowModal();

    bool mmIsRecordMatches(const Model_Checking::Data &tran
        , const std::map<int, Model_Splittransaction::Data_Set>& split);
    bool mmIsRecordMatches(const Model_Billsdeposits::Data &tran
        , const std::map<int, Model_Budgetsplittransaction::Data_Set>& split);
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
public:
    enum groupBy {
        GROUPBY_ACCOUNT,
        GROUPBY_PAYEE,
        GROUPBY_CATEGORY
    };
    int mmGetGroupBy() const;

    const wxArrayInt mmGetAccountsID() const;
    const wxArrayInt mmGetHideColumnsID() const;

    const wxString mmGetBeginDate() const;
    const wxString mmGetEndDate() const;
    int mmGetStartDay() const;
    bool mmIsFutureIgnored() const;
    const wxString mmGetJsonSetings(bool i18n = false) const;
    const wxString mmGetLabelString() const;

private:
    const wxString mmGetTypes() const;
    void mmDoDataToControls(const wxString& json);

    bool mmIsAmountRangeMinChecked() const;
    bool mmIsAmountRangeMaxChecked() const;
    double mmGetAmountMax() const;
    double mmGetAmountMin() const;

    template<class MODEL, class DATA = typename MODEL::DATA>
    bool mmIsPayeeMatches(const DATA &tran);
    template<class MODEL, class DATA = typename MODEL::Data>
    bool mmIsCategoryMatches(const DATA& tran, const std::map<int, typename MODEL::Split_Data_Set>& splits);

    void setTransferTypeCheckBoxes();

    const wxString mmGetStatus() const;
    const wxString mmGetNumber() const;
    const wxString mmGetNotes() const;

    bool isMultiAccount_;
    int accountID_;
    bool isReportMode_;

private:
    void OnDateChanged(wxDateEvent& event);
    /// Returns true if Status string matches.
    bool mmIsStatusMatches(const wxString& itemStatus) const;

    bool mmIsTypeChecked() const;
    bool mmIsTypeMaches(const wxString& typeState, int accountid, int toaccountid) const;
    bool mmIsPayeeChecked() const;
    bool mmIsNumberChecked() const;
    bool mmIsNotesChecked() const;
    bool mmIsColorChecked() const;
    bool mmIsCustomFieldChecked() const;
    bool mmIsCustomFieldMatches(const Model_Checking::Data& tran) const;

    /// Creation
    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = _("Transaction Filter")
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX);
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
    void OnButtonClearClick(wxCommandEvent& event);
    void OnSettingsSelected(wxCommandEvent& event);
    void OnTextEntered(wxCommandEvent& event);
    void OnSaveSettings(wxCommandEvent& event);
    void OnAccountsButton(wxCommandEvent& WXUNUSED(event));
    void OnShowColumnsButton(wxCommandEvent& /*event*/);
    void OnMoreFields(wxCommandEvent& event);
    void OnChoice(wxCommandEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);

private:
    bool mmIsValuesCorrect() const;

    wxCheckBox* accountCheckBox_;
    wxButton* bSelectedAccounts_;
    wxCheckBox* datesCheckBox_;
    wxChoice* rangeChoice_;
    wxCheckBox* dateRangeCheckBox_;
    mmDatePickerCtrl* fromDateCtrl_;
    mmDatePickerCtrl* toDateControl_;
    wxCheckBox* payeeCheckBox_;
    mmComboBoxPayee* cbPayee_;
    wxCheckBox* categoryCheckBox_;
    mmComboBoxCategory* categoryComboBox_;
    wxCheckBox* categorySubCatCheckBox_;
    wxCheckBox* statusCheckBox_;
private:
    wxChoice* choiceStatus_;
    wxCheckBox* typeCheckBox_;
    wxCheckBox* cbTypeWithdrawal_;
    wxCheckBox* cbTypeDeposit_;
    wxCheckBox* cbTypeTransferTo_; // Transfer Out
    wxCheckBox* cbTypeTransferFrom_; // Transfer In
    wxCheckBox* amountRangeCheckBox_;
    mmTextCtrl* amountMinEdit_;
    mmTextCtrl* amountMaxEdit_;
    wxCheckBox* notesCheckBox_;
    wxTextCtrl* notesEdit_;
    wxChoice* m_setting_name;
    wxBitmapButton* m_btnSaveAs;
    wxBitmapButton* m_itemButtonClear;
    wxCheckBox* transNumberCheckBox_;
    wxTextCtrl* transNumberEdit_;
    wxCheckBox* colorCheckBox_;
    mmColorButton* colorButton_;
    wxCheckBox* showColumnsCheckBox_;
    wxButton* bHideColumns_;
    wxCheckBox* groupByCheckBox_;
    wxChoice* bGroupBy_;
private:
    wxString m_settings_json;
    std::vector<wxSharedPtr<mmDateRange>> m_all_date_ranges;
    wxString m_begin_date;
    wxString m_end_date;
    wxString m_filter_key;
    int m_startDay;
    bool m_futureIgnored;
    int m_color_value;
    wxString m_payee_str;

    /* Selected accounts values */
    //All account names
    wxArrayString m_accounts_name;
    //Selected accountns ID
    wxArrayInt m_selected_accounts_id;
    wxArrayInt m_selected_columns_id;
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
inline const wxArrayInt mmFilterTransactionsDialog::mmGetAccountsID() const { return m_selected_accounts_id; }
inline const wxArrayInt mmFilterTransactionsDialog::mmGetHideColumnsID() const { return m_selected_columns_id; }
inline bool mmFilterTransactionsDialog::mmIsDateRangeChecked() const { return dateRangeCheckBox_->GetValue(); }
inline bool mmFilterTransactionsDialog::mmIsRangeChecked() const { return datesCheckBox_->IsChecked(); }
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
inline const wxString mmFilterTransactionsDialog::mmGetCategoryPattern() const { return categoryComboBox_->GetValue(); }

#endif
// FILTERTRANSDIALOG_H_
