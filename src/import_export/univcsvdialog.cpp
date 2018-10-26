/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (c) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2015 Nikolay Akimov
 Copyright (C) 2015 Yosef

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

#include "univcsvdialog.h"

#include "images_list.h"
#include "constants.h"
#include "mmSimpleDialogs.h"
#include "paths.h"
#include "platfdep.h"
#include "util.h"
#include "option.h"
#include "webapp.h"
#include "parsers.h"

#include "Model_Setting.h"
#include "Model_Payee.h"
#include "Model_Category.h"
#include "Model_Subcategory.h"
#include "Model_Infotable.h"

#include <algorithm>

#include <wx/xml/xml.h>
#include <wx/spinctrl.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmUnivCSVDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmUnivCSVDialog, wxDialog)
    EVT_BUTTON(wxID_ADD, mmUnivCSVDialog::OnAdd)
    EVT_BUTTON(ID_UNIVCSVBUTTON_IMPORT, mmUnivCSVDialog::OnImport)
    EVT_BUTTON(ID_UNIVCSVBUTTON_EXPORT, mmUnivCSVDialog::OnExport)
    EVT_BUTTON(wxID_REMOVE, mmUnivCSVDialog::OnRemove)
    EVT_BUTTON(wxID_SAVEAS, mmUnivCSVDialog::OnSave)
    EVT_BUTTON(wxID_UP, mmUnivCSVDialog::OnMoveUp)
    EVT_BUTTON(wxID_DOWN, mmUnivCSVDialog::OnMoveDown)
    EVT_BUTTON(wxID_STANDARD, mmUnivCSVDialog::OnStandard)
    EVT_BUTTON(wxID_BROWSE, mmUnivCSVDialog::OnBrowse)
    EVT_LISTBOX_DCLICK(wxID_ANY, mmUnivCSVDialog::OnListBox)
    EVT_CHOICE(wxID_ANY, mmUnivCSVDialog::OnDateFormatChanged)
wxEND_EVENT_TABLE()

//----------------------------------------------------------------------------
mmUnivCSVDialog::mmUnivCSVDialog()
{
}

mmUnivCSVDialog::mmUnivCSVDialog(
    wxWindow* parent,
    EDialogType dialogType,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style
) :
    dialogType_(dialogType),
    delimit_(","),
    csvFieldCandicate_(nullptr),
    csvListBox_(nullptr),
    m_button_add_(nullptr),
    m_button_remove_(nullptr),
    m_choice_account_(nullptr),
    m_list_ctrl_(nullptr),
    m_text_ctrl_(nullptr),
    log_field_(nullptr),
    m_textDelimiter(nullptr),
    m_rowSelectionStaticBox_(nullptr),
    m_spinIgnoreFirstRows_(nullptr),
    m_spinIgnoreLastRows_(nullptr),
    choiceDateFormat_(nullptr),
    importSuccessful_(false)
{
    CSVFieldName_[UNIV_CSV_DATE] = wxTRANSLATE("Date");
    CSVFieldName_[UNIV_CSV_PAYEE] = wxTRANSLATE("Payee");
    CSVFieldName_[UNIV_CSV_AMOUNT] = wxTRANSLATE("Amount +/-");
    CSVFieldName_[UNIV_CSV_CATEGORY] = wxTRANSLATE("Category");
    CSVFieldName_[UNIV_CSV_SUBCATEGORY] = wxTRANSLATE("SubCategory");
    CSVFieldName_[UNIV_CSV_TRANSNUM] = wxTRANSLATE("Number");
    CSVFieldName_[UNIV_CSV_NOTES] = wxTRANSLATE("Notes");
    CSVFieldName_[UNIV_CSV_DONTCARE] = wxTRANSLATE("Don't Care");
    CSVFieldName_[UNIV_CSV_WITHDRAWAL] = wxTRANSLATE("Withdrawal");
    CSVFieldName_[UNIV_CSV_DEPOSIT] = wxTRANSLATE("Deposit");
    CSVFieldName_[UNIV_CSV_BALANCE] = wxTRANSLATE("Balance");
    Create(parent, IsImporter()?_("Import dialog"):_("Export dialog"), id, pos, size, style);
    this->Connect(wxID_ANY, wxEVT_CHILD_FOCUS, wxChildFocusEventHandler(mmUnivCSVDialog::changeFocus), nullptr, this);
}

bool mmUnivCSVDialog::Create(wxWindow* parent
    , const wxString& caption
    , wxWindowID id
    , const wxPoint& pos
    , const wxSize& size
    , long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    SetSettings(GetStoredSettings(-1));
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmex::getProgramIcon());
    Centre();
    return TRUE;
}

