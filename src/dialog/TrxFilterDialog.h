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

#pragma once

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "TrxFilterDialog.cpp"
#endif

#include "base/defs.h"
#include <wx/dialog.h>

#include "util/_simple.h"
#include "util/mmDateRange.h"
#include "util/mmTextCtrl.h"

#include "FieldValueDialog.h"
#include "report/htmlbuilder.h"

class TrxFilterDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(TrxFilterDialog);
    wxDECLARE_EVENT_TABLE();

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

private:
    enum
    {
        /* Filter Dialog */
        ID_DIALOG_COLUMNS = wxID_HIGHEST + 897,
        ID_BTN_CUSTOMFIELDS,
        ID_DATE_RANGE,
        ID_PERIOD_CB,
        ID_ACCOUNT_CB,
        ID_DATE_RANGE_CB,
        ID_CUSTOMFIELDS     // must be last in the list
    };

private:
    wxString m_settings_json;
    std::vector<wxSharedPtr<mmDateRange>> m_all_date_ranges;
    wxString m_start_date;
    wxString m_end_date;
    wxString m_filter_key;
    int m_startDay = 0;
    bool m_futureIgnored = false;
    bool m_use_date_filter;
    int m_color_value = -1;

    wxArrayString m_account_name_a;
    wxArrayInt64 m_selected_accounts_id;
    wxArrayInt64 m_selected_categories_id;
    wxArrayInt m_selected_columns_id;

private:
    wxSharedPtr<FieldValueDialog> m_custom_fields;
    wxCheckBox*         w_account_cb            = nullptr;
    wxButton*           bSelectedAccounts_      = nullptr;
    wxCheckBox*         datesCheckBox_          = nullptr;
    wxChoice*           rangeChoice_            = nullptr;
    wxCheckBox*         w_range_cb              = nullptr;
    mmDatePickerCtrl*   w_start_date            = nullptr;
    mmDatePickerCtrl*   w_end_date              = nullptr;
    wxCheckBox*         w_payee_cb              = nullptr;
    mmComboBoxPayee*    cbPayee_                = nullptr;
    wxCheckBox*         w_cat_cb                = nullptr;
    mmComboBoxCategory* categoryComboBox_       = nullptr;
    wxCheckBox*         w_subcat_cb             = nullptr;
    wxCheckBox*         w_status_cb             = nullptr;
    wxChoice*           w_status_choice         = nullptr;
    wxCheckBox*         w_type_cb               = nullptr;
    wxCheckBox*         w_withdrawal_cb         = nullptr;
    wxCheckBox*         w_deposit_cb            = nullptr;
    wxCheckBox*         w_transferTo_cb         = nullptr; // Transfer Out
    wxCheckBox*         w_transferFrom_cb       = nullptr; // Transfer In
    wxCheckBox*         w_amount_cb             = nullptr;
    mmTextCtrl*         w_amount_min            = nullptr;
    mmTextCtrl*         w_amount_max            = nullptr;
    wxCheckBox*         w_notes_cb              = nullptr;
    wxTextCtrl*         w_notes_text            = nullptr;
    wxChoice*           w_setting_choice        = nullptr;
    wxBitmapButton*     w_save_btn              = nullptr;
    wxBitmapButton*     w_delete_btn            = nullptr;
    wxCheckBox*         w_trx_number_cb         = nullptr;
    wxTextCtrl*         w_trx_number_text       = nullptr;
    wxCheckBox*         w_tag_cb                = nullptr;
    mmTagTextCtrl*      w_tag_text              = nullptr;
    wxCheckBox*         w_color_cb              = nullptr;
    mmColorButton*      w_color_btn             = nullptr;
    wxCheckBox*         showColumnsCheckBox_    = nullptr;
    wxButton*           bHideColumns_           = nullptr;
    wxCheckBox*         w_group_cb              = nullptr;
    wxChoice*           w_group_choice          = nullptr;
    wxCheckBox*         w_chart_cb              = nullptr;
    wxChoice*           w_chart_choice          = nullptr;
    wxCheckBox*         combineSplitsCheckBox_  = nullptr;
    wxButton*           w_reset_btn             = nullptr;

