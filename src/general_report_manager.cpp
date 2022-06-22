/*******************************************************
 Copyright (C) 2011 Stefano Giorgio
 Copyright (C) 2014 -2017 Nikolay Akimov
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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

#include "general_report_manager.h"
#include "constants.h"
#include "minimal_editor.h"
#include "mmpanelbase.h"
#include "mmSimpleDialogs.h"
#include "images_list.h"
#include "paths.h"
#include "platfdep.h"
#include "util.h"
#include "option.h"
#include "reports/reportbase.h"

#include "model/Model_Infotable.h"
#include "model/Model_Report.h"

#include <memory>
#include <wx/richtooltip.h>
#include <wx/sstream.h>
#include <wx/zipstrm.h>
#include <wx/wxsqlite3.h>

static const wxString SAMPLE_ASSETS_LUA =
R"(local total_balance = 0
function handle_record(record)
    total_balance = total_balance + record:get('VALUE');
end

function complete(result)
    result:set('ASSET_BALANCE', total_balance);
end
)";

static const wxString SAMPLE_ASSETS_SQL =
R"(SELECT STARTDATE, ASSETNAME, ASSETTYPE, VALUE, NOTES, VALUECHANGE, VALUECHANGERATE FROM ASSETS_V1;)";

static const wxString SAMPLE_ASSETS_HTT =
R"(<!DOCTYPE html>
<html>
<head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8" />
    <title><TMPL_VAR REPORTNAME></title>
    <script src = "memory:sorttable.js"></script>
    <link href = "memory:master.css" rel="stylesheet">
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
            element.style.color="#f75e5e";
        }
        element.innerHTML = '<TMPL_VAR PFX_SYMBOL>' + currency(element.innerHTML) +'<TMPL_VAR SFX_SYMBOL>';
    }
</script>
</html>
)";

static const char *HTT_CONTEINER =
R"(<!DOCTYPE html>
<html>
<head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8" />
    <title><TMPL_VAR REPORTNAME></title>
    <script src = "memory:ChartNew.js"></script>
   <script>
        window.Promise || document.write('<script src="memory:polyfill.min.js"><\/script>');
        window.Promise || document.write('<script src="memory:classlist.min.js"><\/script>');
        window.Promise || document.write('<script src="memory:resize-observer.js"><\/script>');
        window.Promise || document.write('<script src="memory:findindex.min.js"><\/script>');
        window.Promise || document.write('<script src="memory:umd.min.js"><\/script>');
    </script>
    <script src = "memory:apexcharts.min.js"></script>
    <script src = "memory:sorttable.js"></script>
    <link href = "memory:master.css" rel = "stylesheet">
</head>
<body>
<div class = "container">
<h3><TMPL_VAR REPORTNAME></h3>
<TMPL_VAR TODAY><hr>
<div class = "row">
<div class = "col-xs-2"></div>
<div class = "col-xs-8">
<table class = "table">
<thead>
    <tr>
%s
    </tr>
</thead>
<tbody>
    <TMPL_LOOP NAME=CONTENTS>
        <tr>
%s
        </tr>
    </TMPL_LOOP>
</tbody>

</table>
</div>
</div>
%s
</div>
<TMPL_LOOP ERRORS>
    <TMPL_VAR ERROR>
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
            element.style.color="#f75e5e";
        }
        element.innerHTML = '<TMPL_VAR PFX_SYMBOL>' + currency(element.innerHTML) +'<TMPL_VAR SFX_SYMBOL>';
    }
</script>
</html>
)";

class MyTreeItemData : public wxTreeItemData
{
public:
    MyTreeItemData(int report_id, const wxString& selectedGroup) : m_report_id(report_id)
        , m_selectedGroup(selectedGroup) { }
    int get_report_id() const { return m_report_id; }
    const wxString get_group_name() const { return m_selectedGroup; }
private:
    int m_report_id;
    wxString m_selectedGroup;
};

wxIMPLEMENT_DYNAMIC_CLASS(mmGeneralReportManager, wxDialog);

wxBEGIN_EVENT_TABLE(mmGeneralReportManager, wxDialog)
    EVT_BUTTON(wxID_OPEN, mmGeneralReportManager::OnImportReportEvt)
    EVT_BUTTON(wxID_SAVE, mmGeneralReportManager::OnUpdateReport)
    EVT_BUTTON(wxID_SAVEAS, mmGeneralReportManager::OnExportReport)
    EVT_BUTTON(wxID_EXECUTE, mmGeneralReportManager::OnRun)
    EVT_BUTTON(wxID_CLOSE, mmGeneralReportManager::OnClose)
    EVT_BUTTON(ID_TEST, mmGeneralReportManager::OnSqlTest)
    EVT_BUTTON(wxID_NEW, mmGeneralReportManager::OnNewTemplate)
    //EVT_TREE_END_LABEL_EDIT(ID_REPORT_LIST, mmGeneralReportManager::OnLabelChanged)
    EVT_TREE_SEL_CHANGED(ID_REPORT_LIST, mmGeneralReportManager::OnSelChanged)
    EVT_TREE_ITEM_MENU(ID_REPORT_LIST, mmGeneralReportManager::OnItemRightClick)
    EVT_MENU(wxID_ANY, mmGeneralReportManager::OnMenuSelected)
wxEND_EVENT_TABLE()

sqlListCtrl::sqlListCtrl(mmGeneralReportManager* grm, wxWindow *parent, wxWindowID winid)
    : mmListCtrl(parent, winid)
    , m_grm(grm)
{
}

mmGeneralReportManager::mmGeneralReportManager(wxWindow* parent, wxSQLite3Database* db)
    : m_db(db)
    , browser_(nullptr)
    , m_buttonOpen(nullptr)
    , m_buttonSave(nullptr)
    , m_buttonSaveAs(nullptr)
    , m_buttonRun(nullptr)
    , m_treeCtrl(nullptr)
    , m_dbView(nullptr)
    , m_sqlListBox(nullptr)
    , m_selectedReportID(0)
{
    this->SetFont(parent->GetFont());
    Create(parent);
    mmSetSize(this);
    Centre();
}

mmGeneralReportManager::~mmGeneralReportManager()
{
    clearVFprintedFiles("grm");
    Model_Infotable::instance().Set("GRM_DIALOG_SIZE", GetSize());
}

bool mmGeneralReportManager::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxString& name
    , const wxPoint& pos
    , const wxSize& size
    , long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    wxAcceleratorEntry entries[2];
    entries[0].Set(wxACCEL_NORMAL, WXK_F9, wxID_EXECUTE);
    entries[1].Set(wxACCEL_CTRL, 'S', wxID_SAVE);
    wxAcceleratorTable accel(2, entries);
    SetAcceleratorTable(accel);

    Connect(wxID_EXECUTE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(mmGeneralReportManager::OnRun), nullptr, this);

    CreateControls();
    fillControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    SetIcon(mmex::getProgramIcon());
    Centre();
    return TRUE;
}

void mmGeneralReportManager::fillControls()
{
    DoWindowsFreezeThaw(this);
    viewControls(false);
    SetEvtHandlerEnabled(false);
    m_treeCtrl->DeleteAllItems();
    m_rootItem = m_treeCtrl->AddRoot(_("Reports"));
    m_selectedItemID = m_rootItem;
    m_treeCtrl->SetItemBold(m_rootItem, true);
    auto records = Model_Report::instance().all();
    std::sort(records.begin(), records.end(), SorterByREPORTNAME());
    std::stable_sort(records.begin(), records.end(), SorterByGROUPNAME());
    wxTreeItemId group;
    wxString group_name;
    for (const auto& record : records)
    {
        bool no_group = record.GROUPNAME.empty();
        if (group_name != record.GROUPNAME && !no_group)
        {
            group_name = record.GROUPNAME;
            group = m_treeCtrl->AppendItem(m_rootItem, group_name);
            m_treeCtrl->SetItemData(group, new MyTreeItemData(-1, group_name));
        }
        wxTreeItemId item = m_treeCtrl->AppendItem(no_group ? m_rootItem : group, record.REPORTNAME);
        m_treeCtrl->SetItemData(item, new MyTreeItemData(record.REPORTID, record.GROUPNAME));

        if (m_selectedReportID == record.REPORTID)
        {
            m_selectedItemID = item;
        }
    }
    m_treeCtrl->ExpandAll();
    m_treeCtrl->SelectItem(m_selectedItemID);
    SetEvtHandlerEnabled(true);
    m_treeCtrl->SetFocus();
    DoWindowsFreezeThaw(this);
    //Show help page or report detailes (bugs:#421)
    wxTreeEvent evt(wxEVT_TREE_SEL_CHANGED, ID_REPORT_LIST);
    evt.SetItem(m_selectedItemID);
    OnSelChanged(evt);
}

void mmGeneralReportManager::CreateControls()
{
    wxBoxSizer* main_box_sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(main_box_sizer);
    wxBoxSizer* top_sizer = new wxBoxSizer(wxHORIZONTAL);
    main_box_sizer->Add(top_sizer, 1, wxGROW | wxALL);
    wxSplitterWindow* splitter_main = new wxSplitterWindow(this, wxID_ANY);
    splitter_main->SetMinimumPaneSize(200);
    top_sizer->Add(splitter_main, 1, wxEXPAND, 0);

    /****************************************
     Navigation tree
     ***************************************/

    wxPanel* left_panel = new wxPanel(splitter_main, wxID_ANY);
    wxBoxSizer* left_sizer = new wxBoxSizer(wxVERTICAL);

    m_treeCtrl = new wxTreeCtrl(left_panel, ID_REPORT_LIST, wxDefaultPosition, wxDefaultSize,
        wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_NO_LINES | wxTR_TWIST_BUTTONS);
    mmThemeMetaColour(m_treeCtrl, meta::COLOR_NAVPANEL);
    mmThemeMetaColour(m_treeCtrl, meta::COLOR_NAVPANEL_FONT, true);
    left_sizer->Add(m_treeCtrl, g_flagsExpand);

    left_panel->SetSizer(left_sizer);
    left_panel->Fit();

    /****************************************
     Script Area
     ***************************************/
    // ListBox for source code
    wxPanel* right_panel = new wxPanel(splitter_main, wxID_ANY);
    wxBoxSizer* right_sizer = new wxBoxSizer(wxVERTICAL);

    wxNotebook* editors_notebook = new wxNotebook(right_panel
        , ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);
    right_sizer->Add(editors_notebook, g_flagsExpand);
    createOutputTab(editors_notebook, ID_TAB_OUT);
    right_panel->SetSizer(right_sizer);

    splitter_main->SplitVertically(left_panel, right_panel);

    /****************************************
     Separation Line
     ***************************************/
    wxStaticLine* static_line = new wxStaticLine(this, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    main_box_sizer->Add(static_line, 0, wxEXPAND | wxALL, 1);

    /****************************************
     Bottom Panel
     ***************************************/
    wxPanel* button_panel = new wxPanel(this, wxID_STATIC
        , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    main_box_sizer->Add(button_panel, wxSizerFlags(g_flagsV).Center());

    wxBoxSizer* buttonPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    button_panel->SetSizer(buttonPanelSizer);

    //
    m_buttonOpen = new wxButton(button_panel, wxID_OPEN, _("&Import"));
    buttonPanelSizer->Add(m_buttonOpen, g_flagsH);
    mmToolTip(m_buttonOpen, _("Locate and load a report file."));

    m_buttonSaveAs = new wxButton(button_panel, wxID_SAVEAS, _("&Export"));
    buttonPanelSizer->Add(m_buttonSaveAs, g_flagsH);
    mmToolTip(m_buttonSaveAs, _("Export the report to a new file."));
    buttonPanelSizer->AddSpacer(50);

    m_buttonSave = new wxButton(button_panel, wxID_SAVE, _("&Save "));
    buttonPanelSizer->Add(m_buttonSave, g_flagsH);
    mmToolTip(m_buttonSave, _("Save changes."));

    m_buttonRun = new wxButton(button_panel, wxID_EXECUTE, _("&Run"));
    buttonPanelSizer->Add(m_buttonRun, g_flagsH);
    mmToolTip(m_buttonRun, _("Run selected report."));

    wxButton* button_Close = new wxButton(button_panel, wxID_CLOSE, wxGetTranslation(g_CloseLabel));
    buttonPanelSizer->Add(button_Close, g_flagsH);
    //mmToolTip(button_Close, _("Save changes before closing. Changes without Save will be lost."));

}

void mmGeneralReportManager::createOutputTab(wxNotebook* editors_notebook, int type)
{
    //Output
    wxPanel* out_tab = new wxPanel(editors_notebook, wxID_ANY);
    editors_notebook->InsertPage(type, out_tab, _("Output"));
    wxBoxSizer* out_sizer = new wxBoxSizer(wxVERTICAL);
    out_tab->SetSizer(out_sizer);

    browser_ = wxWebView::New();
#ifdef __WXMAC__
    // With WKWebView handlers need to be registered before creation
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
    browser_->Create(out_tab, mmID_BROWSER);
#else
    browser_->Create(out_tab, mmID_BROWSER);
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));
#endif
    Bind(wxEVT_WEBVIEW_NEWWINDOW, &mmGeneralReportManager::OnNewWindow, this, mmID_BROWSER);

     out_sizer->Add(browser_, g_flagsExpand);
     out_tab->SetSizerAndFit(out_sizer);
}