void mmUnivCSVDialog::CreateControls()
{
    wxSizerFlags flagsExpand;
    flagsExpand.Align(wxALIGN_LEFT | wxALIGN_TOP).Border(wxLEFT | wxRIGHT | wxTOP, 5);

    // Define the staticBox font and set it as wxFONTWEIGHT_BOLD
    wxFont staticBoxFontSetting = this->GetFont();

    wxBoxSizer* itemBoxSizer0 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer0);
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer0->Add(itemBoxSizer1, 0, wxGROW | wxALL, 0);
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer1->Add(itemBoxSizer2, 8, wxGROW | wxALL, 5);
    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer1->Add(itemBoxSizer11, 5, wxGROW | wxALL, 5);

    //File to import, file path and browse button
    wxPanel* itemPanel6 = new wxPanel(this
        , wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer2->Add(itemPanel6, 0, wxEXPAND | wxALL, 1);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel6->SetSizer(itemBoxSizer7);

    wxStaticText* itemStaticText5 = new wxStaticText(itemPanel6, wxID_ANY, _("File Name:"));
    itemBoxSizer7->Add(itemStaticText5, g_flagsH);
    itemStaticText5->SetFont(staticBoxFontSetting);

    m_text_ctrl_ = new wxTextCtrl(itemPanel6
        , ID_FILE_NAME, wxEmptyString, wxDefaultPosition, wxSize(300, -1), wxTE_PROCESS_ENTER);
    itemBoxSizer7->Add(m_text_ctrl_, 1, wxALL | wxGROW, 5);
    m_text_ctrl_->Connect(ID_FILE_NAME
        , wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(mmUnivCSVDialog::OnFileNameChanged), nullptr, this);
    m_text_ctrl_->Connect(ID_FILE_NAME
        , wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(mmUnivCSVDialog::OnFileNameEntered), nullptr, this);

    const wxString& file_button_label = IsImporter() ? _("&Browse") : _("File");
    wxButton* button_browse = new wxButton(itemPanel6, wxID_BROWSE, file_button_label);
    const wxString& file_tooltip = IsImporter()
        ? _("Choose CSV data file to Import") : _("Choose CSV data file to Export");
    button_browse->SetToolTip(file_tooltip);
    itemBoxSizer7->Add(button_browse, g_flagsH);

    // Predefined settings
    wxPanel* itemPanel67 = new wxPanel(this
        , wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* itemBoxSizer76 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel67->SetSizer(itemBoxSizer76);
    itemBoxSizer2->Add(itemPanel67, wxSizerFlags(g_flagsExpand).Proportion(0).Border(0));
    const wxString settings_choice [] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
    wxRadioBox* radio_box = new wxRadioBox(itemPanel67
        , wxID_APPLY, "", wxDefaultPosition, wxDefaultSize
        , sizeof(settings_choice) / sizeof(wxString)
        , settings_choice, 10, wxRA_SPECIFY_COLS);
    itemBoxSizer76->Add(radio_box, wxSizerFlags(g_flagsH).Center().Proportion(1));
    radio_box->Connect(wxID_APPLY, wxEVT_COMMAND_RADIOBOX_SELECTED
        , wxCommandEventHandler(mmUnivCSVDialog::OnSettingsSelected), nullptr, this);

    wxBitmapButton* itemButton_Save = new wxBitmapButton(itemPanel67
        , wxID_SAVEAS, mmBitmap(png::SAVEAS));
    itemBoxSizer76->Add(itemButton_Save, wxSizerFlags(g_flagsH).Center().Proportion(0));
    itemButton_Save->SetToolTip(_("Save Template"));

    //
    wxStaticText* itemStaticText3 = new wxStaticText(this, wxID_STATIC
       , _("Specify the order of fields in the file"));
    itemBoxSizer2->Add(itemStaticText3, g_flagsV);
    itemStaticText3->SetFont(staticBoxFontSetting);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW | wxALL, 5);

    //CSV fields candicate
    csvFieldCandicate_ = new wxListBox(this, ID_LISTBOX_CANDICATE
        , wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE | wxLB_NEEDED_SB);
    itemBoxSizer3->Add(csvFieldCandicate_, 1, wxGROW | wxALL, 1);
    for (const auto& it : CSVFieldName_)
        csvFieldCandicate_->Append(wxGetTranslation(it.second), new mmListBoxItem(it.first, it.second));

     //Add Remove Area
    wxPanel* itemPanel_AddRemove = new wxPanel(this, ID_PANEL10,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer3->Add(itemPanel_AddRemove, g_flagsH);

    wxBoxSizer* itemBoxSizer_AddRemove = new wxBoxSizer(wxVERTICAL);
    itemPanel_AddRemove->SetSizer(itemBoxSizer_AddRemove);

    //Add button
    m_button_add_= new wxButton(itemPanel_AddRemove, wxID_ADD, _("&Add "));
    itemBoxSizer_AddRemove->Add(m_button_add_, g_flagsV);

    //Remove button
    m_button_remove_ = new wxButton(itemPanel_AddRemove, wxID_REMOVE, _("&Remove "));
    itemBoxSizer_AddRemove->Add(m_button_remove_, g_flagsV);

    //Standard MMEX CSV
    wxButton* itemButton_standard = new wxButton(itemPanel_AddRemove, wxID_STANDARD, _("&MMEX format"));
    itemBoxSizer_AddRemove->Add(itemButton_standard, g_flagsV);
    itemButton_standard->SetToolTip(_("MMEX standard format"));

    //ListBox of attribute order
    csvListBox_ = new wxListBox(this, ID_LISTBOX
        , wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE | wxLB_NEEDED_SB);
    itemBoxSizer3->Add(csvListBox_, 1, wxGROW|wxALL, 1);

   //Arranger Area
    wxPanel* itemPanel_Arranger = new wxPanel(this, ID_PANEL10
        , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer3->Add(itemPanel_Arranger, 0, wxALL, 1);

    wxBoxSizer* itemBoxSizer_Arranger = new wxBoxSizer(wxVERTICAL);
    itemPanel_Arranger->SetSizer(itemBoxSizer_Arranger);

    //Move Up button
    wxButton* itemButton_MoveUp = new wxButton(itemPanel_Arranger, wxID_UP, _("&Up"));
    itemBoxSizer_Arranger->Add(itemButton_MoveUp, g_flagsV);
    itemButton_MoveUp -> SetToolTip (_("Move Up"));

    //Move down button
    wxButton* itemButton_MoveDown = new wxButton(itemPanel_Arranger, wxID_DOWN, _("&Down"));
    itemBoxSizer_Arranger->Add(itemButton_MoveDown, g_flagsV);
    itemButton_MoveDown->SetToolTip(_("Move &Down"));

    wxStaticLine*  m_staticline1 = new wxStaticLine(this
        , wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(m_staticline1, flagsExpand );

    // account to import or export
    wxPanel* itemPanel7 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer2->Add(itemPanel7, 0, wxEXPAND | wxALL, 1);

    wxBoxSizer* itemBoxSizer08 = new wxBoxSizer(wxVERTICAL);
    itemPanel7->SetSizer(itemBoxSizer08);

    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 4, 0, 0);
    itemBoxSizer08->Add(flex_sizer);

    wxStaticText* itemStaticText6 = new wxStaticText(itemPanel7
        , wxID_ANY, _("Account: "));
    flex_sizer->Add(itemStaticText6, g_flagsH);
    itemStaticText6->SetFont(staticBoxFontSetting);

    m_choice_account_ = new wxChoice(itemPanel7, wxID_ACCOUNT, wxDefaultPosition, wxSize(210, -1)
        , Model_Account::instance().all_checking_account_names(), 0);
    m_choice_account_->SetSelection(0);
    flex_sizer->Add(m_choice_account_, g_flagsH);

    wxStaticLine*  m_staticline2 = new wxStaticLine(this
        , wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(m_staticline2, flagsExpand );

    wxStaticText* itemStaticText66 = new wxStaticText(itemPanel7, wxID_STATIC, wxString(_("Date Format: ")));
    flex_sizer->Add(itemStaticText66, g_flagsH);
    itemStaticText66->SetFont(staticBoxFontSetting);

    date_format_ = Option::instance().DateFormat();
    choiceDateFormat_ = new wxChoice(itemPanel7, ID_DATE_FORMAT);
    initDateMask();
    flex_sizer->Add(choiceDateFormat_, g_flagsH);

    // CSV Delimiter
    if (IsCSV())
    {
        wxStaticText* itemStaticText77 = new wxStaticText(itemPanel7, wxID_STATIC, wxString(_("CSV Delimiter:")));
        itemStaticText77->SetFont(staticBoxFontSetting);
        flex_sizer->Add(itemStaticText77, g_flagsH);

        m_textDelimiter = new wxTextCtrl(itemPanel7, ID_UD_DELIMIT, "", wxDefaultPosition, wxDefaultSize
            , wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB);
        m_textDelimiter->SetToolTip(_("Specify the delimiter to use when importing/exporting CSV files"));
        m_textDelimiter->SetMaxLength(1);
        m_textDelimiter->Connect(ID_UD_DELIMIT
            , wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(mmUnivCSVDialog::OnDelimiterChange), nullptr, this);
        initDelimiter();
        flex_sizer->Add(m_textDelimiter, g_flagsH);
    }

    //Encoding
    wxStaticText* itemStaticText88 = new wxStaticText(itemPanel7, wxID_STATIC, wxString(_("Encoding:")));
    itemStaticText88->SetFont(staticBoxFontSetting);
    flex_sizer->Add(itemStaticText88, g_flagsH);

    m_choiceEncoding = new wxChoice(itemPanel7, ID_ENCODING);
    for (const auto i : g_encoding)
        m_choiceEncoding->Append(wxGetTranslation(i.second.second), new wxStringClientData(i.second.second));
    m_choiceEncoding->SetSelection(0);

    flex_sizer->Add(m_choiceEncoding, g_flagsH);

    // Option to add column titles to exported files.
    if (!IsImporter())
    {
        m_checkBoxExportTitles = new wxCheckBox(itemPanel7, wxID_ANY, _("Export column titles"));
        flex_sizer->Add(m_checkBoxExportTitles, g_flagsH);
    }

    // Determine meaning of "amount" field's sign- deposit or withdrawal. 
    // When importing, there format is given and can be either. Exporting is best to be consistent and so this option is not given.
    if (IsImporter())
    {
        // Text title.
        wxStaticText* itemStaticTextAmount = new wxStaticText(itemPanel7, wxID_ANY, _("Amount +/-:"));
        flex_sizer->Add(itemStaticTextAmount, g_flagsH);
        itemStaticTextAmount->SetFont(staticBoxFontSetting);

        // Choice selection.
        m_choiceAmountFieldSign = new wxChoice(itemPanel7, wxID_ANY);
        m_choiceAmountFieldSign->Append(wxGetTranslation(_("Positive values are deposits")));
        m_choiceAmountFieldSign->Append(wxGetTranslation(_("Positive values are withdrawals")));
        m_choiceAmountFieldSign->SetSelection(PositiveIsDeposit);
        flex_sizer->Add(m_choiceAmountFieldSign, g_flagsH);
    }

    // Select rows to import (not relevant for export)
    if(IsImporter())
    {
        // Container.
        m_rowSelectionStaticBox_ = new wxStaticBox(this, wxID_ANY, _("Rows to ignore"));
        m_rowSelectionStaticBox_->SetFont(staticBoxFontSetting);
        wxStaticBoxSizer* rowSelectionStaticBoxSizer = new wxStaticBoxSizer(m_rowSelectionStaticBox_, wxHORIZONTAL);
        itemBoxSizer2->Add(rowSelectionStaticBoxSizer, 0, wxALL | wxEXPAND, 5);

        // "Ignore first" title, spin and event handler.
        wxStaticText* itemStaticText7 = new wxStaticText(rowSelectionStaticBoxSizer->GetStaticBox(), wxID_ANY, _("From start: "));
        rowSelectionStaticBoxSizer->Add(itemStaticText7, g_flagsH);
        m_spinIgnoreFirstRows_ = new wxSpinCtrl(rowSelectionStaticBoxSizer->GetStaticBox(), ID_FIRST_ROW, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0, 0);
        rowSelectionStaticBoxSizer->Add(m_spinIgnoreFirstRows_, g_flagsH);
        m_spinIgnoreFirstRows_->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler(mmUnivCSVDialog::OnSpinCtrlIgnoreRows), nullptr, this);

        // "Ignore last" title, spin and event handler.
        wxStaticText* itemStaticText8 = new wxStaticText(rowSelectionStaticBoxSizer->GetStaticBox(), wxID_ANY, _("From end: "));
        rowSelectionStaticBoxSizer->Add(itemStaticText8, g_flagsH);
        m_spinIgnoreLastRows_ = new wxSpinCtrl(rowSelectionStaticBoxSizer->GetStaticBox(), ID_LAST_ROW, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0, 0);
        rowSelectionStaticBoxSizer->Add(m_spinIgnoreLastRows_, g_flagsH);
        m_spinIgnoreLastRows_->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler(mmUnivCSVDialog::OnSpinCtrlIgnoreRows), nullptr, this);
    }

    // Preview
    wxStaticBoxSizer* m_staticbox = new wxStaticBoxSizer(new wxStaticBox(this, wxID_STATIC, _("Preview")), wxVERTICAL);

    m_list_ctrl_ = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    m_staticbox->Add(m_list_ctrl_, 1, wxGROW | wxALL, 5);
    itemBoxSizer0->Add(m_staticbox, 2, wxALL | wxEXPAND, 5);

    //Import File button
    wxPanel* itemPanel5 = new wxPanel(this, ID_PANEL10
        , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer0->Add(itemPanel5, 0, wxALIGN_RIGHT | wxALL, 1);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel5->SetSizer(itemBoxSizer6);

    if (IsImporter())
    {
        wxButton* itemButton_Import = new wxButton(itemPanel5, ID_UNIVCSVBUTTON_IMPORT, _("&Import")
            , wxDefaultPosition, wxDefaultSize, 0);
        itemBoxSizer6->Add(itemButton_Import, 0, wxALIGN_CENTER|wxALL, 5);
        itemButton_Import -> SetToolTip (_("Import File"));
    }
    else
    {
        wxButton* itemButton_Export = new wxButton(itemPanel5, ID_UNIVCSVBUTTON_EXPORT, _("&Export")
            , wxDefaultPosition, wxDefaultSize, 0);
        itemBoxSizer6->Add(itemButton_Export, 0, wxALIGN_CENTER | wxALL, 5);
        itemButton_Export -> SetToolTip (_("Export File"));
    }

    wxButton* itemCancelButton = new wxButton(itemPanel5, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    itemBoxSizer6->Add(itemCancelButton, 0, wxALIGN_CENTER | wxALL, 5);
    itemCancelButton->SetFocus();

    //Log viewer
    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxVERTICAL);

    itemBoxSizer11->Add(itemBoxSizer22, 1, wxGROW|wxALL, 0);

    log_field_ = new wxTextCtrl( this
        , wxID_STATIC, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxHSCROLL | wxTE_READONLY);
    itemBoxSizer22->Add(log_field_, 1, wxGROW | wxALL, 5);

    wxButton* itemClearButton = new wxButton(this, wxID_CLEAR, _("Clear"));
    itemBoxSizer22->Add(itemClearButton, 0, wxALIGN_CENTER | wxALL, 5);
    itemClearButton->Connect(wxID_CLEAR, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmUnivCSVDialog::OnButtonClear), nullptr, this);

}

void mmUnivCSVDialog::initDateMask()
{
    for (const auto& i : g_date_formats_map)
    {
        choiceDateFormat_->Append(i.second, new wxStringClientData(i.first));
        if (date_format_ == i.first) {
            choiceDateFormat_->SetStringSelection(i.second);
        }
    }
}

void mmUnivCSVDialog::initDelimiter()
{
    if (delimit_.empty()) {
        delimit_ = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);
    }
    m_textDelimiter->ChangeValue(delimit_);

}

