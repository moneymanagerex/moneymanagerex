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
#include "defs.h"
#include "mmtextctrl.h"


class mmFilterTransactionsDialog: public wxDialog
{
    DECLARE_DYNAMIC_CLASS(mmFilterTransactionsDialog)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    mmFilterTransactionsDialog();
    mmFilterTransactionsDialog(wxWindow* parent);

    virtual int ShowModal();

    bool checkAll(const Model_Checking::Data &tran, const int accountID);
    bool checkAll(const Model_Billsdeposits::Data &tran);
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
    int getSortColumn() { return sorterDropDown_->GetSelection(); }

private:
    void BuildPayeeList();

    bool getDateRangeCheckBox()
    {
        return dateRangeCheckBox_->GetValue();
    }
    wxDateTime getFromDateCtrl()
    {
        return fromDateCtrl_->GetValue();
    }
    wxDateTime getToDateControl()
    {
        return toDateControl_->GetValue();
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

    bool checkPayee(const Model_Checking::Data &tran);
    bool checkPayee(const Model_Billsdeposits::Data &tran);
    bool checkCategory(const Model_Checking::Data &tran);
    bool checkCategory(const Model_Billsdeposits::Data &tran);

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
    /// Returns true if Status string matches.
    bool compareStatus(const wxString& itemStatus) const;

    bool getTypeCheckBox()
    {
        return typeCheckBox_->IsChecked();
    }

    bool allowType(const wxString& typeState, bool sameAccount) const;

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

    /// Should we show tooltips?
    const bool ShowToolTips();

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
    wxChoice* sorterDropDown_;
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
    mmDateRange* date_range_;

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
