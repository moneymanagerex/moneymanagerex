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

#include "general_report_manager.h"
#include "minimal_editor.h"
#include "util.h"
#include "paths.h"
#include "constants.h"
#include "mmpanelbase.h"
#include "model/Model_Infotable.h"
#include "model/Model_Report.h"
#include <wx/zipstrm.h>
#include <memory>
#include <wx/richtooltip.h>

#ifndef _WIN32
    #include <sys/time.h>

    unsigned GetTickCount()
    {
            struct timeval tv;
            if(gettimeofday(&tv, NULL) != 0)
                return 0;

            return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
    }
#endif

int titleTextWidth   = 200; // Determines width of Headings Textbox.
int sourceTextHeight = 200; // Determines height of Source Textbox.

IMPLEMENT_DYNAMIC_CLASS( mmGeneralReportManager, wxDialog )

BEGIN_EVENT_TABLE(mmGeneralReportManager, wxDialog)
    EVT_BUTTON(wxID_OPEN, mmGeneralReportManager::OnImportReportEvt)
    EVT_BUTTON(wxID_SAVE, mmGeneralReportManager::OnUpdateReport)
    EVT_BUTTON(wxID_SAVEAS, mmGeneralReportManager::OnExportReport)
    EVT_BUTTON(wxID_EXECUTE, mmGeneralReportManager::OnRun)
    EVT_BUTTON(wxID_CLOSE, mmGeneralReportManager::OnClose)
    EVT_BUTTON(ID_TEST, mmGeneralReportManager::OnSqlTest)
    EVT_BUTTON(wxID_NEW, mmGeneralReportManager::OnNewTemplate)
    //EVT_TREE_END_LABEL_EDIT(wxID_ANY, mmGeneralReportManager::OnLabelChanged)
    EVT_TREE_SEL_CHANGED(wxID_ANY, mmGeneralReportManager::OnSelChanged)
    EVT_TREE_ITEM_MENU(wxID_ANY, mmGeneralReportManager::OnItemRightClick)
    EVT_MENU(wxID_ANY, mmGeneralReportManager::OnMenuSelected)
END_EVENT_TABLE()

mmGeneralReportManager::mmGeneralReportManager(wxWindow* parent)
    : m_buttonOpen()
    , m_buttonSave()
    , m_buttonSaveAs()
    , m_buttonRun()
    , m_treeCtrl()
    , m_outputHTML()
    , m_sqlListBox()
    , m_selectedReportID(0)
{
    // list of reserved names from mmGUIFrame::OnSelChanged()
    m_reservedNames.Add("Home Page");
    m_reservedNames.Add("Help");
    m_reservedNames.Add("Stocks");
    m_reservedNames.Add("Budgeting");
    m_reservedNames.Add("Assets");
    m_reservedNames.Add("Bills & Deposits");
    m_reservedNames.Add("Transaction Report");

    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("General Reports Manager"), wxDefaultPosition, wxDefaultSize, style);
    SetClientSize(wxSize(940, 576));
}

mmGeneralReportManager::~mmGeneralReportManager()
{
}

bool mmGeneralReportManager::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    wxAcceleratorEntry entries[2];
    entries[0].Set(wxACCEL_NORMAL, WXK_F9, wxID_EXECUTE);
    entries[1].Set(wxACCEL_CTRL, 'S', wxID_SAVE);
    wxAcceleratorTable accel(2, entries);
    SetAcceleratorTable(accel);

    Connect(wxID_EXECUTE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(mmGeneralReportManager::OnRun), NULL, this);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());
    Centre();

    fillControls();
    return TRUE;
}

void mmGeneralReportManager::fillControls()
{
    viewControls(false);
    //m_treeCtrl->SetEvtHandlerEnabled(false);
    m_treeCtrl->DeleteAllItems();
    m_rootItem = m_treeCtrl->AddRoot(_("Reports"));
    m_selectedItemID = m_rootItem;
    m_treeCtrl->SetItemBold(m_rootItem, true);
    m_treeCtrl->SetFocus();
    Model_Report::Data_Set records
        = Model_Report::instance().all(Model_Report::COL_REPORTNAME);
    wxTreeItemId group;
    wxString group_name;
    for (const auto& record : records)
    {
        bool no_group = record.GROUPNAME.empty();
        if (group_name != record.GROUPNAME && !no_group)
        {
            group = m_treeCtrl->AppendItem(m_rootItem, record.GROUPNAME);
            m_treeCtrl->SetItemData(group, new MyTreeItemData(-1, record.GROUPNAME));
            group_name = record.GROUPNAME;
        }
        wxTreeItemId item = m_treeCtrl->AppendItem(no_group ? m_rootItem : group, record.REPORTNAME);
        m_treeCtrl->SetItemData(item, new MyTreeItemData(record.REPORTID, record.GROUPNAME));

        if (m_selectedReportID == record.REPORTID)
        {
            m_selectedItemID = item;
            m_treeCtrl->SelectItem(m_selectedItemID);
        }
    }
    m_treeCtrl->ExpandAll();
    //m_treeCtrl->SetEvtHandlerEnabled(true);
}