void mmUnivCSVDialog::OnSettingsSelected(wxCommandEvent& event)
{
    SetSettings(GetStoredSettings(event.GetSelection()));
}

wxString mmUnivCSVDialog::GetStoredSettings(int id)
{
    if (id < 0) id = 0;
    const wxString& setting_id = wxString::Format(GetSettingsPrfix() + "%d", id);
    const wxString& settings_string = Model_Setting::instance().GetStringSetting(setting_id, "");
    wxLogDebug("%s \n %s", setting_id, settings_string);
    return settings_string;
}

void mmUnivCSVDialog::SetSettings(const wxString &json_data)
{
    Document json_doc;
    if (json_doc.Parse(json_data.c_str()).HasParseError()) {
        json_doc.Parse("{}");
    }

    //Date Mask
    Value& date_mask = GetValueByPointerWithDefault(json_doc, "/DATE_MASK", "");
    const wxString df = date_mask.IsString() ? date_mask.GetString() : "";
    if (!df.empty())
    {
        date_format_ = df;
        const auto pos = g_date_formats_map.find(date_format_);
        if (pos != g_date_formats_map.end()) {
            choiceDateFormat_->SetStringSelection(pos->second);
        }
        else {
            wxLogDebug("Unrecognized DATE_MASK %s", df);
        }
    }

    //File
    Value& file_name = GetValueByPointerWithDefault(json_doc, "/FILE_NAME", "");
    const auto fn = file_name.IsString() ? file_name.GetString() : "";
    m_text_ctrl_->ChangeValue(fn);

    // Account
    Value& account_name = GetValueByPointerWithDefault(json_doc, "/ACCOUNT_NAME", "");
    const wxString& an = account_name.IsString() ? account_name.GetString() : "";
    if (!an.empty())
    {
        int itemIndex = m_choice_account_->FindString(an);
        if (wxNOT_FOUND == itemIndex)
            mmErrorDialogs::MessageError(m_choice_account_
                , wxString::Format(_("Default account '%s' for this template does not exist.\n"
                    "Please select a new account."), an)
                ,_("Account does not exist"));
        else
            m_choice_account_->Select(itemIndex);
    }
    else {
        m_choice_account_->Select(-1);
    }

    //Delimiter
    if (IsCSV())
    {
        Value& v_delimiter = GetValueByPointerWithDefault(json_doc, "/DELIMITER", "");
        const wxString& de = v_delimiter.IsString() ? v_delimiter.GetString() : "";
        const wxString& def_delimiter = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);
        delimit_ = de.empty() ? def_delimiter : wxString::FromUTF8(v_delimiter.GetString());
        initDelimiter();
    }


    //CSV fields
    csvFieldOrder_.clear();
    if (json_doc.HasMember("FIELDS") && json_doc["FIELDS"].IsArray())
    {
        Value a = json_doc["FIELDS"].GetArray();
        if (a.IsArray()) 
        {
            for (auto& v : a.GetArray())
            {
                if (v.IsObject())
                {
                    auto obj = v.GetObject(); 
                    const auto value = wxString::FromUTF8(obj.begin()->value.GetString());

                    for (const auto& entry : CSVFieldName_)
                    {
                        if (entry.second == value || wxGetTranslation(entry.second) == value)
                        {
                            int key = entry.first;
                            csvFieldOrder_.push_back(key);
                            break;
                        }
                    }
                }
            }
        }
    }

    if (IsImporter())
    {
        // Amount sign.
        if (json_doc.HasMember("AMOUNT_SIGN") && json_doc["AMOUNT_SIGN"].IsInt())
        {
            int val = json_doc["AMOUNT_SIGN"].GetInt();
            m_choiceAmountFieldSign->Select(val);
        }

        // Row selection settings.
        if (json_doc.HasMember("IGNORE_FIRST_ROWS") && json_doc["IGNORE_FIRST_ROWS"].IsInt())
        {
            int val = json_doc["IGNORE_FIRST_ROWS"].GetInt();
            m_spinIgnoreFirstRows_->SetRange(m_spinIgnoreFirstRows_->GetMin()
                , std::max(val, m_spinIgnoreFirstRows_->GetMax())); // Called before file is loaded so max might still be 0.
            m_spinIgnoreFirstRows_->SetValue(val);
        }

        if (json_doc.HasMember("IGNORE_LAST_ROWS") && json_doc["IGNORE_LAST_ROWS"].IsInt())
        {
            
            int val = json_doc["IGNORE_LAST_ROWS"].GetInt();
            m_spinIgnoreLastRows_->SetRange(m_spinIgnoreLastRows_->GetMin()
                , std::max(val, m_spinIgnoreLastRows_->GetMax())); // Called before file is loaded so max might still be 0.
            m_spinIgnoreLastRows_->SetValue(val);
        }
    }
    else
    {
        if (json_doc.HasMember("EXPORT_TITLES") && json_doc["EXPORT_TITLES"].IsBool())
        {
            m_checkBoxExportTitles->SetValue(json_doc["EXPORT_TITLES"].GetBool());
        }
    }

    OnLoad();
    this->update_preview();
}

