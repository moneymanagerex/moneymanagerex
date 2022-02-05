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
    EVT_CHILD_FOCUS(mmTransDialog::OnFocusChange)
    EVT_DATE_CHANGED(ID_DIALOG_TRANS_BUTTONDATE, mmTransDialog::OnDateChanged)
    EVT_SPIN(ID_DIALOG_TRANS_DATE_SPINNER, mmTransDialog::OnTransDateSpin)
    EVT_COMBOBOX(ID_DIALOG_TRANS_PAYEECOMBO, mmTransDialog::OnAccountOrPayeeUpdated)
    EVT_TEXT_ENTER(ID_DIALOG_TRANS_PAYEECOMBO, mmTransDialog::OnPayeeDialog)
    EVT_COMBOBOX(ID_DIALOG_TRANS_FROMACCOUNT, mmTransDialog::OnFromAccountUpdated)
    EVT_BUTTON(wxID_VIEW_DETAILS, mmTransDialog::OnCategs)
    EVT_CHOICE(ID_DIALOG_TRANS_TYPE, mmTransDialog::OnTransTypeChanged)
    EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, mmTransDialog::OnAdvanceChecked)
    EVT_CHECKBOX(wxID_FORWARD, mmTransDialog::OnSplitChecked)
    EVT_BUTTON(wxID_FILE, mmTransDialog::OnAttachments)
    EVT_BUTTON(ID_DIALOG_TRANS_CUSTOMFIELDS, mmTransDialog::OnMoreFields)
    EVT_MENU_RANGE(wxID_LOWEST, wxID_LOWEST + 20, mmTransDialog::OnNoteSelected)
    EVT_MENU_RANGE(wxID_HIGHEST , wxID_HIGHEST + 8, mmTransDialog::OnColourSelected)
    EVT_BUTTON(wxID_INFO, mmTransDialog::OnColourButton)
    EVT_BUTTON(wxID_OK, mmTransDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmTransDialog::OnCancel)
    EVT_CLOSE(mmTransDialog::OnQuit)
wxEND_EVENT_TABLE()

void mmTransDialog::SetEventHandlers()
{
    cbPayee_->Connect(ID_DIALOG_TRANS_PAYEECOMBO, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(mmTransDialog::OnAccountOrPayeeUpdated), nullptr, this);
    cbAccount_->Connect(ID_DIALOG_TRANS_FROMACCOUNT, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(mmTransDialog::OnFromAccountUpdated), nullptr, this);
    cbPayee_->Bind(wxEVT_CHAR_HOOK, &mmTransDialog::OnComboTabAction, this);
    m_textAmount->Connect(ID_DIALOG_TRANS_TEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmTransDialog::OnTextEntered), nullptr, this);
    toTextAmount_->Connect(ID_DIALOG_TRANS_TOTEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmTransDialog::OnTextEntered), nullptr, this);
    textNumber_->Connect(ID_DIALOG_TRANS_TEXTNUMBER, wxEVT_COMMAND_TEXT_ENTER
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
    , categUpdated_(false)
    , m_advanced(false)
    , m_current_balance(current_balance)
    , m_account_id(account_id)
    , m_currency(nullptr)
    , m_to_currency(nullptr)
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

    Model_Account::Data* acc = Model_Account::instance().get(m_trx_data.ACCOUNTID);
    if (acc)
        m_currency = Model_Account::currency(acc);
    else
        m_currency = Model_Currency::GetBaseCurrency();

    if (m_transfer)
    {
        Model_Account::Data* to_acc = Model_Account::instance().get(m_trx_data.TOACCOUNTID);
        if (to_acc) {
            m_to_currency = Model_Account::currency(to_acc);
        }
        if (m_to_currency) {
            m_advanced = !m_new_trx
                && (m_currency->CURRENCYID != m_to_currency->CURRENCYID
                    || m_trx_data.TRANSAMOUNT != m_trx_data.TOTRANSAMOUNT);
        }
    }

    int ref_id = (m_new_trx) ? -1 : m_trx_data.TRANSID;
    m_custom_fields = new mmCustomDataTransaction(this, ref_id, ID_CUSTOMFIELD);

    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, style, name);
    this->SetMinSize(wxSize(500, 400));

    dataToControls();
}

mmTransDialog::~mmTransDialog()
{}

bool mmTransDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    SetEvtHandlerEnabled(false);
    CreateControls();

    SetIcon(mmex::getProgramIcon());
    m_duplicate ? SetDialogTitle(_("Duplicate Transaction")) : SetDialogTitle(m_new_trx ? _("New Transaction") : _("Edit Transaction"));

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
    Fit();

    SetEventHandlers();
    SetEvtHandlerEnabled(true);

    return TRUE;
}

