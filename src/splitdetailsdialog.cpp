/*******************************************************
Copyright (C) 2006-2012 Madhan Kanagavel
                        Modified by: Stefano Giorgio, Nikolay

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
#include "util.h"
#include "validators.h"

#include "model/Model_Category.h"
#include "model/Model_Checking.h"
#include "model/Model_Subcategory.h"

#include <wx/valnum.h>

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
    , accountID_(accountID)
    , m_currency(Model_Currency::GetBaseCurrency())
{
    transType_ = transType;
    Model_Account::Data *account = Model_Account::instance().get(accountID_);
    if (account)
        m_currency = Model_Account::currency(account);
    Create(parent);
}

bool SplitDetailDialog::Create(wxWindow* parent)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    wxDialog::Create(parent, wxID_ANY, _("Split Detail Dialog"), wxDefaultPosition, wxSize(400, 300), style);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
    DataToControls();

    return TRUE;
}

void SplitDetailDialog::DataToControls()
{
    const wxString& category_name = Model_Category::full_name(split_.CATEGID, split_.SUBCATEGID);
    bCategory_->SetLabelText(category_name);

    if (split_.SPLITTRANSAMOUNT)
        textAmount_->SetValue(std::fabs(split_.SPLITTRANSAMOUNT));

    if (category_name.empty())
    {
        textAmount_->SetFocus();
        textAmount_->SelectAll();
    }
}

void SplitDetailDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, _("Split Transaction Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer4, g_flagsExpand);

    wxPanel* itemPanel7 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemStaticBoxSizer4->Add(itemPanel7, g_flagsExpand);

    wxFlexGridSizer* controlSizer = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel7->SetSizer(controlSizer);

    wxStaticText* staticTextType = new wxStaticText(itemPanel7, wxID_STATIC, _("Type"));
    controlSizer->Add(staticTextType, g_flagsH);

    const wxString itemChoiceStrings[] =
    {
        _("Withdrawal"),
        _("Deposit"),
    };
    choiceType_ = new wxChoice(itemPanel7, ID_DIALOG_SPLTTRANS_TYPE
        , wxDefaultPosition, wxDefaultSize
        , 2, itemChoiceStrings);
    choiceType_->SetSelection(split_.SPLITTRANSAMOUNT < 0 ? !transType_ : transType_);
    choiceType_->SetToolTip(_("Specify the type of transactions to be created."));
    controlSizer->Add(choiceType_, g_flagsH);

    wxStaticText* staticTextAmount = new wxStaticText(itemPanel7, wxID_STATIC, _("Amount"));
    controlSizer->Add(staticTextAmount, g_flagsH);

    textAmount_ = new mmTextCtrl(itemPanel7, ID_TEXTCTRLAMOUNT, ""
        , wxDefaultPosition, wxSize(110, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    controlSizer->Add(textAmount_, g_flagsH);

    wxStaticText* staticTextCategory = new wxStaticText(itemPanel7, wxID_STATIC, _("Category"));
    controlSizer->Add(staticTextCategory, g_flagsH);
    bCategory_ = new wxButton(itemPanel7, ID_BUTTONCATEGORY, ""
        , wxDefaultPosition, wxSize(200, -1));
    controlSizer->Add(bCategory_, g_flagsH);

    /**************************************************************************
     Control Buttons
    ***************************************************************************/
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    itemBoxSizer2->Add(buttons_panel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 10));

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxButton* itemButtonOK = new wxButton(buttons_panel, wxID_OK, _("&OK "));
    buttons_sizer->Add(itemButtonOK, g_flagsH);

    wxButton* itemButtonCancel = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    buttons_sizer->Add(itemButtonCancel, g_flagsH);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTONCATEGORY
 */

void SplitDetailDialog::OnButtonCategoryClick( wxCommandEvent& /*event*/ )
{
    mmCategDialog dlg(this);
    dlg.setTreeSelection(split_.CATEGID, split_.SUBCATEGID);
    if (dlg.ShowModal() == wxID_OK)
    {
        split_.CATEGID = dlg.getCategId();
        split_.SUBCATEGID = dlg.getSubCategId();
        bCategory_->SetLabelText(dlg.getFullCategName());
    }
    wxCommandEvent evt(wxID_ANY, wxID_ANY);
    onTextEntered(evt);
    DataToControls();
}

void SplitDetailDialog::onTextEntered(wxCommandEvent& WXUNUSED(event))
{
    if (textAmount_->Calculate())
        textAmount_->GetDouble(split_.SPLITTRANSAMOUNT);

    DataToControls();
}

void SplitDetailDialog::OnButtonOKClick( wxCommandEvent& /*event*/ )
{
    if (!textAmount_->checkValue(split_.SPLITTRANSAMOUNT))
        return;

    if (Model_Category::full_name(split_.CATEGID, split_.SUBCATEGID).empty())
        return mmErrorDialogs::InvalidCategory(wxDynamicCast(bCategory_, wxWindow));

    if (choiceType_->GetSelection() != transType_)
        split_.SPLITTRANSAMOUNT = -split_.SPLITTRANSAMOUNT;

    EndModal(wxID_OK);
}

void SplitDetailDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}
