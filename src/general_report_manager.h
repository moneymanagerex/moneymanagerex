/*******************************************************
 Copyright (C) 2011 Stefano Giorgio
 Copyright (C) 2014 Nikolay Akimov

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
#include "mmpanelbase.h"

#ifndef _WIN32
#include <sys/time.h>
#endif

class wxSQLite3Database;
class mmGeneralReportManager;

class sqlListCtrl : public mmListCtrl
{
    wxDECLARE_NO_COPY_CLASS(sqlListCtrl);
public:

    sqlListCtrl(mmGeneralReportManager *grm, wxWindow *parent
        , const wxWindowID id);
    virtual wxString OnGetItemText(long item, long column) const;
private:
    mmGeneralReportManager* m_grm;
};

class mmGeneralReportManager: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmGeneralReportManager);
    wxDECLARE_EVENT_TABLE();

public:
    /// Constructors
    mmGeneralReportManager( ) {}
    ~mmGeneralReportManager();

    mmGeneralReportManager(wxWindow* parent, wxSQLite3Database* db);
    wxString OnGetItemText(long item, long column) const;

private:
    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = _t("General Report Manager")
        , const wxString& name = "General Reports Manager"
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);
    /// Creates the controls and sizers
    void CreateControls();
    void fillControls();
    void OnImportReportEvt(wxCommandEvent& event);
    void importReport();
    bool openZipFile(const wxString &reportFileName
        , wxString &htt, wxString &sql, wxString &lua, wxString &readme);
    void OnUpdateReport(wxCommandEvent& event);
    void OnExportReport(wxCommandEvent& event);
    void OnRun(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnSqlTest(wxCommandEvent& event);
    void OnNewTemplate(wxCommandEvent& event);
    void OnItemRightClick(wxTreeEvent& event);
    void OnRightClick(wxMouseEvent& event);
    void OnSelChanged(wxTreeEvent& event);
    //void OnLabelChanged(wxTreeEvent& event);
    void viewControls(bool enable);
    void renameReport(int64 id);
    bool DeleteReport(int64 id);
    bool changeReportGroup(int64 id, bool ungroup);
    void changeReportState(int64 id);
    bool renameReportGroup(const wxString& GroupName);
    void OnMenuSelected(wxCommandEvent& event);
    void newReport(int sample = ID_NEW_EMPTY);
    void createEditorTab(wxNotebook* notebook, int type);
    void createOutputTab(wxNotebook* notebook, int type);
    void showHelp();

    bool getColumns(const wxString& sql, std::vector<std::pair<wxString, int> > &colHeaders);
    void getSqlTableInfo(std::vector<std::pair<wxString, wxArrayString>> &sqlTableInfo);
    bool getSqlQuery(/*in*/ wxString& sql, /*out*/ std::vector <std::vector <wxString> > &sqlQueryData, wxString& SqlError);
    const wxString getTemplate(wxString& sql);
    void OnNewWindow(wxWebViewEvent& evt);

    std::vector <std::vector <wxString> > m_sqlQueryData;

    wxSQLite3Database* m_db = nullptr;
    wxWebView* browser_ = nullptr;

    wxButton* m_buttonOpen = nullptr;
    wxButton* m_buttonSave = nullptr;
    wxButton* m_buttonSaveAs = nullptr;
    wxButton* m_buttonRun = nullptr;
    wxTreeCtrl* m_treeCtrl = nullptr;
    wxTreeCtrl* m_dbView = nullptr;
    sqlListCtrl* m_sqlListBox = nullptr;
    wxTreeItemId m_rootItem;
    wxTreeItemId m_selectedItemID;
    int64 m_selectedReportID = 0;
    wxString m_selectedGroup;

#if wxUSE_DRAG_AND_DROP
    void OnBeginDrag(wxTreeEvent& event);
#endif // wxUSE_DRAG_AND_DROP

    enum
    {
        ID_TAB_OUT = 0,
        ID_TAB_SQL,
        ID_TAB_LUA,
        ID_TAB_HTT,
        MAGIC_NUM = 16,
        ID_NEW_EMPTY = wxID_HIGHEST + 500,
        ID_NEW_SAMPLE_ASSETS,
        ID_NEW_SAMPLE_STOCKS,
        ID_NEW_SAMPLE_STATS,
        ID_DELETE,
        ID_RENAME,
        ID_GROUP,
        ID_UNGROUP,
        ID_NOTEBOOK,
        ID_TYPELABEL,
        ID_TEST,
        ID_SQL_CONTENT,
        ID_LUA_CONTENT,
        ID_TEMPLATE,
        ID_DESCRIPTION,
        ID_REPORT_LIST,
        ID_ACTIVE
    };

};

