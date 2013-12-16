/*******************************************************
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

#include "customreportdialog.h"
#include "util.h"
#include "paths.h"
#include "model/Model_Infotable.h"
#include "model/Model_Report.h"

int titleTextWidth   = 200; // Determines width of Headings Textbox.
int sourceTextHeight = 200; // Determines height of Source Textbox.
enum
{
    MARGIN_LINE_NUMBERS,
    ID_TAB1 = 0,
    ID_TAB2,
    ID_TAB3,
    HEADING_ONLY = wxID_HIGHEST + 1,
    SUB_REPORT,
    ID_NEW1,
    ID_NEW2,
    ID_DELETE,
    ID_OUTPUT_WIN,
    ID_NOTEBOOK
};

IMPLEMENT_DYNAMIC_CLASS( mmGeneralReportManager, wxDialog )

BEGIN_EVENT_TABLE( mmGeneralReportManager, wxDialog )
    EVT_BUTTON(wxID_OPEN,  mmGeneralReportManager::OnOpen)
    EVT_BUTTON(wxID_SAVEAS,  mmGeneralReportManager::OnSave)
    EVT_BUTTON(wxID_REFRESH, mmGeneralReportManager::OnRun)
    EVT_BUTTON(wxID_CLEAR, mmGeneralReportManager::OnClear)
    EVT_BUTTON(wxID_CLOSE, mmGeneralReportManager::OnClose)
    EVT_TREE_SEL_CHANGED(wxID_ANY, mmGeneralReportManager::OnSelChanged)
    EVT_TREE_END_LABEL_EDIT(wxID_ANY, mmGeneralReportManager::OnLabelChanged)
    EVT_TREE_ITEM_MENU(wxID_ANY, mmGeneralReportManager::OnItemRightClick)
    EVT_MENU(wxID_ANY, mmGeneralReportManager::OnMenuSelected)
END_EVENT_TABLE()

mmGeneralReportManager::mmGeneralReportManager(wxWindow* parent)
: tcSourceTxtCtrl_()
, button_Open_()
, button_Save_()
, button_Run_()
, button_Clear_()
, m_reportType()
, treeCtrl_()

{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("Custom Reports Manager"), wxDefaultPosition, wxSize(640, 480), style);
}

mmGeneralReportManager::~mmGeneralReportManager()
{
}

bool mmGeneralReportManager::Create( wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_NORMAL, WXK_F9, wxID_REFRESH);
    wxAcceleratorTable accel(1, entries);
    SetAcceleratorTable(accel);

    Connect(wxID_REFRESH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(mmGeneralReportManager::OnRun));

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
    treeCtrl_->SetEvtHandlerEnabled(false);
    treeCtrl_->DeleteAllItems();
    root_ = treeCtrl_->AddRoot(_("Custom Reports"));
    selectedItemId_ = root_;
    treeCtrl_->SetItemBold(root_, true);
    treeCtrl_->SetFocus();
    Model_Report::Data_Set reports = Model_Report::instance().all(Model_Report::COL_GROUPNAME, Model_Report::COL_REPORTNAME);
    wxTreeItemId group;
    wxString group_name = "\x05";
    for (const auto& report : reports)
    {
        bool no_group = report.GROUPNAME.empty();
        if (group_name != report.GROUPNAME && !no_group)
        {
            group = treeCtrl_->AppendItem(root_, report.GROUPNAME);
            treeCtrl_->SetItemData(group, new MyTreeItemData(-1, report.GROUPNAME));
            group_name = report.GROUPNAME;
        }
        wxTreeItemId item = treeCtrl_->AppendItem(no_group ? root_ : group, report.REPORTNAME);
        treeCtrl_->SetItemData(item, new MyTreeItemData(report.REPORTID, report.GROUPNAME));
    }
    treeCtrl_->ExpandAll();
    treeCtrl_->SetEvtHandlerEnabled(true);
}

void mmGeneralReportManager::CreateControls()
{
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxEXPAND).Border(wxALL, 5).Proportion(1);

    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainBoxSizer);

    /****************************************
     Parameters Area
     ***************************************/

    wxBoxSizer* headingPanelSizerH = new wxBoxSizer(wxHORIZONTAL);
    mainBoxSizer->Add(headingPanelSizerH, 5, wxGROW|wxALL, 5);

    wxBoxSizer* headingPanelSizerH2 = new wxBoxSizer(wxVERTICAL);
    headingPanelSizerH2->AddSpacer(15);

    headingPanelSizerH->Add(headingPanelSizerH2, 0, wxEXPAND);
    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    //

    flex_sizer->Add(new wxStaticText( this, wxID_STATIC, _("Script type:")), flags);
    flex_sizer->AddSpacer(1);

    m_reportType = new wxTextCtrl( this, wxID_PROPERTIES, ""
        , wxDefaultPosition, wxSize(titleTextWidth,-1));

    long treeCtrlFlags = wxTR_EDIT_LABELS | wxTR_SINGLE | wxTR_HAS_BUTTONS;
