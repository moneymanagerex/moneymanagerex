/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011-2022 Nikolay Akimov
 Copyright (C) 2011-2017 Stefano Giorgio [stef145g]
 Copyright (C) 2021-2026 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025-2026 Klaus Wich

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

#include "TrxDialog.h"

#include "base/_defs.h"
#include <wx/numformatter.h>
#include <wx/timectrl.h>
#include <wx/collpane.h>
#include <wx/display.h>

#include "base/_constants.h"
#include "util/mmPath.h"
#include "util/mmImage.h"
#include "util/mmAttachment.h"
#include "util/mmTextCtrl.h"
#include "util/mmCalcValidator.h"
#include "util/_util.h"
#include "util/_simple.h"

#include "model/AccountModel.h"
#include "model/AttachmentModel.h"
#include "model/CategoryModel.h"
#include "model/CurrencyHistoryModel.h"
#include "model/FieldValueModel.h"
#include "model/PrefModel.h"
#include "model/SettingModel.h"
#include "model/TagModel.h"

#include "manager/CategoryManager.h"
#include "manager/PayeeManager.h"

#include "AttachmentDialog.h"
#include "FieldValueDialog.h"
#include "SplitDialog.h"

#include "pref/ViewPref.h"

#include "import_export/webapp.h"

wxIMPLEMENT_DYNAMIC_CLASS(TrxDialog, wxDialog);

wxBEGIN_EVENT_TABLE(TrxDialog, wxDialog)
    EVT_CHAR_HOOK(                                  TrxDialog::onComboKey)
    EVT_CHILD_FOCUS(                                TrxDialog::onFocusChange)
    EVT_COMBOBOX(ID_DIALOG_PAYEE,                   TrxDialog::onPayeeChanged)
    EVT_TEXT(ID_DIALOG_PAYEE,                       TrxDialog::onPayeeChanged)
    EVT_BUTTON(mmID_CATEGORY_SPLIT,                 TrxDialog::onCategs)
    EVT_CHOICE(ID_DIALOG_TRANS_TYPE,                TrxDialog::onTransTypeChanged)
    EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, TrxDialog::onAdvanceChecked)
    EVT_BUTTON(wxID_FILE,                           TrxDialog::onAttachments)
    EVT_BUTTON(ID_DIALOG_TRANS_CUSTOMFIELDS,        TrxDialog::onMoreFields)
    EVT_BUTTON(wxID_OK,                             TrxDialog::onOk)
    EVT_BUTTON(ID_BTN_OK_NEW,                       TrxDialog::onOk)
    EVT_BUTTON(wxID_CANCEL,                         TrxDialog::onCancel)
    EVT_BUTTON(ID_DIALOG_TRANS_TODAY,               TrxDialog::onToday)
    EVT_CLOSE(                                      TrxDialog::onQuit)
    EVT_MENU(wxID_SAVE,                             TrxDialog::onOk)
    EVT_MENU(ID_BTN_OK_NEW,                         TrxDialog::onOk)
    EVT_MENU(ID_DIALOG_TRANS_TODAY,                 TrxDialog::onToday)
wxEND_EVENT_TABLE()

// -- static

wxDateTime TrxDialog::s_previousDate;

// -- constructor

TrxDialog::TrxDialog(
    wxWindow* parent_win,
    JournalKey journal_key,
    bool duplicate,
    int64 account_id,
    TrxType type
) {
    SetEvtHandlerEnabled(false);

    if (journal_key.is_initialized() &&
        Journal::setJournalData(m_journal_d, journal_key)
    ) {
        // a bill can only be duplicated
        m_mode =
            m_journal_d.key().is_scheduled() ? MODE_ENTER :
            duplicate                        ? MODE_ADD   :
                                               MODE_UPDATE;

        for (const auto& tp_d : Journal::split(m_journal_d)) {
            wxArrayInt64 tag_id_a;
            for (const TagLinkData& gl_d : TagLinkModel::instance().find_data_a(
                TagLinkCol::WHERE_REFID(OP_EQ, tp_d.m_id))
            )
                tag_id_a.push_back(gl_d.m_tag_id);
            m_split_a.push_back({
                tp_d.m_category_id, tp_d.m_amount, tp_d.m_notes, tag_id_a
            });
        }

        if (!is_edit() &&
            !SettingModel::instance().getBool(INIDB_USE_ORG_DATE_DUPLICATE, false)
        ) {
            // Use the empty transaction logic to generate the new date to be used
            TrxData emptyTrx;
            TrxModel::instance().setEmptyData(emptyTrx, account_id);
            m_journal_d.m_datetime = emptyTrx.m_datetime;
        }

        // TODO: Avoid premature clone of attachments.
        // If duplicate then we may need to copy the attachments
        if (!is_edit() && InfoModel::instance().getBool("ATTACHMENTSDUPLICATE", false)) {
            mmAttachment::clone_ref_all(
                m_journal_d.key().ref_type(), m_journal_d.key().ref_id(),
                TrxModel::s_ref_type, 0
            );
        }
    }
    else {
        m_mode = MODE_ADD;
        TrxModel::instance().setEmptyData(m_journal_d, account_id);
        m_journal_d.m_type = type;
    }

    m_advanced = m_journal_d.is_transfer() &&
        m_journal_d.m_amount != m_journal_d.m_to_amount;

    w_fv_dlg = new FieldValueDialog(this,
        m_journal_d.key().ref_type(),
        m_journal_d.key().ref_id(),
        ID_CUSTOMFIELDS
    );

    this->SetFont(parent_win->GetFont());
    create(parent_win);
    mmThemeAutoColour(this);
    dataToControls();

    mmSetSize(this);
    // set the initial dialog size to expand the payee and category comboboxes to fit their text
    int minWidth = std::max(
        0,
        w_payee_text->GetSizeFromText(w_payee_text->GetValue()).GetWidth() -
            2 * w_payee_text->GetMinWidth()
    );
    minWidth = std::max(
        minWidth,
        w_cat_text->GetSizeFromText(w_cat_text->GetValue()).GetWidth() -
            2 * w_cat_text->GetMinWidth()
    );

    int custom_fields_width = w_fv_dlg->IsCustomPanelShown()
        ? w_fv_dlg->GetMinWidth()
        : 0;
    wxSize size = wxSize(
        GetMinWidth() + minWidth + custom_fields_width,
        GetSize().GetHeight()
    );
    if (size.GetWidth() > GetSize().GetWidth())
        SetSize(size);
    if (custom_fields_width)
        SetMinSize(wxSize(
            GetMinWidth() + w_fv_dlg->GetMinWidth(),
            GetMinHeight()
        ));
    Centre();
    SetEvtHandlerEnabled(true);
}

TrxDialog::~TrxDialog()
{
    wxSize size = GetSize();
    if (w_fv_dlg->IsCustomPanelShown())
        size = wxSize(
            GetSize().GetWidth() - w_fv_dlg->GetMinWidth(),
            GetSize().GetHeight()
        );
    InfoModel::instance().saveSize("TRANSACTION_DIALOG_SIZE", size);
}

