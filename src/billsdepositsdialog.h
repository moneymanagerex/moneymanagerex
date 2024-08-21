/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016 Stefano Giorgio
 Copyright (C) 2016 - 2022 Nikolay Akimov
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#ifndef MM_EX_BDDIALOG_H_
#define MM_EX_BDDIALOG_H_

#include "mmSimpleDialogs.h"
#include <wx/dialog.h>
#include "model/Model_Billsdeposits.h"
#include "model/Model_Checking.h"
#include "mmcustomdata.h"

class wxStaticText;
class mmTextCtrl;
class wxCalendarCtrl;
class wxCalendarEvent;
class wxCheckBox;
class wxChoice;
class wxBitmapButton;

class mmBDDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmBDDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmBDDialog();
    ~mmBDDialog();
    mmBDDialog(wxWindow* parent, int bdD, bool duplicate, bool enterOccur);
    int GetTransID()
    {
        return m_trans_id;
    }

    void SetDialogHeader(const wxString& header);
    void SetDialogParameters(int trx_id);

private:
    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& caption = _("New Scheduled Transaction"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX,
        const wxString& name = "Scheduled Transaction Dialog");

    void CreateControls();

    // utility functions
    void OnQuit(wxCloseEvent& event);
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnCategs(wxCommandEvent& event);
    void OnPayee(wxCommandEvent& event);
    void OnTypeChanged(wxCommandEvent& event);
    void OnAttachments(wxCommandEvent& event);
    void OnComboKey(wxKeyEvent& event);
private:
    void dataToControls();
    void updateControlsForTransType();
    void OnAccountUpdated(wxCommandEvent& event);
    void OnAutoExecutionUserAckChecked(wxCommandEvent& event);
    void OnAutoExecutionSilentChecked(wxCommandEvent& event);
    void OnFocusChange(wxChildFocusEvent& event);
    void SetAmountCurrencies(int accountID, int toAccountID);
    void OnCalculator(wxCommandEvent& event);
    int m_trans_id;

    bool m_new_bill;
    bool m_dup_bill;
    bool m_enter_occur;
    bool autoExecuteUserAck_ = false;
    bool autoExecuteSilent_ = false;
    bool m_advanced = false;
private:
    wxButton* bCalc_ = nullptr;
    mmCalculatorPopup* calcPopup_ = nullptr;
    wxTextCtrl* textNumber_ = nullptr;
    mmTextCtrl* textAmount_ = nullptr;
    mmTextCtrl* toTextAmount_ = nullptr;
    mmTextCtrl* calcTarget_ = nullptr;
    wxTextCtrl* textNotes_ = nullptr;
    wxTextCtrl* textCategory_ = nullptr;
    wxTextCtrl* textNumRepeats_ = nullptr;
    mmComboBoxCategory* cbCategory_ = nullptr;
    wxBitmapButton* bSplit_ = nullptr;
    mmComboBoxPayee* cbPayee_ = nullptr;
    mmComboBoxAccount* cbAccount_ = nullptr;
    mmComboBoxAccount* cbToAccount_ = nullptr;
    wxButton* bAttachments_ = nullptr;
    wxButton* m_button_cancel = nullptr;
    mmColorButton* bColours_ = nullptr;
    wxCheckBox* cAdvanced_ = nullptr;
    wxChoice* m_choice_status = nullptr;
    wxChoice* m_choice_transaction_type = nullptr;
    mmDatePickerCtrl* m_date_paid = nullptr; // Stored in ::TRANSDATE
    mmDatePickerCtrl* m_date_due = nullptr;  // Stored in ::NEXTOCCURRENCEDATE
    wxChoice* m_choice_repeat = nullptr;
    wxCheckBox* itemCheckBoxAutoExeUserAck_ = nullptr;
    wxCheckBox* itemCheckBoxAutoExeSilent_ = nullptr;
    wxStaticText* staticTimesRepeat_ = nullptr;
    wxStaticText* staticTextRepeats_ = nullptr;
    wxBitmapButton* m_btn_due_prev_date = nullptr;
    wxBitmapButton* m_btn_due_date = nullptr;
    mmTagTextCtrl* tagTextCtrl_ = nullptr;

    bool m_transfer;
    int object_in_focus_;
    wxSize min_size_;
    Model_Billsdeposits::Bill_Data m_bill_data;

    std::vector<wxString> frequentNotes_;

    const wxString payeeWithdrawalTip_ = _("Specify where the transaction is going to");
    const wxString payeeDepositTip_ = _("Specify where the transaction is coming from");
    const wxString payeeTransferTip_ = _("Specify which account the transfer is going to");
    const wxString amountNormalTip_ = _("Specify the amount for this transaction");
    const wxString amountTransferTip_ = _("Specify the amount to be transferred");
