/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011-2022 Nikolay Akimov
 Copyright (C) 2011-2017 Stefano Giorgio [stef145g]
 Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "transdialog.h"
#include "mmcustomdata.h"
#include "attachmentdialog.h"
#include "categdialog.h"
#include "constants.h"
#include "images_list.h"
#include "mmSimpleDialogs.h"
#include "mmTextCtrl.h"
#include "paths.h"
#include "payeedialog.h"
#include "splittransactionsdialog.h"
#include "util.h"
#include "validators.h"
#include "webapp.h"

#include "option.h"
#include "model/Model_Account.h"
#include "model/Model_Attachment.h"
#include "model/Model_Category.h"
#include "model/Model_CurrencyHistory.h"
#include "model/Model_CustomFieldData.h"
#include "model/Model_Subcategory.h"
#include "model/Model_Setting.h"

#include <wx/numformatter.h>
#include <wx/timectrl.h>
#include <wx/collpane.h>


wxIMPLEMENT_DYNAMIC_CLASS(mmTransDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmTransDialog, wxDialog)
    EVT_CHAR_HOOK(mmTransDialog::OnComboKey)
    EVT_CHILD_FOCUS(mmTransDialog::OnFocusChange)
    EVT_COMBOBOX(mmID_PAYEE, mmTransDialog::OnPayeeChanged)
    EVT_BUTTON(mmID_CATEGORY_SPLIT, mmTransDialog::OnCategs)
    EVT_CHOICE(ID_DIALOG_TRANS_TYPE, mmTransDialog::OnTransTypeChanged)
    EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, mmTransDialog::OnAdvanceChecked)
    EVT_BUTTON(wxID_FILE, mmTransDialog::OnAttachments)
    EVT_BUTTON(ID_DIALOG_TRANS_CUSTOMFIELDS, mmTransDialog::OnMoreFields)
    EVT_MENU_RANGE(wxID_LOWEST, wxID_LOWEST + 20, mmTransDialog::OnNoteSelected)
    EVT_BUTTON(wxID_OK, mmTransDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmTransDialog::OnCancel)
    EVT_CLOSE(mmTransDialog::OnQuit)
wxEND_EVENT_TABLE()

mmTransDialog::~mmTransDialog()
{
}

void mmTransDialog::SetEventHandlers()
{
    m_textAmount->Connect(mmID_TEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmTransDialog::OnTextEntered), nullptr, this);
    toTextAmount_->Connect(mmID_TOTEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmTransDialog::OnTextEntered), nullptr, this);

#ifdef __WXGTK__ // Workaround for bug http://trac.wxwidgets.org/ticket/11630
    dpc_->Connect(ID_DIALOG_TRANS_BUTTONDATE, wxEVT_KILL_FOCUS
        , wxFocusEventHandler(mmTransDialog::OnDpcKillFocus), nullptr, this);
#endif
}

mmTransDialog::mmTransDialog(wxWindow* parent
    , int account_id
    , int transaction_id
    , double current_balance
    , bool duplicate
    , int type
    , const wxString& name
) : m_transfer(false)
    , m_duplicate(duplicate)
    , m_advanced(false)
    , m_current_balance(current_balance)
    , m_account_id(account_id)
    , skip_date_init_(false)
    , skip_account_init_(false)
    , skip_amount_init_(false)
    , skip_payee_init_(false)
    , skip_status_init_(false)
    , skip_notes_init_(false)
    , skip_category_init_(false)
    , skip_tooltips_init_(false)
{

    Model_Checking::Data *transaction = Model_Checking::instance().get(transaction_id);
    m_new_trx = (transaction || m_duplicate) ? false : true;
    m_transfer = m_new_trx ? type == Model_Checking::TRANSFER : Model_Checking::is_transfer(transaction);
    if (m_new_trx)
    {
        Model_Checking::getEmptyTransaction(m_trx_data, account_id);
        m_trx_data.TRANSCODE = Model_Checking::all_type()[type];
    }
    else
    {
        Model_Checking::getTransactionData(m_trx_data, transaction);
        const auto s = Model_Checking::splittransaction(transaction);
        for (const auto& item : s)
            m_local_splits.push_back({ item.CATEGID, item.SUBCATEGID, item.SPLITTRANSAMOUNT });

        if (m_duplicate && !Model_Setting::instance().GetBoolSetting(INIDB_USE_ORG_DATE_DUPLICATE, false))
        {
            // Use the empty transaction logic to generate the new date to be used
            Model_Checking::Data emptyTrx;
            Model_Checking::getEmptyTransaction(emptyTrx, account_id);
            m_trx_data.TRANSDATE = emptyTrx.TRANSDATE;
        }
    }

    m_advanced = m_transfer && !m_new_trx && (m_trx_data.TRANSAMOUNT != m_trx_data.TOTRANSAMOUNT);

    int ref_id = (m_new_trx) ? NULL : m_trx_data.TRANSID;
    m_custom_fields = new mmCustomDataTransaction(this, ref_id, ID_CUSTOMFIELD);

    // If duplicate then we may need to copy the attachments
    if (m_duplicate && Model_Infotable::instance().GetBoolInfo("ATTACHMENTSDUPLICATE", false))
    {
        const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
        mmAttachmentManage::CloneAllAttachments(RefType, transaction_id, -1);
    }

    this->SetFont(parent->GetFont());
    Create(parent);
    dataToControls();

    Fit();
    Centre();
}

bool mmTransDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    SetEvtHandlerEnabled(false);
    CreateControls();

    m_duplicate ? SetDialogTitle(_("Duplicate Transaction")) : SetDialogTitle(m_new_trx ? _("New Transaction") : _("Edit Transaction"));

    SetIcon(mmex::getProgramIcon());

    SetEventHandlers();
    SetEvtHandlerEnabled(true);

    return TRUE;
}

