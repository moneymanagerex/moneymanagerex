/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Madhan Kanagavel
// Modified by: Stefano Giorgio, Nikolay
// Mod Date:    Dec 2011
// Created:     04/24/07 21:52:26
// RCS-ID:
// Copyright:
// Licence:
/////////////////////////////////////////////////////////////////////////////

#include "splitdetailsdialog.h"
#include "categdialog.h"
#include "util.h"
#include "mmCurrencyFormatter.h"
#include "constants.h"
#include "validators.h"

#include <wx/valnum.h>

IMPLEMENT_DYNAMIC_CLASS( SplitDetailDialog, wxDialog )

enum
{
    ID_DIALOG_SPLTTRANS_TYPE = wxID_HIGHEST + 1,
    ID_BUTTONCATEGORY,
    ID_TEXTCTRLAMOUNT,
};

BEGIN_EVENT_TABLE( SplitDetailDialog, wxDialog )
    EVT_BUTTON( ID_BUTTONCATEGORY, SplitDetailDialog::OnButtonCategoryClick )
    EVT_BUTTON( mmID_OK, SplitDetailDialog::OnButtonOKClick )
    EVT_BUTTON( mmID_CANCEL, SplitDetailDialog::OnCancel )
    EVT_TEXT_ENTER( ID_TEXTCTRLAMOUNT, SplitDetailDialog::onTextEntered )
END_EVENT_TABLE()

SplitDetailDialog::SplitDetailDialog( )
{
}

SplitDetailDialog::SplitDetailDialog( 
    Model_Splittransaction::Data split
    , mmCoreDB* core
    , const wxString categString
    , int* categID
    , int* subcategID
    , double* amount
    , int transType
    , wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style )
{
    split_ = split;
    core_ = core;
    m_categID_  = categID;
    m_subcategID_ = subcategID;
    transType_ = transType;
    localTransType_ = transType;

    if (*amount < 0.0)
    {
        if (transType == DEF_DEPOSIT)
           localTransType_ = DEF_WITHDRAWAL;
        else if (transType == DEF_WITHDRAWAL)
           localTransType_ = DEF_DEPOSIT;

        *amount = abs(*amount);
    }
    m_amount_ = amount;

    m_categString_ = categString;
    Create(parent, id, caption, pos, size, style);
}

bool SplitDetailDialog::Create( wxWindow* parent, wxWindowID id,
                               const wxString& caption,
                               const wxPoint& pos,
                               const wxSize& size,
                               long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();

    textAmount_->SetValue( *m_amount_ != 0 ? CurrencyFormatter::float2String(*m_amount_) : "" );
    textAmount_->SetFocus();
    return TRUE;
}

void SplitDetailDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, _("Split Transaction Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxTOP|wxRIGHT, 10);

    wxPanel* itemPanel7 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemStaticBoxSizer4->Add(itemPanel7, 0, wxGROW|wxALL, 10);

    wxFlexGridSizer* controlSizer = new wxFlexGridSizer(0, 2, 10, 10);
    itemPanel7->SetSizer(controlSizer);

    wxStaticText* staticTextType = new wxStaticText( itemPanel7, wxID_STATIC, _("Type"));
    controlSizer->Add(staticTextType, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);

    const wxString itemChoiceStrings[] =
    {
        _("Withdrawal"),
        _("Deposit"),
    };
    choiceType_ = new wxChoice(itemPanel7,ID_DIALOG_SPLTTRANS_TYPE,wxDefaultPosition,wxDefaultSize,2,itemChoiceStrings,0);
    choiceType_->SetSelection(localTransType_);
    choiceType_->SetToolTip(_("Specify the type of transactions to be created."));
    controlSizer->Add(choiceType_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);

    wxStaticText* staticTextAmount = new wxStaticText( itemPanel7, wxID_STATIC, _("Amount"));
    controlSizer->Add(staticTextAmount, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);

    textAmount_ = new wxTextCtrl( itemPanel7, ID_TEXTCTRLAMOUNT, ""
        , wxDefaultPosition, wxSize(110,-1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmCalcValidator());
    controlSizer->Add(textAmount_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* staticTextCategory = new wxStaticText( itemPanel7, wxID_STATIC, _("Category"));
    controlSizer->Add(staticTextCategory, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);
    bCategory_ = new wxButton( itemPanel7, ID_BUTTONCATEGORY, m_categString_
        , wxDefaultPosition, wxSize(200, -1), 0 );
    controlSizer->Add(bCategory_, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    /**************************************************************************
     Control Buttons
    ***************************************************************************/
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(buttonSizer, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButtonOK = new wxButton( this, mmID_OK, _("&OK"));
    buttonSizer->Add(itemButtonOK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButtonCancel = new wxButton( this, mmID_CANCEL, _("&Cancel"));
    buttonSizer->Add(itemButtonCancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTONCATEGORY
 */

void SplitDetailDialog::OnButtonCategoryClick( wxCommandEvent& /*event*/ )
{
    mmCategDialog dlg(core_, this);
    dlg.setTreeSelection(*m_categID_, *m_subcategID_);
    if ( dlg.ShowModal() == wxID_OK )
    {
        *m_categID_ = dlg.getCategId();
        *m_subcategID_ = dlg.getSubCategId();

        m_categString_ = dlg.getFullCategName();
        bCategory_->SetLabel(m_categString_);
    }
}

void SplitDetailDialog::onTextEntered(wxCommandEvent& event)
{
    wxString sAmount = "";

    if (mmCalculator(textAmount_->GetValue(), sAmount))
        textAmount_->SetValue(sAmount);
    textAmount_->SetInsertionPoint(textAmount_->GetValue().Len());

    event.Skip();
}

void SplitDetailDialog::OnButtonOKClick( wxCommandEvent& /*event*/ )
{
    if (*m_categID_ == -1)
    {
        mmShowErrorMessage(this, _("Invalid Category Entered "), _("Error"));
        bCategory_->SetFocus();
        return;
    }

    wxString amountStr = textAmount_->GetValue().Trim();
    double amount;
    if (! CurrencyFormatter::formatCurrencyToDouble(amountStr, amount) || (amount < 0.0))
    {
        mmShowErrorMessage(this, _("Invalid Amount Entered "), _("Error"));
        textAmount_->SetFocus();
        return;
    }

    if ( choiceType_->GetSelection() != transType_ )
    {
            amount = -amount;
    }

    *m_amount_ = amount;

    EndModal(wxID_OK);
}

bool SplitDetailDialog::ShowToolTips()
{
    return TRUE;
}

wxBitmap SplitDetailDialog::GetBitmapResource( const wxString& /*name*/ )
{
    return wxNullBitmap;
}

wxIcon SplitDetailDialog::GetIconResource( const wxString& /*name*/ )
{
    return wxNullIcon;
}
void SplitDetailDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}
