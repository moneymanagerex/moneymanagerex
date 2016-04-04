/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011 Stefano Giorgio
 Copyright (C) 2011-2016 Nikolay

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
#include "attachmentdialog.h"
#include "customfielddialog.h"
#include "categdialog.h"
#include "constants.h"
#include "images_list.h"
#include "mmSimpleDialogs.h"
#include "mmtextctrl.h"
#include "paths.h"
#include "splittransactionsdialog.h"
#include "util.h"
#include "validators.h"
#include "webapp.h"

#include "mmOption.h"
#include "model/Model_Account.h"
#include "model/Model_Attachment.h"
#include "model/Model_Category.h"
#include "model/Model_CustomFieldData.h"
#include "model/Model_Subcategory.h"

#include <wx/numformatter.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmTransDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmTransDialog, wxDialog)
    EVT_BUTTON(wxID_OK, mmTransDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmTransDialog::OnCancel)
    EVT_BUTTON(wxID_VIEW_DETAILS, mmTransDialog::OnCategs)
    EVT_BUTTON(wxID_FILE, mmTransDialog::OnAttachments)
    EVT_BUTTON(ID_DIALOG_TRANS_CUSTOMFIELDS, mmTransDialog::OnCustomFields)
    EVT_CLOSE(mmTransDialog::OnQuit)
    EVT_MOVE(mmTransDialog::OnMove)
    EVT_CHOICE(ID_DIALOG_TRANS_TYPE, mmTransDialog::OnTransTypeChanged)
    EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, mmTransDialog::OnAdvanceChecked)
    EVT_CHECKBOX(wxID_FORWARD, mmTransDialog::OnSplitChecked)
    EVT_CHILD_FOCUS(mmTransDialog::onFocusChange)
    EVT_SPIN(wxID_ANY,mmTransDialog::OnSpin)
    EVT_DATE_CHANGED(ID_DIALOG_TRANS_BUTTONDATE, mmTransDialog::OnDateChanged)
    EVT_COMBOBOX(wxID_ANY, mmTransDialog::OnAccountOrPayeeUpdated)
    EVT_MENU(wxID_ANY, mmTransDialog::onNoteSelected)
wxEND_EVENT_TABLE()

mmTransDialog::mmTransDialog(wxWindow* parent
    , int account_id
    , int transaction_id
    , bool duplicate
    , int type
) : m_currency(nullptr)
    , m_to_currency(nullptr)
    , CustomFieldDialog_(nullptr)
    , categUpdated_(false)
    , m_transfer(false)
    , m_advanced(false)
    , m_duplicate(duplicate)
    , skip_account_init_(false)
    , skip_payee_init_(false)
    , skip_status_init_(false)
    , skip_date_init_(false)
    , skip_notes_init_(false)
    , skip_category_init_(false)
    , skip_tooltips_init_(false)
    , skip_amount_init_(false)
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
        for (const auto& item : Model_Checking::splittransaction(transaction))
            local_splits.push_back({ item.CATEGID, item.SUBCATEGID, item.SPLITTRANSAMOUNT });
    }

    Model_Account::Data* acc = Model_Account::instance().get(m_trx_data.ACCOUNTID);
    if (acc)
        m_currency = Model_Account::currency(acc);
    else
        m_currency = Model_Currency::GetBaseCurrency();

    if (m_transfer) 
    {
        Model_Account::Data* to_acc = Model_Account::instance().get(m_trx_data.TOACCOUNTID);
        if (to_acc) 
            m_to_currency = Model_Account::currency(to_acc);
        if (m_to_currency) 
            m_advanced = !m_new_trx && (m_currency->CURRENCYID != m_to_currency->CURRENCYID || m_trx_data.TRANSAMOUNT != m_trx_data.TOTRANSAMOUNT);
    }

    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, "", wxDefaultPosition, wxSize(500, 400), style);
    dataToControls();
    CallAfter(&mmTransDialog::OnStartupCustomFields);
}

mmTransDialog::~mmTransDialog()
{
}

bool mmTransDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());
    m_duplicate ? SetDialogTitle(_("Duplicate Transaction"))
                : SetDialogTitle(m_new_trx ? _("New Transaction") : _("Edit Transaction"));

    Centre();
    Fit();
    return TRUE;
}

