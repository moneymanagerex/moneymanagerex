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

#include "customreportdialog.h"
#include "minimal_editor.h"
#include "util.h"
#include "paths.h"
#include "mmpanelbase.h"
#include "model/Model_Infotable.h"
#include "model/Model_Report.h"
#include <wx/zipstrm.h>
#include <memory>

int titleTextWidth   = 200; // Determines width of Headings Textbox.
int sourceTextHeight = 200; // Determines height of Source Textbox.
static const char LUA_SAMPLE[] =
    "local total_balance = 0\n"
    "function handle_record(record)\n"
        "\ttotal_balance = total_balance + record:get('VALUE');\n"
    "end\n\n"
    "function complete(result)\n"
        "\tresult:set('ASSET_BALANCE', total_balance);\n"
    "end\n";
static const char SQL_SAMPLE[] =
    "SELECT * FROM ASSETS_V1";
static const wxString HTT_SAMPLE = //TODO:
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\n"
    "<html>\n"
    "<head>\n"
    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" /><title>MoneyManagerEx - Report</title>\n"
    "</head>\n"
    "<body>\n"
    "Sample <TMPL_VAR TODAY>\n"
    "<table>\n"
    "<tr>\n"
        "<th>Date</th>\n"
        "<th>Name</th>\n"
        "<th>Type</th>\n"
        "<th>Value</th>\n"
        "<th>Notes</th>\n"
    "</tr>\n"
    "<TMPL_LOOP NAME=CONTENTS>\n"
        "<TMPL_IF NAME=\"__ODD__\">\n"
            "<tr bgcolor=\"#FFFFFF\" >\n"
        "<TMPL_ELSE>\n"
            "<tr bgcolor=\"#E1EDFB\" >\n"
        "</TMPL_IF>\n"
        "<td><TMPL_VAR STARTDATE></td>\n"
        "<td><TMPL_VAR ASSETNAME></td>\n"
        "<td><TMPL_VAR ASSETTYPE></td>\n"
        "<td><TMPL_VAR VALUE></td>\n"
        "<td><TMPL_VAR NOTES></td>\n"
    "</tr>\n"
    "</TMPL_LOOP>\n"
        "<tr>"
        "<td colspan=3 >Total Assets: </td>"
        "<td nowrap align=\"right\"><TMPL_VAR ASSET_BALANCE></td>"
    "</tr>"

    "</table>\n"

    //TODO:
    "</body>\n"
    "</html>\n";


IMPLEMENT_DYNAMIC_CLASS( mmGeneralReportManager, wxDialog )

BEGIN_EVENT_TABLE(mmGeneralReportManager, wxDialog)
    EVT_BUTTON(wxID_OPEN, mmGeneralReportManager::OnImportReportEvt)
    EVT_BUTTON(wxID_SAVE, mmGeneralReportManager::OnUpdateReport)
    EVT_BUTTON(wxID_SAVEAS, mmGeneralReportManager::OnExportReport)
    EVT_BUTTON(wxID_EXECUTE, mmGeneralReportManager::OnRun)
    EVT_BUTTON(wxID_CLOSE, mmGeneralReportManager::OnClose)
    EVT_TREE_SEL_CHANGED(wxID_ANY, mmGeneralReportManager::OnSelChanged)
    EVT_TREE_END_LABEL_EDIT(wxID_ANY, mmGeneralReportManager::OnLabelChanged)
    EVT_TREE_ITEM_MENU(wxID_ANY, mmGeneralReportManager::OnItemRightClick)
    EVT_MENU(wxID_ANY, mmGeneralReportManager::OnMenuSelected)
END_EVENT_TABLE()

mmGeneralReportManager::mmGeneralReportManager(wxWindow* parent)
    : m_buttonOpen()
    , m_buttonSave()
    , m_buttonSaveAs()
    , m_buttonRun()
    , m_treeCtrl()
    , m_fileNameCtrl()
    , m_outputHTML()
    , m_selectedReportID(0)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("Custom Reports Manager"), wxDefaultPosition, wxSize(640, 480), style);
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

    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_NORMAL, WXK_F9, wxID_EXECUTE);
    wxAcceleratorTable accel(1, entries);
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
        = Model_Report::instance().all(Model_Report::COL_GROUPNAME, Model_Report::COL_REPORTNAME);
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
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, 5);
    flagsExpand.Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND).Border(wxALL, 5).Proportion(1);

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

    long treeCtrlFlags = wxTR_EDIT_LABELS | wxTR_SINGLE | wxTR_HAS_BUTTONS;
#if defined (__WXWIN__)
    treeCtrlFlags = wxTR_EDIT_LABELS | wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_ROW_LINES;
