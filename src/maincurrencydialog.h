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

#ifndef MM_EX_MAINCURRENCY_DIALOG_H_
#define MM_EX_MAINCURRENCY_DIALOG_H_

#include "defs.h"
#include <wx/dataview.h>
#include <map>

class mmMainCurrencyDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmMainCurrencyDialog);
    wxDECLARE_EVENT_TABLE();

public:
    /// Constructors
    mmMainCurrencyDialog( ) {}

    mmMainCurrencyDialog(wxWindow* parent,
                          bool bEnableSelect = true);

    static bool Execute(wxWindow* parent, int& currencyID);

private:
    enum cols
    {
        CURR_BASE = 0,
        CURR_SYMBOL,
        CURR_NAME,
        BASE_RATE,
        ID_DIALOG = wxID_HIGHEST + 600,
        MENU_ITEM1,
        MENU_ITEM2
    };

    bool Create(wxWindow* parent
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);

    /// Creates the controls and sizers

    void CreateControls();
    void OnBtnAdd(wxCommandEvent& event);
    void OnBtnEdit(wxCommandEvent& event);
    void OnBtnSelect(wxCommandEvent& event);
    void OnBtnDelete(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnListItemActivated(wxDataViewEvent& event);
    void OnListItemSelected(wxDataViewEvent& event);
    void OnValueChanged(wxDataViewEvent& event);
    void fillControls();
    void OnShowHiddenChbClick(wxCommandEvent& event);

    void OnOnlineUpdateCurRate(wxCommandEvent& event);
    bool onlineUpdateCurRate(int curr_id = -1);
    void OnItemRightClick(wxDataViewEvent& event);
    void OnMenuSelected(wxCommandEvent& event);

    wxDataViewListCtrl* currencyListBox_;
    std::map<int, wxString> ColName_;
    bool bEnableSelect_;
    double curr_rate_;
    wxButton* itemButtonEdit_;
    wxButton* itemButtonDelete_;
    wxCheckBox* cbShowAll_;

    int currencyID_;
    int selectedIndex_;
};

#endif // MM_EX_MAINCURRENCY_DIALOG_H_