bool TrxDialog::create(
    wxWindow* parent_win,
    wxWindowID win_id,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
) {
    style |= wxRESIZE_BORDER;
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent_win, win_id, caption, pos, size, style, name);

    SetEvtHandlerEnabled(false);
    createControls();

    wxString header =
        is_new()                  ? _t("New Transaction") :
        is_dup()                  ? _t("Duplicate Transaction") :
        is_enter()                ? _t("Enter Transaction") :
        !m_journal_d.is_deleted() ? _t("Edit Transaction") :
                                    "";  //_t("View Deleted Transaction");
    setDialogTitle(header);

    SetIcon(mmPath::getProgramIcon());

    setEventHandlers();
    SetEvtHandlerEnabled(true);
    return true;
}

void TrxDialog::createControls()
{
    wxBoxSizer* box_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* box_sizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* box_sizer2 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* box_sizer3 = new wxBoxSizer(wxHORIZONTAL);
    box_sizer->Add(box_sizer1, g_flagsExpand);
    box_sizer1->Add(box_sizer2, wxSizerFlags(g_flagsExpand).Border(0));
    box_sizer1->Add(box_sizer3, wxSizerFlags(g_flagsV).Expand().Border(0));

    wxStaticBox* static_box = new wxStaticBox(this, wxID_ANY, _t("Transaction Details"));
    wxStaticBoxSizer* box_sizer_left = new wxStaticBoxSizer(static_box, wxVERTICAL);
    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 3, 0, 0);
    flex_sizer->AddGrowableCol(1, 0);
    box_sizer_left->Add(flex_sizer, wxSizerFlags(g_flagsV).Expand());
    box_sizer2->Add(box_sizer_left, g_flagsExpand);

    wxFont bold = this->GetFont().Bold();
    // Date -------------------------------------------
    wxStaticText* name_label = new wxStaticText(static_box, wxID_STATIC, _t("Date"));
    flex_sizer->Add(name_label, g_flagsH);
    name_label->SetFont(bold);

    w_date_picker = new mmDatePicker(static_box, ID_DIALOG_TRANS_BUTTONDATE);
    flex_sizer->Add(w_date_picker->mmGetLayout());

    wxBitmapBundle bundle = mmImage::bitmapBundle(mmImage::png::ACC_CLOCK, mmImage::bitmapButtonSize);
    wxBitmapButton* today = new wxBitmapButton(static_box, ID_DIALOG_TRANS_TODAY, bundle);
    today->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrxDialog::onToday), nullptr, this);

    const wxString tooltiptext = _t("Set date to today (Ctrl+;)");
    mmToolTip(today, tooltiptext);
    wxChar hotkeyToday = ExtractHotkeyChar(tooltiptext, '.');
    flex_sizer->Add(today, g_flagsH);

    // Type --------------------------------------------
    w_type_choice = new wxChoice(static_box, ID_DIALOG_TRANS_TYPE);

    std::size_t account_c = AccountModel::instance().find_count();
    for (int i = 0; i < TrxType::size; ++i) {
        if (i == TrxType::e_transfer && account_c < 2)
            continue;
        wxString type_name = TrxType(i).name();
        w_type_choice->Append(
            wxGetTranslation(type_name),
            new wxStringClientData(type_name)
        );
    }

    w_advanced_cb = new wxCheckBox(static_box,
        ID_DIALOG_TRANS_ADVANCED_CHECKBOX,
        _t("&Advanced"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );

    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);

    flex_sizer->Add(new wxStaticText(static_box, wxID_STATIC, _t("Type")), g_flagsH);
    flex_sizer->Add(typeSizer, wxSizerFlags(g_flagsExpand).Border(0));
    typeSizer->Add(w_type_choice, g_flagsExpand);
    typeSizer->Add(w_advanced_cb, g_flagsH);
    flex_sizer->AddSpacer(1);

    // Amount Fields --------------------------------------------
    w_amount_text = new mmTextCtrl(static_box, mmID_TEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    w_amount_text->SetMinSize(w_amount_text->GetSize());
    w_to_amount_text = new mmTextCtrl( static_box, mmID_TOTEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    w_to_amount_text->SetMinSize(w_to_amount_text->GetSize());
    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(w_amount_text, g_flagsExpand);
    amountSizer->Add(w_to_amount_text, g_flagsExpand);

    wxStaticText* amount_label = new wxStaticText(static_box, wxID_STATIC, _t("Amount"));
    amount_label->SetFont(bold);
    flex_sizer->Add(amount_label, g_flagsH);
    flex_sizer->Add(amountSizer, wxSizerFlags(g_flagsExpand).Border(0));

    w_calc_btn = new wxBitmapButton(static_box, wxID_ANY, mmImage::bitmapBundle(mmImage::png::CALCULATOR, mmImage::bitmapButtonSize));
    w_calc_btn->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrxDialog::onCalculator), nullptr, this);
    mmToolTip(w_calc_btn, _t("Open Calculator"));
    flex_sizer->Add(w_calc_btn, g_flagsH);
    w_calc_text = w_amount_text;
    w_calc_popup = new mmCalcPopup(w_calc_btn, w_calc_text);

    // Account ---------------------------------------------
    w_account_label = new wxStaticText(static_box, wxID_STATIC, _t("Account"));
    w_account_label->SetFont(bold);

    w_account_text = new mmComboBoxAccount(static_box, mmID_ACCOUNTNAME,
        wxDefaultSize,
        m_journal_d.m_account_id
    );
    w_account_text->SetMinSize(w_account_text->GetSize());
    flex_sizer->Add(w_account_label, g_flagsH);
    flex_sizer->Add(w_account_text, g_flagsExpand);

    w_invert_btn = new wxBitmapButton(static_box, wxID_ANY,
        mmImage::bitmapBundle(mmImage::png::UPDATE, mmImage::bitmapButtonSize)
    );
    w_invert_btn->Connect(wxID_ANY,
        wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(TrxDialog::onSwitch),
        nullptr, this
    );
    mmToolTip(w_invert_btn, _t("Exchange to and from accounts"));
    flex_sizer->Add(w_invert_btn, g_flagsH);

    // To Account ------------------------------------------------
    w_to_account_label = new wxStaticText(static_box, mmID_TOACCOUNT_LABEL, _t("To"));
    w_to_account_label->SetFont(bold);
    flex_sizer->Add(w_to_account_label, g_flagsH);
    w_to_account_text = new mmComboBoxAccount(static_box, mmID_TOACCOUNTNAME,
        wxDefaultSize,
        m_journal_d.m_to_account_id_n
    );
    w_to_account_text->SetMinSize(w_to_account_text->GetSize());
    flex_sizer->Add(w_to_account_text, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // Payee ---------------------------------------------
    w_payee_label = new wxStaticText(static_box, ID_DIALOG_PAYEE_LABEL, _t("Payee"));
    w_payee_label->SetFont(bold);

    w_payee_text = new mmComboBoxPayee(static_box, ID_DIALOG_PAYEE,
        wxDefaultSize,
        m_journal_d.m_payee_id_n,
        true
    );
    w_payee_text->SetMinSize(w_payee_text->GetSize());
    flex_sizer->Add(w_payee_label, g_flagsH);
    flex_sizer->Add(w_payee_text, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // Category -------------------------------------------------
    w_cat_label = new wxStaticText(static_box, ID_DIALOG_TRANS_CATEGLABEL2, _t("Category"));
    w_cat_label->SetFont(bold);
    w_cat_text = new mmComboBoxCategory(static_box, mmID_CATEGORY,
        wxDefaultSize,
        m_journal_d.m_category_id_n,
        true
    );
    w_cat_text->SetMinSize(w_cat_text->GetSize());
    w_split_btn = new wxBitmapButton(static_box, mmID_CATEGORY_SPLIT,
        mmImage::bitmapBundle(mmImage::png::NEW_TRX, mmImage::bitmapButtonSize)
    );
    mmToolTip(w_split_btn, _t("Use split Categories"));

    flex_sizer->Add(w_cat_label, g_flagsH);
    flex_sizer->Add(w_cat_text, g_flagsExpand);
    flex_sizer->Add(w_split_btn, g_flagsH);

    // Tags  ---------------------------------------------
    w_tag_text = new mmTagTextCtrl(static_box, ID_DIALOG_TRANS_TAGS);
    wxStaticText* tagLabel = new wxStaticText(static_box, wxID_STATIC, _t("Tags"));
    flex_sizer->Add(tagLabel, g_flagsH);
    flex_sizer->Add(w_tag_text, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // Status --------------------------------------------
    w_status_choice = new wxChoice(static_box, ID_DIALOG_TRANS_STATUS);

    for (int i = 0; i < TrxStatus::size; ++i) {
        wxString status_name = TrxStatus(i).name();
        w_status_choice->Append(
            wxGetTranslation(status_name),
            new wxStringClientData(status_name)
        );
    }

    flex_sizer->Add(new wxStaticText(static_box, wxID_STATIC, _t("Status")), g_flagsH);
    flex_sizer->Add(w_status_choice, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // Number  ---------------------------------------------
    w_number_text = new wxTextCtrl(static_box, ID_DIALOG_TRANS_TEXTNUMBER,
        "",
        wxDefaultPosition, wxDefaultSize
    );

    w_auto_btn = new wxBitmapButton(static_box, ID_DIALOG_TRANS_BUTTONTRANSNUM,
        mmImage::bitmapBundle(mmImage::png::TRXNUM, mmImage::bitmapButtonSize)
    );
    w_auto_btn->Connect(ID_DIALOG_TRANS_BUTTONTRANSNUM,
        wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(TrxDialog::onAutoTransNum),
        nullptr, this
    );
    mmToolTip(w_auto_btn, _t("Populate Transaction #"));

    flex_sizer->Add(new wxStaticText(static_box, wxID_STATIC, _t("Number")), g_flagsH);
    flex_sizer->Add(w_number_text, g_flagsExpand);
    flex_sizer->Add(w_auto_btn, g_flagsH);

    // Frequently Used Notes
    wxButton* bFrequentUsedNotes = new wxButton(static_box,
        ID_DIALOG_TRANS_BUTTON_FREQENTNOTES,
        "...",
        wxDefaultPosition, w_auto_btn->GetSize(),
        0
    );
    mmToolTip(bFrequentUsedNotes, _t("Select one of the frequently used notes"));
    bFrequentUsedNotes->Connect(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES,
        wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(TrxDialog::onFrequentUsedNotes),
        nullptr, this
    );

    // Colors
    w_color_btn = new mmColorButton(static_box, wxID_LOWEST, w_auto_btn->GetSize());
    mmToolTip(w_color_btn, _t("User Colors"));
    w_color_btn->SetColor(m_journal_d.m_color.GetValue());

    // Attachments
    w_att_btn = new wxBitmapButton(static_box, wxID_FILE,
        mmImage::bitmapBundle(mmImage::png::CLIP, mmImage::bitmapButtonSize)
    );
    mmToolTip(w_att_btn, _t("Manage transaction attachments"));

    // Now display the Frequently Used Notes, Colour, Attachment buttons
    wxBoxSizer* notes_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(notes_sizer);
    notes_sizer->Add(new wxStaticText(static_box, wxID_STATIC, _t("Notes")), g_flagsH);
    notes_sizer->Add(bFrequentUsedNotes, g_flagsH);

    wxBoxSizer* RightAlign_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(RightAlign_sizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));
    RightAlign_sizer->Add(new wxStaticText(static_box, wxID_STATIC, _t("Color")), g_flagsH);
    RightAlign_sizer->Add(w_color_btn, wxSizerFlags());
    flex_sizer->Add(w_att_btn, g_flagsH);

    // Notes
    w_notes_text = new wxTextCtrl(static_box,
        ID_DIALOG_TRANS_TEXTNOTES,
        "",
        wxDefaultPosition, wxSize(-1, w_date_picker->GetSize().GetHeight() * 5),
        wxTE_MULTILINE
    );
    mmToolTip(w_notes_text, _t("Specify any text notes you want to add to this transaction."));
    box_sizer_left->Add(
        w_notes_text,
        wxSizerFlags(g_flagsExpand).Border(wxLEFT | wxRIGHT | wxBOTTOM, 10)
    );

    // Button Panel with OK and Cancel Buttons
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    box_sizer2->Add(buttons_panel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 0));

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxBitmapButton* button_hide = new wxBitmapButton(buttons_panel,
        ID_DIALOG_TRANS_CUSTOMFIELDS,
        mmImage::bitmapBundle(mmImage::png::RIGHTARROW, mmImage::bitmapButtonSize)
    );
    mmToolTip(button_hide, _t("Show/Hide custom fields window"));
    if (w_fv_dlg->GetCustomFieldsCount() == 0) {
        button_hide->Hide();
    }

    buttons_sizer->Add(
        new wxButton(buttons_panel, wxID_OK, _t("&Save")),
        wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10)
    );
    if (!is_edit()) {
        buttons_sizer->Add(
            new wxButton(buttons_panel, ID_BTN_OK_NEW, _t("Save and &New")),
            wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10)
        );
    }
    w_cancel_btn = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    buttons_sizer->Add(w_cancel_btn, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    buttons_sizer->Add(button_hide, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));

    if (is_edit())
        w_cancel_btn->SetFocus();
    mmThemeAutoColour(buttons_panel, true);
    buttons_sizer->Realize();

    // Custom fields -----------------------------------
    w_fv_dlg->FillCustomFields(box_sizer3);
    if (w_fv_dlg->GetActiveCustomFieldsCount() > 0) {
        wxCommandEvent evt(wxEVT_BUTTON, ID_DIALOG_TRANS_CUSTOMFIELDS);
        onMoreFields(evt);
    }

    this->SetSizerAndFit(box_sizer);
    wxSize panelSize = box_sizer2->GetMinSize();
    w_min_size = wxSize(panelSize.GetWidth() + 20, panelSize.GetHeight());
    SetMinSize(w_min_size);
    box_sizer3->SetMinSize(panelSize);
    w_fv_dlg->SetMinSize(panelSize);

    const wxAcceleratorEntry entries[] = {
        wxAcceleratorEntry(wxACCEL_CTRL, static_cast<int>('S'), wxID_SAVE),
        wxAcceleratorEntry(wxACCEL_CTRL, static_cast<int>('N'), ID_BTN_OK_NEW),
        wxAcceleratorEntry(wxACCEL_CTRL, static_cast<int>(hotkeyToday), ID_DIALOG_TRANS_TODAY),
    };
    wxAcceleratorTable tab(sizeof(entries) / sizeof(*entries), entries);
    SetAcceleratorTable(tab);
}

