/*******************************************************
Copyright (C) 2025 Klaus Wich

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

#include "optionsettingsbase.h"
#include "reports/mmDateRange.h"
#include "wx/spinctrl.h"

class OptionSettingsTransaction : public OptionSettingsBase
{
//    wxDECLARE_EVENT_TABLE();

public:
    OptionSettingsTransaction();

    OptionSettingsTransaction(wxWindow *parent
        , wxWindowID id = wxID_ANY
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL
        , const wxString &name = wxPanelNameStr);

    ~OptionSettingsTransaction();

public:
    virtual bool SaveSettings();
    const wxSharedPtr<mmDateRange> get_inc_vs_exp_date_range() const;

private:
    void Create();
    void OnNavTreeColorChanged(wxCommandEvent& event);

private:
    /// Color Buttons.
    wxButton* m_UDFCB1 = nullptr;
    wxButton* m_UDFCB2 = nullptr;
    wxButton* m_UDFCB3 = nullptr;
    wxButton* m_UDFCB4 = nullptr;
    wxButton* m_UDFCB5 = nullptr;
    wxButton* m_UDFCB6 = nullptr;
    wxButton* m_UDFCB7 = nullptr;

    wxCheckBox* m_use_org_date_copy_paste = nullptr;
    wxCheckBox* m_use_org_date_duplicate = nullptr;
    wxCheckBox* m_use_org_state_duplicate_paste = nullptr;
    wxChoice* m_use_sound = nullptr;

    wxCheckBox* m_budget_financial_years = nullptr;
    wxCheckBox* m_budget_include_transfers = nullptr;
    wxCheckBox* m_budget_summary_without_category = nullptr;
    wxCheckBox* m_budget_override = nullptr;
    wxCheckBox* m_budget_deduct_monthly = nullptr;
    wxCheckBox* m_ignore_future_transactions = nullptr;
    wxCheckBox* m_use_trans_date_time = nullptr;
    wxSpinCtrl* m_budget_days_offset = nullptr;
    wxSpinCtrl* m_reporting_firstday = nullptr;
    wxChoice* m_reporting_first_weekday = nullptr;

    enum
    {
        ID_DIALOG_OPTIONS_BULK_ENTER  = wxID_HIGHEST + 16,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_PAYEE,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_NONTRANSFER,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_CATEGORY_TRANSFER,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_STATUS,
        ID_DIALOG_OPTIONS_DEFAULT_TRANSACTION_DATE,
    };
};
