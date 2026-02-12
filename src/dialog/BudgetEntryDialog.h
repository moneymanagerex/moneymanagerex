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

#pragma once

#include "base/defs.h"
#include "util/mmTextCtrl.h"
#include "model/BudgetModel.h"

class BudgetEntryDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(BudgetEntryDialog);
    wxDECLARE_EVENT_TABLE();

public:
    BudgetEntryDialog();
    BudgetEntryDialog(wxWindow* parent
        , BudgetModel::Data* entry
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
    BudgetModel::Data* budgetEntry_ = nullptr;
    wxChoice* m_choiceItem = nullptr;
    mmTextCtrl* m_textAmount = nullptr;
    wxChoice* m_choiceType = nullptr;
    wxTextCtrl* m_Notes = nullptr;

    wxString catEstimateAmountStr_;
    wxString catActualAmountStr_;

    enum { DEF_TYPE_EXPENSE, DEF_TYPE_INCOME };
    enum { DEF_FREQ_NONE, DEF_FREQ_WEEKLY, DEF_FREQ_BIWEEKLY, DEF_FREQ_MONTHLY, DEF_FREQ_BIMONTHLY, DEF_FREQ_QUARTERLY, DEF_FREQ_HALFYEARLY, DEF_FREQ_YEARLY, DEF_FREQ_DAILY };
};

