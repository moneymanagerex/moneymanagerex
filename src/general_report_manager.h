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
#include <vector>
#include <wx/dataview.h>

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
        ID_TAB_SQL,
        ID_TAB_LUA,
        ID_TAB_HTT,
        MAGIC_NUM = 16,
        ID_NEW_SAMPLE = wxID_HIGHEST + 500,
        ID_NEW_EMPTY,
        ID_DELETE,
        ID_RENAME,
        ID_GROUP, 
        ID_NOTEBOOK,
        ID_WEB,
        ID_TYPELABEL,
        ID_TEMPLATE,
        ID_SQL_CONTENT,
        ID_LUA_CONTENT,
        ID_SQL_GRID,
        ID_CSS_CONTENT,
        ID_JS_CONTENT,
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
    void importReport();
    bool readTextFile(const wxString &fileName, wxString &data);
    bool openZipFile(const wxString &reportFileName
        , wxString &htt, wxString &sql, wxString &lua, wxString &readme);
    void OnUpdateReport(wxCommandEvent& event);
    void OnExportReport(wxCommandEvent& event);
    void OnRun(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnSqlTest(wxCommandEvent& event);
    void OnNewTemplate(wxCommandEvent& event);
    void OnItemRightClick(wxTreeEvent& event);
    void OnSelChanged(wxTreeEvent& event);
    //void OnLabelChanged(wxTreeEvent& event);
    void viewControls(bool enable);
    void renameReport(int id);
    bool DeleteReport(int id);
    void OnMenuSelected(wxCommandEvent& event);
    void newReport(bool sample = false);
    void createEditorTab(wxNotebook* notebook, int type);
    void createOutputTab(wxNotebook* notebook, int type);

    void getSqlQueryData(const wxString& sql);
    std::vector <std::vector <wxString> > m_sqlQueryData;

    wxWebView* m_outputHTML;

    wxButton* m_buttonOpen;
    wxButton* m_buttonSave;
    wxButton* m_buttonSaveAs;
    wxButton* m_buttonRun;
    wxTreeCtrl* m_treeCtrl;
    wxListCtrl* m_sqlListBox;
    wxTreeItemId m_rootItem;
    wxTreeItemId m_selectedItemID;
    int m_selectedReportID;
    wxString m_selectedGroup;
    wxArrayString m_reservedNames;
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