void mmGeneralReportManager::createEditorTab(wxNotebook* editors_notebook, int type)
{
    wxString label;
    switch (type) {
    case ID_SQL_CONTENT: label = _("SQL"); break;
    case ID_LUA_CONTENT: label = _("Lua");  break;
    case ID_TEMPLATE: label = _("Template");  break;
    case ID_DESCRIPTION: label = _("Description"); break;
    //default: ;
    }
    if (FindWindow(type + MAGIC_NUM)) return;

    int tabID = editors_notebook->GetRowCount();
    wxPanel* panel = new wxPanel(editors_notebook, type + MAGIC_NUM);
    editors_notebook->InsertPage(tabID, panel, label);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    if (type == ID_SQL_CONTENT)
    {
        wxBoxSizer *sizermain = new wxBoxSizer(wxVERTICAL);
        wxSplitterWindow *splittermain = new wxSplitterWindow(panel, wxID_ANY);
        splittermain->SetMinimumPaneSize(50); 
        sizermain->Add(splittermain, 1, wxEXPAND,0 );

        wxPanel *pnl1 = new wxPanel(splittermain, wxID_ANY);
        wxBoxSizer *bSizerp1 = new wxBoxSizer(wxVERTICAL);
        pnl1->SetSizer(bSizerp1);

        wxSplitterWindow *splitter_sql = new wxSplitterWindow(pnl1, wxID_ANY);
        splitter_sql->SetSashGravity(0.9);
        splitter_sql->SetMinimumPaneSize(150); // Smallest size of panels
        bSizerp1->Add(splitter_sql, g_flagsExpand);

        MinimalEditor* templateText = new MinimalEditor(splitter_sql, type);

#if defined (__WXMSW__)
        long treeCtrlFlags = wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_ROW_LINES;
#else
        long treeCtrlFlags = wxTR_SINGLE | wxTR_HAS_BUTTONS;
#endif
        m_dbView = new wxTreeCtrl(splitter_sql, wxID_ANY, wxDefaultPosition
            , wxDefaultSize, treeCtrlFlags);
        mmThemeMetaColour(m_dbView, meta::COLOR_NAVPANEL);
        mmThemeMetaColour(m_dbView, meta::COLOR_NAVPANEL_FONT, true);

        splitter_sql->SplitVertically(templateText, m_dbView);
        splitter_sql->SetSashPosition(500);

#if wxUSE_DRAG_AND_DROP
        m_dbView->Connect(wxID_ANY, wxEVT_TREE_BEGIN_DRAG
            , wxTreeEventHandler(mmGeneralReportManager::OnBeginDrag)
            , nullptr, this);
#endif // wxUSE_DRAG_AND_DROP

        wxPanel *pnl2 = new wxPanel(splittermain, wxID_ANY);
        wxBoxSizer* bSizerp2 = new wxBoxSizer(wxVERTICAL);
        pnl2->SetSizer(bSizerp2);

        wxBoxSizer *box_sizer2 = new wxBoxSizer(wxHORIZONTAL);
        wxButton* buttonPlay = new wxButton(pnl2, ID_TEST, _("&Test"));
        wxButton* buttonNewTemplate = new wxButton(pnl2, wxID_NEW, _("Create Template"));
        wxStaticText *info = new wxStaticText(pnl2, wxID_INFO, "");
        buttonNewTemplate->Enable(false);
        box_sizer2->Add(buttonPlay);
        box_sizer2->AddSpacer(10);
        box_sizer2->Add(buttonNewTemplate);
        box_sizer2->AddSpacer(10);
        box_sizer2->Add(info, g_flagsExpand);

        m_sqlListBox = new sqlListCtrl(this, pnl2, wxID_ANY);
        bSizerp2->Add(box_sizer2);
        bSizerp2->Add(m_sqlListBox, g_flagsExpand);
        bSizerp2->SetMinSize(wxSize(-1, 100));

        // Populate database view
        std::vector<std::pair<wxString, wxArrayString>> sqlTableInfo;
        this->getSqlTableInfo(sqlTableInfo);
        wxTreeItemId root_id = m_dbView->AddRoot("Tables");
        for (const auto& t : sqlTableInfo)
        {
            wxTreeItemId id = m_dbView->AppendItem(root_id, t.first);
            for (const auto& c : t.second)
                m_dbView->AppendItem(id, c);
        }
        m_dbView->Expand(root_id);

        sizer->Add(sizermain, g_flagsExpand);
        splittermain->SplitHorizontally(pnl1, pnl2);
    }
    else
    {
        MinimalEditor* editorText = new MinimalEditor(panel, type);
        sizer->Add(editorText, g_flagsExpand);
    }

    panel->SetSizer(sizer);
    panel->Layout();
}

