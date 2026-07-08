/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011-2017 Nikolay Akimov
 Copyright (C) 2011-2017 Stefano Giorgio [stef145g]
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 Klaus Wich

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

#include "base/_defs.h"
#include "util/mmDatePicker.h"
#include "util/mmCalcPopup.h"
#include "util/mmColorButton.h"
#include "util/_simple.h"

#include "model/TrxModel.h"
#include "model/PayeeModel.h"
#include "model/FieldValueModel.h"
#include "model/TrxSplitModel.h"
#include "model/Journal.h"

#include "FieldValueDialog.h"

class mmTextCtrl;
class wxString;
class FieldValueDialog;

class TrxDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(TrxDialog);
    wxDECLARE_EVENT_TABLE();

// -- static

private:
    enum MODE
    {
        MODE_ADD = 0,
        MODE_UPDATE,
        MODE_ENTER,
    };

    enum
    {
        // Transaction Dialog
        ID_DIALOG_TRANS_TYPE = wxID_HIGHEST + 897,
        ID_DIALOG_TRANS_TEXTNUMBER,
        ID_DIALOG_TRANS_BUTTONDATE,
        ID_DIALOG_TRANS_TEXTNOTES,
        mmID_TEXTAMOUNT, //6900
        mmID_TOTEXTAMOUNT,
        ID_DIALOG_TRANS_STATIC_PAYEE,
        ID_DIALOG_TRANS_BUTTONPAYEE,
        ID_DIALOG_TRANS_BUTTONTO,
        ID_DIALOG_TRANS_STATUS,
        ID_DIALOG_TRANS_ADVANCED_CHECKBOX,
        ID_DIALOG_TRANS_ADVANCED_FROM,
        ID_DIALOG_TRANS_ADVANCED_TO,
        ID_DIALOG_TRANS_CATEGLABEL2,
        ID_DIALOG_TRANS_BUTTONTRANSNUM,
        mmID_TOACCOUNT_LABEL,
        mmID_TOACCOUNTNAME,
        ID_DIALOG_PAYEE_LABEL,
        ID_DIALOG_PAYEE,
        mmID_CATEGORY_SPLIT,
        mmID_ACCOUNTNAME,
        ID_DIALOG_TRANS_BUTTON_FREQENTNOTES,
        ID_DIALOG_TRANS_CUSTOMFIELDS,
        ID_DIALOG_TRANS_TAGS,
        ID_DIALOG_TRANS_TODAY,
        ID_BTN_OK_NEW,
        ID_CUSTOMFIELDS      // must be last in the list
    };

private:
    // Store used date between two invocations for save & new
    static wxDateTime s_previousDate;

// -- state

private:
    MODE m_mode = MODE_ADD;
    Journal::Data m_journal_d;
    bool m_advanced = false;
    std::vector<Split> m_split_a;
    std::vector<wxString> m_freq_notes_a;
    bool m_altRefreshDone = false;

    bool m_skip_date_init     = false;
    bool m_skip_account_init  = false;
    bool m_skip_amount_init   = false;
    bool m_skip_payee_init    = false;
    bool m_skip_status_init   = false;
    bool m_skip_notes_init    = false;
    bool m_skip_category_init = false;
    bool m_skip_tag_init      = false;
    bool m_skip_tooltips_init = false;

    wxSize w_min_size;
    int w_focus = wxID_ANY;
    wxSharedPtr<FieldValueDialog> w_fv_dlg;

    mmDatePicker*       w_date_picker      = nullptr;
    wxChoice*           w_type_choice      = nullptr;
    wxChoice*           w_status_choice    = nullptr;
    wxStaticText*       w_account_label    = nullptr;
    mmComboBoxAccount*  w_account_text     = nullptr;
    wxStaticText*       w_to_account_label = nullptr;
    mmComboBoxAccount*  w_to_account_text  = nullptr;
    mmTextCtrl*         w_amount_text      = nullptr;
    mmTextCtrl*         w_to_amount_text   = nullptr;
    wxStaticText*       w_payee_label      = nullptr;
    mmComboBoxPayee*    w_payee_text       = nullptr;
    wxStaticText*       w_cat_label        = nullptr;
    mmComboBoxCategory* w_cat_text         = nullptr;
    wxCheckBox*         w_advanced_cb      = nullptr;
    wxBitmapButton*     w_split_btn        = nullptr;
    wxTextCtrl*         w_number_text      = nullptr;
    wxTextCtrl*         w_notes_text       = nullptr;
    mmTagTextCtrl*      w_tag_text         = nullptr;
    wxBitmapButton*     w_att_btn          = nullptr;
    mmColorButton*      w_color_btn        = nullptr;
    wxBitmapButton*     w_auto_btn         = nullptr;
    wxButton*           w_cancel_btn       = nullptr;
    wxBitmapButton*     w_invert_btn       = nullptr;
    wxBitmapButton*     w_calc_btn         = nullptr;
    mmCalcPopup*        w_calc_popup       = nullptr;
    mmTextCtrl*         w_calc_text        = nullptr;
    //wxButton*         m_button_ok_new    = nullptr;