void mmGeneralReportManager::CreateControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainBoxSizer);

    /****************************************
     Parameters Area
     ***************************************/

    wxBoxSizer* headingPanelSizerH = new wxBoxSizer(wxHORIZONTAL);
    mainBoxSizer->Add(headingPanelSizerH, 5, wxGROW | wxALL, 5);

    wxBoxSizer* headingPanelSizerH2 = new wxBoxSizer(wxVERTICAL);
    headingPanelSizerH2->AddSpacer(15);

    headingPanelSizerH->Add(headingPanelSizerH2, 0, wxEXPAND);
    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    //

    long treeCtrlFlags = wxTR_SINGLE | wxTR_HAS_BUTTONS;
#if defined (__WXWIN__)
    treeCtrlFlags = wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_ROW_LINES;
#endif
    m_treeCtrl = new wxTreeCtrl(this, wxID_ANY
        , wxDefaultPosition, wxSize(titleTextWidth, titleTextWidth), treeCtrlFlags);

    headingPanelSizerH2->Add(flex_sizer, g_flags);
    headingPanelSizerH2->Add(m_treeCtrl, g_flagsExpand);

    /****************************************
     Script Area
     ***************************************/
    // ListBox for source code
    wxBoxSizer* headingPanelSizerV3 = new wxBoxSizer(wxVERTICAL);
    headingPanelSizerH->Add(headingPanelSizerV3, g_flagsExpand);

    wxNotebook* editors_notebook = new wxNotebook(this
        , ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);
    headingPanelSizerV3->Add(editors_notebook, g_flagsExpand);

    createOutputTab(editors_notebook, ID_TAB_OUT);

    /****************************************
     Bottom Panel
     ***************************************/
    wxPanel* buttonPanel = new wxPanel(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    mainBoxSizer->Add(buttonPanel, wxSizerFlags(g_flags).Center());

    wxBoxSizer* buttonPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonPanel->SetSizer(buttonPanelSizer);

    //
    m_buttonOpen = new wxButton(buttonPanel, wxID_OPEN, _("&Import"));
    buttonPanelSizer->Add(m_buttonOpen, g_flags);
    m_buttonOpen->SetToolTip(_("Locate and load a report file."));

    m_buttonSaveAs = new wxButton(buttonPanel, wxID_SAVEAS, _("&Export"));
    buttonPanelSizer->Add(m_buttonSaveAs, g_flags);
    m_buttonSaveAs->SetToolTip(_("Export the report to a new file."));
    buttonPanelSizer->AddSpacer(50);

    m_buttonSave = new wxButton(buttonPanel, wxID_SAVE, _("&Save "));
    buttonPanelSizer->Add(m_buttonSave, g_flags);
    m_buttonSave->SetToolTip(_("Save changes."));

    m_buttonRun = new wxButton(buttonPanel, wxID_EXECUTE, _("&Run"));
    buttonPanelSizer->Add(m_buttonRun, g_flags);
    m_buttonRun->SetToolTip(_("Run selected report."));

    wxButton* button_Close = new wxButton(buttonPanel, wxID_CLOSE, _("&Close "));
    buttonPanelSizer->Add(button_Close, g_flags);
    //button_Close->SetToolTip(_("Save changes before closing. Changes without Save will be lost."));

}

