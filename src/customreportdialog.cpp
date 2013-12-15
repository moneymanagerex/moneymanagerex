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
#include "paths.h"
#include "util.h"
#include "model/Model_Infotable.h"
#include "model/Model_Report.h"

int titleTextWidth   = 200; // Determines width of Headings Textbox.
int sourceTextHeight = 200; // Determines height of Source Textbox.
enum
{
    HEADING_ONLY = wxID_HIGHEST + 1,
    SUB_REPORT
};

IMPLEMENT_DYNAMIC_CLASS( mmGeneralReportManager, wxDialog )

BEGIN_EVENT_TABLE( mmGeneralReportManager, wxDialog )
    EVT_BUTTON(wxID_OPEN,  mmGeneralReportManager::OnOpen)
    EVT_BUTTON(wxID_SAVE,  mmGeneralReportManager::OnSave)
    EVT_BUTTON(wxID_REFRESH, mmGeneralReportManager::OnRun)
    EVT_BUTTON(wxID_CLEAR, mmGeneralReportManager::OnClear)
    EVT_BUTTON(wxID_CLOSE, mmGeneralReportManager::OnClose)
    EVT_CHECKBOX(HEADING_ONLY, mmGeneralReportManager::OnCheckedHeading)
    EVT_CHECKBOX(SUB_REPORT,   mmGeneralReportManager::OnCheckedSubReport)
    EVT_TEXT( wxID_FILE, mmGeneralReportManager::OnTextChangeHeading)
    EVT_TEXT( wxID_VIEW_DETAILS,       mmGeneralReportManager::OnTextChangeSubReport)
    //EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY, mmGeneralReportManager::OnItemRightClick)
    EVT_TREE_SEL_CHANGED(wxID_ANY, mmGeneralReportManager::OnSelChanged)
    EVT_TREE_END_LABEL_EDIT(wxID_ANY, mmGeneralReportManager::OnLabelChanged)
    EVT_TREE_ITEM_MENU(wxID_ANY, mmGeneralReportManager::OnItemRightClick)
    //EVT_TREE_ITEM_ACTIVATED(wxID_ANY,  mmGeneralReportManager::OnDoubleClicked)
    EVT_MENU(wxID_ANY, mmGeneralReportManager::OnMenuSelected)
END_EVENT_TABLE()

mmGeneralReportManager::mmGeneralReportManager(wxWindow* parent)
: tcSourceTxtCtrl_()
, button_Open_()
, button_Save_()
, button_Run_()
, button_Clear_()
, reportTitleTxtCtrl_()
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

    treeCtrl_->DeleteAllItems();
    root_ = treeCtrl_->AddRoot(_("Custom Reports"));
    selectedItemId_ = root_;
    treeCtrl_->SetItemBold(root_, true);
    treeCtrl_->SetFocus();
    Model_Report::Data_Set reports = Model_Report::instance().all(Model_Report::COL_GROUPNAME);
    wxTreeItemId maincat; //TODO:
    wxString group_name;
    for (const auto& report : reports)
    {
        if (group_name != report.GROUPNAME)
        {
            maincat = treeCtrl_->AppendItem(root_, report.GROUPNAME);
            group_name = report.GROUPNAME;
        }
        treeCtrl_->AppendItem(maincat, report.REPORTNAME, 1, 1);
    }
    treeCtrl_->Expand(root_);
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

    flex_sizer->Add(new wxStaticText( this, wxID_ANY, _("Script type:")), flags);
    flex_sizer->AddSpacer(1);

    reportTitleTxtCtrl_ = new wxTextCtrl( this, wxID_FILE, "",
        wxDefaultPosition, wxSize(titleTextWidth,-1));
    reportTitleTxtCtrl_->SetToolTip(_("Report Title is used as the file name of the SQL script."));

    long treeCtrlFlags = wxTR_EDIT_LABELS | wxTR_SINGLE | wxTR_HAS_BUTTONS;
