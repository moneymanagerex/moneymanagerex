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

#include "constants.h"
#include "newacctdialog.h"
#include "maincurrencydialog.h"
#include "util.h"
#include "mmCurrencyFormatter.h"
#include "mmOption.h"
#include "paths.h"
#include <wx/valnum.h>

enum { ACCT_TYPE_CHECKING, ACCT_TYPE_INVESTMENT, ACCT_TYPE_TERM };
enum { ACCT_STATUS_OPEN, ACCT_STATUS_CLOSED };


IMPLEMENT_DYNAMIC_CLASS( mmNewAcctDialog, wxDialog )

BEGIN_EVENT_TABLE( mmNewAcctDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmNewAcctDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmNewAcctDialog::OnCancel)
    EVT_BUTTON(ID_DIALOG_NEWACCT_BUTTON_CURRENCY, mmNewAcctDialog::OnCurrency)
    EVT_MENU_RANGE(0, 99, mmNewAcctDialog::OnCustonImage)
    EVT_CHILD_FOCUS(mmNewAcctDialog::changeFocus)
END_EVENT_TABLE()

mmNewAcctDialog::mmNewAcctDialog( )
{
}

mmNewAcctDialog::mmNewAcctDialog( mmCoreDB* core, bool edit, int accountID,
                                 wxWindow* parent, wxWindowID id, const wxString& caption,
                                 const wxPoint& pos, const wxSize& size, long style )
{
    core_ = core;
    edit_ = edit;
    accountID_ = accountID;
    currencyID_ = -1;
    Create(parent, id, caption, pos, size, style);
    termAccount_ = false;
}

bool mmNewAcctDialog::Create( wxWindow* parent, wxWindowID id,
                             const wxString& caption, const wxPoint& pos, const wxSize& size,
                             long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    access_changed_ = false;
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    Centre();

    if (edit_)
    {
        fillControlsWithData();
    }

    return TRUE;
}

void mmNewAcctDialog::fillControlsWithData()
{
    this->SetTitle(_("Edit Account"));
    mmAccount* pAccount = core_->accountList_.GetAccountSharedPtr(accountID_);
    wxASSERT(pAccount);

    textAccountName_->SetValue(pAccount->name_);

    wxTextCtrl* textCtrl;
    textCtrl = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_ACCTNUMBER);
    textCtrl->SetValue(pAccount->accountNum_);

    textCtrl = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_HELDAT);
    textCtrl->SetValue(pAccount->heldAt_);

    textCtrl = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_WEBSITE);
    textCtrl->SetValue(pAccount->website_);

    textCtrl = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_CONTACT);
    textCtrl->SetValue(pAccount->contactInfo_);

    textCtrl = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_NOTES);
    textCtrl->SetValue(pAccount->notes_);

    wxChoice* itemAcctType = (wxChoice*)FindWindow(ID_DIALOG_NEWACCT_COMBO_ACCTTYPE);
    if (pAccount->acctType_ == ACCOUNT_TYPE_BANK)
       itemAcctType->SetSelection(ACCT_TYPE_CHECKING);
    else if (pAccount->acctType_ == ACCOUNT_TYPE_TERM)
       itemAcctType->SetSelection(ACCT_TYPE_TERM);
    else
       itemAcctType->SetSelection(ACCT_TYPE_INVESTMENT);
    itemAcctType->Enable(false);

    wxChoice* choice = (wxChoice*)FindWindow(ID_DIALOG_NEWACCT_COMBO_ACCTSTATUS);
    choice->SetSelection(ACCT_STATUS_OPEN);
    if (pAccount->status_ == mmAccount::MMEX_Closed)
       choice->SetSelection(ACCT_STATUS_CLOSED);

    wxCheckBox* itemCheckBox = (wxCheckBox*)FindWindow(ID_DIALOG_NEWACCT_CHKBOX_FAVACCOUNT);
    itemCheckBox->SetValue(pAccount->favoriteAcct_);

    textCtrl = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_INITBALANCE);
    double initBal = pAccount->initialBalance_;

    mmCurrency* pCurrency = pAccount->currency_;
    wxASSERT(pCurrency);

    wxButton* bn = (wxButton*)FindWindow(ID_DIALOG_NEWACCT_BUTTON_CURRENCY);
    bn->SetLabel(pCurrency->currencyName_);
    currencyID_ = pCurrency->currencyID_;

    mmDBWrapper::loadCurrencySettings(core_->db_.get(), currencyID_);
    textCtrl->SetValue(CurrencyFormatter::float2String(initBal));

    int selectedImage = mmIniOptions::instance().account_image_id(core_, accountID_);
    bitmaps_button_->SetBitmapLabel(navtree_images_list_()->GetBitmap(selectedImage));

    accessInfo_ = pAccount->accessInfo_;
}

