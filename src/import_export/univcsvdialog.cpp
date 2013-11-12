/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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
#include "constants.h"
#include "univcsvdialog.h"
#include "util.h"
#include "paths.h"
#include "platfdep.h"
#include <algorithm>
#include "model/Model_Infotable.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"
#include "model/Model_Category.h"
#include "model/Model_Checking.h"

IMPLEMENT_DYNAMIC_CLASS(mmUnivCSVDialog, wxDialog)

BEGIN_EVENT_TABLE(mmUnivCSVDialog, wxDialog)
    EVT_BUTTON(wxID_ADD, mmUnivCSVDialog::OnAdd)
    EVT_BUTTON(ID_UNIVCSVBUTTON_IMPORT, mmUnivCSVDialog::OnImport)
    EVT_BUTTON(ID_UNIVCSVBUTTON_EXPORT, mmUnivCSVDialog::OnExport)
    EVT_BUTTON(wxID_REMOVE, mmUnivCSVDialog::OnRemove)
    EVT_BUTTON(wxID_OPEN, mmUnivCSVDialog::OnLoad)
    EVT_BUTTON(wxID_SAVEAS, mmUnivCSVDialog::OnSave)
    EVT_BUTTON(wxID_UP, mmUnivCSVDialog::OnMoveUp)
    EVT_BUTTON(wxID_DOWN, mmUnivCSVDialog::OnMoveDown)
    EVT_BUTTON(wxID_STANDARD, mmUnivCSVDialog::OnStandard)
    EVT_BUTTON(wxID_SEARCH, mmUnivCSVDialog::OnSearch)
    EVT_CHOICE(wxID_ACCOUNT, mmUnivCSVDialog::OnAccountChange)
    EVT_LISTBOX(ID_LISTBOX, mmUnivCSVDialog::OnListBox)
    EVT_RADIOBOX(wxID_RADIO_BOX, mmUnivCSVDialog::OnCheckOrRadioBox)
    EVT_CHOICE(ID_DIALOG_OPTIONS_DATE_FORMAT, mmUnivCSVDialog::OnDateFormatChanged)
END_EVENT_TABLE()

//----------------------------------------------------------------------------
mmUnivCSVDialog::mmUnivCSVDialog()
{
}

mmUnivCSVDialog::mmUnivCSVDialog(
    wxWindow* parent,
    bool is_importer,
    wxWindowID id,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size,
    long style
) :
    is_importer_(is_importer),
    delimit_(","),
    importSuccessful_(false)
{
    CSVFieldName_[UNIV_CSV_DATE] = _("Date");
    CSVFieldName_[UNIV_CSV_PAYEE] = _("Payee");
    CSVFieldName_[UNIV_CSV_AMOUNT] = _("Amount(+/-)");
    CSVFieldName_[UNIV_CSV_CATEGORY] = _("Category");
    CSVFieldName_[UNIV_CSV_SUBCATEGORY] = _("SubCategory");
    CSVFieldName_[UNIV_CSV_TRANSNUM] = _("Number");
    CSVFieldName_[UNIV_CSV_NOTES] = _("Notes");
    CSVFieldName_[UNIV_CSV_DONTCARE] = _("Don't Care");
    CSVFieldName_[UNIV_CSV_WITHDRAWAL] = _("Withdrawal");
    CSVFieldName_[UNIV_CSV_DEPOSIT] = _("Deposit");
    CSVFieldName_[UNIV_CSV_TOAMOUNT] = _("To/From(+/-)");
    CSVFieldName_[UNIV_CSV_BALANCE] = _("Balance");

    Create(parent, id, caption, pos, size, style);
}

bool mmUnivCSVDialog::Create(wxWindow* parent, wxWindowID id,
                      const wxString& caption,
                      const wxPoint& pos,
                      const wxSize& size,
                      long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    Centre();

    return TRUE;
}