#endif
    m_treeCtrl = new wxTreeCtrl(this, wxID_ANY
        , wxDefaultPosition, wxSize(titleTextWidth, titleTextWidth), treeCtrlFlags);

    headingPanelSizerH2->Add(flex_sizer, flags);
    headingPanelSizerH2->Add(m_treeCtrl, flagsExpand);

    /****************************************
     Script Area
     ***************************************/
    // ListBox for source code
    wxBoxSizer* headingPanelSizerV3 = new wxBoxSizer(wxVERTICAL);
    headingPanelSizerH->Add(headingPanelSizerV3, flagsExpand);

    wxNotebook* editors_notebook = new wxNotebook(this
        , ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);
    headingPanelSizerV3->Add(editors_notebook, flagsExpand);

    createOutputTab(editors_notebook, ID_TAB_OUT);

    wxBoxSizer *file_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_fileNameCtrl = new wxTextCtrl(this, wxID_FILE, wxEmptyString
        , wxDefaultPosition, wxSize(480, -1), wxTE_READONLY);
    file_sizer->Add(new wxStaticText(this, wxID_STATIC, _("File Name:")), flags);
    file_sizer->Add(m_fileNameCtrl, flagsExpand);
    headingPanelSizerV3->Add(file_sizer, flagsExpand.Proportion(0));

    /****************************************
     Bottom Panel
     ***************************************/
    wxPanel* buttonPanel = new wxPanel(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    mainBoxSizer->Add(buttonPanel, flags.Center());

    wxBoxSizer* buttonPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonPanel->SetSizer(buttonPanelSizer);

    //
    m_buttonOpen = new wxButton(buttonPanel, wxID_OPEN, _("&Import"));
    buttonPanelSizer->Add(m_buttonOpen, flags);
    m_buttonOpen->SetToolTip(_("Locate and load a report file."));

    m_buttonSaveAs = new wxButton(buttonPanel, wxID_SAVEAS, _("&Export"));
    buttonPanelSizer->Add(m_buttonSaveAs, flags);
    m_buttonSaveAs->SetToolTip(_("Export the report to a new file."));
    buttonPanelSizer->AddSpacer(50);

    m_buttonSave = new wxButton(buttonPanel, wxID_SAVE, _("&Save "));
    buttonPanelSizer->Add(m_buttonSave, flags);
    m_buttonSave->SetToolTip(_("Save changes."));

    m_buttonRun = new wxButton(buttonPanel, wxID_EXECUTE, _("&Run"));
    buttonPanelSizer->Add(m_buttonRun, flags);
    m_buttonRun->SetToolTip(_("Run selected report."));

    wxButton* button_Close = new wxButton(buttonPanel, wxID_CLOSE);
    buttonPanelSizer->Add(button_Close, flags);
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
    out_sizer->Add(m_outputHTML, flagsExpand);
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
    if (FindWindow(editorID + MAGIC_NUM))  return;

    wxSizerFlags flagsExpand;
    flagsExpand.Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND).Border(wxALL, 5).Proportion(1);

    int tabID = editors_notebook->GetRowCount();
    wxPanel* panel = new wxPanel(editors_notebook, editorID + MAGIC_NUM);
    editors_notebook->InsertPage(tabID, panel, label);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(sizer);

    MinimalEditor* templateText = new MinimalEditor(panel, editorID);

    sizer->Add(templateText, flagsExpand);
    panel->SetSizerAndFit(sizer);
}

void mmGeneralReportManager::OnImportReportEvt(wxCommandEvent& /*event*/)
{
    importReport();
}