void mmGeneralReportManager::OnSqlTest(wxCommandEvent& WXUNUSED(event))
{
    MinimalEditor* sqlText = wxDynamicCast(FindWindow(ID_SQL_CONTENT), MinimalEditor);
    wxStaticText* info = wxDynamicCast(FindWindow(wxID_INFO), wxStaticText);
    
    wxString sql = sqlText->GetStringSelection().empty() ? sqlText->GetValue() : sqlText->GetStringSelection();

    wxString SqlError;
    wxLongLong interval = wxGetUTCTimeMillis();
    if (this->getSqlQuery(sql, m_sqlQueryData, SqlError))
    {
        m_sqlListBox->DeleteAllColumns();
        interval = wxGetUTCTimeMillis() - interval;
        info->SetLabelText(wxString::Format(wxPLURAL("%zu row returned, duration: %lld ms",
                                                     "%zu rows returned, duration: %lld ms",
                                                     m_sqlQueryData.size())
            , m_sqlQueryData.size(), interval));

        MinimalEditor* templateText = static_cast<MinimalEditor*>(FindWindow(ID_TEMPLATE));
        std::vector<std::pair<wxString, int> > colHeaders;
        bool colsOK = this->getColumns(sql, colHeaders);
        wxButton* b = static_cast<wxButton*>(FindWindow(wxID_NEW));
        b->Enable(colsOK && templateText->IsEmpty());
        int pos = 0;
        for (const auto& col : colHeaders)
        {
            m_sqlListBox->InsertColumn(pos++, col.first
                , (col.second == WXSQLITE_INTEGER || col.second == WXSQLITE_FLOAT)
                ? wxLIST_FORMAT_RIGHT : wxLIST_FORMAT_LEFT
                , col.first.length() * 10 + 20);
        }

        m_sqlListBox->SetItemCount(m_sqlQueryData.size());
        m_sqlListBox->Refresh();
        m_sqlListBox->Update();
        if (m_sqlQueryData.size() > 0)
            m_sqlListBox->EnsureVisible(0);
    }
    else
    {
        info->SetLabelText(_("SQL Syntax Error") + " (" + SqlError + ")");
    }
}