public:
    // Constructors
    TrxFilterDialog();
    TrxFilterDialog(wxWindow* parent, int64 accountID, bool isReport,  wxString selected = "");
    TrxFilterDialog(wxWindow* parent, const wxString& json);
    ~TrxFilterDialog();

    virtual int ShowModal();

    int mmIsRecordMatches(
        const TrxData &tran,
        const TrxSplitModel::DataA& splits
    );
    int mmIsRecordMatches(
        const TrxData &tran,
        const std::map<int64, TrxSplitModel::DataA>& splits
    );
    int mmIsRecordMatches(
        const SchedData &tran,
        const std::map<int64, SchedSplitModel::DataA>& splits
    );
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
    bool mmIsTagMatches(RefTypeN ref_type, int64 ref_id, bool mergeSplitTags = false);

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

    // Creation
    bool Create(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = _t("Customize Transaction Report"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX,
        const wxString& name = "Transaction Filter"
    );

private:
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
    void OnColorChecked(wxCommandEvent& event);

private:
    bool mmIsValuesCorrect() const;

};

inline const wxString TrxFilterDialog::mmGetBeginDate() const { return m_start_date; }
inline const wxString TrxFilterDialog::mmGetEndDate() const { return m_end_date; }
inline int TrxFilterDialog::mmGetStartDay() const { return m_startDay; }
inline bool TrxFilterDialog::mmIsFutureIgnored() const { return m_futureIgnored; }
inline const wxArrayInt64 TrxFilterDialog::mmGetAccountsID() const { return m_selected_accounts_id; }
inline const wxArrayInt TrxFilterDialog::mmGetHideColumnsID() const { return m_selected_columns_id; }
inline bool TrxFilterDialog::mmIsDateRangeChecked() const { return m_use_date_filter && w_range_cb->GetValue(); }
inline bool TrxFilterDialog::mmIsRangeChecked() const { return m_use_date_filter && datesCheckBox_->IsChecked(); }
inline const wxString TrxFilterDialog::mmGetNumber() const { return w_trx_number_text->GetValue(); }
inline const wxString TrxFilterDialog::mmGetNotes() const { return w_notes_text->GetValue(); }
inline bool TrxFilterDialog::mmIsHideColumnsChecked() const { return showColumnsCheckBox_->IsChecked(); }
inline bool TrxFilterDialog::mmIsTypeChecked() const { return w_type_cb->IsChecked(); }
inline bool TrxFilterDialog::mmIsPayeeChecked() const { return w_payee_cb->IsChecked(); }
inline bool TrxFilterDialog::mmIsNumberChecked() const { return w_trx_number_cb->IsChecked(); }
inline bool TrxFilterDialog::mmIsNotesChecked() const { return w_notes_cb->IsChecked(); }
inline bool TrxFilterDialog::mmIsColorChecked() const { return w_color_cb->IsChecked(); }
inline bool TrxFilterDialog::mmIsCategoryChecked() const { return w_cat_cb->IsChecked(); }
inline bool TrxFilterDialog::mmIsCategorySubCatChecked() const { return w_subcat_cb->IsChecked(); }
inline bool TrxFilterDialog::mmIsStatusChecked() const { return w_status_cb->IsChecked(); }
inline const wxString TrxFilterDialog::mmGetLabelString() const { return  w_setting_choice->GetStringSelection(); }
inline const wxString TrxFilterDialog::mmGetCategoryPattern() const { return categoryComboBox_->mmGetPattern(); }
inline bool TrxFilterDialog::mmIsCombineSplitsChecked() const { return combineSplitsCheckBox_->IsChecked(); }
inline bool TrxFilterDialog::mmIsTagsChecked() const { return w_tag_cb->IsChecked(); }