//Selection dialog for fields to be added to listbox
void mmUnivCSVDialog::OnAdd(wxCommandEvent& WXUNUSED(event))
{
    int index = csvFieldCandicate_->GetSelection();
    if (index != wxNOT_FOUND)
    {
        mmListBoxItem* item = static_cast<mmListBoxItem*> (csvFieldCandicate_->GetClientObject(index));

        int target_position = csvListBox_->GetSelection();
        if (target_position == wxNOT_FOUND) {
            target_position = csvListBox_->GetCount();
        }
        else {
            target_position++;
        }

        mmListBoxItem* i = new mmListBoxItem(item->getIndex(), item->getName());
        csvListBox_->Insert(wxGetTranslation(item->getName()), target_position, i);
        csvListBox_->SetSelection(target_position);

        auto itPos = csvFieldOrder_.begin() + target_position;
        auto newIt = csvFieldOrder_.insert(itPos, item->getIndex());

        if (item->getIndex() != UNIV_CSV_DONTCARE)
        {
            csvFieldCandicate_->Delete(index);
            if (index < (int)csvFieldCandicate_->GetCount()) {
                csvFieldCandicate_->SetSelection(index, true);
            }
            else {
                csvFieldCandicate_->SetSelection(csvFieldCandicate_->GetCount() - 1, true);
            }
        }

        this->update_preview();
    }
}

//Removes an item from the field list box
void mmUnivCSVDialog::OnRemove(wxCommandEvent& WXUNUSED(event))
{
    int index = csvListBox_->GetSelection();
    if (index != wxNOT_FOUND)
    {
        mmListBoxItem *item = static_cast<mmListBoxItem*>(csvListBox_->GetClientObject(index));
        int item_index = item->getIndex();
        wxString item_name = item->getName();

        if (item_index != UNIV_CSV_DONTCARE)
        {
            int pos = 0;
            for (pos = 0; pos < (int)csvFieldCandicate_->GetCount() - 1; pos ++)
            {
                mmListBoxItem *item2 = static_cast<mmListBoxItem*>(csvFieldCandicate_->GetClientObject(pos));
                if (item_index < item2->getIndex()) {
                    break;
                }
            }
            csvFieldCandicate_->Insert(wxGetTranslation(item_name), pos, new mmListBoxItem(item_index, item_name));
        }

        csvListBox_->Delete(index);
        csvFieldOrder_.erase(csvFieldOrder_.begin() + index);

        if (index < (int)csvListBox_->GetCount()) {
            csvListBox_->SetSelection(index, true);
        }
        else {
            csvListBox_->SetSelection(csvListBox_->GetCount() - 1, true);
        }

        this->update_preview();
    }
}

const wxString mmUnivCSVDialog::getCSVFieldName(int index) const
{
    std::map<int, wxString>::const_iterator it = CSVFieldName_.find(index);
    if (it != CSVFieldName_.end()) {
        return wxGetTranslation(it->second);
    }

    return _("Unknown");
}

void mmUnivCSVDialog::OnLoad()
{
    csvListBox_->Clear();
    long num = 0;
    for (const auto& entry : csvFieldOrder_)
    {
        const wxString& item_name = CSVFieldName_[entry];
        csvListBox_->Append(wxGetTranslation(item_name), new mmListBoxItem(num++, item_name));
    }
    // update csvFieldCandicate_
    csvFieldCandicate_->Clear();
    for (const auto& entry : CSVFieldName_)
    {
        std::vector<int>::const_iterator loc = find(csvFieldOrder_.begin(), csvFieldOrder_.end(), entry.first);
        if (loc == csvFieldOrder_.end() || entry.first == UNIV_CSV_DONTCARE)
            csvFieldCandicate_->Append(wxGetTranslation(entry.second), new mmListBoxItem(entry.first, entry.second));
    }
}

//Saves the field order to a template file
void mmUnivCSVDialog::OnSave(wxCommandEvent& WXUNUSED(event))
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();
    
    wxRadioBox* c = (wxRadioBox*) FindWindow(wxID_APPLY);
    int id = c->GetSelection();
    const wxString& settingsPrefix = GetSettingsPrfix();
    const wxString& setting_id = wxString::Format(settingsPrefix +"%d", id);

    const auto an = m_choice_account_->GetStringSelection();
    if (!an.empty())
    {
        json_writer.Key("ACCOUNT_NAME");
        json_writer.String(an.c_str());
    }
    if (!date_format_.empty())
    {
        json_writer.Key("DATE_MASK");
        json_writer.String(date_format_.c_str());
    }
    if (!delimit_.empty())
    {
        json_writer.Key("DELIMITER");
        json_writer.String(delimit_.c_str());
    }

    const auto fileName = m_text_ctrl_->GetValue();
    if (!fileName.empty())
    {
        json_writer.Key("FILE_NAME");
        json_writer.String(fileName.c_str());
    }

    if (IsImporter())
    {
        // Amount sign
        const auto s = m_choiceAmountFieldSign->GetCurrentSelection();
        json_writer.Key("AMOUNT_SIGN");
        json_writer.Int(s);

        // Rows to ignore
        const auto ifr = m_spinIgnoreFirstRows_->GetValue();
        json_writer.Key("IGNORE_FIRST_ROWS");
        json_writer.Int(ifr);

        const auto ilr = m_spinIgnoreLastRows_->GetValue();
        json_writer.Key("IGNORE_LAST_ROWS");
        json_writer.Int(ilr);
    }
    else
    {
        const auto et = m_checkBoxExportTitles->IsChecked();
        json_writer.Key("EXPORT_TITLES");
        json_writer.Bool(et);
    }

    json_writer.Key("FIELDS");
    json_writer.StartArray();
    size_t count = 0;
    for (std::vector<int>::const_iterator it = csvFieldOrder_.begin(); it != csvFieldOrder_.end(); ++it)
    {
        int i = *it;
        json_writer.StartObject();
        json_writer.Key(wxString::Format("%zu", count++).c_str());
        json_writer.String(CSVFieldName_[i].c_str());
        json_writer.EndObject();
    }
    json_writer.EndArray();

    json_writer.EndObject();

    const wxString json_data = json_buffer.GetString();

    Model_Setting::instance().Set(setting_id, json_data);
}