void mmTransDialog::dataToControls()
{
    Model_Checking::getFrequentUsedNotes(frequentNotes_, m_trx_data.ACCOUNTID);
    wxButton* bFrequentUsedNotes = static_cast<wxButton*>(FindWindow(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES));
    bFrequentUsedNotes->Enable(!frequentNotes_.empty());

    if (!skip_date_init_) //Date
    {
        wxDateTime trx_date;
        trx_date.ParseDate(m_trx_data.TRANSDATE);
        dpc_->SetValue(trx_date);
        dpc_->SetFocus();
        skip_date_init_ = true;
    }

    if (!skip_status_init_) //Status
    {
        m_status = m_trx_data.STATUS;
        choiceStatus_->SetSelection(Model_Checking::status(m_status));
        skip_status_init_ = true;
    }

    //Type
    transaction_type_->SetSelection(Model_Checking::type(m_trx_data.TRANSCODE));

    //Advanced
    cAdvanced_->Enable(m_transfer);
    cAdvanced_->SetValue(m_advanced && m_transfer);
    toTextAmount_->Enable(m_advanced && m_transfer);

    if (!skip_amount_init_) //Amounts
    {
        if (m_transfer & m_advanced)
            toTextAmount_->SetValue(m_trx_data.TOTRANSAMOUNT, Model_Currency::precision(m_trx_data.TOACCOUNTID));
        else
            toTextAmount_->ChangeValue("");

        if (!m_new_trx)
            m_textAmount->SetValue(m_trx_data.TRANSAMOUNT, Model_Currency::precision(m_trx_data.ACCOUNTID));
        skip_amount_init_ = true;
    }

    if (!skip_account_init_) //Account
    {
        Model_Account::Data* acc = Model_Account::instance().get(m_trx_data.ACCOUNTID);
        if (acc)
            cbAccount_->ChangeValue(acc->ACCOUNTNAME);

        Model_Account::Data* to_acc = Model_Account::instance().get(m_trx_data.TOACCOUNTID);
        if (to_acc) {
            cbToAccount_->ChangeValue(to_acc->ACCOUNTNAME);
        }

        skip_account_init_ = true;
    }

    if (m_transfer) {
        account_label_->SetLabelText(_("From"));
        payee_label_->SetLabelText(_("To"));
    }
    else if (!Model_Checking::is_deposit(m_trx_data.TRANSCODE)) {
        account_label_->SetLabelText(_("Account"));
        payee_label_->SetLabelText(_("Payee"));
    }
    else {
        account_label_->SetLabelText(_("Account"));
        payee_label_->SetLabelText(_("From"));
    }
    
    if (!skip_payee_init_) //Payee
    {
        cbPayee_->SetEvtHandlerEnabled(false);

        cbAccount_->UnsetToolTip();
        cbPayee_->UnsetToolTip();
        if (!m_transfer)
        {
            if (!Model_Checking::foreignTransaction(m_trx_data)) {
                m_trx_data.TOACCOUNTID = -1;
            }

            int accountID = cbAccount_->mmGetId();
            if (m_new_trx && !m_duplicate && Option::instance().TransPayeeSelection() == Option::LASTUSED
                && (-1 != accountID))
            {
                Model_Checking::Data_Set transactions = Model_Checking::instance().find(
                    Model_Checking::TRANSCODE(Model_Checking::TRANSFER, NOT_EQUAL)
                    , Model_Checking::ACCOUNTID(accountID, EQUAL)
                    , Model_Checking::TRANSDATE(wxDateTime::Today(), LESS_OR_EQUAL));

                if (!transactions.empty()) {
                    Model_Payee::Data* payee = Model_Payee::instance().get(transactions.back().PAYEEID);
                    cbPayee_->ChangeValue(payee->PAYEENAME);
                }
            }
            else if (m_new_trx && !m_duplicate && Option::instance().TransPayeeSelection() == Option::UNUSED)
            {
                Model_Payee::Data *payee = Model_Payee::instance().get(_("Unknown"));
                if (!payee)
                {
                    payee = Model_Payee::instance().create();
                    payee->PAYEENAME = _("Unknown");
                    Model_Payee::instance().save(payee);
                    cbPayee_->mmDoReInitialize();
                }

                cbPayee_->ChangeValue(_("Unknown"));
            }
            else
            {
                Model_Payee::Data* payee = Model_Payee::instance().get(m_trx_data.PAYEEID);
                if (payee)
                    cbPayee_->ChangeValue(payee->PAYEENAME);
            }

            SetCategoryForPayee();
        }
        skip_payee_init_ = true;
        cbPayee_->SetEvtHandlerEnabled(true);
    }

    cbPayee_->Show(!m_transfer);
    payee_label_->Show(!m_transfer);
    to_acc_label_->Show(m_transfer);
    cbToAccount_->Show(m_transfer);
    Layout();

    bool has_split = !m_local_splits.empty();
    if (!skip_category_init_)
    {
        bSplit_->UnsetToolTip();
        if (has_split)
        {
            cbCategory_->SetLabelText(_("Split Transaction"));
            cbCategory_->Disable();
            m_textAmount->SetValue(Model_Splittransaction::get_total(m_local_splits));
            m_trx_data.CATEGID = -1;
            m_trx_data.SUBCATEGID = -1;
        }
        else if (m_transfer && m_new_trx && !m_duplicate 
                    && Option::instance().TransCategorySelectionTransfer() == Option::LASTUSED)
        {
            Model_Checking::Data_Set transactions = Model_Checking::instance().find(
                Model_Checking::TRANSCODE(Model_Checking::TRANSFER, EQUAL)
                , Model_Checking::TRANSDATE(wxDateTime::Today(), LESS_OR_EQUAL));

            if (!transactions.empty()) 
            {
                const int cat = transactions.back().CATEGID;
                const int subcat = transactions.back().SUBCATEGID;
                cbCategory_->ChangeValue(Model_Category::full_name(cat, subcat));
            }
        } else
        {
            auto fullCategoryName = Model_Category::full_name(m_trx_data.CATEGID, m_trx_data.SUBCATEGID);
            cbCategory_->ChangeValue(fullCategoryName);
        }
        skip_category_init_ = true;
    }

    m_textAmount->Enable(m_local_splits.empty());
    cbCategory_->Enable(!has_split);
    bSplit_->Enable(!m_transfer);
    Fit();

    if (!skip_notes_init_) //Notes & Transaction Number
    {
        textNumber_->SetValue(m_trx_data.TRANSACTIONNUMBER);
        textNotes_->SetValue(m_trx_data.NOTES);
        skip_notes_init_ = true;
    }

    if (!skip_tooltips_init_)
        SetTooltips();
}

