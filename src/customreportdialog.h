/*******************************************************
 Copyright (C) 2007 Madhan Kanagavel
 Copyright (C) 2011 Stefano Giorgio

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

#ifndef _MM_EX_CUSTOMREPORTDIALOG_H_
#define _MM_EX_CUSTOMREPORTDIALOG_H_

#include "guiid.h"
#include "customreportindex.h"
#include <wx/timer.h>

#define SYMBOL_CUSTOMSQLDIALOG_TITLE _("Custom Reports Manager")
#define SYMBOL_CUSTOMSQLDIALOG_POSITION wxDefaultPosition
#define SYMBOL_CUSTOMSQLDIALOG_SIZE wxSize(500, 400)
#define SYMBOL_CUSTOMSQLDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX

class mmCustomSQLDialog: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmCustomSQLDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    mmCustomSQLDialog( ) {}
    virtual ~mmCustomSQLDialog();

    mmCustomSQLDialog(CustomReportIndex* reportIndex, wxString customSqlReportSelectedItem, wxWindow* parent,
        wxWindowID id           = wxID_ANY,
        const wxString& caption = SYMBOL_CUSTOMSQLDIALOG_TITLE,
        const wxPoint& pos      = SYMBOL_CUSTOMSQLDIALOG_POSITION,
        const wxSize& size      = SYMBOL_CUSTOMSQLDIALOG_SIZE,
        long style              = SYMBOL_CUSTOMSQLDIALOG_STYLE );

    wxString sScript();
    wxString sReportTitle();
    wxString sSctiptType();

private:
    wxWindow* parent_;
    CustomReportIndex* reportIndex_;
    wxTextCtrl* tcSourceTxtCtrl_;
    wxString sQuery_;
    bool newFileCreated_;
    wxString loadedFileName_;
    wxString customSqlReportSelectedItem_;
    bool edit_;
    bool newload_;
    bool navCtrlUpdateRequired_;
    wxButton* button_Open_;
    wxButton* button_Save_;
    wxButton* button_Run_;
    wxButton* button_Clear_;
    wxTextCtrl* reportTitleTxtCtrl_;
    wxCheckBox* headingOnlyCheckBox_;
    wxCheckBox* subMenuCheckBox_;
    wxRadioBox* m_radio_box_;
    wxTreeCtrl* treeCtrl_;
    int iSelectedId_;
    wxTimer* timer_;
    static const int INTERVAL = 250;

    bool Create( wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = SYMBOL_CUSTOMSQLDIALOG_TITLE,
        const wxPoint& pos = SYMBOL_CUSTOMSQLDIALOG_POSITION,
        const wxSize& size = SYMBOL_CUSTOMSQLDIALOG_SIZE,
        long style = SYMBOL_CUSTOMSQLDIALOG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();
    void fillControls();
    void OnOpen(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    bool SaveCustomReport();
    void OnRun(wxCommandEvent& event);
    void OnClear(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnCheckedHeading(wxCommandEvent& event);
    void OnCheckedSubReport(wxCommandEvent& event);
    void OnTextChangeHeading(wxCommandEvent& event);
    void OnTextChangeSubReport(wxCommandEvent& event);
    void SetDialogBoxForHeadings(bool bHeading);
    void OnItemRightClick(wxTreeEvent& event);
    void OnSelChanged(wxTreeEvent& event);
    void OnLabelChanged(wxTreeEvent& event);
    bool DeleteCustomSqlReport();
    void OnMenuSelected(wxCommandEvent& event);
    void OnSourceTxtChar(wxKeyEvent& event);
    void ShowCursorCoordinates(wxTimerEvent& /*event*/);
};

#endif