void mmTransDialog::dataToControls()
{
    Model_Checking::getFrequentUsedNotes(frequentNotes_, m_trx_data.ACCOUNTID);
    wxButton* bFrequentUsedNotes = static_cast<wxButton*>(FindWindow(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES));
    bFrequentUsedNotes->Enable(!frequentNotes_.empty());
    
    bColours_->SetBackgroundColour(getUDColour(m_trx_data.FOLLOWUPID));

    if (!skip_date_init_) //Date
    {
        wxDateTime trx_date;
        trx_date.ParseDate(m_trx_data.TRANSDATE);
        dpc_->SetValue(trx_date);
        dpc_->SetFocus();
        //process date change event for set weekday name
        wxDateEvent dateEvent(dpc_, trx_date, wxEVT_DATE_CHANGED);
        GetEventHandler()->ProcessEvent(dateEvent);
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
        if (m_transfer)
        {
            if (!m_advanced)
            {
                double exch = 1;
                if (m_to_currency)
                {
                    const double convRateTo = Model_CurrencyHistory::getDayRate(m_to_currency->CURRENCYID, m_trx_data.TRANSDATE);
                    if (convRateTo > 0)
                    {
                        const double convRate = Model_CurrencyHistory::getDayRate(m_currency->CURRENCYID, m_trx_data.TRANSDATE);
                        exch = convRate / convRateTo;
                    }
                }
                m_trx_data.TOTRANSAMOUNT = m_trx_data.TRANSAMOUNT * exch;
            }
            toTextAmount_->SetValue(m_trx_data.TOTRANSAMOUNT, Model_Currency::precision(m_trx_data.TOACCOUNTID));
        }
        else
            toTextAmount_->ChangeValue("");

        if (m_trx_data.TRANSID != -1)
        {
            m_textAmount->SetValue(m_trx_data.TRANSAMOUNT, Model_Currency::precision(m_trx_data.ACCOUNTID));
        }
        skip_amount_init_ = true;
    }

    if (!skip_account_init_) //Account
    {
        cbAccount_->SetEvtHandlerEnabled(false);
        cbAccount_->Clear();
        const wxArrayString account_list = Model_Account::instance().all_checking_account_names(true);
        cbAccount_->Append(account_list);
        cbAccount_->AutoComplete(account_list);

        bool acc_closed = false;
        const auto& accounts = Model_Account::instance().find(
            Model_Account::ACCOUNTTYPE(Model_Account::all_type()[Model_Account::INVESTMENT], NOT_EQUAL));
        for (const auto &account : accounts)
        {
            if (account.ACCOUNTID == m_trx_data.ACCOUNTID)
            {
                cbAccount_->ChangeValue(account.ACCOUNTNAME);
                if (account.STATUS == Model_Account::all_status()[Model_Account::CLOSED])
                {
                    cbAccount_->Append(account.ACCOUNTNAME);
                    acc_closed = true;
                }
            }
        }

        if (account_list.size() == 1 && !acc_closed)
            cbAccount_->ChangeValue(account_list[0]);

        cbAccount_->Enable(account_list.size() > 1); 

        cbAccount_->SetEvtHandlerEnabled(true);
        skip_account_init_ = true;
    }

    if (m_transfer)
    {
        payee_label_->SetLabelText(_("To"));
    } else if (!Model_Checking::is_deposit(m_trx_data.TRANSCODE))
    {
        payee_label_->SetLabelText(_("Payee"));
    } else 
    {
        payee_label_->SetLabelText(_("From"));
    }
    
    if (!skip_payee_init_) //Payee or To Account
    {
        cbPayee_->SetEvtHandlerEnabled(false);

        cbPayee_->Clear();
        cbAccount_->UnsetToolTip();
        cbPayee_->UnsetToolTip();
        wxString payee_tooltip = "";
        if (!m_transfer)
        {
            account_label_->SetLabelText(_("Account"));
            if (!Model_Checking::foreignTransaction(m_trx_data))
            {
                m_trx_data.TOACCOUNTID = -1;
            }

            wxArrayString all_payees = Model_Payee::instance().all_payee_names();
            if (!all_payees.empty()) {
                cbPayee_->Insert(all_payees, 0);
                cbPayee_->AutoComplete(all_payees);
            }

            if (m_new_trx && !m_duplicate && Option::instance().TransPayeeSelection() == Option::LASTUSED
                && (-1 != m_account_id))
            {
                Model_Account::Data* account = Model_Account::instance().get(cbAccount_->GetValue());
                Model_Checking::Data_Set transactions = Model_Checking::instance().find(
                    Model_Checking::TRANSCODE(Model_Checking::TRANSFER, NOT_EQUAL)
                    , Model_Checking::ACCOUNTID(account->ACCOUNTID, EQUAL)
                    , Model_Checking::TRANSDATE(wxDateTime::Today(), LESS_OR_EQUAL));

                if (!transactions.empty()) {
                    Model_Payee::Data* payee = Model_Payee::instance().get(transactions.back().PAYEEID);
                    cbPayee_->ChangeValue(payee->PAYEENAME);
                }
            }

            if (m_new_trx && (Option::instance().TransPayeeSelection() == Option::UNUSED))
            {
                cbPayee_->ChangeValue(_("Unknown"));
            }

            Model_Payee::Data* payee = Model_Payee::instance().get(m_trx_data.PAYEEID);
            if (payee)
            {
                cbPayee_->ChangeValue(payee->PAYEENAME);
            }

            mmTransDialog::SetCategoryForPayee();
        }
        else //transfer
        {
            cbPayee_->Enable(true);
            if (cSplit_->IsChecked())
            {
                cSplit_->SetValue(false);
                m_local_splits.clear();
            }

            if (m_new_trx && !m_duplicate)
            {
                const auto &categs = Model_Category::instance().find(Model_Category::CATEGNAME(_("Transfer")));
                if (!categs.empty())
                {
                    m_trx_data.SUBCATEGID = -1;
                    m_trx_data.CATEGID = categs.begin()->CATEGID;
                    bCategory_->SetLabelText(Model_Category::full_name(m_trx_data.CATEGID, -1));
                }
            }

            wxArrayString account_names = Model_Account::instance().all_checking_account_names(true);
            cbPayee_->Insert(account_names, 0);
            Model_Account::Data *account = Model_Account::instance().get(m_trx_data.TOACCOUNTID);
            if (account)
                cbPayee_->ChangeValue(account->ACCOUNTNAME);

            cbPayee_->AutoComplete(account_names);
            m_trx_data.PAYEEID = -1;
            account_label_->SetLabelText(_("From"));
        }
        skip_payee_init_ = true;
        cbPayee_->SetEvtHandlerEnabled(true);
    }

    if (!skip_category_init_)
    {
        bool has_split = !m_local_splits.empty();
        wxString fullCategoryName;
        bCategory_->UnsetToolTip();
        if (has_split)
        {
            fullCategoryName = _("Categories");
            m_textAmount->SetValue(Model_Splittransaction::get_total(m_local_splits));
            m_trx_data.CATEGID = -1;
            m_trx_data.SUBCATEGID = -1;
        }
        else
        {
            Model_Category::Data *category = Model_Category::instance().get(m_trx_data.CATEGID);
            Model_Subcategory::Data *subcategory = (Model_Subcategory::instance().get(m_trx_data.SUBCATEGID));
            fullCategoryName = Model_Category::full_name(category, subcategory);
            if (fullCategoryName.IsEmpty())
            {
                fullCategoryName = _("Select Category");
            }
        }

        bCategory_->SetLabelText(fullCategoryName);
        cSplit_->SetValue(has_split);
        skip_category_init_ = true;
    }
    m_textAmount->Enable(m_local_splits.empty());
    cSplit_->Enable(!m_transfer);

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
    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    box_sizer_left->Add(flex_sizer, g_flagsV);
    box_sizer2->Add(box_sizer_left, g_flagsExpand);

    // Date --------------------------------------------
    long date_style = wxDP_DROPDOWN | wxDP_SHOWCENTURY;

    dpc_ = new wxDatePickerCtrl(this, ID_DIALOG_TRANS_BUTTONDATE, wxDateTime::Today(), wxDefaultPosition, wxDefaultSize, date_style);

    //Text field for name of day of the week
    wxSize WeekDayNameMaxSize(wxDefaultSize);
    for (wxDateTime::WeekDay d = wxDateTime::Sun;
            d != wxDateTime::Inv_WeekDay;
            d = wxDateTime::WeekDay(d+1))
        WeekDayNameMaxSize.IncTo(GetTextExtent(
            wxGetTranslation(wxDateTime::GetEnglishWeekDayName(d))+ " "));

    itemStaticTextWeek_ = new wxStaticText(this, wxID_STATIC, "", wxDefaultPosition, WeekDayNameMaxSize, wxST_NO_AUTORESIZE);

    wxStaticText* name_label = new wxStaticText(this, wxID_STATIC, _("Date"));
    flex_sizer->Add(name_label, g_flagsH);
    name_label->SetFont(this->GetFont().Bold());
    wxBoxSizer* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(date_sizer);
    date_sizer->Add(dpc_, g_flagsH);
#if defined(__WXMSW__) || defined(__WXGTK__)
    wxSpinButton* spinCtrl = new wxSpinButton(this, ID_DIALOG_TRANS_DATE_SPINNER
        , wxDefaultPosition, wxSize(-1, dpc_->GetSize().GetHeight())
        , wxSP_VERTICAL | wxSP_ARROW_KEYS | wxSP_WRAP);
    spinCtrl->SetRange(-32768, 32768);
    mmToolTip(spinCtrl, _("Retard or advance the date of the transaction"));
    date_sizer->Add(spinCtrl, g_flagsH);
#endif
    date_sizer->Add(itemStaticTextWeek_, g_flagsH);

    // Status --------------------------------------------
    choiceStatus_ = new wxChoice(this, ID_DIALOG_TRANS_STATUS);

    for (const auto& i : Model_Checking::all_status()) {
        choiceStatus_->Append(wxGetTranslation(i), new wxStringClientData(i));
    }

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Status")), g_flagsH);
    flex_sizer->Add(choiceStatus_, g_flagsH);

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

    // Amount Fields --------------------------------------------
    m_textAmount = new mmTextCtrl(this, ID_DIALOG_TRANS_TEXTAMOUNT, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());

    toTextAmount_ = new mmTextCtrl( this, ID_DIALOG_TRANS_TOTEXTAMOUNT, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(m_textAmount, g_flagsExpand);
    amountSizer->Add(toTextAmount_, g_flagsExpand);

    wxStaticText* amount_label = new wxStaticText(this, wxID_STATIC, _("Amount"));
    amount_label->SetFont(this->GetFont().Bold());
    flex_sizer->Add(amount_label, g_flagsH);
    flex_sizer->Add(amountSizer);

    // Account ---------------------------------------------
    cbAccount_ = new wxComboBox(this, ID_DIALOG_TRANS_FROMACCOUNT);

    account_label_ = new wxStaticText(this, wxID_STATIC, _("Account"));
    account_label_->SetFont(this->GetFont().Bold());
    flex_sizer->Add(account_label_, g_flagsH);
    flex_sizer->Add(cbAccount_, g_flagsExpand);

    // Payee ---------------------------------------------
    payee_label_ = new wxStaticText(this, wxID_STATIC, _("Payee"));
    payee_label_->SetFont(this->GetFont().Bold());

    /*Note: If you want to use EVT_TEXT_ENTER(id,func) to receive wxEVT_COMMAND_TEXT_ENTER events,
      you have to add the wxTE_PROCESS_ENTER window style flag.
      If you create a wxComboBox with the flag wxTE_PROCESS_ENTER, the tab key won't jump to the next control anymore.*/
    cbPayee_ = new wxComboBox(this, ID_DIALOG_TRANS_PAYEECOMBO, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        0, NULL, wxTE_PROCESS_ENTER );

    cbPayee_->SetMaxSize(cbAccount_->GetSize());

    flex_sizer->Add(payee_label_, g_flagsH);
    flex_sizer->Add(cbPayee_, g_flagsExpand);

    // Split Category -------------------------------------------
    cSplit_ = new wxCheckBox(this, wxID_FORWARD, _("Split"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    cSplit_->SetValue(FALSE);

    flex_sizer->AddSpacer(20);  // Fill empty space.
    flex_sizer->Add(cSplit_, g_flagsH);

    // Category -------------------------------------------------
    bCategory_ = new wxButton(this, wxID_VIEW_DETAILS);

    wxStaticText* categ_label = new wxStaticText(this, wxID_STATIC, _("Category"));
    categ_label->SetFont(this->GetFont().Bold());
    flex_sizer->Add(categ_label, g_flagsH);
    flex_sizer->Add(bCategory_, g_flagsExpand);

    // Number  ---------------------------------------------
    textNumber_ = new mmTextCtrl(this, ID_DIALOG_TRANS_TEXTNUMBER, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

    wxBitmapButton* bAuto = new wxBitmapButton(this, ID_DIALOG_TRANS_BUTTONTRANSNUM, mmBitmap(png::TRXNUM, mmBitmapButtonSize));
    bAuto->Connect(ID_DIALOG_TRANS_BUTTONTRANSNUM, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmTransDialog::OnAutoTransNum), nullptr, this);
    mmToolTip(bAuto, _("Populate Transaction #"));

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Number")), g_flagsH);
    wxBoxSizer* number_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(number_sizer, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));
    number_sizer->Add(textNumber_, g_flagsExpand);
    number_sizer->Add(bAuto, g_flagsH);

    // Frequently Used Notes
    wxButton* bFrequentUsedNotes = new wxButton(this, ID_DIALOG_TRANS_BUTTON_FREQENTNOTES
        , "...", wxDefaultPosition, bAuto->GetSize(), 0);
    mmToolTip(bFrequentUsedNotes, _("Select one of the frequently used notes"));
    bFrequentUsedNotes->Connect(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES
        , wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmTransDialog::OnFrequentUsedNotes), nullptr, this);

    // Colours
    bColours_ = new wxButton(this, wxID_INFO, " ", wxDefaultPosition, bAuto->GetSize(), 0);
    //bColours->SetBackgroundColour(mmColors::userDefColor1);
    mmToolTip(bColours_, _("User Colors"));

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
    RightAlign_sizer->Add(bColours_, wxSizerFlags().Border(wxRIGHT, 5));
    RightAlign_sizer->Add(bAttachments_, wxSizerFlags());

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

    wxButton* itemButtonOK = new wxButton(buttons_panel, wxID_OK, _("&OK "));
    itemButtonCancel_ = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));

    wxBitmapButton* itemButtonHide = new wxBitmapButton(buttons_panel, ID_DIALOG_TRANS_CUSTOMFIELDS, mmBitmap(png::RIGHTARROW, mmBitmapButtonSize));
    mmToolTip(itemButtonHide, _("Show/Hide custom fields window"));
    if (m_custom_fields->GetCustomFieldsCount() == 0) {
        itemButtonHide->Hide();
    }

    buttons_sizer->Add(itemButtonOK, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    buttons_sizer->Add(itemButtonCancel_, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    buttons_sizer->Add(itemButtonHide, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));

    if (!m_new_trx && !m_duplicate) itemButtonCancel_->SetFocus();

    buttons_sizer->Realize();

    // Custom fields -----------------------------------

    m_custom_fields->FillCustomFields(box_sizer3);
    if (m_custom_fields->GetActiveCustomFieldsCount() > 0) {
        wxCommandEvent evt(wxEVT_BUTTON, ID_DIALOG_TRANS_CUSTOMFIELDS);
        this->GetEventHandler()->AddPendingEvent(evt);
    }

    Center();
    this->SetSizer(box_sizer);
}