void mmTransDialog::CreateControls()
{
    wxBoxSizer* box_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* box_sizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* box_sizer2 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* box_sizer3 = new wxBoxSizer(wxVERTICAL);
    box_sizer->Add(box_sizer1, g_flagsExpand);
    box_sizer1->Add(box_sizer2, g_flagsExpand);
    box_sizer1->Add(box_sizer3, g_flagsExpand);

    wxStaticBox* static_box = new wxStaticBox(this, wxID_ANY, _("Transaction Details"));
    wxStaticBoxSizer* box_sizer_left = new wxStaticBoxSizer(static_box, wxVERTICAL);
    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 3, 0, 0);
    flex_sizer->AddGrowableCol(1, 0);
    box_sizer_left->Add(flex_sizer, g_flagsV);
    box_sizer2->Add(box_sizer_left, g_flagsExpand);

    // Date -------------------------------------------
    wxStaticText* name_label = new wxStaticText(this, wxID_STATIC, _("Date"));
    flex_sizer->Add(name_label, g_flagsH);
    name_label->SetFont(this->GetFont().Bold());
    
    dpc_ = new mmDatePickerCtrl(this, ID_DIALOG_TRANS_BUTTONDATE);
    flex_sizer->Add(dpc_->mmGetLayout());

    flex_sizer->AddSpacer(1);

    // Status --------------------------------------------
    choiceStatus_ = new wxChoice(this, ID_DIALOG_TRANS_STATUS);

    for (const auto& i : Model_Checking::all_status()) {
        choiceStatus_->Append(wxGetTranslation(i), new wxStringClientData(i));
    }

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Status")), g_flagsH);
    flex_sizer->Add(choiceStatus_, g_flagsH);
    flex_sizer->AddSpacer(1);

    // Type --------------------------------------------
    transaction_type_ = new wxChoice(this, ID_DIALOG_TRANS_TYPE);

    for (const auto& i : Model_Checking::all_type())
    {
        if (i != Model_Checking::all_type()[Model_Checking::TRANSFER] || Model_Account::instance().all().size() > 1)
        {
            transaction_type_->Append(wxGetTranslation(i), new wxStringClientData(i));
        }
    }

    cAdvanced_ = new wxCheckBox(this
        , ID_DIALOG_TRANS_ADVANCED_CHECKBOX, _("&Advanced")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );

    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Type")), g_flagsH);
    flex_sizer->Add(typeSizer);
    typeSizer->Add(transaction_type_, g_flagsH);
    typeSizer->Add(cAdvanced_, g_flagsH);
    flex_sizer->AddSpacer(1);

    // Amount Fields --------------------------------------------
    m_textAmount = new mmTextCtrl(this, mmID_TEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());

    toTextAmount_ = new mmTextCtrl( this, mmID_TOTEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(m_textAmount, g_flagsExpand);
    amountSizer->Add(toTextAmount_, g_flagsExpand);

    wxStaticText* amount_label = new wxStaticText(this, wxID_STATIC, _("Amount"));
    amount_label->SetFont(this->GetFont().Bold());
    flex_sizer->Add(amount_label, g_flagsH);
    flex_sizer->Add(amountSizer, wxSizerFlags(g_flagsExpand).Border(0));
    flex_sizer->AddSpacer(1);

    // Account ---------------------------------------------
    account_label_ = new wxStaticText(this, wxID_STATIC, _("Account"));
    account_label_->SetFont(this->GetFont().Bold());

    cbAccount_ = new mmComboBoxAccount(this, mmID_ACCOUNTNAME, wxDefaultSize, m_trx_data.ACCOUNTID);
    cbAccount_->SetMinSize(cbAccount_->GetSize());

    flex_sizer->Add(account_label_, g_flagsH);
    flex_sizer->Add(cbAccount_, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // To Account ------------------------------------------------
    to_acc_label_ = new wxStaticText(this, mmID_TOACCOUNT_LABEL, _("To"));
    to_acc_label_->SetFont(this->GetFont().Bold());
    flex_sizer->Add(to_acc_label_, g_flagsH);
    cbToAccount_ = new mmComboBoxAccount(this, mmID_TOACCOUNTNAME, wxDefaultSize, m_trx_data.TOACCOUNTID);
    cbToAccount_->SetMinSize(cbToAccount_->GetSize());
    flex_sizer->Add(cbToAccount_, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // Payee ---------------------------------------------
    payee_label_ = new wxStaticText(this, mmID_PAYEE_LABEL, _("Payee"));
    payee_label_->SetFont(this->GetFont().Bold());

    cbPayee_ = new mmComboBoxPayee(this, mmID_PAYEE);
    cbPayee_->SetMinSize(cbPayee_->GetSize());

    flex_sizer->Add(payee_label_, g_flagsH);
    flex_sizer->Add(cbPayee_, g_flagsExpand);
    flex_sizer->AddSpacer(1);


    // Category -------------------------------------------------

    categ_label_ = new wxStaticText(this, ID_DIALOG_TRANS_CATEGLABEL2, _("Category"));
    categ_label_->SetFont(this->GetFont().Bold());
    cbCategory_ = new mmComboBoxCategory(this, mmID_CATEGORY);
    cbCategory_->SetMinSize(cbCategory_->GetSize());

    bSplit_ = new wxBitmapButton(this, mmID_CATEGORY_SPLIT, mmBitmap(png::NEW_TRX, mmBitmapButtonSize));
    mmToolTip(bSplit_, _("Use split Categories"));

    flex_sizer->Add(categ_label_, g_flagsH);
    flex_sizer->Add(cbCategory_, g_flagsExpand);
    flex_sizer->Add(bSplit_, g_flagsH);

    // Number  ---------------------------------------------

    textNumber_ = new mmTextCtrl(this, ID_DIALOG_TRANS_TEXTNUMBER, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

    wxBitmapButton* bAuto = new wxBitmapButton(this, ID_DIALOG_TRANS_BUTTONTRANSNUM, mmBitmap(png::TRXNUM, mmBitmapButtonSize));
    bAuto->Connect(ID_DIALOG_TRANS_BUTTONTRANSNUM, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmTransDialog::OnAutoTransNum), nullptr, this);
    mmToolTip(bAuto, _("Populate Transaction #"));

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Number")), g_flagsH);
    flex_sizer->Add(textNumber_, g_flagsExpand);
    flex_sizer->Add(bAuto, g_flagsH);

    // Frequently Used Notes
    wxButton* bFrequentUsedNotes = new wxButton(this, ID_DIALOG_TRANS_BUTTON_FREQENTNOTES
        , "...", wxDefaultPosition, bAuto->GetSize(), 0);
    mmToolTip(bFrequentUsedNotes, _("Select one of the frequently used notes"));
    bFrequentUsedNotes->Connect(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES
        , wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmTransDialog::OnFrequentUsedNotes), nullptr, this);

    // Colours
    bColours_ = new mmColorButton(this, wxID_LOWEST, bAuto->GetSize());
    mmToolTip(bColours_, _("User Colors"));
    bColours_->SetBackgroundColor(m_trx_data.FOLLOWUPID);

    // Attachments
    bAttachments_ = new wxBitmapButton(this, wxID_FILE, mmBitmap(png::CLIP, mmBitmapButtonSize));
    mmToolTip(bAttachments_, _("Organize attachments of this transaction"));

    // Now display the Frequently Used Notes, Colour, Attachment buttons
    wxBoxSizer* notes_sizer = new wxBoxSizer(wxHORIZONTAL); 
    flex_sizer->Add(notes_sizer);
    notes_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Notes")), g_flagsH);
    notes_sizer->Add(bFrequentUsedNotes, g_flagsH);

    wxBoxSizer* RightAlign_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(RightAlign_sizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));
    RightAlign_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Color")), g_flagsH);
    RightAlign_sizer->Add(bColours_, wxSizerFlags());
    flex_sizer->Add(bAttachments_, g_flagsH);

    // Notes
    textNotes_ = new wxTextCtrl(this, ID_DIALOG_TRANS_TEXTNOTES, ""
        , wxDefaultPosition, wxSize(-1, dpc_->GetSize().GetHeight() * 5), wxTE_MULTILINE);
    mmToolTip(textNotes_, _("Specify any text notes you want to add to this transaction."));
    box_sizer_left->Add(textNotes_, wxSizerFlags(g_flagsExpand).Border(wxLEFT | wxRIGHT | wxBOTTOM, 10));

    /**********************************************************************************************
     Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    box_sizer_left->Add(buttons_panel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 0));

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxButton* button_ok = new wxButton(buttons_panel, wxID_OK, _("&OK "));
    m_button_cancel = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));

    wxBitmapButton* button_hide = new wxBitmapButton(buttons_panel, ID_DIALOG_TRANS_CUSTOMFIELDS, mmBitmap(png::RIGHTARROW, mmBitmapButtonSize));
    mmToolTip(button_hide, _("Show/Hide custom fields window"));
    if (m_custom_fields->GetCustomFieldsCount() == 0) {
        button_hide->Hide();
    }

    buttons_sizer->Add(button_ok, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    buttons_sizer->Add(m_button_cancel, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    buttons_sizer->Add(button_hide, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));

    if (!m_new_trx && !m_duplicate) m_button_cancel->SetFocus();

    buttons_sizer->Realize();

    // Custom fields -----------------------------------

    m_custom_fields->FillCustomFields(box_sizer3);
    if (m_custom_fields->GetActiveCustomFieldsCount() > 0) {
        wxCommandEvent evt(wxEVT_BUTTON, ID_DIALOG_TRANS_CUSTOMFIELDS);
        OnMoreFields(evt);
    }

    this->SetSizer(box_sizer);
}

bool mmTransDialog::ValidateData()
{
    if (!m_textAmount->checkValue(m_trx_data.TRANSAMOUNT))
        return false;

    if (!cbAccount_->mmIsValid()) {
        mmErrorDialogs::ToolTip4Object(cbAccount_, _("Invalid value"), _("Account"), wxICON_ERROR);
        return false;
    }
    m_trx_data.ACCOUNTID = cbAccount_->mmGetId();

    if (m_local_splits.empty())
    {
        if (!cbCategory_->mmIsValid()) {
            mmErrorDialogs::ToolTip4Object(cbCategory_, _("Invalid value"), _("Category"), wxICON_ERROR);
            return false;
        }
        m_trx_data.CATEGID = cbCategory_->mmGetCategoryId();
        m_trx_data.SUBCATEGID = cbCategory_->mmGetSubcategoryId();
    }

    if (!m_transfer)
    {
        wxString payee_name = cbPayee_->GetValue();
        if (payee_name.IsEmpty())
        {
            mmErrorDialogs::InvalidPayee(cbPayee_);
            return false;
        }

        // Get payee string from populated list to address issues with case compare differences between autocomplete and payee list
        int payee_loc = cbPayee_->FindString(payee_name);
        if (payee_loc != wxNOT_FOUND)
            payee_name = cbPayee_->GetString(payee_loc);

        Model_Payee::Data* payee = Model_Payee::instance().get(payee_name);
        if (!payee)
        {
            wxMessageDialog msgDlg( this
                , wxString::Format(_("You have not used this payee name before. Is the name correct?\n%s"), payee_name)
                , _("Confirm payee name")
                , wxYES_NO | wxYES_DEFAULT | wxICON_WARNING);
            if (msgDlg.ShowModal() == wxID_YES)
            {
                payee = Model_Payee::instance().create();
                payee->PAYEENAME = payee_name;
                Model_Payee::instance().save(payee);
                mmWebApp::MMEX_WebApp_UpdatePayee();
            }
            else
                return false;
        }
        m_trx_data.TOTRANSAMOUNT = m_trx_data.TRANSAMOUNT;
        m_trx_data.PAYEEID = payee->PAYEEID;
        if (!Model_Checking::foreignTransaction(m_trx_data))
        {
            m_trx_data.TOACCOUNTID = -1;
        }

        if (Option::instance().TransCategorySelectionNonTransfer() == Option::LASTUSED)
        {
            payee->CATEGID = m_trx_data.CATEGID;
            payee->SUBCATEGID = m_trx_data.SUBCATEGID;
            Model_Payee::instance().save(payee);
            mmWebApp::MMEX_WebApp_UpdatePayee();
        }
    }
    else //transfer
    {
        Model_Account::Data *to_account = Model_Account::instance().get(cbToAccount_->GetValue());
        if (!to_account || to_account->ACCOUNTID == m_trx_data.ACCOUNTID
            || Model_Account::type(to_account) == Model_Account::INVESTMENT)
        {
            mmErrorDialogs::InvalidAccount(cbToAccount_, true);
            return false;
        }
        m_trx_data.TOACCOUNTID = to_account->ACCOUNTID;

        if (m_advanced)
        {
            if (!toTextAmount_->checkValue(m_trx_data.TOTRANSAMOUNT))
                return false;
        }
        m_trx_data.PAYEEID = -1;
    }

    /* Check if transaction is to proceed.*/
    Model_Account::Data* account = Model_Account::instance().get(m_trx_data.ACCOUNTID);
    if (Model_Account::BoolOf(account->STATEMENTLOCKED))
    {
        if (dpc_->GetValue() <= Model_Account::DateOf(account->STATEMENTDATE))
        {
            if (wxMessageBox(_(wxString::Format(
                "Locked transaction to date: %s\n\n"
                "Do you wish to continue ? "
                , mmGetDateForDisplay(account->STATEMENTDATE)))
                , _("MMEX Transaction Check"), wxYES_NO | wxICON_WARNING) == wxNO)
            {
                return false;
            }
        }
    }

    //Checking account does not exceed limits
    if (m_new_trx || m_duplicate)
    {
        bool abort_transaction = false;
        double new_value = m_trx_data.TRANSAMOUNT;

        if (m_trx_data.TRANSCODE == Model_Checking::all_type()[Model_Checking::WITHDRAWAL])
        {
            new_value *= -1;
        }

        new_value += m_current_balance;

        if ((account->MINIMUMBALANCE != 0) && (new_value < account->MINIMUMBALANCE))
        {
            abort_transaction = true;
        }

        if ((account->CREDITLIMIT != 0) && (new_value < (account->CREDITLIMIT * -1)))
        {
            abort_transaction = true;
        }

        if (abort_transaction && wxMessageBox(_(
            "This transaction will exceed your account limit.\n\n"
            "Do you wish to continue?")
            , _("MMEX Transaction Check"), wxYES_NO | wxICON_WARNING) == wxNO)
        {
            return false;
        }
    }

    int color_id = bColours_->GetColorId();
    if (color_id > 0 && color_id < 8)
        m_trx_data.FOLLOWUPID = color_id;
    else
        m_trx_data.FOLLOWUPID = -1;

    return true;
}

