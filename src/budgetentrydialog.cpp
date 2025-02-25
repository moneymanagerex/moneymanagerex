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

#include "budgetentrydialog.h"
#include "model/Model_Category.h"
#include "validators.h"
#include "paths.h"
#include <constants.h>
#include "util.h"
#include <wx/valnum.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmBudgetEntryDialog, wxDialog);

wxBEGIN_EVENT_TABLE( mmBudgetEntryDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmBudgetEntryDialog::OnOk)
    EVT_TEXT_ENTER(wxID_ANY, mmBudgetEntryDialog::OnOk)
wxEND_EVENT_TABLE()

mmBudgetEntryDialog::mmBudgetEntryDialog()
{
}

mmBudgetEntryDialog::mmBudgetEntryDialog(wxWindow* parent
    , Model_Budget::Data* entry
    , const wxString& categoryEstimate
    , const wxString& CategoryActual)
    : catEstimateAmountStr_(categoryEstimate)
    , catActualAmountStr_(CategoryActual)
{
    budgetEntry_ = entry;
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _t("Budget Year Entry"), wxDefaultPosition, wxSize(500, 300), style);
}

bool mmBudgetEntryDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption, const wxPoint& pos
    , const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    fillControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmex::getProgramIcon());
    Centre();
    return true;
}

void mmBudgetEntryDialog::fillControls()
{
    double amt = budgetEntry_->AMOUNT;
    int period = Model_Budget::period_id(budgetEntry_);
    m_choiceItem->SetSelection(period);
    if (period == Model_Budget::PERIOD_ID_NONE && amt == 0.0)
        m_choiceItem->SetSelection(DEF_FREQ_MONTHLY);

    if (amt <= 0.0)
        m_choiceType->SetSelection(DEF_TYPE_EXPENSE);
    else
        m_choiceType->SetSelection(DEF_TYPE_INCOME);

    m_textAmount->SetValue(std::fabs(amt));
    m_Notes->SetValue(budgetEntry_->NOTES);
}