void TrxDialog::dataToControls()
{
    // short names for model instances
    CurrencyModel& U  = CurrencyModel::instance();
    AccountModel&  A  = AccountModel::instance();
    PayeeModel&    P  = PayeeModel::instance();
    TrxModel&      T  = TrxModel::instance();
    TagLinkModel&  GL = TagLinkModel::instance();

    TrxModel::instance().getFrequentUsedNotes(m_freq_notes_a, m_journal_d.m_account_id);
    wxButton* freq_notes_btn = static_cast<wxButton*>(
        FindWindow(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES)
    );
    freq_notes_btn->Enable(!m_freq_notes_a.empty());

    // Date
    if (!m_skip_date_init) {
        wxDateTime trx_date;
        if (TrxDialog::s_previousDate.IsValid()) {
            trx_date = TrxDialog::s_previousDate;
        }
        else {
            trx_date = m_journal_d.m_datetime.dateTime(
                PrefModel::instance().getUseTransDateTime()
            );
        }
        w_date_picker->setValue(trx_date);
        w_date_picker->SetFocus();
        m_skip_date_init = true;
    }

    // Status
    if (!m_skip_status_init) {
        bool useOriginalState = (
            is_new() || is_edit() ||
            SettingModel::instance().getBool(INIDB_USE_ORG_STATE_DUPLICATE_PASTE, false)
        );
        TrxStatus status = useOriginalState
            ? m_journal_d.m_status
            : PrefModel::instance().getTrxStatus();
        w_status_choice->SetSelection(status.id());
        m_skip_status_init = true;
    }

    // Type
    w_type_choice->SetSelection(m_journal_d.m_type.id());

    // Account
    if (!m_skip_account_init) {
        const AccountData* acc_n = A.get_idN_data_n(m_journal_d.m_account_id);
        if (acc_n) {
            w_account_text->ChangeValue(acc_n->m_name);
            w_amount_text->SetCurrency(U.get_idN_data_n(acc_n->m_currency_id));
        }
        const AccountData* to_acc_n = A.get_idN_data_n(m_journal_d.m_to_account_id_n);
        if (to_acc_n) {
            w_to_account_text->ChangeValue(to_acc_n->m_name);
            w_to_amount_text->SetCurrency(U.get_idN_data_n(to_acc_n->m_currency_id));
        }

        m_skip_account_init = true;
        // Force amount format update in case account currencies change
        m_skip_amount_init = false;
    }

    bool is_transfer = m_journal_d.is_transfer();
    if (is_transfer) {
        w_account_label->SetLabelText(_t("From"));
        w_payee_label->SetLabelText(_t("To"));
    }
    else if (!m_journal_d.is_deposit()) {
        w_account_label->SetLabelText(_t("Account"));
        w_payee_label->SetLabelText(_t("Payee"));
    }
    else {
        w_account_label->SetLabelText(_t("Account"));
        w_payee_label->SetLabelText(_t("From"));
    }

    // Advanced
    w_advanced_cb->Show(is_transfer);
    w_advanced_cb->SetValue(m_advanced && is_transfer);
    w_to_amount_text->Show(m_advanced && is_transfer);

    w_invert_btn->Show(is_transfer);

    // Amounts
    if (!m_skip_amount_init) {
        if (is_transfer && m_advanced)
            w_to_amount_text->SetValue(m_journal_d.m_to_amount);
        else
            w_to_amount_text->ChangeValue("");

        if (!is_new())
            w_amount_text->SetValue(m_journal_d.m_amount);
        m_skip_amount_init = true;
    }

    // Payee
    if (!m_skip_payee_init) {
        w_payee_text->SetEvtHandlerEnabled(false);

        w_account_text->UnsetToolTip();
        w_payee_text->UnsetToolTip();
        if (!is_transfer) {
            if (!TrxModel::is_foreign(m_journal_d)) {
                m_journal_d.m_to_account_id_n = -1;
            }

            int64 account_id = w_account_text->mmGetId();
            if (is_new() &&
                PrefModel::instance().getTransPayeeNone() == PrefModel::LASTUSED &&
                account_id != -1
            ) {
                for (const TrxData& trx_d : T.find_data_a(
                    TrxModel::WHERE_TYPE(OP_NE, TrxType(TrxType::e_transfer)),
                    TrxCol::WHERE_ACCOUNTID(OP_EQ, account_id),
                    TableClause::ORDERBY(TrxCol::NAME_TRANSID, true),
                    TableClause::LIMIT(1)
                )) {
                    const PayeeData* payee_n = P.get_idN_data_n(trx_d.m_payee_id_n);
                    w_payee_text->ChangeValue(payee_n->m_name);
                    break;
                }
            }
            else if (is_new() &&
                PrefModel::instance().getTransPayeeNone() == PrefModel::UNUSED
            ) {
                const PayeeData* payee_n = P.get_name_data_n(_t("Unknown"));
                if (!payee_n) {
                    PayeeData new_payee_d = PayeeData();
                    new_payee_d.m_name = _t("Unknown");
                    P.add_data_n(new_payee_d);
                    w_payee_text->mmDoReInitialize();
                }

                w_payee_text->ChangeValue(_t("Unknown"));
            }
            else {
                const PayeeData* payee_n = P.get_idN_data_n(
                    m_journal_d.m_payee_id_n
                );
                if (payee_n)
                    w_payee_text->ChangeValue(payee_n->m_name);
            }

            setCategoryForPayee();
        }
        m_skip_payee_init = true;
        w_payee_text->SetEvtHandlerEnabled(true);
    }

    w_payee_text->Show(!is_transfer);
    w_payee_label->Show(!is_transfer);
    w_to_account_label->Show(is_transfer);
    w_to_account_text->Show(is_transfer);
    Layout();

    bool has_split = !m_split_a.empty();
    if (!m_skip_category_init) {
        w_split_btn->UnsetToolTip();
        if (has_split) {
            w_cat_text->ChangeValue(_t("Split Transaction"));
            w_cat_text->Disable();
            w_amount_text->SetValue(TrxSplitModel::instance().get_total(m_split_a));
            m_journal_d.m_category_id_n = -1;
        }
        else if (is_new() && is_transfer &&
            PrefModel::instance().getTransCategoryTransferNone() == PrefModel::LASTUSED
        ) {
            for (const TrxData& trx_d : T.find_data_a(
                TrxModel::WHERE_TYPE(OP_EQ, TrxType(TrxType::e_transfer)),
                TableClause::ORDERBY(TrxCol::NAME_TRANSID, true),
                TableClause::LIMIT(1)
            )) {
                if (CategoryModel::instance().get_id_active(trx_d.m_category_id_n)) {
                    int64 cat_id = trx_d.m_category_id_n;
                    const wxString cat_fullname = CategoryModel::instance().get_id_fullname(
                        cat_id
                    );
                    w_cat_text->ChangeValue(cat_fullname);
                }
                break;
            }
        }
        else {
            int64 cat_id = m_journal_d.m_category_id_n;
            const wxString cat_fullname = CategoryModel::instance().get_id_fullname(cat_id);
            w_cat_text->ChangeValue(cat_fullname);
        }
        m_skip_category_init = true;
    }

    w_amount_text->Enable(!has_split);
    w_calc_btn->Enable(!has_split);
    w_cat_text->Enable(!has_split);
    w_split_btn->Enable(!is_transfer);

    // Tags
    if (!m_skip_tag_init) {
        wxArrayInt64 tag_id_a;
        for (const auto& gl_d : GL.find_data_a(
            TagLinkCol::WHERE_REFTYPE(OP_EQ, m_journal_d.key().ref_type().key_n()),
            TagLinkCol::WHERE_REFID(OP_EQ, m_journal_d.key().ref_id())
        )) {
            tag_id_a.push_back(gl_d.m_tag_id);
        }
        w_tag_text->SetTags(tag_id_a);
        m_skip_tag_init = true;
    }

    // Notes & Transaction Number
    if (!m_skip_notes_init) {
        w_number_text->SetValue(m_journal_d.m_number);
        w_notes_text->SetValue(m_journal_d.m_notes);
        m_skip_notes_init = true;
    }

    if (!m_skip_tooltips_init)
        setTooltips();

    if (m_journal_d.is_deleted()) {
        w_date_picker->Enable(false);
        w_type_choice->Enable(false);
        w_account_text->Enable(false);
        w_status_choice->Enable(false);
        w_amount_text->Enable(false);
        w_calc_btn->Enable(false);
        w_to_account_text->Enable(false);
        w_to_amount_text->Enable(false);
        w_advanced_cb->Enable(false);
        w_payee_text->Enable(false);
        w_cat_text->Enable(false);
        w_tag_text->Enable(false);
        w_split_btn->Enable(false);
        w_auto_btn->Enable(false);
        w_number_text->Enable(false);
        w_notes_text->Enable(false);
        w_color_btn->Enable(false);
        w_att_btn->Enable(false);
        freq_notes_btn->Enable(false);
    }
}

