/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011,2012 Nikolay & Stefano Giorgio
 Copyright (C) 2017 James Higley

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

#include "budgetyearentrydialog.h"
#include "paths.h"
#include "constants.h"
#include <wx/spinctrl.h>
#include "Model_Budgetyear.h"
#include "Model_Budget.h"

wxIMPLEMENT_DYNAMIC_CLASS(mmBudgetYearEntryDialog, wxDialog);

wxBEGIN_EVENT_TABLE( mmBudgetYearEntryDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmBudgetYearEntryDialog::OnOk)
wxEND_EVENT_TABLE()

mmBudgetYearEntryDialog::mmBudgetYearEntryDialog() 
{
}

mmBudgetYearEntryDialog::mmBudgetYearEntryDialog(wxWindow* parent
    , bool withMonth)
{
    withMonth_ = withMonth;
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("Budget Entry Details"), wxDefaultPosition, wxSize(500, 300), style);
    if (withMonth_)
        this->SetTitle(_("Budget Month Entry"));
}

bool mmBudgetYearEntryDialog::Create(wxWindow* parent, wxWindowID id
    , const wxString& caption, const wxPoint& pos
    , const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmex::getProgramIcon());
    Centre();
    return TRUE;
}

void mmBudgetYearEntryDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemGridSizer2, 0, wxALL, 5);
    
    wxStaticText* itemStaticText3 = new wxStaticText( this, wxID_STATIC, _("Budget Year:"));
    itemGridSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

    int year = wxDate::GetCurrentYear();
    textYear_ = new wxSpinCtrl( this, wxID_ANY
        , wxEmptyString, wxDefaultPosition, wxSize(100,-1), wxSP_ARROW_KEYS, 1900, 3000, year);
    textYear_->SetValue(year);
    textYear_->SetToolTip(_("Specify the required year.\n"
        "Use Spin buttons to increase or decrease the year."));
    itemGridSizer2->Add(textYear_, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

    if (withMonth_)
    {
        wxStaticText* itemStaticTextMonth = new wxStaticText(this
            , wxID_STATIC, _("Budget Month:"), wxDefaultPosition, wxDefaultSize, 0);
        itemGridSizer2->Add(itemStaticTextMonth, 0
            , wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

        int month = wxDate::GetCurrentMonth() + 1; // we require months(1..12)
        textMonth_ = new wxSpinCtrl(this, wxID_ANY
            , wxEmptyString, wxDefaultPosition, wxSize(textYear_->GetSize())
            , wxSP_ARROW_KEYS, 1, 12, month);
        textMonth_->SetValue(month);
        textMonth_->SetToolTip(_("Specify the required month.\n"
            "Use Spin buttons to increase or decrease the month."));
 
        itemGridSizer2->Add(textMonth_, 0, wxALIGN_LEFT |wxALIGN_CENTER_VERTICAL|wxALL, 5);
    }

    wxStaticText* itemStaticText51 = new wxStaticText(this
        , wxID_STATIC, _("Base Budget On:"));
    itemGridSizer2->Add(itemStaticText51, 0
        , wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxArrayString itemYearStrings;
    itemYearStrings.Add("None");
    
    itemChoice_ = new wxChoice( this, wxID_ANY 
        , wxDefaultPosition, wxSize(textYear_->GetSize()), itemYearStrings );
    itemGridSizer2->Add(itemChoice_, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemChoice_->SetToolTip(_("Specify year to base budget on."));

    int index = 1;
    for (const auto& e : Model_Budgetyear::instance().all())
    {
        const wxString& budgetYearString = e.BUDGETYEARNAME;
        itemChoice_->Insert(budgetYearString, index++);
    }
    itemChoice_->SetSelection(0);
    
    wxStaticLine* line = new wxStaticLine ( this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(line, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
   
    wxButton* itemButtonOK = new wxButton( this, wxID_OK, _("&OK ") );
    itemBoxSizer9->Add(itemButtonOK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButtonCancel = new wxButton( this, wxID_CANCEL, wxGetTranslation(g_CancelLabel) );
    itemBoxSizer9->Add(itemButtonCancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

void mmBudgetYearEntryDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    wxString currYearText = wxString() << textYear_->GetValue();
    wxString baseYear = itemChoice_->GetStringSelection();

    if (withMonth_)
    {
        wxString currMonthText = wxEmptyString;
        currMonthText << textMonth_->GetValue();
        if (currMonthText.length() != 2 )
            currMonthText = wxString() << "0" << currMonthText;

        currYearText << "-" << currMonthText;
    }

    if (Model_Budgetyear::instance().Get(currYearText) != -1)
    {   
        wxMessageBox(_("Budget Year already exists")
            , _("Budget Entry Details"), wxICON_WARNING);
        return;
    }
    else
    {
        Model_Budgetyear::instance().Add(currYearText);
        if (baseYear != "None")
        {
            int baseYearID = Model_Budgetyear::instance().Get(baseYear);
            int newYearID  = Model_Budgetyear::instance().Get(currYearText);
            Model_Budget::copyBudgetYear(newYearID, baseYearID);
        }
    }

    EndModal(wxID_OK);
}