void mmTransDialog::dataToControls()
{
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
        choiceStatus_->SetSelection(Model_Checking::status(m_trx_data.STATUS));
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
                m_trx_data.TOTRANSAMOUNT = m_trx_data.TRANSAMOUNT 
                    * (m_to_currency ? m_currency->BASECONVRATE / m_to_currency->BASECONVRATE : 1);
            toTextAmount_->SetValue(m_trx_data.TOTRANSAMOUNT);
        }
        else
            toTextAmount_->ChangeValue("");

        if (m_trx_data.TRANSID != -1)
            textAmount_->SetValue(m_trx_data.TRANSAMOUNT);
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
        const auto &accounts = Model_Account::instance().find(
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

        cbAccount_->Enable(!acc_closed && (account_list.size() > 1));

        cbAccount_->SetEvtHandlerEnabled(true);
        skip_account_init_ = true;
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
            if (!Model_Checking::is_deposit(m_trx_data.TRANSCODE))
                payee_label_->SetLabelText(_("Payee"));
            else
                payee_label_->SetLabelText(_("From"));

            account_label_->SetLabelText(_("Account"));
            m_trx_data.TOACCOUNTID = -1;

            wxArrayString all_payees = Model_Payee::instance().all_payee_names();
            if (!all_payees.empty())
            {
                cbPayee_->Insert(all_payees, 0);
                cbPayee_->AutoComplete(all_payees);
            }

            if (Option::instance().TransCategorySelectionNone() == 2)
            {
                cbPayee_->Enable(false);
                cbPayee_->ChangeValue(_("Unknown"));
            }

            Model_Payee::Data* payee = Model_Payee::instance().get(m_trx_data.PAYEEID);
            if (payee)
                cbPayee_->ChangeValue(payee->PAYEENAME);
        }
        else //transfer
        {
            cbPayee_->Enable(true);
            if (cSplit_->IsChecked())
            {
                cSplit_->SetValue(false);
                local_splits.clear();
            }

            if (m_new_trx && !m_duplicate)
            {
                const auto &categs = Model_Category::instance().find(Model_Category::CATEGNAME(wxGetTranslation("Transfer")));
                if (!categs.empty())
                {
                    m_trx_data.SUBCATEGID = -1;
                    m_trx_data.CATEGID = categs.begin()->CATEGID;
                    bCategory_->SetLabelText(Model_Category::full_name(m_trx_data.CATEGID, -1));
                }
            }

            cbPayee_->Insert(Model_Account::instance().all_checking_account_names(true), 0);
            Model_Account::Data *account = Model_Account::instance().get(m_trx_data.TOACCOUNTID);
            if (account)
                cbPayee_->ChangeValue(account->ACCOUNTNAME);

            cbPayee_->AutoComplete(Model_Account::instance().all_checking_account_names());

            payee_label_->SetLabelText(_("To"));
            m_trx_data.PAYEEID = -1;
            account_label_->SetLabelText(_("From"));
        }
        skip_payee_init_ = true;
        cbPayee_->SetEvtHandlerEnabled(true);
    }

    if (!skip_category_init_)
    {
        bool has_split = !local_splits.empty();
        wxString fullCategoryName;
        bCategory_->UnsetToolTip();
        if (has_split)
        {
            fullCategoryName = _("Categories");
            textAmount_->SetValue(Model_Splittransaction::get_total(local_splits));
            m_trx_data.CATEGID = -1;
            m_trx_data.SUBCATEGID = -1;
        }
        else
        {
            Model_Category::Data *category = Model_Category::instance().get(m_trx_data.CATEGID);
            Model_Subcategory::Data *subcategory = (Model_Subcategory::instance().get(m_trx_data.SUBCATEGID));
            fullCategoryName = Model_Category::full_name(category, subcategory);
            if (fullCategoryName.IsEmpty()) fullCategoryName = _("Select Category");
        }

        bCategory_->SetLabelText(fullCategoryName);
        cSplit_->SetValue(has_split);
        skip_category_init_ = true;
    }
    textAmount_->Enable(local_splits.empty());
    cSplit_->Enable(!m_transfer);

    if (!skip_notes_init_) //Notes & Transaction Number
    {
        textNumber_->SetValue(m_trx_data.TRANSACTIONNUMBER);
        textNotes_->SetValue(m_trx_data.NOTES);
        skip_notes_init_ = true;
    }

    if (!skip_tooltips_init_)
        setTooltips();
}

