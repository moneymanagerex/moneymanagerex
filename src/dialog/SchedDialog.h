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

#pragma once

#include "base/defs.h"
#include <wx/dialog.h>

#include "util/_simple.h"
#include "model/SchedModel.h"
#include "model/TrxModel.h"

#include "FieldValueDialog.h"

class wxStaticText;
class mmTextCtrl;
class wxCalendarCtrl;
class wxCalendarEvent;
class wxCheckBox;
class wxChoice;
class wxBitmapButton;

class SchedDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(SchedDialog);
    wxDECLARE_EVENT_TABLE();

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
        ID_CUSTOMFIELDS    // must be last in the list
    };

private:
    const wxString payeeWithdrawalTip_ = _t("Specify where the transaction is going to");
    const wxString payeeDepositTip_    = _t("Specify where the transaction is coming from");
    const wxString payeeTransferTip_   = _t("Specify which account the transfer is going to");
    const wxString amountNormalTip_    = _t("Specify the amount for this transaction");
    const wxString amountTransferTip_  = _t("Specify the amount to be transferred");

private:
    SchedData m_sched_d = SchedData();
    std::vector<Split> m_split_a;
    wxArrayInt64 m_tag_id_a;
    int64 m_sched_id;
    bool m_is_transfer = false;
    bool m_is_new = false;
    bool m_is_duplicate = false;
    bool m_enter = false;
    bool m_mode_suggested = false;
    bool m_mode_automated = false;
    bool m_advanced = false;
    std::vector<wxString> m_frequent_note_a;

    wxSharedPtr<FieldValueDialog> m_custom_fields;
    int                 w_focus             = wxID_ANY;
    wxSize              w_min_size;
    wxBitmapButton*     w_calculator_btn    = nullptr;
    mmCalculatorPopup*  w_calculator        = nullptr;
    wxTextCtrl*         w_number_text       = nullptr;
    mmTextCtrl*         w_amount_text       = nullptr;
    mmTextCtrl*         w_to_amount_text    = nullptr;
    mmTextCtrl*         w_calculator_text   = nullptr;
    wxTextCtrl*         w_notes_text        = nullptr;
    wxTextCtrl*         w_repeat_num_text   = nullptr;
    mmComboBoxCategory* w_category_cb       = nullptr;
    mmComboBoxPayee*    w_payee_cb          = nullptr;
    mmComboBoxAccount*  w_account_cb        = nullptr;
    mmComboBoxAccount*  w_to_account_cb     = nullptr;
    wxBitmapButton*     w_split_btn         = nullptr;
    wxBitmapButton*     w_attachment_btn    = nullptr;
    wxButton*           w_cancel_btn        = nullptr;
    mmColorButton*      w_color_btn         = nullptr;
    wxCheckBox*         w_advanced_cb       = nullptr;
    wxChoice*           w_status_choice     = nullptr;
    wxChoice*           w_type_choice       = nullptr;
    mmDatePickerCtrl*   w_date_paid         = nullptr; // Stored in TRANSDATE
    mmDatePickerCtrl*   w_date_due          = nullptr; // Stored in NEXTOCCURRENCEDATE
    wxChoice*           w_freq_choice       = nullptr;
    wxCheckBox*         w_mode_suggested_cb = nullptr;
    wxCheckBox*         w_mode_automated_cb = nullptr;
    wxStaticText*       w_static_freq       = nullptr;
    wxStaticText*       w_static_repeat_num = nullptr;
    wxBitmapButton*     w_repeat_prev_btn   = nullptr;
    wxBitmapButton*     w_repeat_next_btn   = nullptr;
    mmTagTextCtrl*      w_tag_text          = nullptr;

public:
    SchedDialog();
    SchedDialog(wxWindow* parent, int64 bdD, bool duplicate, bool enterOccur);
    ~SchedDialog();

public:
    int64 GetTransID() { return m_sched_id; }
    void SetDialogHeader(const wxString& header);
    void SetDialogParameters(int64 trx_id);

private:
    bool Create(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = _t("New Scheduled Transaction"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX,
        const wxString& name = "Scheduled Transaction Dialog"
    );

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

    void dataToControls();
    void updateControlsForTransType();
    void OnAccountUpdated(wxCommandEvent& event);
    void OnAutoExecutionUserAckChecked(wxCommandEvent& event);
    void OnAutoExecutionSilentChecked(wxCommandEvent& event);
    void OnFocusChange(wxChildFocusEvent& event);
    void SetAmountCurrencies(int64 accountID, int64 toAccountID);
    void OnCalculator(wxCommandEvent& event);

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
    RepeatFreq getRepeatFreq();
    void OnMoreFields(wxCommandEvent& event);

    void activateSplitTransactionsDlg();
};
