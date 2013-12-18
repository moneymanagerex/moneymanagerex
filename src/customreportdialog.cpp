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
#include <wx/stc/stc.h>

/////////////////////////////////////////////////////////////////////////////
// Minimal editor added by Troels K 2008-04-08
// Thanks to geralds for SetLexerXml() - http://wxforum.shadonet.com/viewtopic.php?t=7155

class MinimalEditor : public wxStyledTextCtrl
{
    enum
    {
        margin_id_lineno,
        margin_id_fold,
    };

public:
    MinimalEditor(wxWindow* parent, wxWindowID id = wxID_ANY) : wxStyledTextCtrl(parent, id)
    {
        SetProperty(wxT("fold"), wxT("1"));
        SetProperty(wxT("fold.comment"), wxT("1"));
        SetProperty(wxT("fold.compact"), wxT("1"));
        SetProperty(wxT("fold.preprocessor"), wxT("1"));
        SetProperty(wxT("fold.html"), wxT("1"));
        SetProperty(wxT("fold.html.preprocessor"), wxT("1"));

        SetMarginType(margin_id_lineno, wxSTC_MARGIN_NUMBER);
        SetMarginWidth(margin_id_lineno, 32);
        SetMarginWidth(margin_id_fold, 5);

        MarkerDefine(wxSTC_MARKNUM_FOLDER,        wxSTC_MARK_BOXPLUS, wxT("WHITE"), wxT("BLACK"));
        MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN,    wxSTC_MARK_BOXMINUS,  wxT("WHITE"), wxT("BLACK"));
        MarkerDefine(wxSTC_MARKNUM_FOLDERSUB,     wxSTC_MARK_VLINE,     wxT("WHITE"), wxT("BLACK"));
        MarkerDefine(wxSTC_MARKNUM_FOLDEREND,     wxSTC_MARK_BOXPLUSCONNECTED, wxT("WHITE"), wxT("BLACK"));
        MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUSCONNECTED, wxT("WHITE"), wxT("BLACK"));
        MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_TCORNER,     wxT("WHITE"), wxT("BLACK"));
        MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL,    wxSTC_MARK_LCORNER,     wxT("WHITE"), wxT("BLACK"));

        SetTabWidth(4);
        SetUseTabs(false);
        SetWrapMode(wxSTC_WRAP_WORD);
        SetWrapVisualFlags(wxSTC_WRAPVISUALFLAG_END);
    }
    virtual bool SetFont(const wxFont& font)
    {
        StyleSetFont(wxSTC_STYLE_DEFAULT, (wxFont&)font);
        return wxStyledTextCtrl::SetFont(font);
    }
#if 0
    void SetLexerXml()
    {
        SetLexer(wxSTC_LEX_XML);
        StyleSetForeground(wxSTC_H_DEFAULT, *wxBLACK);
        StyleSetForeground(wxSTC_H_TAG, *wxBLUE);
        StyleSetForeground(wxSTC_H_TAGUNKNOWN, *wxBLUE);
        StyleSetForeground(wxSTC_H_ATTRIBUTE, *wxRED);
        StyleSetForeground(wxSTC_H_ATTRIBUTEUNKNOWN, *wxRED);
        StyleSetBold(wxSTC_H_ATTRIBUTEUNKNOWN, true);
        StyleSetForeground(wxSTC_H_NUMBER, *wxBLACK);
        StyleSetForeground(wxSTC_H_DOUBLESTRING, *wxBLACK);
        StyleSetForeground(wxSTC_H_SINGLESTRING, *wxBLACK);
        StyleSetForeground(wxSTC_H_OTHER, *wxBLUE);
        StyleSetForeground(wxSTC_H_COMMENT, wxTheColourDatabase->Find(wxT("GREY")));
        StyleSetForeground(wxSTC_H_ENTITY, *wxRED);
        StyleSetBold(wxSTC_H_ENTITY, true);
        StyleSetForeground(wxSTC_H_TAGEND, *wxBLUE);
        StyleSetForeground(wxSTC_H_XMLSTART, *wxBLUE);
        StyleSetForeground(wxSTC_H_XMLEND, *wxBLUE);
        StyleSetForeground(wxSTC_H_CDATA, *wxRED);
    }