void mmTransDialog::CreateControls()
{
    wxBoxSizer* box_sizer1 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* box_sizer2 = new wxBoxSizer(wxVERTICAL);
    box_sizer1->Add(box_sizer2, g_flagsExpand);

    wxStaticBox* static_box = new wxStaticBox(this, wxID_ANY, _("Transaction Details"));
    wxStaticBoxSizer* box_sizer = new wxStaticBoxSizer(static_box, wxVERTICAL);
    box_sizer2->Add(box_sizer, g_flagsExpand);

    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    box_sizer->Add(flex_sizer, g_flags);

    // Date --------------------------------------------
    long date_style = wxDP_DROPDOWN | wxDP_SHOWCENTURY;

    dpc_ = new wxDatePickerCtrl(this, ID_DIALOG_TRANS_BUTTONDATE, wxDateTime::Today()
        , wxDefaultPosition, wxSize(110, -1), date_style);

    //Text field for day of the week
    itemStaticTextWeek_ = new wxStaticText(this, wxID_STATIC, "");
    // Display the day of the week

    spinCtrl_ = new wxSpinButton(this, wxID_STATIC
        , wxDefaultPosition, wxSize(18, wxSize(dpc_->GetSize()).GetHeight())
        , wxSP_VERTICAL | wxSP_ARROW_KEYS | wxSP_WRAP);
    spinCtrl_->SetRange (-32768, 32768);

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Date")), g_flags);
    wxBoxSizer* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(date_sizer);
    date_sizer->Add(dpc_, g_flags);
    date_sizer->Add(spinCtrl_, g_flags);
    date_sizer->Add(itemStaticTextWeek_, g_flags);

    // Status --------------------------------------------
    choiceStatus_ = new wxChoice(this, ID_DIALOG_TRANS_STATUS
        , wxDefaultPosition, wxSize(110, -1));

    for(const auto& i : Model_Checking::all_status())
        choiceStatus_->Append(wxGetTranslation(i), new wxStringClientData(i));

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Status")), g_flags);
    flex_sizer->Add(choiceStatus_, g_flags);

    // Type --------------------------------------------
    transaction_type_ = new wxChoice(this, ID_DIALOG_TRANS_TYPE
        , wxDefaultPosition, wxSize(110, -1));

    for (const auto& i : Model_Checking::all_type())
    {
        if (i != Model_Checking::all_type()[Model_Checking::TRANSFER] || Model_Account::instance().all().size() > 1)
            transaction_type_->Append(wxGetTranslation(i), new wxStringClientData(i));
    }

    cAdvanced_ = new wxCheckBox(this
        , ID_DIALOG_TRANS_ADVANCED_CHECKBOX, _("&Advanced")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );

    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Type")), g_flags);
    flex_sizer->Add(typeSizer);
    typeSizer->Add(transaction_type_, g_flags);
    typeSizer->Add(cAdvanced_, g_flags);

    // Amount Fields --------------------------------------------
    textAmount_ = new mmTextCtrl(this, ID_DIALOG_TRANS_TEXTAMOUNT, ""
        , wxDefaultPosition, wxSize(110, -1)
        , wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());

    toTextAmount_ = new mmTextCtrl( this, ID_DIALOG_TRANS_TOTEXTAMOUNT, ""
        , wxDefaultPosition, wxSize(110, -1)
        , wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(textAmount_, g_flags);
    amountSizer->Add(toTextAmount_, g_flags);

    flex_sizer->Add(new wxStaticText( this, wxID_STATIC, _("Amount")), g_flags);
    flex_sizer->Add(amountSizer);

    // Account ---------------------------------------------
    cbAccount_ = new wxComboBox(this, wxID_ANY, ""
        , wxDefaultPosition, wxSize(230, -1));

    account_label_ = new wxStaticText(this, wxID_STATIC, _("Account"));
    flex_sizer->Add(account_label_, g_flags);
    flex_sizer->Add(cbAccount_, g_flags);

    // Payee ---------------------------------------------
    payee_label_ = new wxStaticText(this, wxID_STATIC, _("Payee"));

    /*Note: If you want to use EVT_TEXT_ENTER(id,func) to receive wxEVT_COMMAND_TEXT_ENTER events,
      you have to add the wxTE_PROCESS_ENTER window style flag.
      If you create a wxComboBox with the flag wxTE_PROCESS_ENTER, the tab key won't jump to the next control anymore.*/
    cbPayee_ = new wxComboBox(this, ID_DIALOG_TRANS_PAYEECOMBO, ""
        , wxDefaultPosition, wxSize(230, -1));

    flex_sizer->Add(payee_label_, g_flags);
    flex_sizer->Add(cbPayee_, g_flags);

    // Split Category -------------------------------------------
    cSplit_ = new wxCheckBox(this, wxID_FORWARD
        , _("Split"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    cSplit_->SetValue(FALSE);

    flex_sizer->AddSpacer(20);  // Fill empty space.
    flex_sizer->Add(cSplit_, g_flags);

    // Category -------------------------------------------------
    bCategory_ = new wxButton(this, wxID_VIEW_DETAILS, ""
        , wxDefaultPosition, wxSize(230, -1));

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Category")), g_flags);
    flex_sizer->Add(bCategory_, g_flags);

    // Number  ---------------------------------------------
    textNumber_ = new mmTextCtrl(this
        , ID_DIALOG_TRANS_TEXTNUMBER, "", wxDefaultPosition
        , wxDefaultSize, wxTE_PROCESS_ENTER);

    wxButton* bAuto = new wxButton(this
        , ID_DIALOG_TRANS_BUTTONTRANSNUM, "...", wxDefaultPosition
        , wxSize(cbPayee_->GetSize().GetY(), cbPayee_->GetSize().GetY()));
    bAuto->Connect(ID_DIALOG_TRANS_BUTTONTRANSNUM,
        wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmTransDialog::OnAutoTransNum), nullptr, this);
    bAuto->SetToolTip(_("Populate Transaction #"));

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Number")), g_flags);
    wxBoxSizer* number_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(number_sizer, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));
    number_sizer->Add(textNumber_, g_flagsExpand);
    number_sizer->Add(bAuto, g_flags);

    // Notes ---------------------------------------------
    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Notes")), g_flags);
    wxButton* bFrequentUsedNotes = new wxButton(this, ID_DIALOG_TRANS_BUTTON_FREQENTNOTES
        , "...", wxDefaultPosition
        , wxSize(cbPayee_->GetSize().GetY(), cbPayee_->GetSize().GetY()), 0);
    bFrequentUsedNotes->SetToolTip(_("Select one of the frequently used notes"));
    bFrequentUsedNotes->Connect(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES
        , wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmTransDialog::OnFrequentUsedNotes), nullptr, this);
    
    // Attachments ---------------------------------------------
    bAttachments_ = new wxBitmapButton(this, wxID_FILE
        , mmBitmap(png::CLIP), wxDefaultPosition
        , wxSize(bFrequentUsedNotes->GetSize().GetY(), bFrequentUsedNotes->GetSize().GetY()));
    bAttachments_->SetToolTip(_("Organize attachments of this transaction"));

    wxBoxSizer* RightAlign_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(RightAlign_sizer, wxSizerFlags(g_flags).Align(wxALIGN_RIGHT));
    RightAlign_sizer->Add(bAttachments_, wxSizerFlags().Border(wxRIGHT, 5));
    RightAlign_sizer->Add(bFrequentUsedNotes, wxSizerFlags().Border(wxLEFT, 5));

    textNotes_ = new wxTextCtrl(this, ID_DIALOG_TRANS_TEXTNOTES, "", wxDefaultPosition, wxSize(-1, 120), wxTE_MULTILINE);
    box_sizer->Add(textNotes_, wxSizerFlags(g_flagsExpand).Border(wxLEFT | wxRIGHT | wxBOTTOM, 10));

    /**********************************************************************************************
     Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    box_sizer1->Add(buttons_panel, wxSizerFlags(g_flags).Center().Border(wxALL, 0));

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxButton* itemButtonOK = new wxButton(buttons_panel, wxID_OK, _("&OK "));
    itemButtonCancel_ = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));

    // Custom Fields ---------------------------------------------
    bCustomFields_ = new wxBitmapButton(buttons_panel, ID_DIALOG_TRANS_CUSTOMFIELDS
        , mmBitmap(png::EDIT_ACC));
    bCustomFields_->SetToolTip(_("Open custom fields window"));

    buttons_sizer->Add(itemButtonOK, wxSizerFlags(g_flags).Border(wxBOTTOM | wxRIGHT, 10));
    buttons_sizer->Add(itemButtonCancel_, wxSizerFlags(g_flags).Border(wxBOTTOM | wxRIGHT, 10));
    buttons_sizer->Add(bCustomFields_, wxSizerFlags(g_flags).Border(wxBOTTOM | wxRIGHT, 10));

    if (!m_new_trx && !m_duplicate) itemButtonCancel_->SetFocus();

    buttons_sizer->Realize();
    Center();
    this->SetSizer(box_sizer1);

    cbPayee_->Connect(ID_DIALOG_TRANS_PAYEECOMBO, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(mmTransDialog::OnAccountOrPayeeUpdated), nullptr, this);
    textAmount_->Connect(ID_DIALOG_TRANS_TEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmTransDialog::onTextEntered), nullptr, this);
    toTextAmount_->Connect(ID_DIALOG_TRANS_TOTEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmTransDialog::onTextEntered), nullptr, this);
    textNumber_->Connect(ID_DIALOG_TRANS_TEXTNUMBER, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmTransDialog::onTextEntered), nullptr, this);

#ifdef __WXGTK__ // Workaround for bug http://trac.wxwidgets.org/ticket/11630
    dpc_->Connect(ID_DIALOG_TRANS_BUTTONDATE, wxEVT_KILL_FOCUS
        , wxFocusEventHandler(mmTransDialog::OnDpcKillFocus), nullptr, this);
#endif

}

bool mmTransDialog::validateData()
{
    if (!textAmount_->checkValue(m_trx_data.TRANSAMOUNT))
        return false;

    Model_Account::Data* account = Model_Account::instance().get(cbAccount_->GetValue());
    if (!account || Model_Account::type(account) == Model_Account::INVESTMENT)
    {
        mmErrorDialogs::InvalidAccount((wxWindow*)cbAccount_);
        return false;
    }
    m_trx_data.ACCOUNTID = account->ACCOUNTID;

    if (!m_transfer)
    {
        wxString payee_name = cbPayee_->GetValue();
        if (payee_name.IsEmpty())
        {
            mmErrorDialogs::InvalidPayee((wxWindow*)cbPayee_);
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
                , wxString::Format(_("Do you want to add new payee: \n%s?"), payee_name)
                , _("Confirm to add new payee")
                , wxYES_NO | wxYES_DEFAULT | wxICON_WARNING);
            if (Option::instance().TransCategorySelectionNone() == 2 || msgDlg.ShowModal() == wxID_YES)
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
        m_trx_data.TOACCOUNTID = -1;

        payee->CATEGID = m_trx_data.CATEGID;
        payee->SUBCATEGID = m_trx_data.SUBCATEGID;
        Model_Payee::instance().save(payee);
        mmWebApp::MMEX_WebApp_UpdatePayee();
    }
    else //transfer
    {
        Model_Account::Data *to_account = Model_Account::instance().get(cbPayee_->GetValue());
        if (!to_account || to_account->ACCOUNTID == m_trx_data.ACCOUNTID || Model_Account::type(to_account) == Model_Account::INVESTMENT)
        {
            mmErrorDialogs::InvalidAccount((wxWindow*)cbPayee_, true);
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

    if ((cSplit_->IsChecked() && local_splits.empty())
        || (!cSplit_->IsChecked() && Model_Category::full_name(m_trx_data.CATEGID, m_trx_data.SUBCATEGID).empty()))
    {
        mmErrorDialogs::InvalidCategory((wxWindow*)bCategory_, false);
        return false;
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

void mmTransDialog::onFocusChange(wxChildFocusEvent& event)
{
    wxWindow *w = event.GetWindow();
    if (w)
        object_in_focus_ = w->GetId();

    m_currency = Model_Currency::GetBaseCurrency();
    wxString accountName = cbAccount_->GetValue();
    wxString toAccountName = cbPayee_->GetValue();
    for (const auto& acc : Model_Account::instance().all_checking_account_names()){
        if (acc.CmpNoCase(accountName) == 0) accountName = acc;
        if (acc.CmpNoCase(toAccountName) == 0) toAccountName = acc;
    }

    const Model_Account::Data* account = Model_Account::instance().get(accountName);
    if (account)
    {
        m_currency = Model_Account::currency(account);
        cbAccount_->SetValue(account->ACCOUNTNAME);
        m_trx_data.ACCOUNTID = account->ACCOUNTID;
    }

    if (!m_transfer)
    {
        Model_Payee::Data * payee = Model_Payee::instance().get(cbPayee_->GetValue());
        if (payee) 
        {
            cbPayee_->ChangeValue(payee->PAYEENAME);
            setCategoryForPayee(payee);
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

    if (object_in_focus_ == textAmount_->GetId())
        textAmount_->SelectAll();
    else 
    {
        if (textAmount_->Calculate()) 
            textAmount_->GetDouble(m_trx_data.TRANSAMOUNT);
        skip_amount_init_ = false;
    }

    if (m_advanced && object_in_focus_ == toTextAmount_->GetId())
        toTextAmount_->SelectAll();
    else 
    {
        if (toTextAmount_->Calculate())
            toTextAmount_->GetDouble(m_trx_data.TOTRANSAMOUNT);
    }

    dataToControls();
    event.Skip();
}

void mmTransDialog::activateSplitTransactionsDlg()
{
    bool bDeposit = Model_Checking::is_deposit(m_trx_data.TRANSCODE);

    if (!textAmount_->GetDouble(m_trx_data.TRANSAMOUNT))
        m_trx_data.TRANSAMOUNT = 0;

    if (!Model_Category::full_name(m_trx_data.CATEGID, m_trx_data.SUBCATEGID).empty() && local_splits.empty())
    {
        Split s;
        s.SPLITTRANSAMOUNT = m_trx_data.TRANSAMOUNT;
        s.CATEGID = m_trx_data.CATEGID;
        s.SUBCATEGID = m_trx_data.SUBCATEGID;
        local_splits.push_back(s);
    }

    SplitTransactionDialog dlg(this, local_splits
        , bDeposit ? Model_Checking::DEPOSIT : Model_Checking::WITHDRAWAL
        , m_trx_data.ACCOUNTID
        , m_trx_data.TRANSAMOUNT);

    if (dlg.ShowModal() == wxID_OK)
    {
        local_splits = dlg.getResult();
    }
    if (!local_splits.empty()) 
    {
        m_trx_data.TRANSAMOUNT = Model_Splittransaction::get_total(local_splits);
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
        itemStaticTextWeek_->SetLabelText(wxGetTranslation(date.GetWeekDayName(date.GetWeekDay())));
        m_trx_data.TRANSDATE = date.FormatISODate();
    }
}

void mmTransDialog::OnSpin(wxSpinEvent& event)
{
    wxDateTime date = dpc_->GetValue();
    int value = event.GetPosition();

    date = date.Add(wxDateSpan::Days(value));
    dpc_->SetValue(date);
    spinCtrl_->SetValue(0);

    //process date change event for set weekday name
    wxDateEvent dateEvent(dpc_, date, wxEVT_DATE_CHANGED);
    GetEventHandler()->ProcessEvent(dateEvent);

    event.Skip();
}

void mmTransDialog::OnTransTypeChanged(wxCommandEvent& event)
{
    const wxString old_type = m_trx_data.TRANSCODE;
    wxStringClientData *client_obj = (wxStringClientData *) event.GetClientObject();
    if (client_obj) m_trx_data.TRANSCODE = client_obj->GetData();
    if (old_type != m_trx_data.TRANSCODE)
    {
        m_transfer = Model_Checking::is_transfer(m_trx_data.TRANSCODE);
        if (!m_transfer) m_trx_data.TOTRANSAMOUNT = m_trx_data.TRANSAMOUNT;
        if (!m_transfer) m_trx_data.TOACCOUNTID = -1;
        if (m_transfer) m_trx_data.PAYEEID = -1;
        skip_payee_init_ = false;
        skip_account_init_ = false;
        skip_tooltips_init_ = false;
        dataToControls();
    }
}

void mmTransDialog::OnAccountOrPayeeUpdated(wxCommandEvent& event)
{
    // Filtering the combobox as the user types because on Mac autocomplete function doesn't work
    // PLEASE DO NOT REMOVE!!!
    #if defined (__WXMAC__)
        if (!m_transfer)
        {
            wxString payeeName = event.GetString();
            if (cbPayee_->GetSelection() == -1) // make sure nothing is selected (ex. user presses down arrow)
            {
                cbPayee_->SetEvtHandlerEnabled(false); // things will crash if events are handled during Clear
                cbPayee_->Clear();
                Model_Payee::Data_Set filtd = Model_Payee::instance().FilterPayees(payeeName);
                std::sort(filtd.rbegin(), filtd.rend(), SorterByPAYEENAME());
                for (int nn=0; nn<filtd.size(); nn++) {
                    cbPayee_->Insert(filtd[nn].PAYEENAME, 0);
                }
                cbPayee_->ChangeValue(payeeName);
                cbPayee_->SetInsertionPointEnd();
                cbPayee_->SetEvtHandlerEnabled(true);
            }
        }
    #endif
    wxChildFocusEvent evt;
    onFocusChange(evt);
}

void mmTransDialog::setCategoryForPayee(const Model_Payee::Data *payee)
{
    // Only for new transactions: if user want to autofill last category used for payee.
    // If this is a Split Transaction, ignore displaying last category for payee
    if (Option::instance().TransCategorySelectionNone() != 0
        && !categUpdated_ && local_splits.empty() && m_new_trx && !m_duplicate)
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
    }
}

void mmTransDialog::OnSplitChecked(wxCommandEvent& /*event*/)
{
    if (cSplit_->IsChecked())
    {
        activateSplitTransactionsDlg();
    }
    else
    {
        if (local_splits.size() > 1)
        {
            //Delete split items first (data protection)
            cSplit_->SetValue(true);
        }
        else
        {
            if (local_splits.size() == 1)
            {
                m_trx_data.CATEGID = local_splits.begin()->CATEGID;
                m_trx_data.SUBCATEGID = local_splits.begin()->SUBCATEGID;
                m_trx_data.TRANSAMOUNT = local_splits.begin()->SPLITTRANSAMOUNT;

                if (m_trx_data.TRANSAMOUNT < 0)
                {
                    m_trx_data.TRANSAMOUNT = -m_trx_data.TRANSAMOUNT;
                    transaction_type_->SetSelection(Model_Checking::WITHDRAWAL);
                }
            }
            else
                m_trx_data.TRANSAMOUNT = 0;

            local_splits.clear();
        }
    }
    skip_category_init_ = false;
    skip_tooltips_init_ = false;
    dataToControls();
}