//----------------------------------------------------------------------------
// Workaround for bug http://trac.wxwidgets.org/ticket/11630
void mmTransDialog::OnDpcKillFocus(wxFocusEvent& event)
{
    if (wxGetKeyState(WXK_TAB) && wxGetKeyState(WXK_SHIFT))
        m_button_cancel->SetFocus();
    else if (wxGetKeyState(WXK_TAB))
        choiceStatus_->SetFocus();
    else if (wxGetKeyState(WXK_UP))
    {
        wxCommandEvent evt(wxEVT_SPIN, wxID_ANY);
        evt.SetInt(1);
        this->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (wxGetKeyState(WXK_DOWN))
    {
        wxCommandEvent evt(wxEVT_SPIN, wxID_ANY);
        evt.SetInt(-1);
        this->GetEventHandler()->AddPendingEvent(evt);
    }
    else
        event.Skip();
}

void mmTransDialog::OnFocusChange(wxChildFocusEvent& event)
{
    wxWindow* w = event.GetWindow();
    if (!w || object_in_focus_ == w->GetId()) {
        return;
    }

    switch (object_in_focus_)
    {
    case mmID_ACCOUNTNAME:
        cbAccount_->ChangeValue(cbAccount_->GetValue());
        if (cbAccount_->mmIsValid())
            m_trx_data.ACCOUNTID = cbAccount_->mmGetId();
        break;
    case mmID_TOACCOUNTNAME:
        cbToAccount_->ChangeValue(cbToAccount_->GetValue());
        if (cbToAccount_->mmIsValid())
            m_trx_data.TOACCOUNTID = cbToAccount_->mmGetId();
        break;
    case mmID_PAYEE:
        cbPayee_->ChangeValue(cbPayee_->GetValue());
        m_trx_data.PAYEEID = cbPayee_->mmGetId();
        SetCategoryForPayee();
        break;
    case mmID_CATEGORY:
        cbCategory_->ChangeValue(cbCategory_->GetValue());
        break;
    case mmID_TEXTAMOUNT:
        if (m_textAmount->Calculate(Model_Currency::precision(m_trx_data.ACCOUNTID))) {
            m_textAmount->GetDouble(m_trx_data.TRANSAMOUNT);
        }
        skip_amount_init_ = false;
        break;
    case mmID_TOTEXTAMOUNT:
        if (toTextAmount_->Calculate(Model_Currency::precision(m_trx_data.TOACCOUNTID))) {
            toTextAmount_->GetDouble(m_trx_data.TOTRANSAMOUNT);
        }
        skip_amount_init_ = false;
        break;
    }

    object_in_focus_ = w->GetId();
    
    if (!m_transfer)
    {
        toTextAmount_->ChangeValue("");
        m_trx_data.TOACCOUNTID = -1;
    }
    else
    {
        const Model_Account::Data* to_account = Model_Account::instance().get(cbToAccount_->mmGetId());
        if (to_account)
            m_trx_data.TOACCOUNTID = to_account->ACCOUNTID;
    }

    dataToControls();
    event.Skip();
}

void mmTransDialog::SetDialogTitle(const wxString& title)
{
    this->SetTitle(title);
}

void mmTransDialog::OnPayeeChanged(wxCommandEvent& /*event*/)
{
    Model_Payee::Data * payee = Model_Payee::instance().get(cbPayee_->GetValue());
    if (payee)
    {
        SetCategoryForPayee(payee);
    }
}
void mmTransDialog::OnTransTypeChanged(wxCommandEvent& event)
{
    const wxString old_type = m_trx_data.TRANSCODE;
    wxStringClientData *client_obj = static_cast<wxStringClientData*>(event.GetClientObject());
    if (client_obj) m_trx_data.TRANSCODE = client_obj->GetData();
    if (old_type != m_trx_data.TRANSCODE)
    {
        m_transfer = Model_Checking::is_transfer(m_trx_data.TRANSCODE);
        if (m_transfer || Model_Checking::is_transfer(old_type))
            skip_payee_init_ = false;
        else
            skip_payee_init_ = true;      
        skip_account_init_ = true;
        skip_tooltips_init_ = false;

        if (m_transfer) {
            m_trx_data.PAYEEID = -1;
            skip_category_init_ = false;
        } else {
            m_trx_data.TOTRANSAMOUNT = m_trx_data.TRANSAMOUNT;
            m_trx_data.TOACCOUNTID = -1;
        }
        dataToControls();
    }
}

void mmTransDialog::OnComboKey(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_RETURN)
    {
        auto id = event.GetId();
        switch (id)
        {
        case mmID_PAYEE:
        {
            const auto payeeName = cbPayee_->GetValue();
            if (payeeName.empty())
            {
                mmPayeeDialog dlg(this, true);
                dlg.ShowModal();
                cbPayee_->mmDoReInitialize();
                int payee_id = dlg.getPayeeId();
                Model_Payee::Data* payee = Model_Payee::instance().get(payee_id);
                if (payee) {
                    cbPayee_->ChangeValue(payee->PAYEENAME);
                    cbPayee_->SetInsertionPointEnd();
                    SetCategoryForPayee(payee);
                }
                return;
            }
        }
        break;
        case mmID_CATEGORY:
        {
            auto category = cbCategory_->GetValue();
            if (category.empty())
            {
                mmCategDialog dlg(this, true, -1, -1);
                dlg.ShowModal();
                cbCategory_->mmDoReInitialize();
                category = Model_Category::full_name(dlg.getCategId(), dlg.getSubCategId());
                cbCategory_->ChangeValue(category);
                return;
            }
        }
        break;
        default:
            break;
        }
    }

    event.Skip();
}