// -- methods

void TrxDialog::setDialogTitle(const wxString& title)
{
    this->SetTitle(title);
}

bool TrxDialog::validateData()
{
    if (!w_amount_text->checkValue(m_journal_d.m_amount))
        return false;

    if (!w_tag_text->IsValid()) {
        mmErrorDialogs::ToolTip4Object(w_tag_text,
            _t("Invalid value"),
            _t("Tags"),
            wxICON_ERROR
        );
        return false;
    }

    if (!w_account_text->mmIsValid()) {
        mmErrorDialogs::ToolTip4Object(w_account_text,
            _t("Invalid value"),
            _t("Account"),
            wxICON_ERROR
        );
        return false;
    }

    m_journal_d.m_account_id = w_account_text->mmGetId();
    const AccountData* account_n = AccountModel::instance().get_idN_data_n(
        m_journal_d.m_account_id
    );

    if (m_journal_d.m_date() < account_n->m_open_date) {
        mmErrorDialogs::ToolTip4Object(
            w_account_text,
            _t("The opening date for the account is later than the date of this transaction"),
            _t("Invalid Date")
        );
        return false;
    }

    if (m_split_a.empty()) {
        if (!w_cat_text->mmIsValid()) {
            mmErrorDialogs::ToolTip4Object(w_cat_text,
                _t("Invalid value"),
                _t("Category"),
                wxICON_ERROR
            );
            return false;
        }
        m_journal_d.m_category_id_n = w_cat_text->mmGetCategoryId();
    }

    if (!m_journal_d.is_transfer()) {
        wxString payee_name = w_payee_text->GetValue();
        if (payee_name.IsEmpty()) {
            mmErrorDialogs::InvalidPayee(w_payee_text);
            return false;
        }

        // Get payee string from populated list to address issues with
        // case compare differences between autocomplete and payee list
        int payee_loc = w_payee_text->FindString(payee_name);
        if (payee_loc != wxNOT_FOUND)
            payee_name = w_payee_text->GetString(payee_loc);

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
                payee_n = PayeeModel::instance().get_idN_data_n(new_payee_d.m_id);
                mmWebApp::uploadPayee();
            }
            else
                return false;
        }
        m_journal_d.m_to_amount = m_journal_d.m_amount;
        m_journal_d.m_payee_id_n = payee_n->m_id;
        if (!TrxModel::is_foreign(m_journal_d)) {
            m_journal_d.m_to_account_id_n = -1;
        }

        if (PrefModel::instance().getTransCategoryNone() == PrefModel::LASTUSED &&
            CategoryModel::instance().get_id_active(m_journal_d.m_category_id_n)
        ) {
            PayeeData payee_d = *payee_n;
            payee_d.m_category_id_n = m_journal_d.m_category_id_n;
            PayeeModel::instance().save_data_n(payee_d);
            mmWebApp::uploadPayee();
        }
    }
    // transfer
    else {
        const AccountData* to_account_n = AccountModel::instance().get_name_data_n(
            w_to_account_text->GetValue()
        );

        if (!to_account_n || to_account_n->m_id == m_journal_d.m_account_id) {
            mmErrorDialogs::InvalidAccount(w_to_account_text, true);
            return false;
        }
        m_journal_d.m_to_account_id_n = to_account_n->m_id;

        if (m_journal_d.m_date() < to_account_n->m_open_date) {
            mmErrorDialogs::ToolTip4Object(w_to_account_text,
                _t("The opening date for the account is later than the date of this transaction"),
                _t("Invalid Date")
            );
            return false;
        }

        if (m_advanced) {
            if (!w_to_amount_text->checkValue(m_journal_d.m_to_amount))
                return false;
        }
        m_journal_d.m_payee_id_n = -1;
    }

    // Check if transaction is to proceed.
    if (account_n->is_locked_for(mmDate(w_date_picker->GetValue()))) {
        if (wxMessageBox(wxString::Format(
            _t("Lock transaction to date: %s") + "\n\n" + _t("Do you want to continue?"),
            mmGetDateTimeForDisplay(account_n->m_stmt_date_n.isoDateN())),
            _t("MMEX Transaction Check"),
            wxYES_NO | wxICON_WARNING
        ) == wxNO) {
            return false;
        }
    }

    // Validate account limits
    if (!is_edit() &&
        !m_journal_d.is_void() &&
        !m_journal_d.is_deposit() &&
        (account_n->m_min_balance != 0 || account_n->m_credit_limit != 0)
    ) {
        const double balance = AccountModel::instance().get_data_balance(*account_n);
        const double new_balance = balance - m_journal_d.m_amount;

        bool abort_transaction =
            (account_n->m_min_balance != 0 && new_balance < account_n->m_min_balance) ||
            (account_n->m_credit_limit != 0 && new_balance < -(account_n->m_credit_limit));

        if (abort_transaction && wxMessageBox(
            _t("The transaction will exceed the account limit.") + "\n\n" +
                _t("Do you want to continue?"),
            _t("MMEX Transaction Check"),
            wxYES_NO | wxICON_WARNING
        ) == wxNO) {
            return false;
        }
    }

    int color_id = w_color_btn->GetColorId();
    if (color_id > 0 && color_id < 8)
        m_journal_d.m_color = color_id;
    else
        m_journal_d.m_color = -1;

    return true;
}

