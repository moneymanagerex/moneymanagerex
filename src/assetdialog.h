/*******************************************************
 Copyright (C) 2013 - 2016, 2020, 2022 Nikolay Akimov

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

#include "model/Model_Asset.h"
#include "model/Model_Currency.h"
#include "model/Model_Translink.h"

class mmDatePickerCtrl;
class mmTextCtrl;
class UserTransactionPanel;
class mmGUIFrame;

class mmAssetDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmAssetDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmAssetDialog(){};
    mmAssetDialog(wxWindow *parent, mmGUIFrame* gui_frame, Model_Asset::Data* asset, bool trans_data = false);
    mmAssetDialog(wxWindow *parent, mmGUIFrame* gui_frame, Model_Translink::Data* transfer_entry, Model_Checking::Data* checking_entry);

    Model_Asset::Data* m_asset;
    void SetTransactionAccountName(const wxString& account_name);
    void SetTransactionDate();

private:
    bool Create(wxWindow* parent
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX);
    void CreateControls();
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnAttachments(wxCommandEvent& event);
    void OnChangeAppreciationType(wxCommandEvent& event);
    void enableDisableRate(bool en);
    void onTextEntered(wxCommandEvent& event);
    void dataToControls();
    void changeFocus(wxChildFocusEvent& event);
    void OnQuit(wxCloseEvent& event);
    void CreateAssetAccount();
    void HideTransactionPanel();
private:
    mmGUIFrame* m_gui_frame;
    wxChoice*  m_assetType;
    mmTextCtrl* m_assetName;
    mmDatePickerCtrl* m_dpc;
    mmTextCtrl* m_notes;
    mmTextCtrl* m_value;
    mmTextCtrl* m_valueChangeRate;
    wxChoice*  m_valueChange;
    wxStaticText* m_valueChangeRateLabel;
    wxBitmapButton* bAttachments_;
    wxStaticBox* m_transaction_frame;
    UserTransactionPanel* m_transaction_panel;
    Model_Translink::Data* m_transfer_entry;
    Model_Checking::Data* m_checking_entry;
    wxString m_dialog_heading;
    bool m_hidden_trans_entry;
    bool assetRichText;

    enum
    {
        IDC_COMBO_TYPE = wxID_HIGHEST + 1100,
        IDC_NOTES,
        IDC_VALUE,
        IDC_RATE,
    };
};