void mmTransDialog::SetCategoryForPayee(const Model_Payee::Data *payee)
{
    // Only for new transactions: if user does not want to use categories.
    // If this is a Split Transaction, ignore displaying last category for payee
    if (Option::instance().TransCategorySelectionNonTransfer() == Option::UNUSED
        && m_local_splits.empty() && m_new_trx && !m_duplicate)
    {
        Model_Category::Data *category = Model_Category::instance().get(_("Unknown"));
        if (!category)
        {
            category = Model_Category::instance().create();
            category->CATEGNAME = _("Unknown");
            Model_Category::instance().save(category);
            cbCategory_->mmDoReInitialize();
        }

        m_trx_data.CATEGID = category->CATEGID;
        cbCategory_->ChangeValue(_("Unknown"));
        return;
    }

    if (!payee)
    {
        payee = Model_Payee::instance().get(cbPayee_->GetValue());
        if (!payee)
            return;
    }

    // Only for new transactions: if user want to autofill last category used for payee.
    // If this is a Split Transaction, ignore displaying last category for payee
    if ((Option::instance().TransCategorySelectionNonTransfer() == Option::LASTUSED ||
         Option::instance().TransCategorySelectionNonTransfer() == Option::DEFAULT)
        && m_local_splits.empty() && m_new_trx && !m_duplicate)
    {
        // if payee has memory of last category used then display last category for payee
        Model_Category::Data *category = Model_Category::instance().get(payee->CATEGID);
        if (category)
        {
            m_trx_data.CATEGID = payee->CATEGID;
            m_trx_data.SUBCATEGID = payee->SUBCATEGID;
            cbCategory_->ChangeValue(Model_Category::full_name(payee->CATEGID, payee->SUBCATEGID));
            wxLogDebug("Category: %s = %.2f", cbCategory_->GetLabel(), m_trx_data.TRANSAMOUNT);
        }
        else
        {
            m_trx_data.CATEGID = -1;
            m_trx_data.SUBCATEGID = -1;
            cbCategory_->ChangeValue("");
        }
    }
}