void mmGeneralReportManager::createOutputTab(wxNotebook* editors_notebook, int type)
{
    wxSizerFlags flagsExpand;
    flagsExpand.Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND).Border(wxALL, 5).Proportion(1);
    //Output
    wxPanel* out_tab = new wxPanel(editors_notebook, wxID_ANY);
    editors_notebook->InsertPage(ID_TAB_OUT, out_tab, _("Output"));
    wxBoxSizer *out_sizer = new wxBoxSizer(wxVERTICAL);
    out_tab->SetSizer(out_sizer);
    m_outputHTML = wxWebView::New(out_tab, ID_WEB);
    out_sizer->Add(m_outputHTML, g_flagsExpand);
    out_tab->SetSizerAndFit(out_sizer);
}

void mmGeneralReportManager::createEditorTab(wxNotebook* editors_notebook, int type)
{
    int editorID = 0;
    wxString label;
    switch (type) {
    case ID_SQL_CONTENT: label = _("SQL"); editorID = ID_SQL_CONTENT; break;
    case ID_LUA_CONTENT: label = _("Lua"); editorID = ID_LUA_CONTENT;  break;
    case ID_TEMPLATE: label = _("htt"); editorID = ID_TEMPLATE;  break;
    //default: ;
    }
    if (FindWindow(editorID + MAGIC_NUM)) return;

    int tabID = editors_notebook->GetRowCount();
    wxPanel* panel = new wxPanel(editors_notebook, editorID + MAGIC_NUM);
    editors_notebook->InsertPage(tabID, panel, label);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(sizer);

    MinimalEditor* templateText = new MinimalEditor(panel, editorID);

    sizer->Add(templateText, g_flagsExpand);

    if (editorID == ID_SQL_CONTENT)
    {
        wxBoxSizer *box_sizer1 = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer *box_sizer2 = new wxBoxSizer(wxHORIZONTAL);
        wxButton* buttonPlay = new wxButton(panel, ID_TEST, _("&Test"));
        wxButton* buttonNewTemplate = new wxButton(panel, wxID_NEW, _("Create Template"));
        wxStaticText *info = new wxStaticText(panel, wxID_INFO, "");
        buttonNewTemplate->Enable(false);
        box_sizer2->Add(buttonPlay);
        box_sizer2->AddSpacer(10);
        box_sizer2->Add(buttonNewTemplate);
        box_sizer2->AddSpacer(10);
        box_sizer2->Add(info, g_flagsExpand);

        m_sqlListBox = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize
            , wxLC_REPORT);
        box_sizer1->Add(box_sizer2, wxSizerFlags(g_flagsExpand).Proportion(0));
        box_sizer1->Add(m_sqlListBox, g_flagsExpand);
        sizer->Add(box_sizer1, wxSizerFlags(g_flagsExpand).Border(0));
    }

    panel->SetSizerAndFit(sizer);
}

void mmGeneralReportManager::OnSqlTest(wxCommandEvent& event)
{
    MinimalEditor* sqlText = (MinimalEditor*) FindWindow(ID_SQL_CONTENT);
    if (sqlText->GetValue().empty()) return;

    const wxString &sql = sqlText->GetValue();
    if (Model_Report::instance().CheckSyntax(sql))
    {
        long interval = GetTickCount();
        if (Model_Report::instance().getSqlQuery(sql, m_sqlQueryData))
        {
            interval = GetTickCount() - interval;
            m_sqlListBox->ClearAll();

            int row = 0, pos = 0;
            m_sqlListBox->InsertColumn(pos, _("#"), wxLIST_FORMAT_RIGHT, 40);
            for (const auto& col : Model_Report::instance().getColumns(sql))
                m_sqlListBox->InsertColumn(++pos, col.first
                , col.second == 1 ? wxLIST_FORMAT_RIGHT : wxLIST_FORMAT_LEFT
                , col.first.length() * 10 + 20);

            for (const auto& dataRow : m_sqlQueryData)
            {
                int pos = 0;
                long itemIndex = m_sqlListBox->InsertItem(row, "", 0);
                m_sqlListBox->SetItem(itemIndex, pos, wxString() << ++row);
                for (const auto& dataCol : dataRow)
                {
                    m_sqlListBox->SetItem(itemIndex, ++pos, dataCol);
                }
            }
            wxStaticText* info = (wxStaticText*) FindWindow(wxID_INFO);
            info->SetLabel(wxString::Format(_("Row(s) returned: %i  Duration: %s ms"), row, wxString()<<interval));

            wxButton* b = (wxButton*) FindWindow(wxID_NEW);
            MinimalEditor* templateText = (MinimalEditor*) FindWindow(ID_TEMPLATE);
            if (Model_Report::instance().CheckHeaders(sql))
            {
                b->Enable(templateText->GetValue().empty());
            }
            else
            {
                b->Enable(false);
                if (templateText->GetValue().empty())
                {
                    wxRichToolTip tip(_("Error")
                        , _("Some columns alias is wrong! Can't create HTML template")
                        + "\n\n"
                        + _("Tip: SQL AS temporarily assigns a table column a new name.")
                        + "\n");
                    tip.SetIcon(wxICON_WARNING);
                    tip.ShowFor((wxWindow*) b);
                }
            }
        }
        else
        {
            wxMessageDialog msgDlg(this, _("Syntax Error"), _("Error"), wxOK | wxICON_ERROR);
            msgDlg.ShowModal();
        }
    }
}

