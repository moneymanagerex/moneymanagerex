/*******************************************************
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

class wxDatePickerCtrl;
class mmTextCtrl;

class mmAssetDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmAssetDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmAssetDialog(){};
    mmAssetDialog(wxWindow *parent, Model_Asset::Data* asset);

    Model_Asset::Data* m_asset;
private:
    bool Create(wxWindow* parent
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);
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

    bool assetRichText;

    mmTextCtrl* m_assetName;
    wxDatePickerCtrl* m_dpc;
    mmTextCtrl* m_notes;
    mmTextCtrl* m_value;
    mmTextCtrl* m_valueChangeRate;
    wxChoice*  m_assetType;
    wxChoice*  m_valueChange;
    wxStaticText* m_valueChangeRateLabel;
	wxBitmapButton* bAttachments_;

    enum
    {
        IDC_COMBO_TYPE = wxID_HIGHEST + 1100,
        IDC_NOTES,
        IDC_VALUE,
        IDC_RATE,
    };
};
