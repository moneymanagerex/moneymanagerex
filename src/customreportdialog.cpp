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
#include "customreportindex.h"
#include "paths.h"
#include "util.h"

int titleTextWidth   = 200; // Determines width of Headings Textbox.
int sourceTextHeight = 200; // Determines height of Source Textbox.

IMPLEMENT_DYNAMIC_CLASS( mmCustomSQLDialog, wxDialog )

BEGIN_EVENT_TABLE( mmCustomSQLDialog, wxDialog )
    EVT_BUTTON(wxID_OPEN,  mmCustomSQLDialog::OnOpen)
    EVT_BUTTON(wxID_SAVE,  mmCustomSQLDialog::OnSave)
    EVT_BUTTON(wxID_REFRESH, mmCustomSQLDialog::OnRun)
    EVT_BUTTON(wxID_CLEAR, mmCustomSQLDialog::OnClear)
    EVT_BUTTON(wxID_CLOSE, mmCustomSQLDialog::OnClose)
    EVT_CHECKBOX(DIALOG_CUSTOM_SQL_CHKBOX_HEADING_ONLY, mmCustomSQLDialog::OnCheckedHeading)
    EVT_CHECKBOX(DIALOG_CUSTOM_SQL_CHKBOX_SUB_REPORT,   mmCustomSQLDialog::OnCheckedSubReport)
    EVT_TEXT( wxID_FILE, mmCustomSQLDialog::OnTextChangeHeading)
    EVT_TEXT( wxID_VIEW_DETAILS,       mmCustomSQLDialog::OnTextChangeSubReport)
    //EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY, mmCustomSQLDialog::OnItemRightClick)
    EVT_TREE_SEL_CHANGED(wxID_ANY, mmCustomSQLDialog::OnSelChanged)
    EVT_TREE_END_LABEL_EDIT(wxID_ANY, mmCustomSQLDialog::OnLabelChanged)
    EVT_TREE_ITEM_MENU(wxID_ANY, mmCustomSQLDialog::OnItemRightClick)
    //EVT_TREE_ITEM_ACTIVATED(wxID_ANY,  mmCustomSQLDialog::OnDoubleClicked)
    EVT_MENU(wxID_ANY, mmCustomSQLDialog::OnMenuSelected)
    EVT_TIMER(wxID_ANY, mmCustomSQLDialog::ShowCursorCoordinates)
END_EVENT_TABLE()

mmCustomSQLDialog::mmCustomSQLDialog(CustomReportIndex* reportIndex, const wxString& customSqlReportSelectedItem, wxWindow* parent,
                                     wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
: reportIndex_(reportIndex)
, tcSourceTxtCtrl_()
, customSqlReportSelectedItem_(customSqlReportSelectedItem)
, navCtrlUpdateRequired_(false)
, newFileCreated_(true)
, parent_(parent)
, edit_(false)
, newload_(false)
{
    Create(parent_, id, caption, pos, size, style);
}

mmCustomSQLDialog::~mmCustomSQLDialog()
{
    timer_->Stop();
}


bool mmCustomSQLDialog::Create( wxWindow* parent, wxWindowID id,
                        const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_NORMAL, WXK_F9, wxID_REFRESH);
    wxAcceleratorTable accel(1, entries);
    SetAcceleratorTable(accel);

    Connect(wxID_REFRESH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(mmCustomSQLDialog::OnRun));

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());
    Centre();

    if (reportIndex_->ValidTitle())
        edit_ = reportIndex_->GetSelectedTitleSelection(customSqlReportSelectedItem_);

    iSelectedId_ = reportIndex_->GetCustomReportId();
    timer_ = new wxTimer(this, wxID_ANY);
    timer_->Start(INTERVAL);

    fillControls();
    return TRUE;
}