bool mmUnivCSVDialog::validateData(tran_holder & holder)
{
    if (!holder.Date.IsValid() || !holder.Amount || holder.Type.Trim().IsEmpty()) {
        return false;
    }

    Model_Payee::Data* payee = Model_Payee::instance().get(holder.PayeeID);
    if (!payee) 
    {
        Model_Payee::Data* u = Model_Payee::instance().get(_("Unknown"));
        if (!u) {
            Model_Payee::Data *p = Model_Payee::instance().create();
            p->PAYEENAME = _("Unknown");
            p->CATEGID = -1;
            p->SUBCATEGID = -1;
            holder.PayeeID = Model_Payee::instance().save(p);
            const wxString& sMsg = wxString::Format(_("Added payee: %s"), p->PAYEENAME);
            log_field_->AppendText(wxString() << sMsg << "\n");
        }
        else {
            holder.PayeeID = u->PAYEEID;
        }
    }
    else
    {
        if (holder.CategoryID < 0){
            holder.CategoryID = payee->CATEGID;
            holder.SubCategoryID = payee->SUBCATEGID;
        }
    }

    if (holder.CategoryID == -1) //The category name is missing in SCV file and not assigned for the payee
    {
        Model_Category::Data* categ = Model_Category::instance().get(_("Unknown"));
        if (categ) {
            holder.CategoryID = categ->CATEGID;
        }
        else
        {
            Model_Category::Data *c = Model_Category::instance().create();
            c->CATEGNAME = _("Unknown");
            holder.CategoryID = Model_Category::instance().save(c);
        }
    }

    return true;
}

void mmUnivCSVDialog::OnImport(wxCommandEvent& WXUNUSED(event))
{
    // date and amount are required
    bool datefield = isIndexPresent(UNIV_CSV_DATE);
    bool amountfields = isIndexPresent(UNIV_CSV_AMOUNT)
                    || (isIndexPresent(UNIV_CSV_WITHDRAWAL)
                     && isIndexPresent(UNIV_CSV_DEPOSIT));
    if (!datefield || !amountfields)
        return mmErrorDialogs::ToolTip4Object(csvListBox_
            , _("Incorrect fields specified for import!")
                + (!datefield ? "\n" + _("Date field is required.") : "")
                + (!amountfields ? "\n" + _("Amount field or both Withdrawal and Deposit fields are required.") : "")
            , _("Import"), wxICON_WARNING);

    bool canceledbyuser = false;
    long countImported = 0;    
    const wxString acctName = m_choice_account_->GetStringSelection();
    Model_Account::Data* from_account = Model_Account::instance().get(acctName);

    if (!from_account) 
    { 
        return mmErrorDialogs::InvalidAccount(m_choice_account_);
    }
    fromAccountID_ = from_account->ACCOUNTID;

    const wxString fileName = m_text_ctrl_->GetValue();
    if (fileName.IsEmpty()) {
        return mmErrorDialogs::InvalidFile(m_text_ctrl_);
    }

    // Open and parse file
    ITransactionsFile *pParser = CreateFileHandler();
    if (!pParser) return; // is this possible?
    if (!pParser->Load(fileName, m_list_ctrl_->GetColumnCount())) {
        return wxDELETE(pParser);
    }

    wxFileName logFile = mmex::GetLogDir(true);
    logFile.SetFullName(fileName);
    logFile.SetExt("txt");

    wxFileOutputStream outputLog(logFile.GetFullPath());
    wxTextOutputStream log(outputLog);

    /* date, payeename, amount(+/-), Number, status, category : subcategory, notes */
    const long totalLines = pParser->GetLinesCount();
    long firstRow = m_spinIgnoreFirstRows_->GetValue();
    long lastRow = totalLines - m_spinIgnoreLastRows_->GetValue();
    const long linesToImport = lastRow-firstRow;
    long countEmptyLines = 0;

    Model_Checking::instance().Savepoint();

    wxProgressDialog progressDlg(_("Universal CSV Import")
        , wxEmptyString, linesToImport
        , nullptr, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_CAN_ABORT
                 | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME
        );

    for (long lineNum = firstRow; lineNum < lastRow; lineNum++)
    {
        const wxString& progressMsg = wxString::Format(_("Transactions imported to account %s: %ld")
            , "'" + acctName + "'", countImported);
        if (!progressDlg.Update(lineNum-firstRow,progressMsg))
        {
            canceledbyuser = true;
            break; // abort processing
        }

        unsigned int numTokens = pParser->GetItemsCount(lineNum);
        if (numTokens ==0)
        {
            wxString msg = wxString::Format(_("Line %ld: Empty"), lineNum+1);
            log << msg << endl;
            *log_field_ << msg << "\n";
            countEmptyLines++;
            continue;
        }

        tran_holder holder;
        for (size_t i = 0; i < csvFieldOrder_.size() && i < numTokens; ++i) {
            parseToken(csvFieldOrder_[i], pParser->GetItem(lineNum, i).Trim(false /*from left*/), holder);
        }

        if (!validateData(holder)) 
        {
            wxString msg = wxString::Format(_("Line %ld: Error:"), lineNum+1);
            if (!holder.Date.IsValid())
                msg << " " << _("Invalid Date.");
            if (!holder.Amount) msg << " " << _("Invalid Amount.");
            if (holder.Type.Trim().IsEmpty())
                msg << " " << _("Type (withdrawal/deposit) unknown.");

            log << msg << endl;
            *log_field_ << msg << "\n";

            continue;
        }

        Model_Checking::Data *pTransaction = Model_Checking::instance().create();
        pTransaction->TRANSDATE = holder.Date.FormatISODate();
        pTransaction->ACCOUNTID = fromAccountID_;
        pTransaction->TOACCOUNTID = holder.ToAccountID;
        pTransaction->PAYEEID = holder.PayeeID;
        pTransaction->TRANSCODE = holder.Type;
        pTransaction->TRANSAMOUNT = holder.Amount;
        pTransaction->TOTRANSAMOUNT = holder.ToAmount;
        pTransaction->CATEGID = holder.CategoryID;
        pTransaction->SUBCATEGID = holder.SubCategoryID;
        pTransaction->STATUS = holder.Status;
        pTransaction->TRANSACTIONNUMBER = holder.Number;
        pTransaction->NOTES = holder.Notes;

        Model_Checking::instance().save(pTransaction);

        countImported++;
        wxString msg = wxString::Format(_("Line %ld: OK, imported."), lineNum+1);
        log << msg << endl;
        *log_field_ << msg << "\n";
    }

    wxDELETE(pParser);
    progressDlg.Update(linesToImport);

    wxString msg = wxString::Format(_("Total Lines: %ld"), totalLines);
    msg << "\n";
    msg << wxString::Format(_("Lines Selected to Import: %ld"), linesToImport);
    msg << "\n\n";
    msg << wxString::Format(_("Empty Lines: %ld"), countEmptyLines);
    msg << "\n";
    msg << wxString::Format(_("Imported: %ld"), countImported);
    msg << "\n";
    msg << wxString::Format(_("Errored: %ld")
        , linesToImport-countEmptyLines-countImported);
    msg << "\n\n";
    msg << wxString::Format(_("Log file written to: %s"), logFile.GetFullPath());

    if (!canceledbyuser && wxMessageBox(
        msg + (countImported > 0 ? "\n\n" + _("Please confirm saving...") : "")
        , _("Import")
        , wxOK | (countImported > 0 ? wxCANCEL : 0)
            | (countImported ==0 ? wxICON_ERROR :
                countImported < linesToImport-countEmptyLines
                    ? wxICON_EXCLAMATION
                    : wxICON_INFORMATION
                )
        ) == wxCANCEL)
            canceledbyuser = true;

    msg << "\n\n";

    // Since all database transactions are only in memory,
    if (!canceledbyuser && countImported > 0)
    {
        // we need to save them to the database.
        Model_Checking::instance().ReleaseSavepoint();
        mmWebApp::MMEX_WebApp_UpdateAccount();
        mmWebApp::MMEX_WebApp_UpdatePayee();
        mmWebApp::MMEX_WebApp_UpdateCategory();
        importSuccessful_ = true;
        // TODO: user cannot see following messages because dialog
        // will be closed (?)
        // msg << _("Imported transactions have been flagged so you can review them.") << "\n";
        // msg << wxString::Format(_("Transactions saved to database in account: %s"), acctName);
    }
    else
    {
        // and discard the database changes.
        Model_Checking::instance().Rollback();
        if (canceledbyuser) msg << _("Imported transactions discarded by user!");
        else msg << _("No imported transactions!");
        msg << "\n\n";           
    }

    *log_field_ << "\n" << msg;

    outputLog.Close();

    if (!canceledbyuser && countImported > 0) Close();
}