#if defined (__WXWIN__)
    treeCtrlFlags = wxTR_EDIT_LABELS | wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_ROW_LINES;
#endif
    treeCtrl_ = new wxTreeCtrl(this, wxID_ANY
        , wxDefaultPosition, wxSize(titleTextWidth, titleTextWidth), treeCtrlFlags);

    headingPanelSizerH2->Add(flex_sizer, flags);
    headingPanelSizerH2->Add(m_reportType, flags);
    headingPanelSizerH2->Add(treeCtrl_, flagsExpand);

    /****************************************
     Script Area
     ***************************************/
    // ListBox for source code
    wxBoxSizer* headingPanelSizerV3 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* headingPanelSizerH4 = new wxBoxSizer(wxHORIZONTAL);
    headingPanelSizerH->Add(headingPanelSizerV3, flagsExpand);

    wxNotebook* editors_notebook = new wxNotebook(this
        , ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);
    wxPanel* script_tab = new wxPanel(editors_notebook, wxID_ANY);
    editors_notebook->InsertPage(ID_TAB1, script_tab, _("Script"));
    wxBoxSizer *script_sizer = new wxBoxSizer(wxVERTICAL);
    script_tab->SetSizer(script_sizer);
    headingPanelSizerV3->Add(editors_notebook, flagsExpand);

    tcSourceTxtCtrl_ = new wxTextCtrl(script_tab, wxID_VIEW_DETAILS
        , "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
#if 0
    tcSourceTxtCtrl_->StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour (75, 75, 75) );
    tcSourceTxtCtrl_->StyleSetBackground (wxSTC_STYLE_LINENUMBER, wxColour (220, 220, 220));
    tcSourceTxtCtrl_->SetMarginType (MARGIN_LINE_NUMBERS, wxSTC_MARGIN_NUMBER);
    tcSourceTxtCtrl_->SetMarginWidth (MARGIN_LINE_NUMBERS, 50);
    tcSourceTxtCtrl_->Connect(wxID_ANY, wxEVT_CHAR
        , wxKeyEventHandler(mmGeneralReportManager::OnSourceTxtChar), NULL, this);
    int font_size = this->GetFont().GetPointSize();
    wxFont teletype( font_size, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
    tcSourceTxtCtrl_->SetFont(teletype);
#endif
    script_sizer->Add(tcSourceTxtCtrl_, flagsExpand);

    //Template
    wxPanel* html_tab = new wxPanel(editors_notebook, wxID_ANY);
    editors_notebook->InsertPage(ID_TAB2, html_tab, _("Template"));
    wxBoxSizer *html_sizer = new wxBoxSizer(wxVERTICAL);
    html_tab->SetSizer(html_sizer);

    wxBoxSizer *file_sizer = new wxBoxSizer(wxHORIZONTAL);
    file_name_ctrl_ = new wxTextCtrl(html_tab, wxID_FILE, wxEmptyString
        , wxDefaultPosition, wxSize(200, -1), wxTE_READONLY);
    file_sizer->Add(new wxStaticText(html_tab, wxID_STATIC, _("File Name:")), flags);
    file_sizer->Add(file_name_ctrl_, flagsExpand);

    html_text_ = new wxTextCtrl(html_tab, wxID_ANY
        , "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);

#if 0
    html_text_->SetMarginWidth (MARGIN_LINE_NUMBERS, 50);
    html_text_->StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour (75, 75, 75) );
    html_text_->StyleSetBackground (wxSTC_STYLE_LINENUMBER, wxColour (220, 220, 220));
    html_text_->SetMarginType (MARGIN_LINE_NUMBERS, wxSTC_MARGIN_NUMBER);
    html_text_->SetWrapMode (wxSTC_WRAP_WORD);
    html_text_->StyleClearAll();
    html_text_->SetLexer(wxSTC_LEX_HTML);
    html_text_->StyleSetForeground (wxSTC_H_DOUBLESTRING,     wxColour(255,0,0));
    html_text_->StyleSetForeground (wxSTC_H_SINGLESTRING,     wxColour(255,0,0));
    html_text_->StyleSetForeground (wxSTC_H_ENTITY,           wxColour(255,0,0));
    html_text_->StyleSetForeground (wxSTC_H_TAG,              wxColour(0,150,0));
    html_text_->StyleSetForeground (wxSTC_H_TAGUNKNOWN,       wxColour(0,150,0));
    html_text_->StyleSetForeground (wxSTC_H_ATTRIBUTE,        wxColour(0,0,150));
    html_text_->StyleSetForeground (wxSTC_H_ATTRIBUTEUNKNOWN, wxColour(0,0,150));
    html_text_->StyleSetForeground (wxSTC_H_COMMENT,          wxColour(150,150,150));
#endif
    html_sizer->Add(file_sizer);
    html_sizer->Add(html_text_, flagsExpand);
    html_sizer->Add(headingPanelSizerH4, flags.Center());

    button_Open_ = new wxButton(html_tab, wxID_OPEN, _("Open"));
    headingPanelSizerH4->Add(button_Open_, flags);
    button_Open_->SetToolTip(_("Locate and load a script file into the script area."));

    button_Save_ = new wxButton(html_tab, wxID_SAVEAS, _("Save As..."));
    headingPanelSizerH4->Add(button_Save_, flags);
    button_Save_->SetToolTip(_("Save the script to file name set by the Report Title."));

    button_Clear_ = new wxButton(html_tab, wxID_CLEAR);
    headingPanelSizerH4->Add(button_Clear_, flags);
    button_Clear_->SetToolTip(_("Clear the Source script area"));

    //Output
    wxPanel* out_tab = new wxPanel(editors_notebook, wxID_ANY);
    editors_notebook->InsertPage(ID_TAB3, out_tab, _("Output"));
    wxBoxSizer *out_sizer = new wxBoxSizer(wxVERTICAL);
    out_tab->SetSizer(out_sizer);
    out_html_ = new wxHtmlWindow(out_tab, ID_OUTPUT_WIN
        , wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    out_sizer->Add(out_html_, flagsExpand);

    /****************************************
     Bottom Panel
     ***************************************/
    wxPanel* buttonPanel = new wxPanel( this, wxID_STATIC,  wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    mainBoxSizer->Add(buttonPanel, flags);

    wxBoxSizer* buttonPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonPanel->SetSizer(buttonPanelSizer);

    button_Run_ = new wxButton(buttonPanel, wxID_REFRESH, _("&Run"));
    buttonPanelSizer->Add(button_Run_, flags);
    button_Run_->SetToolTip(_("Test script. Save before running."));

    wxButton* button_Close = new wxButton( buttonPanel, wxID_CLOSE);
    buttonPanelSizer->Add(button_Close, flags);
    button_Close->SetToolTip(_("Save changes before closing. Changes without Save will be lost."));

}

void mmGeneralReportManager::OnOpen(wxCommandEvent& /*event*/)
{
    wxString sScriptFileName = wxFileSelector( _("Load file:")
        , mmex::getPathUser(mmex::DIRECTORY), wxEmptyString, wxEmptyString
        , "File(*.html)|*.html"
        , wxFD_FILE_MUST_EXIST);
    if ( !sScriptFileName.empty() )
    {
        wxFileName selectedFileName(sScriptFileName);
        wxString reportText;

        wxTextFile reportFile(sScriptFileName);
        if (reportFile.Open())
        {
            reportText << reportFile.GetFirstLine() << "\n";
            size_t currentline = 1;
            while (! reportFile.Eof())
            {
                reportText << reportFile.GetNextLine();
                currentline ++;
                if (currentline < reportFile.GetLineCount())
                {
                    reportText << "\n";
                }
            }
            html_text_->SetValue(reportText);
            reportFile.Close();
        }
        else
        {
            wxString msg = wxString() << _("Unable to open file.") << sScriptFileName << "\n\n";
            wxMessageBox(msg, "Under Constraction", wxOK | wxICON_ERROR);
        }
    }
}

void mmGeneralReportManager::OnSave(wxCommandEvent& /*event*/)
{
    fillControls();
}

void mmGeneralReportManager::OnRun(wxCommandEvent& /*event*/)
{
    wxNotebook* n = (wxNotebook*)  FindWindow(ID_NOTEBOOK);
    n->SetSelection(ID_TAB3);

    //TODO:
    //EndModal(wxID_MORE);
}

void mmGeneralReportManager::OnClear(wxCommandEvent& /*event*/)
{
    tcSourceTxtCtrl_->Clear();
    button_Save_->Disable();
    button_Run_->Disable();
    button_Open_->Enable();
    tcSourceTxtCtrl_->SetFocus();
}

void mmGeneralReportManager::OnClose(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmGeneralReportManager::OnItemRightClick(wxTreeEvent& event)
{
    wxTreeItemId id = event.GetItem();
    treeCtrl_ ->SelectItem(id);
    int report_id = -1;
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(treeCtrl_->GetItemData(event.GetItem()));
    if (iData) report_id = iData->get_report_id();


    wxMenu* customReportMenu = new wxMenu;
    customReportMenu->Append(ID_NEW1, _("New SQL Custom Report"));
    customReportMenu->Append(ID_NEW2, _("New Lua Custom Report"));
    customReportMenu->AppendSeparator();
    customReportMenu->Append(ID_DELETE, _("Delete Custom Report"));
    customReportMenu->Enable(ID_DELETE, report_id > 0);
    PopupMenu(customReportMenu);
    delete customReportMenu;
}

void mmGeneralReportManager::OnSelChanged(wxTreeEvent& event)
{
    selectedItemId_ = event.GetItem();
    m_selectedGroup = "";
    m_reportType->ChangeValue("");
    tcSourceTxtCtrl_->ChangeValue("");
    file_name_ctrl_->ChangeValue("");
    html_text_->ChangeValue("");
    wxNotebook* n = (wxNotebook*)  FindWindow(ID_NOTEBOOK);
    n->SetSelection(0);
    button_Run_->Enable(false);

    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(treeCtrl_->GetItemData(event.GetItem()));
    if (!iData) return;

    int id = iData->get_report_id();
    m_selectedGroup = iData->get_group_name();
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        m_reportType->ChangeValue(report->CONTENTTYPE);
        file_name_ctrl_->ChangeValue(report->TEMPLATEPATH);
        tcSourceTxtCtrl_->ChangeValue(report->CONTENT);
#if 0
        tcSourceTxtCtrl_->StyleClearAll();
        tcSourceTxtCtrl_->SetLexer(wxSTC_LEX_SQL);
        tcSourceTxtCtrl_->StyleSetForeground (wxSTC_SQL_WORD,     wxColour(0,150,0));
        tcSourceTxtCtrl_->SetKeyWords(0, "select from where and or");
#endif

        wxString full_path = mmex::getPathUser(mmex::DIRECTORY) + report->TEMPLATEPATH;
        wxTextFile tFile;
        tFile.Open(full_path);
        if (!tFile.Open())
        {
            wxMessageBox(wxString::Format(_("Unable to open file %s"), full_path)
                , _("General Reports Manager"), wxOK|wxICON_ERROR);
        }
        else
        {
            wxFileInputStream input(full_path);
            wxTextInputStream text(input, "\x09", wxConvUTF8);
            while (input.IsOk() && !input.Eof())
            {
                *html_text_ << text.ReadLine() << "\n";
            }
            button_Run_->Enable(true);
        }
    }

    //TODO:
}

void mmGeneralReportManager::OnLabelChanged(wxTreeEvent& event)
{
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(treeCtrl_->GetItemData(event.GetItem()));
    if (!iData) return;

    int id = iData->get_report_id();
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        report->REPORTNAME = event.GetLabel();
        Model_Report::instance().save(report);
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
            Model_Report::instance().remove(id);
            return true;
        }
    }
    return false;
}

