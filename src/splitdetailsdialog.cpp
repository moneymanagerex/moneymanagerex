/*******************************************************
 Copyright (C) 2006-2012 Madhan Kanagavel
 Copyright (C) 2013-2016, 2020 - 2022 Nikolay Akimov
Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)
 Modified by: Stefano Giorgio

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
#include "paths.h"

#include "model/Model_Category.h"

wxIMPLEMENT_DYNAMIC_CLASS(SplitDetailDialog, wxDialog);

enum
{
    mmID_TYPE = wxID_HIGHEST + 1800,
    mmID_AMOUNT,
};

wxBEGIN_EVENT_TABLE(SplitDetailDialog, wxDialog)
    EVT_CHILD_FOCUS(SplitDetailDialog::OnFocusChange)
    EVT_BUTTON(wxID_OK, SplitDetailDialog::OnButtonOKClick)
    EVT_BUTTON(wxID_CANCEL, SplitDetailDialog::OnCancel)
    EVT_TEXT_ENTER(mmID_AMOUNT, SplitDetailDialog::onTextEntered)
wxEND_EVENT_TABLE()

SplitDetailDialog::SplitDetailDialog()
{
}

SplitDetailDialog::SplitDetailDialog(
    wxWindow* parent
    , Split &split
    , int transType
    , Model_Currency::Data* currency)
    : split_(split)
    , transType_(transType)
    , m_currency(currency)
    , m_choice_type(nullptr)
    , m_text_amount(nullptr)
    , cbCategory_(nullptr)
    , m_cancel_button(nullptr)
    , object_in_focus_(-1)
{
    Create(parent);
}

bool SplitDetailDialog::Create(wxWindow* parent)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    wxDialog::Create(parent, wxID_ANY, _("Split Detail Dialog")
        , wxDefaultPosition, wxDefaultSize, style);

    CreateControls();
    SetIcon(mmex::getProgramIcon());
    DataToControls();

    Layout();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();

    return TRUE;
}

void SplitDetailDialog::DataToControls()
{
    const wxString& category_name = Model_Category::full_name(split_.CATEGID
        , split_.SUBCATEGID);
    cbCategory_->ChangeValue(category_name);

    if (split_.SPLITTRANSAMOUNT)
        m_text_amount->SetValue(fabs(split_.SPLITTRANSAMOUNT), Model_Currency::precision(m_currency));

    if (category_name.empty())
    {
        m_text_amount->SetFocus();
        m_text_amount->SelectAll();
    }
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
    m_choice_type = new wxChoice(itemPanel7, mmID_TYPE
        , wxDefaultPosition, wxDefaultSize
        , 2, itemChoiceStrings);
    mmToolTip(m_choice_type, _("Specify the type of transactions to be created."));
    controlSizer->Add(m_choice_type, g_flagsExpand);
    m_choice_type->SetSelection(split_.SPLITTRANSAMOUNT < 0 ? !transType_ : transType_);

    wxStaticText* staticTextAmount = new wxStaticText(itemPanel7
        , wxID_STATIC, _("Amount"));
    controlSizer->Add(staticTextAmount, g_flagsH);

    m_text_amount = new mmTextCtrl(itemPanel7, mmID_AMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    controlSizer->Add(m_text_amount, g_flagsExpand);

    wxStaticText* staticTextCategory = new wxStaticText(itemPanel7
        , wxID_STATIC, _("Category"));
    controlSizer->Add(staticTextCategory, g_flagsH);
    cbCategory_ = new mmComboBoxCategory(itemPanel7, mmID_CATEGORY);
    cbCategory_->SetMinSize(wxSize(180, -1));
    controlSizer->Add(cbCategory_, g_flagsExpand);

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

void SplitDetailDialog::onTextEntered(wxCommandEvent& WXUNUSED(event))
{
    if (m_text_amount->Calculate(Model_Currency::precision(m_currency)))
        m_text_amount->GetDouble(split_.SPLITTRANSAMOUNT);

    DataToControls();
}

void SplitDetailDialog::OnButtonOKClick(wxCommandEvent& event)
{
    if (!m_text_amount->checkValue(split_.SPLITTRANSAMOUNT)) {
        return;
    }

    if (!cbCategory_->mmIsValid()) {
        return mmErrorDialogs::ToolTip4Object(cbCategory_, _("Invalid value"), _("Category"), wxICON_ERROR);
    }
    split_.CATEGID = cbCategory_->mmGetCategoryId();
    split_.SUBCATEGID = cbCategory_->mmGetSubcategoryId();

    if (m_choice_type->GetSelection() != transType_)
        split_.SPLITTRANSAMOUNT = -split_.SPLITTRANSAMOUNT;

    EndModal(wxID_OK);
}

void SplitDetailDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void SplitDetailDialog::OnFocusChange(wxChildFocusEvent& event)
{
    switch (object_in_focus_)
    {
    case mmID_CATEGORY:
        cbCategory_->ChangeValue(cbCategory_->GetValue());
        break;
    case mmID_AMOUNT:
        if (m_text_amount->Calculate(Model_Currency::precision(log10(m_currency->SCALE)))) {
            m_text_amount->GetDouble(split_.SPLITTRANSAMOUNT);
        }
    }

    wxWindow* w = event.GetWindow();
    if (w) {
        object_in_focus_ = w->GetId();
    }

    if (object_in_focus_ == mmID_AMOUNT) {
        m_text_amount->SelectAll();
    }
}