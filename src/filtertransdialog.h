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

#ifndef FILTERTRANSDIALOG_H_
#define FILTERTRANSDIALOG_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "filtertransdialog.cpp"
#endif

#include "reports/mmDateRange.h"
#include "reports/htmlbuilder.h"

#include <wx/dialog.h>
#include "mmtextctrl.h"


class mmFilterTransactionsDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmFilterTransactionsDialog);
    wxDECLARE_EVENT_TABLE();

public:
    /// Constructors
    mmFilterTransactionsDialog();
    mmFilterTransactionsDialog(wxWindow* parent);

    virtual int ShowModal();

    bool checkAll(const Model_Checking::Data &tran, int accountID, const std::map<int, Model_Splittransaction::Data_Set>& split);
    bool checkAll(const Model_Billsdeposits::Data &tran, const std::map<int, Model_Budgetsplittransaction::Data_Set>& split);
    void getDescription(mmHTMLBuilder &hb);
    bool somethingSelected();
    void setAccountToolTip(const wxString& tip) const;
    bool getStatusCheckBox()
    {
        return statusCheckBox_->IsChecked();
    }
    bool getAccountCheckBox()
    {
        return accountCheckBox_->GetValue();
    }
    int getAccountID()
    {
        return refAccountID_;
    }
    bool getCategoryCheckBox()
    {
        return categoryCheckBox_->IsChecked();
    }
    bool getSimilarStatus() { return bSimilarCategoryStatus_; }
    int getCategId() { return categID_; }
    int getSubCategId() { return subcategID_; }

private:
    void BuildPayeeList();

    bool getDateRangeCheckBox()
    {
        return dateRangeCheckBox_->GetValue();
    }
    bool getAmountRangeCheckBoxMin()
    {
        return amountRangeCheckBox_->GetValue() && !amountMinEdit_->GetValue().IsEmpty();
    }
    bool getAmountRangeCheckBoxMax()
    {
        return amountRangeCheckBox_->GetValue() && !amountMaxEdit_->GetValue().IsEmpty();
    }
    double getAmountMax();
    double getAmountMin();

    template<class MODEL, class DATA = typename MODEL::DATA>
    bool checkPayee(const DATA &tran);
    template<class MODEL, class DATA = typename MODEL::Data>
    bool checkCategory(const DATA& tran, const std::map<int, typename MODEL::Split_Data_Set>& splits);

    wxString getStatus() const;

    wxString getNumber()
    {
        return transNumberEdit_->GetValue();
    }
    wxString getNotes()
    {
        return notesEdit_->GetValue();
    }

private:
    void OnDateChanged(wxDateEvent& event);
    /// Returns true if Status string matches.
    bool compareStatus(const wxString& itemStatus) const;

    bool getTypeCheckBox()
    {
        return typeCheckBox_->IsChecked();
    }

    bool allowType(const wxString& typeState, bool sameAccount) const;

    bool getPayeeCheckBox()
    {
        return payeeCheckBox_->IsChecked();
    }

    bool getNumberCheckBox()
    {
        return transNumberCheckBox_->IsChecked();
    }

    bool getNotesCheckBox()
    {
        return notesCheckBox_->IsChecked();
    }

    void setPresettings(const wxString& view);
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
    void OnButtonSaveClick(wxCommandEvent& event);
    void OnButtonClearClick(wxCommandEvent& event);
    void OnSettingsSelected(wxCommandEvent& event);
    void datePresetMenu(wxMouseEvent& event);
    void datePresetMenuSelected(wxCommandEvent& event);
    void OnPayeeUpdated(wxCommandEvent& event);
    void OnTextEntered(wxCommandEvent& event);

    void OnCategs(wxCommandEvent& event);
    wxString to_json();
    void from_json(const wxString &data);

    wxString settings_string_;
    wxString prev_value_;
    wxTextCtrl* m_settingLabel;
    wxCheckBox* accountCheckBox_;
    wxChoice* accountDropDown_;
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
    wxRadioBox* m_radio_box_;
    wxCheckBox* transNumberCheckBox_;
    wxTextCtrl* transNumberEdit_;
    wxString m_begin_date;
    wxString m_end_date;

    int categID_;
    int subcategID_;
    bool bSimilarCategoryStatus_;
    int payeeID_;
    wxString payeeStr_;
    int refAccountID_;
    wxString refAccountStr_;
};

#endif
// FILTERTRANSDIALOG_H_