//        MODE_NEW  => is_new()
//        MODE_DUP  => is_dup() || is_enter()
//        MODE_EDIT => is_edit()

public:
    bool is_new() { return (m_mode == MODE_ADD && m_journal_d.m_id <= 0); }
    bool is_dup() { return (m_mode == MODE_ADD && m_journal_d.m_id > 0); }
    bool is_edit() { return (m_mode == MODE_UPDATE); }
    bool is_enter() { return (m_mode == MODE_ENTER); }
    auto trx_id() const -> int64 { return m_journal_d.m_id; }
    auto trx_account_id() const -> int64 { return m_journal_d.m_account_id; }
    auto trx_to_account_id() const -> int64 { return m_journal_d.m_to_account_id_n; }

// -- constructor

public:
    TrxDialog() {}
    virtual ~TrxDialog();

    TrxDialog(
        wxWindow* parent_win,
        JournalKey journal_key,
        bool duplicate = false,
        int64 account_id = -1,
        TrxType type = TrxType(TrxType::e_withdrawal)
    );

private:
    bool create(
        wxWindow* parent_win,
        wxWindowID win_id = wxID_ANY,
        const wxString& caption = _t("Transactions Dialog"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX,
        const wxString& name = "Transactions Dialog"
    );
    void createControls();
    void dataToControls();

// -- methods

public:
    void setDialogTitle(const wxString& title);

private:
    bool validateData();
    void setEventHandlers();
    void setTooltips();
    void setCategoryForPayee(const PayeeData *payee = nullptr);

// -- event handlers

private:
    void onOk(                wxCommandEvent&    event);
    void onCancel(            wxCommandEvent&    event);
    void onMoreFields(        wxCommandEvent&    event);
    void onQuit(              wxCloseEvent&      event);
    void onCategs(            wxCommandEvent&    event);
    void onAttachments(       wxCommandEvent&    event);
    void onComboKey(          wxKeyEvent&        event);
    void onDpcKillFocus(      wxFocusEvent&      event);
    void onAutoTransNum(      wxCommandEvent&    event);
    void onFrequentUsedNotes( wxCommandEvent&    event);
    void onNoteSelected(      wxCommandEvent&    event);
    void onTransTypeChanged(  wxCommandEvent&    event);
    void onPayeeChanged(      wxCommandEvent&    event);
    void onFocusChange(       wxChildFocusEvent& event);
    void onTextEntered(       wxCommandEvent&    event);
    void onAdvanceChecked(    wxCommandEvent&    event);
    void onCalculator(        wxCommandEvent&    event);
    void onSwitch(            wxCommandEvent&    event);
    void onToday(             wxCommandEvent&    event);
};