void mmGeneralReportManager::OnNewTemplate(wxCommandEvent& WXUNUSED(event))
{
    MinimalEditor* templateText = static_cast<MinimalEditor*>(FindWindow(ID_TEMPLATE));
    if (!templateText->IsEmpty()) return;
    MinimalEditor* sqlText = static_cast<MinimalEditor*>(FindWindow(ID_SQL_CONTENT));

    wxNotebook* n = static_cast<wxNotebook*>(FindWindow(ID_NOTEBOOK));
    n->SetSelection(ID_TAB_HTT);

    wxString sql_txt = sqlText->GetValue();
    const wxString tmplt = getTemplate(sql_txt);
    templateText->ChangeValue(tmplt);

    wxButton* b = static_cast<wxButton*>(FindWindow(wxID_NEW));
    b->Enable(false);

    wxCommandEvent evt;
    OnUpdateReport(evt);
}

void mmGeneralReportManager::OnImportReportEvt(wxCommandEvent& WXUNUSED(event))
{
    importReport();
}

void mmGeneralReportManager::importReport()
{
    const wxString reportFileName = wxFileSelector(_("Load report file:")
        , mmex::getPathResource(mmex::REPORTS), wxEmptyString, wxEmptyString
        , _("General Report Manager files (*.grm)")+"|*.grm|"+_("ZIP files (*.zip)")+"|*.zip"
        , wxFD_FILE_MUST_EXIST);

    if (reportFileName.empty()) return;

    wxFileName fn(reportFileName);
    wxString sql, lua, htt, txt, reportName;
    openZipFile(reportFileName, htt, sql, lua, txt);

    reportName = fn.FileName(reportFileName).GetName();
    Model_Report::Data *report = Model_Report::instance().get(reportName);

    if (!report) report = Model_Report::instance().create();
    report->GROUPNAME = m_selectedGroup;
    report->REPORTNAME = reportName;
    report->SQLCONTENT = sql;
    report->LUACONTENT = lua;
    report->TEMPLATECONTENT = htt;
    report->DESCRIPTION = txt;
    m_selectedReportID = Model_Report::instance().save(report);

    fillControls();
}