void mmUnivCSVDialog::OnExport(wxCommandEvent& WXUNUSED(event))
{
    // date and amount are required
    if (!isIndexPresent(UNIV_CSV_DATE) || (!isIndexPresent(UNIV_CSV_AMOUNT)
        && (!isIndexPresent(UNIV_CSV_WITHDRAWAL) || !isIndexPresent(UNIV_CSV_DEPOSIT))))
    {
        return mmErrorDialogs::MessageWarning(this
            , _("Incorrect fields specified for export! Requires at least Date and Amount.")
            , _("Export"));
    }

    const wxString& fileName = m_text_ctrl_->GetValue();
    if (fileName.IsEmpty())
        return mmErrorDialogs::InvalidFile(m_text_ctrl_);

    wxFileName out_file(fileName);
    if (out_file.Exists()) 
    {
        if (wxMessageBox(_("Overwrite existing file?"), _("File exists"), wxYES_NO | wxICON_WARNING) != wxYES)
            return;
 
        if (!wxRemoveFile(fileName))
        {
            return mmErrorDialogs::MessageWarning(this,
                _("Failed to delete existing file. File may be locked by another program."),
                _("Destination file error"));
        }
    }
 
    const wxString& acctName = m_choice_account_->GetStringSelection();
    Model_Account::Data* from_account = Model_Account::instance().get(acctName);

    if (!from_account)
        return;

    const auto split = Model_Splittransaction::instance().get_all();
    int fromAccountID = from_account->ACCOUNTID;

    wxDateTime trx_date;

    long numRecords = 0;
    Model_Currency::Data* currency = Model_Account::currency(from_account);

    ITransactionsFile *pTxFile = CreateFileHandler();

    // Write titles to file.
    if (m_checkBoxExportTitles->IsChecked())
    {
        pTxFile->AddNewLine();
        for (std::vector<int>::const_iterator sit = csvFieldOrder_.begin(); sit != csvFieldOrder_.end(); ++sit)
        {
            pTxFile->AddNewItem(wxGetTranslation(CSVFieldName_[*sit]));
        }
    }

    // Write transactions to file.
    double account_balance = from_account->INITIALBAL;
    for (const auto& pBankTransaction : Model_Checking::instance().find_or(Model_Checking::ACCOUNTID(fromAccountID)
        , Model_Checking::TOACCOUNTID(fromAccountID)))
    {
        if (Model_Checking::status(pBankTransaction) == Model_Checking::VOID_)
            continue;

        pTxFile->AddNewLine();

        Model_Checking::Full_Data tran(0, pBankTransaction, split);

        double value = Model_Checking::balance(pBankTransaction, fromAccountID);
        account_balance += value;

        const wxString& amount = Model_Currency::toStringNoFormatting(value, currency);
        const wxString& amount_abs = Model_Currency::toStringNoFormatting(fabs(value), currency);

        Model_Category::Data* category = Model_Category::instance().get(pBankTransaction.CATEGID);
        Model_Subcategory::Data* sub_category = Model_Subcategory::instance().get(pBankTransaction.SUBCATEGID);
        for (std::vector<int>::const_iterator sit = csvFieldOrder_.begin(); sit != csvFieldOrder_.end(); ++ sit)
        {
            wxString entry = "";
            ITransactionsFile::ItemType itemType = ITransactionsFile::TYPE_STRING;
            switch (*sit)
            {
                case UNIV_CSV_DATE:
                    trx_date = Model_Checking::TRANSDATE(pBankTransaction);
                    entry = trx_date.Format(date_format_);
                    break;
                case UNIV_CSV_PAYEE:
                    entry = tran.real_payee_name(fromAccountID);
                    break;
                case UNIV_CSV_AMOUNT:
                    entry = amount;
                    itemType = ITransactionsFile::TYPE_NUMBER;
                    break;
                case UNIV_CSV_CATEGORY:
                    entry = category ? category->CATEGNAME : "";
                    break;
                case UNIV_CSV_SUBCATEGORY:
                    entry = sub_category ? sub_category->SUBCATEGNAME : "";
                    break;
                case UNIV_CSV_TRANSNUM:
                    entry = pBankTransaction.TRANSACTIONNUMBER;
                    break;
                case UNIV_CSV_NOTES:
                    entry = wxString(pBankTransaction.NOTES).Trim();
                    entry.Replace("\n", "\\n");
                    break;
                case UNIV_CSV_DEPOSIT:
                    entry = (value > 0.0) ? amount : "";
                    itemType = ITransactionsFile::TYPE_NUMBER;
                    break;
                case UNIV_CSV_WITHDRAWAL:
                    entry = value >= 0.0 ? "" : amount_abs;
                    itemType = ITransactionsFile::TYPE_NUMBER;
                    break;
                case UNIV_CSV_BALANCE:
                    entry = Model_Currency::toString(account_balance, currency);
                    itemType = ITransactionsFile::TYPE_NUMBER;
                    break;
                case UNIV_CSV_DONTCARE:
                default:
                    break;
            }
            pTxFile->AddNewItem(entry, itemType);
        }

        ++ numRecords;
    }

    pTxFile->Save(fileName);
    const wxString& msg = wxString::Format(_("Transactions exported: %ld"), numRecords);
    mmErrorDialogs::MessageWarning(this, msg, _("Export"));

    delete pTxFile;
}