void mmTransDialog::OnAutoTransNum(wxCommandEvent& /*event*/)
{
    double next_number = 0, temp_num;
    for (const auto &num : Model_Checking::instance()
        .find(Model_Checking::ACCOUNTID(m_trx_data.ACCOUNTID)))
    {
        if (num.TRANSACTIONNUMBER.empty() || !num.TRANSACTIONNUMBER.IsNumber()) continue;
        if (num.TRANSACTIONNUMBER.ToDouble(&temp_num) && temp_num > next_number)
            next_number = temp_num;
    }

    next_number++;
    textNumber_->SetValue(wxNumberFormatter::ToString(next_number, 0, wxNumberFormatter::Style_None));
}

void mmTransDialog::OnAdvanceChecked(wxCommandEvent& /*event*/)
{
    m_advanced = cAdvanced_->IsChecked();
    skip_amount_init_ = false;
    dataToControls();
}

void mmTransDialog::OnCategs(wxCommandEvent& /*event*/)
{
    if (cSplit_->IsChecked())
    {
        activateSplitTransactionsDlg();
    }
    else
    {
        mmCategDialog dlg(this, true, false);
        dlg.setTreeSelection(m_trx_data.CATEGID, m_trx_data.SUBCATEGID);
        if (dlg.ShowModal() == wxID_OK)
        {
            m_trx_data.CATEGID = dlg.getCategId();
            m_trx_data.SUBCATEGID = dlg.getSubCategId();
            bCategory_->SetLabelText(dlg.getFullCategName());
            categUpdated_ = true;
        }
    }
    skip_amount_init_ = false;
    skip_tooltips_init_ = false;
    dataToControls();
}

