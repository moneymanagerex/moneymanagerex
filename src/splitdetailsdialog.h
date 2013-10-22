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

#include "defs.h"
#include "model/Model_Splittransaction.h"

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

class SplitDetailDialog: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( SplitDetailDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    SplitDetailDialog();
    SplitDetailDialog( 
        wxWindow* parent
        , Model_Splittransaction::Data* split
        , const wxString& categString
        , int* categID
        , int* subcategID
        , double* amount
        , int transType
    );

    int* m_categID_;
    int* m_subcategID_;
    double* m_amount_;
    wxString m_categString_;

private:
    /// Creation
    bool Create(wxWindow* parent);

    /// Creates the controls and sizers
    void CreateControls();

////@begin SplitDetailDialog event handler declarations
    void OnButtonCategoryClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTONOK
    void OnButtonOKClick( wxCommandEvent& event );
    void onTextEntered(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& /*event*/);

////@end SplitDetailDialog event handler declarations

////@begin SplitDetailDialog member function declarations
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin SplitDetailDialog member variables
    Model_Splittransaction::Data* split_;

    int transType_;
    int localTransType_;

    wxChoice* choiceType_;
    wxTextCtrl* textAmount_;
    wxButton* bCategory_;
};

#endif
