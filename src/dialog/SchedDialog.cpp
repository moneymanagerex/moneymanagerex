/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016 Stefano Giorgio
 Copyright (C) 2016 - 2022 Nikolay Akimov
 Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 KLaus Wich

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

#include "base/defs.h"
#include <wx/calctrl.h>
#include <wx/valnum.h>

#include "base/constants.h"
#include "base/paths.h"
#include "base/images_list.h"
#include "util/_util.h"
#include "util/_simple.h"
#include "util/mmDate.h"
#include "util/mmTextCtrl.h"
#include "util/mmCalcValidator.h"

#include "model/PayeeModel.h"
#include "model/CurrencyHistoryModel.h"
#include "model/PrefModel.h"

#include "manager/CategoryManager.h"
#include "manager/PayeeManager.h"
#include "AttachmentDialog.h"
#include "SchedDialog.h"
#include "SplitDialog.h"
#include "import_export/webapp.h"

// Used to determine if we need to refresh the tag text ctrl after
// accelerator hints are shown which only occurs once.
static bool altRefreshDone;

wxIMPLEMENT_DYNAMIC_CLASS(SchedDialog, wxDialog);

wxBEGIN_EVENT_TABLE(SchedDialog, wxDialog)
    EVT_CHAR_HOOK(                                 SchedDialog::OnComboKey)
    EVT_CHILD_FOCUS(                               SchedDialog::OnFocusChange)
    EVT_BUTTON(wxID_OK,                            SchedDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL,                        SchedDialog::OnCancel)
    EVT_BUTTON(mmID_CATEGORY,                      SchedDialog::OnCategs)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONSPLIT,        SchedDialog::OnCategs)
    EVT_TEXT(mmID_PAYEE,                           SchedDialog::OnPayee)
    EVT_BUTTON(wxID_FILE,                          SchedDialog::OnAttachments)
    EVT_BUTTON(ID_BTN_CUSTOMFIELDS,                SchedDialog::OnMoreFields)
    EVT_CHOICE(wxID_VIEW_DETAILS,                  SchedDialog::OnTypeChanged)
    EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, SchedDialog::OnAdvanceChecked)
    EVT_CHECKBOX(ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_USERACK, SchedDialog::OnAutoExecutionUserAckChecked)
    EVT_CHECKBOX(ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_SILENT, SchedDialog::OnAutoExecutionSilentChecked)
    EVT_CHOICE(ID_DIALOG_BD_COMBOBOX_REPEATS, SchedDialog::OnRepeatTypeChanged)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONTRANSNUMPREV, SchedDialog::OnsetPrevOrNextRepeatDate)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONTRANSNUM,     SchedDialog::OnsetPrevOrNextRepeatDate)
    EVT_TEXT(mmID_ACCOUNTNAME,                     SchedDialog::OnAccountUpdated)
    EVT_CLOSE(                                     SchedDialog::OnQuit)
wxEND_EVENT_TABLE()

SchedDialog::SchedDialog()
{
}

SchedDialog::~SchedDialog()
{
    wxSize size = GetSize();
    if (m_custom_fields->IsCustomPanelShown())
        size = wxSize(
            GetSize().GetWidth() - m_custom_fields->GetMinWidth(),
            GetSize().GetHeight()
        );
    InfoModel::instance().setSize("RECURRINGTRANS_DIALOG_SIZE", size);
}

SchedDialog::SchedDialog(
    wxWindow* parent,
    int64 sched_id,
    bool duplicate,
    bool enterOccur
) :
    m_is_duplicate(duplicate),
    m_enter(enterOccur)
{
    const SchedData* sched_n = SchedModel::instance().get_id_data_n(sched_id);
    m_is_new = !sched_n;

    if (!m_is_new) {
        // If duplicate then we will be creating a new identity
        if (!m_is_duplicate) {
            m_sched_d.m_id = sched_id;
        }
        m_sched_d.m_date_time       = sched_n->m_date_time;
        m_sched_d.m_type            = sched_n->m_type;
        m_sched_d.m_status          = sched_n->m_status;
        m_sched_d.m_account_id      = sched_n->m_account_id;
        m_sched_d.m_to_account_id_n = sched_n->m_to_account_id_n;
        m_sched_d.m_payee_id_n      = sched_n->m_payee_id_n;
        m_sched_d.m_category_id_n   = sched_n->m_category_id_n;
        m_sched_d.m_amount          = sched_n->m_amount;
        m_sched_d.m_to_amount       = sched_n->m_to_amount;
        m_sched_d.m_notes           = sched_n->m_notes;
        m_sched_d.m_number          = sched_n->m_number;
        m_sched_d.m_followup_id     = sched_n->m_followup_id;
        m_sched_d.m_color           = sched_n->m_color;
        m_sched_d.m_due_date        = sched_n->m_due_date;
        m_sched_d.m_repeat          = sched_n->m_repeat;

        m_tag_id_a.clear();
        for (const auto& gl_d : SchedModel::instance().find_id_gl_a(sched_n->m_id)) {
            m_tag_id_a.push_back(gl_d.m_tag_id);
        }

        for (const auto& qp_d : SchedModel::instance().find_id_qp_a(sched_n->m_id)) {
            wxArrayInt64 split_tag_id_a;
            for (const auto& gl_d : SchedSplitModel::instance().find_id_gl_a(qp_d.m_id))
                split_tag_id_a.push_back(gl_d.m_tag_id);
            m_split_a.push_back({
                qp_d.m_category_id, qp_d.m_amount, qp_d.m_notes, split_tag_id_a
            });
        }

        // If duplicate then we may need to copy the attachments
        if (m_is_duplicate && InfoModel::instance().getBool("ATTACHMENTSDUPLICATE", false)) {
            // FIXME: id 0 does not exist in database
            mmAttachmentManage::CloneAllAttachments(
                SchedModel::s_ref_type, sched_id, 0
            );
        }
    }

    m_is_transfer = m_sched_d.is_transfer();

    m_custom_fields = new mmCustomDataTransaction(this,
        SchedModel::s_ref_type,
        (m_is_duplicate ? sched_id : !m_is_new ? m_sched_d.m_id : -1),
        ID_CUSTOMFIELDS
    );

    this->SetFont(parent->GetFont());
    Create(parent);
    mmThemeAutoColour(this);
}

bool SchedDialog::Create(
    wxWindow* parent,
    wxWindowID id,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
) {
    altRefreshDone = false; // reset the ALT refresh indicator on new dialog creation
    style |= wxRESIZE_BORDER;
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    CreateControls();
    dataToControls();

    //generate date change events for set weekday name
    wxDateEvent dateEventPaid(w_date_paid, w_date_paid->GetValue(), wxEVT_DATE_CHANGED);
    GetEventHandler()->ProcessEvent(dateEventPaid);
    wxDateEvent dateEventDue(w_date_due, w_date_due->GetValue(), wxEVT_DATE_CHANGED);
    GetEventHandler()->ProcessEvent(dateEventDue);

    mmSetSize(this);
    // set the initial dialog size to expand the payee and category comboboxes to fit their text
    int minWidth = std::max(
        w_payee_cb->GetSize().GetX(),
        w_payee_cb->GetSizeFromTextSize(
            w_payee_cb->GetTextExtent(w_payee_cb->GetValue()).GetX()
        ).GetX()
    ) - w_payee_cb->GetSize().GetWidth();
    minWidth = std::max(
        minWidth,
        w_category_cb->GetSizeFromTextSize(
            w_category_cb->GetTextExtent(w_category_cb->GetValue()).GetX()
        ).GetX() - w_category_cb->GetSize().GetWidth()
    );
    wxSize sz = wxSize(
        GetMinWidth() + minWidth +
            (m_custom_fields->IsCustomPanelShown() ? m_custom_fields->GetMinWidth() : 0),
        GetMinHeight()
    );
    if (sz.GetWidth() > GetSize().GetWidth())
        SetSize(sz);
    SetIcon(mmex::getProgramIcon());
    Centre(wxCENTER_ON_SCREEN);

    return true;
}