void TrxDialog::setEventHandlers()
{
    w_amount_text->Connect(
        mmID_TEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(TrxDialog::onTextEntered),
        nullptr, this
    );
    w_to_amount_text->Connect(
        mmID_TOTEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(TrxDialog::onTextEntered),
        nullptr, this
    );

#ifdef __WXGTK__ // Workaround for bug http://trac.wxwidgets.org/ticket/11630
    w_date_picker->Connect(
        ID_DIALOG_TRANS_BUTTONDATE, wxEVT_KILL_FOCUS,
        wxFocusEventHandler(TrxDialog::onDpcKillFocus),
        nullptr, this
    );
#endif
}

void TrxDialog::setTooltips()
{
    w_split_btn->UnsetToolTip();
    m_skip_tooltips_init = true;
    if (m_split_a.empty())
        mmToolTip(w_split_btn, _t("Use split Categories"));
    else {
        const AccountData* account_n = AccountModel::instance().get_idN_data_n(
            m_journal_d.m_account_id
        );
        const CurrencyData* currency_n = account_n
            ? AccountModel::instance().get_data_currency_p(*account_n)
            : CurrencyModel::instance().get_base_data_n();

        w_split_btn->SetToolTip(
            TrxSplitModel::instance().get_tooltip(m_split_a, currency_n)
        );
    }
    if (!is_new())
        return;

    w_amount_text->UnsetToolTip();
    w_to_amount_text->UnsetToolTip();
    w_account_text->UnsetToolTip();
    w_payee_text->UnsetToolTip();

    if (m_journal_d.is_transfer()) {
        mmToolTip(w_account_text, _t("Specify account the money is taken from"));
        mmToolTip(w_payee_text, _t("Specify account the money is moved to"));
        mmToolTip(w_amount_text, _t("Specify the transfer amount in the From Account."));

        if (m_advanced)
            mmToolTip(w_to_amount_text, _t("Specify the transfer amount in the To Account"));
    }
    else {
        mmToolTip(w_amount_text, _t("Specify the amount for this transaction"));
        mmToolTip(w_account_text, _t("Specify account for the transaction"));
        if (!m_journal_d.is_deposit())
            mmToolTip(w_payee_text, _t("Specify to whom the transaction is going to"));
        else
            mmToolTip(w_payee_text, _t("Specify where the transaction is coming from"));
    }

    // Not dynamically changed tooltips
    mmToolTip(w_date_picker, _t("Specify the date of the transaction"));
    mmToolTip(w_status_choice, _t("Specify the status for the transaction"));
    mmToolTip(w_type_choice, _t("Specify the type of transactions to be created."));
    mmToolTip(w_number_text, _t("Specify any associated check number or transaction number"));
    mmToolTip(w_notes_text, _t("Specify any text notes you want to add to this transaction."));
    mmToolTip(w_advanced_cb, _t("Allows the setting of different amounts in the FROM and TO accounts."));
}

