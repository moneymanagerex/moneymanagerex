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

#ifndef MM_EX_SPLITTRANSACTIONDIALOG_H_
#define MM_EX_SPLITTRANSACTIONDIALOG_H_

#include "defs.h"
#include "Model_Checking.h"
#include <wx/dataview.h>

class wxListCtrl;
class wxStaticText;

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

class SplitTransactionDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(SplitTransactionDialog);
    wxDECLARE_EVENT_TABLE();

public:
    /// Constructors
    SplitTransactionDialog();
    SplitTransactionDialog(wxWindow* parent
        , std::vector<Split>& split
        , int transType
        , int accountID
        , double totalAmount = 0.0
        , const wxString& name = "SplitTransactionDialog"
        );

    /// Creation
    bool Create(
        wxWindow* parent,
        wxWindowID id,
        const wxString& caption,
        const wxPoint& pos,
        const wxSize& size,
        long style
        , const wxString& name
        );
    std::vector<Split> getResult() { return m_splits; }
    bool isItemsChanged(){ return items_changed_; }
    void SetDisplaySplitCategories();

private:
    /// Creates the controls and sizers
    void CreateControls();

    void DataToControls();

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_NEW
    void OnButtonAddClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_DELETE
    void OnButtonRemoveClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_EDIT
    void OnButtonEditClick( wxCommandEvent& event );
    void OnOk( wxCommandEvent& event );

    void UpdateSplitTotal();

    wxDataViewListCtrl* lcSplit_;
    wxStaticText* transAmount_;

    std::vector<Split> m_splits;
    std::vector<Split> m_local_splits;
    int transType_;
    int accountID_;
    double totalAmount_;
    bool items_changed_;

    wxButton* itemButtonNew_;
    wxButton* itemButtonEdit_;
    wxButton* itemButtonDelete_;
    wxButton* itemButtonOK_;

    void SetDisplayEditDeleteButtons();
    void OnListDblClick(wxDataViewEvent& event);
    void OnListItemSelected(wxDataViewEvent& event);
    void EditEntry(int id);
    int selectedIndex_;
};

#endif