bool mmTransDialog::ValidateData()
{
    if (!m_textAmount->checkValue(m_trx_data.TRANSAMOUNT))
        return false;

    Model_Account::Data* account = Model_Account::instance().get(cbAccount_->GetValue());
    if (!account || Model_Account::type(account) == Model_Account::INVESTMENT)
    {
        mmErrorDialogs::InvalidAccount(cbAccount_);
        return false;
    }
    m_trx_data.ACCOUNTID = account->ACCOUNTID;

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
            if (Option::instance().TransCategorySelection() == Option::UNUSED || msgDlg.ShowModal() == wxID_YES)
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

        if (Option::instance().TransCategorySelection() == Option::LASTUSED)
        {
            payee->CATEGID = m_trx_data.CATEGID;
            payee->SUBCATEGID = m_trx_data.SUBCATEGID;
            Model_Payee::instance().save(payee);
            mmWebApp::MMEX_WebApp_UpdatePayee();
        }
    }
    else //transfer
    {
        Model_Account::Data *to_account = Model_Account::instance().get(cbPayee_->GetValue());
        if (!to_account || to_account->ACCOUNTID == m_trx_data.ACCOUNTID
            || Model_Account::type(to_account) == Model_Account::INVESTMENT)
        {
            mmErrorDialogs::InvalidAccount(cbPayee_, true);
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

    if ((cSplit_->IsChecked() && m_local_splits.empty())
        || (!cSplit_->IsChecked() && Model_Category::full_name(m_trx_data.CATEGID, m_trx_data.SUBCATEGID).empty()))
    {
        mmErrorDialogs::InvalidCategory(bCategory_, false);
        return false;
    }

    /* Check if transaction is to proceed.*/
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

    return true;
}

//----------------------------------------------------------------------------
// Workaround for bug http://trac.wxwidgets.org/ticket/11630
void mmTransDialog::OnDpcKillFocus(wxFocusEvent& event)
{
    if (wxGetKeyState(WXK_TAB) && wxGetKeyState(WXK_SHIFT))
        itemButtonCancel_->SetFocus();
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
    wxWindow *w = event.GetWindow();
    if (w)
    {
        object_in_focus_ = w->GetId();
    }

    m_currency = Model_Currency::GetBaseCurrency();
    wxString accountName = cbAccount_->GetValue();
    wxString toAccountName = cbPayee_->GetValue();
    for (const auto& acc : Model_Account::instance().all_checking_account_names())
    {
        if (acc.CmpNoCase(accountName) == 0) accountName = acc;
        if (acc.CmpNoCase(toAccountName) == 0) toAccountName = acc;
    }

    const Model_Account::Data* account = Model_Account::instance().get(accountName);
    if (account)
    {
        m_currency = Model_Account::currency(account);
        if (cbAccount_->GetValue() != accountName) 
            cbAccount_->SetValue(account->ACCOUNTNAME);
    }

    if (!m_transfer)
    {
        Model_Payee::Data * payee = Model_Payee::instance().get(cbPayee_->GetValue());
        if (payee)
        {
            cbPayee_->ChangeValue(payee->PAYEENAME);
            SetCategoryForPayee(payee);
        }
        toTextAmount_->ChangeValue("");
    }
    else
    {
        const Model_Account::Data* to_account = Model_Account::instance().get(toAccountName);
        if (to_account)
        {
            m_to_currency = Model_Account::currency(to_account);
            cbPayee_->ChangeValue(to_account->ACCOUNTNAME);
            m_trx_data.TOACCOUNTID = to_account->ACCOUNTID;
        }
    }

    if (object_in_focus_ == m_textAmount->GetId())
    {
        m_textAmount->SelectAll();
    }
    else
    {
        if (m_textAmount->Calculate(Model_Currency::precision(m_trx_data.ACCOUNTID)))
        {
            m_textAmount->GetDouble(m_trx_data.TRANSAMOUNT);
        }
        skip_amount_init_ = false;
    }

    if (m_advanced && object_in_focus_ == toTextAmount_->GetId())
    {
        toTextAmount_->SelectAll();
    }
    else
    {
        if (toTextAmount_->Calculate(Model_Currency::precision(m_trx_data.TOACCOUNTID)))
        {
            toTextAmount_->GetDouble(m_trx_data.TOTRANSAMOUNT);
        }
    }

    dataToControls();
    event.Skip();
}

void mmTransDialog::ActivateSplitTransactionsDlg()
{
    bool bDeposit = Model_Checking::is_deposit(m_trx_data.TRANSCODE);

    if (!m_textAmount->GetDouble(m_trx_data.TRANSAMOUNT))
        m_trx_data.TRANSAMOUNT = 0;

    const auto full_category_name = Model_Category::full_name(m_trx_data.CATEGID, m_trx_data.SUBCATEGID);
    if (!full_category_name.empty() && m_local_splits.empty() && m_trx_data.TRANSAMOUNT != 0)
    {
        Split s;
        s.SPLITTRANSAMOUNT = m_trx_data.TRANSAMOUNT;
        s.CATEGID = m_trx_data.CATEGID;
        s.SUBCATEGID = m_trx_data.SUBCATEGID;
        m_local_splits.push_back(s);
    }

    SplitTransactionDialog dlg(this, m_local_splits
        , bDeposit ? Model_Checking::DEPOSIT : Model_Checking::WITHDRAWAL
        , m_trx_data.ACCOUNTID
        , m_trx_data.TRANSAMOUNT);

    if (dlg.ShowModal() == wxID_OK)
    {
        m_local_splits = dlg.getResult();
    }
    if (!m_local_splits.empty())
    {
        m_trx_data.TRANSAMOUNT = Model_Splittransaction::get_total(m_local_splits);
        skip_category_init_ = !dlg.isItemsChanged();
    }
}

void mmTransDialog::SetDialogTitle(const wxString& title)
{
    this->SetTitle(title);
}

//** --------------=Event handlers=------------------ **//
void mmTransDialog::OnDateChanged(wxDateEvent& event)
{
    //get weekday name
    wxDateTime date = dpc_->GetValue();
    if (event.GetDate().IsValid())
    {
        itemStaticTextWeek_->SetLabelText(wxGetTranslation(date.GetEnglishWeekDayName(date.GetWeekDay())));
        m_trx_data.TRANSDATE = date.FormatISODate();
    }
}

void mmTransDialog::OnTransDateSpin(wxSpinEvent& event)
{
    wxDateTime date = dpc_->GetValue();
    int value = event.GetPosition();
    wxSpinButton* spinCtrl = static_cast<wxSpinButton*>(event.GetEventObject());
    if (spinCtrl) spinCtrl->SetValue(0);

    date = date.Add(wxDateSpan::Days(value));
    dpc_->SetValue(date);

    //process date change event for set weekday name
    wxDateEvent dateEvent(dpc_, date, wxEVT_DATE_CHANGED);
    GetEventHandler()->ProcessEvent(dateEvent);

    event.Skip();
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
        } else {
            m_trx_data.TOTRANSAMOUNT = m_trx_data.TRANSAMOUNT;
            m_trx_data.TOACCOUNTID = -1;
        }
        dataToControls();
    }
}

