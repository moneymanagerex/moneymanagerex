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
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxListCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_SPLITTRANSACTIONS_DIALOG 10117
#define SYMBOL_SPLITTRANSACTIONDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_SPLITTRANSACTIONDIALOG_TITLE _("Split Transaction Dialog")
#define SYMBOL_SPLITTRANSACTIONDIALOG_IDNAME ID_SPLITTRANSACTIONS_DIALOG
#define SYMBOL_SPLITTRANSACTIONDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_SPLITTRANSACTIONDIALOG_POSITION wxDefaultPosition
#define ID_LISTCTRL3 10118
//#define ID_BUTTONADD 10119
//#define ID_BUTTONREMOVE 10120
//#define ID_BUTTONCLOSE 10121
////@end control identifiers

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
        mmCoreDB* core,
        mmSplitTransactionEntries* split,
        int transType,
        wxWindow* parent,
        wxWindowID id = SYMBOL_SPLITTRANSACTIONDIALOG_IDNAME,
        const wxString& caption = SYMBOL_SPLITTRANSACTIONDIALOG_TITLE,
        const wxPoint& pos = SYMBOL_SPLITTRANSACTIONDIALOG_POSITION,
        const wxSize& size = SYMBOL_SPLITTRANSACTIONDIALOG_SIZE,
        long style = SYMBOL_SPLITTRANSACTIONDIALOG_STYLE );

    /// Creation
    bool Create(
        wxWindow* parent,
        wxWindowID id = SYMBOL_SPLITTRANSACTIONDIALOG_IDNAME,
        const wxString& caption = SYMBOL_SPLITTRANSACTIONDIALOG_TITLE,
        const wxPoint& pos = SYMBOL_SPLITTRANSACTIONDIALOG_POSITION,
        const wxSize& size = SYMBOL_SPLITTRANSACTIONDIALOG_SIZE,
        long style = SYMBOL_SPLITTRANSACTIONDIALOG_STYLE );

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
    mmCoreDB* core_;
    mmSplitTransactionEntries* split_;
    int transType_;

    wxButton* itemButtonNew_;
    wxButton* itemButtonEdit_;
    wxButton* itemButtonDelete_;
    wxButton* itemButtonOK_;

    void OnListDblClick(wxListEvent& event);
    void EditEntry();
    long GetSelectedItem();
};

#endif