void SchedDialog::dataToControls()
{
    TrxModel::instance().getFrequentUsedNotes(m_frequent_note_a);
    wxButton* bFrequentUsedNotes = static_cast<wxButton*>(
        FindWindow(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES)
    );
    bFrequentUsedNotes->Enable(!m_frequent_note_a.empty());

    w_color_btn->SetColor(m_sched_d.m_color.GetValue());

    for (int i = 0; i < RepeatFreq::size; ++i) {
        RepeatFreq freq = RepeatFreq(i);
        wxString freq_name = wxGetTranslation(freq.name());
        if (freq.has_x())
            freq_name.Replace("%s", "(n)");
        w_freq_choice->Append(freq_name);
    }
    w_freq_choice->SetSelection(RepeatFreq::e_1_month);

    for (int i = 0; i < TrxType::size; ++i) {
        if (i == TrxType::e_transfer && AccountModel::instance().find_all().size() < 2)
            break;
        wxString type_name = TrxType(i).name();
        w_type_choice->Append(
            wxGetTranslation(type_name),
            new wxStringClientData(type_name)
        );
    }
    w_type_choice->SetSelection(TrxType::e_withdrawal);

    SetTransferControls();  // hide appropriate fields
    setCategoryLabel();

    if (m_is_new && !m_enter) {
        return;
    }

    w_status_choice->SetSelection(m_sched_d.m_status.id());
    w_date_paid->SetValue(m_sched_d.m_date_time.getDateTime());
    w_date_due->SetValue(m_sched_d.m_due_date.getDateTime());
    w_freq_choice->SetSelection(m_sched_d.m_repeat.m_freq.id());

    if (m_sched_d.m_repeat.m_freq.has_num() && m_sched_d.m_repeat.m_num > 0) {
        w_repeat_num_text->SetValue(wxString::Format("%i", m_sched_d.m_repeat.m_num));
    }
    else if (m_sched_d.m_repeat.m_freq.has_x() && m_sched_d.m_repeat.m_x > 0) {
        w_repeat_num_text->SetValue(wxString::Format("%i", m_sched_d.m_repeat.m_x));
    }

    if (m_sched_d.m_repeat.m_mode.id() == RepeatMode::e_automated) {
        m_mode_automated = true;
        w_mode_automated_cb->SetValue(true);
        w_mode_suggested_cb->Enable(false);
    }
    else if (m_sched_d.m_repeat.m_mode.id() == RepeatMode::e_suggested) {
        m_mode_suggested = true;
        w_mode_suggested_cb->SetValue(true);
        w_mode_automated_cb->Enable(false);
    }
    setRepeatDetails();

    w_type_choice->SetSelection(m_sched_d.m_type.id());
    updateControlsForTransType();

    const AccountData* account_n = AccountModel::instance().get_id_data_n(m_sched_d.m_account_id);
    w_account_cb->ChangeValue(account_n ? account_n->m_name : "");

    w_tag_text->SetTags(m_tag_id_a);

    w_notes_text->SetValue(m_sched_d.m_notes);
    w_number_text->SetValue(m_sched_d.m_number);

    if (!m_split_a.empty())
        m_sched_d.m_amount = TrxSplitModel::instance().get_total(m_split_a);

    SetAmountCurrencies(m_sched_d.m_account_id, m_sched_d.m_to_account_id_n);
    w_amount_text->SetValue(m_sched_d.m_amount);

    if (m_is_transfer) {
        m_sched_d.m_payee_id_n = -1;

        // When editing an advanced transaction record, we do not reset the m_sched_d.m_to_amount
        if ((!m_is_new || m_enter)
            && (m_sched_d.m_to_amount != m_sched_d.m_amount)
        ) {
            w_advanced_cb->SetValue(true);
            SetAdvancedTransferControls(true);
        }
    }

    if (!m_enter) {
        if (m_is_duplicate)
            SetDialogHeader(_t("Duplicate Scheduled Transaction"));
        else
            SetDialogHeader(_t("Edit Scheduled Transaction"));
        w_amount_text->SetFocus();
    }
    else {
        SetDialogHeader(_t("Enter Scheduled Transaction"));
        w_date_due->Enable(false);
        w_type_choice->Disable();
        w_freq_choice->Disable();
        w_mode_automated_cb->Disable();
        w_mode_suggested_cb->Disable();
        w_repeat_num_text->Disable();
        w_repeat_prev_btn->Disable();
        w_repeat_next_btn->Disable();
        auto bok = static_cast<wxButton*>(FindWindowById(wxID_OK, this));
        if (bok) bok->SetFocus();
    }

    setTooltips();
}

void SchedDialog::SetDialogHeader(const wxString& header)
{
    this->SetTitle(header);
}

void SchedDialog::SetDialogParameters(int64 trx_id)
{
    const auto trxId_tpA_m = TrxSplitModel::instance().find_all_mTrxId();
    const auto schedId_glA_m = TagLinkModel::instance().find_refType_mRefId(
        SchedModel::s_ref_type
    );
    //const auto trx = TrxModel::instance().find(TrxCol::TRANSID(trx_id)).at(0);
    const TrxData* trx_n = TrxModel::instance().get_id_data_n(trx_id);
    TrxModel::DataExt trx_dx(*trx_n, trxId_tpA_m, schedId_glA_m);
    m_sched_d.m_account_id = trx_dx.m_account_id;
    w_account_cb->SetValue(trx_dx.ACCOUNTNAME);

    m_sched_d.m_type = trx_dx.m_type;
    w_type_choice->SetSelection(trx_dx.m_type.id());
    m_is_transfer = m_sched_d.is_transfer();
    updateControlsForTransType();

    m_sched_d.m_amount = trx_dx.m_amount;
    SetAmountCurrencies(trx_dx.m_account_id, trx_dx.m_to_account_id_n);
    w_amount_text->SetValue(m_sched_d.m_amount);

    if (m_is_transfer) {
        m_sched_d.m_to_account_id_n = trx_dx.m_to_account_id_n;
        w_to_account_cb->ChangeValue(trx_dx.TOACCOUNTNAME);

        m_sched_d.m_to_amount = trx_dx.m_to_amount;
        w_to_amount_text->SetValue(m_sched_d.m_to_amount);
        if (m_sched_d.m_to_amount != m_sched_d.m_amount) {
            w_advanced_cb->SetValue(true);
            SetAdvancedTransferControls(true);
        }
    }
    else {
        m_sched_d.m_payee_id_n = trx_dx.m_payee_id_n;
        w_payee_cb->ChangeValue(trx_dx.PAYEENAME);
    }

    if (trx_dx.has_split()) {
        for (auto& tp_d : trx_dx.m_tp_a) {
            Split split_d;
            split_d.m_category_id = tp_d.m_category_id;
            split_d.m_amount      = tp_d.m_amount;
            split_d.m_notes       = tp_d.m_notes;
            m_split_a.push_back(split_d);
        }
    }
    else {
        m_sched_d.m_category_id_n = trx_dx.m_category_id_n;
    }

    m_sched_d.m_number = trx_dx.m_number;
    w_number_text->SetValue(m_sched_d.m_number);
    m_sched_d.m_notes = trx_dx.m_notes;
    w_notes_text->SetValue(m_sched_d.m_notes);
    setCategoryLabel();
}

void SchedDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizerOuter = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* mainBoxSizerInner = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* custom_fields_box_sizer = new wxBoxSizer(wxHORIZONTAL);

    /**********************************************************************************************
     Determining where the controls go
    ***********************************************************************************************/

    /* Bills & Deposits Details */

    wxStaticBox* repeatDetailsStaticBox = new wxStaticBox(this, wxID_ANY, _t("Scheduled Transaction Details"));
    wxStaticBoxSizer* repeatTransBoxSizer = new wxStaticBoxSizer(repeatDetailsStaticBox, wxVERTICAL);

    //mainBoxSizerInner will align contents horizontally
    mainBoxSizerInner->Add(repeatTransBoxSizer, g_flagsV);
    //mainBoxSizerOuter will align contents vertically
    mainBoxSizerOuter->Add(mainBoxSizerInner, g_flagsExpand);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    repeatTransBoxSizer->Add(itemFlexGridSizer5);

    // Date Due --------------------------------------------

    w_date_due = new mmDatePickerCtrl(this, ID_DIALOG_BD_DUE_DATE);
    mmToolTip(w_date_due, _t("Specify the date when this bill or deposit is due"));
    itemFlexGridSizer5->Add(new wxStaticText(this, wxID_STATIC, _t("Date Due")), g_flagsH);
    itemFlexGridSizer5->Add(w_date_due->mmGetLayout(false));

    // Repeats --------------------------------------------

    w_static_freq = new wxStaticText(this, wxID_STATIC, _t("Repeats"));
    itemFlexGridSizer5->Add(w_static_freq, g_flagsH);
    itemFlexGridSizer5->AddSpacer(1);

    w_repeat_prev_btn = new wxBitmapButton(this,
        ID_DIALOG_TRANS_BUTTONTRANSNUMPREV,
        mmBitmapBundle(png::LEFTARROW, mmBitmapButtonSize)
    );
    mmToolTip(w_repeat_prev_btn,
        _t("Back to the last occurring date with the specified values")
    );

    w_freq_choice = new wxChoice(this, ID_DIALOG_BD_COMBOBOX_REPEATS);

    wxBoxSizer* repeatBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    w_repeat_next_btn = new wxBitmapButton(this,
        ID_DIALOG_TRANS_BUTTONTRANSNUM,
        mmBitmapBundle(png::RIGHTARROW, mmBitmapButtonSize)
    );
    mmToolTip(w_repeat_next_btn,
        _t("Advance the next occurring date with the specified values")
    );
    repeatBoxSizer->Add(w_repeat_prev_btn, g_flagsH);
    repeatBoxSizer->Add(w_freq_choice, wxSizerFlags(g_flagsExpand));
    repeatBoxSizer->Add(w_repeat_next_btn, g_flagsH);

    repeatTransBoxSizer->Add(repeatBoxSizer);

    wxFlexGridSizer* itemFlexGridSizer52 = new wxFlexGridSizer(0, 2, 0, 0);
    repeatTransBoxSizer->Add(itemFlexGridSizer52);

    // Repeat Times --------------------------------------------
    w_static_repeat_num = new wxStaticText(this, wxID_STATIC, _t("Payments Left"));
    itemFlexGridSizer52->Add(w_static_repeat_num, g_flagsH);

    wxBoxSizer* repeatTimesBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer52->Add(repeatTimesBoxSizer);

    wxIntegerValidator<int> intValidator(nullptr, wxNUM_VAL_ZERO_AS_BLANK);
    intValidator.SetMin(0);
    w_repeat_num_text = new wxTextCtrl(this,
        ID_DIALOG_BD_TEXTCTRL_NUM_TIMES,
        "",
        wxDefaultPosition, wxDefaultSize, 0, intValidator
    );
    repeatTimesBoxSizer->Add(w_repeat_num_text, g_flagsH);
    w_repeat_num_text->SetMaxLength(12);
    setRepeatDetails();

    /* Auto Execution Status */
    w_mode_suggested_cb = new wxCheckBox(this,
        ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_USERACK,
        _t("Request user to enter payment"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    mmToolTip(w_mode_suggested_cb,
        _t("User requested to enter this transaction on the 'Date Paid'")
    );

    w_mode_automated_cb = new wxCheckBox(this,
        ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_SILENT,
        _t("Grant automatic execute"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    mmToolTip(w_mode_automated_cb,
        _t("The requested payment will occur without user interaction")
    );

    repeatTransBoxSizer->Add(w_mode_suggested_cb, g_flagsExpand);
    repeatTransBoxSizer->Add(w_mode_automated_cb, g_flagsExpand);

    /*************************************************************************************************************/

    wxStaticBox* transDetailsStaticBox = new wxStaticBox(this, wxID_REMOVE,
        _t("Transaction Details")
    );
    wxStaticBoxSizer* transDetailsStaticBoxSizer = new wxStaticBoxSizer(
        transDetailsStaticBox,
        wxVERTICAL
    );
    wxFlexGridSizer* transPanelSizer = new wxFlexGridSizer(0, 3, 0, 0);
    transPanelSizer->AddGrowableCol(1, 0);
    transDetailsStaticBoxSizer->Add(transPanelSizer, wxSizerFlags(g_flagsV).Expand());
    mainBoxSizerInner->Add(transDetailsStaticBoxSizer, g_flagsExpand);

    // Trans Date --------------------------------------------
    w_date_paid = new mmDatePickerCtrl(this, ID_DIALOG_TRANS_BUTTON_PAYDATE);
    mmToolTip(w_date_paid,
        _t("Specify the date the user is requested to enter this transaction")
    );
    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _t("Date Paid")), g_flagsH);
    transPanelSizer->Add(w_date_paid->mmGetLayout());
    transPanelSizer->AddSpacer(1);

    // Status --------------------------------------------
    w_status_choice = new wxChoice(this, ID_DIALOG_TRANS_STATUS);

    for (int i = 0; i < TrxStatus::size; ++i) {
        wxString status = TrxStatus(i).name();
        w_status_choice->Append(wxGetTranslation(status), new wxStringClientData(status));
    }
    w_status_choice->SetSelection(PrefModel::instance().getTransStatusReconciled());
    mmToolTip(w_status_choice, _t("Specify the status for the transaction"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _t("Status")), g_flagsH);
    transPanelSizer->Add(w_status_choice, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Type --------------------------------------------
    w_type_choice = new wxChoice(this, wxID_VIEW_DETAILS);
    mmToolTip(w_type_choice, _t("Specify the type of transactions to be created."));
    w_advanced_cb = new wxCheckBox(this,
        ID_DIALOG_TRANS_ADVANCED_CHECKBOX,
        _t("&Advanced"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );
    w_advanced_cb->SetValue(false);
    mmToolTip(w_advanced_cb,
        _t("Allows the setting of different amounts in the FROM and TO accounts.")
    );

    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);
    typeSizer->Add(w_type_choice, g_flagsExpand);
    typeSizer->Add(w_advanced_cb, g_flagsH);

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _t("Type")), g_flagsH);
    transPanelSizer->Add(typeSizer, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Amount Fields --------------------------------------------
    wxStaticText* amount_label = new wxStaticText(this, wxID_STATIC, _t("Amount"));
    amount_label->SetFont(this->GetFont().Bold());

    w_amount_text = new mmTextCtrl(this,
        ID_DIALOG_TRANS_TEXTAMOUNT,
        "",
        wxDefaultPosition, wxDefaultSize,
        wxALIGN_RIGHT | wxTE_PROCESS_ENTER,
        mmCalcValidator()
    );
    mmToolTip(w_amount_text, amountNormalTip_);

    w_to_amount_text = new mmTextCtrl(this,
        ID_DIALOG_TRANS_TOTEXTAMOUNT,
        "",
        wxDefaultPosition, wxDefaultSize,
        wxALIGN_RIGHT | wxTE_PROCESS_ENTER,
        mmCalcValidator()
    );
    mmToolTip(w_to_amount_text, _t("Specify the transfer amount in the To Account"));

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(w_amount_text, g_flagsExpand);
    amountSizer->Add(w_to_amount_text, g_flagsExpand);

    transPanelSizer->Add(amount_label, g_flagsH);
    transPanelSizer->Add(amountSizer, wxSizerFlags(g_flagsExpand).Border(0));

    w_calculator_btn = new wxBitmapButton(this, wxID_ANY,
        mmBitmapBundle(png::CALCULATOR, mmBitmapButtonSize)
    );
    w_calculator_btn->Connect(wxID_ANY,
        wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SchedDialog::OnCalculator), nullptr, this
    );
    mmToolTip(w_calculator_btn, _t("Open Calculator"));
    transPanelSizer->Add(w_calculator_btn, g_flagsH);
    w_calculator_text = w_amount_text;
    w_calculator = new mmCalculatorPopup(w_calculator_btn, w_calculator_text);

    // Account ------------------------------------------------
    wxStaticText* acc_label = new wxStaticText(this,
        ID_DIALOG_TRANS_STATIC_ACCOUNT,
        _t("Account")
    );
    acc_label->SetFont(this->GetFont().Bold());
    transPanelSizer->Add(acc_label, g_flagsH);
    w_account_cb = new mmComboBoxAccount(this,
        mmID_ACCOUNTNAME,
        wxDefaultSize,
        m_sched_d.m_account_id
    );
    w_account_cb->SetMinSize(w_account_cb->GetSize());
    mmToolTip(w_account_cb,
        _t("Specify the Account that will own the scheduled transaction")
    );
    transPanelSizer->Add(w_account_cb, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // To Account ------------------------------------------------
    wxStaticText* to_acc_label = new wxStaticText(this,
        ID_DIALOG_TRANS_STATIC_TOACCOUNT,
        _t("To")
    );
    to_acc_label->SetFont(this->GetFont().Bold());
    transPanelSizer->Add(to_acc_label, g_flagsH);
    w_to_account_cb = new mmComboBoxAccount(this,
        mmID_TOACCOUNTNAME,
        wxDefaultSize,
        m_sched_d.m_to_account_id_n
    );
    w_to_account_cb->SetMinSize(w_to_account_cb->GetSize());
    mmToolTip(w_to_account_cb, payeeTransferTip_);
    transPanelSizer->Add(w_to_account_cb, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Payee ------------------------------------------------
    wxStaticText* payee_label = new wxStaticText(this,
        ID_DIALOG_TRANS_STATIC_PAYEE,
        _t("Payee")
    );
    payee_label->SetFont(this->GetFont().Bold());

    w_payee_cb = new mmComboBoxPayee(this,
        mmID_PAYEE, wxDefaultSize,
        m_sched_d.m_payee_id_n, true
    );
    mmToolTip(w_payee_cb, payeeWithdrawalTip_);
    w_payee_cb->SetMinSize(w_payee_cb->GetSize());
    transPanelSizer->Add(payee_label, g_flagsH);
    transPanelSizer->Add(w_payee_cb, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Category ---------------------------------------------
    wxStaticText* categ_label2 = new wxStaticText(this, ID_DIALOG_TRANS_CATEGLABEL, _t("Category"));
    categ_label2->SetFont(this->GetFont().Bold());
    w_category_cb = new mmComboBoxCategory(this,
        mmID_CATEGORY, wxDefaultSize,
        m_sched_d.m_category_id_n, true
    );
    w_category_cb->SetMinSize(w_category_cb->GetSize());
    w_split_btn = new wxBitmapButton(this, ID_DIALOG_TRANS_BUTTONSPLIT, mmBitmapBundle(png::NEW_TRX, mmBitmapButtonSize));
    mmToolTip(w_split_btn, _t("Use split Categories"));

    transPanelSizer->Add(categ_label2, g_flagsH);
    transPanelSizer->Add(w_category_cb, g_flagsExpand);
    transPanelSizer->Add(w_split_btn, g_flagsH);

    // Tags ---------------------------------------------
    wxStaticText* tag_label = new wxStaticText(this, wxID_ANY, _t("Tags"));
    w_tag_text = new mmTagTextCtrl(this);

    transPanelSizer->Add(tag_label, g_flagsH);
    transPanelSizer->Add(w_tag_text, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Number ---------------------------------------------
    w_number_text = new wxTextCtrl(this,
        ID_DIALOG_TRANS_TEXTNUMBER,
        "",
        wxDefaultPosition, wxDefaultSize
    );
    mmToolTip(w_number_text,
        _t("Specify any associated check number or transaction number")
    );

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _t("Number")), g_flagsH);
    transPanelSizer->Add(w_number_text, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Frequently Used Notes
    wxButton* bFrequentUsedNotes = new wxButton(this,
        ID_DIALOG_TRANS_BUTTON_FREQENTNOTES,
        "...",
        wxDefaultPosition, w_split_btn->GetSize()
    );
    mmToolTip(bFrequentUsedNotes, _t("Select one of the frequently used notes"));
    bFrequentUsedNotes->Connect(
        ID_DIALOG_TRANS_BUTTON_FREQENTNOTES,
        wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SchedDialog::OnFrequentUsedNotes),
        nullptr, this
    );

    // Colours
    w_color_btn = new mmColorButton(this, wxID_LOWEST, w_split_btn->GetSize());
    mmToolTip(w_color_btn, _t("User Colors"));

    // Attachments
    w_attachment_btn = new wxBitmapButton(this, wxID_FILE,
        mmBitmapBundle(png::CLIP, mmBitmapButtonSize)
    );
    mmToolTip(w_attachment_btn,
        _t("Organize attachments of this scheduled transaction")
    );

    // Now display the Frequntly Used Notes, Colour, Attachment buttons
    wxBoxSizer* notes_sizer = new wxBoxSizer(wxHORIZONTAL);
    transPanelSizer->Add(notes_sizer);
    notes_sizer->Add(new wxStaticText(this, wxID_STATIC, _t("Notes")), g_flagsH);
    notes_sizer->Add(bFrequentUsedNotes, g_flagsH);

    wxBoxSizer* RightAlign_sizer = new wxBoxSizer(wxHORIZONTAL);
    transPanelSizer->Add(RightAlign_sizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));
    RightAlign_sizer->Add(new wxStaticText(this, wxID_STATIC, _t("Color")), g_flagsH);
    RightAlign_sizer->Add(w_color_btn, wxSizerFlags());
    transPanelSizer->Add(w_attachment_btn, g_flagsH);

    // Notes
    w_notes_text = new wxTextCtrl(this,
        ID_DIALOG_TRANS_TEXTNOTES,
        "",
        wxDefaultPosition,
        wxSize(-1, w_date_due->GetSize().GetHeight() * 5),
        wxTE_MULTILINE
    );
    mmToolTip(w_notes_text,
        _t("Specify any text notes you want to add to this transaction.")
    );
    transDetailsStaticBoxSizer->Add(
        w_notes_text,
        wxSizerFlags(g_flagsExpand).Border(wxLEFT | wxRIGHT | wxBOTTOM, 10)
    );

    // Button Panel with OK and Cancel Buttons
    wxPanel* buttonsPanel = new wxPanel(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL
    );
    wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);
    buttonsPanel->SetSizer(button_sizer);

    wxButton* button_ok = new wxButton(buttonsPanel, wxID_OK, _t("&OK "));

    w_cancel_btn = new wxButton(buttonsPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));

    mainBoxSizerOuter->Add(buttonsPanel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 0));
    wxBitmapButton* button_hide = new wxBitmapButton(buttonsPanel,
        ID_BTN_CUSTOMFIELDS,
        mmBitmapBundle(png::RIGHTARROW, mmBitmapButtonSize)
    );
    mmToolTip(button_hide, _t("Show/Hide custom fields window"));
    if (m_custom_fields->GetCustomFieldsCount() == 0) {
        button_hide->Hide();
    }

    button_sizer->Add(button_ok, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    button_sizer->Add(w_cancel_btn, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    button_sizer->Add(button_hide, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));

    // Custom fields
    m_custom_fields->FillCustomFields(custom_fields_box_sizer);
    if (m_custom_fields->GetActiveCustomFieldsCount() > 0) {
        wxCommandEvent evt(wxEVT_BUTTON, ID_BTN_CUSTOMFIELDS);
        this->GetEventHandler()->AddPendingEvent(evt);
    }

    mainBoxSizerInner->Add(custom_fields_box_sizer, 0, wxEXPAND);
    this->SetSizerAndFit(mainBoxSizerOuter);
    w_min_size = GetMinSize();
    custom_fields_box_sizer->SetMinSize(transDetailsStaticBoxSizer->GetSize());
    m_custom_fields->SetMinSize(custom_fields_box_sizer->GetMinSize());
}

void SchedDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    if (m_enter && m_sched_d.m_id > 0) {
        mmAttachmentManage::DeleteAllAttachments(SchedModel::s_ref_type, m_sched_d.m_id);
    }
    EndModal(wxID_CANCEL);
}

void SchedDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
#ifdef __WXMSW__
    wxWindow* w = FindFocus();
    if (w && w->GetId() != wxID_CANCEL && wxGetKeyState(WXK_ESCAPE))
        return w_cancel_btn->SetFocus();

    if (w && w->GetId() != wxID_CANCEL) {
        return;
    }
#endif

    if (m_enter && m_sched_d.m_id > 0) {
        mmAttachmentManage::DeleteAllAttachments(SchedModel::s_ref_type, m_sched_d.m_id);
    }
    EndModal(wxID_CANCEL);
}

void SchedDialog::OnPayee(wxCommandEvent& WXUNUSED(event))
{
    const PayeeData* payee_n = PayeeModel::instance().get_id_data_n(w_payee_cb->mmGetId());
    if (!payee_n || !m_is_new)
        return;

    // Only for new/duplicate transactions: if user want to autofill last category used for payee.
    // If this is a Split Transaction, ignore displaying last category for payee
    if (m_split_a.empty() &&
        ( PrefModel::instance().getTransCategoryNone() == PrefModel::LASTUSED ||
            PrefModel::instance().getTransCategoryNone() == PrefModel::DEFAULT
        ) &&
        CategoryModel::instance().get_id_active(payee_n->m_category_id_n) &&
        CategoryModel::instance().get_id_active(payee_n->m_category_id_n)
    ) {
        m_sched_d.m_category_id_n = payee_n->m_category_id_n;

        w_category_cb->ChangeValue(
            CategoryModel::instance().get_id_fullname(m_sched_d.m_category_id_n)
        );
    }
}