bool mmGeneralReportManager::openZipFile(const wxString &reportFileName
    , wxString &htt, wxString &sql, wxString &lua, wxString &txt)
{
    if (!reportFileName.empty())
    {
        wxTextFile reportFile(reportFileName);
        if (reportFile.Open())
        {
            std::unique_ptr<wxZipEntry> entry;
            wxFFileInputStream in(reportFileName);
            wxZipInputStream zip(in);
            while (entry.reset(zip.GetNextEntry()), entry.get() != nullptr)
            {
                // access meta-data
                const wxString f = entry->GetName();
                // read 'zip' to access the entry's data
                zip.OpenEntry(*entry.get());
                if (!zip.CanRead())
                {
                    wxLogError("Can not read zip entry '" + f + "'.");
                    return false;
                }

                wxString textdata;
                wxStringOutputStream out_stream(&textdata);
                zip.Read(out_stream);

                if (f.EndsWith(".sql"))
                    sql = textdata;
                else if (f.EndsWith(".lua"))
                    lua = textdata;
                else if (f.EndsWith(".txt") || f.EndsWith(".html"))
                    txt << textdata;
                else if (f.EndsWith(".htt"))
                    htt << textdata;
            }
        }
        else
        {
            wxString msg = wxString::Format(_("Unable to open file:\n%s\n\n"), reportFileName);
            wxMessageBox(msg, _("General Reports Manager"), wxOK | wxICON_ERROR);
            return false;
        }
    }
    return true;
}
void mmGeneralReportManager::OnUpdateReport(wxCommandEvent& WXUNUSED(event))
{
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(m_treeCtrl->GetItemData(m_selectedItemID));
    if (!iData) return;

    int id = iData->get_report_id();
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        MinimalEditor* templateText = static_cast<MinimalEditor*>(FindWindow(ID_TEMPLATE));
        MinimalEditor* SqlScriptText = static_cast<MinimalEditor*>(FindWindow(ID_SQL_CONTENT));
        MinimalEditor* LuaScriptText = static_cast<MinimalEditor*>(FindWindow(ID_LUA_CONTENT));
        MinimalEditor* descriptionText = static_cast<MinimalEditor*>(FindWindow(ID_DESCRIPTION));
        report->SQLCONTENT = SqlScriptText->GetValue();
        report->LUACONTENT = LuaScriptText->GetValue();
        report->TEMPLATECONTENT = templateText->GetValue();
        report->DESCRIPTION = descriptionText->GetValue();

        Model_Report::instance().save(report);
        browser_->SetPage(report->DESCRIPTION, "");
    }
}

void mmGeneralReportManager::OnRun(wxCommandEvent& WXUNUSED(event))
{
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(m_treeCtrl->GetItemData(m_selectedItemID));
    if (!iData) return;

    int id = iData->get_report_id();
    m_selectedGroup = iData->get_group_name();
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        wxNotebook* n = static_cast<wxNotebook*>(FindWindow(ID_NOTEBOOK));
        n->SetSelection(ID_TAB_OUT);
        browser_->ClearBackground();

        mmGeneralReport gr(report); //TODO: limit 500 line
        auto data = gr.getHTMLText();
        const auto& name = getVFname4print("grm", data);
        browser_->LoadURL(name);
    }
}

void mmGeneralReportManager::OnItemRightClick(wxTreeEvent& event)
{
    wxTreeItemId id = event.GetItem();
    m_treeCtrl ->SelectItem(id);
    int report_id = -1;
    MyTreeItemData *iData = dynamic_cast<MyTreeItemData*>(m_treeCtrl->GetItemData(id));
    if (iData) report_id = iData->get_report_id();
    Model_Report::Data *report = Model_Report::instance().get(report_id);

    wxMenu* samplesMenu = new wxMenu;
    samplesMenu->Append(ID_NEW_SAMPLE_ASSETS, _("Assets"));

    wxMenu customReportMenu;
    customReportMenu.Append(ID_NEW_EMPTY, _("New Empty Report"));
    customReportMenu.Append(wxID_ANY, _("New Sample Report"), samplesMenu);
    customReportMenu.AppendSeparator();
    if (report)
        customReportMenu.Append(ID_GROUP, _("Change Group"));
    else
        customReportMenu.Append(ID_GROUP, _("Rename Group"));
    customReportMenu.Append(ID_UNGROUP, _("UnGroup"));
    customReportMenu.Append(ID_RENAME, _("Rename Report"));
    customReportMenu.AppendSeparator();
    customReportMenu.Append(ID_DELETE, _("Delete Report"));

    if (report)
    {
        customReportMenu.Enable(ID_UNGROUP, !report->GROUPNAME.empty());
    }
    else
    {
        if (m_selectedGroup == "")
            customReportMenu.Enable(ID_GROUP, false);

        customReportMenu.Enable(ID_UNGROUP, false);
        customReportMenu.Enable(ID_RENAME, false);
        customReportMenu.Enable(ID_DELETE, false);
    }
    PopupMenu(&customReportMenu);
}