void mmTransDialog::OnAutoTransNum(wxCommandEvent& WXUNUSED(event))
{
    auto d = Model_Checking::TRANSDATE(m_trx_data).Subtract(wxDateSpan::Months(12));
    double next_number = 0, temp_num;
    const auto numbers = Model_Checking::instance().find(
        Model_Checking::ACCOUNTID(m_trx_data.ACCOUNTID, EQUAL)
        , Model_Checking::TRANSDATE(d, GREATER_OR_EQUAL)
        , Model_Checking::TRANSACTIONNUMBER("", NOT_EQUAL));
    for (const auto &num : numbers)
    {
        if (!num.TRANSACTIONNUMBER.IsNumber()) continue;
        if (num.TRANSACTIONNUMBER.ToDouble(&temp_num) && temp_num > next_number)
            next_number = temp_num;
    }

    next_number++;
    textNumber_->SetValue(wxString::FromDouble(next_number, 0));
}

void mmTransDialog::OnAdvanceChecked(wxCommandEvent& WXUNUSED(event))
{
    m_advanced = cAdvanced_->IsChecked();
    skip_amount_init_ = false;
    dataToControls();
}

void mmTransDialog::OnCategs(wxCommandEvent& WXUNUSED(event))
{
    if (!m_textAmount->GetDouble(m_trx_data.TRANSAMOUNT)) {
        m_trx_data.TRANSAMOUNT = 0;
    }

    if (m_local_splits.empty() && cbCategory_->mmIsValid())
    {
        Split s;
        s.SPLITTRANSAMOUNT = m_trx_data.TRANSAMOUNT;
        s.CATEGID = cbCategory_->mmGetCategoryId();
        s.SUBCATEGID = cbCategory_->mmGetSubcategoryId();
        m_local_splits.push_back(s);
    }

    bool isDeposit = Model_Checking::is_deposit(m_trx_data.TRANSCODE);
    mmSplitTransactionDialog dlg(this, m_local_splits
        , m_trx_data.ACCOUNTID
        , isDeposit ? Model_Checking::DEPOSIT : Model_Checking::WITHDRAWAL
        , m_trx_data.TRANSAMOUNT);

    dlg.ShowModal();

    m_local_splits = dlg.mmGetResult();

    if (m_local_splits.size() == 1) {
        m_trx_data.CATEGID = m_local_splits[0].CATEGID;
        m_trx_data.SUBCATEGID = m_local_splits[0].SUBCATEGID;
        m_trx_data.TRANSAMOUNT = m_local_splits[0].SPLITTRANSAMOUNT;
        m_textAmount->SetValue(m_trx_data.TRANSAMOUNT);
        m_local_splits.clear();
    }

    if (!m_local_splits.empty()) {
        m_textAmount->SetValue(m_trx_data.TRANSAMOUNT);
    }

    skip_category_init_ = false;
    skip_amount_init_ = false;
    skip_tooltips_init_ = false;
    dataToControls();
}