#endif
    void SetLexerSql()
    {
        StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(75, 75, 75));
        StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(220, 220, 220));
        SetMarginWidth(margin_id_lineno, 32);
        StyleClearAll();
        SetLexer(wxSTC_LEX_SQL);
        StyleSetForeground(wxSTC_SQL_WORD, wxColour(0, 150, 0));
        const wxString sqlwords =
            "asc by delete desc from group having insert into order select set update values where";
        SetKeyWords(0, sqlwords);
    }
    void SetLexerLua()
    {
        SetLexer(wxSTC_LEX_LUA);
        const wxString luawords =
            "function end if then do else for in return break while local repeat elseif and or not false true nil require";
        SetKeyWords(0, luawords);
        //TODO: https://code.google.com/p/wxamcl/source/browse/trunk/scriptedit.cpp?r=63
    }
    void SetLexerHtml()
    {
        SetLexer(wxSTC_LEX_HTML);
        SetMarginWidth(margin_id_lineno, 32);
        StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(75, 75, 75));
        StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(220, 220, 220));
        SetWrapMode(wxSTC_WRAP_WORD);
        StyleClearAll();
        StyleSetForeground(wxSTC_H_DOUBLESTRING, *wxRED);
        StyleSetForeground(wxSTC_H_SINGLESTRING, *wxRED);
        StyleSetForeground(wxSTC_H_ENTITY, *wxRED);
        StyleSetForeground(wxSTC_H_TAG, wxColour(0, 150, 0));
        StyleSetForeground(wxSTC_H_TAGUNKNOWN, wxColour(0, 150, 0));
        StyleSetForeground(wxSTC_H_ATTRIBUTE, wxColour(0, 0, 150));
        StyleSetForeground(wxSTC_H_ATTRIBUTEUNKNOWN, wxColour(0, 0, 150));
        StyleSetForeground(wxSTC_H_COMMENT, wxColour(150, 150, 150));
    }
protected:
    void OnMarginClick(wxStyledTextEvent&);
    void OnText(wxStyledTextEvent&);
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MinimalEditor, wxStyledTextCtrl)
    EVT_STC_MARGINCLICK(wxID_ANY, MinimalEditor::OnMarginClick)
    EVT_STC_CHANGE(wxID_ANY, MinimalEditor::OnText)
END_EVENT_TABLE()

void MinimalEditor::OnMarginClick(wxStyledTextEvent &event)
{
    if (event.GetMargin() == margin_id_fold)
    {
        int lineClick = LineFromPosition(event.GetPosition());
        int levelClick = GetFoldLevel(lineClick);
        if ((levelClick & wxSTC_FOLDLEVELHEADERFLAG) > 0)
        {
            ToggleFold(lineClick);
        }
    }
}

void MinimalEditor::OnText(wxStyledTextEvent& event)
{
    wxLogDebug(wxT("Modified"));
    event.Skip();
}

int titleTextWidth   = 200; // Determines width of Headings Textbox.
int sourceTextHeight = 200; // Determines height of Source Textbox.

IMPLEMENT_DYNAMIC_CLASS( mmGeneralReportManager, wxDialog )