void mmUnivCSVDialog::CreateControls()
{
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_TOP).Border(wxLEFT|wxRIGHT|wxTOP, 5);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_TOP).Border(wxLEFT|wxRIGHT|wxTOP, 5);

    // Define the staticBox font and set it as wxFONTWEIGHT_BOLD
    wxFont staticBoxFontSetting = this->GetFont();
    staticBoxFontSetting.SetWeight(wxFONTWEIGHT_BOLD);

    wxBoxSizer* itemBoxSizer0 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer0);
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer0->Add(itemBoxSizer1, 7, wxGROW|wxALL, 0);
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer1->Add(itemBoxSizer2, 8, wxGROW|wxALL, 5);
    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer1->Add(itemBoxSizer11, 5, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText3 = new wxStaticText(this, wxID_STATIC,
       _("Specify the order of fields in the CSV file"));
    itemBoxSizer2->Add(itemStaticText3, flags);
    itemStaticText3->SetFont(staticBoxFontSetting);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 5);

    //CSV fields candicate
    csvFieldCandicate_ = new wxListBox(this, ID_LISTBOX_CANDICATE,
        wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE|wxLB_NEEDED_SB);
    itemBoxSizer3->Add(csvFieldCandicate_, 1, wxGROW|wxALL, 1);
    for (const auto& it : CSVFieldName_)
        csvFieldCandicate_->Append(it.second, new mmListBoxItem(it.first, it.second));

     //Add Remove Area
    wxPanel* itemPanel_AddRemove = new wxPanel(this, ID_PANEL10,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer3->Add(itemPanel_AddRemove, flags);

    wxBoxSizer* itemBoxSizer_AddRemove = new wxBoxSizer(wxVERTICAL);
    itemPanel_AddRemove->SetSizer(itemBoxSizer_AddRemove);

    //Add button
    m_button_add_= new wxButton(itemPanel_AddRemove, wxID_ADD);
    itemBoxSizer_AddRemove->Add(m_button_add_, flags);

    //Remove button
    m_button_remove_ = new wxButton(itemPanel_AddRemove, wxID_REMOVE);
    itemBoxSizer_AddRemove->Add(m_button_remove_, flags);

    //Standard MMEX CSV
    wxButton* itemButton_standard = new wxButton(itemPanel_AddRemove, wxID_STANDARD, _("&MMEX format"));
    itemBoxSizer_AddRemove->Add(itemButton_standard, flags);
    itemButton_standard->SetToolTip(_("MMEX standard format"));

    //ListBox of attribute order
    csvListBox_ = new wxListBox(this, ID_LISTBOX,
        wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE|wxLB_NEEDED_SB);
    itemBoxSizer3->Add(csvListBox_, 1, wxGROW|wxALL, 1);

   //Arranger Area
    wxPanel* itemPanel_Arranger = new wxPanel(this, ID_PANEL10,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer3->Add(itemPanel_Arranger, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    wxBoxSizer* itemBoxSizer_Arranger = new wxBoxSizer(wxVERTICAL);
    itemPanel_Arranger->SetSizer(itemBoxSizer_Arranger);

    //Move Up button
    wxButton* itemButton_MoveUp = new wxButton(itemPanel_Arranger, wxID_UP, _("&Up"));
    itemBoxSizer_Arranger->Add(itemButton_MoveUp, flags);
    itemButton_MoveUp -> SetToolTip (_("Move Up"));

    //Move down button
    wxButton* itemButton_MoveDown = new wxButton(itemPanel_Arranger, wxID_DOWN, _("&Down"));
    itemBoxSizer_Arranger->Add(itemButton_MoveDown, flags);
    itemButton_MoveDown -> SetToolTip (_("Move &Down"));

    //Load Template button
    wxButton* itemButton_Load = new wxButton(itemPanel_Arranger, wxID_OPEN, _("&Open"));
    itemBoxSizer_Arranger->Add(itemButton_Load, flags);
    itemButton_Load -> SetToolTip (_("Load Template"));

    //Save As Template button
    wxButton* itemButton_Save = new wxButton(itemPanel_Arranger, wxID_SAVEAS, _("Save As..."));
    itemBoxSizer_Arranger->Add(itemButton_Save, flags);
    itemButton_Save -> SetToolTip (_("Save Template"));

    wxStaticLine*  m_staticline1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(m_staticline1, flagsExpand );

    if (this->is_importer_)
    {
        //file to import, file path and search button
        wxPanel* itemPanel6 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
        itemBoxSizer2->Add(itemPanel6, 0, wxEXPAND|wxALL, 1);

        wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
        itemPanel6->SetSizer(itemBoxSizer7);

        wxStaticText* itemStaticText5 = new wxStaticText(itemPanel6, wxID_ANY, _("File Name:"), wxDefaultPosition, wxDefaultSize, 0);
        itemBoxSizer7->Add(itemStaticText5, flags);
        itemStaticText5->SetFont(staticBoxFontSetting);

        m_text_ctrl_ = new wxTextCtrl(itemPanel6, ID_FILE_NAME, wxEmptyString, wxDefaultPosition, wxSize(300, -1), wxTE_PROCESS_ENTER );
        itemBoxSizer7->Add(m_text_ctrl_, 1, wxALL|wxGROW, 5);
        m_text_ctrl_->Connect(ID_FILE_NAME, wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(mmUnivCSVDialog::OnFileNameChanged), NULL, this);
        m_text_ctrl_->Connect(ID_FILE_NAME, wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(mmUnivCSVDialog::OnFileNameEntered), NULL, this);

        wxButton* button_search = new wxButton(itemPanel6, wxID_SEARCH, _("&Search"));
        itemBoxSizer7->Add(button_search, flags);
    }

    // account to import or export
    wxPanel* itemPanel7 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer2->Add(itemPanel7, 0, wxEXPAND|wxALL, 1);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel7->SetSizer(itemBoxSizer8);

    wxStaticText* itemStaticText6 = new wxStaticText(itemPanel7, wxID_ANY, _("Account: "), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer8->Add(itemStaticText6, flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL));
    itemStaticText6->SetFont(staticBoxFontSetting);

    wxSortedArrayString accountArray;
    for (const auto& account: Model_Account::instance().all()) accountArray.Add(account.ACCOUNTNAME);

    m_choice_account_ = new wxChoice(itemPanel7, wxID_ACCOUNT, wxDefaultPosition, wxSize(210, -1), accountArray, 0);
    m_choice_account_->SetSelection(0);
    itemBoxSizer8->Add(m_choice_account_, flags);

    wxStaticLine*  m_staticline2 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(m_staticline2, flagsExpand );

    wxStaticText* itemStaticText66 = new wxStaticText(itemPanel7, wxID_STATIC, wxString(_("Date Format: ")));
    itemBoxSizer8->AddSpacer(10);
    itemBoxSizer8->Add(itemStaticText66, flags);
    itemStaticText66->SetFont(staticBoxFontSetting);
    //itemStaticText66->Enable(!this->is_importer_);

    date_format_ = mmOptions::instance().dateFormat_;
    choiceDateFormat_ = new wxChoice(itemPanel7, ID_DIALOG_OPTIONS_DATE_FORMAT);
    for (const auto& i : date_formats_map())
    {
        choiceDateFormat_->Append(i.second, new wxStringClientData(i.first));
        if (date_format_ == i.first) choiceDateFormat_->SetStringSelection(i.second);
    }
    itemBoxSizer8->Add(choiceDateFormat_, flags);

    // CSV Delimiter
    wxString choices[] = { _("Comma"), _("Semicolon"), _("TAB"), _("User Defined")};
    int num = sizeof(choices) / sizeof(wxString);
    m_radio_box_ = new wxRadioBox(this, wxID_RADIO_BOX, "", wxDefaultPosition, wxDefaultSize, num, choices, 4, wxRA_SPECIFY_COLS);

    delimit_ = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);

    textDelimiter4 = new wxTextCtrl( this, ID_UD_DELIMIT, delimit_, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    textDelimiter4->SetToolTip(_("Specify the delimiter to use when importing/exporting CSV files"));
    textDelimiter4->SetMaxLength(1);
    textDelimiter4->Disable();
    textDelimiter4->Connect(ID_UD_DELIMIT, wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(mmUnivCSVDialog::OnCheckOrRadioBox), NULL, this);

    if (delimit_ == ",")
        m_radio_box_->SetSelection(0);
    else if (delimit_ == ";")
        m_radio_box_->SetSelection(1);
    else if (delimit_ == "\t")
        m_radio_box_->SetSelection(2);
    else {
        m_radio_box_->SetSelection(3);
        textDelimiter4->Enable();
     }

    wxStaticBox* importExportStaticBox = new wxStaticBox(this, wxID_ANY, _("CSV Delimiter"));
    importExportStaticBox->SetFont(staticBoxFontSetting);
    wxStaticBoxSizer* importExportStaticBoxSizer = new wxStaticBoxSizer(importExportStaticBox, wxHORIZONTAL);

    itemBoxSizer2->Add(importExportStaticBoxSizer, 0, wxALL|wxEXPAND, 5);

    importExportStaticBoxSizer->Add(m_radio_box_, 0, wxALL|wxEXPAND, 3);
    importExportStaticBoxSizer->Add(textDelimiter4, 0, wxALIGN_BOTTOM|wxALL, 8);

    // Preview
    wxStaticBoxSizer* m_staticbox = new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC, "&Preview"), wxVERTICAL);

    m_list_ctrl_ = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    m_staticbox->Add(m_list_ctrl_, 1, wxGROW|wxALL, 5);
    itemBoxSizer0->Add(m_staticbox, 3, wxALL|wxEXPAND, 5);

    //Import File button
    wxPanel* itemPanel5 = new wxPanel(this, ID_PANEL10,
    wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer0->Add(itemPanel5, 0, wxALIGN_RIGHT|wxALL, 1);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel5->SetSizer(itemBoxSizer6);

    if (this->is_importer_)
    {
        wxButton* itemButton_Import = new wxButton(itemPanel5, ID_UNIVCSVBUTTON_IMPORT, _("&Import"),
            wxDefaultPosition, wxDefaultSize, 0);
        itemBoxSizer6->Add(itemButton_Import, 0, wxALIGN_CENTER|wxALL, 5);
        itemButton_Import -> SetToolTip (_("Import File"));
    }
    else
    {
        wxButton* itemButton_Export = new wxButton(itemPanel5, ID_UNIVCSVBUTTON_EXPORT, _("&Export"),
            wxDefaultPosition, wxDefaultSize, 0);
        itemBoxSizer6->Add(itemButton_Export, 0, wxALIGN_CENTER|wxALL, 5);
        itemButton_Export -> SetToolTip (_("Export File"));
    }

    wxButton* itemCancelButton = new wxButton(itemPanel5, wxID_CANCEL, _("&Cancel"));
    itemBoxSizer6->Add(itemCancelButton, 0, wxALIGN_CENTER|wxALL, 5);
    itemCancelButton->SetFocus();

    //Log viewer
    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxVERTICAL);

    itemBoxSizer11->Add(itemBoxSizer22, 1, wxGROW|wxALL, 0);

    log_field_ = new wxTextCtrl( this, wxID_STATIC, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxHSCROLL );
    itemBoxSizer22->Add(log_field_, 1, wxGROW|wxALL, 5);

    wxButton* itemClearButton = new wxButton(this, wxID_CLEAR, _("Clear"));
    itemBoxSizer22->Add(itemClearButton, 0, wxALIGN_CENTER|wxALL, 5);
    itemClearButton->Connect(wxID_CLEAR, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmUnivCSVDialog::OnButtonClear), NULL, this);

}