void TrxDialog::setCategoryForPayee(const PayeeData* payee_n)
{
    // Only for new transactions.
    // If this is a Split Transaction, do not show category.
    if (!is_new() || !m_split_a.empty())
        return;

    // If user does not want to use categories.
    if (PrefModel::instance().getTransCategoryNone() == PrefModel::UNUSED) {
        const CategoryData* cat_n = CategoryModel::instance().get_key_data_n(
            _t("Unknown"), int64(-1)
        );
        if (!cat_n) {
            CategoryData new_cat_d = CategoryData();
            new_cat_d.m_name = _t("Unknown");
            CategoryModel::instance().add_data_n(new_cat_d);
            cat_n = CategoryModel::instance().get_idN_data_n(new_cat_d.m_id);
            w_cat_text->mmDoReInitialize();
        }

        m_journal_d.m_category_id_n = cat_n->m_id;
        w_cat_text->ChangeValue(cat_n->m_name);
        return;
    }

    if (!payee_n) {
        payee_n = PayeeModel::instance().get_name_data_n(w_payee_text->GetValue());
        if (!payee_n)
            return;
    }

    // Autofill last category used for payee.
    if ((PrefModel::instance().getTransCategoryNone() == PrefModel::LASTUSED ||
            PrefModel::instance().getTransCategoryNone() == PrefModel::DEFAULT
        ) &&
        CategoryModel::instance().get_id_active(payee_n->m_category_id_n)
    ) {
        // if payee has memory of last category used then display last category for payee
        const CategoryData* cat_n = CategoryModel::instance().get_idN_data_n(
            payee_n->m_category_id_n
        );
        if (cat_n) {
            m_journal_d.m_category_id_n = payee_n->m_category_id_n;
            w_cat_text->ChangeValue(CategoryModel::instance().get_id_fullname(
                payee_n->m_category_id_n
            ));
            wxLogDebug("Category: %s = %.2f", w_cat_text->GetLabel(), m_journal_d.m_amount);
        }
        else {
            m_journal_d.m_category_id_n = -1;
            w_cat_text->ChangeValue("");
        }
    }
}

// -- event handlers

