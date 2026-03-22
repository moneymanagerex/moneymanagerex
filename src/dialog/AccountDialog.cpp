/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2022  Mark Whalley (mark@ipx.co.uk)

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
#include <wx/valnum.h>

#include "base/constants.h"
#include "base/paths.h"
#include "base/images_list.h"
#include "util/_util.h"
#include "util/_simple.h"
#include "util/mmTextCtrl.h"
#include "util/mmCalcValidator.h"

#include "model/CurrencyModel.h"
#include "model/InfoModel.h"
#include "model/PrefModel.h"
#include "model/StockModel.h"

#include "AccountDialog.h"
#include "AttachmentDialog.h"
#include "CurrencyChoiceDialog.h"
#include "import_export/webapp.h"

enum {
    ID_DIALOG_NEWACCT_BUTTON_CURRENCY = wxID_HIGHEST + 1000,
    ID_DIALOG_NEWACCT_TEXTCTRL_ACCTNAME,
    ID_ACCTNUMBER,
    ID_DIALOG_NEWACCT_TEXTCTRL_HELDAT,
    ID_DIALOG_NEWACCT_TEXTCTRL_WEBSITE,
    ID_DIALOG_NEWACCT_TEXTCTRL_CONTACT,
    ID_DIALOG_NEWACCT_TEXTCTRL_ACCESSINFO,
    ID_DIALOG_NEWACCT_TEXTCTRL_NOTES,
    ID_DIALOG_NEWACCT_TEXTCTRL_INITBALANCE,
    ID_DIALOG_NEWACCT_COMBO_ACCTSTATUS,
    ID_DIALOG_NEWACCT_CHKBOX_FAVACCOUNT,
    ID_DIALOG_NEWACCT_COMBO_ACCTTYPE
};

wxIMPLEMENT_DYNAMIC_CLASS(AccountDialog, wxDialog);

wxBEGIN_EVENT_TABLE(AccountDialog, wxDialog)
    EVT_BUTTON(wxID_OK,                            AccountDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL,                        AccountDialog::OnCancel)
    EVT_BUTTON(ID_DIALOG_NEWACCT_BUTTON_CURRENCY,  AccountDialog::OnCurrency)
    EVT_BUTTON(wxID_FILE,                          AccountDialog::OnAttachments)
    EVT_MENU_RANGE(
        wxID_HIGHEST,
        wxID_HIGHEST + static_cast<int>(acc_img::MAX_ACC_ICON),
                                                   AccountDialog::OnCustonImage)
    EVT_CHOICE(ID_DIALOG_NEWACCT_COMBO_ACCTSTATUS, AccountDialog::OnAccountStatus)
wxEND_EVENT_TABLE()

AccountDialog::AccountDialog()
{
}

AccountDialog::AccountDialog(AccountData* account, wxWindow* parent) :
    m_account_n(account)
{
    m_images = navtree_images_list();
    m_currencyID = m_account_n->m_currency_id;
    [[maybe_unused]] const CurrencyData* currency = CurrencyModel::instance().get_id_data_n(m_currencyID);
    wxASSERT(currency);

    this->SetFont(parent->GetFont());
    Create(parent);
    this->SetMinSize(wxSize(550, 300));
    this->Connect(wxID_ANY, wxEVT_CHILD_FOCUS, wxChildFocusEventHandler(AccountDialog::OnChangeFocus), nullptr, this);
    mmThemeAutoColour(this);
}

AccountDialog::~AccountDialog()
{}

bool AccountDialog::Create(
    wxWindow* parent,
    wxWindowID id,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size,
    long style
) {
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);
    this->SetTitle(_t("Edit Account"));
    SetIcon(mmex::getProgramIcon());

    CreateControls();
    fillControls();

    OnAccountStatus();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    Centre();

    return true;
}

void AccountDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, g_flagsExpand);

    wxFlexGridSizer* grid_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    grid_sizer->AddGrowableCol(1, 1);
    itemBoxSizer3->Add(grid_sizer, g_flagsExpand);

    grid_sizer->Add(new wxStaticText(this, wxID_STATIC, _t("Account Name:")), g_flagsH);

    m_textAccountName = new wxTextCtrl(this, wxID_ANY, "");
    mmToolTip(m_textAccountName, _t("Enter the Name of the Account. This name can be renamed at any time."));
    grid_sizer->Add(m_textAccountName, g_flagsExpand);

    grid_sizer->Add(new wxStaticText(this, wxID_STATIC, _t("Account Type:")), g_flagsH);

    wxChoice* itemChoice61 = new wxChoice(this, ID_DIALOG_NEWACCT_COMBO_ACCTTYPE, wxDefaultPosition, wxDefaultSize, NavigatorTypes::instance().getAccountSelectionNames());
    mmToolTip(itemChoice61, _t("Specify the account type to be created."));
    grid_sizer->Add(itemChoice61, g_flagsExpand);
    itemChoice61->SetSelection(0);

    grid_sizer->Add(new wxStaticText(this, wxID_STATIC, _t("Account Status:")), g_flagsH);

    wxChoice* itemChoice6 = new wxChoice(this, ID_DIALOG_NEWACCT_COMBO_ACCTSTATUS);
    for (int i = 0; i < AccountStatus::size; ++i) {
        wxString status = AccountStatus(i).name();
        itemChoice6->Append(wxGetTranslation(status), new wxStringClientData(status));
    }
    mmToolTip(itemChoice6, _t("Specify if this account has been closed. Closed accounts are inactive in most calculations, reporting etc."));
    grid_sizer->Add(itemChoice6, g_flagsExpand);
    itemChoice6->SetSelection(0);

    grid_sizer->Add(new wxStaticText(this, wxID_STATIC, wxString::Format(_t("Initial Balance: %s"), "")), g_flagsH);
    m_initbalance_ctrl = new mmTextCtrl(this, ID_DIALOG_NEWACCT_TEXTCTRL_INITBALANCE, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    grid_sizer->Add(m_initbalance_ctrl, g_flagsExpand);
    mmToolTip(m_initbalance_ctrl, _t("Enter the initial balance in this account."));

    grid_sizer->Add(new wxStaticText(this, wxID_STATIC, _t("Opening Date:")), g_flagsH);

    m_initdate_ctrl = new mmDatePickerCtrl(this, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    mmToolTip(m_initdate_ctrl, _t("The date when the account was opened"));
    grid_sizer->Add(m_initdate_ctrl, g_flagsExpand);

    grid_sizer->Add(new wxStaticText(this, wxID_STATIC, _t("Currency:")), g_flagsH);

    wxString currName = _t("Select Currency");
    const CurrencyData* base_currency = CurrencyModel::instance().get_base_data_n();
    if (base_currency)
        currName = base_currency->m_name;

    wxButton* itemButton71 = new wxButton(this, ID_DIALOG_NEWACCT_BUTTON_CURRENCY, currName);
    mmToolTip(itemButton71, _t("Specify the currency to be used by this account."));
    grid_sizer->Add(itemButton71, g_flagsExpand);

    wxCheckBox* itemCheckBox10 = new wxCheckBox(this, ID_DIALOG_NEWACCT_CHKBOX_FAVACCOUNT, _t("Favorite Account"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemCheckBox10->SetValue(TRUE);
    mmToolTip(itemCheckBox10, _t("Select whether this is an account that is used often. This is used to filter accounts display view."));
    grid_sizer->AddSpacer(1);
    grid_sizer->Add(itemCheckBox10, g_flagsH);

    // Notes  ---------------------------------------------

    m_notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);
    wxPanel* notes_tab = new wxPanel(m_notebook, wxID_ANY);
    m_notebook->AddPage(notes_tab, _t("Notes"));
    wxBoxSizer *notes_sizer = new wxBoxSizer(wxVERTICAL);
    notes_tab->SetSizer(notes_sizer);

    m_notesCtrl = new wxTextCtrl(notes_tab, ID_DIALOG_NEWACCT_TEXTCTRL_NOTES, "", wxDefaultPosition, wxSize(270, 180), wxTE_MULTILINE);
    mmToolTip(m_notesCtrl, _t("Enter user notes and details about this account."));
    notes_sizer->Add(m_notesCtrl, g_flagsExpand);

    wxPanel* others_tab = new wxPanel(m_notebook, wxID_ANY);
    m_notebook->AddPage(others_tab, _t("Other"));
    wxBoxSizer *others_sizer = new wxBoxSizer(wxVERTICAL);
    others_tab->SetSizer(others_sizer);

    wxFlexGridSizer* grid_sizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    grid_sizer2->AddGrowableCol(1, 1);
    others_sizer->Add(grid_sizer2, g_flagsExpand);

    grid_sizer2->Add(new wxStaticText(others_tab, wxID_STATIC, (AccountModel::type_id(*m_account_n) == NavigatorTypes::TYPE_ID_CREDIT_CARD ? _t("Card Number:") : _t("Account Number:"))), g_flagsH);
    wxTextCtrl* itemTextCtrl6 = new wxTextCtrl(others_tab, ID_ACCTNUMBER, "", wxDefaultPosition, wxDefaultSize);
    mmToolTip(itemTextCtrl6, _t("Enter the Account Number associated with this account."));
    grid_sizer2->Add(itemTextCtrl6, g_flagsExpand);

    grid_sizer2->Add(new wxStaticText(others_tab, wxID_STATIC, _t("Held At:")), g_flagsH);
    wxTextCtrl* itemTextCtrl8 = new wxTextCtrl(others_tab, ID_DIALOG_NEWACCT_TEXTCTRL_HELDAT, "");
    mmToolTip(itemTextCtrl8, _t("Enter the name of the financial institution in which the account is held."));
    grid_sizer2->Add(itemTextCtrl8, g_flagsExpand);

    grid_sizer2->Add(new wxStaticText(others_tab, wxID_STATIC, _t("Website:")), g_flagsH);
    wxTextCtrl* itemTextCtrl10 = new wxTextCtrl(others_tab, ID_DIALOG_NEWACCT_TEXTCTRL_WEBSITE, "");
    mmToolTip(itemTextCtrl10, _t("Enter the URL of the website for the financial institution."));
    grid_sizer2->Add(itemTextCtrl10, g_flagsExpand);

    grid_sizer2->Add(new wxStaticText(others_tab, wxID_STATIC, _t("Contact:")), g_flagsH);
    wxTextCtrl* itemTextCtrl12 = new wxTextCtrl(others_tab, ID_DIALOG_NEWACCT_TEXTCTRL_CONTACT, "");
    mmToolTip(itemTextCtrl12, _t("Enter any contact information for the financial institution."));
    grid_sizer2->Add(itemTextCtrl12, g_flagsExpand);

    grid_sizer2->Add(new wxStaticText(others_tab, wxID_STATIC, _t("Access Info:")), g_flagsH);
    wxTextCtrl* itemTextCtrl14 = new wxTextCtrl(others_tab, ID_DIALOG_NEWACCT_TEXTCTRL_ACCESSINFO, "********************");
    mmToolTip(itemTextCtrl14, _t("Enter any login/access information for the financial institution. This is not secure as anyone with access to the mmb file can access it."));
    grid_sizer2->Add(itemTextCtrl14, g_flagsExpand);

    //-------------------------------------------------------------------------------------
    wxPanel* statement_tab = new wxPanel(m_notebook, wxID_ANY);
    m_notebook->AddPage(statement_tab, _t("Statement"));
    wxBoxSizer* statement_sizer = new wxBoxSizer(wxVERTICAL);
    statement_tab->SetSizer(statement_sizer);

    wxFlexGridSizer* statement_grid_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    statement_grid_sizer->AddGrowableCol(1, 1);
    statement_sizer->Add(statement_grid_sizer, g_flagsExpand);

    statement_grid_sizer->Add(new wxStaticText(statement_tab, wxID_STATIC, _t("Statement:")), g_flagsH);
    m_statement_lock_ctrl = new wxCheckBox(statement_tab, wxID_ANY, _t("Locked"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    mmToolTip(m_statement_lock_ctrl, _t("Enable or disable the transaction Lock"));
    statement_grid_sizer->Add(m_statement_lock_ctrl, g_flagsExpand);

    statement_grid_sizer->Add(new wxStaticText(statement_tab, wxID_STATIC, _t("Reconciled Date:")), g_flagsH);
    m_statement_date_ctrl = new mmDatePickerCtrl(statement_tab, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    mmToolTip(m_statement_date_ctrl, _t("The date of the transaction lock"));
    statement_grid_sizer->Add(m_statement_date_ctrl, g_flagsExpand);

    statement_grid_sizer->Add(new wxStaticText(statement_tab, wxID_STATIC, _t("Minimum Bal:")), g_flagsH);
    m_minimum_balance_ctrl = new mmTextCtrl(statement_tab, wxID_ANY, "0.00", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    mmToolTip(m_minimum_balance_ctrl, _t("Account balance lower limit. Zero to disable"));
    statement_grid_sizer->Add(m_minimum_balance_ctrl, g_flagsExpand);

    //-------------------------------------------------------------------------------------
    wxPanel* credit_tab = new wxPanel(m_notebook, wxID_ANY);
    m_notebook->AddPage(credit_tab, _t("Credit"));
    wxBoxSizer* credit_sizer = new wxBoxSizer(wxVERTICAL);
    credit_tab->SetSizer(credit_sizer);

    wxFlexGridSizer* credit_grid_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    credit_grid_sizer->AddGrowableCol(1, 1);
    credit_sizer->Add(credit_grid_sizer, g_flagsExpand);

    credit_grid_sizer->Add(new wxStaticText(credit_tab, wxID_STATIC, _t("Credit Limit:")), g_flagsH);
    m_credit_limit_ctrl = new mmTextCtrl(credit_tab, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    mmToolTip(m_credit_limit_ctrl, _t("Credit limit for the Account. Zero to disable"));
    credit_grid_sizer->Add(m_credit_limit_ctrl, g_flagsExpand);

    credit_grid_sizer->Add(new wxStaticText(credit_tab, wxID_STATIC, _t("Interest Rate:")), g_flagsH);
    m_interest_rate_ctrl = new mmTextCtrl(credit_tab, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_interest_rate_ctrl->SetAltPrecision(2);
    credit_grid_sizer->Add(m_interest_rate_ctrl, g_flagsExpand);

    credit_grid_sizer->Add(new wxStaticText(credit_tab, wxID_STATIC, _t("Payment Due Date:")), g_flagsH);
    m_payment_due_date_ctrl = new mmDatePickerCtrl(credit_tab, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    credit_grid_sizer->Add(m_payment_due_date_ctrl, g_flagsExpand);

    credit_grid_sizer->Add(new wxStaticText(credit_tab, wxID_STATIC, _t("Minimum Payment:")), g_flagsH);
    m_minimum_payment_ctrl = new mmTextCtrl(credit_tab, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    credit_grid_sizer->Add(m_minimum_payment_ctrl, g_flagsExpand);
    //-------------------------------------------------------------------------------------

    itemBoxSizer3->Add(m_notebook, g_flagsExpand);

    //Buttons
    wxPanel* itemPanel27 = new wxPanel(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer3->Add(itemPanel27, g_flagsV);

    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel27->SetSizer(itemBoxSizer28);

    m_bitmapButtons = new wxBitmapButton(itemPanel27, wxID_STATIC, wxNullBitmap);
    m_bitmapButtons->Connect(wxID_STATIC, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(AccountDialog::OnImageButton), nullptr, this);
    itemBoxSizer28->Add(m_bitmapButtons, g_flagsH);

    bAttachments_ = new wxBitmapButton(itemPanel27, wxID_FILE, mmBitmapBundle(png::CLIP));
    mmToolTip(bAttachments_, _t("Organize attachments of this account"));
    itemBoxSizer28->Add(bAttachments_, g_flagsH);

    itemBoxSizer28->AddSpacer(20);

    wxButton* itemButton29 = new wxButton(itemPanel27, wxID_OK, _t("&OK "));
    itemBoxSizer28->Add(itemButton29, g_flagsH);

    wxButton* itemButton30 = new wxButton(itemPanel27, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    itemBoxSizer28->Add(itemButton30, g_flagsH);
}

void AccountDialog::fillControls()
{
    if (!m_account_n)
        return;
    const CurrencyData* currency_p = AccountModel::instance().get_data_currency_p(*m_account_n);

    m_textAccountName->SetValue(m_account_n->m_name);

    wxTextCtrl* textCtrl = static_cast<wxTextCtrl*>(FindWindow(ID_ACCTNUMBER));
    textCtrl->SetValue(m_account_n->m_num);

    textCtrl = static_cast<wxTextCtrl*>(FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_HELDAT));
    textCtrl->SetValue(m_account_n->m_held_at);

    textCtrl = static_cast<wxTextCtrl*>(FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_WEBSITE));
    textCtrl->SetValue(m_account_n->m_website);

    textCtrl = static_cast<wxTextCtrl*>(FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_CONTACT));
    textCtrl->SetValue(m_account_n->m_contact_info);

    textCtrl = static_cast<wxTextCtrl*>(FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_NOTES));
    textCtrl->SetValue(m_account_n->m_notes);

    wxChoice* itemAcctType = static_cast<wxChoice*>(FindWindow(ID_DIALOG_NEWACCT_COMBO_ACCTTYPE));
    itemAcctType->SetStringSelection(wxGetTranslation(m_account_n->m_type_));
    itemAcctType->Enable(false);

    wxChoice* choice = static_cast<wxChoice*>(FindWindow(ID_DIALOG_NEWACCT_COMBO_ACCTSTATUS));
    choice->SetSelection(m_account_n->m_status.id());

    wxCheckBox* itemCheckBox = static_cast<wxCheckBox*>(FindWindow(ID_DIALOG_NEWACCT_CHKBOX_FAVACCOUNT));
    itemCheckBox->SetValue(m_account_n->is_favorite());

    wxButton* bn = static_cast<wxButton*>(FindWindow(ID_DIALOG_NEWACCT_BUTTON_CURRENCY));
    bn->SetLabelText(currency_p->m_name);

    double initBal = m_account_n->m_open_balance;
    m_initbalance_ctrl->SetCurrency(currency_p);
    m_initbalance_ctrl->SetValue(initBal);

    m_initdate_ctrl->SetValue(m_account_n->m_open_date.getDateTime());

    int selectedImage = PrefModel::instance().AccountImageId(
        m_account_n->m_id, false, true
    );
    m_bitmapButtons->SetBitmap(m_images.at(selectedImage));

    m_accessInfo = m_account_n->m_access_info;

    m_credit_limit_ctrl->SetCurrency(currency_p);
    m_credit_limit_ctrl->SetValue(m_account_n->m_credit_limit);

    m_interest_rate_ctrl->SetValue(m_account_n->m_interest_rate, 2);

    if (m_account_n->m_payment_due_date_n.has_value()) {
        m_payment_due_date_ctrl->SetValue(m_account_n->m_payment_due_date_n.getDateTimeN());
    }

    m_minimum_payment_ctrl->SetCurrency(currency_p);
    m_minimum_payment_ctrl->SetValue(m_account_n->m_min_payment);

    m_statement_lock_ctrl->SetValue(m_account_n->m_stmt_locked);

    if (m_account_n->m_stmt_date_n.has_value()) {
        m_statement_date_ctrl->SetValue(m_account_n->m_stmt_date_n.value().getDateTime());
    }
    m_minimum_balance_ctrl->SetCurrency(currency_p);
    m_minimum_balance_ctrl->SetValue(m_account_n->m_min_balance);
}

void AccountDialog::OnAccountStatus()
{
    wxChoice* choice = static_cast<wxChoice*>(
        FindWindow(ID_DIALOG_NEWACCT_COMBO_ACCTSTATUS)
    );
    wxCheckBox* itemCheckBox = static_cast<wxCheckBox*>(
        FindWindow(ID_DIALOG_NEWACCT_CHKBOX_FAVACCOUNT)
    );
    // Can only change if account is open
    if (choice->GetSelection() == AccountStatus::e_closed)
        itemCheckBox->Disable();
    else
        itemCheckBox->Enable();
}

void AccountDialog::OnAccountStatus(wxCommandEvent& /*event*/)
{
    OnAccountStatus();
}

void AccountDialog::OnCurrency(wxCommandEvent& /*event*/)
{
    if (CurrencyChoiceDialog::Execute(this, m_currencyID)) {
        const CurrencyData* currency = CurrencyModel::instance().get_id_data_n(m_currencyID);
        wxButton* bn = static_cast<wxButton*>(FindWindow(ID_DIALOG_NEWACCT_BUTTON_CURRENCY));
        bn->SetLabelText(currency->m_name);

        double value;

        m_initbalance_ctrl->SetCurrency(currency);
        if (m_initbalance_ctrl->checkValue(value, false))
            m_initbalance_ctrl->SetValue(value);

        m_credit_limit_ctrl->SetCurrency(currency);
        if (m_credit_limit_ctrl->checkValue(value, false))
            m_credit_limit_ctrl->SetValue(value);

        m_minimum_balance_ctrl->SetCurrency(currency);
        if (m_minimum_balance_ctrl->checkValue(value, false))
            m_minimum_balance_ctrl->SetValue(value);

        m_minimum_payment_ctrl->SetCurrency(currency);
        if (m_minimum_payment_ctrl->checkValue(value, false))
            m_minimum_payment_ctrl->SetValue(value);

        if (m_account_n) {
            m_account_n->m_currency_id = currency->m_id;
        }
    }
}

void AccountDialog::OnAttachments(wxCommandEvent& /*event*/)
{
    AttachmentDialog dlg(this, AccountModel::s_ref_type, m_account_n->m_id);
    dlg.ShowModal();
}


void AccountDialog::OnImageButton(wxCommandEvent& /*event*/)
{
    wxMenu mainMenu;
    wxMenuItem* menuItem = new wxMenuItem(&mainMenu, wxID_HIGHEST + static_cast<int>(acc_img::ACC_ICON_MONEY) - 1, _t("Default Image"));

    menuItem->SetBitmap(m_images.at(
        PrefModel::instance().AccountImageId(m_account_n->m_id, true)
    ));
    mainMenu.Append(menuItem);

    for (int i = img::LAST_NAVTREE_PNG; i < acc_img::MAX_ACC_ICON; ++i)
    {
        menuItem = new wxMenuItem(&mainMenu, wxID_HIGHEST + i
            , wxString::Format(_t("Image #%i"), i - img::LAST_NAVTREE_PNG + 1));
        menuItem->SetBitmap(m_images.at(i));
        mainMenu.Append(menuItem);
    }

    PopupMenu(&mainMenu);
}

void AccountDialog::OnCustonImage(wxCommandEvent& event)
{
    int selectedImage = (event.GetId() - wxID_HIGHEST) - img::LAST_NAVTREE_PNG + 1;
    int image_id = PrefModel::instance().AccountImageId(m_account_n->m_id, true);

    InfoModel::instance().setInt(
        wxString::Format("ACC_IMAGE_ID_%lld", m_account_n->m_id),
        selectedImage
    );
    if (selectedImage != 0)
        image_id = selectedImage + img::LAST_NAVTREE_PNG - 1;

    m_bitmapButtons->SetBitmap(m_images.at(image_id));
}

void AccountDialog::OnChangeFocus(wxChildFocusEvent& event)
{
    wxWindow *w = event.GetWindow();
    int oject_in_focus = 0;
    if (w) oject_in_focus = w->GetId();

    wxTextCtrl* textCtrl = static_cast<wxTextCtrl*>(
        FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_ACCESSINFO)
    );
    if (oject_in_focus == ID_DIALOG_NEWACCT_TEXTCTRL_ACCESSINFO) {
        if (!m_accessinfo_infocus) {
            textCtrl->SetValue(m_accessInfo);
            m_accessinfo_infocus = true;
        }
    }
    else {
        if (m_accessinfo_infocus) {
            m_accessInfo = textCtrl->GetValue();
            textCtrl->SetValue("********************");
            m_accessinfo_infocus = false;
        }
    }
}

void AccountDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void AccountDialog::OnOk(wxCommandEvent& /*event*/)
{
    wxString name = m_textAccountName->GetValue().Trim();
    if (name.IsEmpty() || !AccountModel::instance().find_name_data_a(name).empty()) {
        // FIXME: if (!m_account_n || m_account_n->m_name.CmpNoCase(name) != 0)
        if (m_account_n && m_account_n->m_name.CmpNoCase(name) != 0)
            return mmErrorDialogs::MessageInvalid(this, _t("Account Name "));
    }

    const CurrencyData* currency_n = CurrencyModel::instance().get_id_data_n(m_currencyID);
    if (!currency_n)
        return mmErrorDialogs::MessageInvalid(this, _t("Currency"));

    wxChoice* status_choice = static_cast<wxChoice*>(
        FindWindow(ID_DIALOG_NEWACCT_COMBO_ACCTSTATUS)
    );
    wxCheckBox* favorite_cb = static_cast<wxCheckBox*>(
        FindWindow(ID_DIALOG_NEWACCT_CHKBOX_FAVACCOUNT)
    );
    wxTextCtrl* num_ctrl = static_cast<wxTextCtrl*>(
        FindWindow(ID_ACCTNUMBER)
    );
    wxTextCtrl* held_at_ctrl = static_cast<wxTextCtrl*>(
        FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_HELDAT)
    );
    wxTextCtrl* website_ctrl = static_cast<wxTextCtrl*>(
        FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_WEBSITE)
    );
    wxTextCtrl* contact_info_ctrl = static_cast<wxTextCtrl*>(
        FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_CONTACT)
    );

    if (!website_ctrl->GetValue().empty() && !isValidURI(website_ctrl->GetValue())) {
        m_notebook->SetSelection(1);
        return mmErrorDialogs::ToolTip4Object(
            website_ctrl,
            _t("Please enter a valid URL"),
            _t("Invalid URL")
        );
    }

    mmDate open_date = mmDate(m_initdate_ctrl->GetValue());
    if (open_date > mmDate::today())
        return mmErrorDialogs::ToolTip4Object(
            m_initdate_ctrl,
            _t("Opening date shall not be in the future"),
            _t("Invalid Date")
        );
    if (m_account_n) {
        bool valid_open_date_trx1 = TrxModel::instance().find(
            TrxCol::TRANSDATE(OP_LT, open_date.isoStart()),
            TrxCol::ACCOUNTID(OP_EQ, m_account_n->m_id)
        ).empty();
        bool valid_open_date_trx2 = TrxModel::instance().find(
            TrxCol::TRANSDATE(OP_LT, open_date.isoStart()),
            TrxCol::TOACCOUNTID(OP_EQ, m_account_n->m_id)
        ).empty();
        if (!valid_open_date_trx1 || !valid_open_date_trx2)
            return mmErrorDialogs::ToolTip4Object(
                m_initdate_ctrl,
                _t("Transactions for this account already exist before this date"),
                _t("Invalid Date")
            );
        bool valid_open_date_stock = StockModel::instance().find(
            StockCol::PURCHASEDATE(OP_LT, open_date.isoStart()),
            StockCol::HELDAT(OP_EQ, m_account_n->m_id)
        ).empty();
        if (!valid_open_date_stock)
            return mmErrorDialogs::ToolTip4Object(
                m_initdate_ctrl,
                _t("Stock purchases for this account already exist before this date"),
                _t("Invalid Date")
            );
        bool valid_open_date_sched1 = SchedModel::instance().find(
            SchedCol::TRANSDATE(OP_LT, open_date.isoStart()),
            SchedCol::ACCOUNTID(OP_EQ, m_account_n->m_id)
        ).empty();
        bool valid_open_date_sched2 = SchedModel::instance().find(
            SchedCol::TRANSDATE(OP_LT, open_date.isoStart()),
            SchedCol::TOACCOUNTID(OP_EQ, m_account_n->m_id)
        ).empty();
        if (!valid_open_date_sched1 || !valid_open_date_sched2)
            return mmErrorDialogs::ToolTip4Object(
                m_initdate_ctrl,
                _t("Scheduled transactions for this account are scheduled before this date."),
                _t("Invalid Date")
            );
    }

    double open_balance = 0.0;
    if (!m_initbalance_ctrl->checkValue(open_balance, false))
        return;

    double min_balance   = 0.0; m_minimum_balance_ctrl->checkValue(min_balance);
    double credit_limit  = 0.0; m_credit_limit_ctrl->checkValue(credit_limit);
    double interest_rate = 0.0; m_interest_rate_ctrl->checkValue(interest_rate);
    double min_payment   = 0.0; m_minimum_payment_ctrl->checkValue(min_payment);

    if (!m_account_n) {
        m_account_d = AccountData();
        m_account_n = &m_account_d;
    }

    // CHECK: m_type_ is missing
    m_account_n->m_name               = name;
    m_account_n->m_currency_id        = m_currencyID;
    m_account_n->m_status             = AccountStatus(status_choice->GetSelection());
    m_account_n->m_favorite           = AccountFavorite(favorite_cb->IsChecked());
    m_account_n->m_num                = num_ctrl->GetValue();
    m_account_n->m_notes              = m_notesCtrl->GetValue();
    m_account_n->m_held_at            = held_at_ctrl->GetValue();
    m_account_n->m_website            = website_ctrl->GetValue();
    m_account_n->m_contact_info       = contact_info_ctrl->GetValue();
    m_account_n->m_access_info        = m_accessInfo;
    m_account_n->m_open_date          = open_date;
    m_account_n->m_open_balance       = open_balance;
    m_account_n->m_stmt_locked        = m_statement_lock_ctrl->GetValue();
    m_account_n->m_stmt_date_n        = mmDateN(m_statement_date_ctrl->GetValue());
    m_account_n->m_min_balance        = min_balance;
    m_account_n->m_credit_limit       = credit_limit;
    m_account_n->m_interest_rate      = interest_rate;
    m_account_n->m_payment_due_date_n = mmDateN(m_payment_due_date_ctrl->GetValue());
    m_account_n->m_min_payment        = min_payment;
    AccountModel::instance().unsafe_save_data_n(m_account_n);

    EndModal(wxID_OK);
    mmWebApp::uploadAccount();
}