bool mmUnivCSVDialog::ShowToolTips()
{
    return TRUE;
}

void mmUnivCSVDialog::csv2tab_separated_values(wxString& line, const wxString& delimit)
{
    //csv line example:
    //12.02.2010,Payee,-1105.08,Category,Subcategory,,"Fuel ""95"", 42.31 l (24.20) 212366"
    int i=0;
    //Single quotes will be used instead double quotes
    //Replace all single quotes first
    line.Replace("'", "\6");
    //Replace double quotes that used twice to replacer
    line.Replace("\"\"\""+delimit+"\"\"\"", "\5\""+delimit+"\"\5");
    line.Replace("\"\"\""+delimit, "\5\""+delimit);
    line.Replace(delimit+"\"\"\"", delimit+"\"\5");
    line.Replace("\"\""+delimit, "\5"+delimit);
    line.Replace(delimit+"\"\"", delimit+"\5");

    //replace delimiter to TAB and double quotes to single quotes
    line.Replace("\""+delimit+"\"", "'\t'");
    line.Replace("\""+delimit, "'\t");
    line.Replace(delimit+"\"", "\t'");
    line.Replace("\"\"", "\5");
    line.Replace("\"", "'");

    wxString temp_line = wxEmptyString;
    wxString token;
    wxStringTokenizer tkz1(line, "'");

    while (tkz1.HasMoreTokens())
    {
        token = tkz1.GetNextToken();
        if (0 == fmod((double)i,2))
            token.Replace(delimit,"\t");
        temp_line << token;
        i++;
    };
    //Replace back all replacers to the original value
    temp_line.Replace("\5", "\"");
    temp_line.Replace("\6", "'");
    line = temp_line;
}

wxBitmap mmUnivCSVDialog::GetBitmapResource(const wxString& /*name*/)
{
    return wxNullBitmap;
}

wxIcon mmUnivCSVDialog::GetIconResource(const wxString& /*name*/)
{
    return wxNullIcon;
}

//Selection dialog for fields to be added to listbox
void mmUnivCSVDialog::OnAdd(wxCommandEvent& /*event*/)
{
    int index = csvFieldCandicate_->GetSelection();
    if (index != wxNOT_FOUND)
    {
        mmListBoxItem* item = (mmListBoxItem*)csvFieldCandicate_->GetClientObject(index);

        csvListBox_->Append(item->getName(), new mmListBoxItem(item->getIndex(), item->getName()));
        csvFieldOrder_.push_back(item->getIndex());

        if (item->getIndex() != UNIV_CSV_DONTCARE)
        {
            csvFieldCandicate_->Delete(index);
            if (index < (int)csvFieldCandicate_->GetCount())
                csvFieldCandicate_->SetSelection(index, true);
            else
                csvFieldCandicate_->SetSelection(csvFieldCandicate_->GetCount() - 1, true);
        }

        this->update_preview();
    }
}

//Removes an item from the field list box
void mmUnivCSVDialog::OnRemove(wxCommandEvent& /*event*/)
{
    int index = csvListBox_->GetSelection();
    if (index != wxNOT_FOUND)
    {
        mmListBoxItem *item = (mmListBoxItem*)csvListBox_->GetClientObject(index);
        int item_index = item->getIndex();
        wxString item_name = item->getName();

        if (item_index != UNIV_CSV_DONTCARE)
        {
            int pos = 0;
            for (pos = 0; pos < (int)csvFieldCandicate_->GetCount() - 1; pos ++)
            {
                mmListBoxItem *item = (mmListBoxItem*)csvFieldCandicate_->GetClientObject(pos);
                if (item_index < item->getIndex())
                    break;
            }
            csvFieldCandicate_->Insert(item_name, pos, new mmListBoxItem(item_index, item_name));
        }

        csvListBox_->Delete(index);
        csvFieldOrder_.erase(csvFieldOrder_.begin() + index);

        if (index < (int)csvListBox_->GetCount())
            csvListBox_->SetSelection(index, true);
        else
            csvListBox_->SetSelection(csvListBox_->GetCount() - 1, true);

        this->update_preview();
    }
}

bool mmUnivCSVDialog::isIndexPresent(int index) const
{
    for(std::vector<int>::const_iterator it = csvFieldOrder_.begin(); it != csvFieldOrder_.end(); ++ it)
    {
        if (*it == index) return true;
    }

    return false;
}