void mmGeneralReportManager::OnMenuSelected(wxCommandEvent& event)
{
    int id = event.GetId();
    if (id == ID_NEW1)
    {
        wxString group_name;
        if (selectedItemId_ == root_)
        {
            group_name = wxGetTextFromUser(_("Enter the name for the new report group")
                , _("Add Report Group"), "");
            if (group_name.IsEmpty())
                return;
        }
        else
        {
            group_name = m_selectedGroup;
        }
        int i = Model_Report::instance().all().size();
        Model_Report::Data* report = Model_Report::instance().create();
        report->GROUPNAME = group_name;
        report->REPORTNAME = wxString::Format(_("New SQL Report %i"), i);
        report->CONTENTTYPE = "SQL";
        report->CONTENT = "select 'Hello World'";
        report->TEMPLATEPATH = "sample.html";
        Model_Report::instance().save(report);
    }
    else if (id == ID_NEW2)
    {
        m_reportType->SetValue(_("Lua"));
        tcSourceTxtCtrl_->ChangeValue("return \"Hello World\"");
    }
    else if (id == ID_DELETE)
    {
        MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(treeCtrl_->GetItemData(selectedItemId_));
        if (iData)
        {
            int report_id = iData->get_report_id();
            this->DeleteReport(report_id);
        }
    }
    fillControls();
}

void mmGeneralReportManager::OnSourceTxtChar(wxKeyEvent& event)
{
    if (wxGetKeyState(wxKeyCode('A')) && wxGetKeyState(WXK_CONTROL))
        tcSourceTxtCtrl_->SetSelection(-1, -1); //select all
    event.Skip();
}
