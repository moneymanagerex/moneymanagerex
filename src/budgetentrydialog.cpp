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
#include <wx/valnum.h>

IMPLEMENT_DYNAMIC_CLASS( mmBudgetEntryDialog, wxDialog )

BEGIN_EVENT_TABLE( mmBudgetEntryDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmBudgetEntryDialog::OnOk)
    EVT_TEXT_ENTER(wxID_ANY, mmBudgetEntryDialog::OnOk)
END_EVENT_TABLE()

mmBudgetEntryDialog::mmBudgetEntryDialog()
{
}

mmBudgetEntryDialog::mmBudgetEntryDialog(wxWindow* parent
    , Model_Budget::Data* entry
    , const wxString& categoryEstimate
    , const wxString& CategoryActual)
    : catEstimateAmountStr_(categoryEstimate)
    , catActualAmountStr_(CategoryActual)
    , m_choiceItem()
    , m_textAmount()
    , m_choiceType()
{
    budgetEntry_ = entry;
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("Budget Year Entry"), wxDefaultPosition, wxSize(500, 300), style);
}

bool mmBudgetEntryDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption, const wxPoint& pos
    , const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());
    
    fillControls();

    Centre();
    return TRUE;
}

void mmBudgetEntryDialog::fillControls()
{
    m_choiceItem->SetSelection(Model_Budget::period(budgetEntry_));

    double amt = budgetEntry_->AMOUNT;
    if (amt < 0.0)
    {
        m_choiceType->SetSelection(DEF_TYPE_EXPENSE);
        amt = -amt;
    }
    else
        m_choiceType->SetSelection(DEF_TYPE_INCOME);
    
    m_textAmount->SetValue(amt);
}