#if defined (__WXMAC__)
void mmTransDialog::OnAccountOrPayeeUpdated(wxCommandEvent& event)
{
    // Filtering the combobox as the user types because on Mac autocomplete function doesn't work
    // PLEASE DO NOT REMOVE!!
    wxString payeeName = event.GetString();
    if (cbPayee_->GetSelection() == -1) // make sure nothing is selected (ex. user presses down arrow)
    {
        cbPayee_->SetEvtHandlerEnabled(false); // things will crash if events are handled during Clear
        cbPayee_->Clear();
        if (m_transfer)
        {
            Model_Account::Data_Set filtd = Model_Account::instance().FilterAccounts(payeeName,true);
            std::sort(filtd.rbegin(), filtd.rend(), SorterByACCOUNTNAME());
            for (const auto &account : filtd)
                cbPayee_->Insert(account.ACCOUNTNAME, 0);
        } else
        {
            Model_Payee::Data_Set filtd = Model_Payee::instance().FilterPayees(payeeName);        
            std::sort(filtd.rbegin(), filtd.rend(), SorterByPAYEENAME());
            for (const auto &payee : filtd)
                cbPayee_->Insert(payee.PAYEENAME, 0);
        }
        cbPayee_->ChangeValue(payeeName);
        cbPayee_->SetInsertionPointEnd();
        cbPayee_->Popup();
        cbPayee_->SetEvtHandlerEnabled(true);
    }
#else
void mmTransDialog::OnAccountOrPayeeUpdated(wxCommandEvent& WXUNUSED(event))
{
#endif
    wxChildFocusEvent evt;
    OnFocusChange(evt);
}

void mmTransDialog::OnComboTabAction(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_TAB)
    {
        if (wxIsShiftDown())
            cbPayee_->Navigate(wxNavigationKeyEvent::IsBackward);
        else 
            cbPayee_->Navigate(wxNavigationKeyEvent::IsForward);
    }
    else
        event.Skip();
}

