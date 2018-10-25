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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "filtertransdialog.cpp"
#endif

#include <wx/dialog.h>
#include "Model_Checking.h"
#include "Model_Billsdeposits.h"
#include "reports/mmDateRange.h"
#include <wx/bmpbuttn.h>

class mmTextCtrl;
class mmCustomData;
class mmHTMLBuilder;
class wxCheckBox;
class wxChoice;
class wxDatePickerCtrl;
class wxComboBox;
class wxRadioBox;

class mmFilterTransactionsDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmFilterTransactionsDialog);
    wxDECLARE_EVENT_TABLE();

public:
    /// Constructors
    mmFilterTransactionsDialog();
    ~mmFilterTransactionsDialog();
    mmFilterTransactionsDialog(wxWindow* parent, int account_id);

    virtual int ShowModal();

    bool checkAll(const Model_Checking::Full_Data &tran, int accountID);
    bool checkAll(const Model_Billsdeposits::Full_Data &tran);
    void getDescription(mmHTMLBuilder &hb);
    bool SomethingSelected();
    void ResetFilterStatus();
    void setAccountToolTip(const wxString& tip) const;
    bool getStatusCheckBox();
    bool getAccountCheckBox();
    int getAccountID();
    bool getCategoryCheckBox();
  
    bool getSimilarStatus();
    int getCategId();
    int getSubCategId();

private:
    void getFilterStatus();
    void BuildPayeeList();

    bool getDateRangeCheckBox();
    bool getAmountRangeCheckBox();

    template<class MODEL, class FULL_DATA = typename MODEL::Full_Data>
    bool checkPayee(const FULL_DATA &tran);
    template<class MODEL, class FULL_DATA = typename MODEL::Full_Data>
    bool checkCategory(const FULL_DATA &tran);
    template<class MODEL, class FULL_DATA = typename MODEL::Full_Data>
    bool checkAmount(const FULL_DATA &tran);

    wxString getNumber();
    wxString getNotes();

private:
    /// Returns true if Status string matches.
    bool compareStatus(const wxString& itemStatus, const wxString& filterStatus) const;

    bool getTypeCheckBox();
    bool allowType(const wxString& typeState, bool sameAccount) const;
    bool getPayeeCheckBox();
    bool getNumberCheckBox();
    bool getNotesCheckBox();
    void clearSettings();

    /// Creation
    bool Create(wxWindow* parent
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);

    /// Creates the controls and sizers
    void CreateControls();
    void dataToControls();
    wxString GetStoredSettings(int id);

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOXACCOUNT
    void OnCheckboxClick( wxCommandEvent& event );

    void OnButtonokClick(wxCommandEvent& event);
    void OnButtoncancelClick(wxCommandEvent& event);
    void SaveSettings();
    void OnButtonClearClick(wxCommandEvent& event);
    void OnMoreFields(wxCommandEvent& event);
    void OnPayeeUpdated(wxCommandEvent& event);
    void OnTextEntered(wxCommandEvent& event);
    void OnDateRangeChanged(wxCommandEvent& event);
    void OnSaveSettings(wxCommandEvent& event);
    void OnSettingsSelected(wxCommandEvent& event);

    void OnCategs(wxCommandEvent& event);
    wxString to_json(bool i18n = false);
    void from_json(const wxString &data);

    wxString settings_string_;
    wxString prev_value_;
    wxCheckBox* accountCheckBox_;
    wxChoice* accountDropDown_;
    wxCheckBox* m_dateRangeCheckBox;
    wxDatePickerCtrl* m_fromDateCtrl;
    wxDatePickerCtrl* m_toDateControl;
    wxCheckBox* payeeCheckBox_;
    wxComboBox* cbPayee_;
    wxCheckBox* categoryCheckBox_;
    wxButton* btnCategory_;
    wxBitmapButton* m_btnSaveAs;
    wxChoice* m_setting_name;
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
    wxCheckBox* transNumberCheckBox_;
    wxTextCtrl* transNumberEdit_;
    wxChoice* m_date_ranges;

    wxString m_begin_date;
    wxString m_end_date;
    int categID_;
    int subcategID_;
    bool bSimilarCategoryStatus_;
    int payeeID_;
    wxString payeeStr_;
    int refAccountID_;
    wxString refAccountStr_;
    double m_min_amount;
    double m_max_amount;
    wxString m_filterStatus;
    int m_settings_id;
    void SetSettingsID(int id) { m_settings_id = id; }
    int GetSettingsID() { return m_settings_id; }

    mmCustomData* m_custom_fields;
    std::vector<mmDateRange*> m_all_date_ranges;

    enum RepPanel
    {
        ID_CHOICE_DATE_RANGE = wxID_HIGHEST + 1,
    };

};
