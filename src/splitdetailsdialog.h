/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Madhan Kanagavel
// Modified by: Stefano Giorgio, Nikolay
// Mod Date:    Dec 2011
// Created:     04/24/07 21:15:09
// RCS-ID:
// Copyright:
// Licence:
/////////////////////////////////////////////////////////////////////////////

#ifndef _MM_EX_SPLITDETAILSDIALOG_H_
#define _MM_EX_SPLITDETAILSDIALOG_H_

/*!
 * Includes
 */

////@begin includes
#include "guiid.h"
#include "defs.h"
#include "mmcoredb.h"
////@end includes


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_SPLIT_DETAIL_DIALOG 10125
#define SYMBOL_SPLITDETAILDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_SPLITDETAILDIALOG_TITLE _("Split Detail Dialog")
#define SYMBOL_SPLITDETAILDIALOG_IDNAME ID_SPLIT_DETAIL_DIALOG
#define SYMBOL_SPLITDETAILDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_SPLITDETAILDIALOG_POSITION wxDefaultPosition
//#define ID_BUTTONCANCEL 10126
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
 * SplitDetailDialog class declaration
 */

class SplitDetailDialog: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( SplitDetailDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    SplitDetailDialog( );
    SplitDetailDialog( mmCoreDB* core,
                       const wxString categString,
                       int* categID,
                       int* subcategID,
                       double* amount,
                       int transType,
                       wxWindow* parent,
                       wxWindowID id = SYMBOL_SPLITDETAILDIALOG_IDNAME,
                       const wxString& caption = SYMBOL_SPLITDETAILDIALOG_TITLE,
                       const wxPoint& pos = SYMBOL_SPLITDETAILDIALOG_POSITION,
                       const wxSize& size = SYMBOL_SPLITDETAILDIALOG_SIZE, long style = SYMBOL_SPLITDETAILDIALOG_STYLE );

    int* m_categID_;
    int* m_subcategID_;
    double* m_amount_;
    wxString m_categString_;

private:
    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_SPLITDETAILDIALOG_IDNAME, const wxString& caption = SYMBOL_SPLITDETAILDIALOG_TITLE, const wxPoint& pos = SYMBOL_SPLITDETAILDIALOG_POSITION, const wxSize& size = SYMBOL_SPLITDETAILDIALOG_SIZE, long style = SYMBOL_SPLITDETAILDIALOG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin SplitDetailDialog event handler declarations
    void OnButtonCategoryClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTONOK
    void OnButtonOKClick( wxCommandEvent& event );
    void onTextEntered(wxCommandEvent& event);

////@end SplitDetailDialog event handler declarations

////@begin SplitDetailDialog member function declarations
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin SplitDetailDialog member variables
    mmCoreDB* core_;
    int transType_;
    int localTransType_;

    wxChoice* choiceType_;
    wxTextCtrl* textAmount_;
    wxButton* bCategory_;
};

#endif