void SchedDialog::SetAmountCurrencies(int64 accountID, int64 toAccountID)
{
    const AccountData* account_n = AccountModel::instance().get_id_data_n(accountID);
    if (account_n)
        w_amount_text->SetCurrency(CurrencyModel::instance().get_id_data_n(account_n->m_currency_id));

    account_n = AccountModel::instance().get_id_data_n(toAccountID);
    if (account_n)
        w_to_amount_text->SetCurrency(CurrencyModel::instance().get_id_data_n(account_n->m_currency_id));
}

void SchedDialog::OnCategs(wxCommandEvent& WXUNUSED(event))
{
    activateSplitTransactionsDlg();
}

void SchedDialog::OnTypeChanged(wxCommandEvent& WXUNUSED(event))
{
    updateControlsForTransType();
}

void SchedDialog::OnComboKey(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_RETURN) {
        auto id = event.GetId();
        switch (id) {
        case mmID_PAYEE: {
            const auto payeeName = w_payee_cb->GetValue();
            if (payeeName.empty()) {
                mmPayeeDialog dlg(this, true);
                dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    w_payee_cb->mmDoReInitialize();
                int64 payee_id = dlg.getPayeeId();
                const PayeeData* payee_n = PayeeModel::instance().get_id_data_n(payee_id);
                if (payee_n) {
                    w_payee_cb->ChangeValue(payee_n->m_name);
                    w_payee_cb->SelectAll();
                    wxCommandEvent evt;
                    OnPayee(evt);
                }
                return;
            }
            break;
        }
        case mmID_CATEGORY: {
            auto category = w_category_cb->GetValue();
            if (category.empty()) {
                CategoryManager dlg(this, true, -1);
                dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    w_category_cb->mmDoReInitialize();
                category = CategoryModel::instance().get_id_fullname(dlg.getCategId());
                w_category_cb->ChangeValue(category);
                w_category_cb->SelectAll();
                return;
            }
            break;
        }
        default:
            break;
        }
    }

    // The first time the ALT key is pressed accelerator hints are drawn, but custom painting on the tags button
    // is not applied. We need to refresh the tag ctrl to redraw the drop button with the correct image.
    if (event.AltDown() && !altRefreshDone) {
        w_tag_text->Refresh();
        altRefreshDone = true;
    }

    event.Skip();
}

void SchedDialog::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    AttachmentDialog dlg(this, SchedModel::s_ref_type, m_sched_d.m_id);
    dlg.ShowModal();
}

void SchedDialog::updateControlsForTransType()
{
    wxStaticText* accountLabel = static_cast<wxStaticText*>(FindWindow(ID_DIALOG_TRANS_STATIC_ACCOUNT));
    wxStaticText* stp = static_cast<wxStaticText*>(FindWindow(ID_DIALOG_TRANS_STATIC_PAYEE));

    m_is_transfer = false;
    switch (w_type_choice->GetSelection())
    {
    case TrxType::e_transfer: {
        m_is_transfer = true;
        mmToolTip(w_amount_text, amountTransferTip_);
        accountLabel->SetLabelText(_t("From"));

        w_to_account_cb->mmSetId(m_sched_d.m_to_account_id_n);
        m_sched_d.m_payee_id_n = -1;
        break;
    }
    case TrxType::e_withdrawal: {
        mmToolTip(w_amount_text, amountNormalTip_);
        accountLabel->SetLabelText(_t("Account"));
        stp->SetLabelText(_t("Payee"));
        mmToolTip(w_payee_cb, payeeWithdrawalTip_);

        w_payee_cb->mmSetId(m_sched_d.m_payee_id_n);
        m_sched_d.m_to_account_id_n = -1;
        wxCommandEvent evt;
        OnPayee(evt);
        break;
    }
    case TrxType::e_deposit: {
        mmToolTip(w_amount_text, amountNormalTip_);
        accountLabel->SetLabelText(_t("Account"));
        stp->SetLabelText(_t("From"));
        mmToolTip(w_payee_cb, payeeDepositTip_);

        w_payee_cb->mmSetId(m_sched_d.m_payee_id_n);
        m_sched_d.m_to_account_id_n = -1;
        wxCommandEvent evt;
        OnPayee(evt);
        break;
    }
    }

    SetTransferControls(m_is_transfer);

    if (w_advanced_cb->IsChecked()) {
        SetAdvancedTransferControls(true);
    }
}

void SchedDialog::OnFrequentUsedNotes(wxCommandEvent& WXUNUSED(event))
{
    wxMenu menu;
    int id = wxID_HIGHEST;
    for (const auto& entry : m_frequent_note_a) {
        const wxString& label = entry.Mid(0, 30) + (entry.size() > 30 ? "..." : "");
        menu.Append(++id, label);

    }
    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &SchedDialog::OnNoteSelected, this);
    if (!m_frequent_note_a.empty())
        PopupMenu(&menu);
}

void SchedDialog::OnNoteSelected(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_HIGHEST;
    if (i > 0 && static_cast<size_t>(i) <= m_frequent_note_a.size()) {
        if (!w_notes_text->GetValue().EndsWith("\n") && !w_notes_text->GetValue().empty())
            w_notes_text->AppendText("\n");
        w_notes_text->AppendText(m_frequent_note_a[i - 1]);
    }
}

void SchedDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    // Ideally 'paid date' should be on or before the 'due date'
    if (w_date_paid->GetValue().GetDateOnly() > w_date_due->GetValue())
        if (wxMessageBox(_t("The payment date is after the due date. Is this intended?"),
            _t("Looks like a late payment"),
            wxYES_NO | wxNO_DEFAULT | wxICON_WARNING) != wxYES)
            return;

    if (!w_account_cb->mmIsValid()) {
        return mmErrorDialogs::InvalidAccount(w_account_cb,
            m_is_transfer,
            mmErrorDialogs::MESSAGE_DROPDOWN_BOX
        );
    }
    m_sched_d.m_account_id = w_account_cb->mmGetId();
    const AccountData* acc = AccountModel::instance().get_id_data_n(m_sched_d.m_account_id);

    if (!w_amount_text->checkValue(m_sched_d.m_amount))
        return;

    m_sched_d.m_to_amount = m_sched_d.m_amount;
    if (m_is_transfer) {
        if (!w_to_account_cb->mmIsValid()) {
            return mmErrorDialogs::InvalidAccount(w_to_account_cb,
                m_is_transfer,
                mmErrorDialogs::MESSAGE_DROPDOWN_BOX
            );
        }
        m_sched_d.m_to_account_id_n = w_to_account_cb->mmGetId();

        if (m_sched_d.m_to_account_id_n == m_sched_d.m_account_id) {
            return mmErrorDialogs::InvalidAccount(w_payee_cb, true);
        }

        if (m_advanced && !w_to_amount_text->checkValue(m_sched_d.m_to_amount))
            return;
    }
    else {
        wxString payee_name = w_payee_cb->GetValue();
        if (payee_name.IsEmpty()) {
            mmErrorDialogs::InvalidPayee(w_payee_cb);
            return;
        }

        // Get payee string from populated list to address issues with case compare
        // differences between autocomplete and payee list
        int payee_loc = w_payee_cb->FindString(payee_name);
        if (payee_loc != wxNOT_FOUND)
            payee_name = w_payee_cb->GetString(payee_loc);

        const PayeeData* payee_n = PayeeModel::instance().get_name_data_n(payee_name);
        if (!payee_n) {
            wxMessageDialog msgDlg(this,
                wxString::Format(_t("Payee name has not been used before. Is the name correct?\n%s"), payee_name),
                _t("Confirm payee name"),
                wxYES_NO | wxYES_DEFAULT | wxICON_WARNING
            );
            if (msgDlg.ShowModal() == wxID_YES) {
                PayeeData new_payee_d = PayeeData();
                new_payee_d.m_name = payee_name;
                PayeeModel::instance().add_data_n(new_payee_d);
                payee_n = PayeeModel::instance().get_id_data_n(new_payee_d.m_id);
                mmWebApp::MMEX_WebApp_UpdatePayee();
            }
            else
                return;
        }
        m_sched_d.m_payee_id_n = payee_n->id();
    }

    if (m_split_a.empty()) {
        if (!w_category_cb->mmIsValid()) {
            return mmErrorDialogs::ToolTip4Object(w_category_cb,
                _t("Invalid value"),
                _t("Category"),
                wxICON_ERROR
            );
        }
        m_sched_d.m_category_id_n = w_category_cb->mmGetCategoryId();
    }

    if (!w_tag_text->IsValid()) {
        return mmErrorDialogs::ToolTip4Object(w_tag_text,
            _t("Invalid value"),
            _t("Tags"),
            wxICON_ERROR
        );
    }

    if (!m_custom_fields->ValidateCustomValues())
        return;

    if (!m_advanced || m_sched_d.m_to_amount < 0) {
        // if we are adding a new record and the user did not touch advanced dialog
        // we are going to use the transfer amount by calculating conversion rate.
        // subsequent edits will not allow automatic update of the amount
        if (m_is_new) {
            if (m_sched_d.m_to_account_id_n != -1) {
                const AccountData* to_account = AccountModel::instance().get_id_data_n(
                    m_sched_d.m_to_account_id_n
                );
                const CurrencyData* from_currency = AccountModel::instance().get_data_currency_p(*acc);
                const CurrencyData* to_currency = AccountModel::instance().get_data_currency_p(*to_account);

                double rateFrom = CurrencyHistoryModel::instance().get_id_date_rate(
                    from_currency->m_id,
                    m_sched_d.m_date()
                );
                double rateTo = CurrencyHistoryModel::instance().get_id_date_rate(
                    to_currency->m_id,
                    m_sched_d.m_date()
                );

                double convToBaseFrom = rateFrom * m_sched_d.m_amount;
                m_sched_d.m_to_amount = convToBaseFrom / rateTo;
            }
            else {
                m_sched_d.m_to_amount = m_sched_d.m_amount;
            }
        }
    }

    const wxString& num_str = w_repeat_num_text->GetValue();
    long num = -1;
    if (!num_str.empty() && num_str.ToLong(&num)) {
        wxASSERT(num <= std::numeric_limits<int>::max());
    }
    m_sched_d.m_repeat = Repeat(
        RepeatMode(
            m_mode_automated ? RepeatMode::e_automated :
            m_mode_suggested ? RepeatMode::e_suggested :
                               RepeatMode::e_none
        ),
        getRepeatFreq(),
        num
    );

    m_sched_d.m_due_date = mmDate(w_date_due->GetValue());
    m_sched_d.m_date_time = mmDateTime(w_date_paid->GetValue());

    wxStringClientData* status_obj = static_cast<wxStringClientData *>(
        w_status_choice->GetClientObject(w_status_choice->GetSelection())
    );
    if (status_obj) {
        m_sched_d.m_status = TrxStatus(status_obj->GetData());
    }

    m_sched_d.m_number = w_number_text->GetValue();
    m_sched_d.m_notes = w_notes_text->GetValue();

    int color_id = w_color_btn->GetColorId();
    if (color_id > 0 && color_id < 8)
        m_sched_d.m_color = color_id;
    else
        m_sched_d.m_color = -1;

    const AccountData* account = AccountModel::instance().get_id_data_n(m_sched_d.m_account_id);
    const AccountData* toAccount = AccountModel::instance().get_id_data_n(m_sched_d.m_to_account_id_n);
    if (m_sched_d.m_date() < account->m_open_date)
        return mmErrorDialogs::ToolTip4Object(
            w_account_cb,
            _t("The opening date for the account is later than the date of this transaction"),
            _t("Invalid Date")
        );

    if (toAccount && m_sched_d.m_date() < toAccount->m_open_date)
        return mmErrorDialogs::ToolTip4Object(
            w_to_account_cb,
            _t("The opening date for the account is later than the date of this transaction"),
            _t("Invalid Date")
        );

    if (!m_enter) {
        SchedData sched_d = (!m_is_new && !m_is_duplicate)
            ? *(SchedModel::instance().get_id_data_n(m_sched_d.m_id))
            : SchedData();
        sched_d.m_date_time       = m_sched_d.m_date_time;
        sched_d.m_type            = TrxType(w_type_choice->GetSelection());
        sched_d.m_status          = m_sched_d.m_status;
        sched_d.m_account_id      = m_sched_d.m_account_id;
        sched_d.m_to_account_id_n = m_sched_d.m_to_account_id_n;
        sched_d.m_payee_id_n      = m_sched_d.m_payee_id_n;
        sched_d.m_category_id_n   = m_sched_d.m_category_id_n;
        sched_d.m_amount          = m_sched_d.m_amount;
        sched_d.m_to_amount       = m_sched_d.m_to_amount;
        sched_d.m_number          = m_sched_d.m_number;
        sched_d.m_notes           = m_sched_d.m_notes;
        sched_d.m_followup_id     = m_sched_d.m_followup_id;
        sched_d.m_color           = m_sched_d.m_color;
        sched_d.m_due_date        = m_sched_d.m_due_date;
        sched_d.m_repeat          = m_sched_d.m_repeat;
        SchedModel::instance().save_data_n(sched_d);
        m_sched_id = sched_d.m_id;

        SchedSplitModel::DataA new_qp_a;
        for (const auto& split_d : m_split_a) {
            SchedSplitData new_qp_d = SchedSplitData();
            new_qp_d.m_category_id = split_d.m_category_id;
            new_qp_d.m_amount      = split_d.m_amount;
            new_qp_d.m_notes       = split_d.m_notes;
            new_qp_a.push_back(new_qp_d);
        }
        SchedSplitModel::instance().update(m_sched_id, new_qp_a);

        // Save split tags
        for (size_t i = 0; i < m_split_a.size(); i++) {
            TagLinkModel::DataA new_qp_gl_a;
            for (const auto& tag_id : m_split_a.at(i).m_tag_id_a) {
                TagLinkData new_gl_d = TagLinkData();
                new_gl_d.m_tag_id   = tag_id;
                new_gl_d.m_ref_type = SchedSplitModel::s_ref_type;
                new_gl_d.m_ref_id   = new_qp_a.at(i).m_id;
                new_qp_gl_a.push_back(new_gl_d);
            }
            TagLinkModel::instance().update(
                SchedSplitModel::s_ref_type, new_qp_a.at(i).m_id,
                new_qp_gl_a
            );
        }

        // FIXME: ref_id 0 does not exists in database
        mmAttachmentManage::RelocateAllAttachments(
            SchedModel::s_ref_type, 0,
            SchedModel::s_ref_type, m_sched_id
        );

        // Save base transaction tags
        TagLinkModel::DataA new_gl_a;
        for (const auto& tag_id : w_tag_text->GetTagIDs()) {
            TagLinkData new_gl_d = TagLinkData();
            new_gl_d.m_tag_id   = tag_id;
            new_gl_d.m_ref_type = SchedModel::s_ref_type;
            new_gl_d.m_ref_id   = m_sched_id;
            new_gl_a.push_back(new_gl_d);
        }
        TagLinkModel::instance().update(
            SchedModel::s_ref_type, m_sched_id,
            new_gl_a
        );

        //Custom Data
        m_custom_fields->SaveCustomValues(SchedModel::s_ref_type, m_sched_id);
    }
    else {
        // FIXME: use m_sched_d directly
        SchedData sched_d;
        sched_d.m_account_id = m_sched_d.m_account_id;
        sched_d.m_type       = m_sched_d.m_type;
        sched_d.m_amount     = m_sched_d.m_amount;
        if (!SchedModel::instance().is_data_allowed(sched_d))
            return;

        TrxData new_trx_d = TrxData();
        new_trx_d.m_type            = TrxType(w_type_choice->GetSelection());
        new_trx_d.m_date_time       = m_sched_d.m_date_time;
        new_trx_d.m_status          = m_sched_d.m_status;
        new_trx_d.m_account_id      = m_sched_d.m_account_id;
        new_trx_d.m_to_account_id_n = m_sched_d.m_to_account_id_n;
        new_trx_d.m_payee_id_n      = m_sched_d.m_payee_id_n;
        new_trx_d.m_category_id_n   = m_sched_d.m_category_id_n;
        new_trx_d.m_amount          = m_sched_d.m_amount;
        new_trx_d.m_to_amount       = m_sched_d.m_to_amount;
        new_trx_d.m_number          = m_sched_d.m_number;
        new_trx_d.m_notes           = m_sched_d.m_notes;
        new_trx_d.m_followup_id     = m_sched_d.m_followup_id;
        new_trx_d.m_color           = m_sched_d.m_color;
        TrxModel::instance().save_trx_n(new_trx_d);
        int64 new_trx_id = new_trx_d.m_id;

        TrxSplitModel::DataA new_tp_a;
        for (auto& split_d : m_split_a) {
            TrxSplitData new_tp_d = TrxSplitData();
            new_tp_d.m_trx_id      = new_trx_id;
            new_tp_d.m_category_id = split_d.m_category_id;
            new_tp_d.m_amount      = split_d.m_amount;
            new_tp_d.m_notes       = split_d.m_notes;
            new_tp_a.push_back(new_tp_d);
        }
        TrxSplitModel::instance().update_trx(new_trx_id, new_tp_a);

        // Save split tags
        for (size_t i = 0; i < m_split_a.size(); i++) {
            TagLinkModel::DataA new_tp_gl_a;
            for (const auto& tag_id : m_split_a.at(i).m_tag_id_a) {
                TagLinkData new_gl_d = TagLinkData();
                new_gl_d.m_tag_id   = tag_id;
                new_gl_d.m_ref_type = TrxSplitModel::s_ref_type;
                new_gl_d.m_ref_id   = new_tp_a.at(i).m_id;
                new_tp_gl_a.push_back(new_gl_d);
            }
            TagLinkModel::instance().update(
                TrxSplitModel::s_ref_type, new_tp_a.at(i).m_id,
                new_tp_gl_a
            );
        }

        // Custom Data
        m_custom_fields->SaveCustomValues(TrxModel::s_ref_type, new_trx_id);

        mmAttachmentManage::RelocateAllAttachments(
            SchedModel::s_ref_type, m_sched_d.m_id,
            TrxModel::s_ref_type, new_trx_id
        );

        // Save base transaction tags
        TagLinkModel::DataA new_gl_a;
        for (const auto& tag_id : w_tag_text->GetTagIDs()) {
            TagLinkData new_gl_d = TagLinkData();
            new_gl_d.m_tag_id   = tag_id;
            new_gl_d.m_ref_type = TrxModel::s_ref_type;
            new_gl_d.m_ref_id   = new_trx_id;
            new_gl_a.push_back(new_gl_d);
        }
        TagLinkModel::instance().update(
            TrxModel::s_ref_type, new_trx_id,
            new_gl_a
        );
        SchedModel::instance().reschedule_id(m_sched_d.m_id);
    }

    EndModal(wxID_OK);
}