void mmGeneralReportManager::viewControls(bool enable)
{
    m_buttonRun->Enable(enable);
    m_buttonSaveAs->Enable(enable);
    m_buttonSave->Enable(enable);
    if (!enable)
    {
        m_selectedGroup = "";
    }
}

void mmGeneralReportManager::OnSelChanged(wxTreeEvent& event)
{
    viewControls(false);
    m_selectedItemID = event.GetItem();
    if (!m_selectedItemID) return;

    wxNotebook* editors_notebook = static_cast<wxNotebook*>(FindWindow(ID_NOTEBOOK));
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(m_treeCtrl->GetItemData(m_selectedItemID));
    if (!iData)
    {
        for (size_t n = editors_notebook->GetPageCount() - 1; n >= 1; n--)
            editors_notebook->DeletePage(n);
        showHelp();
        return;
    }

    int id = iData->get_report_id();
    m_selectedGroup = iData->get_group_name();
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        m_selectedReportID = report->REPORTID;
        createEditorTab(editors_notebook, ID_DESCRIPTION);
        createEditorTab(editors_notebook, ID_TEMPLATE);
        createEditorTab(editors_notebook, ID_LUA_CONTENT);
        createEditorTab(editors_notebook, ID_SQL_CONTENT);

        MinimalEditor* SqlScriptText = static_cast<MinimalEditor*>(FindWindow(ID_SQL_CONTENT));
        MinimalEditor* LuaScriptText = static_cast<MinimalEditor*>(FindWindow(ID_LUA_CONTENT));
        MinimalEditor* templateText = static_cast<MinimalEditor*>(FindWindow(ID_TEMPLATE));
        MinimalEditor* descriptionText = static_cast<MinimalEditor*>(FindWindow(ID_DESCRIPTION));

        templateText->ChangeValue(report->TEMPLATECONTENT);
        templateText->SetLexerHtml();
        SqlScriptText->ChangeValue(report->SQLCONTENT);
        SqlScriptText->SetLexerSql();
        LuaScriptText->ChangeValue(report->LUACONTENT);
        LuaScriptText->SetLexerLua();
        wxString description = report->DESCRIPTION;
        descriptionText->ChangeValue(description);
        if (!description.Contains("<!DOCTYPE html"))
            description.Replace("\n", "<BR>\n");

        browser_->SetPage(description, "");

        if (m_sqlListBox) m_sqlListBox->DeleteAllItems();
        if (m_sqlListBox) m_sqlListBox->DeleteAllColumns();
        wxButton* createTemplate = static_cast<wxButton*>(FindWindow(wxID_NEW));
        if (createTemplate) createTemplate->Enable(false);
        wxStaticText *info = static_cast<wxStaticText*>(FindWindow(wxID_INFO));
        if (info) info->SetLabelText("");

        viewControls(true);
    }
}

/*void mmGeneralReportManager::OnLabelChanged(wxTreeEvent& event)
{
    event.Veto();
}*/

void mmGeneralReportManager::renameReport(int id)
{
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        wxString label = wxGetTextFromUser(_("Enter the name for the report")
            , _("General Report Manager"), report->REPORTNAME);
        label.Trim();

        if (Model_Report::instance().find(Model_Report::REPORTNAME(label)).empty()
            && !label.empty())
        {
            report->REPORTNAME = label;
            Model_Report::instance().save(report);
            fillControls();
        }
    }
}

bool mmGeneralReportManager::DeleteReport(int id)
{
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        wxString msg = wxString() << _("Delete the Report Title:")
            << "\n\n"
            << report->REPORTNAME;
        int iError = wxMessageBox(msg, "General Reports Manager", wxYES_NO | wxICON_ERROR);
        if (iError == wxYES)
        {
            Model_Report::instance().remove(id);
            m_selectedReportID = -1;
            m_selectedItemID = m_rootItem;
            fillControls();
            return true;
        }
    }
    return false;
}

bool mmGeneralReportManager::changeReportGroup(int id, bool ungroup)
{
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        if (ungroup)
        {
            report->GROUPNAME = "";
            Model_Report::instance().save(report);
            return true;
        }
        else
        {
            mmDialogComboBoxAutocomplete dlg(this, _("Enter or choose name for the new report group"),
                _("Change report group"), report->GROUPNAME, Model_Report::instance().allGroupNames());

            if (dlg.ShowModal() == wxID_OK)
            {
                const wxString groupName = dlg.getText();
                report->GROUPNAME = groupName;
                Model_Report::instance().save(report);
                return true;
            }
        }
    }
    return false;
}