BEGIN_EVENT_TABLE( mmGeneralReportManager, wxDialog )
    EVT_BUTTON(wxID_OPEN, mmGeneralReportManager::OnOpenTemplate)
    EVT_BUTTON(wxID_SAVE, mmGeneralReportManager::OnSaveTemplate)
    EVT_BUTTON(wxID_SAVEAS, mmGeneralReportManager::OnSaveTemplateAs)
    EVT_BUTTON(wxID_EXECUTE, mmGeneralReportManager::OnRun)
    EVT_BUTTON(wxID_REFRESH, mmGeneralReportManager::OnUpdateScript)
    EVT_BUTTON(wxID_CLOSE, mmGeneralReportManager::OnClose)
    EVT_TREE_SEL_CHANGED(wxID_ANY, mmGeneralReportManager::OnSelChanged)
    EVT_TREE_END_LABEL_EDIT(wxID_ANY, mmGeneralReportManager::OnLabelChanged)
    EVT_TREE_ITEM_MENU(wxID_ANY, mmGeneralReportManager::OnItemRightClick)
    EVT_MENU(wxID_ANY, mmGeneralReportManager::OnMenuSelected)
END_EVENT_TABLE()

mmGeneralReportManager::mmGeneralReportManager(wxWindow* parent)
: button_Open_()
, button_Save_()
, button_SaveAs_()
, button_Run_()
, button_Clear_()
, button_Update_()
, treeCtrl_()
, m_fileNameCtrl()
, m_outputHTML()
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("Custom Reports Manager"), wxDefaultPosition, wxSize(640, 480), style);
}