void mmNewAcctDialog::CreateControls()
{
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Expand().Proportion(1);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, flagsExpand);

    wxFlexGridSizer* grid_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    grid_sizer->AddGrowableCol(1, 1);
    itemBoxSizer3->Add(grid_sizer, flagsExpand);

    grid_sizer->Add(new wxStaticText( this, wxID_STATIC, _("Account Name:")), flags);

    textAccountName_ = new wxTextCtrl( this, wxID_ANY, "");
    grid_sizer->Add(textAccountName_, flagsExpand);

    grid_sizer->Add(new wxStaticText( this, wxID_STATIC, _("Account Type:")), flags);

    wxArrayString itemAcctTypeStrings;
       itemAcctTypeStrings.Add(_("Checking"));   // ACCOUNT_TYPE_BANK
       itemAcctTypeStrings.Add(_("Investment")); // ACCOUNT_TYPE_STOCK
       itemAcctTypeStrings.Add(_("Term"));       // ACCOUNT_TYPE_TERM

    wxChoice* itemChoice61 = new wxChoice( this, ID_DIALOG_NEWACCT_COMBO_ACCTTYPE,
        wxDefaultPosition, wxDefaultSize, itemAcctTypeStrings);
    grid_sizer->Add(itemChoice61, flagsExpand);
    itemChoice61->SetSelection(ACCT_TYPE_CHECKING);

    grid_sizer->Add(new wxStaticText( this, wxID_STATIC, _("Account Status:")), flags);

    wxArrayString itemChoice6Strings;
    itemChoice6Strings.Add(_("Open"));
    itemChoice6Strings.Add(_("Closed"));

    wxChoice* itemChoice6 = new wxChoice( this,
        ID_DIALOG_NEWACCT_COMBO_ACCTSTATUS, wxDefaultPosition, wxDefaultSize, itemChoice6Strings);
    grid_sizer->Add(itemChoice6, flagsExpand);
    itemChoice6->SetSelection(ACCT_STATUS_OPEN);

    grid_sizer->Add(new wxStaticText( this, wxID_STATIC
        , wxString::Format(_("Initial Balance: %s"),"")), flags);

    wxTextCtrl* itemTextCtrl19 = new wxTextCtrl( this,
        ID_DIALOG_NEWACCT_TEXTCTRL_INITBALANCE,
        "", wxDefaultPosition, wxDefaultSize, 0, wxFloatingPointValidator<double>());
    grid_sizer->Add(itemTextCtrl19, flagsExpand);

    grid_sizer->Add(new wxStaticText( this, wxID_STATIC, _("Currency:")), flags);

    currencyID_ = core_->currencyList_.GetBaseCurrencySettings();
    wxString currName = _("Select Currency");
    if (currencyID_ != -1)
    {
       currName = core_->currencyList_.getCurrencyName(currencyID_);
    }

    wxButton* itemButton71 = new wxButton( this,
        ID_DIALOG_NEWACCT_BUTTON_CURRENCY, currName );
    grid_sizer->Add(itemButton71, flagsExpand);

    wxCheckBox* itemCheckBox10 = new wxCheckBox( this,
        ID_DIALOG_NEWACCT_CHKBOX_FAVACCOUNT, _("Favorite Account"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemCheckBox10->SetValue(TRUE);
    grid_sizer->AddSpacer(1);
    grid_sizer->Add(itemCheckBox10, flags);

    // Notes  ---------------------------------------------

    wxNotebook* acc_notebook = new wxNotebook(this,
        wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE );
    wxPanel* notes_tab = new wxPanel(acc_notebook, wxID_ANY);
    acc_notebook->AddPage(notes_tab, _("Notes"));
    wxBoxSizer *notes_sizer = new wxBoxSizer(wxVERTICAL);
    notes_tab->SetSizer(notes_sizer);

    notesCtrl_ = new wxTextCtrl(notes_tab, ID_DIALOG_NEWACCT_TEXTCTRL_NOTES, "",
        wxDefaultPosition, wxSize(270,180), wxTE_MULTILINE);
    notes_sizer->Add(notesCtrl_, flagsExpand);
    //

    wxPanel* others_tab = new wxPanel(acc_notebook, wxID_ANY);
    acc_notebook->AddPage(others_tab, _("Others"));
    wxBoxSizer *others_sizer = new wxBoxSizer(wxVERTICAL);
    others_tab->SetSizer(others_sizer);

    wxFlexGridSizer* grid_sizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    grid_sizer2->AddGrowableCol(1, 1);
    others_sizer->Add(grid_sizer2, flagsExpand);

    grid_sizer2->Add(new wxStaticText(others_tab, wxID_STATIC,
        _("Account Number:")), flags);

    wxTextCtrl* itemTextCtrl6 = new wxTextCtrl(others_tab,
        ID_DIALOG_NEWACCT_TEXTCTRL_ACCTNUMBER, "", wxDefaultPosition, wxDefaultSize);
    grid_sizer2->Add(itemTextCtrl6, flagsExpand);

    grid_sizer2->Add(new wxStaticText(others_tab, wxID_STATIC, _("Held At:")), flags);

    wxTextCtrl* itemTextCtrl8 = new wxTextCtrl(others_tab,
        ID_DIALOG_NEWACCT_TEXTCTRL_HELDAT, "" );
    grid_sizer2->Add(itemTextCtrl8, flagsExpand);

    grid_sizer2->Add(new wxStaticText(others_tab, wxID_STATIC, _("Website:")), flags);
    wxTextCtrl* itemTextCtrl10 = new wxTextCtrl(others_tab,
        ID_DIALOG_NEWACCT_TEXTCTRL_WEBSITE, "" );
    grid_sizer2->Add(itemTextCtrl10, flagsExpand);

    grid_sizer2->Add(new wxStaticText(others_tab, wxID_STATIC, _("Contact:")),
        flags);
    wxTextCtrl* itemTextCtrl12 = new wxTextCtrl(others_tab,
        ID_DIALOG_NEWACCT_TEXTCTRL_CONTACT, "" );
    grid_sizer2->Add(itemTextCtrl12, flagsExpand);

    grid_sizer2->Add(new wxStaticText(others_tab, wxID_STATIC, _("Access Info:")),
         flags);
    wxTextCtrl* itemTextCtrl14 = new wxTextCtrl(others_tab,
        ID_DIALOG_NEWACCT_TEXTCTRL_ACCESSINFO, "*********");
    grid_sizer2->Add(itemTextCtrl14, flagsExpand);

    itemBoxSizer3->Add(acc_notebook);

    //Buttons
    wxPanel* itemPanel27 = new wxPanel( this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer3->Add(itemPanel27, flags);

    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel27->SetSizer(itemBoxSizer28);

    bitmaps_button_ = new wxBitmapButton( itemPanel27,
        wxID_STATIC, wxNullBitmap, wxDefaultPosition,
        wxSize(textAccountName_->GetSize().GetHeight(), textAccountName_->GetSize().GetHeight()));
    itemBoxSizer28->Add(bitmaps_button_, flags);
    bitmaps_button_->Connect(wxID_STATIC, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(mmNewAcctDialog::OnImageButton), NULL, this);
    itemBoxSizer28->AddSpacer(20);

    wxButton* itemButton29 = new wxButton( itemPanel27, wxID_OK);
    itemBoxSizer28->Add(itemButton29, flags);

    wxButton* itemButton30 = new wxButton( itemPanel27, wxID_CANCEL);
    itemBoxSizer28->Add(itemButton30, flags);

    Fit();

    if (true == true)
    {
        textAccountName_->SetToolTip(_("Enter the Name of the Account. This name can be renamed at any time."));
        itemChoice61->SetToolTip(_("Specify the type of account to be created."));
        itemChoice6->SetToolTip(_("Specify if this account has been closed. Closed accounts are inactive in most calculations, reporting etc."));
        itemTextCtrl19->SetToolTip(_("Enter the initial balance in this account."));
        itemButton71->SetToolTip(_("Specify the currency to be used by this account."));
        itemCheckBox10->SetToolTip(_("Select whether this is an account that is used often. This is used to filter accounts display view."));
        notesCtrl_->SetToolTip(_("Enter user notes and details about this account."));
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
    //mmMainCurrencyDialog dlg(core_, this);
    //if ( dlg.ShowModal() == wxID_OK )
    if (mmMainCurrencyDialog::Execute(core_, this,currencyID_))
    {
        //currencyID_ = dlg.currencyID_;
        wxString currName = core_->currencyList_.getCurrencyName(currencyID_);
        wxButton* bn = (wxButton*)FindWindow(ID_DIALOG_NEWACCT_BUTTON_CURRENCY);
        bn->SetLabel(currName);
    }
}

bool mmNewAcctDialog::termAccountActivated()
{
    return termAccount_;
}

void mmNewAcctDialog::OnOk(wxCommandEvent& /*event*/)
{
    wxString acctName = textAccountName_->GetValue().Trim();
    if (acctName.IsEmpty())
    {
        mmShowErrorMessageInvalid(this, _("Account Name "));
        return;
    }

    int checkAcctID = core_->accountList_.GetAccountId(acctName);
    if ((checkAcctID != -1) && (checkAcctID != accountID_))
    {
        mmShowErrorMessage(this, _("Account Name already exists"), _("Error"));
        return;
    }

    if (currencyID_ == -1)
    {
        mmShowErrorMessageInvalid(this, _("Currency"));
        return;
    }

    wxChoice* itemAcctType = (wxChoice*)FindWindow(ID_DIALOG_NEWACCT_COMBO_ACCTTYPE);
    int acctType = itemAcctType->GetSelection();

    mmAccount* pAccount;
    if (!edit_)
    {
       mmAccount* tAccount(new mmAccount());
       pAccount = tAccount;
    }
    else
    {
       pAccount = core_->accountList_.GetAccountSharedPtr(accountID_);
    }

    pAccount->acctType_ = ACCOUNT_TYPE_BANK;
    if (acctType == ACCT_TYPE_INVESTMENT)
        pAccount->acctType_ = ACCOUNT_TYPE_STOCK;
    if (acctType == ACCT_TYPE_TERM)
    {
        pAccount->acctType_ = ACCOUNT_TYPE_TERM;
        termAccount_ = true;
    }

    wxTextCtrl* textCtrlAcctNumber = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_ACCTNUMBER);
    wxTextCtrl* textCtrlHeldAt = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_HELDAT);
    wxTextCtrl* textCtrlWebsite = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_WEBSITE);
    wxTextCtrl* textCtrlContact = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_CONTACT);
    wxTextCtrl* textCtrlAccess = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_ACCESSINFO);

    wxChoice* choice = (wxChoice*)FindWindow(ID_DIALOG_NEWACCT_COMBO_ACCTSTATUS);
    int acctStatus = choice->GetSelection();
    pAccount->status_ = mmAccount::MMEX_Open;
    if (acctStatus == ACCT_STATUS_CLOSED)
        pAccount->status_ = mmAccount::MMEX_Closed;

    wxCheckBox* itemCheckBox = (wxCheckBox*)FindWindow(ID_DIALOG_NEWACCT_CHKBOX_FAVACCOUNT);
    if (itemCheckBox->IsChecked())
       pAccount->favoriteAcct_ = true;
    else
       pAccount->favoriteAcct_ = false;

    mmDBWrapper::loadCurrencySettings(core_->db_.get(), currencyID_);
    wxTextCtrl* textCtrlInit = (wxTextCtrl*)FindWindow(ID_DIALOG_NEWACCT_TEXTCTRL_INITBALANCE);
    wxString bal = textCtrlInit->GetValue().Trim();
    pAccount->initialBalance_ = 0.0;
    if (!bal.IsEmpty())
    {
        if (! CurrencyFormatter::formatCurrencyToDouble(bal, pAccount->initialBalance_))
        {
            mmShowErrorMessageInvalid(this, wxString::Format(_("Initial Balance: %s"), bal));
            return;
        }
    }

    pAccount->name_ = acctName;
    pAccount->accountNum_ = textCtrlAcctNumber->GetValue();
    pAccount->notes_ = notesCtrl_->GetValue();
    pAccount->heldAt_ = textCtrlHeldAt->GetValue();
    pAccount->website_ = textCtrlWebsite->GetValue();
    pAccount->contactInfo_ = textCtrlContact->GetValue();
    pAccount->currencyID_ = currencyID_;
    pAccount->currency_ = core_->currencyList_.getCurrencySharedPtr(currencyID_);
    if (access_changed_)
        pAccount->accessInfo_ = textCtrlAccess->GetValue();

    if (edit_)
    {
        if (core_->accountList_.UpdateAccount(pAccount) == 0)
            EndModal(wxID_OK);
    }
    else
    {
        if (core_->accountList_.AddAccount(pAccount) > 0)
            EndModal(wxID_OK);
    }

}