bool mmGeneralReportManager::renameReportGroup(const wxString& GroupName)
{
    mmDialogComboBoxAutocomplete dlg(this, _("Enter or choose name for the new group"),
        _("Rename Group"), GroupName, Model_Report::instance().allGroupNames());

    if (dlg.ShowModal() == wxID_OK)
    {
        const wxString groupName = dlg.getText();
        auto reports = Model_Report::instance().find(Model_Report::GROUPNAME(GroupName));
        for (auto &report : reports)
        {
            report.GROUPNAME = groupName;
        }
        Model_Report::instance().save(reports);
        return true;
    }
    return false;
}

void mmGeneralReportManager::OnMenuSelected(wxCommandEvent& event)
{
    int id = event.GetId();

    if (id == ID_NEW_EMPTY || id == ID_NEW_SAMPLE_ASSETS)
    {
        newReport(id);
        fillControls();
        return;
    }

    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(m_treeCtrl->GetItemData(m_selectedItemID));
    if (iData)
    {
        int report_id = iData->get_report_id();

        if (report_id > -1)
        {
            switch (id){
            case ID_RENAME:
                this->renameReport(report_id);
                break;
            case ID_DELETE:
                this->DeleteReport(report_id);
                break;
            case ID_GROUP:
                this->changeReportGroup(report_id, false);
                break;
            case ID_UNGROUP:
                this->changeReportGroup(report_id, true);
                break;
            }
        }
        else if (id == ID_GROUP)
        {
            this->renameReportGroup(m_selectedGroup);
        }
    }

    fillControls();
}

void mmGeneralReportManager::newReport(int sample)
{
    wxString group_name;
    if (m_selectedItemID == m_rootItem)
    {
        mmDialogComboBoxAutocomplete dlg(this, _("Enter or choose name for the new report group")
            , _("Add Report Group"), "", Model_Report::instance().allGroupNames());
        if (dlg.ShowModal() == wxID_OK)
            group_name = dlg.getText();
        else
            return;
    }
    else
    {
        group_name = m_selectedGroup;
    }


    const wxDateTime now = wxDateTime::Now();
    wxString report_name = wxString::Format(_("New Report %s"), now.Format("%Y%m%d%H%M%S"));

    int max_attempts = 3;
    for (int i = 0; i < max_attempts; i++)
    {
        report_name = wxGetTextFromUser(_("Enter the name for the report")
            , _("General Report Manager"), report_name);

        if (report_name.empty())
            return; //Canceled by user
        if (!report_name.empty() && Model_Report::instance().find(Model_Report::REPORTNAME(report_name)).empty())
            break;
        if (i == max_attempts - 1)
            return mmErrorDialogs::MessageError(this, _("Report Name already exists"), _("New Report"));
    }

    wxString sqlContent, luaContent, httContent, description;
    switch (sample) {
    case ID_NEW_EMPTY:
        sqlContent = ""; luaContent = ""; httContent = "";
        description = _("New Report");
        break;
    case ID_NEW_SAMPLE_ASSETS:
        sqlContent = SAMPLE_ASSETS_SQL;
        luaContent = SAMPLE_ASSETS_LUA;
        httContent = SAMPLE_ASSETS_HTT;
        description = _("Assets");
        break;
    }

    Model_Report::Data* report = Model_Report::instance().create();
    report->GROUPNAME = group_name;
    report->REPORTNAME = report_name;
    report->SQLCONTENT = sqlContent;
    report->LUACONTENT = luaContent;
    report->TEMPLATECONTENT = httContent;
    report->DESCRIPTION = description;
    m_selectedReportID = Model_Report::instance().save(report);
}

void mmGeneralReportManager::OnExportReport(wxCommandEvent& WXUNUSED(event))
{
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(m_treeCtrl->GetItemData(m_selectedItemID));
    if (!iData) return;

    int id = iData->get_report_id();
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        wxString file_name = report->REPORTNAME + ".grm";
        wxFileDialog dlg(this
            , _("Choose file to Save As Report")
            , wxEmptyString
            , file_name
            , _("General Report Manager files (*.grm)")+"|*.grm|"+_("ZIP files (*.zip)")+"|*.zip"
            , wxFD_SAVE | wxFD_OVERWRITE_PROMPT
            );

        if (dlg.ShowModal() != wxID_OK)
            return;

        file_name = dlg.GetPath();

        wxFFileOutputStream out(file_name);
        wxZipOutputStream zip(out);
        wxTextOutputStream txt(zip, wxEOL_UNIX);
        zip.PutNextEntry("sqlcontent.sql");
        txt << report->SQLCONTENT;
        zip.PutNextEntry("luacontent.lua");
        txt << report->LUACONTENT;
        zip.PutNextEntry("template.htt");
        txt << report->TEMPLATECONTENT;
        zip.PutNextEntry("description.txt");
        txt << report->DESCRIPTION;
    }
}

void mmGeneralReportManager::showHelp()
{
    const auto url = mmex::getPathDoc(mmex::HTML_CUSTOM_SQL);
    browser_->LoadURL(url);
}

wxString mmGeneralReportManager::OnGetItemText(long item, long column) const
{
    return m_sqlQueryData.at(item).at(column);
}

wxString sqlListCtrl::OnGetItemText(long item, long column) const
{
    return m_grm->OnGetItemText(item, column);
}

void mmGeneralReportManager::OnClose(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_OK);
}