void mmTransDialog::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    int TransID = m_trx_data.TRANSID;
    if (m_duplicate) TransID = -1;
    mmAttachmentDialog dlg(this, RefType, TransID);
    dlg.ShowModal();
}

void mmTransDialog::OnTextEntered(wxCommandEvent& WXUNUSED(event))
{
    if (object_in_focus_ == m_textAmount->GetId())
    {
        if (m_textAmount->Calculate(Model_Currency::precision(m_trx_data.ACCOUNTID)))
        {
            m_textAmount->GetDouble(m_trx_data.TRANSAMOUNT);
        }
    }
    else if (object_in_focus_ == toTextAmount_->GetId())
    {
        if (toTextAmount_->Calculate(Model_Currency::precision(m_trx_data.TOACCOUNTID)))
        {
            toTextAmount_->GetDouble(m_trx_data.TOTRANSAMOUNT);
        }
    }
    skip_amount_init_ = false;
    dataToControls();
}

void mmTransDialog::OnFrequentUsedNotes(wxCommandEvent& WXUNUSED(event))
{
    if (!frequentNotes_.empty())
    {
        wxMenu menu;
        int id = wxID_LOWEST;
        for (const auto& entry : frequentNotes_) {
            wxString label = entry.Mid(0, 36) + (entry.size() > 36 ? "..." : "");
            label.Replace("\n", " ");
            menu.Append(++id, label);
        }
        PopupMenu(&menu);
    }
}

void mmTransDialog::OnNoteSelected(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_LOWEST;
    if (i > 0 && static_cast<size_t>(i) <= frequentNotes_.size()) {
        if (!textNotes_->GetValue().EndsWith("\n") && !textNotes_->GetValue().empty())
            textNotes_->AppendText("\n");
        textNotes_->AppendText(frequentNotes_[i - 1]);
    }
}

void mmTransDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    m_trx_data.STATUS = "";
    m_trx_data.NOTES = textNotes_->GetValue();
    m_trx_data.TRANSACTIONNUMBER = textNumber_->GetValue();
    m_trx_data.TRANSDATE = dpc_->GetValue().FormatISODate();
    wxStringClientData* status_obj = static_cast<wxStringClientData*>(choiceStatus_->GetClientObject(choiceStatus_->GetSelection()));
    if (status_obj)
    {
        m_status = Model_Checking::toShortStatus(status_obj->GetData());
        m_trx_data.STATUS = m_status;
    }

    if (!ValidateData()) return;
    if (!m_custom_fields->ValidateCustomValues(m_trx_data.TRANSID)) return;

    if (!m_advanced)
        m_trx_data.TOTRANSAMOUNT = m_trx_data.TRANSAMOUNT;

    if (m_transfer && !m_advanced && (Model_Account::currency(Model_Account::instance().get(m_trx_data.ACCOUNTID))
            != Model_Account::currency(Model_Account::instance().get(m_trx_data.TOACCOUNTID))))
    {
        wxMessageDialog msgDlg( this
            , _("The two accounts have different currencies but you have not defined an advanced transaction. Is this correct?")
            , _("Currencies are different")
            , wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
        if (msgDlg.ShowModal() == wxID_NO)
            return;
    }

    Model_Checking::Data *r = Model_Checking::instance().get(m_trx_data.TRANSID);
    if (m_new_trx || m_duplicate)
        r = Model_Checking::instance().create();

    Model_Checking::putDataToTransaction(r, m_trx_data);
    m_trx_data.TRANSID = Model_Checking::instance().save(r);

    Model_Splittransaction::Data_Set splt;
    for (const auto& entry : m_local_splits)
    {
        Model_Splittransaction::Data *s = Model_Splittransaction::instance().create();
        s->CATEGID = entry.CATEGID;
        s->SUBCATEGID = entry.SUBCATEGID;
        s->SPLITTRANSAMOUNT = entry.SPLITTRANSAMOUNT;
        splt.push_back(*s);
    }
    Model_Splittransaction::instance().update(splt, m_trx_data.TRANSID);

    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    if (m_new_trx || m_duplicate)
    {
        mmAttachmentManage::RelocateAllAttachments(RefType, -1, m_trx_data.TRANSID);
    }

    m_custom_fields->SaveCustomValues(m_trx_data.TRANSID);

    const Model_Checking::Data& tran(*r);
    Model_Checking::Full_Data trx(tran);
    wxLogDebug("%s", trx.to_json());

    bool loop = Option::instance().get_bulk_transactions();
    bool s = (wxGetKeyState(WXK_SHIFT) && !loop) || (!wxGetKeyState(WXK_SHIFT) && loop);
    if (m_new_trx && s)
        return EndModal(wxID_NEW);

    EndModal(wxID_OK);
}

void mmTransDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
#ifdef __WXMSW__
    if (object_in_focus_ != wxID_CANCEL && wxGetKeyState(WXK_ESCAPE))
            return m_button_cancel->SetFocus();

    if (object_in_focus_ != wxID_CANCEL) {
        return;
    }
#endif

    if (m_new_trx || m_duplicate)
    {
        const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
        mmAttachmentManage::DeleteAllAttachments(RefType, -1);
        Model_CustomFieldData::instance().DeleteAllData(RefType, -1);
    }
    EndModal(wxID_CANCEL);
}

void mmTransDialog::SetTooltips()
{
    bSplit_->UnsetToolTip();
    skip_tooltips_init_ = true;
    if (this->m_local_splits.empty())
        mmToolTip(bSplit_, _("Use split Categories"));
    else {
        const Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
        const Model_Account::Data* account = Model_Account::instance().get(m_trx_data.ACCOUNTID);
        if (account)
            currency = Model_Account::currency(account);

        bSplit_->SetToolTip(Model_Splittransaction::get_tooltip(m_local_splits, currency));
    }
    if (!m_new_trx) return;

    m_textAmount->UnsetToolTip();
    toTextAmount_->UnsetToolTip();
    cbAccount_->UnsetToolTip();
    cbPayee_->UnsetToolTip();

    if (m_transfer)
    {
        mmToolTip(cbAccount_, _("Specify account the money is taken from"));
        mmToolTip(cbPayee_, _("Specify account the money is moved to"));
        mmToolTip(m_textAmount, _("Specify the transfer amount in the From Account."));

        if (m_advanced)
            mmToolTip(toTextAmount_, _("Specify the transfer amount in the To Account"));
    }
    else
    {
        mmToolTip(m_textAmount, _("Specify the amount for this transaction"));
        mmToolTip(cbAccount_, _("Specify account for the transaction"));
        if (!Model_Checking::is_deposit(m_trx_data.TRANSCODE))
            mmToolTip(cbPayee_, _("Specify to whom the transaction is going to"));
        else
            mmToolTip(cbPayee_, _("Specify where the transaction is coming from"));
    }

    // Not dynamically changed tooltips
    mmToolTip(dpc_, _("Specify the date of the transaction"));
    mmToolTip(choiceStatus_, _("Specify the status for the transaction"));
    mmToolTip(transaction_type_, _("Specify the type of transactions to be created."));
    mmToolTip(bSplit_, _("Use split Categories"));
    mmToolTip(textNumber_, _("Specify any associated check number or transaction number"));
    mmToolTip(textNotes_, _("Specify any text notes you want to add to this transaction."));
    mmToolTip(cAdvanced_, _("Allows the setting of different amounts in the FROM and TO accounts."));
}

void mmTransDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    if (m_new_trx || m_duplicate) {
        mmAttachmentManage::DeleteAllAttachments(RefType, -1);
        Model_CustomFieldData::instance().DeleteAllData(RefType, -1);
    }
    EndModal(wxID_CANCEL);
}

void mmTransDialog::OnMoreFields(wxCommandEvent& WXUNUSED(event))
{
    wxBitmapButton* button = static_cast<wxBitmapButton*>(FindWindow(ID_DIALOG_TRANS_CUSTOMFIELDS));

    if (button)
        button->SetBitmap(mmBitmap(m_custom_fields->IsCustomPanelShown() ? png::RIGHTARROW : png::LEFTARROW, mmBitmapButtonSize));

    m_custom_fields->ShowHideCustomPanel();

    this->SetMinSize(wxSize(0, 0));
    this->Fit();
}