mmGeneralReportManager::~mmGeneralReportManager()
{
    MinimalEditor* me = (MinimalEditor*) FindWindow(ID_TEMPLATE);
    if (me) delete me;
    me = (MinimalEditor*) FindWindow(wxID_VIEW_DETAILS);
    if (me) delete me;
    wxNotebook* n = (wxNotebook*) FindWindow(ID_NOTEBOOK);
    if (n) delete n;
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
    treeCtrl_->SetEvtHandlerEnabled(false);
    treeCtrl_->DeleteAllItems();
    root_ = treeCtrl_->AddRoot(_("Custom Reports"));
    selectedItemId_ = root_;
    treeCtrl_->SetItemBold(root_, true);
    treeCtrl_->SetFocus();
    Model_Report::Data_Set records = Model_Report::instance().all(Model_Report::COL_GROUPNAME, Model_Report::COL_REPORTNAME);
    wxTreeItemId group;
    wxString group_name = "\x05";
    for (const auto& record : records)
    {
        bool no_group = record.GROUPNAME.empty();
        if (group_name != record.GROUPNAME && !no_group)
        {
            group = treeCtrl_->AppendItem(root_, record.GROUPNAME);
            treeCtrl_->SetItemData(group, new MyTreeItemData(-1, record.GROUPNAME));
            group_name = record.GROUPNAME;
        }
        wxTreeItemId item = treeCtrl_->AppendItem(no_group ? root_ : group, record.REPORTNAME);
        treeCtrl_->SetItemData(item, new MyTreeItemData(record.REPORTID, record.GROUPNAME));
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

    wxStaticText* type_label = new wxStaticText(this, ID_TYPELABEL
        , wxString::Format(_("Script type: %s"), "")
        , wxDefaultPosition, wxSize(titleTextWidth, -1));
    flex_sizer->Add(type_label, flags);
    flex_sizer->AddSpacer(1);

    long treeCtrlFlags = wxTR_EDIT_LABELS | wxTR_SINGLE | wxTR_HAS_BUTTONS;
#if defined (__WXWIN__)
    treeCtrlFlags = wxTR_EDIT_LABELS | wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_ROW_LINES;
#endif
    treeCtrl_ = new wxTreeCtrl(this, wxID_ANY
        , wxDefaultPosition, wxSize(titleTextWidth, titleTextWidth), treeCtrlFlags);

    headingPanelSizerH2->Add(flex_sizer, flags);
    headingPanelSizerH2->Add(treeCtrl_, flagsExpand);

    /****************************************
     Script Area
     ***************************************/
    // ListBox for source code
    wxBoxSizer* headingPanelSizerV3 = new wxBoxSizer(wxVERTICAL);
    headingPanelSizerH->Add(headingPanelSizerV3, flagsExpand);

    wxNotebook* editors_notebook = new wxNotebook(this
        , ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);
    wxPanel* script_tab = new wxPanel(editors_notebook, wxID_ANY);
    editors_notebook->InsertPage(ID_TAB1, script_tab, _("Script"));
    wxBoxSizer *script_sizer = new wxBoxSizer(wxVERTICAL);
    script_tab->SetSizer(script_sizer);
    headingPanelSizerV3->Add(editors_notebook, flagsExpand);

    MinimalEditor* m_scriptText = new MinimalEditor(script_tab, wxID_VIEW_DETAILS);
    m_scriptText->SetLexerSql();
    int font_size = this->GetFont().GetPointSize();
    wxFont teletype(font_size, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_scriptText->SetFont(teletype);

    m_scriptText->Connect(wxID_ANY, wxEVT_CHAR
        , wxKeyEventHandler(mmGeneralReportManager::OnSourceTxtChar), NULL, this);

    button_Update_ = new wxButton(script_tab, wxID_REFRESH, _("&Update"));
    wxBoxSizer* headingPanelSizerH3 = new wxBoxSizer(wxHORIZONTAL);
    headingPanelSizerH3->Add(button_Update_, flags);
    button_Update_->SetToolTip(_("Update the scrit data into DB."));

    button_Clear_ = new wxButton(script_tab, wxID_CLEAR);
    headingPanelSizerH3->Add(button_Clear_, flags);
    button_Clear_->SetToolTip(_("Clear the Source script area"));

    script_sizer->Add(m_scriptText, flagsExpand);
    script_sizer->Add(headingPanelSizerH3, flags.Center());

    //Template
    wxPanel* template_tab = new wxPanel(editors_notebook, wxID_ANY);
    editors_notebook->InsertPage(ID_TAB2, template_tab, _("Template"));
    wxBoxSizer *html_sizer = new wxBoxSizer(wxVERTICAL);
    template_tab->SetSizer(html_sizer);

    wxBoxSizer *file_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_fileNameCtrl = new wxTextCtrl(template_tab, wxID_FILE, wxEmptyString
        , wxDefaultPosition, wxSize(200, -1), wxTE_READONLY);
    file_sizer->Add(new wxStaticText(template_tab, wxID_STATIC, _("File Name:")), flags);
    file_sizer->Add(m_fileNameCtrl, flagsExpand);

    MinimalEditor* m_templateText = new MinimalEditor(template_tab, ID_TEMPLATE);
    m_templateText->SetLexerHtml();

    html_sizer->Add(file_sizer);
    html_sizer->Add(m_templateText, flagsExpand);
    wxBoxSizer* headingPanelSizerH4 = new wxBoxSizer(wxHORIZONTAL);
    html_sizer->Add(headingPanelSizerH4, flags.Center());

    button_Open_ = new wxButton(template_tab, wxID_OPEN, _("Open"));
    headingPanelSizerH4->Add(button_Open_, flags);
    button_Open_->SetToolTip(_("Locate and load a template file into the template area."));

    button_Save_ = new wxButton(template_tab, wxID_SAVE, _("Save"));
    headingPanelSizerH4->Add(button_Save_, flags);
    button_Save_->SetToolTip(_("Save the template to file."));

    button_SaveAs_ = new wxButton(template_tab, wxID_SAVEAS, _("Save As..."));
    headingPanelSizerH4->Add(button_SaveAs_, flags);
    button_SaveAs_->SetToolTip(_("Save the template to a new file and assign it with the script."));

    //Output
    wxPanel* out_tab = new wxPanel(editors_notebook, wxID_ANY);
    editors_notebook->InsertPage(ID_TAB3, out_tab, _("Output"));
    wxBoxSizer *out_sizer = new wxBoxSizer(wxVERTICAL);
    out_tab->SetSizer(out_sizer);
    m_outputHTML = new wxHtmlWindow(out_tab, ID_OUTPUT_WIN
        , wxDefaultPosition, wxDefaultSize
        , wxHW_SCROLLBAR_AUTO | wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    out_sizer->Add(m_outputHTML, flagsExpand);

    /****************************************
     Bottom Panel
     ***************************************/
    wxPanel* buttonPanel = new wxPanel( this, wxID_STATIC,  wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    mainBoxSizer->Add(buttonPanel, flags);

    wxBoxSizer* buttonPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonPanel->SetSizer(buttonPanelSizer);

    button_Run_ = new wxButton(buttonPanel, wxID_EXECUTE, _("&Run"));
    buttonPanelSizer->Add(button_Run_, flags);
    button_Run_->SetToolTip(_("Test script."));

    wxButton* button_Close = new wxButton( buttonPanel, wxID_CLOSE);
    buttonPanelSizer->Add(button_Close, flags);
    //button_Close->SetToolTip(_("Save changes before closing. Changes without Save will be lost."));

}

void mmGeneralReportManager::OnOpenTemplate(wxCommandEvent& /*event*/)
{
    wxString sScriptFileName = wxFileSelector( _("Load file:")
        , mmex::getPathUser(mmex::DIRECTORY), wxEmptyString, wxEmptyString
        , "File(*.html)|*.html"
        , wxFD_FILE_MUST_EXIST);
    if ( !sScriptFileName.empty() )
    {
        MinimalEditor* m_templateText = (MinimalEditor*) FindWindow(ID_TEMPLATE);
        m_templateText->SetEvtHandlerEnabled(false);
        wxTextFile reportFile(sScriptFileName);
        if (reportFile.Open())
        {
            while (! reportFile.Eof())
                m_templateText->AppendText(reportFile.GetNextLine() + "\n");
            
            reportFile.Close();
        }
        else
        {
            wxString msg = wxString() << _("Unable to open file.") << sScriptFileName << "\n\n";
            wxMessageBox(msg, _("General Reports Manager"), wxOK | wxICON_ERROR);
        }
        m_templateText->SetEvtHandlerEnabled(true);
    }
}

void mmGeneralReportManager::OnSaveTemplate(wxCommandEvent& /*event*/)
{
    //TODO:
}

void mmGeneralReportManager::OnSaveTemplateAs(wxCommandEvent& /*event*/)
{
    //TODO:
}

void mmGeneralReportManager::OnUpdateScript(wxCommandEvent& /*event*/)
{
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(treeCtrl_->GetItemData(selectedItemId_));
    if (!iData) return;

    int id = iData->get_report_id();
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        MinimalEditor* m_scriptText = (MinimalEditor*) FindWindow(wxID_VIEW_DETAILS);
        report->CONTENT = m_scriptText->GetValue();
        m_scriptText->ChangeValue(report->CONTENT);
        Model_Report::instance().save(report);
    }
}

void mmGeneralReportManager::OnRun(wxCommandEvent& /*event*/)
{
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(treeCtrl_->GetItemData(selectedItemId_));
    if (!iData) return;
    
    int id = iData->get_report_id();
    m_selectedGroup = iData->get_group_name();
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        wxNotebook* n = (wxNotebook*) FindWindow(ID_NOTEBOOK);
        n->SetSelection(ID_TAB3);
        m_outputHTML->ClearBackground();
        mmGeneralReport gr(report);
        m_outputHTML->SetPage(gr.getHTMLText());
    }
}

void mmGeneralReportManager::OnItemRightClick(wxTreeEvent& event)
{
    wxTreeItemId id = event.GetItem();
    treeCtrl_ ->SelectItem(id);
    int report_id = -1;
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(treeCtrl_->GetItemData(id));
    if (iData) report_id = iData->get_report_id();

    wxMenu* customReportMenu = new wxMenu;
    customReportMenu->Append(ID_NEW1, _("New SQL Custom Report"));
    //TODO: Hided till release the workload
    //customReportMenu->Append(ID_NEW2, _("New Lua Custom Report"));
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
    wxStaticText* script_label = (wxStaticText*) FindWindow(ID_TYPELABEL);
    MinimalEditor* m_scriptText = (MinimalEditor*) FindWindow(wxID_VIEW_DETAILS);
    MinimalEditor* m_templateText = (MinimalEditor*) FindWindow(ID_TEMPLATE);
    m_scriptText->ChangeValue("");
    m_fileNameCtrl->ChangeValue("");
    m_templateText->ChangeValue("");
    script_label->SetLabel("");
    wxNotebook* n = (wxNotebook*) FindWindow(ID_NOTEBOOK);
    n->SetSelection(0);
    button_Run_->Enable(false);
    button_Update_->Enable(false);
    button_Clear_->Enable(false);

    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(treeCtrl_->GetItemData(event.GetItem()));
    if (!iData) return;

    int id = iData->get_report_id();
    m_selectedGroup = iData->get_group_name();
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        script_label->SetLabel(wxString::Format(_("Script Type: %s"), report->CONTENTTYPE));
        m_fileNameCtrl->ChangeValue(report->TEMPLATEPATH);
        m_scriptText->ChangeValue(report->CONTENT);
        m_scriptText->SetLexerSql();

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
            wxFileInputStream input(full_path);
            wxTextInputStream text(input, "\x09", wxConvUTF8);
            while (input.IsOk() && !input.Eof())
            {
                m_templateText->AppendText(text.ReadLine() + "\n");
            }
            button_Run_->Enable(true);
            button_Update_->Enable(true);
            button_Clear_->Enable(true);
        }
    }
}

