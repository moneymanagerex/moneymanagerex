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
#include "util.h"
#include "model/Model_Category.h"
#include "mmCalculator.h"
#include "validators.h"
#include "paths.h"
#include <wx/valnum.h>

enum { DEF_TYPE_EXPENSE, DEF_TYPE_INCOME };
enum { DEF_FREQ_NONE, DEF_FREQ_WEEKLY, DEF_FREQ_BIWEEKLY, DEF_FREQ_MONTHLY, DEF_FREQ_BIMONTHLY, DEF_FREQ_QUARTERLY, DEF_FREQ_HALFYEARLY, DEF_FREQ_YEARLY, DEF_FREQ_DAILY };

IMPLEMENT_DYNAMIC_CLASS( mmBudgetEntryDialog, wxDialog )

BEGIN_EVENT_TABLE( mmBudgetEntryDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmBudgetEntryDialog::OnOk)
    EVT_TEXT_ENTER(wxID_ANY, mmBudgetEntryDialog::OnOk)
END_EVENT_TABLE()

mmBudgetEntryDialog::mmBudgetEntryDialog( )
{
}

mmBudgetEntryDialog::mmBudgetEntryDialog(wxWindow* parent, Model_Budget::Data* entry,
                                         const wxString& categoryEstimate, const wxString& CategoryActual)
                                         : catEstimateAmountStr_(categoryEstimate)
                                         , catActualAmountStr_(CategoryActual)
{
    budgetEntry_ = entry;
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("Budget Year Entry"), wxDefaultPosition, wxSize(500, 300), style);
}

bool mmBudgetEntryDialog::Create( wxWindow* parent, wxWindowID id, 
                           const wxString& caption, const wxPoint& pos, 
                           const wxSize& size, long style )
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
    wxString period = budgetEntry_->PERIOD;
    if (period == "")
        period = "Monthly";

    if (period == "None")
        itemChoice_->SetSelection(DEF_FREQ_NONE);
    else if (period == "Monthly")
        itemChoice_->SetSelection(DEF_FREQ_MONTHLY);
    else if (period == "Yearly")
        itemChoice_->SetSelection(DEF_FREQ_YEARLY);
    else if (period == "Weekly")
        itemChoice_->SetSelection(DEF_FREQ_WEEKLY);
    else if (period == "Bi-Weekly")
        itemChoice_->SetSelection(DEF_FREQ_BIWEEKLY);
    else if (period == "Bi-Monthly")
        itemChoice_->SetSelection(DEF_FREQ_BIMONTHLY);
    else if (period == "Quarterly")
        itemChoice_->SetSelection(DEF_FREQ_QUARTERLY);
    else if (period == "Half-Yearly")
        itemChoice_->SetSelection(DEF_FREQ_HALFYEARLY);
    else if (period == "Daily")
        itemChoice_->SetSelection(DEF_FREQ_DAILY);
    else
        wxASSERT(false);

    wxString displayAmtString = "0";
    double amt = budgetEntry_->AMOUNT;
    if (amt < 0.0)
    {
        type_->SetSelection(DEF_TYPE_EXPENSE);
        amt = -amt;
    }
    else
        type_->SetSelection(DEF_TYPE_INCOME);
    
    textAmount_->SetValue(amt);
}

