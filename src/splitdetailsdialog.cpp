/*******************************************************
Copyright (C) 2006-2012 Madhan Kanagavel
Modified by: Stefano Giorgio, Nikolay Akimov

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

#include "splitdetailsdialog.h"
#include "categdialog.h"
#include "constants.h"
#include "mmSimpleDialogs.h"
#include "validators.h"

#include "Model_Category.h"

wxIMPLEMENT_DYNAMIC_CLASS(SplitDetailDialog, wxDialog);

enum
{
    ID_DIALOG_SPLTTRANS_TYPE = wxID_HIGHEST + 1800,
    ID_BUTTONCATEGORY,
    ID_TEXTCTRLAMOUNT,
};

wxBEGIN_EVENT_TABLE(SplitDetailDialog, wxDialog)
    EVT_BUTTON( ID_BUTTONCATEGORY, SplitDetailDialog::OnButtonCategoryClick )
    EVT_BUTTON( wxID_OK, SplitDetailDialog::OnButtonOKClick )
    EVT_BUTTON( wxID_CANCEL, SplitDetailDialog::OnCancel )
    EVT_TEXT_ENTER( ID_TEXTCTRLAMOUNT, SplitDetailDialog::onTextEntered )
wxEND_EVENT_TABLE()

SplitDetailDialog::SplitDetailDialog()
{
}

SplitDetailDialog::SplitDetailDialog( 
    wxWindow* parent
    , Split &split
    , int transType
    , int accountID)
    : split_(split)
    , m_currency(Model_Currency::GetBaseCurrency())
    , m_choice_type(nullptr)
    , m_text_mount(nullptr)
    , m_bcategory(nullptr)
    , m_cancel_button(nullptr)
{
    transType_ = transType;
    Model_Account::Data *account = Model_Account::instance().get(accountID);
    if (account)
        m_currency = Model_Account::currency(account);
    Create(parent);
}

bool SplitDetailDialog::Create(wxWindow* parent)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    wxDialog::Create(parent, wxID_ANY, _("Split Detail Dialog")
        , wxDefaultPosition, wxSize(400, 300), style);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
    DataToControls();

    return TRUE;
}

void SplitDetailDialog::DataToControls()
{
    const wxString& category_name = Model_Category::full_name(split_.CATEGID
        , split_.SUBCATEGID);
    m_bcategory->SetLabelText(category_name);

    if (split_.SPLITTRANSAMOUNT)
        m_text_mount->SetValue(split_.SPLITTRANSAMOUNT, Model_Currency::precision(m_currency));

    if (category_name.empty())
    {
        m_text_mount->SetFocus();
        m_text_mount->SelectAll();
    }
    else
        m_cancel_button->SetFocus();
}

void SplitDetailDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxStaticBox* sb4 = new wxStaticBox(this
        , wxID_ANY, _("Split Transaction Details"));
    wxStaticBoxSizer* bs4 = new wxStaticBoxSizer(sb4, wxVERTICAL);
    itemBoxSizer2->Add(bs4, g_flagsExpand);

    wxPanel* itemPanel7 = new wxPanel(this, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    bs4->Add(itemPanel7, g_flagsExpand);

    wxFlexGridSizer* controlSizer = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel7->SetSizer(controlSizer);

    wxStaticText* staticTextType = new wxStaticText(itemPanel7, wxID_STATIC, _("Type"));
    controlSizer->Add(staticTextType, g_flagsH);

    const wxString itemChoiceStrings[] =
    {
        _("Withdrawal"),
        _("Deposit"),
    };
    m_choice_type = new wxChoice(itemPanel7, ID_DIALOG_SPLTTRANS_TYPE
        , wxDefaultPosition, wxDefaultSize
        , 2, itemChoiceStrings);
    m_choice_type->SetSelection(split_.SPLITTRANSAMOUNT < 0 ? !transType_ : transType_);
    m_choice_type->SetToolTip(_("Specify the type of transactions to be created."));
    controlSizer->Add(m_choice_type, g_flagsH);

    wxStaticText* staticTextAmount = new wxStaticText(itemPanel7
        , wxID_STATIC, _("Amount"));
    controlSizer->Add(staticTextAmount, g_flagsH);

    m_text_mount = new mmTextCtrl(itemPanel7, ID_TEXTCTRLAMOUNT, ""
        , wxDefaultPosition, wxSize(110, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    controlSizer->Add(m_text_mount, g_flagsH);

    wxStaticText* staticTextCategory = new wxStaticText(itemPanel7
        , wxID_STATIC, _("Category"));
    controlSizer->Add(staticTextCategory, g_flagsH);
    m_bcategory = new wxButton(itemPanel7, ID_BUTTONCATEGORY, ""
        , wxDefaultPosition, wxSize(200, -1));
    controlSizer->Add(m_bcategory, g_flagsH);

    /**************************************************************************
     Control Buttons
    ***************************************************************************/
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    itemBoxSizer2->Add(buttons_panel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 10));

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxButton* itemButtonOK = new wxButton(buttons_panel, wxID_OK, _("&OK "));
    buttons_sizer->Add(itemButtonOK, g_flagsH);

    m_cancel_button = new wxButton(buttons_panel
        , wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    buttons_sizer->Add(m_cancel_button, g_flagsH);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTONCATEGORY
 */

void SplitDetailDialog::OnButtonCategoryClick( wxCommandEvent& event )
{
    mmCategDialog dlg(this, split_.CATEGID, split_.SUBCATEGID, false);
    if (dlg.ShowModal() == wxID_OK)
    {
        split_.CATEGID = dlg.getCategId();
        split_.SUBCATEGID = dlg.getSubCategId();
        m_bcategory->SetLabelText(dlg.getFullCategName());
    }
    onTextEntered(event);
}

void SplitDetailDialog::onTextEntered(wxCommandEvent& WXUNUSED(event))
{
    if (m_text_mount->Calculate(Model_Currency::precision(m_currency)))
        m_text_mount->GetDouble(split_.SPLITTRANSAMOUNT);

    DataToControls();
}

void SplitDetailDialog::OnButtonOKClick( wxCommandEvent& event )
{
    onTextEntered(event);
    if (!m_text_mount->checkValue(split_.SPLITTRANSAMOUNT))
        return;

    if (Model_Category::full_name(split_.CATEGID, split_.SUBCATEGID).empty())
        return mmErrorDialogs::InvalidCategory(wxDynamicCast(m_bcategory, wxWindow));

    if (m_choice_type->GetSelection() != transType_)
        split_.SPLITTRANSAMOUNT = -split_.SPLITTRANSAMOUNT;

    EndModal(wxID_OK);
}

void SplitDetailDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}