void mmUnivCSVDialog::update_preview()
{
    this->m_list_ctrl_->ClearAll();
    unsigned long colCount = 0;
    this->m_list_ctrl_->InsertColumn(colCount, _("#"));
    this->m_list_ctrl_->SetColumnWidth(colCount, 30);
    ++colCount;
    int date_position = 0;
    const int MAX_ROWS_IN_PREVIEW = 20;
    const int MAX_COLS = 30; // Not including line number col.
    const wxString NOTES_FIELD_NAME = getCSVFieldName(UNIV_CSV_NOTES);
    const wxString DATE_FIELD_NAME = getCSVFieldName(UNIV_CSV_DATE);
    for (std::vector<int>::const_iterator it = csvFieldOrder_.begin(); it != csvFieldOrder_.end(); ++ it)
    {
        wxString item_name = this->getCSVFieldName(*it);
        this->m_list_ctrl_->InsertColumn(colCount, item_name);
        if (item_name == NOTES_FIELD_NAME)
        {
            this->m_list_ctrl_->SetColumnWidth(colCount, 300);
        }
        else if (item_name == DATE_FIELD_NAME)
        {
            date_position = colCount;
        }
        ++colCount;
    }

    if (IsImporter())
    {
        const wxString fileName = m_text_ctrl_->GetValue();
        wxFileName csv_file(fileName);

        if (fileName.IsEmpty() || !csv_file.FileExists())
            return;

        // Open and parse file
        ITransactionsFile *pImporter = CreateFileHandler();
        pImporter->Load(fileName, MAX_COLS);

        // Import- Add rows to preview
        for (unsigned int row = 0; row < pImporter->GetLinesCount(); row++)
        {
            unsigned int col = 0;
            wxString buf;
            buf.Printf("%d", col);
            long itemIndex = m_list_ctrl_->InsertItem(row, buf, 0);
            buf.Printf("%d", row + 1);
            m_list_ctrl_->SetItem(itemIndex, col, buf);

            // Cols
            while (col < pImporter->GetItemsCount(row) && col + 1 <= MAX_COLS)
            {
                // Add a new column
                if (col == colCount - 1)
                {
                    m_list_ctrl_->InsertColumn(colCount, getCSVFieldName(UNIV_CSV_DONTCARE));
                    colCount++;
                }
                
                wxString content = pImporter->GetItem(row, col);
                ++col; 
                m_list_ctrl_->SetItem(itemIndex, col, content);
            }
        }
        delete pImporter;

        // Limit spin control's max value to number of lines in file.
        m_spinIgnoreFirstRows_->SetRange(m_spinIgnoreFirstRows_->GetMin(), m_list_ctrl_->GetItemCount());
        m_spinIgnoreLastRows_->SetRange(m_spinIgnoreLastRows_->GetMin(), m_list_ctrl_->GetItemCount());
        UpdateListItemBackground();
    }
    else // exporter preview
    {
        const wxString acctName = m_choice_account_->GetStringSelection();
        Model_Account::Data* from_account = Model_Account::instance().get(acctName);

        if (from_account)
        {
            const auto split = Model_Splittransaction::instance().get_all();
            int fromAccountID = from_account->ACCOUNTID;
            size_t count = 0;
            int row = 0;
            const wxString& delimit = this->delimit_;

            double account_balance = from_account->INITIALBAL;
            for (const auto& pBankTransaction : Model_Checking::instance().find_or(Model_Checking::ACCOUNTID(fromAccountID)
                , Model_Checking::TOACCOUNTID(fromAccountID)))
            {
                if (Model_Checking::status(pBankTransaction) == Model_Checking::VOID_)
                    continue;

                Model_Checking::Full_Data tran(0, pBankTransaction, split);

                double value = Model_Checking::balance(pBankTransaction, fromAccountID);
                account_balance += value;
                
                Model_Currency::Data* currency = Model_Account::currency(from_account);
                const wxString amount = Model_Currency::toString(value, currency);
                const wxString amount_abs = Model_Currency::toString(fabs(value), currency);

                int col = 0;
                wxString buf;
                buf.Printf("%d", col);
                long itemIndex = m_list_ctrl_->InsertItem(row, buf, 0);
                buf.Printf("%d", row + 1);
                m_list_ctrl_->SetItem(itemIndex, col, buf);

                Model_Category::Data* category = Model_Category::instance().get(pBankTransaction.CATEGID);
                Model_Subcategory::Data* sub_category = Model_Subcategory::instance().get(pBankTransaction.SUBCATEGID);
                for (std::vector<int>::const_iterator sit = csvFieldOrder_.begin(); sit != csvFieldOrder_.end(); ++ sit)
                {
                    ++ col;
                    wxString text;
                    switch (*sit)
                    {
                        case UNIV_CSV_DATE:
                        {
                            text << inQuotes(Model_Checking::TRANSDATE(pBankTransaction).Format(date_format_), delimit);
                            break;
                        }
                        case UNIV_CSV_PAYEE:
                            text << inQuotes(tran.real_payee_name(fromAccountID), delimit);
                            break;
                        case UNIV_CSV_AMOUNT:
                            text << inQuotes(amount, delimit);
                            break;
                        case UNIV_CSV_CATEGORY:
                            text << inQuotes(category ? category->CATEGNAME : "", delimit);
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
                            text << inQuotes(value >= 0.0 ? "" : amount_abs, delimit);
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
                            mmParseDisplayStringToDate(dtdt, text, date_format_);
                            text = dtdt.Format(date_format_);
                        }
                        m_list_ctrl_->SetItem(itemIndex, col, text);
                    }

                }
                if (++ count >= MAX_ROWS_IN_PREVIEW) break;
                ++ row;
            }
        }
    }
}

void mmUnivCSVDialog::OnMoveUp(wxCommandEvent& WXUNUSED(event))
{
    int index = csvListBox_->GetSelection();
    if (index != wxNOT_FOUND && index != 0)
    {
        mmListBoxItem* item = static_cast<mmListBoxItem*>(csvListBox_->GetClientObject(index));
        int item_index = item->getIndex();
        const wxString item_name = item->getName();

        csvListBox_->Delete(index);
        csvListBox_->Insert(wxGetTranslation(item_name), index - 1, new mmListBoxItem(item_index, item_name));

        csvListBox_->SetSelection(index - 1, true);
        std::swap(csvFieldOrder_[index - 1], csvFieldOrder_[index]);

        this->update_preview();
    }
}

void mmUnivCSVDialog::OnMoveDown(wxCommandEvent& WXUNUSED(event))
{
    int index = csvListBox_->GetSelection();
    if (index != wxNOT_FOUND && index != (int)csvListBox_->GetCount() - 1)
    {
        mmListBoxItem* item = static_cast<mmListBoxItem*>(csvListBox_->GetClientObject(index));
        int item_index = item->getIndex();
        wxString item_name = item->getName();

        csvListBox_->Delete(index);
        csvListBox_->Insert(wxGetTranslation(item_name), index + 1, new mmListBoxItem(item_index, item_name));

        csvListBox_->SetSelection(index + 1, true);
        std::swap(csvFieldOrder_[index + 1], csvFieldOrder_[index]);

        this->update_preview();
    }
}

void mmUnivCSVDialog::OnStandard(wxCommandEvent& WXUNUSED(event))
{
    csvListBox_->Clear();
    csvFieldOrder_.clear();
    int standard[] = {UNIV_CSV_DATE, UNIV_CSV_PAYEE, UNIV_CSV_AMOUNT, UNIV_CSV_CATEGORY, UNIV_CSV_SUBCATEGORY, UNIV_CSV_TRANSNUM, UNIV_CSV_NOTES};
    for (size_t i = 0; i < sizeof(standard)/sizeof(UNIV_CSV_DATE); ++ i)
    {
        csvListBox_->Append(wxGetTranslation(CSVFieldName_[standard[i]]), new mmListBoxItem(standard[i], CSVFieldName_[standard[i]]));
        csvFieldOrder_.push_back(standard[i]);
    }

    csvFieldCandicate_->Clear();
    int rest[] = { UNIV_CSV_DONTCARE, UNIV_CSV_WITHDRAWAL, UNIV_CSV_DEPOSIT, UNIV_CSV_BALANCE };
    for (size_t i = 0; i < sizeof(rest)/sizeof(UNIV_CSV_DATE); ++ i)
    {
        csvFieldCandicate_->Append(wxGetTranslation(CSVFieldName_[rest[i]]), new mmListBoxItem(rest[i], CSVFieldName_[rest[i]]));
    }

    update_preview();
}

void mmUnivCSVDialog::OnBrowse(wxCommandEvent& WXUNUSED(event))
{
    wxString fileName = m_text_ctrl_->GetValue();
    wxString header;
    switch (dialogType_)
    {
    case DIALOG_TYPE_IMPORT_CSV:
        header = _("Choose CSV data file to import");
        break;
    case DIALOG_TYPE_EXPORT_CSV:
        header = _("Choose CSV data file to export");
        break;
    case DIALOG_TYPE_IMPORT_XML:
        header = _("Choose XML data file to import");
        break;
    case DIALOG_TYPE_EXPORT_XML:
        header = _("Choose XML data file to export");
        break;
    default:
        break;
    }

    long flags = IsImporter() ? wxFD_FILE_MUST_EXIST | wxFD_OPEN : wxFD_SAVE;
    const wxString defaultWildcard = IsXML() ? wxString() << _("XML Files (*.xml)") << "|*.xml;*.XML|" << _("All Files") << "|" << wxFileSelectorDefaultWildcardStr :
        wxString() << _("CSV Files (*.csv)") << "|*.csv;*.CSV";
    const wxString chooseExt = IsXML() ? "*.xml" : "*.csv";

    if (!IsImporter()) correctEmptyFileExt("csv", fileName);

    fileName = wxFileSelector(header
        , fileName, wxEmptyString, chooseExt, defaultWildcard, flags);

    if (!fileName.IsEmpty())
    {
        m_text_ctrl_->SetValue(fileName);

        if (IsImporter()) {
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
                if (++count >= 10) break;
            }
            // TODO: update_preview() is called twice. Once here and once in OnFileNameChanged(). 
            // This leads to double work and double error messages to the user.
            *log_field_ << "\n";
            this->update_preview();
        }
    }
}

