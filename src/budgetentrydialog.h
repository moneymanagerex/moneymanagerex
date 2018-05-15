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

#ifndef MM_EX_BUDGETENTRYDIALOG_H_
#define MM_EX_BUDGETENTRYDIALOG_H_

#include <wx/dialog.h>
#include "Model_Budget.h"
class mmTextCtrl;
class wxChoice;

class mmBudgetEntryDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmBudgetEntryDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmBudgetEntryDialog();
    mmBudgetEntryDialog(wxWindow* parent
        , Model_Budget::Data* entry
        , const wxString& categoryEstimate, const wxString& CategoryActual);

    bool Create(wxWindow* parent, wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);

    void CreateControls();
    void onChoiceChar(wxKeyEvent& event);
    // utility functions
    void OnOk(wxCommandEvent& event);

    void fillControls();

private:
    void OnTextEntered(wxCommandEvent& event);

    Model_Budget::Data* budgetEntry_;
    wxChoice* m_choiceItem;
    mmTextCtrl* m_textAmount;
    wxChoice* m_choiceType;

    wxString catEstimateAmountStr_;
    wxString catActualAmountStr_;

    enum { DEF_TYPE_EXPENSE, DEF_TYPE_INCOME };
    enum { DEF_FREQ_NONE, DEF_FREQ_WEEKLY, DEF_FREQ_BIWEEKLY, DEF_FREQ_MONTHLY, DEF_FREQ_BIMONTHLY, DEF_FREQ_QUARTERLY, DEF_FREQ_HALFYEARLY, DEF_FREQ_YEARLY, DEF_FREQ_DAILY };

};

#endif

