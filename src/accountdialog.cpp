/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#include "accountdialog.h"
#include "constants.h"
#include "maincurrencydialog.h"
#include "images_list.h"
#include "util.h"
#include "mmOption.h"
#include "paths.h"
#include "validators.h"
#include "webapp.h"
#include "attachmentdialog.h"
#include <wx/valnum.h>
#include "model/Model_Infotable.h"
#include "model/Model_Account.h"
#include "model/Model_Currency.h"
#include "model/Model_Attachment.h"
#include "../resources/attachment.xpm"

IMPLEMENT_DYNAMIC_CLASS( mmNewAcctDialog, wxDialog )

BEGIN_EVENT_TABLE( mmNewAcctDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmNewAcctDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmNewAcctDialog::OnCancel)
    EVT_BUTTON(ID_DIALOG_NEWACCT_BUTTON_CURRENCY, mmNewAcctDialog::OnCurrency)
	EVT_BUTTON(wxID_FILE, mmNewAcctDialog::OnAttachments)
    EVT_MENU_RANGE(wxID_HIGHEST, wxID_HIGHEST + 99, mmNewAcctDialog::OnCustonImage)
END_EVENT_TABLE()

mmNewAcctDialog::mmNewAcctDialog( )
{
}

mmNewAcctDialog::mmNewAcctDialog(Model_Account::Data* account, wxWindow* parent)
    : m_account(account)
    , m_currencyID(-1)
    , m_termAccount(false)
    , m_textAccountName()
    , m_notesCtrl()
    , m_itemInitValue()
    , m_imageList()
    , m_bitmapButtons()
{
    m_imageList = navtree_images_list();
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("New Account"), wxDefaultPosition, wxSize(550, 300), style);

    fillControls();
    this->Connect(wxID_ANY, wxEVT_CHILD_FOCUS, wxChildFocusEventHandler(mmNewAcctDialog::changeFocus), nullptr, this);
}

mmNewAcctDialog::~mmNewAcctDialog()
{
    if (m_imageList)
        delete m_imageList;
}

bool mmNewAcctDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    m_accessChanged = false;
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());
    Centre();

    return TRUE;
}

void mmNewAcctDialog::fillControls()
{

    this->SetTitle(_("Edit Account"));

    if (!this->m_account) return;

    m_textAccountName->SetValue(m_account->ACCOUNTNAME);

    wxTextCtrl* textCtrl = (wxTextCtrl*)FindWindow(ID_ACCTNUMBER);
    textCtrl->SetValue(m_account->ACCOUNTNUM);

    textCtrl = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_HELDAT);
    textCtrl->SetValue(m_account->HELDAT);

    textCtrl = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_WEBSITE);
    textCtrl->SetValue(m_account->WEBSITE);

    textCtrl = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_CONTACT);
    textCtrl->SetValue(m_account->CONTACTINFO);

    textCtrl = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_NOTES);
    textCtrl->SetValue(m_account->NOTES);

    wxChoice* itemAcctType = (wxChoice*)FindWindow(ID_DIALOG_NEWACCT_COMBO_ACCTTYPE);
    itemAcctType->SetStringSelection(wxGetTranslation(m_account->ACCOUNTTYPE));
    itemAcctType->Enable(false);

    wxChoice* choice = (wxChoice*)FindWindow(ID_DIALOG_NEWACCT_COMBO_ACCTSTATUS);
    choice->SetSelection(Model_Account::status(m_account));

    wxCheckBox* itemCheckBox = (wxCheckBox*)FindWindow(ID_DIALOG_NEWACCT_CHKBOX_FAVACCOUNT);
    itemCheckBox->SetValue(m_account->FAVORITEACCT == "TRUE");

    Model_Account::currency(m_account);
    wxButton* bn = (wxButton*)FindWindow(ID_DIALOG_NEWACCT_BUTTON_CURRENCY);
    bn->SetLabel(Model_Account::currency(m_account)->CURRENCYNAME);
    m_currencyID = m_account->CURRENCYID;

    double initBal = m_account->INITIALBAL;
    m_itemInitValue->SetValue(Model_Currency::toString(initBal, Model_Account::currency(m_account)));

    int selectedImage = mmIniOptions::instance().account_image_id(m_account->ACCOUNTID);
    m_bitmapButtons->SetBitmapLabel(m_imageList->GetBitmap(selectedImage));

    m_accessInfo = m_account->ACCESSINFO;
}

void mmNewAcctDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, g_flagsExpand);

    wxFlexGridSizer* grid_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    grid_sizer->AddGrowableCol(1, 1);
    itemBoxSizer3->Add(grid_sizer, g_flagsExpand);

    grid_sizer->Add(new wxStaticText( this, wxID_STATIC, _("Account Name:")), g_flags);

    m_textAccountName = new wxTextCtrl( this, wxID_ANY, "");
    grid_sizer->Add(m_textAccountName, g_flagsExpand);

    grid_sizer->Add(new wxStaticText( this, wxID_STATIC, _("Account Type:")), g_flags);

    wxChoice* itemChoice61 = new wxChoice( this, ID_DIALOG_NEWACCT_COMBO_ACCTTYPE);
    for (const auto& type: Model_Account::all_type())
        itemChoice61->Append(wxGetTranslation(type), new wxStringClientData(type));

    grid_sizer->Add(itemChoice61, g_flagsExpand);
    itemChoice61->SetSelection(0);

    grid_sizer->Add(new wxStaticText( this, wxID_STATIC, _("Account Status:")), g_flags);

    wxChoice* itemChoice6 = new wxChoice( this, ID_DIALOG_NEWACCT_COMBO_ACCTSTATUS);
    for(const auto& status: Model_Account::all_status())
        itemChoice6->Append(wxGetTranslation(status), new wxStringClientData(status));
    grid_sizer->Add(itemChoice6, g_flagsExpand);
    itemChoice6->SetSelection(0);

    grid_sizer->Add(new wxStaticText( this, wxID_STATIC
        , wxString::Format(_("Initial Balance: %s"),"")), g_flags);

    m_itemInitValue = new mmTextCtrl(this
        , ID_DIALOG_NEWACCT_TEXTCTRL_INITBALANCE
        , "", wxDefaultPosition, wxDefaultSize, 0, mmCalcValidator());
    grid_sizer->Add(m_itemInitValue, g_flagsExpand);
    m_itemInitValue->Connect(ID_DIALOG_NEWACCT_TEXTCTRL_INITBALANCE, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(mmNewAcctDialog::OnTextEntered), nullptr, this);

    grid_sizer->Add(new wxStaticText( this, wxID_STATIC, _("Currency:")), g_flags);

    wxString currName = _("Select Currency");
    Model_Currency::Data* base_currency = Model_Currency::GetBaseCurrency();
    if (base_currency)
        currName = base_currency->CURRENCYNAME;

    wxButton* itemButton71 = new wxButton( this,
        ID_DIALOG_NEWACCT_BUTTON_CURRENCY, currName );
    grid_sizer->Add(itemButton71, g_flagsExpand);

    wxCheckBox* itemCheckBox10 = new wxCheckBox( this
        , ID_DIALOG_NEWACCT_CHKBOX_FAVACCOUNT
        , _("Favorite Account"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemCheckBox10->SetValue(TRUE);
    grid_sizer->AddSpacer(1);
    grid_sizer->Add(itemCheckBox10, g_flags);

    // Notes  ---------------------------------------------

    wxNotebook* acc_notebook = new wxNotebook(this
        , wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE );
    wxPanel* notes_tab = new wxPanel(acc_notebook, wxID_ANY);
    acc_notebook->AddPage(notes_tab, _("Notes"));
    wxBoxSizer *notes_sizer = new wxBoxSizer(wxVERTICAL);
    notes_tab->SetSizer(notes_sizer);

    m_notesCtrl = new wxTextCtrl(notes_tab, ID_DIALOG_NEWACCT_TEXTCTRL_NOTES, ""
        , wxDefaultPosition, wxSize(270,180), wxTE_MULTILINE);
    notes_sizer->Add(m_notesCtrl, g_flagsExpand);
    //

    wxPanel* others_tab = new wxPanel(acc_notebook, wxID_ANY);
    acc_notebook->AddPage(others_tab, _("Others"));
    wxBoxSizer *others_sizer = new wxBoxSizer(wxVERTICAL);
    others_tab->SetSizer(others_sizer);

    wxFlexGridSizer* grid_sizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    grid_sizer2->AddGrowableCol(1, 1);
    others_sizer->Add(grid_sizer2, g_flagsExpand);

    grid_sizer2->Add(new wxStaticText(others_tab, wxID_STATIC,
        _("Account Number:")), g_flags);

    wxTextCtrl* itemTextCtrl6 = new wxTextCtrl(others_tab
        , ID_ACCTNUMBER, "", wxDefaultPosition, wxDefaultSize);
    grid_sizer2->Add(itemTextCtrl6, g_flagsExpand);

    grid_sizer2->Add(new wxStaticText(others_tab, wxID_STATIC, _("Held At:")), g_flags);

    wxTextCtrl* itemTextCtrl8 = new wxTextCtrl(others_tab
        , ID_DIALOG_NEWACCT_TEXTCTRL_HELDAT, "" );
    grid_sizer2->Add(itemTextCtrl8, g_flagsExpand);

    grid_sizer2->Add(new wxStaticText(others_tab, wxID_STATIC, _("Website:")), g_flags);
    wxTextCtrl* itemTextCtrl10 = new wxTextCtrl(others_tab
        , ID_DIALOG_NEWACCT_TEXTCTRL_WEBSITE, "" );
    grid_sizer2->Add(itemTextCtrl10, g_flagsExpand);

    grid_sizer2->Add(new wxStaticText(others_tab
        , wxID_STATIC, _("Contact:")), g_flags);
    wxTextCtrl* itemTextCtrl12 = new wxTextCtrl(others_tab
        , ID_DIALOG_NEWACCT_TEXTCTRL_CONTACT, "" );
    grid_sizer2->Add(itemTextCtrl12, g_flagsExpand);

    grid_sizer2->Add(new wxStaticText(others_tab
        , wxID_STATIC, _("Access Info:")), g_flags);
    wxTextCtrl* itemTextCtrl14 = new wxTextCtrl(others_tab
        , ID_DIALOG_NEWACCT_TEXTCTRL_ACCESSINFO, "*********");
    grid_sizer2->Add(itemTextCtrl14, g_flagsExpand);

    itemBoxSizer3->Add(acc_notebook);

    //Buttons
    wxPanel* itemPanel27 = new wxPanel( this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer3->Add(itemPanel27, g_flags);

    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel27->SetSizer(itemBoxSizer28);

    m_bitmapButtons = new wxBitmapButton( itemPanel27
        , wxID_STATIC, wxNullBitmap, wxDefaultPosition
        , wxSize(m_textAccountName->GetSize().GetHeight(), m_textAccountName->GetSize().GetHeight()));
    m_bitmapButtons->Connect(wxID_STATIC, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmNewAcctDialog::OnImageButton), nullptr, this);
    itemBoxSizer28->Add(m_bitmapButtons, g_flags);

	bAttachments_ = new wxBitmapButton(itemPanel27, wxID_FILE
		, wxBitmap(attachment_xpm), wxDefaultPosition
		, wxSize(m_textAccountName->GetSize().GetHeight(), m_textAccountName->GetSize().GetHeight()));
	bAttachments_->SetToolTip(_("Organize attachments of this account"));
	itemBoxSizer28->Add(bAttachments_, g_flags);

    itemBoxSizer28->AddSpacer(20);

    wxButton* itemButton29 = new wxButton(itemPanel27, wxID_OK, _("&OK "));
    itemBoxSizer28->Add(itemButton29, g_flags);

    wxButton* itemButton30 = new wxButton(itemPanel27, wxID_CANCEL, _("&Cancel "));
    itemBoxSizer28->Add(itemButton30, g_flags);

    Fit();

    if (true)
    {
        m_textAccountName->SetToolTip(_("Enter the Name of the Account. This name can be renamed at any time."));
        itemChoice61->SetToolTip(_("Specify the type of account to be created."));
        itemChoice6->SetToolTip(_("Specify if this account has been closed. Closed accounts are inactive in most calculations, reporting etc."));
        m_itemInitValue->SetToolTip(_("Enter the initial balance in this account."));
        itemButton71->SetToolTip(_("Specify the currency to be used by this account."));
        itemCheckBox10->SetToolTip(_("Select whether this is an account that is used often. This is used to filter accounts display view."));
        m_notesCtrl->SetToolTip(_("Enter user notes and details about this account."));
        itemTextCtrl6->SetToolTip(_("Enter the Account Number associated with this account."));
        itemTextCtrl8->SetToolTip(_("Enter the name of the financial institution in which the account is held."));
        itemTextCtrl10->SetToolTip(_("Enter the URL of the website for the financial institution."));
        itemTextCtrl12->SetToolTip(_("Enter any contact information for the financial institution."));
        itemTextCtrl14->SetToolTip(_("Enter any login/access information for the financial institution. This is not secure as anyone with access to the mmb file can access it."));
    }
}

void mmNewAcctDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmNewAcctDialog::OnCurrency(wxCommandEvent& /*event*/)
{
    if (mmMainCurrencyDialog::Execute(this, m_currencyID))
    {
        Model_Currency::Data* currency = Model_Currency::instance().get(m_currencyID);
        wxButton* bn = (wxButton*)FindWindow(ID_DIALOG_NEWACCT_BUTTON_CURRENCY);
        bn->SetLabel(currency->CURRENCYNAME);

        if (this->m_account) m_account->CURRENCYID = currency->CURRENCYID;
    }
}

void mmNewAcctDialog::OnAttachments(wxCommandEvent& /*event*/)
{
	wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::BANKACCOUNT);
	mmAttachmentDialog dlg(this, RefType, m_account->ACCOUNTID);
	dlg.ShowModal();
}