void mmGeneralReportManager::importReport()
{
    wxString reportFileName = wxFileSelector(_("Load report file:")
        , mmex::getPathUser(mmex::DIRECTORY), wxEmptyString, wxEmptyString
        , "File(*.grm)|*.grm"
        , wxFD_FILE_MUST_EXIST);

    if (reportFileName.empty()) return;

    wxFileName fn(reportFileName);
    wxString clearFileName = fn.FileName(reportFileName).GetName();
    wxFileDialog dlg(this
        , _("Choose file to Save As HTML Template")
        , wxEmptyString
        , clearFileName + ".htt"
        , "HTML File(*.htt)|*.htt"
        , wxFD_SAVE | wxFD_OVERWRITE_PROMPT
        );

    if (dlg.ShowModal() != wxID_OK)
        return;

    const wxString directory = dlg.GetDirectory() + wxFileName::GetPathSeparator();
    const wxString httFileName = dlg.GetFilename();
    const wxString httFullFileName = directory + httFileName;
    wxString sql, lua, htt, readme;
    Model_Report::Data_Set reports = Model_Report::instance().find(Model_Report::REPORTNAME(clearFileName));
    if (!reports.empty())
    {
        mmShowErrorMessage(this, _("Report with same name exists"), _("General Report Manager"));
        return;
    }
    else
    {
        openZipFile(reportFileName, httFileName, directory, sql, lua, readme);
        Model_Report::Data *report = 0;
        report = Model_Report::instance().create();
        report->GROUPNAME = m_selectedGroup;
        report->REPORTNAME = clearFileName;
        report->SQLCONTENT = sql;
        report->LUACONTENT = lua;
        report->TEMPLATEPATH = httFullFileName;
        if (!report->TEMPLATEPATH.empty())
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
    , const wxString &httFileName
    , const wxString directoryToExtract
    , wxString &sql, wxString &lua, wxString &readme)
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
                else
                {
                    wxString file = f;
                    if (f.EndsWith(".htt")) file = httFileName;
                    wxLogDebug("copy %s to %s", f, directoryToExtract + file);
                    wxFileOutputStream output(directoryToExtract + file);
                    wxTextOutputStream text(output);
                    text << textdata;
                    output.Close();
                }
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
        MinimalEditor* SqlScriptText = (MinimalEditor*) FindWindow(ID_SQL_CONTENT);
        MinimalEditor* LuaScriptText = (MinimalEditor*) FindWindow(ID_LUA_CONTENT);
        report->SQLCONTENT = SqlScriptText->GetValue();
        report->LUACONTENT = LuaScriptText->GetValue();

        wxFileOutputStream output(report->TEMPLATEPATH);
        wxTextOutputStream text(output);
        MinimalEditor* templateText = (MinimalEditor*) FindWindow(ID_TEMPLATE);
        text << templateText->GetValue();
        output.Close();

        Model_Report::instance().save(report);
    }
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
        zip.PutNextEntry("template.html");
        MinimalEditor* templateText = (MinimalEditor*) FindWindow(ID_TEMPLATE);
        txt << templateText->GetValue();
        //TODO: save all files from VFS handler???
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

    wxMenu* customReportMenu = new wxMenu;
    customReportMenu->Append(ID_NEW1, _("New Custom Report"));
    customReportMenu->AppendSeparator();
    customReportMenu->Append(ID_GROUP, _("Change Group"));
    customReportMenu->Enable(ID_GROUP, report_id > 0);
    customReportMenu->AppendSeparator();
    customReportMenu->Append(ID_DELETE, _("Delete Custom Report"));
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
        m_fileNameCtrl->ChangeValue("");
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

        m_fileNameCtrl->ChangeValue(report->TEMPLATEPATH);
        wxString data;
        readTextFile(report->TEMPLATEPATH, data);
        templateText->ChangeValue(data);
        templateText->SetLexerHtml();
        SqlScriptText->ChangeValue(report->SQLCONTENT);
        SqlScriptText->SetLexerSql();
        LuaScriptText->ChangeValue(report->LUACONTENT);
        LuaScriptText->SetLexerLua();

        //m_outputHTML->SetPage(report->REPORTNAME, ""); //TODO: provide report info

        wxString full_path = report->TEMPLATEPATH;
        wxTextFile tFile;
        tFile.Open(full_path);
        if (!tFile.Open())
        {
            wxMessageBox(wxString::Format(_("Unable to open file %s"), full_path)
                , _("General Reports Manager"), wxOK | wxICON_ERROR);
        }
        else
        {
            templateText->ClearAll();
            wxFileInputStream input(full_path);
            wxTextInputStream text(input);
            while (input.IsOk() && !input.Eof())
            {
                const wxString line = text.ReadLine();
                if (!line.IsEmpty())
                    templateText->AppendText(line + "\n");
            }
        }
        viewControls(true);
    }
}

void mmGeneralReportManager::OnLabelChanged(wxTreeEvent& event)
{
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(m_treeCtrl->GetItemData(event.GetItem()));
    if (!iData) return;

    wxString label = event.GetLabel();
    int id = iData->get_report_id();
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        if (Model_Report::instance().find(Model_Report::REPORTNAME(label)).empty()
            && !label.empty())
        {
            report->REPORTNAME = label;
            Model_Report::instance().save(report);
            fillControls();
        }
        else
            event.Veto();
    }
    else
        event.Veto();
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
    if (id == ID_NEW1)
    {
        newReport();
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
    fillControls();
}

void mmGeneralReportManager::newReport()
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
    report->SQLCONTENT = SQL_SAMPLE;
    report->LUACONTENT = LUA_SAMPLE;
    report->TEMPLATEPATH = openTemplate();
    if (!report->TEMPLATEPATH.empty())
        m_selectedReportID = Model_Report::instance().save(report);
}

wxString mmGeneralReportManager::openTemplate()
{
    wxString sTemplateFileName = wxFileSelector(_("Save template file:")
        , mmex::getPathUser(mmex::DIRECTORY), wxEmptyString, wxEmptyString
        , "File(*.htt)|*.htt"
        , wxFD_SAVE);
    if (!sTemplateFileName.empty())
    {
        correctEmptyFileExt("htt", sTemplateFileName);
        wxTextFile templateFile(sTemplateFileName);
        if (!templateFile.Exists() && !templateFile.Create())
        {
            wxMessageBox(_("Unable to write to file."), _("General Report Manager"), wxOK|wxICON_WARNING);
            return "";
        }
        else
        {
            //clear the contents of the current file
            templateFile.Clear();
            templateFile.AddLine(HTT_SAMPLE); //TODO: Line separators
        }
        templateFile.Write();
        templateFile.Close();
    }
    return sTemplateFileName;
}

void mmGeneralReportManager::OnClose(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}