bool mmGeneralReportManager::getColumns(const wxString& sql, std::vector<std::pair<wxString, int> > &colHeaders)
{
    wxSQLite3Statement stmt;
    wxSQLite3ResultSet q;
    int columnCount = 0;
    try
    {
        stmt = this->m_db->PrepareStatement(sql);
        if (!stmt.IsReadOnly())
            return false;
        q = stmt.ExecuteQuery();
        columnCount = q.GetColumnCount();
    }
    catch (const wxSQLite3Exception& /*e*/)
    {
        return false;
    }

    for (int i = 0; i < columnCount; ++i)
    {
        std::pair<wxString, int> col_and_type;
        col_and_type.second = q.GetColumnType(i);
        col_and_type.first = q.GetColumnName(i);

        colHeaders.push_back(col_and_type);
    }
    return true;
}

void mmGeneralReportManager::getSqlTableInfo(std::vector<std::pair<wxString, wxArrayString>> &sqlTableInfo)
{
    const wxString sqlTables = "SELECT type, name FROM sqlite_master WHERE type = 'table' or type = 'view' ORDER BY type, name";
    sqlTableInfo.clear();

    // Get a list of the database tables
    wxSQLite3ResultSet qTables;
    try
    {
        qTables = this->m_db->ExecuteQuery(sqlTables);
        while (qTables.NextRow())
        {
            const wxString table_name = qTables.GetAsString(1);

            // Get a list of the table columns
            const wxString& sql = wxString::Format("PRAGMA table_info(%s);", table_name);
            wxSQLite3ResultSet qColumns;
            try
            {
                qColumns = this->m_db->ExecuteQuery(sql);
                wxArrayString column_names;
                while (qColumns.NextRow())
                    column_names.push_back(qColumns.GetAsString(1));

                sqlTableInfo.push_back(std::make_pair(table_name, column_names));
            }
            catch (const wxSQLite3Exception &e)
            {
                wxLogError("%s\n Exception \n%s", sql, e.GetMessage().utf8_str());
            }
        }
    }
    catch (const wxSQLite3Exception &e)
    {
        wxLogError("SQL Exception: \n%s", e.GetMessage().utf8_str());
    }
}

bool mmGeneralReportManager::getSqlQuery(/*in*/ wxString& sql
    , /*out*/ std::vector <std::vector <wxString> > &sqlQueryData
    , wxString &SqlError)
{
    wxSQLite3ResultSet q;
    int columnCount = 0;
    try
    {
        std::map <wxString, wxString> rep_params;
        Model_Report::PrepareSQL(sql, rep_params);
        wxSQLite3Statement stmt = this->m_db->PrepareStatement(sql);
        if (!stmt.IsReadOnly())
            return false;
        q = stmt.ExecuteQuery();
        columnCount = q.GetColumnCount();
    }
    catch (const wxSQLite3Exception& e)
    {
        SqlError = e.GetMessage();
        SqlError.Replace(" or missing database[1]:", "");
        return false;
    }

    sqlQueryData.clear();
    while (q.NextRow())
    {
        std::vector<wxString> row;
        for (int i = 0; i < columnCount; ++i)
            row.push_back(q.GetAsString(i));
        sqlQueryData.push_back(row);
    }
    return true;
}

const wxString mmGeneralReportManager::getTemplate(wxString& sql)
{
    wxString body, header, SqlError;

    std::map <wxString, wxString> rep_params;
    try
    {
        Model_Report::PrepareSQL(sql, rep_params);
    }
    catch (const wxSQLite3Exception& e)
    {
        SqlError = e.GetMessage();
        SqlError.Replace(" or missing database[1]:", "");
        return SqlError;
    }

    std::vector<std::pair<wxString, int> > colHeaders;

    this->getColumns(sql, colHeaders);
    for (const auto& col : colHeaders)
    {
        header += wxString::Format("        <th>%s</th>\n", col.first);
        if (col.second == WXSQLITE_FLOAT)
            body += wxString::Format("        <td class = \"money\"><TMPL_VAR \"%s\"></td>\n", col.first);
        else if (col.second == WXSQLITE_INTEGER)
            body += wxString::Format("        <td class = \"text-right\"><TMPL_VAR \"%s\"></td>\n", col.first);
        else
            body += wxString::Format("        <td><TMPL_VAR \"%s\"></td>\n", col.first);
    }

    wxString params = rep_params.empty() ? "" : _("Parameters:");
    for (const auto& entry : rep_params)
    {
        for (const auto & item : Model_Report::getParamNames()) {
            if (entry.first == item.first.Mid(1)) {
                params += wxString::Format("<BR> %s <TMPL_VAR %s>", item.second, entry.first);
            }
        }
    }
    return wxString::Format(HTT_CONTEINER, header, body, params);
}

#if wxUSE_DRAG_AND_DROP
void mmGeneralReportManager::OnBeginDrag(wxTreeEvent& (event))
{
    wxTreeItemId selectedItem = event.GetItem();
    if (!selectedItem) return;

    wxTextDataObject data;
    data.SetText(m_dbView->GetItemText(selectedItem));
    MinimalEditor* templateText = static_cast<MinimalEditor*>(FindWindow(ID_TEMPLATE));
    wxDropSource dragSource(templateText);
    dragSource.SetData(data);
    dragSource.DoDragDrop();
}
#endif // wxUSE_DRAG_AND_DROP

void mmGeneralReportManager::OnNewWindow(wxWebViewEvent& evt)
{
    const wxString uri = evt.GetURL();

    wxRegEx pattern(R"(^(https?:)|(file:)\/\/)");
    if (pattern.Matches(uri))
    {
        wxLaunchDefaultBrowser(uri);
    }

    evt.Skip();
}