const wxString mmUnivCSVDialog::getCSVFieldName(int index) const
{
    std::map<int, wxString>::const_iterator it = CSVFieldName_.find(index);
    if (it != CSVFieldName_.end())
        return it->second;

    return _("Unknown");
}

void mmUnivCSVDialog::OnLoad(wxCommandEvent& /*event*/)
{
   wxString fileName = wxFileSelector(_("Choose Universal CSV format file to load"),
      wxEmptyString, wxEmptyString, wxEmptyString,  "CSV Template(*.mcv)|*.mcv", wxFD_FILE_MUST_EXIST);
   if (!fileName.empty())
   {
      wxTextFile tFile(fileName);
      if (!tFile.Open())
      {
         wxMessageBox(_("Unable to open file."), _("Universal CSV Import"), wxOK|wxICON_WARNING);
         return;
      }
      csvFieldOrder_.clear();
      csvListBox_->Clear();

      wxString str;
      for (str = tFile.GetFirstLine(); !tFile.Eof(); str = tFile.GetNextLine())
      {
         long num = 0;
         if (str.ToLong(&num))
         {
             wxString item_name = getCSVFieldName(num);
             csvListBox_->Append(item_name, new mmListBoxItem(num, item_name));
             csvFieldOrder_.push_back(num);
         }
      }
      // update csvFieldCandicate_
      csvFieldCandicate_->Clear();
      for (std::map<int, wxString>::const_iterator it = CSVFieldName_.begin(); it != CSVFieldName_.end(); ++ it)
      {
          std::vector<int>::const_iterator loc = find(csvFieldOrder_.begin(), csvFieldOrder_.end(), it->first);
          if (loc == csvFieldOrder_.end() || it->first == UNIV_CSV_DONTCARE)
              csvFieldCandicate_->Append(it->second, new mmListBoxItem(it->first, it->second));
      }

      tFile.Write();
      tFile.Close();

      this->update_preview();
   }
}

//Saves the field order to a template file
void mmUnivCSVDialog::OnSave(wxCommandEvent& /*event*/)
{
    wxString fileName = wxFileSelector(_("Choose Universal CSV format file to save"),
                wxEmptyString, wxEmptyString, wxEmptyString, "CSV Template(*.mcv)|*.mcv", wxFD_SAVE);
    if (!fileName.empty())
    {
        correctEmptyFileExt("mcv",fileName);

        wxTextFile tFile(fileName);
        //if the file does not exist and cannot be created, throw an error
        //if the file does exist, then skip to else section
        if (!tFile.Exists() && !tFile.Create())
        {
            wxMessageBox(_("Unable to write to file."), _("Universal CSV Import"), wxOK|wxICON_WARNING);
            return;
        }
        else
        {
            //clear the contents of the current file
            tFile.Clear();
            for (std::vector<int>::const_iterator it = csvFieldOrder_.begin(); it != csvFieldOrder_.end(); ++ it)
            {
                wxString line = wxString::Format("%d", *it);
                tFile.AddLine(line);
            }
        }
        tFile.Write();
        tFile.Close();
    }
}

void mmUnivCSVDialog::OnImport(wxCommandEvent& /*event*/)
{
    // date, amount, payee are required
    if (!isIndexPresent(UNIV_CSV_DATE) ||
        !isIndexPresent(UNIV_CSV_PAYEE) ||
        (!isIndexPresent(UNIV_CSV_AMOUNT) && (!isIndexPresent(UNIV_CSV_WITHDRAWAL) ||
        !isIndexPresent(UNIV_CSV_DEPOSIT))))
    {
         wxMessageBox(_("Incorrect fields specified for CSV import! Requires at least Date, Amount and Payee."),
                      _("Universal CSV Import"), wxOK|wxICON_WARNING);
         return;
    }

    bool canceledbyuser = false;
    wxString acctName = m_choice_account_->GetStringSelection();
    Model_Account::Data* from_account = Model_Account::instance().get(acctName);

    if (from_account)
    {
        fromAccountID_ = from_account->ACCOUNTID;
        wxString fileName = m_text_ctrl_->GetValue();
        wxFileName csv_file(fileName);
        if (fileName.IsEmpty() || !csv_file.FileExists())
        {
            return;
        }
        else
        {
            wxTextFile tFile(fileName);
            if (!tFile.Open())
            {
                 wxMessageBox(_("Unable to open file."), _("Universal CSV Import"), wxOK|wxICON_WARNING);
                 return;
            }

            wxFileName logFile = mmex::GetLogDir(true);
            logFile.SetFullName(fileName);
            logFile.SetExt("txt");

            wxFileOutputStream outputLog(logFile.GetFullPath());
            wxTextOutputStream log(outputLog);

            /* date, payeename, amount(+/-), Number, status, category : subcategory, notes */
            long countNumTotal = 0;
            long countImported = 0;

            wxProgressDialog progressDlg(_("Universal CSV Import"),
                _("Transactions imported from CSV: "), 100,
                NULL, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_CAN_ABORT);
            Model_Checking::instance().Begin();

            wxString line;
            for (line = tFile.GetFirstLine(); !tFile.Eof(); line = tFile.GetNextLine())
            {
                wxString progressMsg;
                progressMsg << _("Transactions imported from CSV\nto account ") << acctName << ": " << countImported;
                if (!progressDlg.Update(static_cast<int>((static_cast<double>(countImported)/100.0
                    - countNumTotal/100) *99), progressMsg))
                {
                    canceledbyuser = true;
                    break; // abort processing
                }

                if (!line.IsEmpty())
                    ++countNumTotal;
                else
                    continue;

                dt_.clear();
                payee_.clear();
                type_.clear();
                amount_.clear();
                categ_.clear();
                subcateg_.clear();
                transNum_.clear();
                notes_.clear();
                payeeID_ = -1;
                categID_ = -1;
                subCategID_ = -1;
                val_ = 0.0;

                this->csv2tab_separated_values(line, delimit_);
                wxStringTokenizer tkz(line, "\t", wxTOKEN_RET_EMPTY_ALL);
                int numTokens = (int)tkz.CountTokens();
                if (numTokens < (int)csvFieldOrder_.size())
                {
                    log << _("Line : ") << wxString::Format("%ld", countNumTotal)
                        << _(" file contains insufficient number of tokens") << endl;
                    *log_field_ << _("Line : ") << wxString::Format("%ld", countNumTotal)
                        << _(" file contains insufficient number of tokens") << "\n";
                    continue;
                }

                std::vector<wxString> tokens;
                while (tkz.HasMoreTokens())
                {
                    wxString token = tkz.GetNextToken();
                    tokens.push_back(token);
                }

                for (size_t i = 0; i < csvFieldOrder_.size(); ++i)
                {
                    if (tokens.size() >= i)
                    {
                        parseToken(csvFieldOrder_[i], tokens[i]);
                    }
                }

                if (dt_.Trim().IsEmpty() || payeeID_ == -1 ||
                    amount_.Trim().IsEmpty() ||  type_.Trim().IsEmpty())
                {
                    log << _("Line : ") << wxString::Format("%ld", countNumTotal)
                        << _(" One of the following fields: Date, Payee, Amount, Type is missing, skipping") << endl;
                    *log_field_ << _("Line : ") << wxString::Format("%ld", countNumTotal)
                        << _(" One of the following fields: Date, Payee, Amount, Type is missing, skipping") << "\n";
                    continue;
                }

                if (categID_ == -1)
                {
                    Model_Payee::Data* payee = Model_Payee::instance().get(payeeID_);
                    if (payee)
                    {
                        categID_ = payee->CATEGID;
                        subCategID_ = payee->SUBCATEGID;
                    }
                }

               wxString status = "F";
               int toAccountID = -1;

               Model_Checking::Data *pTransaction = Model_Checking::instance().create();
               pTransaction->ACCOUNTID = fromAccountID_;
               pTransaction->TOACCOUNTID = toAccountID;
               pTransaction->PAYEEID = payeeID_;
               pTransaction->TRANSCODE = type_;
               pTransaction->TRANSAMOUNT = val_;
               pTransaction->STATUS = status;
               pTransaction->TRANSACTIONNUMBER = transNum_;
               pTransaction->NOTES = notes_;
               pTransaction->TRANSDATE = dtdt_.FormatISODate();
               pTransaction->TOTRANSAMOUNT = 0.0;

               Model_Checking::instance().save(pTransaction);

               countImported++;
               log << wxString::Format(_("Line : %ld imported OK."), countNumTotal) << endl;
               *log_field_ << wxString::Format(_("Line : %ld imported OK."), countNumTotal) << "\n";
            }

            progressDlg.Destroy();

            wxString msg = wxString::Format(_("Total Lines : %ld"), countNumTotal);
            msg << "\n";
            msg << wxString::Format(_("Total Imported : %ld"), countImported);
            msg << "\n\n";
            msg << wxString::Format(_("Log file written to : %s"), logFile.GetFullPath());
            msg << "\n\n";

            wxString confirmMsg = msg + _("Please confirm saving...");
            if (!canceledbyuser && wxMessageBox(confirmMsg, _("Importing CSV"), wxOK|wxCANCEL|wxICON_INFORMATION) == wxCANCEL)
                canceledbyuser = true;

            if (countImported > 0)
                msg << _ ("Imported transactions have been flagged so you can review them.");

            // Since all database transactions are only in memory,
            if (!canceledbyuser)
            {
                // we need to save them to the database.
                Model_Checking::instance().Commit();
                importSuccessful_ = true;
                msg << _("Transactions saved to database in account: ") << acctName;
            }
            else
            {
                // and discard the database changes.
                Model_Checking::instance().Rollback();
                msg  << _("Imported transactions discarded by user!");
            }

            *log_field_ << msg;

            outputLog.Close();
        }
    }

    if (!canceledbyuser) Close();
}