private:
    void setTooltips();
    void setCategoryLabel();
    void OnAdvanceChecked(wxCommandEvent& event);
    void SetTransferControls(bool transfers = false);
    void SetAdvancedTransferControls(bool advanced = false);
    void SetSplitControls(bool split = false);
    void OnFrequentUsedNotes(wxCommandEvent& event);
    void OnNoteSelected(wxCommandEvent& event);

    void OnRepeatTypeChanged(wxCommandEvent& event);
    void OnsetPrevOrNextRepeatDate(wxCommandEvent& event);
    void setRepeatDetails();
    int getRepeatType();
    void setRepeatType(int repeatType);
    void OnMoreFields(wxCommandEvent& event);

    void activateSplitTransactionsDlg();
    static const std::vector<std::pair<int, wxString> > BILLSDEPOSITS_REPEATS;
    wxSharedPtr<mmCustomData> m_custom_fields;

private:
    enum
    {
        ID_DIALOG_TRANS_TYPE = wxID_HIGHEST + 200,
        ID_DIALOG_TRANS_BUTTONSPLIT,
        ID_DIALOG_TRANS_CATEGLABEL,
        ID_DIALOG_TRANS_STATIC_ACCOUNT,
        ID_DIALOG_TRANS_STATIC_TOACCOUNT,
        mmID_TOACCOUNTNAME,
        ID_DIALOG_TRANS_TEXTNUMBER,
        ID_DIALOG_TRANS_BUTTON_PAYDATE,
        ID_DIALOG_TRANS_TEXTNOTES,
        ID_DIALOG_TRANS_TEXTAMOUNT,
        ID_DIALOG_TRANS_TOTEXTAMOUNT,
        ID_DIALOG_TRANS_STATIC_PAYEE,
        mmID_PAYEE,
        ID_DIALOG_TRANS_BUTTONTO,
        ID_DIALOG_TRANS_STATUS,
        ID_DIALOG_TRANS_ADVANCED_CHECKBOX,
        ID_DIALOG_TRANS_ADVANCED_FROM,
        ID_DIALOG_TRANS_ADVANCED_TO,
        ID_DIALOG_TRANS_SPLITCHECKBOX,
        ID_DIALOG_TRANS_BUTTONTRANSNUMPREV,
        ID_DIALOG_TRANS_BUTTONTRANSNUM,
        ID_DIALOG_TRANS_PAYEECOMBO,
        ID_DIALOG_TRANS_BUTTON_FREQENTNOTES,
        ID_DIALOG_BD,
        mmID_ACCOUNTNAME,
        ID_DIALOG_BD_COMBOBOX_REPEATS,
        ID_DIALOG_BD_TEXTCTRL_NUM_TIMES,
        ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_USERACK,
        ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_SILENT,
        ID_DIALOG_BD_CALENDAR,
        ID_DIALOG_BD_DUE_DATE,
        ID_PANEL_REPORTS_HTMLWINDOW,
        ID_PANEL_REPORTS_HEADER_PANEL,
        ID_PANEL_REPORTS_STATIC_HEADER,
        ID_BTN_CUSTOMFIELDS,
        ID_CUSTOMFIELDS,
    };
};

#endif
