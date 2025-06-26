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

#include "option.h"
#include "model/Model_Asset.h"
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
    mmAssetDialog(wxWindow *parent, Model_Asset::Data* asset, const bool trans_data = false);
    mmAssetDialog(wxWindow *parent, Model_Translink::Data* transfer_entry, Model_Checking::Data* checking_entry);

    Model_Asset::Data* m_asset = nullptr;
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
    void enableDisableRate(bool en);
    double convertRate(int changeType, double xRate, int xCompounding, int yCompounding = Option::COMPOUNDING_ID_DAY);
    void OnChangeAppreciationType(wxCommandEvent& event);
    void OnChangeCompounding(wxCommandEvent& event);
    void dataToControls();
    void changeFocus(wxChildFocusEvent& event);
    void OnQuit(wxCloseEvent& event);
    void CreateAssetAccount();
    void HideTransactionPanel();
private:
    wxChoice*  m_assetType = nullptr;
    wxTextCtrl* m_assetName = nullptr;
    mmDatePickerCtrl* m_dpc = nullptr;
    wxTextCtrl* m_notes = nullptr;
    mmTextCtrl* m_value = nullptr;
    mmTextCtrl* m_curr_val = nullptr;
    wxChoice* m_valueChange = nullptr;
    wxStaticText* m_compoundingLabel = nullptr;
    wxChoice* m_compoundingChoice = nullptr;
    Option::COMPOUNDING_ID m_compounding = Option::COMPOUNDING_ID_DAY;
    wxStaticText* m_valueChangeRateLabel = nullptr;
    mmTextCtrl* m_valueChangeRate = nullptr;
    wxBitmapButton* bAttachments_ = nullptr;
    wxStaticBox* m_transaction_frame = nullptr;
    UserTransactionPanel* m_transaction_panel = nullptr;
    Model_Translink::Data* m_transfer_entry = nullptr;
    Model_Checking::Data* m_checking_entry = nullptr;
    wxString m_dialog_heading = _t("New Asset");
    bool m_hidden_trans_entry = true;
    bool assetRichText = true;

    enum
    {
        IDC_COMBO_TYPE = wxID_HIGHEST + 1100,
        IDC_VALUE,
        IDC_CURR_VAL,
        IDC_COMPOUNDING,
        IDC_RATE,
        IDC_NOTES,
    };
};