void mmUnivCSVDialog::OnExport(wxCommandEvent& /*event*/)
{
    wxString delimit = this->delimit_;
    wxString acctName = m_choice_account_->GetStringSelection();
    Model_Account::Data* from_account = Model_Account::instance().get(acctName);
    int fromAccountID = from_account->ACCOUNTID;

    if(from_account)
    {
        wxString chooseExt;
        chooseExt << _("CSV Files") << " (*.csv)|*.csv;*.CSV";
        wxString fileName = wxFileSelector(_("Choose CSV data file to Export"),
                            wxEmptyString, wxEmptyString, wxEmptyString, chooseExt, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (fileName.empty()) return;

        correctEmptyFileExt("csv",fileName);

        wxFileOutputStream output(fileName);
        wxTextOutputStream text(output);
        wxString buffer;
        wxDateTime trx_date;

        long numRecords = 0;
        Model_Currency::Data* currency = Model_Account::currency(from_account);

        double account_balance = from_account->INITIALBAL;
        for (const auto& pBankTransaction : Model_Checking::instance().all())
        {
            if (pBankTransaction.ACCOUNTID != fromAccountID && pBankTransaction.TOACCOUNTID != fromAccountID) continue;

            Model_Payee::Data* payee = Model_Payee::instance().get(pBankTransaction.PAYEEID);
            int fAccountID = pBankTransaction.ACCOUNTID;
            int tAccountID = pBankTransaction.TOACCOUNTID;

            double value = pBankTransaction.TRANSAMOUNT;
            double tovalue = 0;
            wxString toamount = "";
            double value_temp = value;

            double transaction_amount = (Model_Checking::status(pBankTransaction) != Model_Checking::VOID_) ? Model_Checking::balance(pBankTransaction, fromAccountID) : 0;
            account_balance += transaction_amount;

            if (Model_Checking::type(pBankTransaction) == Model_Checking::TRANSFER)
            {
                tovalue = pBankTransaction.TOTRANSAMOUNT;

                if (tAccountID == fromAccountID) {
                    value = tovalue;
                    tovalue = -value_temp;
                } else if (fAccountID == fromAccountID) {
                    value = -value;
                }
                toamount = Model_Currency::toString(tovalue, currency);
            }
            else if (Model_Checking::type(pBankTransaction) == Model_Checking::WITHDRAWAL)
                value = -value;

            wxString amount = Model_Currency::toString(value, currency);

            wxString amount_tmp = Model_Currency::toString(-value, currency);

            buffer = "";
            Model_Category::Data* category = Model_Category::instance().get(pBankTransaction.CATEGID);
            Model_Subcategory::Data* sub_category = Model_Subcategory::instance().get(pBankTransaction.SUBCATEGID);
            for (std::vector<int>::const_iterator sit = csvFieldOrder_.begin(); sit != csvFieldOrder_.end(); ++ sit)
            {
                switch (*sit)
                {
                    case UNIV_CSV_DATE:
                        trx_date = Model_Checking::TRANSDATE(pBankTransaction);
                        buffer << inQuotes(trx_date.Format(date_format_), delimit);
                        break;
                    case UNIV_CSV_PAYEE:
                        if (Model_Checking::type(pBankTransaction) == Model_Checking::TRANSFER)
                        {
                            if (tAccountID == fromAccountID) {
                                Model_Account::Data* account = Model_Account::instance().get(fAccountID);
                                buffer << inQuotes(account ? account->ACCOUNTNAME : "", delimit);
                            }
                            else if (fAccountID == fromAccountID) {
                                Model_Account::Data* account = Model_Account::instance().get(tAccountID);
                                buffer << inQuotes(account ? account->ACCOUNTNAME : "", delimit);
                            }
                            else
                                buffer << "";
                        }
                        else
                            buffer << inQuotes(payee ? payee->PAYEENAME : "", delimit);
                        break;
                    case UNIV_CSV_AMOUNT:
                        buffer << inQuotes(amount, delimit);
                        break;
                    case UNIV_CSV_CATEGORY:
                        buffer << inQuotes(category ? category->CATEGNAME : "", delimit);
                        break;
                    case UNIV_CSV_SUBCATEGORY:
                        buffer << inQuotes(sub_category ? sub_category->SUBCATEGNAME : "" , delimit);
                        break;
                    case UNIV_CSV_TRANSNUM:
                        buffer << inQuotes(pBankTransaction.TRANSACTIONNUMBER, delimit);
                        break;
                    case UNIV_CSV_NOTES:
                        buffer << inQuotes(wxString(pBankTransaction.NOTES).Trim(), delimit);
                        break;
                    case UNIV_CSV_DEPOSIT:
                        buffer << inQuotes((value > 0.0) ? amount : "", delimit);
                        break;
                    case UNIV_CSV_WITHDRAWAL:
                        text << inQuotes(value >= 0.0 ? "" : amount_tmp, delimit);
                        break;
                    case UNIV_CSV_TOAMOUNT:
                        buffer << inQuotes(toamount, delimit);
                        break;
                    case UNIV_CSV_BALANCE:
                        buffer << inQuotes(Model_Currency::toString(account_balance, currency), delimit);
                        break;
                    case UNIV_CSV_DONTCARE:
                    default:
                        break;
                }

                buffer << delimit;
            }

            buffer.RemoveLast(1);
            text << buffer << endl;
            *log_field_ << buffer << "\n";

            ++ numRecords;
        }

        wxString msg = wxString::Format(wxTRANSLATE("Transactions exported: %ld"), numRecords);
        mmShowErrorMessage(this, wxGetTranslation(msg), _("Export to CSV"));
    }
}

void mmUnivCSVDialog::update_preview()
{
    this->m_list_ctrl_->ClearAll();
    long index = 0;
    this->m_list_ctrl_->InsertColumn(index, _("#"));
    this->m_list_ctrl_->SetColumnWidth(index, 30);
    int date_position = 0;

    for (std::vector<int>::const_iterator it = csvFieldOrder_.begin(); it != csvFieldOrder_.end(); ++ it)
    {
        ++ index;
        wxString item_name = this->getCSVFieldName(*it);
        this->m_list_ctrl_->InsertColumn(index, item_name);
        if (item_name == _("Notes"))
            this->m_list_ctrl_->SetColumnWidth(index, 300);
        else if (item_name == _("Date")) {
            date_position = index;
        }

    }

    //TODO re use code in OnImport & OnExport
    if (this->is_importer_)
    {
        wxString fileName = m_text_ctrl_->GetValue();
        wxFileName csv_file(fileName);

        if (!fileName.IsEmpty() && csv_file.FileExists())
        {
            wxTextFile tFile(fileName);
            if (!tFile.Open())
            {
                 wxMessageBox(_("Unable to open file."), _("Universal CSV Import"), wxOK|wxICON_WARNING);
                 return;
            }

            wxString delimit = this->delimit_;
            wxString line;
            size_t count = 0;
            int row = 0;
            for (line = tFile.GetFirstLine(); !tFile.Eof(); line = tFile.GetNextLine())
            {
                this->csv2tab_separated_values(line, delimit);
                wxStringTokenizer tkz(line, "\t", wxTOKEN_RET_EMPTY_ALL);

                int col = 0;
                wxString buf;
                buf.Printf(_T("%d"), col);
                long itemIndex = m_list_ctrl_->InsertItem(row, buf, 0);
                buf.Printf(_T("%d"), row + 1);
                m_list_ctrl_->SetItem(itemIndex, col, buf);
                while (tkz.HasMoreTokens())
                {
                    ++ col;
                    wxString token = tkz.GetNextToken();

                    if (col >= m_list_ctrl_->GetColumnCount())
                        break;
                    else
                    {
                        m_list_ctrl_->SetItem(itemIndex, col, token);
                    }
                }

                if (++ count >= 10) break;
                ++ row;
            }
        }
    }
    else // exporter preview
    {
        wxString date_format = Model_Infotable::instance().GetStringInfo("DATEFORMAT", mmex::DEFDATEFORMAT);
        wxString acctName = m_choice_account_->GetStringSelection();
        Model_Account::Data* from_account = Model_Account::instance().get(acctName);
        int fromAccountID = from_account->ACCOUNTID;

        if (from_account)
        {
            size_t count = 0;
            int row = 0;
            wxString delimit = this->delimit_;
            Model_Currency::Data* currency = Model_Account::currency(from_account);

            double account_balance = from_account->INITIALBAL;
            for (const auto& pBankTransaction : Model_Checking::instance().all())
            {
                if (pBankTransaction.ACCOUNTID != fromAccountID && pBankTransaction.TOACCOUNTID != fromAccountID) continue;

                Model_Payee::Data* payee = Model_Payee::instance().get(pBankTransaction.PAYEEID);
                int fAccountID = pBankTransaction.ACCOUNTID;
                int tAccountID = pBankTransaction.TOACCOUNTID;

                double value = pBankTransaction.TRANSAMOUNT;
                double tovalue = 0;
                wxString toamount = "";
                double value_temp = value;

                double transaction_amount = (Model_Checking::status(pBankTransaction) != Model_Checking::VOID_) ? Model_Checking::balance(pBankTransaction, fromAccountID) : 0;
                account_balance += transaction_amount;

                if (Model_Checking::type(pBankTransaction) == Model_Checking::TRANSFER)
                {
                    Model_Account::Data* to_account = Model_Account::instance().get(tAccountID);
                    const wxString fromAccount = from_account->ACCOUNTNAME;
                    const wxString toAccount = to_account->ACCOUNTNAME;
                    tovalue = pBankTransaction.TOTRANSAMOUNT;

                    if (tAccountID == fromAccountID) {
                        value = tovalue;
                        tovalue = -value_temp;
                    } else if (fAccountID == fromAccountID) {
                        value = -value;
                    }
                    toamount = Model_Currency::toString(tovalue, currency);
                }
                else if (Model_Checking::type(pBankTransaction) == Model_Checking::WITHDRAWAL)
                    value = -value;

                wxString amount = Model_Currency::toString(value, currency);

                wxString amount_tmp = Model_Currency::toString(-value, currency);

                int col = 0;
                wxString buf;
                buf.Printf(_T("%d"), col);
                long itemIndex = m_list_ctrl_->InsertItem(row, buf, 0);
                buf.Printf(_T("%d"), row + 1);
                m_list_ctrl_->SetItem(itemIndex, col, buf);

                Model_Category::Data* category = Model_Category::instance().get(pBankTransaction.CATEGID);
                Model_Subcategory::Data* sub_category = Model_Subcategory::instance().get(pBankTransaction.SUBCATEGID);
                for (std::vector<int>::const_iterator sit = csvFieldOrder_.begin(); sit != csvFieldOrder_.end(); ++ sit)
                {
                    ++ col;
                    wxString text;
                    switch (*sit)
                    {
                        case UNIV_CSV_DATE: //TODO: Proper date format
                            text << inQuotes(mmGetDateForDisplay(Model_Checking::TRANSDATE(pBankTransaction)), delimit);
                            break;
                        case UNIV_CSV_PAYEE:
                            if (Model_Checking::type(pBankTransaction) == Model_Checking::TRANSFER)
                            {
                                if (tAccountID == fromAccountID) {
                                    Model_Account::Data* account = Model_Account::instance().get(fAccountID);
                                    text << inQuotes(account ? account->ACCOUNTNAME : "", delimit);
                                }
                                else if (fAccountID == fromAccountID) {
                                    Model_Account::Data* account = Model_Account::instance().get(tAccountID);
                                    text << inQuotes(account ? account->ACCOUNTNAME : "", delimit);
                                }
                                else
                                    text << "";
                            }
                            else
                                text << inQuotes(payee->PAYEENAME, delimit);
                            break;
                        case UNIV_CSV_AMOUNT:
                            text << inQuotes(amount, delimit);
                            break;
                        case UNIV_CSV_CATEGORY:
                            text << inQuotes(category->CATEGNAME, delimit);
                            break;
                        case UNIV_CSV_SUBCATEGORY:
                            text << inQuotes(sub_category ? sub_category->SUBCATEGNAME : "", delimit);
                            break;
                        case UNIV_CSV_TRANSNUM:
                            text << inQuotes(pBankTransaction.TRANSACTIONNUMBER, delimit);
                            break;
                        case UNIV_CSV_NOTES:
                            text << inQuotes(wxString(pBankTransaction.NOTES).Trim(), delimit);
                            break;
                        case UNIV_CSV_DEPOSIT:
                            text << inQuotes(value > 0.0 ? amount : "", delimit);
                            break;
                        case UNIV_CSV_WITHDRAWAL:
                            text << inQuotes(value >= 0.0 ? "" : amount_tmp, delimit);
                            break;
                        case UNIV_CSV_TOAMOUNT:
                            text << inQuotes(toamount, delimit);
                            break;
                        case UNIV_CSV_BALANCE:
                            text << inQuotes(Model_Currency::toString(account_balance, currency), delimit);
                            break;
                        case UNIV_CSV_DONTCARE:
                        default:
                            break;
                    }
                    if (col >= m_list_ctrl_->GetColumnCount())
                        break;
                    else
                    {
                        if (col == date_position)
                        {
                            wxDateTime dtdt;
                            mmParseDisplayStringToDate(dtdt, text, date_format);
                            text = dtdt.Format(date_format);
                        }
                        m_list_ctrl_->SetItem(itemIndex, col, text);
                    }

                }
                if (++ count >= 10) break;
                ++ row;
            }
        }
    }
}

void mmUnivCSVDialog::OnMoveUp(wxCommandEvent& /*event*/)
{
    int index = csvListBox_->GetSelection();
    if (index != wxNOT_FOUND && index != 0)
    {
        mmListBoxItem* item = (mmListBoxItem*)csvListBox_->GetClientObject(index);
        int item_index = item->getIndex();
        wxString item_name = item->getName();

        csvListBox_->Delete(index);
        csvListBox_->Insert(item_name, index - 1, new mmListBoxItem(item_index, item_name));

        csvListBox_->SetSelection(index - 1, true);
        std::swap(csvFieldOrder_[index - 1], csvFieldOrder_[index]);

        this->update_preview();
    }
}

void mmUnivCSVDialog::OnMoveDown(wxCommandEvent& /*event*/)
{
    int index = csvListBox_->GetSelection();
    if (index != wxNOT_FOUND && index != (int)csvListBox_->GetCount() - 1)
    {
        mmListBoxItem* item = (mmListBoxItem*)csvListBox_->GetClientObject(index);
        int item_index = item->getIndex();
        wxString item_name = item->getName();

        csvListBox_->Delete(index);
        csvListBox_->Insert(item_name, index + 1, new mmListBoxItem(item_index, item_name));

        csvListBox_->SetSelection(index + 1, true);
        std::swap(csvFieldOrder_[index + 1], csvFieldOrder_[index]);

        this->update_preview();
    }
}

void mmUnivCSVDialog::OnStandard(wxCommandEvent& /*event*/)
{
    csvListBox_->Clear();
    csvFieldOrder_.clear();
    int standard[] = {UNIV_CSV_DATE, UNIV_CSV_PAYEE, UNIV_CSV_AMOUNT, UNIV_CSV_CATEGORY, UNIV_CSV_SUBCATEGORY, UNIV_CSV_TRANSNUM, UNIV_CSV_NOTES, UNIV_CSV_TOAMOUNT};
    for (size_t i = 0; i < sizeof(standard)/sizeof(UNIV_CSV_DATE); ++ i)
    {
        csvListBox_->Append(CSVFieldName_[standard[i]], new mmListBoxItem(standard[i], CSVFieldName_[standard[i]]));
        csvFieldOrder_.push_back(standard[i]);
    }

    csvFieldCandicate_->Clear();
    int rest[] = { UNIV_CSV_DONTCARE, UNIV_CSV_WITHDRAWAL, UNIV_CSV_DEPOSIT, UNIV_CSV_BALANCE };
    for (size_t i = 0; i < sizeof(rest)/sizeof(UNIV_CSV_DATE); ++ i)
    {
        csvFieldCandicate_->Append(CSVFieldName_[rest[i]], new mmListBoxItem(rest[i], CSVFieldName_[rest[i]]));
    }

    update_preview();
}

void mmUnivCSVDialog::OnSearch(wxCommandEvent& /*event*/)
{
    wxString fileName = m_text_ctrl_->GetValue();

    fileName = wxFileSelector(_("Choose CSV data file to import"),
            wxEmptyString, fileName, wxEmptyString, "*.csv", wxFD_FILE_MUST_EXIST);

    if (!fileName.IsEmpty())
    {
        m_text_ctrl_->SetValue(fileName);

        wxTextFile tFile(fileName);
        if (!tFile.Open())
        {
             *log_field_ << _("Unable to open file.") << "\n";
             return;
        }

        wxString line;
        size_t count = 0;
        for (line = tFile.GetFirstLine(); !tFile.Eof(); line = tFile.GetNextLine())
        {
            *log_field_ << line << "\n";
            if (++ count >= 10) break;
        }
        *log_field_ << "\n";
        this->update_preview();
    }
}

void mmUnivCSVDialog::OnAccountChange(wxCommandEvent& event)
{
    int sel = event.GetInt();
    if (sel != wxNOT_FOUND)
    {
        update_preview();
    }
    wxString acctName = m_choice_account_->GetStringSelection();
    Model_Account::Data* account = Model_Account::instance().get(acctName);
    Model_Currency::Data* currency = Model_Account::currency(account);
    *log_field_ << _("Currency:") << " " << currency->CURRENCYNAME << "\n";
}

void mmUnivCSVDialog::OnListBox(wxCommandEvent& event)
{
    int sel = event.GetInt();
    if (sel != wxNOT_FOUND)
    {
        //TODO  update relate widget status
    }
}

void mmUnivCSVDialog::OnCheckOrRadioBox(wxCommandEvent& event)
{
    wxString ud_delimit = textDelimiter4->GetValue();

    switch(m_radio_box_->GetSelection())
    {
        case 0:
            delimit_ = ",";
            textDelimiter4->Disable();
            break;
        case 1:
            delimit_ = ";";
            textDelimiter4->Disable();
            break;
        case 2:
            delimit_ = "\t";
            textDelimiter4->Disable();
            break;
        case 3:
            delimit_ = ud_delimit;
            textDelimiter4->Enable();
            break;
        default:
            break;
    }

    textDelimiter4->SetEvtHandlerEnabled(false);
    textDelimiter4->SetValue(delimit_);
    textDelimiter4->SetEvtHandlerEnabled(true);
    event.Skip();

    if (!delimit_.IsEmpty()) this->update_preview();
}

void mmUnivCSVDialog::parseToken(int index, wxString& token)
{
    if (token.Trim().IsEmpty()) return;
    Model_Payee::Data* payee = 0;
    Model_Category::Data* category = 0;
    Model_Subcategory::Data* sub_category = 0;

    switch (index)
    {
        case UNIV_CSV_DATE:
            mmParseDisplayStringToDate(dtdt_, token, date_format_);
            dt_ = dtdt_.GetDateOnly().FormatISODate();
            break;

        case UNIV_CSV_PAYEE:
            payee = Model_Payee::instance().get(token);
            if (!payee)
            {
                payee = Model_Payee::instance().create();
                payee->PAYEENAME = token;
                Model_Payee::instance().save(payee);
            }

            payeeID_ = payee->PAYEEID;
            break;

        case UNIV_CSV_AMOUNT:
            token.Replace(" ", wxEmptyString);

            if (!Model_Currency::fromString(token, val_, Model_Account::currency(Model_Account::instance().get(fromAccountID_)))) return;

            if (val_ <= 0.0)
                type_ = TRANS_TYPE_WITHDRAWAL_STR;
            else
                type_ = TRANS_TYPE_DEPOSIT_STR;

            val_ = fabs(val_);
            amount_ = token;
            break;

        case UNIV_CSV_CATEGORY:
            category = Model_Category::instance().get(token);
            if (!category)
            {
                category = Model_Category::instance().create();
                category->CATEGNAME = token;
                Model_Category::instance().save(category);
            }

            categID_ = category->CATEGID;
            break;

        case UNIV_CSV_SUBCATEGORY:
            if (categID_ == -1)
                return;

            sub_category = (!token.IsEmpty() ? Model_Subcategory::instance().get(token, categID_) : 0);
            if (!sub_category)
            {
                sub_category = Model_Subcategory::instance().create();
                sub_category->CATEGID = categID_;
                sub_category->SUBCATEGNAME = token;
                Model_Subcategory::instance().save(sub_category);
            }
            subCategID_ = sub_category->SUBCATEGID; 
            break;

        case UNIV_CSV_NOTES:
            notes_ = token;
            break;

        case UNIV_CSV_TRANSNUM:
            transNum_ = token;
            break;

        case UNIV_CSV_DONTCARE:
            // do nothing
            break;

        case UNIV_CSV_DEPOSIT:
            if (!Model_Currency::fromString(token, val_), Model_Account::currency(Model_Account::instance().get(fromAccountID_))) return;
            if (val_ <= 0.0) return;

            type_ = TRANS_TYPE_DEPOSIT_STR;
            amount_ = token;
            break;

        case UNIV_CSV_WITHDRAWAL:
            if (!Model_Currency::fromString(token, val_), Model_Account::currency(Model_Account::instance().get(fromAccountID_))) return;
            if (val_ <= 0.0) return;

            type_ = TRANS_TYPE_WITHDRAWAL_STR;
            amount_ = token;
            break;

        case UNIV_CSV_BALANCE:
            // do nothing
            break;

        default:
            wxASSERT(true);
            break;
    }
}

void mmUnivCSVDialog::OnButtonClear(wxCommandEvent& /*event*/)
{
    log_field_->Clear();
}

void mmUnivCSVDialog::OnFileNameChanged(wxCommandEvent& event)
{
    wxString file_name = m_text_ctrl_->GetValue();
    if (file_name.Contains("\n") || file_name.Contains("file://"))
    {

        file_name.Replace("\n", "");
#ifdef __WXGTK__
        file_name.Replace("file://", "");
        file_name.Trim();
#endif
        m_text_ctrl_->SetEvtHandlerEnabled(false);
        m_text_ctrl_->SetValue(file_name);
        m_text_ctrl_->SetEvtHandlerEnabled(true);
    }
    event.Skip();

    wxFileName csv_file(file_name);
    if (csv_file.FileExists())
        this->update_preview();
}
void mmUnivCSVDialog::OnFileNameEntered(wxCommandEvent& event)
{
    wxString file_name = m_text_ctrl_->GetValue();
    file_name.Trim();

    event.Skip();
    wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, wxID_SEARCH);
    this->GetEventHandler()->AddPendingEvent(evt);
}

void mmUnivCSVDialog::OnDateFormatChanged(wxCommandEvent& /*event*/)
{
    this->update_preview();
    wxStringClientData* data = (wxStringClientData*)(choiceDateFormat_->GetClientObject(choiceDateFormat_->GetSelection()));
    if (data) date_format_ = data->GetData();
    *log_field_ << date_format_ << "\n";
}