void mmGeneralReportManager::OnNewTemplate(wxCommandEvent& event)
{
    MinimalEditor* templateText = (MinimalEditor*) FindWindow(ID_TEMPLATE);
    if (!templateText->GetValue().empty()) return;
    MinimalEditor* sqlText = (MinimalEditor*) FindWindow(ID_SQL_CONTENT);

    wxNotebook* n = (wxNotebook*) FindWindow(ID_NOTEBOOK);
    n->SetSelection(ID_TAB_HTT);

    templateText->ChangeValue(Model_Report::instance().getTemplate(sqlText->GetValue()));

    wxButton* b = (wxButton*) FindWindow(wxID_NEW);
    b->Enable(false);

    wxCommandEvent evt;
    OnUpdateReport(evt);
}

void mmGeneralReportManager::OnImportReportEvt(wxCommandEvent& /*event*/)
{
    importReport();
}

void mmGeneralReportManager::importReport()
{
    const wxString reportFileName = wxFileSelector(_("Load report file:")
        , mmex::getPathUser(mmex::DIRECTORY), wxEmptyString, wxEmptyString
        , "GRM Files (*.grm)|*.grm|ZIP files (*.zip)|*.zip"
        , wxFD_FILE_MUST_EXIST);

    if (reportFileName.empty()) return;

    wxFileName fn(reportFileName);
    const wxString clearFileName = fn.FileName(reportFileName).GetName();

    wxString sql, lua, htt, readme;
    Model_Report::Data_Set reports = Model_Report::instance().find(Model_Report::REPORTNAME(clearFileName));
    if (!reports.empty())
    {
        mmShowErrorMessage(this, _("Report with same name exists"), _("General Report Manager"));
        return;
    }
    else
    {
        openZipFile(reportFileName, htt, sql, lua, readme);
        Model_Report::Data *report = 0;
        report = Model_Report::instance().create();
        report->GROUPNAME = m_selectedGroup;
        report->REPORTNAME = clearFileName;
        report->SQLCONTENT = sql;
        report->LUACONTENT = lua;
        report->TEMPLATEPATH = htt;
        m_selectedReportID = Model_Report::instance().save(report);
    }

    if (!readme.empty())
    {
        wxNotebook* n = (wxNotebook*) FindWindow(ID_NOTEBOOK);
        n->SetSelection(ID_TAB_OUT);
        m_outputHTML->ClearBackground();
        m_outputHTML->SetPage(readme, "readme");
    }
    fillControls();
}

bool mmGeneralReportManager::readTextFile(const wxString &fileName, wxString &data)
{
    bool ok = true;
    wxTextFile reportFile(fileName);
    if (reportFile.Open())
    {
        while (!reportFile.Eof())
            data.Append(reportFile.GetNextLine() + "\n");

        reportFile.Close();
    }
    else
    {
        wxString msg = wxString::Format(_("Unable to open file \n%s\n\n"), fileName);
        wxMessageBox(msg, _("General Reports Manager"), wxOK | wxICON_ERROR);
        ok = false;
    }
    return ok;
}