void mmTransDialog::OnAttachments(wxCommandEvent& /*event*/)
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    int TransID = m_trx_data.TRANSID;
    if (m_duplicate) TransID = -1;
    mmAttachmentDialog dlg(this, RefType, TransID);
    dlg.ShowModal();
}

void mmTransDialog::OnCustomFields(wxCommandEvent& /*event*/)
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    int TransID = m_trx_data.TRANSID;
    if (m_duplicate) TransID = -1;
    CustomFieldDialog_ = new mmCustomFieldDialog(this, GetScreenPosition(), GetSize(), RefType, TransID);
    CustomFieldDialog_->Show();
}

void mmTransDialog::onTextEntered(wxCommandEvent& WXUNUSED(event))
{
    if (object_in_focus_ == textAmount_->GetId()) 
    {
        if (textAmount_->Calculate())
            textAmount_->GetDouble(m_trx_data.TRANSAMOUNT);
        skip_amount_init_ = false;
        dataToControls();
    }
    else if (object_in_focus_ == toTextAmount_->GetId())
        toTextAmount_->Calculate();
    else if (object_in_focus_ == textNumber_->GetId())
        textNotes_->SetFocus();

}

void mmTransDialog::OnFrequentUsedNotes(wxCommandEvent& WXUNUSED(event))
{
    Model_Checking::getFrequentUsedNotes(frequentNotes_, m_trx_data.ACCOUNTID);
    wxMenu menu;
    int id = wxID_HIGHEST;
    for (const auto& entry : frequentNotes_) 
    {
        const wxString& label = entry.Mid(0, 30) + (entry.size() > 30 ? "..." : "");
        menu.Append(++id, label);
    }

    if (!frequentNotes_.empty())
        PopupMenu(&menu);
}

