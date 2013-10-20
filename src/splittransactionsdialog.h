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

#ifndef _MM_EX_SPLITTRANSACTIONDIALOG_H_
#define _MM_EX_SPLITTRANSACTIONDIALOG_H_

#include "guiid.h"
#include "mmtransaction.h"
#include "model/Model_Splittransaction.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxListCtrl;
////@end forward declarations

#define ID_LISTCTRL3 10118

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * SplitTransactionDialog class declaration
 */

class SplitTransactionDialog: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( SplitTransactionDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    SplitTransactionDialog( );
    SplitTransactionDialog(
        Model_Splittransaction::Data_Set &split,
        wxWindow* parent,
        int transType,
        mmCoreDB* core,
        mmSplitTransactionEntries* splt);

    /// Creation
    bool Create(
        wxWindow* parent,
        wxWindowID id,
        const wxString& caption,
        const wxPoint& pos,
        const wxSize& size,
        long style);

    /// Creates the controls and sizers
    void CreateControls();

    void DataToControls();

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_NEW
    void OnButtonAddClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_DELETE
    void OnButtonRemoveClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_EDIT
    void OnButtonEditClick( wxCommandEvent& event );

    void UpdateSplitTotal();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxListCtrl* lcSplit_;
    wxStaticText* transAmount_;

    void SetDisplaySplitCategories();

private:
    Model_Splittransaction::Data_Set split_;
    int transType_;

    mmCoreDB* core_; // TODO
    mmSplitTransactionEntries* splt_;

    wxButton* itemButtonNew_;
    wxButton* itemButtonEdit_;
    wxButton* itemButtonDelete_;
    wxButton* itemButtonOK_;

    void OnListDblClick(wxListEvent& event);
    void EditEntry();
    long GetSelectedItem();
};

#endif