bool mmGeneralReportManager::openZipFile(const wxString &reportFileName
    , wxString &htt, wxString &sql, wxString &lua, wxString &readme)
{
    if (!reportFileName.empty())
    {
        wxTextFile reportFile(reportFileName);
        if (reportFile.Open())
        {
            std::auto_ptr<wxZipEntry> entry;
            wxFFileInputStream in(reportFileName);
            wxZipInputStream zip(in);
            while (entry.reset(zip.GetNextEntry()), entry.get() != NULL)
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
                else if (f.StartsWith("readme"))
                    readme << textdata;
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
void mmGeneralReportManager::OnUpdateReport(wxCommandEvent& /*event*/)
{
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(m_treeCtrl->GetItemData(m_selectedItemID));
    if (!iData) return;

    int id = iData->get_report_id();
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        MinimalEditor* templateText = (MinimalEditor*) FindWindow(ID_TEMPLATE);
        MinimalEditor* SqlScriptText = (MinimalEditor*) FindWindow(ID_SQL_CONTENT);
        MinimalEditor* LuaScriptText = (MinimalEditor*) FindWindow(ID_LUA_CONTENT);
        report->SQLCONTENT = SqlScriptText->GetValue();
        report->LUACONTENT = LuaScriptText->GetValue();
        report->TEMPLATEPATH = templateText->GetValue();

        Model_Report::instance().save(report);
    }
}

void mmGeneralReportManager::OnRun(wxCommandEvent& /*event*/)
{
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(m_treeCtrl->GetItemData(m_selectedItemID));
    if (!iData) return;

    int id = iData->get_report_id();
    m_selectedGroup = iData->get_group_name();
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        wxNotebook* n = (wxNotebook*) FindWindow(ID_NOTEBOOK);
        n->SetSelection(ID_TAB_OUT);
        m_outputHTML->ClearBackground();
        mmGeneralReport gr(report);
        m_outputHTML->SetPage(gr.getHTMLText(),"");
    }
}

void mmGeneralReportManager::OnItemRightClick(wxTreeEvent& event)
{
    wxTreeItemId id = event.GetItem();
    m_treeCtrl ->SelectItem(id);
    int report_id = -1;
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(m_treeCtrl->GetItemData(id));
    if (iData) report_id = iData->get_report_id();

    wxMenu* samplesMenu = new wxMenu;
    samplesMenu->Append(ID_NEW_SAMPLE_ASSETS, _("Assets"));

    wxMenu* customReportMenu = new wxMenu;
    customReportMenu->Append(ID_NEW_EMPTY, _("New Empty Report"));
    customReportMenu->Append(wxID_ANY, _("New Sample Report"), samplesMenu);
    customReportMenu->AppendSeparator();
    customReportMenu->Append(ID_GROUP, _("Change Group"));
    customReportMenu->Enable(ID_GROUP, report_id > 0);
    customReportMenu->Append(ID_RENAME, _("Rename Report"));
    customReportMenu->Enable(ID_RENAME, report_id > 0);
    customReportMenu->AppendSeparator();
    customReportMenu->Append(ID_DELETE, _("Delete Report"));
    customReportMenu->Enable(ID_DELETE, report_id > 0);
    PopupMenu(customReportMenu);
    delete customReportMenu;
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

    wxNotebook* editors_notebook = (wxNotebook*) FindWindow(ID_NOTEBOOK);
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(m_treeCtrl->GetItemData(m_selectedItemID));
    if (!iData)
    {
        for (size_t n = editors_notebook->GetPageCount() - 1; n >= 1; n--) editors_notebook->DeletePage(n);

        wxFileName helpIndexFile(mmex::getPathDoc((mmex::EDocFile)mmex::HTML_CUSTOM_SQL));
        if (mmOptions::instance().language_ != "english") helpIndexFile.AppendDir(mmOptions::instance().language_);
        wxString url = "file://" + mmex::getPathDoc((mmex::EDocFile)mmex::HTML_CUSTOM_SQL);
        if (helpIndexFile.FileExists()) // Load the help file for the given language
            url = "file://" + helpIndexFile.GetPathWithSep() + helpIndexFile.GetFullName();
        m_outputHTML->LoadURL(url);
        wxLogDebug("%s", url);
        return;
    }

    int id = iData->get_report_id();
    m_selectedGroup = iData->get_group_name();
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (!report)
    {
        for (size_t n = editors_notebook->GetPageCount() - 1; n >= 1; n--) editors_notebook->DeletePage(n);
    }
    else
    {
        m_selectedReportID = report->REPORTID;
        createEditorTab(editors_notebook, ID_TEMPLATE);
        createEditorTab(editors_notebook, ID_LUA_CONTENT);
        createEditorTab(editors_notebook, ID_SQL_CONTENT);

        MinimalEditor* SqlScriptText = (MinimalEditor*) FindWindow(ID_SQL_CONTENT);
        MinimalEditor* LuaScriptText = (MinimalEditor*) FindWindow(ID_LUA_CONTENT);
        MinimalEditor* templateText = (MinimalEditor*) FindWindow(ID_TEMPLATE);

        templateText->ChangeValue(report->TEMPLATEPATH);
        templateText->SetLexerHtml();
        SqlScriptText->ChangeValue(report->SQLCONTENT);
        SqlScriptText->SetLexerSql();
        LuaScriptText->ChangeValue(report->LUACONTENT);
        LuaScriptText->SetLexerLua();

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
        /*for (auto reserved : m_reservedNames)
        {
            if (label == reserved)
            {
                wxMessageBox(_("Invalid name"), _("MMEX reserved name"), wxOK | wxICON_ERROR);
                return;
            }
        }*/
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
        wxString msg = wxString() << _("Delete the Custom Report Title:")
            << "\n\n"
            << report->REPORTNAME;
        int iError = wxMessageBox(msg, "General Reports Manager", wxYES_NO | wxICON_ERROR);
        if (iError == wxYES)
        {
            m_selectedReportID = -1;
            Model_Report::instance().remove(id);
            wxListEvent evt(wxEVT_TREE_SEL_CHANGED, wxID_ANY);
            AddPendingEvent(evt);

            fillControls();
            return true;
        }
    }
    return false;
}

void mmGeneralReportManager::OnMenuSelected(wxCommandEvent& event)
{
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(m_treeCtrl->GetItemData(m_selectedItemID));
    int id = event.GetId();

    if (iData && id == ID_RENAME)
    {
        int report_id = iData->get_report_id();
        this->renameReport(report_id);
    }
    else if (iData && id == ID_DELETE)
    {
            int report_id = iData->get_report_id();
            this->DeleteReport(report_id);
    }
    else if (iData && id == ID_GROUP)
    {
        int report_id = iData->get_report_id();
        Model_Report::Data * report = Model_Report::instance().get(report_id);
        if (report)
        {
            report->GROUPNAME = wxGetTextFromUser(_("Enter the name for the new report group")
                , _("General Report Manager"), "");
            Model_Report::instance().save(report);
        }
    }
    else
        newReport(id);

    fillControls();
}

void mmGeneralReportManager::newReport(int sample)
{
    wxString group_name;
    if (m_selectedItemID == m_rootItem)
    {
        group_name = wxGetTextFromUser(_("Enter the name for the new report group")
            , _("Add Report Group"), "");
    }
    else
    {
        group_name = m_selectedGroup;
    }

    Model_Report::Data* report = Model_Report::instance().create();
    report->GROUPNAME = group_name;
    int i = Model_Report::instance().all().size();
    wxString report_name = _("New Report");
    while (!Model_Report::instance().find(Model_Report::REPORTNAME(report_name)).empty())
        report_name = wxString::Format(_("New Report %i"), ++i);
    report->REPORTNAME = report_name;
    wxString sqlContent, luaContent, httContent;
    switch (sample) {
    case ID_NEW_EMPTY:
        sqlContent = ""; luaContent = ""; httContent = ""; break;
    case ID_NEW_SAMPLE_ASSETS:
        sqlContent = SAMPLE_ASSETS_SQL;
        luaContent = SAMPLE_ASSETS_LUA;
        httContent = SAMPLE_ASSETS_HTT;
        break;
    }

    report->SQLCONTENT = sqlContent;
    report->LUACONTENT = luaContent;
    report->TEMPLATEPATH = httContent; //TODO: rename TEMPLATEPATH to TEMPLATECONTENT
    m_selectedReportID = Model_Report::instance().save(report);
}

void mmGeneralReportManager::OnExportReport(wxCommandEvent& /*event*/)
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
            , "GRM File(*.grm)|*.grm"
            , wxFD_SAVE | wxFD_OVERWRITE_PROMPT
            );

        if (dlg.ShowModal() != wxID_OK)
            return;

        file_name = dlg.GetPath();

        wxFFileOutputStream out(file_name);
        wxZipOutputStream zip(out);
        wxTextOutputStream txt(zip);
        zip.PutNextEntry("sqlcontent.sql");
        txt << report->SQLCONTENT;
        zip.PutNextEntry("luacontent.lua");
        txt << report->LUACONTENT;
        zip.PutNextEntry("template.htt");
        txt << report->TEMPLATEPATH;
    }
}

void mmGeneralReportManager::OnClose(wxCommandEvent& /*event*/)
{
    EndModal(wxID_OK);
}