void mmNewAcctDialog::OnImageButton(wxCommandEvent& /*event*/)
{
    wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY) ;
    ev.SetEventObject( this );

    //Skip all images before custom images
    int k = 18;
    wxMenu* mainMenu = new wxMenu;
    mainMenu->Append(new wxMenuItem(mainMenu, 0, "-=======-"));

    wxImageList* imageList = navtree_images_list_();

    for (int i = k; i < imageList->GetImageCount(); ++i)
    {
        wxMenuItem* menuItem = new wxMenuItem(mainMenu, i, wxString(_("Image #")) << i - k +1);
        menuItem->SetBitmap(imageList->GetBitmap(i));
        mainMenu->Append(menuItem);
    }
    //TODO: Provide wxMenu with pictures or spin buttons
    PopupMenu(mainMenu);
}

void mmNewAcctDialog::OnCustonImage(wxCommandEvent& event)
{
    int selectedImage = event.GetId();

    core_->dbInfoSettings_->SetSetting(wxString::Format("ACC_IMAGE_ID_%d", accountID_), wxString()<<selectedImage);
    if (selectedImage == 0)
        selectedImage = mmIniOptions::instance().account_image_id(core_, accountID_);

    bitmaps_button_->SetBitmapLabel(navtree_images_list_()->GetBitmap(selectedImage));

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
        textCtrl->SetValue(accessInfo_);
        access_changed_=true;
    }
    if (!edit_ && notesCtrl_->GetValue() == notesLabel_)
    {
        notesCtrl_->SetValue("");
        notesCtrl_->SetForegroundColour(notesColour_);
    }
}