void mmCustomSQLDialog::fillControls()
{
    button_Save_->Disable(); // Will be activated when changes detected.

    if (edit_)
    {
        m_radio_box_->SetSelection(reportIndex_->CurrentReportFileType() == "LUA");
        reportTitleTxtCtrl_->ChangeValue(reportIndex_->CurrentReportTitle());
        if (reportIndex_->CurrentReportFileName().IsEmpty())
        {
            headingOnlyCheckBox_->SetValue(true);
            SetDialogBoxForHeadings(true);
        }
        else
        {
            loadedFileName_ = reportIndex_->CurrentReportFileName(false);
            subMenuCheckBox_->SetValue(reportIndex_->ReportIsSubReport());

            if (wxFileExists(reportIndex_->CurrentReportFileName()))
            {
                wxString sSQLData;
                reportIndex_->GetReportFileData(sSQLData);
                tcSourceTxtCtrl_->ChangeValue(sSQLData);
            }
        }
        button_Open_->Disable();
    }
    else
    {
        button_Run_->Disable();
        iSelectedId_ = -1;
    }

    treeCtrl_->DeleteAllItems();
    wxTreeItemId root_ = treeCtrl_->AddRoot(_("Custom Reports"));

    if (reportIndex_->HasActiveReports())
    {
        int reportNumber = -1;
        wxString reportNumberStr;
        wxTreeItemId customSqlReportRootItem;
        wxTreeItemId customReportSelectedItem;
        reportIndex_->ResetReportsIndex();
        wxString reportTitle = reportIndex_->NextReportTitle();
        while (reportIndex_->ValidTitle())
        {
            wxTreeItemId customReportItem;
            if (reportIndex_->ReportIsSubReport() && reportNumber >= 0 )
            {
                customReportItem = treeCtrl_->AppendItem(customSqlReportRootItem, reportTitle);
            }
            else
            {
                customReportItem = treeCtrl_->AppendItem(root_, reportTitle);
                customSqlReportRootItem = customReportItem;
            }
            reportNumberStr.Printf("Custom_Report_%d", ++reportNumber);
            treeCtrl_->SetItemData(customReportItem, new mmTreeItemData(reportNumberStr));

            if (reportNumber == iSelectedId_) customReportSelectedItem = customReportItem;

            reportTitle = reportIndex_->NextReportTitle();
        }
        treeCtrl_->ExpandAll();
        if (customReportSelectedItem) treeCtrl_->SelectItem(customReportSelectedItem);
    }
}

void mmCustomSQLDialog::CreateControls()
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
    wxString choices[] = { _("SQL"), _("Lua")};
    int num = sizeof(choices) / sizeof(wxString);
    m_radio_box_ = new wxRadioBox(this, wxID_STATIC, ""
        , wxDefaultPosition, wxDefaultSize, num, choices, 2, wxRA_SPECIFY_COLS);
    flex_sizer->Add(m_radio_box_, flags.Center());

    headingOnlyCheckBox_ = new wxCheckBox(this, DIALOG_CUSTOM_SQL_CHKBOX_HEADING_ONLY, _("Heading"));
    flex_sizer->Add(headingOnlyCheckBox_, flags);

    subMenuCheckBox_ = new wxCheckBox( this, DIALOG_CUSTOM_SQL_CHKBOX_SUB_REPORT, _("Sub-Menu"));
    flex_sizer->Add(subMenuCheckBox_, flags);

    flex_sizer->Add(new wxStaticText( this, wxID_ANY, _("Report Title:")), flags);
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

    headingPanelSizerV3->Add(new wxStaticText( this, wxID_PROPERTIES, _("Custom script:")), flags);
    tcSourceTxtCtrl_ = new wxTextCtrl( this, wxID_VIEW_DETAILS, "",
        wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxHSCROLL|wxTE_NOHIDESEL );
    tcSourceTxtCtrl_->Connect(wxID_ANY, wxEVT_CHAR,
        wxKeyEventHandler(mmCustomSQLDialog::OnSourceTxtChar), NULL, this);
    int font_size = this->GetFont().GetPointSize();
    wxFont teletype( font_size, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
    tcSourceTxtCtrl_->SetFont(teletype);
    headingPanelSizerV3->Add(tcSourceTxtCtrl_, flagsExpand);
    headingPanelSizerV3->Add(headingPanelSizerH4, flags.Center());

    button_Open_ = new wxButton( this, wxID_OPEN);
    headingPanelSizerH4->Add(button_Open_, flags);
    button_Open_->SetToolTip(_("Locate and load a script file into the script area."));

    button_Save_ = new wxButton( this, wxID_SAVE);
    headingPanelSizerH4->Add(button_Save_, flags);
    button_Save_->SetToolTip(_("Save the script to file name set by the Report Title."));

    button_Clear_ = new wxButton( this, wxID_CLEAR);
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

wxString mmCustomSQLDialog::sScript()
{
    return sQuery_;
}

wxString mmCustomSQLDialog::sReportTitle()
{
    return reportTitleTxtCtrl_->GetValue();
}

wxString mmCustomSQLDialog::sSctiptType()
{
    int i = m_radio_box_->GetSelection();
    if (i == 0)
        return "SQL";
    else
        return "LUA";
}

void mmCustomSQLDialog::OnOpen(wxCommandEvent& /*event*/)
{
    wxString sScriptFileName = wxFileSelector( sSctiptType()=="SQL" ?
        _("Load Custom SQL file:") : _("Load Custom Lua file:"),
        mmex::getPathUser(mmex::DIRECTORY), wxEmptyString, wxEmptyString,
        sSctiptType()=="SQL" ? "SQL File(*.sql)|*.sql" : "Lua File(*.lua)|*.lua"
        , wxFD_FILE_MUST_EXIST);
    if ( !sScriptFileName.empty() )
    {
        wxFileName selectedFileName(sScriptFileName);
        loadedFileName_ = selectedFileName.GetFullName();
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
            newFileCreated_ = false;
            reportFile.Close();
            reportTitleTxtCtrl_->SetLabel(selectedFileName.GetName());
            newload_ = true;
        }
        else
        {
            wxString msg = wxString() << _("Unable to open file.") << sScriptFileName << "\n\n";
            wxMessageBox(msg,reportIndex_->UserDialogHeading(),wxOK|wxICON_ERROR);
        }
    }
}