bool mmNewAcctDialog::termAccountActivated()
{
    return m_termAccount;
}

void mmNewAcctDialog::OnOk(wxCommandEvent& /*event*/)
{
    wxString acctName = m_textAccountName->GetValue().Trim();
    if (acctName.IsEmpty())
    {
        mmShowErrorMessageInvalid(this, _("Account Name "));
        return;
    }

    if (m_currencyID == -1)
    {
        mmShowErrorMessageInvalid(this, _("Currency"));
        return;
    }

    wxChoice* itemAcctType = (wxChoice*)FindWindow(ID_DIALOG_NEWACCT_COMBO_ACCTTYPE);
    int acctType = itemAcctType->GetSelection();
    
    if (!this->m_account) this->m_account = Model_Account::instance().create();

    m_account->ACCOUNTNAME = acctName;
    m_account->ACCOUNTTYPE = Model_Account::all_type()[acctType];
    if (acctType == Model_Account::TERM)
        this->m_termAccount = true;

    wxTextCtrl* textCtrlAcctNumber = (wxTextCtrl*)FindWindow(ID_ACCTNUMBER);
    wxTextCtrl* textCtrlHeldAt = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_HELDAT);
    wxTextCtrl* textCtrlWebsite = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_WEBSITE);
    wxTextCtrl* textCtrlContact = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_CONTACT);
    wxTextCtrl* textCtrlAccess = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_ACCESSINFO);

    wxChoice* choice = (wxChoice*)FindWindow(ID_DIALOG_NEWACCT_COMBO_ACCTSTATUS);
    m_account->STATUS = Model_Account::all_status()[choice->GetSelection()];

    wxCheckBox* itemCheckBox = (wxCheckBox*)FindWindow(ID_DIALOG_NEWACCT_CHKBOX_FAVACCOUNT);
    m_account->FAVORITEACCT = itemCheckBox->IsChecked() ? "TRUE" : "FALSE";

    wxString bal = m_itemInitValue->GetValue().Trim();
    if (!Model_Currency::fromString(bal, m_account->INITIALBAL, Model_Currency::instance().get(m_currencyID)))
        m_account->INITIALBAL = 0.0;
    
    m_account->ACCOUNTNUM = textCtrlAcctNumber->GetValue();
    m_account->NOTES = m_notesCtrl->GetValue();
    m_account->HELDAT = textCtrlHeldAt->GetValue();
    m_account->WEBSITE = textCtrlWebsite->GetValue();
    m_account->CONTACTINFO = textCtrlContact->GetValue();
    m_account->CURRENCYID = m_currencyID;
    if (m_accessChanged)
        m_account->ACCESSINFO = textCtrlAccess->GetValue();

    Model_Account::instance().save(m_account);

    EndModal(wxID_OK);
	mmWebApp::MMEX_WebApp_UpdateAccount();
}

