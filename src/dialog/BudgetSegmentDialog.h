/*******************************************************
 Copyright (C) 2026

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

#include "base/_defs.h"
#include "model/BudgetSegmentModel.h"

// Defines the intra-period windows of a budget period, so a month can be
// planned against paycheck-aligned halves instead of a single lump sum.
class BudgetSegmentDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(BudgetSegmentDialog);
    wxDECLARE_EVENT_TABLE();

public:
    BudgetSegmentDialog();
    BudgetSegmentDialog(wxWindow* parent, int64 budget_period_id);

    bool Create(wxWindow* parent, wxWindowID id,
        const wxString& caption,
        const wxPoint& pos,
        const wxSize& size,
        long style);

private:
    void CreateControls();
    void fillControls();

    void OnAdd(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnSplitHalves(wxCommandEvent& event);
    void OnDoubleClicked(wxCommandEvent& event);

    // Prompt for one segment's fields. Returns false if cancelled or invalid.
    bool editSegment(BudgetSegmentData& seg);

    int64      m_bp_id = -1;
    wxListBox* m_listBox = nullptr;

    enum { ID_SPLIT_HALVES = wxID_HIGHEST + 480 };
};
