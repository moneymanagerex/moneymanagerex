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
#include "util/mmTextCtrl.h"
#include "model/PlanAssumptionModel.h"

// Edits a single assumption: the value a plan is calculated from, such as an
// assumed share price or tax rate.
class PlanAssumptionEntryDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(PlanAssumptionEntryDialog);
    wxDECLARE_EVENT_TABLE();

public:
    PlanAssumptionEntryDialog();
    PlanAssumptionEntryDialog(wxWindow* parent, PlanAssumptionData* assumption);

    bool Create(wxWindow* parent, wxWindowID id,
        const wxString& caption,
        const wxPoint& pos,
        const wxSize& size,
        long style);

private:
    void CreateControls();
    void fillControls();
    void OnOk(wxCommandEvent& event);
    void OnKindChanged(wxCommandEvent& event);
    void updateHint();

    PlanAssumptionData* m_assumption_n = nullptr;

    wxTextCtrl* m_name  = nullptr;
    wxChoice*   m_kind  = nullptr;
    mmTextCtrl* m_value = nullptr;
    wxTextCtrl* m_scope = nullptr;
    wxTextCtrl* m_notes = nullptr;
    wxStaticText* m_hint = nullptr;
};

// Lists the assumptions the plan rests on, and how many items depend on each.
class PlanAssumptionDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(PlanAssumptionDialog);
    wxDECLARE_EVENT_TABLE();

public:
    PlanAssumptionDialog();
    PlanAssumptionDialog(wxWindow* parent);

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
    void OnDoubleClicked(wxCommandEvent& event);

    wxListBox* m_listBox = nullptr;
};
