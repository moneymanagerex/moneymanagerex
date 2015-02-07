/*******************************************************
Copyright (C) 2014 Nikolay
Copyright (C) 2015 Stefano Giorgio

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

#include "constants.h"
#include "defs.h"
#include <wx/spinbutt.h>
#include "mmtextctrl.h"

class mmUserPanelTrans : public wxPanel
{
    wxDECLARE_EVENT_TABLE();

public:
    mmUserPanelTrans();

    mmUserPanelTrans(wxWindow *parent
        , wxWindowID id = wxID_ANY
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL
        , const wxString &name = wxPanelNameStr);

    ~mmUserPanelTrans();

//    virtual void Save();

public:
    int m_checking_trans_id;
    int m_account_id;
    int m_payee_id;
    int m_category_id;
    int m_subcategory_id;

private:
    void Create();
    void OnActivateAccountButton(wxCommandEvent& WXUNUSED(event));
    void OnActivatePayeeButton(wxCommandEvent& WXUNUSED(event));
    void OnActivateCategoryButton(wxCommandEvent& WXUNUSED(event));

    void OnDateSelectorForward(wxSpinEvent& WXUNUSED(event));
    void OnDateSelectorBackward(wxSpinEvent& WXUNUSED(event));
    void SetNewDate(wxDatePickerCtrl* dpc, bool forward = true);

    void OnEnteredText(wxCommandEvent& event);
    void OnFrequentNotes(wxCommandEvent& event);
    void onSelectedNote(wxCommandEvent& event);
    void OnAttachments(wxCommandEvent& WXUNUSED(event));

private:
    wxDatePickerCtrl* m_date_selector;
    wxSpinButton* m_date_controller;

    wxButton* m_account;
    wxChoice* m_status_selector;
    wxChoice* m_type_selector;
    mmTextCtrl* m_entered_amount;
    wxCheckBox* m_transfer;
    wxButton* m_payee;
    wxButton* m_category;
    wxTextCtrl* m_entered_number;
    wxTextCtrl* m_entered_notes;
    std::vector<wxString> m_frequent_notes;
    wxBitmapButton* m_attachment;
 
    enum
    {
        ID_TRANS_DATE_SELECTOR = wxID_HIGHEST + 10,
        ID_TRANS_DATE_CONTROLLER,
        ID_TRANS_STATUS_SELECTOR,
        ID_TRANS_ACCOUNT_BUTTON_TEXT,
        ID_TRANS_ACCOUNT_BUTTON,
        ID_TRANS_PAYEE_BUTTON_TEXT,
        ID_TRANS_PAYEE_BUTTON,
        ID_TRANS_CATEGORY_BUTTON,
        ID_TRANS_ENTERED_NUMBER,
        ID_TRANS_ENTERED_AMOUNT,
        ID_TRANS_ENTERED_NOTES,
        ID_TRANS_FREQUENT_NOTES,
        ID_TRANS_TRANSFER
    };
};