void mmUnivCSVDialog::OnListBox(wxCommandEvent& event)
{
    if (m_oject_in_focus == ID_LISTBOX_CANDICATE)
        OnAdd(event);
    else if (m_oject_in_focus == ID_LISTBOX)
        OnRemove(event);
}

void mmUnivCSVDialog::OnDelimiterChange(wxCommandEvent& event)
{
    const wxString delimit = m_textDelimiter->GetValue();
    event.Skip();

    if (!delimit.IsEmpty())
    {
        delimit_ = delimit;
        this->update_preview();
    }
}

void mmUnivCSVDialog::parseToken(int index, const wxString& orig_token, tran_holder& holder)
{
    wxString token = orig_token;
    if (token.Trim().IsEmpty()) return;
    Model_Payee::Data* payee = nullptr;
    Model_Category::Data* category = nullptr;
    Model_Subcategory::Data* sub_category = nullptr;

    wxDateTime dtdt;
    switch (index)
    {
        case UNIV_CSV_DATE:
            if (mmParseDisplayStringToDate(dtdt, token, date_format_))
                holder.Date = dtdt.GetDateOnly();
            break;

        case UNIV_CSV_PAYEE:
            payee = Model_Payee::instance().get(token);
            if (!payee)
            {
                payee = Model_Payee::instance().create();
                payee->PAYEENAME = token;
                Model_Payee::instance().save(payee);
            }

            holder.PayeeID = payee->PAYEEID;
            break;

        case UNIV_CSV_AMOUNT:
        {
            token.Replace(" ", wxEmptyString);

            if (!Model_Currency::fromString(token, holder.Amount, Model_Account::currency(Model_Account::instance().get(fromAccountID_)))) return;

            Model_Checking::TYPE txType = Model_Checking::WITHDRAWAL;
            if ((holder.Amount > 0.0 && m_choiceAmountFieldSign->GetCurrentSelection() == PositiveIsDeposit) ||
                (holder.Amount <= 0.0 && m_choiceAmountFieldSign->GetCurrentSelection() == PositiveIsWithdrawal))
            {
                txType = Model_Checking::DEPOSIT;
            }
            holder.Type = Model_Checking::all_type()[txType];

            holder.Amount = fabs(holder.Amount);
            break;
        }
        case UNIV_CSV_CATEGORY:
            category = Model_Category::instance().get(token);
            if (!category)
            {
                category = Model_Category::instance().create();
                category->CATEGNAME = token;
                Model_Category::instance().save(category);
            }

            holder.CategoryID = category->CATEGID;
            break;

        case UNIV_CSV_SUBCATEGORY:
            if (holder.CategoryID == -1)
                return;

            sub_category = (!token.IsEmpty() ? Model_Subcategory::instance().get(token, holder.CategoryID) : 0);
            if (!sub_category)
            {
                sub_category = Model_Subcategory::instance().create();
                sub_category->CATEGID = holder.CategoryID;
                sub_category->SUBCATEGNAME = token;
                Model_Subcategory::instance().save(sub_category);
            }
            holder.SubCategoryID = sub_category->SUBCATEGID; 
            break;

        case UNIV_CSV_NOTES:
            holder.Notes = token;
            holder.Notes.Replace("\\n", "\n");
            break;

        case UNIV_CSV_TRANSNUM:
            holder.Number = token;
            break;

        case UNIV_CSV_DONTCARE:
            // do nothing
            break;

        case UNIV_CSV_DEPOSIT:
            if (token.IsEmpty())
                return;
            // do nothing if an amount has already been stored by a previous call
            if (holder.Amount != 0.0)
                break;
            if (!Model_Currency::fromString(token, holder.Amount, Model_Account::currency(Model_Account::instance().get(fromAccountID_))))
                return;
            holder.Amount = fabs(holder.Amount);
            holder.Type = Model_Checking::all_type()[Model_Checking::DEPOSIT];
            break;

        case UNIV_CSV_WITHDRAWAL:
            if (token.IsEmpty())
                return;
            // do nothing if an amount has already been stored by a previous call
            if (holder.Amount != 0.0)
                break;
            if (!Model_Currency::fromString(token, holder.Amount, Model_Account::currency(Model_Account::instance().get(fromAccountID_))))
                return;
            holder.Amount = fabs(holder.Amount);
            holder.Type = Model_Checking::all_type()[Model_Checking::WITHDRAWAL];
            break;

        case UNIV_CSV_BALANCE:
            // do nothing
            break;

        default:
            wxASSERT(false);
            break;
    }
}

void mmUnivCSVDialog::OnButtonClear(wxCommandEvent& WXUNUSED(event))
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
    wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, wxID_BROWSE);
    this->GetEventHandler()->AddPendingEvent(evt);
}

void mmUnivCSVDialog::OnDateFormatChanged(wxCommandEvent& event)
{
    int sel = event.GetInt();
    if (sel == wxNOT_FOUND)
        return;
    int i = event.GetId();
    if (i == ID_DATE_FORMAT)
    {
        wxStringClientData* data = (wxStringClientData*)(choiceDateFormat_->GetClientObject(choiceDateFormat_->GetSelection()));
        if (data) date_format_ = data->GetData();
        *log_field_ << date_format_ << "\n";
    }
    else if (i == wxID_ACCOUNT)
    {
        wxString acctName = m_choice_account_->GetStringSelection();
        Model_Account::Data* account = Model_Account::instance().get(acctName);
        Model_Currency::Data* currency = Model_Account::currency(account);
        *log_field_ << _("Currency:") << " " << wxGetTranslation(currency->CURRENCYNAME) << "\n";
    }
    else if (i == ID_ENCODING)
    {
        *log_field_ << m_choiceEncoding->GetStringSelection() << "\n";
    }

    this->update_preview();
}

void mmUnivCSVDialog::changeFocus(wxChildFocusEvent& event)
{
    wxWindow *w = event.GetWindow();
    if (w)
        m_oject_in_focus = w->GetId();
}

void mmUnivCSVDialog::OnSpinCtrlIgnoreRows(wxSpinEvent& WXUNUSED(event))
{
    UpdateListItemBackground();
}

void mmUnivCSVDialog::UpdateListItemBackground()
{
    const int firstRow = m_spinIgnoreFirstRows_->GetValue();
    const int lastRow = m_list_ctrl_->GetItemCount() - m_spinIgnoreLastRows_->GetValue() - 1;
    for (int row = 0; row < m_list_ctrl_->GetItemCount(); row++)
    {
        m_list_ctrl_->SetItemBackgroundColour(row, row >= firstRow && row <= lastRow ? m_list_ctrl_->GetBackgroundColour() :*wxLIGHT_GREY);
    }
}

bool mmUnivCSVDialog::isIndexPresent(int index) const
{
    for (std::vector<int>::const_iterator it = csvFieldOrder_.begin(); it != csvFieldOrder_.end(); ++it)
    {
        if (*it == index) return true;
    }

    return false;
}

bool mmUnivCSVDialog::ShowToolTips()
{
    return TRUE;
}

ITransactionsFile *mmUnivCSVDialog::CreateFileHandler()
{
    // XML
    if (IsXML())
        return new FileXML(this, g_encoding.at(m_choiceEncoding->GetSelection()).second);
    
    // CSV
    return new FileCSV(this, g_encoding.at(m_choiceEncoding->GetSelection()).first, delimit_);
}