void SchedDialog::SetSplitControls(bool split)
{
    w_amount_text->Enable(!split);
    w_calculator_btn->Enable(!split);
    if (split) {
        m_sched_d.m_amount = TrxSplitModel::instance().get_total(m_split_a);
        m_sched_d.m_category_id_n = -1;
    }
    else {
        m_split_a.clear();
    }
    setCategoryLabel();
}

void SchedDialog::OnAutoExecutionUserAckChecked(wxCommandEvent& WXUNUSED(event))
{
    m_mode_suggested = !m_mode_suggested;
    if (m_mode_suggested) {
        w_mode_automated_cb->SetValue(false);
        w_mode_automated_cb->Enable(false);
        m_mode_automated = false;
    }
    else {
        w_mode_automated_cb->Enable(true);
    }
}

void SchedDialog::OnAutoExecutionSilentChecked(wxCommandEvent& WXUNUSED(event))
{
    m_mode_automated = !m_mode_automated;
    if (m_mode_automated) {
        w_mode_suggested_cb->SetValue(false);
        w_mode_suggested_cb->Enable(false);
        m_mode_suggested = false;
    }
    else {
        w_mode_suggested_cb->Enable(true);
    }
}

void SchedDialog::OnAdvanceChecked(wxCommandEvent& WXUNUSED(event))
{
    SetAdvancedTransferControls(w_advanced_cb->IsChecked());
}

void SchedDialog::SetTransferControls(bool transfers)
{
    wxStaticText* stp = static_cast<wxStaticText*>(FindWindow(ID_DIALOG_TRANS_STATIC_PAYEE));
    wxStaticText* stta = static_cast<wxStaticText*>(FindWindow(ID_DIALOG_TRANS_STATIC_TOACCOUNT));

    w_advanced_cb->Enable(transfers);
    if (transfers) {
        SetSplitControls();
    }
    else {
        SetAdvancedTransferControls();
        w_to_amount_text->ChangeValue("");
        w_advanced_cb->SetValue(false);
    }

    w_split_btn->Enable(!transfers);
    w_to_account_cb->Show(transfers);
    w_payee_cb->Show(!transfers);
    stta->Show(m_is_transfer);
    stp->Show(!m_is_transfer);
    Layout();
}

void SchedDialog::SetAdvancedTransferControls(bool advanced)
{
    m_advanced = advanced;
    w_to_amount_text->Enable(m_advanced);
    mmToolTip(w_amount_text, m_advanced
        ? amountTransferTip_
        : _t("Specify the transfer amount in the From Account")
    );
    if (m_advanced)
        w_to_amount_text->SetValue(m_sched_d.m_to_amount);
    else
        w_to_amount_text->ChangeValue("");
}

void SchedDialog::setRepeatDetails()
{
    w_static_freq->SetLabelText(_t("Repeats"));

    RepeatFreq repeat_freq = getRepeatFreq();
    if (repeat_freq.id() == RepeatFreq::e_in_x_days ||
        repeat_freq.id() == RepeatFreq::e_every_x_days
    ) {
        w_static_repeat_num->SetLabelText(_t("Period: Days"));
        const auto toolTipsStr = _t("Specify period in Days.");
        mmToolTip(w_repeat_num_text, toolTipsStr);
    }
    else if (repeat_freq.id() == RepeatFreq::e_in_x_months ||
        repeat_freq.id() == RepeatFreq::e_every_x_months
    ) {
        w_static_repeat_num->SetLabelText(_t("Period: Months"));
        const auto toolTipsStr = _t("Specify period in Months.");
        mmToolTip(w_repeat_num_text, toolTipsStr);
    }
    else if (repeat_freq.id() == RepeatFreq::e_once) {
        w_static_repeat_num->SetLabelText(_t("Payments Left"));
        const auto toolTipsStr = _t("Ignored (leave blank).");
        mmToolTip(w_repeat_num_text, toolTipsStr);
    }
    else {
        w_static_repeat_num->SetLabelText(_t("Payments Left"));
        const auto toolTipsStr = _t("Specify the number of payments to be made.\n"
            "Leave blank if the payments continue forever.");
        mmToolTip(w_repeat_num_text, toolTipsStr);
    }
}

void SchedDialog::OnRepeatTypeChanged(wxCommandEvent& WXUNUSED(event))
{
    setRepeatDetails();
}