void mmTransDialog::OnPayeeDialog(wxCommandEvent& event)
{
    if (!m_transfer)
    {
        wxString payeeName = event.GetString();
        if (payeeName.empty())
        {
            mmPayeeDialog dlg(this, true);
            dlg.DisableTools();
            dlg.ShowModal();

            int payee_id = dlg.getPayeeId();
            Model_Payee::Data* payee = Model_Payee::instance().get(payee_id);
            if (payee)
            {
                cbPayee_->ChangeValue(payee->PAYEENAME);
                cbPayee_->SetInsertionPointEnd();
            }
        }
    }
}

#if defined (__WXMAC__) 
void mmTransDialog::OnFromAccountUpdated(wxCommandEvent& event)
{
    // Filtering the combobox as the user types because on Mac autocomplete function doesn't work
    // PLEASE DO NOT REMOVE!!
    wxString accountName = event.GetString();
    if (cbAccount_->GetSelection() == -1) // make sure nothing is selected (ex. user presses down arrow)
    {
        cbAccount_->SetEvtHandlerEnabled(false); // things will crash if events are handled during Clear
        cbAccount_->Clear();
        
        Model_Account::Data_Set filtd = Model_Account::instance().FilterAccounts(accountName, true);
        std::sort(filtd.rbegin(), filtd.rend(), SorterByACCOUNTNAME());
        for (const auto &account : filtd)
            cbAccount_->Insert(account.ACCOUNTNAME, 0);
  
        cbAccount_->ChangeValue(accountName);
        cbAccount_->SetInsertionPointEnd();
        cbAccount_->Popup();
        cbAccount_->SetEvtHandlerEnabled(true);
    }
#else
void mmTransDialog::OnFromAccountUpdated(wxCommandEvent& WXUNUSED(event))
{
#endif
    wxChildFocusEvent evt;
    OnFocusChange(evt);
}

void mmTransDialog::SetCategoryForPayee(const Model_Payee::Data *payee)
{
    // Only for new transactions: if user do not want to use categories.
    // If this is a Split Transaction, ignore displaying last category for payee
    if (Option::instance().TransCategorySelection() == Option::UNUSED
        && !categUpdated_ && m_local_splits.empty() && m_new_trx && !m_duplicate)
    {
        Model_Category::Data *category = Model_Category::instance().get(_("Unknown"));
        if (!category)
        {
            category = Model_Category::instance().create();
            category->CATEGNAME = _("Unknown");
            Model_Category::instance().save(category);
        }

        m_trx_data.CATEGID = category->CATEGID;
        bCategory_->SetLabelText(_("Unknown"));
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
    if ((Option::instance().TransCategorySelection() == Option::LASTUSED ||
         Option::instance().TransCategorySelection() == Option::DEFAULT)
        && !categUpdated_ && m_local_splits.empty() && m_new_trx && !m_duplicate)
    {
        // if payee has memory of last category used then display last category for payee
        Model_Category::Data *category = Model_Category::instance().get(payee->CATEGID);
        if (category)
        {
            Model_Subcategory::Data *subcategory = (payee->SUBCATEGID != -1 ? Model_Subcategory::instance().get(payee->SUBCATEGID) : 0);
            wxString fullCategoryName = Model_Category::full_name(category, subcategory);

            m_trx_data.CATEGID = payee->CATEGID;
            m_trx_data.SUBCATEGID = payee->SUBCATEGID;
            bCategory_->SetLabelText(fullCategoryName);
            wxLogDebug("Category: %s = %.2f", bCategory_->GetLabel(), m_trx_data.TRANSAMOUNT);
        }
        else
        {
            bCategory_->SetLabelText(_("Select Category"));
            m_trx_data.CATEGID = -1;
            m_trx_data.SUBCATEGID = -1;
        }
    }
}

void mmTransDialog::OnSplitChecked(wxCommandEvent& WXUNUSED(event))
{
    if (cSplit_->IsChecked())
    {
        if (!m_textAmount->IsEmpty() && !m_textAmount->checkValue(m_trx_data.TRANSAMOUNT)) {
            cSplit_->SetValue(false);
            return;
        }
        ActivateSplitTransactionsDlg();
    }
    else
    {
        if (m_local_splits.size() > 1)
        {
            //Delete split items first (data protection)
            cSplit_->SetValue(true);
        }
        else
        {
            if (m_local_splits.size() == 1)
            {
                m_trx_data.CATEGID = m_local_splits.begin()->CATEGID;
                m_trx_data.SUBCATEGID = m_local_splits.begin()->SUBCATEGID;
                m_trx_data.TRANSAMOUNT = m_local_splits.begin()->SPLITTRANSAMOUNT;

                if (m_trx_data.TRANSAMOUNT < 0)
                {
                    m_trx_data.TRANSAMOUNT = -m_trx_data.TRANSAMOUNT;
                    transaction_type_->SetSelection(Model_Checking::WITHDRAWAL);
                }
            }
            else
            {
                m_trx_data.TRANSAMOUNT = 0;
            }
            m_local_splits.clear();
        }
    }
    skip_category_init_ = false;
    skip_tooltips_init_ = false;
    dataToControls();
}

void mmTransDialog::OnAutoTransNum(wxCommandEvent& WXUNUSED(event))
{
    auto d = Model_Checking::TRANSDATE(m_trx_data).Subtract(wxDateSpan::Days(300));
    double next_number = 0, temp_num;
    const auto numbers = Model_Checking::instance().find(Model_Checking::ACCOUNTID(m_trx_data.ACCOUNTID, EQUAL), Model_Checking::TRANSDATE(d, GREATER_OR_EQUAL));
    for (const auto &num : numbers)
    {
        if (num.TRANSACTIONNUMBER.empty() || !num.TRANSACTIONNUMBER.IsNumber()) continue;
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
    if (cSplit_->IsChecked())
    {
        ActivateSplitTransactionsDlg();
    }
    else
    {
        mmCategDialog dlg(this, true, m_trx_data.CATEGID, m_trx_data.SUBCATEGID);
        if (dlg.ShowModal() == wxID_OK)
        {
            m_trx_data.CATEGID = dlg.getCategId();
            m_trx_data.SUBCATEGID = dlg.getSubCategId();
            bCategory_->SetLabelText(dlg.getFullCategName());
            categUpdated_ = true;
        }
    }
    skip_amount_init_ = true;
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
        if (m_textAmount->Calculate(Model_Currency::precision(m_currency)))
        {
            m_textAmount->GetDouble(m_trx_data.TRANSAMOUNT);
        }
        skip_amount_init_ = false;
        dataToControls();
    }
    else if (object_in_focus_ == toTextAmount_->GetId())
    {
        if (toTextAmount_->Calculate(Model_Currency::precision(m_trx_data.TOACCOUNTID)))
        {
            toTextAmount_->GetDouble(m_trx_data.TOTRANSAMOUNT);
        }
    }
    else if (object_in_focus_ == textNumber_->GetId())
    {
        m_textAmount->SetFocus();
    }
}

void mmTransDialog::OnFrequentUsedNotes(wxCommandEvent& WXUNUSED(event))
{
    wxMenu menu;
    int id = wxID_LOWEST;
    for (const auto& entry : frequentNotes_)
    {
        const wxString& label = entry.Mid(0, 30) + (entry.size() > 30 ? "..." : "");
        menu.Append(++id, label);
    }

    if (!frequentNotes_.empty())
        PopupMenu(&menu);
}

void mmTransDialog::OnNoteSelected(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_LOWEST;
    if (i > 0 && static_cast<size_t>(i) <= frequentNotes_.size())
        textNotes_->ChangeValue(frequentNotes_[i - 1]);
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
#ifndef __WXMAC__
    if (object_in_focus_ != itemButtonCancel_->GetId() && wxGetKeyState(WXK_ESCAPE))
            return itemButtonCancel_->SetFocus();
#endif

    if (m_new_trx)
    {
        const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
        mmAttachmentManage::DeleteAllAttachments(RefType, m_trx_data.TRANSID);
        Model_CustomFieldData::instance().DeleteAllData(RefType, m_trx_data.TRANSID);
    }
    EndModal(wxID_CANCEL);
}

void mmTransDialog::SetTooltips()
{
    bCategory_->UnsetToolTip();
    skip_tooltips_init_ = true;
    if (this->m_local_splits.empty())
        mmToolTip(bCategory_, _("Specify the category for this transaction"));
    else {
        const Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
        const Model_Account::Data* account = Model_Account::instance().get(m_trx_data.ACCOUNTID);
        if (account)
            currency = Model_Account::currency(account);

        mmToolTip(bCategory_, Model_Splittransaction::get_tooltip(m_local_splits, currency));
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
    mmToolTip(cSplit_, _("Use split Categories"));
    mmToolTip(textNumber_, _("Specify any associated check number or transaction number"));
    mmToolTip(textNotes_, _("Specify any text notes you want to add to this transaction."));
    mmToolTip(cAdvanced_, _("Allows the setting of different amounts in the FROM and TO accounts."));
}

void mmTransDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    if (m_new_trx || m_duplicate) {
        mmAttachmentManage::DeleteAllAttachments(RefType, m_trx_data.TRANSID);
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

void mmTransDialog::OnColourButton(wxCommandEvent& /*event*/)
{
    wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, wxID_INFO);
    ev.SetEventObject(this);

    wxMenu* mainMenu = new wxMenu;

    wxMenuItem* menuItem = new wxMenuItem(mainMenu, wxID_HIGHEST, wxString::Format(_("Clear color"), 0));
    mainMenu->Append(menuItem);

    for (int i = 1; i <= 7; ++i)
    {
        menuItem = new wxMenuItem(mainMenu, wxID_HIGHEST + i, wxString::Format(_("Color #%i"), i));
#ifdef __WXMSW__
        menuItem->SetBackgroundColour(getUDColour(i)); //only available for the wxMSW port.
#endif
        wxBitmap bitmap(mmBitmap(png::EMPTY, mmBitmapButtonSize).GetSize());
        wxMemoryDC memoryDC(bitmap);
        wxRect rect(memoryDC.GetSize());

        memoryDC.SetBackground(wxBrush(getUDColour(i)));
        memoryDC.Clear();
        memoryDC.DrawBitmap(mmBitmap(png::EMPTY, mmBitmapButtonSize), 0, 0, true);
        memoryDC.SelectObject(wxNullBitmap);
        menuItem->SetBitmap(bitmap);

        mainMenu->Append(menuItem);
    }

    PopupMenu(mainMenu);
    delete mainMenu;
}

void mmTransDialog::OnColourSelected(wxCommandEvent& event)
{
    int selected_nemu_item = event.GetId() - wxID_HIGHEST;
    bColours_->SetBackgroundColour(getUDColour(selected_nemu_item));
    m_trx_data.FOLLOWUPID = selected_nemu_item;
}
