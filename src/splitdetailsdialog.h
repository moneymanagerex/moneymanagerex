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
#include "mmtextctrl.h"
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
        , int transType
    );

private:
    bool Create(wxWindow* parent);
    void CreateControls();
    void DataToControls();

    void OnButtonCategoryClick( wxCommandEvent& event );

    void OnButtonOKClick( wxCommandEvent& event );
    void onTextEntered(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& /*event*/);

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );
    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
    /// Should we show tooltips?
    static bool ShowToolTips();

    Model_Splittransaction::Data* split_;

    int transType_;
    int localTransType_;

    wxChoice* choiceType_;
    mmTextCtrl* textAmount_;
    wxButton* bCategory_;
};

#endif