void mmCustomSQLDialog::OnSave(wxCommandEvent& /*event*/)
{
    if (SaveCustomReport() && navCtrlUpdateRequired_)
        fillControls();
}

bool mmCustomSQLDialog::SaveCustomReport()
{
    wxString reportfileName = reportTitleTxtCtrl_->GetValue();

    if (reportfileName.IsEmpty())
    {
        wxMessageBox(_("Please supply the Report Title before saving"),
            reportIndex_->UserDialogHeading(), wxOK|wxICON_WARNING);
        return false;
    }

    reportfileName.Replace(" ", "_"); // Replace spaces with underscore character
    reportfileName += sSctiptType() == "SQL" ? ".sql" : ".lua";  // Add the file extenstion

    if (reportfileName == loadedFileName_ && !newload_)
    {
        edit_ = true;
    }
    else if (loadedFileName_.IsEmpty())
    {
        loadedFileName_ = reportfileName;
        navCtrlUpdateRequired_ = true;
    }

    if ( !edit_ && reportIndex_->ReportListHasItems() )
    {
        reportIndex_->GetUserTitleSelection(_(" entry location:\n\nOK....: Insert before selected entry\nCancel: Add at bottom of List."));
    }

    if ( headingOnlyCheckBox_->IsChecked() )
    {
        reportIndex_->AddReportTitle(reportTitleTxtCtrl_->GetValue(), edit_);
        navCtrlUpdateRequired_ = !edit_;
    }
    else
    {
        // If we have a filename, and we have something to save, save the file.
        wxString sqlSource = tcSourceTxtCtrl_->GetValue();
        if ( !reportfileName.empty() && !sqlSource.empty() )
        {
            reportfileName.Replace(":", "_"); // else windows 7 will hide the filename.
            wxTextFile tfSourceFile(mmex::getPathUser(mmex::DIRECTORY) + reportfileName);

            // If the file does not exist and cannot be created, throw an error
            if ( !tfSourceFile.Exists() && !tfSourceFile.Create() )
            {
                wxMessageBox(_("Unable to write to file."),reportIndex_->UserDialogHeading(), wxOK|wxICON_ERROR);
                return false;
            }

            // if the file does exist, then update the file contents to the new value.
            if (tfSourceFile.Exists())
            {
                // Make sure the file is open
                if (! tfSourceFile.IsOpened())
                {
                    tfSourceFile.Open();
                }
                tfSourceFile.Clear();
                tfSourceFile.AddLine(sqlSource);
            }
            tfSourceFile.Write();
            tfSourceFile.Close();
        }

        if (reportIndex_->ReportIsSubReport() != subMenuCheckBox_->GetValue())
        {
            navCtrlUpdateRequired_ = true;
        }
        // update the index file
        reportIndex_->AddReportTitle(reportTitleTxtCtrl_->GetValue(), edit_, reportfileName, subMenuCheckBox_->GetValue());
        iSelectedId_ = reportIndex_->GetCustomReportId();
        navCtrlUpdateRequired_ = navCtrlUpdateRequired_ || !edit_;
        loadedFileName_ = reportfileName;
        newload_ = false;
    }

    button_Save_->Disable();
    edit_ = true;
    return true;
}