void mmTransDialog::onNoteSelected(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_HIGHEST;
    if (i > 0 && i <= (int)frequentNotes_.size())
        textNotes_->ChangeValue(frequentNotes_[i - 1]);
}

void mmTransDialog::OnStartupCustomFields()
{
    if (Model_Infotable::instance().OpenCustomDialog(Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION)))
    {
        wxCommandEvent evt;
        mmTransDialog::OnCustomFields(evt);
    }
}

void mmTransDialog::OnOk(wxCommandEvent& event)
{
    m_trx_data.STATUS = "";
    m_trx_data.NOTES = textNotes_->GetValue();
    m_trx_data.TRANSACTIONNUMBER = textNumber_->GetValue();
    m_trx_data.TRANSDATE = dpc_->GetValue().FormatISODate();
    wxStringClientData* status_obj = (wxStringClientData*) choiceStatus_->GetClientObject(choiceStatus_->GetSelection());
    if (status_obj) m_trx_data.STATUS = Model_Checking::toShortStatus(status_obj->GetData());

    if (!validateData()) return;

    Model_Checking::Data *r = Model_Checking::instance().get(m_trx_data.TRANSID);
    if (m_new_trx || m_duplicate)
        r = Model_Checking::instance().create();

    Model_Checking::putDataToTransaction(r, m_trx_data);

    m_trx_data.TRANSID = Model_Checking::instance().save(r);

    Model_Splittransaction::Data_Set splt;
    for (const auto& entry : local_splits) 
    {
        Model_Splittransaction::Data *s = Model_Splittransaction::instance().create();
        s->CATEGID = entry.CATEGID;
        s->SUBCATEGID = entry.SUBCATEGID;
        s->SPLITTRANSAMOUNT = entry.SPLITTRANSAMOUNT;
        splt.push_back(*s);
    }
    Model_Splittransaction::instance().update(splt, m_trx_data.TRANSID);

    if (CustomFieldDialog_)
        CustomFieldDialog_->OnSave(true);

    if (m_new_trx || m_duplicate)
    {
        const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
        mmAttachmentManage::RelocateAllAttachments(RefType, -1, m_trx_data.TRANSID);
        Model_CustomFieldData::instance().RelocateAllData(RefType, -1, m_trx_data.TRANSID);
    }

    const Model_Checking::Data& tran(*r);
    Model_Checking::Full_Data trx(tran);
    wxLogDebug("%s", trx.to_json());
    EndModal(wxID_OK);
}