void mmGeneralReportManager::OnLabelChanged(wxTreeEvent& event)
{
    MyTreeItemData* iData = dynamic_cast<MyTreeItemData*>(treeCtrl_->GetItemData(event.GetItem()));
    if (!iData) return;

    wxString label = event.GetLabel();
    int id = iData->get_report_id();
    Model_Report::Data * report = Model_Report::instance().get(id);
    if (report)
    {
        report->REPORTNAME = label;
        Model_Report::instance().save(report);
    }
    else if (event.GetItem() != root_)
    {
        Model_Report::Data_Set reports = Model_Report::instance().find(Model_Report::GROUPNAME(m_selectedGroup));
        for (auto & record : reports)
        {
            record.GROUPNAME = label;
        }
        Model_Report::instance().save(reports);
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
    if (id == ID_NEW1 || id == ID_NEW2)
    {
        bool sql = id == ID_NEW1;
        wxString group_name;
        if (selectedItemId_ == root_)
        {
            group_name = wxGetTextFromUser(_("Enter the name for the new report group")
                , _("Add Report Group"), "");
        }
        else
        {
            group_name = m_selectedGroup;
        }
        int i = Model_Report::instance().all().size();
        Model_Report::Data* report = Model_Report::instance().create();
        report->GROUPNAME = group_name;
        report->REPORTNAME = wxString::Format(id == ID_NEW1 ? _("New SQL Report %i") : _("New Lua Report %i"), i);
        report->CONTENTTYPE = sql ? "SQL" : "Lua";
        report->CONTENT = sql ? "select 'Hello World' as COLUMN1" : "return \"Hello World\"";
        report->TEMPLATEPATH = "sample.html";
        Model_Report::instance().save(report);
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
    MinimalEditor* m_scriptText = (MinimalEditor*) FindWindow(wxID_VIEW_DETAILS);
    if (wxGetKeyState(wxKeyCode('A')) && wxGetKeyState(WXK_CONTROL))
        m_scriptText->SetSelection(-1, -1); //select all
    event.Skip();
}

void mmGeneralReportManager::OnClose(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}