void mmCustomSQLDialog::OnRun(wxCommandEvent& /*event*/)
{
   if (tcSourceTxtCtrl_->IsEmpty()) return;
   sQuery_ = tcSourceTxtCtrl_->GetValue();
   EndModal(wxID_MORE);
}

void mmCustomSQLDialog::OnClear(wxCommandEvent& /*event*/)
{
    tcSourceTxtCtrl_->Clear();
    button_Save_->Disable();
    button_Run_->Disable();
    if (! headingOnlyCheckBox_->GetValue())
        button_Open_->Enable();
    tcSourceTxtCtrl_->SetFocus();
}

void mmCustomSQLDialog::OnClose(wxCommandEvent& /*event*/)
{
    if (navCtrlUpdateRequired_ && !button_Save_->IsEnabled())
        EndModal(wxID_OK);
    else
        EndModal(wxID_CANCEL);
}

void mmCustomSQLDialog::SetDialogBoxForHeadings(bool bHeading)
{
    headingOnlyCheckBox_->Enable(tcSourceTxtCtrl_->IsEmpty());
    headingOnlyCheckBox_->SetValue(bHeading && tcSourceTxtCtrl_->IsEmpty());
    subMenuCheckBox_->Enable( !headingOnlyCheckBox_->GetValue());
    tcSourceTxtCtrl_->Enable(!headingOnlyCheckBox_->GetValue());
    button_Open_->Enable(!headingOnlyCheckBox_->GetValue() && tcSourceTxtCtrl_->IsEmpty());
    button_Run_->Enable(!headingOnlyCheckBox_->GetValue() && !tcSourceTxtCtrl_->IsEmpty());
    button_Clear_->Enable(!headingOnlyCheckBox_->GetValue() && !tcSourceTxtCtrl_->IsEmpty());
}

void mmCustomSQLDialog::OnCheckedHeading(wxCommandEvent& /*event*/)
{
    button_Save_->Enable(!reportTitleTxtCtrl_->IsEmpty());

    SetDialogBoxForHeadings(headingOnlyCheckBox_->IsChecked());
}

void mmCustomSQLDialog::OnCheckedSubReport(wxCommandEvent& /*event*/)
{
    button_Save_->Enable();
    headingOnlyCheckBox_->Enable(!subMenuCheckBox_->GetValue());
    navCtrlUpdateRequired_ = true;
}

void mmCustomSQLDialog::OnTextChangeHeading(wxCommandEvent& /*event*/)
{
    button_Save_->Enable();
    edit_ = false;          // Allow saving as a new file name.
    navCtrlUpdateRequired_ = !edit_;
}

void mmCustomSQLDialog::OnTextChangeSubReport(wxCommandEvent& /*event*/)
{
    button_Save_->Enable(!reportTitleTxtCtrl_->IsEmpty());
    button_Run_->Enable(!tcSourceTxtCtrl_->IsEmpty());
    button_Open_->Enable(tcSourceTxtCtrl_->IsEmpty());
    navCtrlUpdateRequired_ = !edit_;
}

void mmCustomSQLDialog::OnItemRightClick(wxTreeEvent& event)
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

void mmCustomSQLDialog::OnSelChanged(wxTreeEvent& event)
{
    mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(treeCtrl_->GetItemData(event.GetItem()));
    if (!iData) return;

    wxStaticText* st = (wxStaticText*)FindWindow(wxID_PROPERTIES);
    st->SetLabel(wxString::Format(_("Custom script:")));

    iSelectedId_ = reportIndex_->GetCustomReportId();
    reportIndex_->GetSelectedTitleSelection(iData->getString());

    loadedFileName_ = reportIndex_->CurrentReportFileName(false);
    if (reportIndex_->CurrentReportFileName().IsEmpty())
    {
        tcSourceTxtCtrl_->ChangeValue("");
        headingOnlyCheckBox_->SetValue(true);
    }
    else
    {
        subMenuCheckBox_->SetValue(reportIndex_->ReportIsSubReport());

        wxString sSQLData;
        reportIndex_->GetReportFileData(sSQLData);
        tcSourceTxtCtrl_->ChangeValue(sSQLData);
        if (reportIndex_->CurrentReportFileType() == "LUA")
            m_radio_box_->SetSelection(1);
        else
            m_radio_box_->SetSelection(0);
        edit_ = true;
    }
    reportTitleTxtCtrl_->ChangeValue(reportIndex_->CurrentReportTitle());
    SetDialogBoxForHeadings(!subMenuCheckBox_->IsChecked());
}