// Workaround for bug http://trac.wxwidgets.org/ticket/11630
void TrxDialog::onDpcKillFocus(wxFocusEvent& event)
{
    if (wxGetKeyState(WXK_TAB) && wxGetKeyState(WXK_SHIFT))
        w_cancel_btn->SetFocus();
    else if (wxGetKeyState(WXK_TAB))
        w_status_choice->SetFocus();
    else if (wxGetKeyState(WXK_UP)) {
        wxCommandEvent evt(wxEVT_SPIN, wxID_ANY);
        evt.SetInt(1);
        this->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (wxGetKeyState(WXK_DOWN)) {
        wxCommandEvent evt(wxEVT_SPIN, wxID_ANY);
        evt.SetInt(-1);
        this->GetEventHandler()->AddPendingEvent(evt);
    }
    else
        event.Skip();
}

void TrxDialog::onFocusChange(wxChildFocusEvent& event)
{
    wxWindow* w = event.GetWindow();
    if (!w || w_focus == w->GetId()) {
        return;
    }

    switch (w_focus)
    {
    case mmID_ACCOUNTNAME:
        w_account_text->ChangeValue(w_account_text->GetValue());
        if (w_account_text->mmIsValid()) {
            m_journal_d.m_account_id = w_account_text->mmGetId();
            m_skip_account_init = false;
        }
        break;
    case mmID_TOACCOUNTNAME:
        w_to_account_text->ChangeValue(w_to_account_text->GetValue());
        if (w_to_account_text->mmIsValid()) {
            m_journal_d.m_to_account_id_n = w_to_account_text->mmGetId();
            m_skip_account_init = false;
        }
        break;
    case ID_DIALOG_PAYEE:
        w_payee_text->ChangeValue(w_payee_text->GetValue());
        m_journal_d.m_payee_id_n = w_payee_text->mmGetId();
        setCategoryForPayee();
        break;
    case mmID_CATEGORY:
        w_cat_text->ChangeValue(w_cat_text->GetValue());
        break;
    case mmID_TEXTAMOUNT:
        if (w_amount_text->Calculate()) {
            w_amount_text->GetDouble(m_journal_d.m_amount);
        }
        m_skip_amount_init = false;
        w_calc_text = w_amount_text;
        break;
    case mmID_TOTEXTAMOUNT:
        if (w_to_amount_text->Calculate()) {
            w_to_amount_text->GetDouble(m_journal_d.m_to_amount);
        }
        m_skip_amount_init = false;
        w_calc_text = w_to_amount_text;
        break;
    }

    w_focus = w->GetId();

    if (!m_journal_d.is_transfer()) {
        w_to_amount_text->ChangeValue("");
        m_journal_d.m_to_account_id_n = -1;
    }
    else {
        const AccountData* to_account_n = AccountModel::instance().get_idN_data_n(
            w_to_account_text->mmGetId()
        );
        if (to_account_n)
            m_journal_d.m_to_account_id_n = to_account_n->m_id;
    }

    dataToControls();
    event.Skip();
}

void TrxDialog::onPayeeChanged(wxCommandEvent& /*event*/)
{
    const PayeeData* payee_n = PayeeModel::instance().get_name_data_n(
        w_payee_text->GetValue()
    );
    if (payee_n) {
        setCategoryForPayee(payee_n);
    }
}

void TrxDialog::onTransTypeChanged(wxCommandEvent& event)
{
    const TrxType old_type = m_journal_d.m_type;
    wxStringClientData* client_obj = static_cast<wxStringClientData*>(
        event.GetClientObject()
    );
    if (client_obj)
        m_journal_d.m_type = TrxType(client_obj->GetData());
    if (old_type == m_journal_d.m_type)
        return;

    m_skip_payee_init = (!m_journal_d.is_transfer() && old_type.id() != TrxType::e_transfer);
    m_skip_account_init = true;
    m_skip_tooltips_init = false;

    if (m_journal_d.is_transfer()) {
        m_journal_d.m_payee_id_n = -1;
        m_skip_category_init = false;
    } else {
        m_journal_d.m_to_amount = m_journal_d.m_amount;
        m_journal_d.m_to_account_id_n = -1;
    }
    dataToControls();
}

void TrxDialog::onComboKey(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_RETURN) {
        auto id = event.GetId();
        switch (id)
        {
        case ID_DIALOG_PAYEE: {
            const auto payeeName = w_payee_text->GetValue();
            if (payeeName.empty()) {
                mmPayeeDialog dlg(this, true);
                dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    w_payee_text->mmDoReInitialize();
                int64 payee_id = dlg.getPayeeId();
                const PayeeData* payee_n = PayeeModel::instance().get_idN_data_n(payee_id);
                if (payee_n) {
                    w_payee_text->ChangeValue(payee_n->m_name);
                    w_payee_text->SetInsertionPointEnd();
                    setCategoryForPayee(payee_n);
                }
                return;
            }
            break;
        }
        case mmID_CATEGORY: {
            auto category = w_cat_text->GetValue();
            if (category.empty()) {
                CategoryManager dlg(this, true, -1);
                int rc = dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    w_cat_text->mmDoReInitialize();
                if (rc != wxID_CANCEL)
                    w_cat_text->ChangeValue(CategoryModel::instance().get_id_fullname(
                        dlg.getCategId()
                    ));
                return;
            }
            break;
        }
        default:
            break;
        }
    }

    // The first time the ALT key is pressed accelerator hints are drawn,
    // but custom painting on the tags button is not applied.
    // We need to refresh the tag ctrl to redraw the drop button with the correct image.
    if (event.AltDown() && !m_altRefreshDone) {
        w_tag_text->Refresh();
        m_altRefreshDone = true;
    }

    event.Skip();
}

void TrxDialog::onCalculator(wxCommandEvent& WXUNUSED(event))
{
    w_calc_popup->setTarget(w_calc_text);
    w_calc_popup->Popup();
}

void TrxDialog::onSwitch(wxCommandEvent& WXUNUSED(event))
{
    wxString temp = w_to_account_text->GetValue();
    w_to_account_text->ChangeValue(w_account_text->GetValue());
    w_account_text->ChangeValue(temp);
}

void TrxDialog::onToday(wxCommandEvent& WXUNUSED(event))
{
    w_date_picker->setValue(wxDateTime::Today());
}

void TrxDialog::onAutoTransNum(wxCommandEvent& WXUNUSED(event))
{
    mmDate date = m_journal_d.m_date();
    date.subDateSpan(wxDateSpan::Months(12));
    double next_number = 0, temp_num;
    for (const auto& trx_d : TrxModel::instance().find_data_a(
        TrxModel::WHERE_DATE(OP_GE, date),
        TrxCol::WHERE_ACCOUNTID(OP_EQ, m_journal_d.m_account_id),
        TrxCol::WHERE_TRANSACTIONNUMBER(OP_NEN, "")
    )) {
        if (!trx_d.m_number.IsNumber())
            continue;
        if (trx_d.m_number.ToDouble(&temp_num) && temp_num > next_number)
            next_number = temp_num;
    }

    next_number++;
    w_number_text->SetValue(wxString::FromDouble(next_number, 0));
}

void TrxDialog::onAdvanceChecked(wxCommandEvent& WXUNUSED(event))
{
    m_advanced = w_advanced_cb->IsChecked();
    m_skip_amount_init = false;
    dataToControls();
}

void TrxDialog::onCategs(wxCommandEvent& WXUNUSED(event))
{
    if (!w_amount_text->GetDouble(m_journal_d.m_amount)) {
        m_journal_d.m_amount = 0;
    }

    if (w_cat_text->IsEnabled() &&
        !w_cat_text->GetValue().IsEmpty() &&
        !w_cat_text->mmIsValid()
    ) {
        mmErrorDialogs::ToolTip4Object(w_cat_text,
            _t("Invalid value"),
            _t("Category"),
            wxICON_ERROR
        );
        return;
    }
    wxLogDebug("Cat Valid %d, Cat Is Empty %d, Cat value [%s]",
        w_cat_text->mmIsValid(),
        w_cat_text->GetValue().IsEmpty(),
        w_cat_text->GetValue()
    );
    if (m_split_a.empty()) {
        Split split_d;
        split_d.m_amount = m_journal_d.m_amount;
        if (w_cat_text->mmIsValid())
            split_d.m_category_id = w_cat_text->mmGetCategoryId();
        m_split_a.push_back(split_d);
    }

    SplitDialog dlg(this, m_split_a, m_journal_d.m_account_id);

    if (dlg.ShowModal() == wxID_OK) {
        m_split_a = dlg.mmGetResult();

        if (m_split_a.size() == 1) {
            m_journal_d.m_category_id_n = m_split_a[0].m_category_id;
            m_journal_d.m_amount = m_split_a[0].m_amount;
            w_amount_text->SetValue(m_journal_d.m_amount);
            m_split_a.clear();
        }

        if (!m_split_a.empty()) {
            w_amount_text->SetValue(m_journal_d.m_amount);
        }

        m_skip_category_init = false;
        m_skip_amount_init = false;
        m_skip_tooltips_init = false;
        dataToControls();
    }
    w_tag_text->Reinitialize();
}

void TrxDialog::onAttachments(wxCommandEvent& WXUNUSED(event))
{
    int64 ref_id = is_edit() ? m_journal_d.m_id : 0;
    AttachmentDialog dlg(this, TrxModel::s_ref_type, ref_id);
    dlg.ShowModal();
}

void TrxDialog::onTextEntered(wxCommandEvent& WXUNUSED(event))
{
    if (w_focus == w_amount_text->GetId()) {
        if (w_amount_text->Calculate()) {
            w_amount_text->GetDouble(m_journal_d.m_amount);
        }
    }
    else if (w_focus == w_to_amount_text->GetId()) {
        if (w_to_amount_text->Calculate()) {
            w_to_amount_text->GetDouble(m_journal_d.m_to_amount);
        }
    }
    m_skip_amount_init = false;
    dataToControls();
}

