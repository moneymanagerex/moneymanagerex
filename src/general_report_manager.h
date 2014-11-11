/*******************************************************
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
#include "mmpanelbase.h"

#ifndef _WIN32
#include <sys/time.h>
#endif

class wxStyledTextCtrl;
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
    virtual ~mmGeneralReportManager();

    mmGeneralReportManager(wxWindow* parent);
    wxString OnGetItemText(long item, long column) const;

private:
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
    bool openZipFile(const wxString &reportFileName
        , wxString &htt, wxString &sql, wxString &lua, wxString &readme, wxString &reportName);
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
    void newReport(int sample = ID_NEW_EMPTY);
    void createEditorTab(wxNotebook* notebook, int type);
    void createOutputTab(wxNotebook* notebook, int type);
    void showHelp();

    std::vector <std::vector <wxString> > m_sqlQueryData;

    wxWebView* m_outputHTML;

    wxButton* m_buttonOpen;
    wxButton* m_buttonSave;
    wxButton* m_buttonSaveAs;
    wxButton* m_buttonRun;
    wxTreeCtrl* m_treeCtrl;
    sqlListCtrl* m_sqlListBox;
    wxTreeItemId m_rootItem;
    wxTreeItemId m_selectedItemID;
    int m_selectedReportID;
    wxString m_selectedGroup;

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
        ID_WEB,
        ID_TYPELABEL,
        ID_TEST,
        ID_SQL_CONTENT,
        ID_LUA_CONTENT,
        ID_TEMPLATE,
        ID_DESCRIPTION,
        ID_SQL_GRID,
        ID_REPORT_LIST,
    };
    
};

static const wxString SAMPLE_ASSETS_LUA =
"local total_balance = 0\n"
"function handle_record(record)\n"
"\ttotal_balance = total_balance + record:get('VALUE');\n"
"end\n\n"
"function complete(result)\n"
"\tresult:set('ASSET_BALANCE', total_balance);\n"
"end\n";

static const wxString SAMPLE_ASSETS_SQL =
"SELECT STARTDATE, ASSETNAME, ASSETTYPE, VALUE, NOTES, VALUECHANGE, VALUECHANGERATE FROM ASSETS_V1;";

static const wxString SAMPLE_ASSETS_HTT = R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8" />
    <meta http - equiv = "Content-Type" content = "text/html" />
    <title><TMPL_VAR REPORTNAME></title>
    <script src = "sorttable.js"></script>
    <link href="master.css" rel="stylesheet">
</head>
<body>
<div class = "container">
<h3><TMPL_VAR REPORTNAME></h3>
<TMPL_VAR TODAY><hr>
<div class = "row">
<div class = "col-xs-2"></div>
<div class = "col-xs-8">
<table class="table sortable">
<thead>
    <tr>
        <th>STARTDATE</th>
        <th>ASSETNAME</th>
        <th>ASSETTYPE</th>
        <th>VALUE</th>
        <th>NOTES</th>
    </tr>
</thead>
<tbody>
    <TMPL_LOOP NAME=CONTENTS>
        <tr>
        <td><TMPL_VAR STARTDATE></td>
        <td><TMPL_VAR ASSETNAME></td>
        <td><TMPL_VAR ASSETTYPE></td>
        <td class="money" sorttable_customkey="<TMPL_VAR VALUE>"><TMPL_VAR VALUE></td>
        <td><TMPL_VAR NOTES></td>
    </tr>
    </TMPL_LOOP>
</tbody>
<tfoot>
    <tr class="total">
        <td colspan=3>Total Assets: </td>
        <td class="money"><TMPL_VAR ASSET_BALANCE></td>
        <td></td>
    </tr>
</tfoot>
</table>
</div></div></div>
<TMPL_LOOP ERRORS>
    <hr>    <TMPL_VAR ERROR>
</TMPL_LOOP>

</body>
<script>
    <!-- Format double to base currency -->
    function currency(n) {
        n = parseFloat(n);
        n =  isNaN(n) ? 0 : n.toFixed(2);
        var out = n.toString().replace(".", "|");
        out = out.toString().replace(/\B(?=(\d{3})+(?!\d))/g, "<TMPL_VAR GROUP_SEPARATOR>");
        out = out.replace("|", "<TMPL_VAR DECIMAL_POINT>");
        return out;
    }
    var elements= document.getElementsByClassName("money");
    for (var i = 0; i < elements.length; i++) {
        var element = elements[i];
        element.style.textAlign='right';
        if (element.innerHTML.indexOf("-") > -1) {
            element.style.color="#ff0000";
        } 
        element.innerHTML = '<TMPL_VAR PFX_SYMBOL>' + currency(element.innerHTML) +'<TMPL_VAR SFX_SYMBOL>';
    }
</script>
</html>
)";