void mmCustomSQLDialog::OnLabelChanged(wxTreeEvent& event)
{
    edit_ = true;
    wxString sOldLabel = reportIndex_->CurrentReportTitle();
    wxString sLabel = event.GetLabel();
    wxString reportNumberStr = wxString::Format("Custom_Report_%d", iSelectedId_);

    if (!sLabel.IsEmpty() && sLabel!=sOldLabel)
    {
        treeCtrl_->SetItemData(event.GetItem(), new mmTreeItemData(reportNumberStr));
        reportTitleTxtCtrl_->ChangeValue(sLabel);
        wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, wxID_SAVE);
        OnSave(evt);
    }
    navCtrlUpdateRequired_ = true;
}

bool mmCustomSQLDialog::DeleteCustomSqlReport()
{
    wxString msg = wxString() << _("Delete the Custom Report Title:")
                              << "\n\n"
                              << reportIndex_->CurrentReportTitle();
    int iError = wxMessageBox(msg ,reportIndex_->UserDialogHeading(),wxYES_NO|wxICON_QUESTION);
    if ( iError == wxYES )
    {
        wxString msg = _("Do you want to delete the SQL file as well?");
        if (reportIndex_->CurrentReportFileType() == "LUA")
        {
            msg = _("Do you want to delete the Lua file as well?");
        }
        msg << "\n";

        bool delete_file = false;
        if ( wxMessageBox(msg, reportIndex_->UserDialogHeading()
            , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION) == wxYES)
        {
            delete_file = true;
        }
        reportIndex_->DeleteCurrentReportTitle(delete_file);
        if (! reportIndex_->ValidTitle())
        {
            loadedFileName_ = wxEmptyString;
            reportTitleTxtCtrl_->SetLabel(wxEmptyString);
            wxCommandEvent dummy;
            OnClear(dummy);
        }
        else
        {
            loadedFileName_ = reportIndex_->ReportFileName(iSelectedId_);
        }
        navCtrlUpdateRequired_ = true;
    }
    return (iError == wxYES);
}

void mmCustomSQLDialog::OnMenuSelected(wxCommandEvent& event)
{
    int id = event.GetId();
    if (id == 1)
    {
        reportTitleTxtCtrl_->SetValue(_("New SQL Custom Report"));
        tcSourceTxtCtrl_->ChangeValue("select 'Hello World'");
        loadedFileName_.Clear();
        m_radio_box_->SetSelection(0);
        navCtrlUpdateRequired_ = SaveCustomReport();
    }
    if (id == 2)
    {
        reportTitleTxtCtrl_->SetValue(_("New Lua Custom Report"));
        tcSourceTxtCtrl_->ChangeValue("return \"Hello World\"");
        loadedFileName_.Clear();
        m_radio_box_->SetSelection(1);
        navCtrlUpdateRequired_ = SaveCustomReport();
    }
    else if (id == wxID_DELETE)
    {
        navCtrlUpdateRequired_ = DeleteCustomSqlReport();
        if (navCtrlUpdateRequired_) iSelectedId_--;
    }
    if (navCtrlUpdateRequired_) fillControls();
}

void mmCustomSQLDialog::OnSourceTxtChar(wxKeyEvent& event)
{
    if (wxGetKeyState(wxKeyCode('A')) && wxGetKeyState(WXK_CONTROL))
        tcSourceTxtCtrl_->SetSelection(-1, -1); //select all
    event.Skip();
}

void mmCustomSQLDialog::ShowCursorCoordinates(wxTimerEvent& /*event*/)
{
    wxWindow *w = FindFocus();
    if (w && w->GetId() != wxID_VIEW_DETAILS) return;

    long lCursorPosition = tcSourceTxtCtrl_->GetInsertionPoint();
    wxStaticText* st = (wxStaticText*)FindWindow(wxID_PROPERTIES);
    long x = 0, y = 0;
    tcSourceTxtCtrl_->PositionToXY(lCursorPosition, &x, &y);
    st->SetLabel(wxString::Format(_("Line: %ld position: %ld"), y+1, x+1));
}