void mmBudgetEntryDialog::CreateControls()
{    
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, ""); //  ,_("Budget Entry Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxTOP|wxRIGHT, 10);

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
    
    itemGridSizer2->Add(itemTextCatTag,    0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);
    itemGridSizer2->Add(itemTextCatName,   0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);
    // only add the subcategory if it exists.
    if (budgetEntry_->SUBCATEGID >= 0) {
        wxStaticText* itemTextSubCatTag = new wxStaticText( itemPanel7, wxID_STATIC, 
            _("Sub Category: "), wxDefaultPosition, wxDefaultSize, 0 );
        wxStaticText* itemTextSubCatName = new wxStaticText( itemPanel7, wxID_STATIC, sub_category->SUBCATEGNAME);
        
        itemGridSizer2->Add(itemTextSubCatTag, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);
        itemGridSizer2->Add(itemTextSubCatName,0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);
    }
    itemGridSizer2->Add(itemTextEstCatTag,0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);
    itemGridSizer2->Add(itemTextEstCatAmt,0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);
    itemGridSizer2->Add(itemTextActCatTag,0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);
    itemGridSizer2->Add(itemTextActCatAmt,0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);

    wxStaticText* itemStaticText101 = new wxStaticText(itemPanel7, wxID_STATIC
        , _("Type:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizer2->Add(itemStaticText101, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);

    wxArrayString itemTypeStrings;  
    itemTypeStrings.Add(_("Expense"));
    itemTypeStrings.Add(_("Income"));

    type_ = new wxChoice( itemPanel7, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, itemTypeStrings);
    itemGridSizer2->Add(type_, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    type_->SetSelection(DEF_TYPE_EXPENSE);
    type_->SetToolTip(_("Specify whether this category is an income or an expense category"));

    wxStaticText* itemStaticText51 = new wxStaticText( itemPanel7, wxID_STATIC, 
        _("Frequency:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizer2->Add(itemStaticText51, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);

    wxArrayString itemFrequencyTypeStrings;  
    itemFrequencyTypeStrings.Add(_("None"));
    itemFrequencyTypeStrings.Add(_("Weekly"));
    itemFrequencyTypeStrings.Add(_("Bi-Weekly"));
    itemFrequencyTypeStrings.Add(_("Monthly"));
    itemFrequencyTypeStrings.Add(_("Bi-Monthly"));
    itemFrequencyTypeStrings.Add(_("Quarterly"));
    itemFrequencyTypeStrings.Add(_("Half-Yearly"));
    itemFrequencyTypeStrings.Add(_("Yearly"));
    itemFrequencyTypeStrings.Add(_("Daily"));

    itemChoice_ = new wxChoice( itemPanel7, wxID_ANY, 
        wxDefaultPosition, wxDefaultSize, itemFrequencyTypeStrings);
    itemGridSizer2->Add(itemChoice_, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    itemChoice_->SetSelection(DEF_FREQ_MONTHLY);
    itemChoice_->SetToolTip(_("Specify the frequency of the expense or deposit"));
    itemChoice_->Connect(wxID_ANY, wxEVT_CHAR, wxKeyEventHandler(mmBudgetEntryDialog::onChoiceChar), NULL, this);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel7, wxID_STATIC
        , _("Amount:"));
    itemGridSizer2->Add(itemStaticText3, 0, 
        wxALIGN_LEFT |wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);

    textAmount_ = new mmTextCtrl( itemPanel7, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemGridSizer2->Add(textAmount_);
    textAmount_->SetToolTip(_("Enter the amount budgeted for this category."));
    textAmount_->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(mmBudgetEntryDialog::OnTextEntered), NULL, this);
    textAmount_->SetFocus();
    
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
   
    wxButton* itemButton7 = new wxButton(this, wxID_OK, _("&OK "));
    itemBoxSizer9->Add(itemButton7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( this, wxID_CANCEL);
    itemBoxSizer9->Add(itemButton8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
}


void mmBudgetEntryDialog::OnOk(wxCommandEvent& event)
{
    int typeSelection = type_->GetSelection();

    int periodSel = itemChoice_->GetSelection();
    wxString period;
    if (periodSel == DEF_FREQ_NONE)
        period = "None";
    else if (periodSel == DEF_FREQ_MONTHLY)
        period = "Monthly";
    else if (periodSel == DEF_FREQ_YEARLY)
        period = "Yearly";
    else if (periodSel == DEF_FREQ_WEEKLY)
        period = "Weekly";
    else if (periodSel == DEF_FREQ_BIWEEKLY)
        period = "Bi-Weekly";
    else if (periodSel == DEF_FREQ_BIMONTHLY)
        period = "Bi-Monthly";
    else if (periodSel == DEF_FREQ_QUARTERLY)
        period = "Quarterly";
    else if (periodSel == DEF_FREQ_HALFYEARLY)
        period = "Half-Yearly";
    else if (periodSel == DEF_FREQ_DAILY)
        period = "Daily";
    else
        wxASSERT(false);

    wxString displayAmtString = textAmount_->GetValue().Trim();
    double amt = 0.0;
    if (!Model_Currency::fromString(displayAmtString, amt, Model_Currency::GetBaseCurrency()) || amt < 0)
    {
        mmShowErrorMessage(this, _("Invalid Amount Entered "), _("Error"));
        return;
    }

    if (period == "None" && amt > 0) {
        itemChoice_->SetFocus();
        itemChoice_->SetSelection(DEF_FREQ_MONTHLY);
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

    int i = itemChoice_->GetSelection();
    if (event.GetKeyCode()==WXK_DOWN) 
    {
        if (i < DEF_FREQ_DAILY ) 
            itemChoice_->SetSelection(++i);
    } 
    else if (event.GetKeyCode()==WXK_UP)
    {
        if (i > DEF_FREQ_NONE)
            itemChoice_->SetSelection(--i);
    } 
    else 
        event.Skip();

}

void mmBudgetEntryDialog::OnTextEntered(wxCommandEvent& event)
{
    if (event.GetId() == textAmount_->GetId())
    {
        textAmount_->Calculate(Model_Currency::GetBaseCurrency());
    }
}