void mmBudgetEntryDialog::CreateControls()
{    
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, ""); //  ,_("Budget Entry Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxTOP | wxRIGHT, 10);

    wxPanel* itemPanel7 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemStaticBoxSizer4->Add(itemPanel7, 0, wxGROW|wxALL, 10);

    wxFlexGridSizer* itemGridSizer2 = new wxFlexGridSizer(0, 2, 10, 10);
    itemPanel7->SetSizer(itemGridSizer2);
    
    const Model_Category::Data* category = Model_Category::instance().get(budgetEntry_->CATEGID);
    const Model_Subcategory::Data* sub_category = (budgetEntry_->SUBCATEGID != -1 ? Model_Subcategory::instance().get(budgetEntry_->SUBCATEGID) : 0);
    wxStaticText* itemTextCatTag = new wxStaticText( itemPanel7, wxID_STATIC, _("Category: "));
    wxStaticText* itemTextCatName = new wxStaticText( itemPanel7, wxID_STATIC, 
        category->CATEGNAME, wxDefaultPosition, wxDefaultSize, 0 );

    wxStaticText* itemTextEstCatTag = new wxStaticText( itemPanel7, wxID_STATIC, _("Estimated:"));
    wxStaticText* itemTextEstCatAmt = new wxStaticText( itemPanel7, wxID_STATIC, catEstimateAmountStr_);
    wxStaticText* itemTextActCatTag = new wxStaticText( itemPanel7, wxID_STATIC, _("Actual:"));
    wxStaticText* itemTextActCatAmt = new wxStaticText( itemPanel7, wxID_STATIC, catActualAmountStr_);
    
    itemGridSizer2->Add(itemTextCatTag, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL | wxADJUST_MINSIZE, 0);
    itemGridSizer2->Add(itemTextCatName, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL | wxADJUST_MINSIZE, 0);
    // only add the subcategory if it exists.
    if (budgetEntry_->SUBCATEGID >= 0) {
        wxStaticText* itemTextSubCatTag = new wxStaticText( itemPanel7, wxID_STATIC
            , _("Sub Category: "));
        wxStaticText* itemTextSubCatName = new wxStaticText( itemPanel7, wxID_STATIC, sub_category->SUBCATEGNAME);
        
        itemGridSizer2->Add(itemTextSubCatTag, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL | wxADJUST_MINSIZE, 0);
        itemGridSizer2->Add(itemTextSubCatName, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL | wxADJUST_MINSIZE, 0);
    }
    itemGridSizer2->Add(itemTextEstCatTag, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL | wxADJUST_MINSIZE, 0);
    itemGridSizer2->Add(itemTextEstCatAmt, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL | wxADJUST_MINSIZE, 0);
    itemGridSizer2->Add(itemTextActCatTag, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL | wxADJUST_MINSIZE, 0);
    itemGridSizer2->Add(itemTextActCatAmt, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL | wxADJUST_MINSIZE, 0);

    itemGridSizer2->Add(new wxStaticText(itemPanel7, wxID_STATIC, _("Type:")));

    wxArrayString itemTypeStrings;  
    itemTypeStrings.Add(_("Expense"));
    itemTypeStrings.Add(_("Income"));

    m_choiceType = new wxChoice(itemPanel7, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, itemTypeStrings);
    itemGridSizer2->Add(m_choiceType, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
    m_choiceType->SetSelection(DEF_TYPE_EXPENSE);
    m_choiceType->SetToolTip(_("Specify whether this category is an income or an expense category"));

    itemGridSizer2->Add(new wxStaticText(itemPanel7, wxID_STATIC, _("Frequency:")));

    m_choiceItem = new wxChoice(itemPanel7, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, Model_Budget::all_period());
    itemGridSizer2->Add(m_choiceItem, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
    m_choiceItem->SetSelection(DEF_FREQ_MONTHLY);
    m_choiceItem->SetToolTip(_("Specify the frequency of the expense or deposit"));
    m_choiceItem->Connect(wxID_ANY, wxEVT_CHAR, wxKeyEventHandler(mmBudgetEntryDialog::onChoiceChar), NULL, this);

    itemGridSizer2->Add(new wxStaticText(itemPanel7, wxID_STATIC, _("Amount:")));

    m_textAmount = new mmTextCtrl( itemPanel7, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemGridSizer2->Add(m_textAmount);
    m_textAmount->SetToolTip(_("Enter the amount budgeted for this category."));
    m_textAmount->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmBudgetEntryDialog::OnTextEntered), NULL, this);
    m_textAmount->SetFocus();
    
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
   
    wxButton* itemButton7 = new wxButton(this, wxID_OK, _("&OK "));
    itemBoxSizer9->Add(itemButton7, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxButton* itemButton8 = new wxButton(this, wxID_CANCEL, _("&Cancel "));
    itemBoxSizer9->Add(itemButton8, 0, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL, 5);
}


void mmBudgetEntryDialog::OnOk(wxCommandEvent& event)
{
    int typeSelection = m_choiceType->GetSelection();
    
    wxString period = Model_Budget::all_period()[m_choiceItem->GetSelection()];

    wxString displayAmtString = m_textAmount->GetValue().Trim(); //TODO: simplification needed
    double amt = 0.0;
    if (!Model_Currency::fromString(displayAmtString, amt, Model_Currency::GetBaseCurrency()) || amt < 0)
    {
        wxMessageDialog msgDlg(this, _("Invalid Amount Entered "), _("Error"), wxOK | wxICON_ERROR);
        msgDlg.ShowModal();
        return;
    }

    if (period == "None" && amt > 0) {
        m_choiceItem->SetFocus();
        m_choiceItem->SetSelection(DEF_FREQ_MONTHLY);
        event.Skip();
        return;
    }
    
    if (amt == 0.0)
        period = "None";

    if (typeSelection == DEF_TYPE_EXPENSE)
        amt = -amt;

    budgetEntry_->PERIOD = period;
    budgetEntry_->AMOUNT = amt;
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

void mmBudgetEntryDialog::OnTextEntered(wxCommandEvent& event)
{
    if (event.GetId() == m_textAmount->GetId())
    {
        m_textAmount->Calculate(Model_Currency::GetBaseCurrency());
    }
}