RepeatFreq SchedDialog::getRepeatFreq()
{
    int freq_id = w_freq_choice->GetSelection();
    return freq_id >= 0 ? RepeatFreq(freq_id) : RepeatFreq();
}

void SchedDialog::OnsetPrevOrNextRepeatDate(wxCommandEvent& event)
{
    Repeat repeat;
    repeat.m_freq = getRepeatFreq();
    repeat.m_x = 1;
    wxString num_str = w_repeat_num_text->GetValue();
    bool goPrev = (event.GetId() == ID_DIALOG_TRANS_BUTTONTRANSNUMPREV);
    bool ok = true;

    if (repeat.m_freq.has_x()) {
        repeat.m_x = wxAtoi(num_str);
        if (!num_str.IsNumber() || !repeat.m_x) {
            mmErrorDialogs::ToolTip4Object(w_repeat_num_text,
                _t("Invalid value"),
                _t("Error")
            );
            return;
        }
    }

    w_date_paid->SetValue(repeat.next_datetime(w_date_paid->GetValue(), goPrev));
    w_date_due->SetValue( repeat.next_datetime(w_date_due->GetValue(),  goPrev));
}

void SchedDialog::activateSplitTransactionsDlg()
{
    if (m_split_a.empty()) {
        if (!w_amount_text->GetDouble(m_sched_d.m_amount)) {
            m_sched_d.m_amount = 0;
        }
        Split split_d;
        split_d.m_category_id = m_sched_d.m_category_id_n;
        split_d.m_amount      = m_sched_d.m_amount;
        split_d.m_notes       = m_sched_d.m_notes;
        m_split_a.push_back(split_d);
    }

    SplitDialog dlg(this, m_split_a, m_sched_d.m_account_id);
    if (dlg.ShowModal() == wxID_OK) {
        m_split_a    = dlg.mmGetResult();
        m_sched_d.m_amount        = TrxSplitModel::instance().get_total(m_split_a);
        m_sched_d.m_category_id_n = -1;
        if (w_type_choice->GetSelection() == TrxType::e_transfer &&
            m_sched_d.m_amount < 0
        ) {
            m_sched_d.m_amount = -m_sched_d.m_amount;
        }
        w_amount_text->SetValue(m_sched_d.m_amount);
    }

    if (m_split_a.size() == 1) {
        m_sched_d.m_category_id_n = m_split_a[0].m_category_id;
        w_notes_text->SetValue(m_split_a[0].m_notes);
        m_split_a.clear();
    }

    setCategoryLabel();
}

void SchedDialog::setTooltips()
{
    if (!this->m_split_a.empty()) {
        const CurrencyData* currency = CurrencyModel::instance().get_base_data_n();
        const AccountData* account = AccountModel::instance().get_id_data_n(m_sched_d.m_account_id);
        if (account) {
            currency = AccountModel::instance().get_data_currency_p(*account);
        }

        w_split_btn->SetToolTip(TrxSplitModel::instance().get_tooltip(m_split_a, currency));
    }
    else
        mmToolTip(w_split_btn, _t("Use split Categories"));
}

void SchedDialog::setCategoryLabel()
{
    bool has_split = !m_split_a.empty();

    w_split_btn->UnsetToolTip();
    if (has_split) {
        w_category_cb->SetLabelText(_t("Split Transaction"));
        w_amount_text->SetValue(TrxSplitModel::instance().get_total(m_split_a));
        m_sched_d.m_category_id_n = -1;
    }
    else if (m_is_transfer && m_is_new
        && PrefModel::instance().getTransCategoryTransferNone() == PrefModel::LASTUSED
    ) {
        TrxModel::DataA transactions = TrxModel::instance().find(
            TrxModel::DATE(OP_LE, mmDate::today()),
            TrxModel::TYPE(OP_EQ, TrxType(TrxType::e_transfer))
        );

        if (!transactions.empty()) {
            int64 cat_id = transactions.back().m_category_id_n;
            w_category_cb->ChangeValue(CategoryModel::instance().get_id_fullname(cat_id));
        }
    }
    else {
        const auto cat_fullname = CategoryModel::instance().get_id_fullname(m_sched_d.m_category_id_n);
        w_category_cb->ChangeValue(cat_fullname);
    }

    setTooltips();

    bool is_split = !m_split_a.empty();
    w_amount_text->Enable(!is_split);
    w_calculator_btn->Enable(!is_split);
    wxBitmapButton* bSplit = static_cast<wxBitmapButton*>(FindWindow(ID_DIALOG_TRANS_BUTTONSPLIT));
    bSplit->Enable(!m_is_transfer);
    w_category_cb->Enable(!is_split);
    Layout();
}

void SchedDialog::OnMoreFields(wxCommandEvent& WXUNUSED(event))
{
    wxBitmapButton* button = static_cast<wxBitmapButton*>(FindWindow(ID_BTN_CUSTOMFIELDS));

    if (button)
        button->SetBitmap(mmBitmapBundle(m_custom_fields->IsCustomPanelShown() ? png::RIGHTARROW : png::LEFTARROW, mmBitmapButtonSize));

    m_custom_fields->ShowHideCustomPanel();

    if (m_custom_fields->IsCustomPanelShown()) {
        SetMinSize(wxSize(
            w_min_size.GetWidth() + m_custom_fields->GetMinWidth(),
            w_min_size.GetHeight()
        ));
        SetSize(wxSize(
            GetSize().GetWidth() + m_custom_fields->GetMinWidth(),
            GetSize().GetHeight()
        ));
    }
    else {
        SetMinSize(w_min_size);
        SetSize(wxSize(
            GetSize().GetWidth() - m_custom_fields->GetMinWidth(),
            GetSize().GetHeight()
        ));
    }
}

void SchedDialog::OnAccountUpdated(wxCommandEvent& WXUNUSED(event))
{
    int64 account_id = w_account_cb->mmGetId();
    const AccountData* account_n = AccountModel::instance().get_id_data_n(account_id);
    if (account_n) {
        SetAmountCurrencies(account_id, -1);
        if (w_amount_text->Calculate()) {
            w_amount_text->GetDouble(m_sched_d.m_amount);
        }

        m_sched_d.m_account_id = account_n->m_id;
    }
}

void SchedDialog::OnFocusChange(wxChildFocusEvent& event)
{
    switch (w_focus)
    {
    case mmID_ACCOUNTNAME:
        w_account_cb->ChangeValue(w_account_cb->GetValue());
        if (w_account_cb->mmIsValid()) {
            m_sched_d.m_account_id = w_account_cb->mmGetId();
            SetAmountCurrencies(m_sched_d.m_account_id, -1);
        }
        break;
    case mmID_TOACCOUNTNAME:
        w_to_account_cb->ChangeValue(w_to_account_cb->GetValue());
        if (w_to_account_cb->mmIsValid()) {
            m_sched_d.m_to_account_id_n = w_to_account_cb->mmGetId();
            SetAmountCurrencies(-1, m_sched_d.m_to_account_id_n);
        }
        break;
    case mmID_PAYEE:
        w_payee_cb->ChangeValue(w_payee_cb->GetValue());
        break;
    case mmID_CATEGORY:
        w_category_cb->ChangeValue(w_category_cb->GetValue());
        if (w_category_cb->mmIsValid()) {
            m_sched_d.m_category_id_n = w_category_cb->mmGetCategoryId();
        }
        break;
    case ID_DIALOG_TRANS_TEXTAMOUNT:
        w_calculator_text = w_amount_text;
        break;
    case ID_DIALOG_TRANS_TOTEXTAMOUNT:
        w_calculator_text = w_to_amount_text;
        break;

    }

    wxWindow* w = event.GetWindow();
    if (w) {
        w_focus = w->GetId();
    }

    if (w_amount_text->Calculate()) {
        w_amount_text->GetDouble(m_sched_d.m_amount);
        w_amount_text->SelectAll();
    }
    if (m_advanced && w_to_amount_text->Calculate()) {
        w_to_amount_text->GetDouble(m_sched_d.m_to_amount);
        w_to_amount_text->SelectAll();
    }
}

void SchedDialog::OnCalculator(wxCommandEvent& WXUNUSED(event))
{
    w_calculator->SetTarget(w_calculator_text);
    w_calculator->Popup();
}
