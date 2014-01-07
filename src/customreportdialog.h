/*******************************************************
 Copyright (C) 2007 Madhan Kanagavel
 Copyright (C) 2011 Stefano Giorgio
 Copyright (C) 2014 Nikolay

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

#include "defs.h"
#include <wx/webview.h>
#include <wx/webviewfshandler.h>

class wxStyledTextCtrl;
class mmGeneralReportManager: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmGeneralReportManager )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    mmGeneralReportManager( ) {}
    virtual ~mmGeneralReportManager();

    mmGeneralReportManager(wxWindow* parent);

private:
    enum
    {
        ID_TAB_OUT = 0,
        ID_TAB1,
        ID_TAB2,
        ID_TAB3,
        ID_NEW1 = wxID_HIGHEST + 500,
        ID_DELETE,
        ID_OUTPUT_WIN,
        ID_NOTEBOOK,
        ID_TEMPLATE,
        ID_WEB,
        ID_TYPELABEL,
        ID_LUACONTENT
    };

    bool Create(wxWindow* parent
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);
    /// Creates the controls and sizers
    void CreateControls();
    void fillControls();
    void OnImportReportEvt(wxCommandEvent& event);
    void openReport();
    bool openZipFile(const wxString &reportFileName, wxString &sql, wxString &lua, wxString &htt, wxString &readme);
    void OnUpdateReport(wxCommandEvent& event);
    wxString openTemplate();
    void OnExportReport(wxCommandEvent& event);
    void OnRun(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnItemRightClick(wxTreeEvent& event);
    void OnSelChanged(wxTreeEvent& event);
    void viewControls(bool enable);
    void OnLabelChanged(wxTreeEvent& event);
    bool DeleteReport(int id);
    void OnMenuSelected(wxCommandEvent& event);
    void newReport();

    wxTextCtrl* m_fileNameCtrl;
    wxWebView* m_outputHTML;

    wxButton* button_Open_;
    wxButton* button_Save_;
    wxButton* button_SaveAs_;
    wxButton* button_Run_;
    wxTreeCtrl* treeCtrl_;
    wxTreeItemId root_;
    wxTreeItemId selectedItemId_;
    wxString m_selectedGroup;
};

class MyTreeItemData : public wxTreeItemData
{
public:
    MyTreeItemData(int report_id, wxString selectedGroup) : m_report_id(report_id)
        , m_selectedGroup(selectedGroup) { }
    int get_report_id() { return m_report_id; }
    wxString get_group_name() { return m_selectedGroup; }
private:
    int m_report_id;
    wxString m_selectedGroup;
};