void mmBudgetEntryDialog::CreateControls()
{    
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, ""); //  ,_t("Budget Entry Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxTOP | wxRIGHT, 10);

    wxPanel* itemPanel7 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemStaticBoxSizer4->Add(itemPanel7, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel7->SetSizer(itemGridSizer2);
    
    const Model_Category::Data* category = Model_Category::instance().get(budgetEntry_->CATEGID);
    wxASSERT(category);
    
    wxStaticText* itemTextEstCatAmt = new wxStaticText(itemPanel7, wxID_STATIC, catEstimateAmountStr_);
    wxStaticText* itemTextActCatAmt = new wxStaticText(itemPanel7, wxID_STATIC, catActualAmountStr_);
    
    itemGridSizer2->Add(new wxStaticText(itemPanel7, wxID_STATIC, _t("Category: ")), g_flagsH);
    wxString categname = Model_Category::full_name(category);
    wxStaticText* categNameLabel = new wxStaticText(itemPanel7, wxID_STATIC,
        (categname.size() > 50 ? wxString::FromUTF8("\u2026") + categname.substr(categname.size() - 50) : categname));
    if (categname.size() > 50) categNameLabel->SetToolTip(categname);
    itemGridSizer2->Add(categNameLabel, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));
    itemGridSizer2->Add(new wxStaticText(itemPanel7, wxID_STATIC, _t("Estimated:")), g_flagsH);
    itemGridSizer2->Add(itemTextEstCatAmt, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));
    itemGridSizer2->Add(new wxStaticText(itemPanel7, wxID_STATIC, _t("Actual:")), g_flagsH);
    itemGridSizer2->Add(itemTextActCatAmt, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));

    itemGridSizer2->Add(new wxStaticText(itemPanel7, wxID_STATIC, _t("Type:")), g_flagsH);

    wxArrayString itemTypeStrings;  
    itemTypeStrings.Add(_t("Expense"));
    itemTypeStrings.Add(_t("Income"));

    m_choiceType = new wxChoice(itemPanel7, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, itemTypeStrings);
    itemGridSizer2->Add(m_choiceType, g_flagsExpand);
    mmToolTip(m_choiceType, _t("Specify whether this category is an income or an expense category"));

    itemGridSizer2->Add(new wxStaticText(itemPanel7, wxID_STATIC, _t("Frequency:")), g_flagsH);

    wxArrayString period;
    for (int i = 0; i < Model_Budget::PERIOD_ID_size; ++i) {
        period.Add(wxGetTranslation(Model_Budget::period_name(i)));
    }
    m_choiceItem = new wxChoice(
        itemPanel7, wxID_ANY,
        wxDefaultPosition, wxDefaultSize,
        period
    );
    itemGridSizer2->Add(m_choiceItem, g_flagsExpand);
    mmToolTip(m_choiceItem, _t("Specify the income or expense frequency"));
    m_choiceItem->Connect(wxID_ANY, wxEVT_CHAR, wxKeyEventHandler(mmBudgetEntryDialog::onChoiceChar), nullptr, this);

    itemGridSizer2->Add(new wxStaticText(itemPanel7, wxID_STATIC, _t("Amount:")), g_flagsH);

    m_textAmount = new mmTextCtrl(itemPanel7, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemGridSizer2->Add(m_textAmount, g_flagsExpand);
    mmToolTip(m_textAmount, _t("Enter the amount budgeted for this category."));
    m_textAmount->SetFocus();

    itemStaticBoxSizer4->Add(new wxStaticText(this, wxID_STATIC, _t("Notes")),0, wxGROW|wxALL, 5);
    m_Notes = new wxTextCtrl(this, wxID_ANY, ""
        , wxDefaultPosition, wxSize(-1, m_textAmount->GetSize().GetHeight() * 5), wxTE_MULTILINE);
    itemStaticBoxSizer4->Add(m_Notes,0, wxGROW|wxALL, 5);
    mmToolTip(m_Notes, _t("Enter notes to describe this budget entry"));
    
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, wxSizerFlags(g_flagsV).Center());
   
    wxButton* itemButton7 = new wxButton(this, wxID_OK, _t("&OK "));
    itemBoxSizer9->Add(itemButton7, g_flagsH);

    wxButton* itemButton8 = new wxButton(this, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    itemBoxSizer9->Add(itemButton8, g_flagsH);
}


void mmBudgetEntryDialog::OnOk(wxCommandEvent& event)
{
    int typeSelection = m_choiceType->GetSelection();    
    int period = m_choiceItem->GetSelection();
    double amt = 0.0;

    if (!m_textAmount->checkValue(amt))
        return;

    if (period == Model_Budget::PERIOD_ID_NONE && amt > 0) {
        m_choiceItem->SetFocus();
        m_choiceItem->SetSelection(DEF_FREQ_MONTHLY);
        event.Skip();
        return;
    }
    
    if (amt == 0.0)
        period = Model_Budget::PERIOD_ID_NONE;

    if (typeSelection == DEF_TYPE_EXPENSE)
        amt = -amt;

    budgetEntry_->PERIOD = Model_Budget::period_name(period);
    budgetEntry_->AMOUNT = amt;
    budgetEntry_->NOTES = m_Notes->GetValue();
    Model_Budget::instance().save(budgetEntry_);

    EndModal(wxID_OK);
}

void mmBudgetEntryDialog::onChoiceChar(wxKeyEvent& event) {

    int i = m_choiceItem->GetSelection();
    if (event.GetKeyCode()==WXK_DOWN) 
    {
        if (i < DEF_FREQ_DAILY ) 
            m_choiceItem->SetSelection(++i);
    } 
    else if (event.GetKeyCode()==WXK_UP)
    {
        if (i > DEF_FREQ_NONE)
            m_choiceItem->SetSelection(--i);
    } 
    else 
        event.Skip();

}
