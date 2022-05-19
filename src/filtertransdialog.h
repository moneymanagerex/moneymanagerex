/*******************************************************
Copyright (C) 2013 - 2022 Nikolay Akimov
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
    mmFilterTransactionsDialog(wxWindow* parent, bool showAccountFilter = true, bool isReportMode = false);
    mmFilterTransactionsDialog(wxWindow* parent, const wxString& json);

    virtual int ShowModal();

    bool checkAll(const Model_Checking::Data &tran
        , const std::map<int, Model_Splittransaction::Data_Set>& split);
    bool checkAll(const Model_Billsdeposits::Data &tran
        , const std::map<int, Model_Budgetsplittransaction::Data_Set>& split);
    const wxString getDescriptionToolTip() const;
    void getDescription(mmHTMLBuilder &hb);
    void ResetFilterStatus();

    bool isSomethingSelected() const;
    bool is_status_cb_active() const;
    bool is_account_cb_active() const;
    bool is_category_cb_active() const;
    bool getSimilarStatus() const;
    bool getRangeCheckBox() const;
    bool is_date_range_cb_active() const;
    bool getStartDateCheckBox() const;
    bool getHideColumnsCheckBox() const;

    enum groupBy {
        GROUPBY_ACCOUNT,
        GROUPBY_PAYEE,
        GROUPBY_CATEGORY
    };
    int getGroupBy() const;

    int getCategId() const;
    int getSubCategId() const;
    const wxArrayInt getAccountsID() const;
    const wxArrayInt getHideColumnsID() const;

    const wxString getBeginDate() const;
    const wxString getEndDate() const;
    int getStartDay() const;
    bool isFutureIgnored() const;

private:
    void SetJsonSettings(const wxString& data);
    void dataToControls(const wxString& json);
    void BuildPayeeList();

    bool is_amountrange_min_cb_active() const;
    bool is_amount_range_max_cb_active() const;
    double getAmountMax() const;
    double getAmountMin() const;

    template<class MODEL, class DATA = typename MODEL::DATA>
    bool is_payee_matches(const DATA &tran);
    template<class MODEL, class DATA = typename MODEL::Data>
    bool is_category_matches(const DATA& tran, const std::map<int, typename MODEL::Split_Data_Set>& splits);

    const wxString getStatus() const;

    const wxString getNumber() const;
    const wxString getNotes() const;

    bool isMultiAccount_;
    bool isReportMode_;

private:
    void OnDateChanged(wxDateEvent& event);
    /// Returns true if Status string matches.
    bool is_status_matches(const wxString& itemStatus) const;

    bool is_type_cb_active() const;
    bool is_type_maches(const wxString& typeState, int accountid, int toaccountid) const;
    bool is_payee_cb_active() const;
    bool is_number_cb_active() const;
    bool is_notes_cb_active() const;
    bool is_colour_cb_active() const;
    bool is_custom_field_active() const;
    bool is_custom_field_matches(const Model_Checking::Data& tran) const;
    void setPresettings(const wxString& view);

    /// Creation
    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = _("Transaction Filter")
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX);
private:
    /// Creates the controls and sizers
    void CreateControls();
    void DoInitSettingNameChoice() const;
    void DoUpdateSettings();
    int FindLabelInJSON(const wxString& settingName) const;

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOXACCOUNT
    void OnCheckboxClick( wxCommandEvent& event );

    void OnButtonOkClick(wxCommandEvent& event);
    void OnButtonCancelClick(wxCommandEvent& event);
    void OnButtonClearClick(wxCommandEvent& event);
    void OnSettingsSelected(wxCommandEvent& event);
    void OnMenuSelected(wxCommandEvent& event);
    void OnPayeeUpdated(wxCommandEvent& event);
    void OnTextEntered(wxCommandEvent& event);
    void OnSaveSettings(wxCommandEvent& event);
    void DoSaveSettings(bool is_user_request = false);
    void OnAccountsButton(wxCommandEvent& WXUNUSED(event));
    void OnColourButton(wxCommandEvent& /*event*/);
    void OnShowColumnsButton(wxCommandEvent& /*event*/);
    void OnMoreFields(wxCommandEvent& event);
private:
    void OnCategs(wxCommandEvent& event);
    const wxString GetJsonSetings(bool i18n = false) const;

    bool is_values_correct();

    wxCheckBox* accountCheckBox_;
    wxButton* bSelectedAccounts_;
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
private:
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
    wxCheckBox* colourCheckBox_;
    wxButton* colourButton_;
    wxCheckBox* showColumnsCheckBox_;
    wxButton* bHideColumns_;
    wxCheckBox* groupByCheckBox_;
    wxChoice* bGroupBy_;
private:
    wxString m_settings_json;
    std::vector<wxSharedPtr<mmDateRange>> m_all_date_ranges;
    wxString m_begin_date;
    wxString m_end_date;
    int m_startDay;
    bool m_futureIgnored;
    int m_colour_value;
    int m_categ_id;
    int m_subcateg_id;
    int payeeID_;
    bool is_similar_category_status;
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
        /* FIlter Dialog */
        ID_DIALOG_COLOUR = wxID_HIGHEST + 897,
        ID_DIALOG_COLUMNS,
        ID_BTN_CUSTOMFIELDS,
        ID_CUSTOMFIELDS,
    };
};

inline const wxString mmFilterTransactionsDialog::getBeginDate() const { return m_begin_date; }
inline const wxString mmFilterTransactionsDialog::getEndDate() const { return m_end_date; }
inline int mmFilterTransactionsDialog::getStartDay() const { return m_startDay; }
inline bool mmFilterTransactionsDialog::isFutureIgnored() const { return m_futureIgnored; }

#endif
// FILTERTRANSDIALOG_H_