#if defined (__WXWIN__)
    treeCtrlFlags = wxTR_EDIT_LABELS | wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_ROW_LINES;
#endif
    treeCtrl_ = new wxTreeCtrl( this, wxID_ANY,
    wxDefaultPosition, wxSize(titleTextWidth, titleTextWidth), treeCtrlFlags );

    headingPanelSizerH2->Add(flex_sizer, flags);
    headingPanelSizerH2->Add(reportTitleTxtCtrl_, flags);
    headingPanelSizerH2->Add(treeCtrl_, flagsExpand);

    /****************************************
     Script Area
     ***************************************/
    // ListBox for source code
    wxBoxSizer* headingPanelSizerV3 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* headingPanelSizerH4 = new wxBoxSizer(wxHORIZONTAL);
    headingPanelSizerH->Add(headingPanelSizerV3, flagsExpand);

    wxNotebook* editors_notebook = new wxNotebook(this
        , wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);
    wxPanel* script_tab = new wxPanel(editors_notebook, wxID_ANY);
    editors_notebook->AddPage(script_tab, _("Script"));
    wxBoxSizer *script_sizer = new wxBoxSizer(wxVERTICAL);
    script_tab->SetSizer(script_sizer);
    headingPanelSizerV3->Add(editors_notebook, flagsExpand);

    tcSourceTxtCtrl_ = new wxTextCtrl(script_tab, wxID_VIEW_DETAILS, ""
        , wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxHSCROLL | wxTE_NOHIDESEL);
    tcSourceTxtCtrl_->Connect(wxID_ANY, wxEVT_CHAR
        , wxKeyEventHandler(mmGeneralReportManager::OnSourceTxtChar), NULL, this);
    int font_size = this->GetFont().GetPointSize();
    wxFont teletype( font_size, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
    tcSourceTxtCtrl_->SetFont(teletype);
    script_sizer->Add(tcSourceTxtCtrl_, flagsExpand);
    script_sizer->Add(headingPanelSizerH4, flags.Center());

    wxPanel* html_tab = new wxPanel(editors_notebook, wxID_ANY);
    editors_notebook->AddPage(html_tab, _("html"));
    wxBoxSizer *html_sizer = new wxBoxSizer(wxVERTICAL);
    html_tab->SetSizer(html_sizer);
    html_text_ = new wxTextCtrl(html_tab, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxHSCROLL | wxTE_NOHIDESEL);
    html_sizer->Add(html_text_, flagsExpand);


    button_Open_ = new wxButton(script_tab, wxID_OPEN);
    headingPanelSizerH4->Add(button_Open_, flags);
    button_Open_->SetToolTip(_("Locate and load a script file into the script area."));

    button_Save_ = new wxButton(script_tab, wxID_SAVE);
    headingPanelSizerH4->Add(button_Save_, flags);
    button_Save_->SetToolTip(_("Save the script to file name set by the Report Title."));

    button_Clear_ = new wxButton(script_tab, wxID_CLEAR);
    headingPanelSizerH4->Add(button_Clear_, flags);
    button_Clear_->SetToolTip(_("Clear the Source script area"));

    /****************************************
     Bottom Panel
     ***************************************/
    wxPanel* buttonPanel = new wxPanel( this, wxID_STATIC,  wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    mainBoxSizer->Add(buttonPanel, flags);

    wxBoxSizer* buttonPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonPanel->SetSizer(buttonPanelSizer);

    button_Run_ = new wxButton( buttonPanel, wxID_REFRESH, _("&Run"));
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
        , "File(*.*)|*.*" 
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
            tcSourceTxtCtrl_->SetValue(reportText);
            reportFile.Close();
            reportTitleTxtCtrl_->SetLabel(selectedFileName.GetName());
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

bool mmGeneralReportManager::SaveCustomReport()
{
    wxString reportfileName = reportTitleTxtCtrl_->GetValue();

    if (reportfileName.IsEmpty())
    {
        wxMessageBox(_("Please supply the Report Title before saving"),
            "Under Constraction", wxOK|wxICON_WARNING);
        return false;
    }
    //TODO:
    return true;
}

void mmGeneralReportManager::OnRun(wxCommandEvent& /*event*/)
{
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

void mmGeneralReportManager::SetDialogBoxForHeadings(bool bHeading)
{
    button_Open_->Enable(tcSourceTxtCtrl_->IsEmpty());
    button_Run_->Enable(!tcSourceTxtCtrl_->IsEmpty());
    button_Clear_->Enable(!tcSourceTxtCtrl_->IsEmpty());
}

void mmGeneralReportManager::OnCheckedHeading(wxCommandEvent& /*event*/)
{
    button_Save_->Enable(!reportTitleTxtCtrl_->IsEmpty());
}

void mmGeneralReportManager::OnCheckedSubReport(wxCommandEvent& /*event*/)
{
    button_Save_->Enable();
}

void mmGeneralReportManager::OnTextChangeHeading(wxCommandEvent& /*event*/)
{
    button_Save_->Enable();
}

void mmGeneralReportManager::OnTextChangeSubReport(wxCommandEvent& /*event*/)
{
    button_Save_->Enable(!reportTitleTxtCtrl_->IsEmpty());
    button_Run_->Enable(!tcSourceTxtCtrl_->IsEmpty());
    button_Open_->Enable(tcSourceTxtCtrl_->IsEmpty());
}

void mmGeneralReportManager::OnItemRightClick(wxTreeEvent& event)
{
    wxTreeItemId id = event.GetItem();
    treeCtrl_ ->SelectItem(id);

    wxMenu* customReportMenu = new wxMenu;
    customReportMenu->Append(1, _("New SQL Custom Report"));
    customReportMenu->Append(2, _("New Lua Custom Report"));
    customReportMenu->AppendSeparator();
    customReportMenu->Append(wxID_DELETE, _("Delete Custom Report"));
    PopupMenu(customReportMenu);
    delete customReportMenu;
}

void mmGeneralReportManager::OnSelChanged(wxTreeEvent& event)
{
    mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(treeCtrl_->GetItemData(event.GetItem()));
    if (!iData) return;

    wxStaticText* st = (wxStaticText*)FindWindow(wxID_PROPERTIES);
    st->SetLabel(wxString::Format(_("Custom script:")));

    //TODO:
}

void mmGeneralReportManager::OnLabelChanged(wxTreeEvent& event)
{
    //TODO:
}

bool mmGeneralReportManager::DeleteCustomSqlReport()
{
    wxString msg = wxString() << _("Delete the Custom Report Title:")
                              << "\n\n"
                              << "Under Constraction";
    int iError = wxMessageBox(msg, "Under Constraction", wxYES_NO | wxICON_QUESTION);
    if ( iError == wxYES )
    {
        //TODO:
    }
    return (iError == wxYES);
}

void mmGeneralReportManager::OnMenuSelected(wxCommandEvent& event)
{
    int id = event.GetId();
    if (id == 1)
    {
        reportTitleTxtCtrl_->SetValue(_("New SQL Custom Report"));
        tcSourceTxtCtrl_->ChangeValue("select 'Hello World'");
    }
    if (id == 2)
    {
        reportTitleTxtCtrl_->SetValue(_("New Lua Custom Report"));
        tcSourceTxtCtrl_->ChangeValue("return \"Hello World\"");
    }
    else if (id == wxID_DELETE)
    {
        //if (navCtrlUpdateRequired_) iSelectedId_--;
    }
}

void mmGeneralReportManager::OnSourceTxtChar(wxKeyEvent& event)
{
    if (wxGetKeyState(wxKeyCode('A')) && wxGetKeyState(WXK_CONTROL))
        tcSourceTxtCtrl_->SetSelection(-1, -1); //select all
    event.Skip();
}