void mmNewAcctDialog::OnImageButton(wxCommandEvent& /*event*/)
{
    wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY);
    ev.SetEventObject( this );

    //Skip all images before custom images
    int k = 18;
    wxMenu* mainMenu = new wxMenu;
    wxMenuItem* menuItem = new wxMenuItem(mainMenu, wxID_HIGHEST, _("Default Image"));
    menuItem->SetBitmap(m_imageList->GetBitmap(mmIniOptions::instance().account_image_id(this->m_account->ACCOUNTID)));
    mainMenu->Append(menuItem);

    for (int i = k; i < m_imageList->GetImageCount(); ++i)
    {
        menuItem = new wxMenuItem(mainMenu, wxID_HIGHEST + i
            , wxString::Format(_("Image #%i"), i - k + 1));
        menuItem->SetBitmap(m_imageList->GetBitmap(i));
        mainMenu->Append(menuItem);
    }

    PopupMenu(mainMenu);
    delete mainMenu;
}

void mmNewAcctDialog::OnCustonImage(wxCommandEvent& event)
{
    int selectedImage = event.GetId() - wxID_HIGHEST;

    Model_Infotable::instance().Set(wxString::Format("ACC_IMAGE_ID_%i", this->m_account->ACCOUNTID), selectedImage);
    if (selectedImage == 0)
        selectedImage = mmIniOptions::instance().account_image_id(this->m_account->ACCOUNTID);

    m_bitmapButtons->SetBitmapLabel(m_imageList->GetBitmap(selectedImage));

}

void mmNewAcctDialog::changeFocus(wxChildFocusEvent& event)
{
    wxWindow *w = event.GetWindow();
    int oject_in_focus = 0;
    if ( w )
        oject_in_focus = w->GetId();

    if (oject_in_focus == ID_DIALOG_NEWACCT_TEXTCTRL_ACCESSINFO)
    {
        wxTextCtrl* textCtrl = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_ACCESSINFO);
        textCtrl->SetValue(m_accessInfo);
        m_accessChanged = true;
    }

    if (m_notesCtrl->GetValue() == m_notesLabel)
    {
        m_notesCtrl->SetValue("");
    }
}

void mmNewAcctDialog::OnTextEntered(wxCommandEvent& event)
{
    if (event.GetId() == m_itemInitValue->GetId())
    {
        Model_Currency::Data* currency = Model_Currency::instance().get(m_currencyID);
        if (!currency)
            currency = Model_Currency::GetBaseCurrency();
        m_itemInitValue->Calculate(currency);
    }
}
