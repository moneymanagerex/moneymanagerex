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

#include <wx/dialog.h>
#include "base/_defs.h"
#include "util/mmDatePicker.h"
#include "util/mmCalcPopup.h"
#include "util/mmColorButton.h"
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

// -- static

public:
    enum MODE
    {
        MODE_ADD = 0,
        MODE_UPDATE,
        MODE_ENTER,
    };

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
        ID_DIALOG_mmID_PAYEE,
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
    const wxString s_payeeWithdrawalTip = _t("Specify where the transaction is going to");
    const wxString s_amountNormalTip    = _t("Specify the amount for this transaction");
    const wxString s_amountTransferTip  = _t("Specify the amount to be transferred");

// -- state

private:
    MODE m_mode;
    SchedData m_sched_d = SchedData();
    int64 m_sched_id;
    std::vector<Split> m_split_a;
    wxArrayInt64 m_tag_id_a;
    bool m_is_transfer = false;
    bool m_mode_suggested = false;
    bool m_mode_automated = false;
    bool m_advanced = false;
    std::vector<wxString> m_frequent_note_a;

    wxSharedPtr<FieldValueDialog> w_fv_dialog;
    int                 w_focus             = wxID_ANY;
    wxSize              w_min_size;
    wxBitmapButton*     w_calc_btn          = nullptr;
    mmCalcPopup*        w_calc              = nullptr;
    wxTextCtrl*         w_number_text       = nullptr;
    mmTextCtrl*         w_amount_text       = nullptr;
    mmTextCtrl*         w_to_amount_text    = nullptr;
    mmTextCtrl*         w_calculator_text   = nullptr;
    wxTextCtrl*         w_notes_text        = nullptr;
    wxTextCtrl*         w_repeat_num_text   = nullptr;
    mmComboBoxCategory* w_cat_text          = nullptr;
    mmComboBoxPayee*    w_payee_text        = nullptr;
    mmComboBoxAccount*  w_account_text      = nullptr;
    mmComboBoxAccount*  w_to_account_text   = nullptr;
    wxBitmapButton*     w_split_btn         = nullptr;
    wxBitmapButton*     w_attachment_btn    = nullptr;
    wxButton*           w_cancel_btn        = nullptr;
    mmColorButton*      w_color_btn         = nullptr;
    wxCheckBox*         w_advanced_cb       = nullptr;
    wxChoice*           w_status_choice     = nullptr;
    wxChoice*           w_type_choice       = nullptr;
    mmDatePicker*       w_pay_date          = nullptr; // Stored in TRANSDATE
    mmDatePicker*       w_due_date          = nullptr; // Stored in NEXTOCCURRENCEDATE
    wxChoice*           w_freq_choice       = nullptr;
    wxCheckBox*         w_mode_suggested_cb = nullptr;
    wxCheckBox*         w_mode_automated_cb = nullptr;
    wxStaticText*       w_static_freq       = nullptr;
    wxStaticText*       w_static_repeat_num = nullptr;
    wxBitmapButton*     w_repeat_prev_btn   = nullptr;
    wxBitmapButton*     w_repeat_next_btn   = nullptr;
    mmTagTextCtrl*      w_tag_text          = nullptr;

public:
    bool is_new() { return (m_mode == MODE_ADD && m_sched_d.m_id <= 0); }
    bool is_dup() { return (m_mode == MODE_ADD && m_sched_d.m_id > 0); }
    bool is_edit() { return (m_mode == MODE_UPDATE); }
    bool is_enter() { return (m_mode == MODE_ENTER); }
    auto sched_id() -> int64 { return m_sched_id; }

// -- constructor

public:
    SchedDialog() {}
    SchedDialog(
        wxWindow* parent_win,
        MODE mode,
        int64 sched_id_n
    );
    ~SchedDialog();

private:
    bool create(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = _t("New Scheduled Transaction"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX,
        const wxString& name = "Scheduled Transaction Dialog"
    );
    void createControls();
    void dataToControls();

// -- methods

public:
    void setDialogHeader(const wxString& header);
    void setDialogParameters(int64 trx_id);

private:
    auto getRepeatFreq() -> RepeatFreq;
    void updateControlsForTransType();
    void setAmountCurrencies(int64 account_id, int64 to_account_id);
    void setTooltips();
    void setCategoryLabel();
    void setTransferControls(bool is_transfer = false);
    void setAdvancedTransferControls(bool advanced = false);
    void setSplitControls(bool split = false);
    void setRepeatDetails();
    void activateSplitTransactionsDlg();

// -- event handlers

private:
    void onQuit(                        wxCloseEvent&      event);
    void onOk(                          wxCommandEvent&    event);
    void onCancel(                      wxCommandEvent&    event);
    void onCategs(                      wxCommandEvent&    event);
    void onPayee(                       wxCommandEvent&    event);
    void onTypeChanged(                 wxCommandEvent&    event);
    void onAttachments(                 wxCommandEvent&    event);
    void onComboKey(                    wxKeyEvent&        event);
    void onAccountUpdated(              wxCommandEvent&    event);
    void onAutoExecutionUserAckChecked( wxCommandEvent&    event);
    void onAutoExecutionSilentChecked(  wxCommandEvent&    event);
    void onFocusChange(                 wxChildFocusEvent& event);
    void onCalculator(                  wxCommandEvent&    event);
    void onAdvanceChecked(              wxCommandEvent&    event);
    void onFrequentUsedNotes(           wxCommandEvent&    event);
    void onNoteSelected(                wxCommandEvent&    event);
    void onRepeatTypeChanged(           wxCommandEvent&    event);
    void onsetPrevOrNextRepeatDate(     wxCommandEvent&    event);
    void onMoreFields(                  wxCommandEvent&    event);
};