void TrxDialog::onFrequentUsedNotes(wxCommandEvent& WXUNUSED(event))
{
    if (m_freq_notes_a.empty())
        return;

    wxMenu menu;
    int id = wxID_HIGHEST;

    for (const auto& entry : m_freq_notes_a) {
        wxString label = entry.Mid(0, 36) + (entry.size() > 36 ? "..." : "");
        label.Replace("\n", " ");
        menu.Append(++id, label);
    }
    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &TrxDialog::onNoteSelected, this);
    PopupMenu(&menu);
}

void TrxDialog::onNoteSelected(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_HIGHEST;

    if (i > 0 && static_cast<size_t>(i) <= m_freq_notes_a.size()) {
        if (!w_notes_text->GetValue().EndsWith("\n") && !w_notes_text->GetValue().empty())
            w_notes_text->AppendText("\n");
        w_notes_text->AppendText(m_freq_notes_a[i - 1]);
    }
}

void TrxDialog::onOk(wxCommandEvent& event)
{
    m_journal_d.m_notes = w_notes_text->GetValue();
    m_journal_d.m_number = w_number_text->GetValue();
    m_journal_d.m_datetime = mmDateTime(w_date_picker->GetValue());
    wxStringClientData* status_obj = static_cast<wxStringClientData*>(
        w_status_choice->GetClientObject(w_status_choice->GetSelection())
    );
    if (status_obj) {
        m_journal_d.m_status = TrxStatus(status_obj->GetData());
    }

    if (!validateData())
        return;
    if (!w_fv_dlg->ValidateCustomValues())
        return;

    if (!m_advanced)
        m_journal_d.m_to_amount = m_journal_d.m_amount;

    if (m_journal_d.is_transfer() && !m_advanced && (
        AccountModel::instance().get_id_currency_p(m_journal_d.m_account_id) !=
        AccountModel::instance().get_id_currency_p(m_journal_d.m_to_account_id_n)
    )) {
        wxMessageDialog msgDlg(this,
            _t("The two accounts have different currencies, but no advanced transaction is defined. Is this correct?"),
            _t("Currencies are different"),
            wxYES_NO | wxNO_DEFAULT | wxICON_WARNING
        );
        if (msgDlg.ShowModal() == wxID_NO)
            return;
    }

    TrxData* trx_n;
    TrxData trx_d;
    if (is_edit()) {
        trx_n = TrxModel::instance().unsafe_get_idN_data_n(m_journal_d.m_id);
    }
    else {
        trx_d = TrxData();
        trx_n = &trx_d;
    }
    TrxModel::copy_from_trx(trx_n, m_journal_d);
    TrxModel::instance().unsafe_save_trx_n(trx_n);

    TrxSplitModel::DataA tp_a;
    for (const auto& split_d : m_split_a) {
        TrxSplitData tp_d = TrxSplitData();
        tp_d.m_category_id = split_d.m_category_id;
        tp_d.m_amount      = split_d.m_amount;
        tp_d.m_notes       = split_d.m_notes;
        tp_a.push_back(tp_d);
    }
    TrxSplitModel::instance().update_trx(trx_n->m_id, tp_a);

    // Save split tags
    for (unsigned int i = 0; i < m_split_a.size(); i++) {
        TagLinkModel::DataA new_tp_gl_a;
        for (const auto& tag_id : m_split_a.at(i).m_tag_id_a) {
            TagLinkData new_gl_d = TagLinkData();
            new_gl_d.m_tag_id   = tag_id;
            new_gl_d.m_ref_type = TrxSplitModel::s_ref_type;
            new_gl_d.m_ref_id   = tp_a.at(i).m_id;
            new_tp_gl_a.push_back(new_gl_d);
        }
        TagLinkModel::instance().update(
            TrxSplitModel::s_ref_type, tp_a.at(i).m_id,
            new_tp_gl_a
        );
    }
    if (!is_edit()) {
        mmAttachment::relocate_ref_all(
            TrxModel::s_ref_type, 0,
            TrxModel::s_ref_type, trx_n->m_id
        );
    }

    w_fv_dlg->SaveCustomValues(TrxModel::s_ref_type, trx_n->m_id);

    // Save base transaction tags
    TagLinkModel::DataA new_gl_a;
    for (const auto& tag_id : w_tag_text->GetTagIDs()) {
        TagLinkData new_gl_d = TagLinkData();
        new_gl_d.m_tag_id   = tag_id;
        new_gl_d.m_ref_type = TrxModel::s_ref_type;
        new_gl_d.m_ref_id   = trx_n->m_id;
        new_gl_a.push_back(new_gl_d);
    }
    TagLinkModel::instance().update(
        TrxModel::s_ref_type.key_n(), trx_n->m_id,
        new_gl_a
    );

    //TrxModel::DataExt trx(trx_d);
    //wxLogDebug("%s", trx.to_json());

    if (event.GetId() == ID_BTN_OK_NEW) {
        // store date for next invocation
        TrxDialog::s_previousDate = w_date_picker->GetValue();
    }
    else {
        TrxDialog::s_previousDate = wxDateTime();
    }

    m_journal_d.m_id        = trx_n->m_id;
    m_journal_d.m_sched_id  = -1;
    m_journal_d.m_repeat_id = -1;

    EndModal((
        (event.GetId() == ID_BTN_OK_NEW || wxGetKeyState(WXK_SHIFT))
    ) ? wxID_NEW : wxID_OK);
}

void TrxDialog::onMoreFields(wxCommandEvent& WXUNUSED(event))
{
    wxBitmapButton* button = static_cast<wxBitmapButton*>(
        FindWindow(ID_DIALOG_TRANS_CUSTOMFIELDS)
    );

    if (button)
        button->SetBitmap(mmImage::bitmapBundle(
            w_fv_dlg->IsCustomPanelShown()
                ? mmImage::png::RIGHTARROW
                : mmImage::png::LEFTARROW,
            mmImage::bitmapButtonSize
        ));

    w_fv_dlg->ShowHideCustomPanel();
    if (w_fv_dlg->IsCustomPanelShown()) {
        SetMinSize(wxSize(
            w_min_size.GetWidth() + w_fv_dlg->GetMinWidth(),
            w_min_size.GetHeight()
        ));
        SetSize(wxSize(
            GetSize().GetWidth() + w_fv_dlg->GetMinWidth(),
            GetSize().GetHeight()
        ));
    }
    else {
        SetMinSize(w_min_size);
        SetSize(wxSize(
            GetSize().GetWidth() - w_fv_dlg->GetMinWidth(),
            GetSize().GetHeight()
        ));
    }
}

void TrxDialog::onCancel(wxCommandEvent& WXUNUSED(event))
{
#ifdef __WXMSW__
    if (w_focus != wxID_CANCEL && wxGetKeyState(WXK_ESCAPE))
        return w_cancel_btn->SetFocus();

    if (w_focus != wxID_CANCEL) {
        return;
    }
#endif

    if (!is_edit()) {
        AttachmentModel::instance().purge_ref_all(TrxModel::s_ref_type, 0);
    }
    // invalidate!
    TrxDialog::s_previousDate = wxDateTime();
    EndModal(wxID_CANCEL);
}

void TrxDialog::onQuit(wxCloseEvent& WXUNUSED(event))
{
    if (!is_edit()) {
        AttachmentModel::instance().purge_ref_all(TrxModel::s_ref_type, 0);
    }
    EndModal(wxID_CANCEL);
}