void mmTransDialog::OnCancel(wxCommandEvent& /*event*/)
{
#ifndef __WXMAC__
    if (object_in_focus_ != itemButtonCancel_->GetId() && wxGetKeyState(wxKeyCode(WXK_ESCAPE))) 
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

void mmTransDialog::setTooltips()
{
    bCategory_->UnsetToolTip();
    skip_tooltips_init_ = true;
    if (this->local_splits.empty())
        bCategory_->SetToolTip(_("Specify the category for this transaction"));
    else {
        const Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
        const Model_Account::Data* account = Model_Account::instance().get(m_trx_data.ACCOUNTID);
        if (account)
            currency = Model_Account::currency(account);

        bCategory_->SetToolTip(Model_Splittransaction::get_tooltip(local_splits, currency));
    }
    if (!m_new_trx) return;

    textAmount_->UnsetToolTip();
    toTextAmount_->UnsetToolTip();
    cbAccount_->UnsetToolTip();
    cbPayee_->UnsetToolTip();

    if (m_transfer)
    {
        cbAccount_->SetToolTip(_("Specify account the money is taken from"));
        cbPayee_->SetToolTip(_("Specify account the money is moved to"));
        textAmount_->SetToolTip(_("Specify the transfer amount in the From Account."));

        if (m_advanced)
            toTextAmount_->SetToolTip(_("Specify the transfer amount in the To Account"));
    }
    else
    {
        textAmount_->SetToolTip(_("Specify the amount for this transaction"));
        cbAccount_->SetToolTip(_("Specify account for the transaction"));
        if (!Model_Checking::is_deposit(m_trx_data.TRANSCODE))
            cbPayee_->SetToolTip(_("Specify to whom the transaction is going to"));
        else
            cbPayee_->SetToolTip(_("Specify where the transaction is coming from"));
    }

    //Permanent
    dpc_->SetToolTip(_("Specify the date of the transaction"));
    spinCtrl_->SetToolTip(_("Retard or advance the date of the transaction"));
    choiceStatus_->SetToolTip(_("Specify the status for the transaction"));
    transaction_type_->SetToolTip(_("Specify the type of transactions to be created."));
    cSplit_->SetToolTip(_("Use split Categories"));
    textNumber_->SetToolTip(_("Specify any associated check number or transaction number"));
    textNotes_->SetToolTip(_("Specify any text notes you want to add to this transaction."));
    cAdvanced_->SetToolTip(_("Allows the setting of different amounts in the FROM and TO accounts."));
}

void mmTransDialog::OnQuit(wxCloseEvent& /*event*/)
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    if (m_new_trx || m_duplicate)
        mmAttachmentManage::DeleteAllAttachments(RefType, m_trx_data.TRANSID);
    EndModal(wxID_CANCEL);
}

void mmTransDialog::OnMove(wxMoveEvent& /*event*/)
{
    if (CustomFieldDialog_)
        CustomFieldDialog_->OnMove(GetScreenPosition(), GetSize());
}