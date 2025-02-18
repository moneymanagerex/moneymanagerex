/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (c) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2015, 2019, 2021 Nikolay Akimov
 Copyright (C) 2015 Yosef
 Copyright (C) 2022  Mark Whalley (mark@ipx.co.uk)

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

#include <algorithm>
#include <cctype>
#include <string>
#include <memory>
#include <regex>

#include <wx/xml/xml.h>
#include <wx/spinctrl.h>
#include <wx/display.h>

#include "univcsvdialog.h"

#include "images_list.h"
#include "constants.h"
#include "mmSimpleDialogs.h"
#include "paths.h"
#include "platfdep.h"
#include "util.h"
#include "option.h"
#include "util.h"
#include "webapp.h"
#include "parsers.h"
#include "payeedialog.h"
#include "categdialog.h"

#include "model/Model_Setting.h"
#include "model/Model_Infotable.h"
#include "model/Model_Payee.h"
#include "model/Model_Category.h"
#include "model/Model_CustomFieldData.h"
#include "model/Model_Tag.h"

enum tab_id {
    DATA_TAB = 1,
    PAYEE_TAB = 2,
    CAT_TAB = 4,
};

wxIMPLEMENT_DYNAMIC_CLASS(mmUnivCSVDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmUnivCSVDialog, wxDialog)
EVT_BUTTON(ID_UNIVCSVBUTTON_IMPORT, mmUnivCSVDialog::OnImport)
EVT_BUTTON(ID_UNIVCSVBUTTON_EXPORT, mmUnivCSVDialog::OnExport)
EVT_BUTTON(wxID_REMOVE, mmUnivCSVDialog::OnRemove)
EVT_BUTTON(wxID_ADD, mmUnivCSVDialog::OnAdd)
EVT_BUTTON(wxID_SAVEAS, mmUnivCSVDialog::OnSettingsSave)
EVT_BUTTON(wxID_UP, mmUnivCSVDialog::OnMoveUp)
EVT_BUTTON(wxID_DOWN, mmUnivCSVDialog::OnMoveDown)
EVT_BUTTON(wxID_CLEAR, mmUnivCSVDialog::OnButtonClearClick)
EVT_BUTTON(wxID_STANDARD, mmUnivCSVDialog::OnStandard)
EVT_BUTTON(wxID_BROWSE, mmUnivCSVDialog::OnFileBrowse)
EVT_LISTBOX_DCLICK(wxID_ANY, mmUnivCSVDialog::OnListBox)
EVT_CHOICE(wxID_ANY, mmUnivCSVDialog::OnChoiceChanged)
EVT_CHECKBOX(wxID_ANY, mmUnivCSVDialog::OnCheckboxClick)
EVT_MENU(wxID_HIGHEST, mmUnivCSVDialog::OnMenuSelected)
EVT_LIST_COL_END_DRAG(wxID_ANY, mmUnivCSVDialog::OnColumnResize)
wxEND_EVENT_TABLE()

//----------------------------------------------------------------------------
mmUnivCSVDialog::mmUnivCSVDialog()
{
}

mmUnivCSVDialog::mmUnivCSVDialog(
    wxWindow* parent,
    EDialogType dialogType,
    int64 account_id,
    const wxString& file_path,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style
) :
    dialogType_(dialogType),
    m_account_id(account_id),
    m_file_path(file_path),
    decimal_(Model_Currency::GetBaseCurrency()->DECIMAL_POINT),
    depositType_(Model_Checking::TYPE_STR_DEPOSIT)
{
    CSVFieldName_[UNIV_CSV_ID]          = _n("ID");
    CSVFieldName_[UNIV_CSV_DATE]        = _n("Date");
    CSVFieldName_[UNIV_CSV_STATUS]      = _n("Status");
    CSVFieldName_[UNIV_CSV_TYPE]        = _n("Type");
    CSVFieldName_[UNIV_CSV_ACCOUNT]     = _n("Account");
    CSVFieldName_[UNIV_CSV_PAYEE]       = _n("Payee");
    CSVFieldName_[UNIV_CSV_AMOUNT]      = _n("Amount");
    CSVFieldName_[UNIV_CSV_CURRENCY]    = _n("Currency");
    CSVFieldName_[UNIV_CSV_CATEGORY]    = _n("Category");
    CSVFieldName_[UNIV_CSV_SUBCATEGORY] = _n("SubCategory");
    CSVFieldName_[UNIV_CSV_TAGS]        = _n("Tags");
    CSVFieldName_[UNIV_CSV_TRANSNUM]    = _n("Number");
    CSVFieldName_[UNIV_CSV_NOTES]       = _n("Notes");
    CSVFieldName_[UNIV_CSV_DONTCARE]    = _n("Don't Care");
    CSVFieldName_[UNIV_CSV_WITHDRAWAL]  = _n("Withdrawal");
    CSVFieldName_[UNIV_CSV_DEPOSIT]     = _n("Deposit");
    CSVFieldName_[UNIV_CSV_BALANCE]     = _n("Balance");   

    wxString header;
    switch (dialogType_)
    {
    case DIALOG_TYPE_IMPORT_CSV:
        header = _("Import from CSV file");
        break;
    case DIALOG_TYPE_EXPORT_CSV:
        header = _("Export as CSV file");
        break;
    case DIALOG_TYPE_IMPORT_XML:
        header = _("Import from XML file");
        break;
    case DIALOG_TYPE_EXPORT_XML:
        header = _("Export as XML file");
        break;
    default:
        break;
    }

    Create(parent, header, id, pos, size, style);
    this->Connect(wxID_ANY, wxEVT_CHILD_FOCUS, wxChildFocusEventHandler(mmUnivCSVDialog::changeFocus), nullptr, this);
}

bool mmUnivCSVDialog::Create(wxWindow* parent
    , const wxString& caption
    , wxWindowID id
    , const wxPoint& pos
    , const wxSize& size
    , long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    wxSize sz = GetSize();
    wxSharedPtr<wxDisplay> display(new wxDisplay(GetParent()));
    wxSize screenSize = display.get()->GetGeometry().GetSize();
    SetSize(wxSize(std::min(sz.GetX(), screenSize.GetX()), std::min(sz.GetY(), screenSize.GetY())));
    SetMinSize(GetSize());
    SetIcon(mmex::getProgramIcon());
    Centre();
   return true;
}

void mmUnivCSVDialog::CreateControls()
{
    wxSizerFlags flagsExpand;
    flagsExpand.Align(wxALIGN_LEFT | wxALIGN_TOP).Border(wxLEFT | wxRIGHT | wxTOP, 5);

    // Define the staticBox font and set it as wxFONTWEIGHT_BOLD
    wxFont staticBoxFontSetting = this->GetFont();

    wxScrolledWindow* scrolledWindow = new wxScrolledWindow(this, wxID_ANY);
    wxBoxSizer* itemBoxSizer0 = new wxBoxSizer(wxVERTICAL);
    scrolledWindow->SetSizer(itemBoxSizer0);
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer0->Add(itemBoxSizer1, 0, wxGROW | wxALL, 0);
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer1->Add(itemBoxSizer2, 8, wxGROW | wxALL, 5);
    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer1->Add(itemBoxSizer11, 5, wxGROW | wxALL, 5);

    //File to import, file path and browse button
    wxPanel* itemPanel6 = new wxPanel(scrolledWindow
        , wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer2->Add(itemPanel6, 0, wxEXPAND | wxALL, 1);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel6->SetSizer(itemBoxSizer7);

    wxStaticText* itemStaticText5 = new wxStaticText(itemPanel6, wxID_ANY, _("File Name:"));
    itemBoxSizer7->Add(itemStaticText5, g_flagsH);
    itemStaticText5->SetFont(staticBoxFontSetting);

    m_text_ctrl_ = new wxTextCtrl(itemPanel6
        , ID_FILE_NAME, m_file_path, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_text_ctrl_->SetMinSize(wxSize(300, -1));
    itemBoxSizer7->Add(m_text_ctrl_, 1, wxALL | wxGROW, 5);
    m_text_ctrl_->Connect(ID_FILE_NAME
        , wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(mmUnivCSVDialog::OnFileNameChanged), nullptr, this);
    m_text_ctrl_->Connect(ID_FILE_NAME
        , wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(mmUnivCSVDialog::OnFileNameEntered), nullptr, this);

    const wxString file_button_label = _tu("&Browse…");
    wxButton* button_browse = new wxButton(itemPanel6, wxID_BROWSE, file_button_label);
    itemBoxSizer7->Add(button_browse, g_flagsH);

    // Account to import/export
    wxFlexGridSizer* preset_flex_sizer = new wxFlexGridSizer(0,3,0,0);

    wxStaticText* itemStaticText6 = new wxStaticText(scrolledWindow, wxID_ANY, _("Account: "), wxDefaultPosition, itemStaticText5->GetSize());
    preset_flex_sizer->Add(itemStaticText6, g_flagsH);
    itemStaticText6->SetFont(staticBoxFontSetting);

    m_choice_account_ = new wxChoice(scrolledWindow, wxID_ACCOUNT, wxDefaultPosition, wxDefaultSize, Model_Account::instance().all_checking_account_names(), 0);
    m_choice_account_->SetMinSize(wxSize(210, -1));
    preset_flex_sizer->Add(m_choice_account_, g_flagsExpand);
    preset_flex_sizer->AddSpacer(0);

    // Predefined settings
    wxStaticText* preset_label = new wxStaticText(scrolledWindow, wxID_ANY, _("Preset:"), wxDefaultPosition, itemStaticText5->GetSize());
    preset_flex_sizer->Add(preset_label, g_flagsH);

    Document account_default_presets;
    if (!account_default_presets.Parse(Model_Infotable::instance().getString((IsCSV() ? "CSV_ACCOUNT_PRESETS" : "XML_ACCOUNT_PRESETS"), "{}").utf8_str()).HasParseError())
    {
        for (const auto& member : account_default_presets.GetObject()) {
            m_acct_default_preset[std::stoll(member.name.GetString())] = member.value.GetString();
        }
    }

    wxArrayString preset_choices;
    wxString prefix = GetSettingsPrfix();
    prefix.Replace("%d", "");
    wxString init_preset_name;
    for (const auto& setting : Model_Setting::instance().find(Model_Setting::SETTINGNAME(prefix + "0", GREATER_OR_EQUAL), Model_Setting::SETTINGNAME(prefix + "A", LESS)))
    {
        Document json_doc;
        if (json_doc.Parse(setting.SETTINGVALUE.utf8_str()).HasParseError()) {
            continue;
        }

        Value& template_name = GetValueByPointerWithDefault(json_doc, "/SETTING_NAME", "");
        const wxString setting_name = template_name.IsString() ? wxString::FromUTF8(template_name.GetString()) : "??";
        preset_choices.Add(setting_name);
        m_preset_id[setting_name] = setting.SETTINGNAME;
        if (!m_acct_default_preset[m_account_id].IsEmpty() && m_acct_default_preset[m_account_id] == setting.SETTINGNAME) init_preset_name = setting_name;
    }

    m_choice_preset_name = new wxChoice(scrolledWindow, wxID_APPLY, wxDefaultPosition, wxDefaultSize, preset_choices, wxCB_SORT);
    m_choice_preset_name->SetMinSize(wxSize(210, -1));
    m_choice_preset_name->Connect(wxID_APPLY, wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(mmUnivCSVDialog::OnSettingsSelected), nullptr, this);

    wxBoxSizer* preset_box_sizer = new wxBoxSizer(wxHORIZONTAL);
    preset_box_sizer->Add(m_choice_preset_name, g_flagsH);
    

    if (!init_preset_name.IsEmpty())
        m_choice_preset_name->SetStringSelection(init_preset_name);

    wxBitmapButton* itemButton_Save = new wxBitmapButton(scrolledWindow, wxID_SAVEAS, mmBitmapBundle(png::SAVE, mmBitmapButtonSize));
    preset_box_sizer->Add(itemButton_Save, g_flagsH);

    wxBitmapButton* itemButtonClear = new wxBitmapButton(scrolledWindow, wxID_CLEAR, mmBitmapBundle(png::CLEAR, mmBitmapButtonSize));
    preset_box_sizer->Add(itemButtonClear, g_flagsH);

    preset_flex_sizer->Add(preset_box_sizer, wxSizerFlags(g_flagsExpand).Border(0).Proportion(0));
    m_checkbox_preset_default = new wxCheckBox(scrolledWindow, wxID_DEFAULT, wxString::Format(_("Load this Preset when Account is:\n%s"), wxEmptyString));
    m_checkbox_preset_default->Enable(m_account_id > -1 && !init_preset_name.IsEmpty());
    preset_flex_sizer->Add(m_checkbox_preset_default, g_flagsH);
    itemBoxSizer2->Add(preset_flex_sizer, wxSizerFlags(g_flagsExpand).Border(wxALL, 0).Proportion(0));

    //
    wxStaticText* itemStaticText3 = new wxStaticText(scrolledWindow, wxID_STATIC
        , _("Specify the order of fields in the file"));
    itemBoxSizer2->Add(itemStaticText3, g_flagsV);
    itemStaticText3->SetFont(staticBoxFontSetting);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW | wxALL, 5);

    //CSV fields candicate
    csvFieldCandicate_ = new wxListBox(scrolledWindow, ID_LISTBOX_CANDICATE
        , wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE | wxLB_NEEDED_SB);
    itemBoxSizer3->Add(csvFieldCandicate_, 1, wxGROW | wxALL, 1);
    for (const auto& it : CSVFieldName_)
        csvFieldCandicate_->Append(wxGetTranslation(it.second), new mmListBoxItem(it.first, it.second));

    //Custom Fields
    Model_CustomField::Data_Set fields = Model_CustomField::instance().find(Model_CustomField::REFTYPE(Model_Attachment::REFTYPE_STR_TRANSACTION));
    if (!fields.empty())
    {
        std::sort(fields.begin(), fields.end(), SorterByDESCRIPTION());
        for (const Model_CustomField::Data& entry : fields)
        {
            // Can't use an enum for the field index since there can be infinite custom fields
            // Instead we offset the last enum by the custom FIELDID to get a unique index for each
            int csvField = UNIV_CSV_LAST + entry.FIELDID.GetValue();
            CSVFieldName_[csvField] = entry.DESCRIPTION;
            csvFieldCandicate_->Append(entry.DESCRIPTION, new mmListBoxItem(csvField, entry.DESCRIPTION));
        }
    }

    //Add Remove Area
    wxPanel* itemPanel_AddRemove = new wxPanel(scrolledWindow, ID_PANEL10,
        wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer3->Add(itemPanel_AddRemove, g_flagsH);

    wxBoxSizer* itemBoxSizer_AddRemove = new wxBoxSizer(wxVERTICAL);
    itemPanel_AddRemove->SetSizer(itemBoxSizer_AddRemove);

    //Add button
    m_button_add_ = new wxButton(itemPanel_AddRemove, wxID_ADD, _("&Add "));
    itemBoxSizer_AddRemove->Add(m_button_add_, g_flagsV);

    //Remove button
    m_button_remove_ = new wxButton(itemPanel_AddRemove, wxID_REMOVE, _("&Remove "));
    itemBoxSizer_AddRemove->Add(m_button_remove_, g_flagsV);

    //Standard MMEX CSV
    wxButton* itemButton_standard = new wxButton(itemPanel_AddRemove, wxID_STANDARD, _("&MMEX format"));
    itemBoxSizer_AddRemove->Add(itemButton_standard, g_flagsV);

    //ListBox of attribute order
    csvListBox_ = new wxListBox(scrolledWindow, ID_LISTBOX
        , wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE | wxLB_NEEDED_SB);
    itemBoxSizer3->Add(csvListBox_, 1, wxGROW | wxALL, 1);

    //Arranger Area
    wxPanel* itemPanel_Arranger = new wxPanel(scrolledWindow, ID_PANEL10
        , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer3->Add(itemPanel_Arranger, 0, wxALL, 1);

    wxBoxSizer* itemBoxSizer_Arranger = new wxBoxSizer(wxVERTICAL);
    itemPanel_Arranger->SetSizer(itemBoxSizer_Arranger);

    //Move Up button
    wxButton* itemButton_MoveUp = new wxButton(itemPanel_Arranger, wxID_UP, _("&Up"));
    itemBoxSizer_Arranger->Add(itemButton_MoveUp, g_flagsV);

    //Move down button
    wxButton* itemButton_MoveDown = new wxButton(itemPanel_Arranger, wxID_DOWN, _("&Down"));
    itemBoxSizer_Arranger->Add(itemButton_MoveDown, g_flagsV);

    wxStaticLine* m_staticline1 = new wxStaticLine(scrolledWindow
        , wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    itemBoxSizer2->Add(m_staticline1, flagsExpand);

    // Date Format
    wxPanel* itemPanel7 = new wxPanel(scrolledWindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer2->Add(itemPanel7, 0, wxEXPAND | wxALL, 1);

    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 4, 0, 0);
    itemPanel7->SetSizer(flex_sizer);

    wxStaticLine* m_staticline2 = new wxStaticLine(scrolledWindow
        , wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    itemBoxSizer2->Add(m_staticline2, flagsExpand);

    wxStaticText* itemStaticText66 = new wxStaticText(itemPanel7, wxID_STATIC, wxString(_("Date Format: ")));
    flex_sizer->Add(itemStaticText66, g_flagsH);
    itemStaticText66->SetFont(staticBoxFontSetting);

    date_format_ = Option::instance().getDateFormat();
    choiceDateFormat_ = new wxChoice(itemPanel7, ID_DATE_FORMAT);
    flex_sizer->Add(choiceDateFormat_, g_flagsH);
    initDateMask();

    flex_sizer->AddSpacer(0);
    flex_sizer->AddSpacer(0);

    // CSV Delimiter
    if (IsCSV())
    {
        wxStaticText* itemStaticText77 = new wxStaticText(itemPanel7, wxID_STATIC, wxString(_("CSV Delimiter:")));
        itemStaticText77->SetFont(staticBoxFontSetting);
        flex_sizer->Add(itemStaticText77, g_flagsH);

        m_textDelimiter = new wxTextCtrl(itemPanel7, ID_UD_DELIMIT);
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
    for (const auto &i : g_encoding)
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
        wxStaticText* itemStaticTextAmount = new wxStaticText(itemPanel7, wxID_ANY, _("Amount:"));
        flex_sizer->Add(itemStaticTextAmount, g_flagsH);
        itemStaticTextAmount->SetFont(staticBoxFontSetting);

        // Choice selection.
        m_choiceAmountFieldSign = new wxChoice(itemPanel7, wxID_REPLACE);
        m_choiceAmountFieldSign->Append(_("Positive values are deposits"));
        m_choiceAmountFieldSign->Append(_("Positive values are withdrawals"));
        m_choiceAmountFieldSign->SetSelection(PositiveIsDeposit);
        flex_sizer->Add(m_choiceAmountFieldSign, g_flagsH);

        wxStaticText* itemStaticTextDigits = new wxStaticText(itemPanel7, wxID_ANY, _("Decimal Char"));
        flex_sizer->Add(itemStaticTextDigits, g_flagsH);
        m_choiceDecimalSeparator = new mmChoiceAmountMask(itemPanel7, ID_UD_DECIMAL);
        flex_sizer->Add(m_choiceDecimalSeparator, g_flagsH);
        m_choiceDecimalSeparator->Connect(ID_UD_DECIMAL, wxEVT_COMMAND_CHOICE_SELECTED
            , wxCommandEventHandler(mmUnivCSVDialog::OnDecimalChange), nullptr, this);


        wxBoxSizer* itemBoxSizer111 = new wxBoxSizer(wxHORIZONTAL);
        itemBoxSizer2->Add(itemBoxSizer111);

        // Select rows to import (not relevant for export)
        // Container.
        wxStaticBox* rowSelectionStaticBox = new wxStaticBox(scrolledWindow, wxID_ANY, _("Rows to ignore"));
        rowSelectionStaticBox->SetFont(staticBoxFontSetting);
        wxStaticBoxSizer* rowSelectionStaticBoxSizer = new wxStaticBoxSizer(rowSelectionStaticBox, wxHORIZONTAL);
        itemBoxSizer111->Add(rowSelectionStaticBoxSizer, 0, wxALL | wxEXPAND, 5);

        // "Ignore first" title, spin and event handler.
        wxStaticText* itemStaticText7 = new wxStaticText(rowSelectionStaticBoxSizer->GetStaticBox()
            , wxID_ANY, _("From start: "));
        rowSelectionStaticBoxSizer->Add(itemStaticText7, g_flagsH);
        m_spinIgnoreFirstRows_ = new wxSpinCtrl(rowSelectionStaticBoxSizer->GetStaticBox(), ID_FIRST_ROW
            , wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS);
        m_spinIgnoreFirstRows_->SetMinSize(wxSize(50, -1));
        rowSelectionStaticBoxSizer->Add(m_spinIgnoreFirstRows_, g_flagsH);
        m_spinIgnoreFirstRows_->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED
            , wxSpinEventHandler(mmUnivCSVDialog::OnSpinCtrlIgnoreRows), nullptr, this);

        rowSelectionStaticBoxSizer->AddSpacer(30);

        // Colour
        colorCheckBox_ = new wxCheckBox(scrolledWindow, mmID_COLOR, _("Color")
            , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
        itemBoxSizer111->Add(colorCheckBox_, wxSizerFlags(g_flagsH).Border(wxLEFT | wxTOP | wxBOTTOM, 5));
        colorButton_ = new mmColorButton(scrolledWindow, wxID_HIGHEST, wxSize(itemButton_Save->GetSize().GetY(), itemButton_Save->GetSize().GetY()));
        itemBoxSizer111->Add(colorButton_, wxSizerFlags(g_flagsH).Border(wxRIGHT | wxTOP | wxBOTTOM, 5));
        colorButton_->Enable(false);

        // Payee Match
        wxBoxSizer* payeeMatchSizer = new wxBoxSizer(wxVERTICAL);
        payeeMatchCheckBox_ = new wxCheckBox(scrolledWindow, mmID_PAYEE, _("Pattern match Payees")
            , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
        payeeMatchSizer->Add(payeeMatchCheckBox_, g_flagsV);
        payeeMatchCheckBox_->Disable();
        payeeRegExInitialized_ = false;

        payeeMatchAddNotes_ = new wxCheckBox(scrolledWindow, wxID_ANY, _("Add match details to Notes")
            , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
        payeeMatchSizer->Add(payeeMatchAddNotes_, g_flagsV);
        payeeMatchAddNotes_->Disable();
        itemBoxSizer111->Add(payeeMatchSizer, wxSizerFlags(g_flagsH).Border(wxLEFT, 10));

        // "Ignore last" title, spin and event handler.
        wxStaticText* itemStaticText8 = new wxStaticText(rowSelectionStaticBoxSizer->GetStaticBox()
            , wxID_ANY, _("From end: "));
        rowSelectionStaticBoxSizer->Add(itemStaticText8, g_flagsH);
        m_spinIgnoreLastRows_ = new wxSpinCtrl(rowSelectionStaticBoxSizer->GetStaticBox()
            , ID_LAST_ROW, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0, 0);
        m_spinIgnoreLastRows_->SetMinSize(wxSize(50, -1));
        rowSelectionStaticBoxSizer->Add(m_spinIgnoreLastRows_, g_flagsH);
        m_spinIgnoreLastRows_->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED
            , wxSpinEventHandler(mmUnivCSVDialog::OnSpinCtrlIgnoreRows), nullptr, this);
    }

    // Preview
    wxStaticBoxSizer* m_staticbox = new wxStaticBoxSizer(new wxStaticBox(scrolledWindow
        , wxID_STATIC, _("Preview")), wxVERTICAL);

    m_preview_notebook = new wxNotebook(scrolledWindow
        , wxID_PREVIEW, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);
    m_staticbox->Add(m_preview_notebook, g_flagsExpand);

    wxPanel* data_tab = new wxPanel(m_preview_notebook, wxID_ANY);
    m_preview_notebook->AddPage(data_tab, _("Data"));
    wxBoxSizer* data_sizer = new wxBoxSizer(wxVERTICAL);
    data_tab->SetSizer(data_sizer);

    m_list_ctrl_ = new wxListCtrl(data_tab, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
    data_sizer->Add(m_list_ctrl_, g_flagsExpand);
    itemBoxSizer0->Add(m_staticbox, 2, wxALL | wxEXPAND, 5);

    if (IsImporter())
    {
        //Payees
        wxPanel* payee_tab = new wxPanel(m_preview_notebook, wxID_ANY);
        m_preview_notebook->AddPage(payee_tab, _("Payee"));
        wxBoxSizer* payee_sizer = new wxBoxSizer(wxHORIZONTAL);
        payee_tab->SetSizer(payee_sizer);

        payeeListBox_ = new wxDataViewListCtrl(payee_tab, wxID_FILE1);
        payeeListBox_->AppendTextColumn(_("Name"), wxDATAVIEW_CELL_INERT, 250, wxALIGN_LEFT);
        payeeListBox_->AppendTextColumn(_("Status"), wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT);
        payee_sizer->Add(payeeListBox_, g_flagsExpand);

        //Category
        wxPanel* categ_tab = new wxPanel(m_preview_notebook, wxID_ANY);
        m_preview_notebook->AddPage(categ_tab, _("Category"));
        wxBoxSizer* category_sizer = new wxBoxSizer(wxHORIZONTAL);
        categ_tab->SetSizer(category_sizer);
        categoryListBox_ = new wxDataViewListCtrl(categ_tab, wxID_FILE2);
        categoryListBox_->AppendTextColumn(_("Name"), wxDATAVIEW_CELL_INERT, 250, wxALIGN_LEFT);
        categoryListBox_->AppendTextColumn(_("Status"), wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT);
        category_sizer->Add(categoryListBox_, g_flagsExpand);

        payeeListBox_->GetMainWindow()->Bind(wxEVT_LEFT_DCLICK, &mmUnivCSVDialog::OnShowPayeeDialog, this);
        categoryListBox_->GetMainWindow()->Bind(wxEVT_LEFT_DCLICK, &mmUnivCSVDialog::OnShowCategDialog, this);
    }
    //Import File button
    wxPanel* itemPanel5 = new wxPanel(scrolledWindow, ID_PANEL10
        , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer0->Add(itemPanel5, 0, wxALIGN_CENTER | wxALL, 1);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel5->SetSizer(itemBoxSizer6);

    if (IsImporter()) {
        bImport_ = new wxButton(itemPanel5, ID_UNIVCSVBUTTON_IMPORT, _("&Import"));
    } else {
        bImport_ = new wxButton(itemPanel5, ID_UNIVCSVBUTTON_EXPORT, _("&Export"));
    }
    itemBoxSizer6->Add(bImport_, 0, wxALIGN_CENTER | wxALL, 5);

    wxButton* itemCloseButton = new wxButton(itemPanel5, wxID_CANCEL, wxGetTranslation(g_CloseLabel));
    itemBoxSizer6->Add(itemCloseButton, 0, wxALIGN_CENTER | wxALL, 5);
    itemCloseButton->SetFocus();

    //Log viewer
    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxVERTICAL);

    itemBoxSizer11->Add(itemBoxSizer22, 1, wxGROW | wxALL, 0);

    log_field_ = new wxTextCtrl(scrolledWindow
        , wxID_STATIC, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxHSCROLL | wxTE_READONLY);
    itemBoxSizer22->Add(log_field_, 1, wxGROW | wxALL, 5);

    wxButton* itemClearButton = new wxButton(scrolledWindow, wxID_CLEAR, _("Clear"));
    itemBoxSizer22->Add(itemClearButton, 0, wxALIGN_CENTER | wxALL, 5);
    itemClearButton->Connect(wxID_CLEAR, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmUnivCSVDialog::OnButtonClear), nullptr, this);

    const wxString file_tooltip = IsImporter()
        ? (IsXML() ? _("Choose XML data file to Import") : _("Choose CSV data file to Import"))
        : (IsXML() ? _("Choose XML data file to Export") : _("Choose CSV data file to Export"));
    mmToolTip(button_browse, file_tooltip);

    mmToolTip(m_choice_preset_name, _("Preset name"));
    mmToolTip(itemButton_Save, _("Save current settings as a Preset"));
    mmToolTip(itemButtonClear, _("Delete Preset"));
    mmToolTip(itemButton_standard, _("MMEX standard format"));
    mmToolTip(itemButton_MoveUp, _("Move Up"));
    mmToolTip(itemButton_MoveDown, _("Move Down"));
    if (IsCSV()) mmToolTip(m_textDelimiter, _("Specify the delimiter to use when importing/exporting CSV files"));
    if (IsImporter()) mmToolTip(bImport_, _("Import File"));
    if (!IsImporter()) mmToolTip(bImport_, _("Export File"));

    m_text_ctrl_->SetFocus();

    SetSettings(GetStoredSettings(m_choice_preset_name->GetSelection()));

    if (m_choice_account_->GetSelection() >= 0)
    {
        wxString acct_name = m_choice_account_->GetStringSelection();
        m_checkbox_preset_default->SetLabelText(wxString::Format(_("Load this Preset when Account is:\n%s"), acct_name));
        *log_field_ << _("Currency:") << " " <<
            wxGetTranslation(Model_Account::currency(Model_Account::instance().get(acct_name))->CURRENCYNAME) << "\n";
        if (!init_preset_name.IsEmpty())
            *log_field_ << wxString::Format(_("Preset '%1$s' loaded because Account '%2$s' selected"), init_preset_name, acct_name) << "\n";
    }

    scrolledWindow->SetScrollRate(10, 10);
    wxBoxSizer* dialogSizer = new wxBoxSizer(wxVERTICAL);
    dialogSizer->Add(scrolledWindow, 1, wxEXPAND);
    this->SetSizer(dialogSizer);
    itemBoxSizer0->Fit(this);
}

void mmUnivCSVDialog::initDateMask()
{
    for (const auto& i : g_date_formats_map())
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
        delimit_ = Model_Infotable::instance().getString("DELIMITER", mmex::DEFDELIMTER);
    }
    m_textDelimiter->ChangeValue(delimit_ == "\t" ? "\\t" : delimit_);

}

void mmUnivCSVDialog::OnShowPayeeDialog(wxMouseEvent&)
{
    wxString payeeName;
    if (payeeListBox_->GetSelectedRow() >= 0) {
        wxVariant value;
        payeeListBox_->GetValue(value, payeeListBox_->GetSelectedRow(), 0);
        payeeName = (std::get<0>(m_CSVpayeeNames[value.GetString()]) != -1) ?
            std::get<1>(m_CSVpayeeNames[value.GetString()]) :
            value.GetString();
    }
    mmPayeeDialog dlg(this, false, "mmPayeeDialog", payeeName);
    dlg.ShowModal();
    if (dlg.getRefreshRequested())
    {
        payeeRegExInitialized_ = false;
        refreshTabs(PAYEE_TAB);
    } 
}

void mmUnivCSVDialog::OnShowCategDialog(wxMouseEvent&)
{
    int64 id = -1;
    if (categoryListBox_->GetSelectedRow() >= 0)
    {
        wxVariant value;
        categoryListBox_->GetValue(value, categoryListBox_->GetSelectedRow(), 0);
        wxString selectedCategname = value.GetString();
        id = m_CSVcategoryNames[selectedCategname];
        if (id == -1) {
            std::map<wxString, int64 > categories = Model_Category::all_categories();
            for (const auto& category : categories)
            {
                if (category.first.CmpNoCase(selectedCategname) <= 0) id = category.second;
                else break;
            }
        }
    }
    mmCategDialog dlg(this, false, id);
    dlg.ShowModal();
    if (dlg.getRefreshRequested())
    {
        refreshTabs(CAT_TAB);
    }   
}

void mmUnivCSVDialog::OnColumnResize(wxListEvent& event)
{
    const unsigned int col = event.GetColumn();
    if (col == 0 || col > csvFieldOrder_.size()) return;
    csvFieldOrder_.at(col - 1).second = m_list_ctrl_->GetColumnWidth(col);
}

void mmUnivCSVDialog::OnSettingsSelected(wxCommandEvent&)
{
    SetSettings(GetStoredSettings(m_choice_preset_name->GetSelection()));
}

const wxString mmUnivCSVDialog::GetStoredSettings(int id) const
{
    if (id < 0) return wxEmptyString;
    const wxString& setting_id = m_preset_id.at(m_choice_preset_name->GetString(id));
    const wxString& settings_string = Model_Setting::instance().getString(setting_id, "");
    wxLogDebug("%s \n %s", setting_id, settings_string);
    return settings_string;
}

void mmUnivCSVDialog::SetSettings(const wxString &json_data)
{
    if (json_data.empty()) {
        m_choice_preset_name->SetSelection(-1);
        if (m_account_id > 0)
        {
            const Model_Account::Data* account = Model_Account::instance().get(m_account_id);
            if (account)
                m_choice_account_->SetStringSelection(account->ACCOUNTNAME);
        }
        if (m_file_path != wxEmptyString)
            update_preview();
        return;
    }

    Document json_doc;
    if (json_doc.Parse(json_data.utf8_str()).HasParseError()) {
        json_doc.Parse("{}");
    }

    //Setting name
    Value& template_name = GetValueByPointerWithDefault(json_doc, "/SETTING_NAME", "");
    const wxString setting_name = template_name.IsString() ? wxString::FromUTF8(template_name.GetString()) : "??";
    m_choice_preset_name->SetStringSelection(setting_name);

    //Date Mask
    Value& date_mask = GetValueByPointerWithDefault(json_doc, "/DATE_MASK", "");
    const wxString df = wxString::FromUTF8(date_mask.IsString() ? date_mask.GetString() : "");
    if (!df.empty())
    {
        const auto m = g_date_formats_map();

        auto it = std::find_if(m.begin(), m.end(),
            [&df](const std::pair<wxString, wxString>& element) { return element.first == df; });

        if (it != m.end()) {
            const wxString mask = it->second;
            choiceDateFormat_->SetStringSelection(mask);
            date_format_ = df;
            m_userDefinedDateMask = true;
        }
        else {
            wxLogDebug("Unrecognized DATE_MASK %s", df);
        }
    }
    else
        m_userDefinedDateMask = false;

    // Account
    wxString an;
    if (m_account_id > -1)
    {
        const Model_Account::Data* account = Model_Account::instance().get(m_account_id);
        if (account)
            an = account->ACCOUNTNAME;
        else
            m_account_id = -1;
    }

    if (!an.empty())
    {
        int itemIndex = m_choice_account_->FindString(an);
        if (wxNOT_FOUND == itemIndex)
            mmErrorDialogs::MessageError(m_choice_account_
                , wxString::Format(_("Default account '%s' for this template does not exist.\n"
                    "Please select a new account."), an)
                , _("Account does not exist"));
        else
        {
            m_choice_account_->Select(itemIndex);
            m_checkbox_preset_default->SetValue(m_preset_id[setting_name] == m_acct_default_preset[m_account_id]);
            m_checkbox_preset_default->Enable();
        }
    }
    else {
        m_choice_account_->Select(-1);
        m_checkbox_preset_default->Disable();
    }

    //Delimiter
    if (IsCSV())
    {
        Value& v_delimiter = GetValueByPointerWithDefault(json_doc, "/DELIMITER", "");
        const wxString& de = wxString::FromUTF8(v_delimiter.IsString() ? v_delimiter.GetString() : "");
        const wxString& def_delimiter = Model_Infotable::instance().getString("DELIMITER", mmex::DEFDELIMTER);
        delimit_ = (de.empty() ? def_delimiter : de);
        initDelimiter();
    }

    //Decimal Char
    if (IsImporter())
    {
        Value& v_decimal = GetValueByPointerWithDefault(json_doc, "/DECIMAL", "");
        const wxString d = wxString::FromUTF8(v_decimal.IsString() ? v_decimal.GetString() : "");
        if (!d.empty()) {
            decimal_ = d;
            m_choiceDecimalSeparator->SetDecimalChar(decimal_);
        }
    }

    //Encoding
    Value& v_encoding = GetValueByPointerWithDefault(json_doc, "/ENCODING", "");
    const wxString e = wxString::FromUTF8(v_encoding.IsString() ? v_encoding.GetString() : "");
    m_choiceEncoding->SetStringSelection(wxGetTranslation(e));

    //CSV fields
    csvFieldOrder_.clear();
    if (json_doc.HasMember("FIELDS") && json_doc["FIELDS"].IsArray())
    {
        Value a = json_doc["FIELDS"].GetArray();
        if (a.IsArray())
        {
            for (auto& v : a.GetArray())
            {
                const auto value = wxString::FromUTF8(v.IsString() ? v.GetString() : "");

                for (const auto& entry : CSVFieldName_)
                {
                    if (entry.second == value || wxGetTranslation(entry.second) == value)
                    {
                        int key = entry.first;
                        csvFieldOrder_.push_back(std::make_pair(key,-1));
                        break;
                    }
                    else if (value.StartsWith("UDF: "))
                    {
                        // remove the "UDF: "
                        int fieldId = wxAtoi(value.Mid(5));
                        // need to check for validity since the custom field may have been deleted
                        if(CSVFieldName_.find(fieldId + UNIV_CSV_LAST) != CSVFieldName_.end())
                            csvFieldOrder_.push_back(std::make_pair(UNIV_CSV_LAST + fieldId, -1));
                        break;
                    }
                }
            }
        }
    }

    if (json_doc.HasMember("FIELD_WIDTHS") && json_doc["FIELD_WIDTHS"].IsArray())
    {
        Value a = json_doc["FIELD_WIDTHS"].GetArray();
        if (a.IsArray())
        {
            unsigned int col = 0;
            for (auto& v : a.GetArray())
            {
                const auto value = v.GetInt();
                // in case a custom field was deleted there will be too many widths, so check count
                if (col < csvFieldOrder_.size())
                    csvFieldOrder_.at(col++).second = value;
            }
        }
    }

    if (IsImporter())
    {
        // Amount sign.
        if (json_doc.HasMember("AMOUNT_SIGN") && json_doc["AMOUNT_SIGN"].IsInt()) {
            int val = json_doc["AMOUNT_SIGN"].GetInt();
            m_choiceAmountFieldSign->Select(val);
            if (val == DefindByType) {
                if (json_doc.HasMember("TYPE_DEPOSIT") && json_doc["TYPE_DEPOSIT"].IsString()) {
                    depositType_ = wxString::FromUTF8(json_doc["TYPE_DEPOSIT"].GetString());
                }
            }
        }
        else {
            if (m_choiceAmountFieldSign->GetCount() > DefindByType) {
                m_choiceAmountFieldSign->Delete(DefindByType);
                m_choiceAmountFieldSign->Select(PositiveIsDeposit);
            }
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

        if (json_doc.HasMember("PAYEE_PATTERN_MATCH"))
            payeeMatchCheckBox_->SetValue(json_doc["PAYEE_PATTERN_MATCH"].GetBool());
        else
            payeeMatchCheckBox_->SetValue(false);

        if (json_doc.HasMember("PAYEE_PATTERN_MATCH_ADD_NOTES"))
            payeeMatchAddNotes_->SetValue(json_doc["PAYEE_PATTERN_MATCH_ADD_NOTES"].GetBool());
        else
            payeeMatchAddNotes_->SetValue(false);

        if (json_doc.HasMember("APPLY_COLOR"))
        {
            colorCheckBox_->SetValue(json_doc["APPLY_COLOR"].GetBool());
            colorButton_->Enable(colorCheckBox_->IsChecked());
        }
        else
        {
            colorCheckBox_->SetValue(false);
            colorButton_->Disable();
        }

        if (json_doc.HasMember("COLOR_SELECTION"))
            colorButton_->SetBackgroundColor(json_doc["COLOR_SELECTION"].GetInt());
        else
            colorButton_->SetBackgroundColor(-1);
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
    Fit();
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
        csvFieldOrder_.insert(itPos, std::make_pair(item->getIndex().GetValue(), -1));

        if (item->getIndex() != UNIV_CSV_DONTCARE
            && (item->getIndex() != UNIV_CSV_NOTES || !IsImporter()))
        {
            csvFieldCandicate_->Delete(index);
            if (static_cast<size_t>(index) < csvFieldCandicate_->GetCount()) {
                csvFieldCandicate_->SetSelection(index, true);
            }
            else {
                csvFieldCandicate_->SetSelection(csvFieldCandicate_->GetCount() - 1, true);
            }
        }

        if (IsImporter())
        {
            if (i->getIndex() == UNIV_CSV_TYPE) {
                m_choiceAmountFieldSign->Append(wxString::Format(_("Positive if type has '%s'"), depositType_));
                m_choiceAmountFieldSign->Select(DefindByType);
                wxCommandEvent evt(wxEVT_CHOICE, wxID_ANY);
                evt.SetId(wxID_REPLACE);
                GetEventHandler()->AddPendingEvent(evt);
            }
            else if (i->getIndex() == UNIV_CSV_PAYEE)
            {
                payeeMatchCheckBox_->Enable();
                payeeMatchAddNotes_->Enable();
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
        int item_index = item->getIndex().GetValue();
        wxString item_name = item->getName();

        if (item_index != UNIV_CSV_DONTCARE
            && (item->getIndex() != UNIV_CSV_NOTES || !IsImporter()))
        {
            unsigned int pos;
            for (pos = 0; pos < csvFieldCandicate_->GetCount() - 1; pos++)
            {
                mmListBoxItem *item2 = static_cast<mmListBoxItem*>(csvFieldCandicate_->GetClientObject(pos));
                if (item_index < item2->getIndex()) {
                    break;
                }
            }
            csvFieldCandicate_->Insert(wxGetTranslation(item_name), pos, new mmListBoxItem(item_index, item_name));
        }

        if (IsImporter())
        {
            if (item_index == UNIV_CSV_TYPE)
            {
                m_choiceAmountFieldSign->Delete(DefindByType);
                m_choiceAmountFieldSign->SetSelection(PositiveIsDeposit);
            }
            else if (item_index == UNIV_CSV_PAYEE)
            {
                payeeMatchCheckBox_->SetValue(false);
                payeeMatchCheckBox_->Disable();
                payeeMatchAddNotes_->SetValue(false);
                payeeMatchAddNotes_->Disable();
            }
        }

        csvListBox_->Delete(index);
        csvFieldOrder_.erase(csvFieldOrder_.begin() + index);

        if (static_cast<size_t>(index) < csvListBox_->GetCount()) {
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
        return (it->second);
    }

    return wxTRANSLATE("Unknown");
}

void mmUnivCSVDialog::OnLoad()
{
    csvListBox_->Clear();
    if (IsImporter() && m_choiceAmountFieldSign->GetCount() > DefindByType) m_choiceAmountFieldSign->Delete(DefindByType);
    for (const auto& entry : csvFieldOrder_)
    {
        const wxString& item_name = CSVFieldName_[entry.first];
        csvListBox_->Append(wxGetTranslation(item_name), new mmListBoxItem(entry.first, item_name));
        if (IsImporter())
        {
            if (entry.first == UNIV_CSV_TYPE) {
                unsigned int i = m_choiceAmountFieldSign->GetCount();
                if (i <= DefindByType) {
                    m_choiceAmountFieldSign->AppendString(wxString::Format(_("Positive if type has '%s'"), depositType_));
                }
                m_choiceAmountFieldSign->SetSelection(DefindByType);
            }
            if (entry.first == UNIV_CSV_PAYEE)
            {
                payeeMatchCheckBox_->Enable();
                payeeMatchAddNotes_->Enable();
            }
        }
    }
    // update csvFieldCandicate_
    csvFieldCandicate_->Clear();
    for (const auto& entry : CSVFieldName_)
    {
        std::vector<std::pair<int, int>>::const_iterator loc = find_if(csvFieldOrder_.begin(), csvFieldOrder_.end(), [&entry](const std::pair<int, int> &element) {return element.first == entry.first; });
        if (loc == csvFieldOrder_.end() || entry.first == UNIV_CSV_DONTCARE || entry.first == UNIV_CSV_NOTES) {
            csvFieldCandicate_->Append(wxGetTranslation(entry.second), new mmListBoxItem(entry.first, entry.second));
        }
    }
}

//Saves the field order to a template file
void mmUnivCSVDialog::OnSettingsSave(wxCommandEvent& WXUNUSED(event))
{
    const wxString label = m_choice_preset_name->GetStringSelection();

    wxString user_label = wxGetTextFromUser(_("Preset Name"), _("Save Preset"), label);

    if (user_label.empty())
        return;

    user_label.Trim(false).Trim();

    wxString setting_id = m_preset_id[user_label];
    wxArrayString label_names;

    for (unsigned int i = 0; i < m_choice_preset_name->GetCount(); i++) {
        label_names.Add(m_choice_preset_name->GetString(i));
    }

    if (label_names.Index(user_label) == wxNOT_FOUND)
    {
        m_choice_preset_name->Append(user_label);
        // find first available setting id to add a new setting
        int i = 0;
        for (; i < std::max({ static_cast<int>(m_choice_preset_name->GetCount()), 10 }); i++)
        {
            setting_id = wxString::Format(GetSettingsPrfix(), i);
            if (!Model_Setting::instance().contains(setting_id))
                break;
        }
        m_preset_id[user_label] = setting_id;
    }
    else if (label != user_label)
    {
        if (wxMessageBox(_("The entered name is already in use"), _("Warning"), wxOK | wxICON_WARNING) == wxOK)
        {
        }
    }

    m_choice_preset_name->SetStringSelection(user_label);

    m_checkbox_preset_default->SetValue(m_preset_id[user_label] == m_acct_default_preset[m_account_id]);
    m_checkbox_preset_default->Enable(m_choice_account_->GetSelection() > -1);

    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();

    const auto s_name = user_label;
    if (!s_name.empty())
    {
        json_writer.Key("SETTING_NAME");
        json_writer.String(s_name.utf8_str());
    }

    const auto an = m_choice_account_->GetStringSelection();
    if (!an.empty())
    {
        json_writer.Key("ACCOUNT_NAME");
        json_writer.String(an.utf8_str());
    }
    if (!date_format_.empty())
    {
        json_writer.Key("DATE_MASK");
        json_writer.String(date_format_.utf8_str());
    }
    if (!delimit_.empty())
    {
        json_writer.Key("DELIMITER");
        json_writer.String(delimit_.utf8_str());
    }

    wxString decimal = (decimal_.empty() ? m_choiceDecimalSeparator->GetStringSelection() : decimal_);
    json_writer.Key("DECIMAL");
    json_writer.String(decimal.utf8_str());


    const auto encoding = g_encoding.at(m_choiceEncoding->GetSelection()).second;
    if (!encoding.empty())
    {
        json_writer.Key("ENCODING");
        json_writer.String(encoding.utf8_str());
    }

    if (IsImporter())
    {
        // Amount sign
        const auto s = m_choiceAmountFieldSign->GetCurrentSelection();
        json_writer.Key("AMOUNT_SIGN");
        json_writer.Int(s);
        if (s == DefindByType) {
            json_writer.Key("TYPE_DEPOSIT");
            json_writer.String(depositType_.utf8_str());
        }

        // Rows to ignore
        const auto ifr = m_spinIgnoreFirstRows_->GetValue();
        json_writer.Key("IGNORE_FIRST_ROWS");
        json_writer.Int(ifr);

        const auto ilr = m_spinIgnoreLastRows_->GetValue();
        json_writer.Key("IGNORE_LAST_ROWS");
        json_writer.Int(ilr);

        // Payee matching
        json_writer.Key("PAYEE_PATTERN_MATCH");
        json_writer.Bool(payeeMatchCheckBox_->IsChecked());

        json_writer.Key("PAYEE_PATTERN_MATCH_ADD_NOTES");
        json_writer.Bool(payeeMatchAddNotes_->IsChecked());

        json_writer.Key("APPLY_COLOR");
        json_writer.Bool(colorCheckBox_->IsChecked());

        json_writer.Key("COLOR_SELECTION");
        json_writer.Int(colorButton_->GetColorId());
    }
    else
    {
        const auto et = m_checkBoxExportTitles->IsChecked();
        json_writer.Key("EXPORT_TITLES");
        json_writer.Bool(et);
    }

    json_writer.Key("FIELDS");
    json_writer.StartArray();
    for (std::vector<std::pair<int, int>>::const_iterator it = csvFieldOrder_.begin(); it != csvFieldOrder_.end(); ++it)
    {
        int i = (*it).first;
        if (i > UNIV_CSV_LAST) // This is a custom field
        {
            //"UDF: <FIELDID>"
            json_writer.String(("UDF: " + std::to_string(i - UNIV_CSV_LAST)).c_str());
        }
        else
            json_writer.String(CSVFieldName_[i].utf8_str());
    }
    json_writer.EndArray();

    json_writer.Key("FIELD_WIDTHS");
    json_writer.StartArray();
    for (int i = 1; i < m_list_ctrl_->GetColumnCount(); i++)
    {
        json_writer.Int(m_list_ctrl_->GetColumnWidth(i));
    }
    json_writer.EndArray();
    json_writer.EndObject();

    const wxString json_data = wxString::FromUTF8(json_buffer.GetString());

    Model_Setting::instance().setString(setting_id, json_data);
}

void mmUnivCSVDialog::saveAccountPresets()
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    for (const auto& preset : m_acct_default_preset) {
        if (preset.second.IsEmpty()) continue;
        json_writer.Key(wxString::Format("%lld", preset.first).utf8_str());
        json_writer.String(preset.second.utf8_str());
    }
    json_writer.EndObject();

    Model_Infotable::instance().setString(
        (IsCSV() ? "CSV_ACCOUNT_PRESETS" : "XML_ACCOUNT_PRESETS"),
        wxString::FromUTF8(json_buffer.GetString())
    );
}

bool mmUnivCSVDialog::validateData(tran_holder & holder, wxString& message)
{
    bool is_valid = true;
    if (!holder.valid) {
        is_valid = false;
        if (!holder.Date.IsValid()) message << " " << _("Invalid Date.");
        if (!holder.Amount) message << " " << _("Invalid Amount.");
        if (holder.Type.Trim().IsEmpty()) message << " " << _("Type (withdrawal/deposit) unknown.");
    }

    // If we are importing any custom field data test for validity
    if (!holder.customFieldData.empty())
        for (auto& cfdata : holder.customFieldData)
            is_valid &= validateCustomFieldData(cfdata.first, cfdata.second, message);

    Model_Payee::Data* payee = Model_Payee::instance().get(holder.PayeeID);
    if (!payee)
    {
        Model_Payee::Data* u = Model_Payee::instance().get(_("Unknown"));
        if (!u) {
            Model_Payee::Data *p = Model_Payee::instance().create();
            p->PAYEENAME = _("Unknown");
            p->ACTIVE = 1;
            p->CATEGID = -1;
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
        if (holder.CategoryID < 0) {
            holder.CategoryID = payee->CATEGID;
        }
    }

    if (holder.CategoryID == -1) //The category name is missing in SCV file and not assigned for the payee
    {
        Model_Category::Data* categ = Model_Category::instance().get(_("Unknown"), int64(-1));
        if (categ) {
            holder.CategoryID = categ->CATEGID;
        }
        else
        {
            Model_Category::Data *c = Model_Category::instance().create();
            c->CATEGNAME = _("Unknown");
            c->ACTIVE = 1;
            c->PARENTID = -1;
            holder.CategoryID = Model_Category::instance().save(c);
        }
    }

    return is_valid;
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

    bool is_canceled = false;
    long nImportedLines = 0;
    const wxString acctName = m_choice_account_->GetStringSelection();
    Model_Account::Data* account = Model_Account::instance().get(acctName);

    if (!account){
        return mmErrorDialogs::InvalidAccount(m_choice_account_);
    }

    accountID_ = account->ACCOUNTID;

    const wxString fileName = m_text_ctrl_->GetValue();
    if (fileName.IsEmpty()) {
        return mmErrorDialogs::InvalidFile(m_text_ctrl_);
    }

    // Open and parse file
    wxSharedPtr<ITransactionsFile> pParser(CreateFileHandler());
    if (!pParser) return; // is this possible?
    if (!pParser->Load(fileName, m_list_ctrl_->GetColumnCount())) {
        return;
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
    const long linesToImport = lastRow - firstRow;
    long countEmptyLines = 0;
    int color_id = colorCheckBox_->IsChecked() ? colorButton_->GetColorId() : -1;
    if (colorCheckBox_->IsChecked() && (color_id < 0 || color_id > 7) ) {
        return mmErrorDialogs::ToolTip4Object(colorButton_, _("Color"), _("Invalid value"), wxICON_ERROR);
    }

    Model_Checking::instance().Begin();
    Model_Checking::instance().Savepoint("IMP");
    Model_CustomFieldData::instance().Savepoint("IMP");

    wxProgressDialog progressDlg(_("Universal CSV Import")
        , wxEmptyString, linesToImport
        , nullptr, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_CAN_ABORT
        | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME
    );
    progressDlg.Fit();

    m_reverce_sign = m_choiceAmountFieldSign->GetCurrentSelection() == PositiveIsWithdrawal;
    // A place to store all rejected rows to display after import
    wxString rejectedRows;
    wxString reftype = Model_Attachment::REFTYPE_STR_TRANSACTION;
    for (long nLines = firstRow; nLines < lastRow; nLines++)
    {
        const wxString& progressMsg = wxString::Format(_("Transactions imported to account %s: %ld")
            , "'" + acctName + "'", nImportedLines);
        if (!progressDlg.Update(nLines - firstRow, progressMsg))
        {
            is_canceled = true;
            break; // abort processing
        }

        unsigned int numTokens = pParser->GetItemsCount(nLines);
        unsigned int blankTokenCount = 0;
        tran_holder holder;
        wxString rowString;
        if (numTokens != 0)
        {
            for (size_t i = 0; i < csvFieldOrder_.size() && i < numTokens; ++i) {
                wxString token = pParser->GetItem(nLines, i).Trim(false /*from left*/);
                // Store the CSV row to display in case the row is rejected
                rowString << inQuotes(token,",") << ((i < numTokens - 1) ? "," : "");
                if (!token.IsEmpty())
                    parseToken(csvFieldOrder_[i].first, token, holder);
                else blankTokenCount++; // keep track of blank fields
            }
        }
        // if the line had no field separators or all fields were blank (",,,,,")
        if (numTokens == 0 || blankTokenCount == numTokens)
        {
            wxString msg = wxString::Format(_("Line %ld: Empty"), nLines + 1);
            log << msg << endl;
            *log_field_ << msg << "\n";
            countEmptyLines++;
            continue;
        }

        wxString message;
        // validate data and store any error messages
        if (!validateData(holder, message))
        {
            wxString msg = wxString::Format(_("Line %ld: Error:"), nLines + 1);
            msg << " " << message;
            log << msg << endl;
            *log_field_ << msg << "\n";
            // row was rejected so save it to rejectedRows
            rejectedRows << rowString << "\n";
            continue;
        }

        wxString trxDate = holder.Date.FormatISOCombined();
        Model_Account::Data* account2 = Model_Account::instance().get(accountID_);
        const Model_Account::Data* toAccount = Model_Account::instance().get(holder.ToAccountID);
        if ((trxDate < account2->INITIALDATE) ||
            (toAccount && (trxDate < toAccount->INITIALDATE)))
        {
            wxString msg = wxString::Format(_("Line %ld: %s"), nLines + 1,
                _("The opening date for the account is later than the date of this transaction"));
            log << msg << endl;
            *log_field_ << msg << "\n";
            // row was rejected so save it to rejectedRows
            rejectedRows << rowString << "\n";
            continue;
        }

        Model_Checking::Data *pTransaction = Model_Checking::instance().create();
        pTransaction->TRANSDATE = trxDate;
        pTransaction->ACCOUNTID = accountID_;
        pTransaction->TOACCOUNTID = holder.ToAccountID;
        pTransaction->PAYEEID = holder.PayeeID;
        pTransaction->TRANSCODE = holder.Type;
        pTransaction->TRANSAMOUNT = holder.Amount;
        pTransaction->TOTRANSAMOUNT = holder.ToAmount;
        pTransaction->CATEGID = holder.CategoryID;
        pTransaction->STATUS = holder.Status;
        pTransaction->TRANSACTIONNUMBER = holder.Number;
        pTransaction->NOTES = holder.Notes;
        if (payeeMatchAddNotes_->IsChecked() && !holder.PayeeMatchNotes.IsEmpty())
            pTransaction->NOTES.Append((pTransaction->NOTES.IsEmpty() ? "" : "\n" ) + holder.PayeeMatchNotes);
        pTransaction->COLOR = color_id;

        Model_Checking::instance().save(pTransaction);

        // save custom field data
        if (!holder.customFieldData.empty())
        {
            for (const auto& field : holder.customFieldData)
            {
                Model_CustomFieldData::Data* cfdata = Model_CustomFieldData::instance().create();
                cfdata->FIELDID = field.first;
                cfdata->REFID = pTransaction->TRANSID;
                cfdata->CONTENT = field.second;
                Model_CustomFieldData::instance().save(cfdata);
            }
        }

        // save tags
        if (!holder.tagIDs.empty())
        {
            for (const auto& tag : holder.tagIDs)
            {
                Model_Taglink::Data* taglink = Model_Taglink::instance().create();
                taglink->REFTYPE = reftype;
                taglink->REFID = pTransaction->TRANSID;
                taglink->TAGID = tag;
                Model_Taglink::instance().save(taglink);
            }
        }

        nImportedLines++;
        wxString msg = wxString::Format(_("Line %ld: OK, imported."), nLines + 1);
        log << msg << endl;
        *log_field_ << msg << "\n";
    }

    // If any rows were rejected, display CSV rows in the log field and log file
    // so that users can easily copy/paste errored records for reimport
    if (!rejectedRows.IsEmpty())
    {
        *log_field_ << "\n" << _("Rejected rows:") << "\n" << rejectedRows;
        log << "\nRejected rows:\n" << rejectedRows;
    }
    progressDlg.Update(linesToImport);

    wxString msg = wxString::Format(_("Total Lines: %ld"), totalLines);
    msg << "\n";
    msg << wxString::Format(_("Lines Selected to Import: %ld"), linesToImport);
    msg << "\n\n";
    msg << wxString::Format(_("Empty Lines: %ld"), countEmptyLines);
    msg << "\n";
    msg << wxString::Format(_("Imported: %ld"), nImportedLines);
    msg << "\n";
    msg << wxString::Format(_("Errored: %ld")
        , linesToImport - countEmptyLines - nImportedLines);
    msg << "\n\n";
    msg << wxString::Format(_("Log file written to: %s"), logFile.GetFullPath());

    if (!is_canceled && wxMessageBox(
        msg + (nImportedLines > 0 ? "\n\n" + _("Please confirm saving") : "")
        , _("Import")
        , wxOK | (nImportedLines > 0 ? wxCANCEL : 0)
        | (nImportedLines == 0 ? wxICON_ERROR :
            nImportedLines < linesToImport - countEmptyLines - m_spinIgnoreFirstRows_->GetValue() - m_spinIgnoreLastRows_->GetValue()
            ? wxICON_EXCLAMATION
            : wxICON_INFORMATION
            )
    ) == wxCANCEL)
        is_canceled = true;

    msg << "\n\n";

    Model_Checking::instance().ReleaseSavepoint("IMP");
    Model_CustomFieldData::instance().ReleaseSavepoint("IMP");

    if (!is_canceled && nImportedLines > 0)
    {
        // we need to save them to the database.
        Model_Checking::instance().Commit();
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
        // discard the database changes.
        Model_Checking::instance().Rollback("");
        if (is_canceled) msg << _("Imported transactions discarded by user!");
        else msg << _("No imported transactions!");
        msg << "\n\n";
    }

    *log_field_ << "\n" << msg;

    outputLog.Close();

    /*if (!canceledbyuser && nImportedLines > 0)
        Close(); // bugfix #3877 */
    bImport_->Disable();

    refreshTabs(PAYEE_TAB | CAT_TAB);
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
                _("An error occurred while deleting the existing file. File may be locked by another program."),
                _("Destination file error"));
        }
    }

    const wxString& acctName = m_choice_account_->GetStringSelection();
    Model_Account::Data* from_account = Model_Account::instance().get(acctName);

    if (!from_account)
        return mmErrorDialogs::ToolTip4Object(m_choice_account_, _("Invalid Account"), _("Error"));

    const auto split = Model_Splittransaction::instance().get_all();
    const auto tags = Model_Taglink::instance().get_all(Model_Attachment::REFTYPE_STR_TRANSACTION);
    int64 fromAccountID = from_account->ACCOUNTID;

    long numRecords = 0;
    Model_Currency::Data* currency = Model_Account::currency(from_account);

    wxSharedPtr<ITransactionsFile> pTxFile(CreateFileHandler());

    // Write titles to file.
    if (m_checkBoxExportTitles->IsChecked())
    {
        pTxFile->AddNewLine();
        for (std::vector<std::pair<int, int>>::const_iterator sit = csvFieldOrder_.begin(); sit != csvFieldOrder_.end(); ++sit)
        {
            pTxFile->AddNewItem(wxGetTranslation(CSVFieldName_[(*sit).first]));
        }
    }

    // Write transactions to file.
    double account_balance = from_account->INITIALBAL;

    Model_Checking::Data_Set txns = Model_Checking::instance().find_or(Model_Checking::ACCOUNTID(fromAccountID), Model_Checking::TOACCOUNTID(fromAccountID));
    std::sort(txns.begin(), txns.end());
    std::stable_sort(txns.begin(), txns.end(), SorterByTRANSDATE());

    for (const auto& pBankTransaction : txns)
    {
        if (Model_Checking::status_id(pBankTransaction) == Model_Checking::STATUS_ID_VOID || !pBankTransaction.DELETEDTIME.IsEmpty())
            continue;

        Model_Checking::Full_Data tran(pBankTransaction, split, tags);
        bool has_split = tran.has_split();
        double value = Model_Checking::account_flow(pBankTransaction, fromAccountID);
        account_balance += value;

        if (!has_split)
        {
            Model_Splittransaction::Data *splt = Model_Splittransaction::instance().create();
            splt->TRANSID = tran.TRANSID;
            splt->CATEGID = tran.CATEGID;
            splt->SPLITTRANSAMOUNT = value;
            tran.m_splits.push_back(*splt);
        }

        for (const auto& splt : tran.m_splits)
        {
            pTxFile->AddNewLine();

            Model_Category::Data* category = Model_Category::instance().get(splt.CATEGID);

            double amt = splt.SPLITTRANSAMOUNT;
            if (Model_Checking::type_id(pBankTransaction) == Model_Checking::TYPE_ID_WITHDRAWAL
                && has_split) {
                amt = -amt;
            }
            const wxString amount = Model_Currency::toStringNoFormatting(amt, currency);
            const wxString amount_abs = Model_Currency::toStringNoFormatting(fabs(amt), currency);

            for (const auto& it : csvFieldOrder_)
            {
                wxString entry = "";
                ITransactionsFile::ItemType itemType = ITransactionsFile::TYPE_STRING;
                switch (it.first)
                {
                case UNIV_CSV_DATE:
                    entry = mmGetDateTimeForDisplay(Model_Checking::TRANSDATE(pBankTransaction).FormatISODate(), date_format_);
                    break;
                case UNIV_CSV_PAYEE:
                    entry = tran.real_payee_name(fromAccountID);
                    break;
                case UNIV_CSV_AMOUNT:
                    entry = amount;
                    itemType = ITransactionsFile::TYPE_NUMBER;
                    break;
                case UNIV_CSV_ACCOUNT:
                    entry = tran.get_account_name(fromAccountID);
                    break;
                case UNIV_CSV_CURRENCY:
                    entry = tran.get_currency_code(fromAccountID);
                    break;
                case UNIV_CSV_CATEGORY:
                    if (category)
                    {
                        if (isIndexPresent(UNIV_CSV_SUBCATEGORY) && category->PARENTID != -1)
                            entry = wxGetTranslation(Model_Category::full_name(category->PARENTID, ":"));
                        else
                            entry = wxGetTranslation(Model_Category::full_name(category->CATEGID, ":"));
                    }
                    break;
                case UNIV_CSV_SUBCATEGORY:
                    if(category && category->PARENTID != -1)
                        entry = wxGetTranslation(category->CATEGNAME);
                    break;
                case UNIV_CSV_TAGS:
                {
                    wxString splitTags;
                    for (const auto& tag : Model_Taglink::instance().get(Model_Attachment::REFTYPE_STR_TRANSACTIONSPLIT, splt.SPLITTRANSID))
                        splitTags.Append((splitTags.IsEmpty() ? "" : " ") + tag.first);
                    entry = tran.TAGNAMES;
                    if (!splitTags.IsEmpty())
                        entry.Append((tran.TAGNAMES.IsEmpty() ? "" : " ") + splitTags);
                    break;
                }
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
                    entry = Model_Currency::toStringNoFormatting(account_balance, currency);
                    itemType = ITransactionsFile::TYPE_NUMBER;
                    break;
                case UNIV_CSV_TYPE:
                    entry = Model_Checking::TYPE_STR[Model_Checking::type_id(pBankTransaction)];
                    break;
                case UNIV_CSV_ID:
                    entry = wxString::Format("%lld", tran.TRANSID);
                    break;
                default:
                    if (it.first > UNIV_CSV_LAST) // Custom Fields
                    {
                        // Get field content
                        Model_CustomFieldData::Data* data = Model_CustomFieldData::instance().get(it.first - UNIV_CSV_LAST, pBankTransaction.TRANSID);
                        if (data)
                        {
                            // format date fields
                            if (Model_CustomField::type_id(Model_CustomField::instance().get(data->FIELDID)) == Model_CustomField::TYPE_ID_DATE)
                                entry = mmGetDateTimeForDisplay(data->CONTENT, date_format_);
                            else
                                entry = data->CONTENT;
                        }
                    }
                    break;
                }
                pTxFile->AddNewItem(entry, itemType);
            }

            ++numRecords;
        }
    }

    pTxFile->Save(fileName);
    const wxString& msg = wxString::Format(_("Transactions exported: %ld"), numRecords);
    mmErrorDialogs::MessageWarning(this, msg, _("Export"));

}

void mmUnivCSVDialog::update_preview()
{
    this->m_list_ctrl_->ClearAll();
    m_payee_names.clear();
    m_CSVcategoryNames.clear();
    m_CSVpayeeNames.clear();
    unsigned long colCount = 0;
    this->m_list_ctrl_->InsertColumn(colCount, "#");
    this->m_list_ctrl_->SetColumnWidth(colCount, 30);
    ++colCount;

    const int MAX_ROWS_IN_PREVIEW = 50;
    const int MAX_COLS = 30; // Not including line number col.
    int date_col = -1;
    int payee_col = -1;
    int cat_col = -1;
    int subcat_col = -1;
    for (const auto& field : csvFieldOrder_)
    {
        int it = field.first;
        const wxString& item_name = this->getCSVFieldName(it);
        this->m_list_ctrl_->InsertColumn(colCount, wxGetTranslation(item_name));
        if (field.second != -1) m_list_ctrl_->SetColumnWidth(colCount, field.second);
        if (it == UNIV_CSV_DATE) {
            date_col = colCount - 1;
        }
        if (it == UNIV_CSV_PAYEE) {
            payee_col = colCount - 1;
        }
        if (it == UNIV_CSV_CATEGORY)
            cat_col = colCount - 1;

        if (it == UNIV_CSV_SUBCATEGORY)
            subcat_col = colCount - 1;

        ++colCount;
    }

    if (IsImporter())
    {
        const wxString fileName = m_text_ctrl_->GetValue();
        wxFileName csv_file(fileName);

        if (fileName.IsEmpty() || !csv_file.FileExists()) {
            return;
        }

        if (payee_col >= 0) compilePayeeRegEx();

        // Open and parse file
        std::unique_ptr <ITransactionsFile> pImporter(CreateFileHandler());
        pImporter->Load(fileName, MAX_COLS);

        unsigned int totalLines = pImporter->GetLinesCount();
        unsigned int firstRow = m_spinIgnoreFirstRows_->GetValue();
        unsigned int lastRow = totalLines - m_spinIgnoreLastRows_->GetValue();

        std::unique_ptr<mmDates> dParser(new mmDates);
        wxRegEx categDelimiterRegex(" ?: ?");
        // Import- Add rows to preview
        for (unsigned int row = 0; row < totalLines; row++)
        {
            unsigned int col = 0;
            wxString buf;
            buf.Printf("%d", col);
            long itemIndex = m_list_ctrl_->InsertItem(row, buf, 0);
            buf.Printf("%d", row + 1);
            m_list_ctrl_->SetItem(itemIndex, col, buf);
            wxString categ_name;
            wxString subcat_name;
            // Cols
            while (col < pImporter->GetItemsCount(row) && col + 1 <= MAX_COLS)
            {
                // Add a new column
                if (col == colCount - 1)
                {
                    m_list_ctrl_->InsertColumn(colCount, getCSVFieldName(-1));
                    colCount++;
                }

                auto content = pImporter->GetItem(row, col).Trim().Trim(false);
                
                // add payee names to list
                if (row >= firstRow
                    && row < lastRow
                    && col == static_cast<unsigned>(payee_col))

                {
                    if (!content.IsEmpty())
                    {
                        if (m_payee_names.Index(content, false) == wxNOT_FOUND)
                            m_payee_names.Add(content);
                    } else if (m_payee_names.Index(_("Unknown"), false) == wxNOT_FOUND)
                    {
                        m_payee_names.Add(_("Unknown"));
                        m_CSVpayeeNames[_("Unknown")] = std::make_tuple(-1, "", "");
                    }
                }

                if (row >= firstRow
                    && row < lastRow
                    && col == static_cast<unsigned>(cat_col))
                {
                    if (!content.IsEmpty())
                    {
                        // Use the ":" category delimiter to put all imported names in a consistent format.
                        categDelimiterRegex.Replace(&content, ":");
                        m_CSVcategoryNames[content] = -1;
                        categ_name = content;
                    } else
                        m_CSVcategoryNames[_("Unknown")] = -1;
                }                    

                if (row >= firstRow
                    && row < lastRow
                    && col == static_cast<unsigned>(subcat_col))
                {
                    subcat_name = content;
                }

                if (!m_userDefinedDateMask
                    && row >= firstRow
                    && row < lastRow
                    && col == static_cast<unsigned>(date_col))
                {
                    dParser->doHandleStatistics(content);
                }

                ++col;
                m_list_ctrl_->SetItem(itemIndex, col, content);
            }

            if (!subcat_name.IsEmpty())
            {
                subcat_name.Replace(":", "|");
                categ_name.Append((!categ_name.IsEmpty() ? ":" : "") + subcat_name);
                m_CSVcategoryNames[categ_name] = -1;
            }
        }

        m_spinIgnoreLastRows_->SetRange(m_spinIgnoreLastRows_->GetMin(), m_list_ctrl_->GetItemCount());
        UpdateListItemBackground();

        if (!m_userDefinedDateMask)
        {
            dParser->doFinalizeStatistics();
            if (dParser->isDateFormatFound())
            {
                const wxString date_mask = dParser->getDateMask();
                date_format_ = dParser->getDateFormat();
                choiceDateFormat_->SetStringSelection(date_mask);
                m_userDefinedDateMask = true;
            }
        }
        refreshTabs(PAYEE_TAB | CAT_TAB);
    }
    else // exporter preview
    {
        const wxString acctName = m_choice_account_->GetStringSelection();
        Model_Account::Data* from_account = Model_Account::instance().get(acctName);

        if (from_account)
        {
            const auto split = Model_Splittransaction::instance().get_all();
            const auto tags = Model_Taglink::instance().get_all(Model_Attachment::REFTYPE_STR_TRANSACTION);
            int64 fromAccountID = from_account->ACCOUNTID;
            size_t count = 0;
            int row = 0;
            const wxString& delimit = this->delimit_;

            double account_balance = from_account->INITIALBAL;
            Model_Checking::Data_Set txns =
                Model_Checking::instance().find_or(Model_Checking::ACCOUNTID(fromAccountID), Model_Checking::TOACCOUNTID(fromAccountID));
            std::sort(txns.begin(), txns.end());
            std::stable_sort(txns.begin(), txns.end(), SorterByTRANSDATE());
            for (const auto& pBankTransaction : txns)
            {
                if (Model_Checking::status_id(pBankTransaction) == Model_Checking::STATUS_ID_VOID || !pBankTransaction.DELETEDTIME.IsEmpty())
                    continue;

                Model_Checking::Full_Data tran(pBankTransaction, split, tags);
                bool has_split = tran.has_split();
                double value = Model_Checking::account_flow(pBankTransaction, fromAccountID);
                account_balance += value;

                if (!has_split)
                {
                    Model_Splittransaction::Data *splt = Model_Splittransaction::instance().create();
                    splt->TRANSID = tran.TRANSID;
                    splt->CATEGID = tran.CATEGID;
                    splt->SPLITTRANSAMOUNT = value;
                    tran.m_splits.push_back(*splt);
                }

                for (const auto& splt : tran.m_splits)
                {

                    int col = 0;
                    wxString buf;
                    buf.Printf("%d", col);
                    long itemIndex = m_list_ctrl_->InsertItem(row, buf, 0);
                    buf.Printf("%d", row + 1);
                    m_list_ctrl_->SetItem(itemIndex, col, buf);
                    m_list_ctrl_->SetItemData(itemIndex, row);
                    Model_Category::Data* category = Model_Category::instance().get(splt.CATEGID);

                    Model_Currency::Data* currency = Model_Account::currency(from_account);

                    double amt = splt.SPLITTRANSAMOUNT;
                    if (Model_Checking::type_id(pBankTransaction) == Model_Checking::TYPE_ID_WITHDRAWAL
                        && has_split) {
                        amt = -amt;
                    }
                    const wxString amount = Model_Currency::toStringNoFormatting(amt, currency);
                    const wxString amount_abs = Model_Currency::toStringNoFormatting(fabs(amt), currency);

                    for (const auto& field : csvFieldOrder_)
                    {
                        int it = field.first;
                        wxString text;
                        switch (it)
                        {
                        case UNIV_CSV_ID:
                            text << wxString::Format("%lld", tran.TRANSID);
                            break;
                        case UNIV_CSV_DATE:
                            text << inQuotes(mmGetDateTimeForDisplay(Model_Checking::TRANSDATE(pBankTransaction).FormatISODate(), date_format_), delimit);
                            break;
                        case UNIV_CSV_PAYEE:
                            text << inQuotes(tran.real_payee_name(fromAccountID), delimit);
                            break;
                        case UNIV_CSV_ACCOUNT:
                            text << inQuotes(tran.get_account_name(fromAccountID), delimit);
                            break;
                        case UNIV_CSV_CURRENCY:
                            text << inQuotes(tran.get_currency_code(fromAccountID), delimit);
                            break;
                        case UNIV_CSV_AMOUNT:
                            text << inQuotes(amount, delimit);
                            break;
                        case UNIV_CSV_CATEGORY:
                            if (category)
                            {
                                if (isIndexPresent(UNIV_CSV_SUBCATEGORY) && category->PARENTID != -1)
                                    text << inQuotes(Model_Category::full_name(category->PARENTID, ":"), delimit);
                                else
                                    text << inQuotes(Model_Category::full_name(category->CATEGID, ":"), delimit);
                            }
                            else text << inQuotes("", delimit);
                            break;
                        case UNIV_CSV_SUBCATEGORY:
                            if (category && category->PARENTID != -1)
                                text << inQuotes(category ? category->CATEGNAME : "", delimit);
                            else text << inQuotes("", delimit);
                            break;
                        case UNIV_CSV_TAGS:
                        {
                            wxString splitTags;
                            for (const auto& tag :
                                 Model_Taglink::instance().get(Model_Attachment::REFTYPE_STR_TRANSACTIONSPLIT, splt.SPLITTRANSID))
                                splitTags.Append((splitTags.IsEmpty() ? "" : " ") + tag.first);
                            text << inQuotes(tran.TAGNAMES + (tran.TAGNAMES.IsEmpty() ? "" : " ") + splitTags, delimit);
                            break;
                        }
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
                        case UNIV_CSV_TYPE:
                            text << pBankTransaction.TRANSCODE;
                            break;
                        case UNIV_CSV_DONTCARE:
                            break;
                        default:
                            if (it > UNIV_CSV_LAST) // Custom Fields
                            {
                                Model_CustomFieldData::Data* data = Model_CustomFieldData::instance().get(it - UNIV_CSV_LAST, pBankTransaction.TRANSID);
                                if (data)
                                {
                                    // Format date fields
                                    if (Model_CustomField::type_id(Model_CustomField::instance().get(data->FIELDID)) == Model_CustomField::TYPE_ID_DATE)
                                        text << inQuotes(mmGetDateTimeForDisplay(data->CONTENT, date_format_), delimit);
                                    else
                                        text << inQuotes(data->CONTENT, delimit);
                                }
                            }
                                break;
                        }

                        if (col >= m_list_ctrl_->GetColumnCount())
                            break;

                        col++;
                        m_list_ctrl_->SetItem(itemIndex, col, text);
                    }
                }
                if (++count >= MAX_ROWS_IN_PREVIEW) break;
                ++row;
            }
            // sort to align all splits together in the preview
            m_list_ctrl_->SortItems(
                [](wxIntPtr itemIndex1, wxIntPtr itemIndex2, wxIntPtr WXUNUSED(sortData))
                {
                    if (itemIndex1 < itemIndex2)
                        return -1;
                    if (itemIndex1 > itemIndex2)
                        return 1;
                    return 0;
                },
                0);
        }
    }
}

// refresh data in payee & category tabs
void mmUnivCSVDialog::refreshTabs(int tabs) {
    int num = 0;
    if (tabs & DATA_TAB)
        update_preview();
    if (tabs & PAYEE_TAB)
    {
        validatePayees();
        payeeListBox_->DeleteAllItems();
        for (const auto& payee : m_payee_names)
        {
            wxVector<wxVariant> data;
            data.push_back(wxVariant(payee));
            if (payee == _("Unknown") || (m_CSVpayeeNames.find(payee) != m_CSVpayeeNames.end() && std::get<0>(m_CSVpayeeNames[payee]) != -1))
            {
                if (std::get<2>(m_CSVpayeeNames[payee]) == wxEmptyString)
                    data.push_back(wxVariant(_("OK")));
                else
                    data.push_back(wxVariant(wxString::Format(_("Matched to %1$s by pattern %2$s"),
                        std::get<1>(m_CSVpayeeNames[payee]),
                        std::get<2>(m_CSVpayeeNames[payee])
                    )));
            }
            else
                data.push_back(wxVariant(_("Missing")));
            payeeListBox_->AppendItem(data, static_cast<wxUIntPtr>(num++));
        }
    }
    if (tabs & CAT_TAB)
    {
        validateCategories();
        num = 0;
        categoryListBox_->DeleteAllItems();
        for (const auto& categ : m_CSVcategoryNames)
        {
            wxVector<wxVariant> data;
            data.push_back(wxVariant(categ.first));
            if (categ.second == -1)
                data.push_back(wxVariant(_("Missing")));
            else
                data.push_back(wxVariant(_("OK")));
            categoryListBox_->AppendItem(data, static_cast<wxUIntPtr>(num++));
        }
    }
}

void mmUnivCSVDialog::OnMoveUp(wxCommandEvent& WXUNUSED(event))
{
    int index = csvListBox_->GetSelection();
    if (index != wxNOT_FOUND && index != 0)
    {
        mmListBoxItem* item = static_cast<mmListBoxItem*>(csvListBox_->GetClientObject(index));
        int item_index = item->getIndex().GetValue();
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
    if (index != wxNOT_FOUND && static_cast<size_t>(index) != csvListBox_->GetCount() - 1)
    {
        mmListBoxItem* item = static_cast<mmListBoxItem*>(csvListBox_->GetClientObject(index));
        int item_index = item->getIndex().GetValue();
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
    int standard[] = { UNIV_CSV_ID, UNIV_CSV_DATE, UNIV_CSV_STATUS, UNIV_CSV_TYPE, UNIV_CSV_ACCOUNT, UNIV_CSV_PAYEE
                     , UNIV_CSV_CATEGORY, UNIV_CSV_SUBCATEGORY, UNIV_CSV_AMOUNT, UNIV_CSV_CURRENCY, UNIV_CSV_TRANSNUM, UNIV_CSV_NOTES };
    for (const auto i : standard)
    {
        csvListBox_->Append(wxGetTranslation(CSVFieldName_[i]), new mmListBoxItem(i, CSVFieldName_[i]));
        csvFieldOrder_.push_back(std::make_pair(i, -1));
    }

    csvFieldCandicate_->Clear();
    int rest[] = { UNIV_CSV_NOTES, UNIV_CSV_DONTCARE, UNIV_CSV_WITHDRAWAL, UNIV_CSV_DEPOSIT, UNIV_CSV_BALANCE };
    for (const auto i : rest)
    {
        csvFieldCandicate_->Append(wxGetTranslation(CSVFieldName_[i]), new mmListBoxItem(i, CSVFieldName_[i]));
    }

    update_preview();
}

void mmUnivCSVDialog::OnButtonClearClick(wxCommandEvent& WXUNUSED(event))
{
    int sel = m_choice_preset_name->GetSelection();
    int size = m_choice_preset_name->GetCount();
    if (sel >= 0 && size > 0)
    {
        wxString preset_name = m_choice_preset_name->GetStringSelection();
        if (wxMessageBox(
            wxString::Format(_("Preset '%s' will be deleted"), preset_name) + "\n\n" +
            _("Do you want to continue?")
            , _("Delete Preset"), wxYES_NO | wxICON_WARNING) == wxNO)
        {
            return;
        }
        wxString preset_id = m_preset_id[preset_name];
        Model_Setting::Data_Set data = Model_Setting::instance().find(Model_Setting::SETTINGNAME(preset_id));
        if (data.size() > 0)
            Model_Setting::instance().remove(data[0].SETTINGID);

        // update default presets to remove any that reference the deleted item
        for (auto& member : m_acct_default_preset)
            if (member.second == preset_id)
                member.second = "";
        saveAccountPresets();

        m_choice_preset_name->Delete(sel);
        m_choice_preset_name->SetSelection(-1);
    }
    SetSettings("{}");
    m_checkbox_preset_default->SetValue(false);
    m_checkbox_preset_default->Disable();
}


void mmUnivCSVDialog::OnFileBrowse(wxCommandEvent& WXUNUSED(event))
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
    const wxString defaultWildcard = IsXML()
        ? wxString() << _("XML Files (*.xml)") << "|*.xml;*.XML|" << _("All Files") << "|" << wxFileSelectorDefaultWildcardStr
        : wxString() << _("CSV Files (*.csv)") << "|*.csv;*.CSV";
    const wxString chooseExt = IsXML() ? "*.xml" : "*.csv";

    if (!IsImporter()) correctEmptyFileExt("csv", fileName);

    fileName = wxFileSelector(header
        , fileName, wxEmptyString, chooseExt, defaultWildcard, flags);

    if (!fileName.IsEmpty())
    {
        m_text_ctrl_->ChangeValue(fileName);

        if (IsImporter()) {
            wxTextFile tFile(fileName);
            if (!tFile.Open())
            {
                *log_field_ << _("Unable to open file.") << "\n";
                return;
            }

            wxSharedPtr<mmSeparator> sep(new mmSeparator);
            wxString line;
            size_t count = 0;
            for (line = tFile.GetFirstLine(); !tFile.Eof(); line = tFile.GetNextLine())
            {
                *log_field_ << line << "\n";
                if (++count >= 10) break;
                sep->isStringHasSeparator(line);
            }

            *log_field_ << "\n";

            delimit_ = sep->getSeparator();
            if (!IsXML()) m_textDelimiter->ChangeValue(delimit_);

            // TODO: update_preview() is called twice. Once here and once in OnFileNameChanged().
            // This leads to double work and double error messages to the user.
            this->update_preview();
            bImport_->Enable();
        }
    }
}

void mmUnivCSVDialog::OnListBox(wxCommandEvent& event)
{
    if (m_object_in_focus == ID_LISTBOX_CANDICATE)
        OnAdd(event);
    else if (m_object_in_focus == ID_LISTBOX)
        OnRemove(event);
}

void mmUnivCSVDialog::OnDelimiterChange(wxCommandEvent&)
{
    wxString delimit = m_textDelimiter->GetValue();

    if (!delimit.IsEmpty())
    {
        if (delimit == "\\") {
            delimit = "\t";
            m_textDelimiter->ChangeValue("\\t");
        }
        m_textDelimiter->SelectAll();
        delimit_ = delimit;
        this->update_preview();
    }
}

void mmUnivCSVDialog::OnDecimalChange(wxCommandEvent& event)
{
    int i = m_choiceDecimalSeparator->GetSelection();
    wxStringClientData* type_obj = static_cast<wxStringClientData*>(m_choiceDecimalSeparator->GetClientObject(i));
    if (type_obj) {
        decimal_ = type_obj->GetData();
    }

    event.Skip();
}

void mmUnivCSVDialog::compilePayeeRegEx() {
    // pre-compile all payee match strings if not already done
    if (payeeMatchCheckBox_->IsChecked() && !payeeRegExInitialized_) {
        payeeMatchPatterns_.clear();
        // only look at payees that have a match pattern set
        Model_Payee::Data_Set payees = Model_Payee::instance().find(Model_Payee::PATTERN(wxEmptyString, NOT_EQUAL));
        for (const auto& payee : payees) {
            Document json_doc;
            if (json_doc.Parse(payee.PATTERN.utf8_str()).HasParseError()) {
                continue;
            }
            int key = -1;
            // loop over all keys in the pattern json data
            for (const auto& member : json_doc.GetObject()) {
                key++;
                wxString pattern = member.value.GetString();
                // add the pattern string (for non-regex match, match notes, and the payee tab preview)
                payeeMatchPatterns_[std::make_pair(payee.PAYEEID, payee.PAYEENAME)][key].first = pattern;
                // complie the regex if necessary
                if (pattern.StartsWith("regex:")) {
                    payeeMatchPatterns_[std::make_pair(payee.PAYEEID, payee.PAYEENAME)][key].second.Compile(pattern.Right(pattern.length() - 6), wxRE_ICASE | wxRE_EXTENDED);
                }
            }
        }
        payeeRegExInitialized_ = true;
    }
}

void mmUnivCSVDialog::validatePayees() {
    if (!payeeRegExInitialized_) compilePayeeRegEx();

    for (const auto& payee_name : m_payee_names) {
        bool payee_found = false;
        // initialize
        m_CSVpayeeNames[payee_name] = std::make_tuple(-1, "", "");
        // perform pattern match
        if (payeeMatchCheckBox_->IsChecked()) {
            // loop over all the precompiled patterns
            for (const auto& payeeId : payeeMatchPatterns_) {
                for (const auto& pattern : payeeId.second) {
                    bool match = false;
                    // match against regex if the pattern begins with "regex:"
                    if (pattern.second.first.StartsWith("regex:"))
                        match = pattern.second.second.Matches(payee_name);
                    else // use the normal wxString match for non-regex patterns
                        match = payee_name.Lower().Matches(pattern.second.first.Lower());
                    if (match)
                    {
                        payee_found = true;
                        // save the target payee ID, name, and match details
                        m_CSVpayeeNames[payee_name] = std::make_tuple(payeeId.first.first, payeeId.first.second, pattern.second.first);
                        break;
                    }
                }
                if (payee_found) break;
            }
        }
        if (!payee_found) {
            Model_Payee::Data* payee = Model_Payee::instance().get(payee_name);
            if (payee) {
                m_CSVpayeeNames[payee_name] = std::make_tuple(payee->PAYEEID, payee->PAYEENAME, "");
            }
        }
    }
}

void mmUnivCSVDialog::validateCategories() {
    for(const auto& catname : m_CSVcategoryNames)
    {
        wxString search_name = catname.first;
        int64 parentID = -1;
        // delimit string by ":"
        wxStringTokenizer categs = wxStringTokenizer(search_name, ":");
        // check each level of category exists
        Model_Category::Data* category = nullptr;
        while (categs.HasMoreTokens()) {
            wxString categname = categs.GetNextToken();
            category = Model_Category::instance().get(categname, parentID);
            if (!category)
            {
                break;
            }
            parentID = category->CATEGID;
        }

        if (category) m_CSVcategoryNames[search_name] = category->CATEGID;
    }
}

void mmUnivCSVDialog::parseToken(int index, const wxString& orig_token, tran_holder& holder)
{
    if (orig_token.IsEmpty()) return;
    wxString token = orig_token.Strip(wxString::leading).Strip(wxString::trailing);

    double amount;

    switch (index)
    {
    case UNIV_CSV_DATE:
    {
        wxDateTime dtdt;
        if (mmParseDisplayStringToDate(dtdt, token, date_format_))
            holder.Date = dtdt;
        else
            holder.valid = false;
        break;
    }
    case UNIV_CSV_PAYEE:
        if (m_CSVpayeeNames.find(token) != m_CSVpayeeNames.end() && std::get<0>(m_CSVpayeeNames[token]) != -1)
        {
            holder.PayeeID = std::get<0>(m_CSVpayeeNames[token]);
            if (payeeMatchAddNotes_->IsChecked() && !std::get<2>(m_CSVpayeeNames[token]).IsEmpty())
            {
                holder.PayeeMatchNotes = wxString::Format(_("%1$s matched by %2$s"), token, std::get<2>(m_CSVpayeeNames[token]));
            }
        }
        else
        {
            Model_Payee::Data* payee = Model_Payee::instance().create();
            payee->PAYEENAME = token;
            payee->ACTIVE = 1;
            holder.PayeeID = Model_Payee::instance().save(payee);
            m_CSVpayeeNames[token] = std::make_tuple(holder.PayeeID, token, wxEmptyString);
        }
        break;

    case UNIV_CSV_AMOUNT:
        mmTrimAmount(token, decimal_, ".").ToCDouble(&amount);

        if (find_if(csvFieldOrder_.begin(), csvFieldOrder_.end(), [](const std::pair<int, int>& element) {return element.first == UNIV_CSV_TYPE; }) == csvFieldOrder_.end()) {
            if ((amount > 0.0 && !m_reverce_sign) || (amount <= 0.0 && m_reverce_sign)) {
                holder.Type = Model_Checking::TYPE_STR_DEPOSIT;
            }
        }

        holder.Amount = fabs(amount);
        break;

    case UNIV_CSV_CATEGORY:
    {
        // Convert to standard delimiter for consistency
        wxRegEx categDelimiterRegex(" ?: ?");
        categDelimiterRegex.Replace(&token, ":");
        // check if we already have this category
        if (m_CSVcategoryNames.find(token) != m_CSVcategoryNames.end() && m_CSVcategoryNames[token] != -1)
            holder.CategoryID = m_CSVcategoryNames[token];
        else // create category and any missing parent categories
        {
            Model_Category::Data* category = nullptr;
            int64 parentID = -1;
            wxStringTokenizer tokenizer = wxStringTokenizer(token, ":");
            while (tokenizer.HasMoreTokens())
            {
                wxString categname = tokenizer.GetNextToken().Trim().Trim(false);
                category = Model_Category::instance().get(categname, parentID);
                if (!category)
                {
                    category = Model_Category::instance().create();
                    category->CATEGNAME = categname;
                    category->PARENTID = parentID;
                    category->ACTIVE = 1;
                    Model_Category::instance().save(category);
                }
                parentID = category->CATEGID;
            }

            if (category)
            {
                holder.CategoryID = category->CATEGID;
                m_CSVcategoryNames[token] = category->CATEGID;
            }
        }
        break;
    }
    case UNIV_CSV_SUBCATEGORY:
    {
        if (holder.CategoryID == -1)
            return;

        token.Replace(":", "|");
        wxString categname = Model_Category::full_name(holder.CategoryID);
        categname.Append(":" + token);
        if (m_CSVcategoryNames.find(categname) != m_CSVcategoryNames.end() && m_CSVcategoryNames[categname] != -1)
            holder.CategoryID = m_CSVcategoryNames[categname];
        else
        {
            Model_Category::Data* category = Model_Category::instance().create();
            category->PARENTID = holder.CategoryID;
            category->CATEGNAME = token;
            category->ACTIVE = 1;
            Model_Category::instance().save(category);

            holder.CategoryID = category->CATEGID;
            m_CSVcategoryNames[categname] = category->CATEGID;
        }
        break;
    }
    case UNIV_CSV_TAGS:
    {
        // split the tag string at space characters
        wxStringTokenizer tokenizer = wxStringTokenizer(token, " ");
        while (tokenizer.HasMoreTokens())
        {
            wxString tagname = tokenizer.GetNextToken();
            // check for an existing tag
            Model_Tag::Data* tag = Model_Tag::instance().get(tagname);
            if (!tag)
            {
                // create a new tag if we didn't find one
                tag = Model_Tag::instance().create();
                tag->TAGNAME = tagname;
                tag->ACTIVE = 1;
                Model_Tag::instance().save(tag);
            }
            // add the tagID to the transaction if it isn't already there
            if (std::find(holder.tagIDs.begin(), holder.tagIDs.end(), tag->TAGID) == holder.tagIDs.end())
                holder.tagIDs.push_back(tag->TAGID);
        }
        break;
    }
    case UNIV_CSV_TRANSNUM:
        holder.Number = token;
        break;

    case UNIV_CSV_NOTES:
        token.Replace("\\n", "\n");
        holder.Notes += (holder.Notes.IsEmpty() ? "" : "\n") + token;
        break;

    case UNIV_CSV_WITHDRAWAL:
        if (token.IsEmpty())
            return;

        // do nothing if an amount has already been stored by a previous call #3168
        if (holder.Amount != 0.0)
            break;

        if (!mmTrimAmount(token, decimal_, ".").ToCDouble(&amount))
            break;

        if (amount == 0.0)
            break;

        holder.Amount = fabs(amount);
        holder.Type = Model_Checking::TYPE_STR_WITHDRAWAL;
        break;

    case UNIV_CSV_DEPOSIT:
        if (token.IsEmpty())
            return;

        // do nothing if an amount has already been stored by a previous call #3168
        if (holder.Amount != 0.0)
            break;

        if (!mmTrimAmount(token, decimal_, ".").ToCDouble(&amount))
            break;

        if (amount == 0.0)
            break;

        holder.Amount = fabs(amount);
        holder.Type = Model_Checking::TYPE_STR_DEPOSIT;
        break;

        // A number of type options are supported to make amount positive 
        // ('debit' seems odd but is there for backwards compatability!)
    case UNIV_CSV_TYPE:
        if (m_choiceAmountFieldSign->GetSelection() == DefindByType)
        {
            if (depositType_.CmpNoCase(token) == 0)
            {
                holder.Type = Model_Checking::TYPE_STR_DEPOSIT;
                break;
            }
        }
        else
        {
            for (const wxString entry : { "debit", "deposit", "+" }) {
                if (entry.CmpNoCase(token) == 0) {
                    holder.Type = Model_Checking::TYPE_STR_DEPOSIT;
                    break;
                }
            }
        }
        break;
    default:
        if (index > UNIV_CSV_LAST) // custom fields
            holder.customFieldData[index - UNIV_CSV_LAST] = token;
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

void mmUnivCSVDialog::OnChoiceChanged(wxCommandEvent& event)
{
    int sel = event.GetInt();
    if (sel == wxNOT_FOUND)
        return;
    int i = event.GetId();
    if (i == ID_DATE_FORMAT)
    {
        wxStringClientData* data = static_cast<wxStringClientData*>(event.GetClientObject());
        if (data) date_format_ = data->GetData();
        *log_field_ << date_format_ << "\n";
    }
    else if (i == wxID_ACCOUNT)
    {
        wxString acctName = m_choice_account_->GetStringSelection();
        Model_Account::Data* account = Model_Account::instance().get(acctName);
        m_account_id = account->ACCOUNTID;
        Model_Currency::Data* currency = Model_Account::currency(account);
        *log_field_ << _("Currency:") << " " << wxGetTranslation(currency->CURRENCYNAME) << "\n";

        m_checkbox_preset_default->Enable(m_choice_preset_name->GetSelection() >= 0);
        m_checkbox_preset_default->SetValue(false);
        m_checkbox_preset_default->SetLabelText(wxString::Format(_("Load this Preset when Account is:\n%s"), acctName));
        Fit();
        for (const auto& preset : m_preset_id)
            if (preset.second == m_acct_default_preset[m_account_id])
            {
                m_choice_preset_name->SetStringSelection(preset.first);
                SetSettings(GetStoredSettings(m_choice_preset_name->GetSelection()));
                *log_field_ << wxString::Format(_("Preset '%1$s' loaded because Account '%2$s' selected"), preset.first, acctName) << "\n";
                break;
            }
    }
    else if (i == ID_ENCODING)
    {
        *log_field_ << m_choiceEncoding->GetStringSelection() << "\n";
    }
    else if (i == wxID_REPLACE)
    {
        if (m_choiceAmountFieldSign->GetSelection() == DefindByType) {
            wxString depositType = wxGetTextFromUser(_("Please, type the word indicating positive values in the CSV file, e.g., 'credit'."), _("Enter a value"), depositType_);
            if (!depositType.empty()) {
                depositType_ = depositType;
            }
            m_choiceAmountFieldSign->SetString(DefindByType, wxString::Format(_("Positive if type has '%s'"), depositType_));
            m_choiceAmountFieldSign->SetSelection(DefindByType);
        }
        else if (std::find_if(csvFieldOrder_.begin(), csvFieldOrder_.end(), [](const std::pair<int, int>& element) {return element.first == UNIV_CSV_TYPE; }) != csvFieldOrder_.end()) {
            m_choiceAmountFieldSign->Select(DefindByType);
            mmErrorDialogs::ToolTip4Object(m_choiceAmountFieldSign
                , _("Amount sign must be defined by type when 'Type' is selected for import")
                , _("Invalid Entry"), wxICON_WARNING);
        }
    }
    m_userDefinedDateMask = true;
    this->update_preview();
}

void mmUnivCSVDialog::changeFocus(wxChildFocusEvent& event)
{
    wxWindow *w = event.GetWindow();
    if (w)
        m_object_in_focus = w->GetId();
}

void mmUnivCSVDialog::OnSpinCtrlIgnoreRows(wxSpinEvent& WXUNUSED(event))
{
    UpdateListItemBackground();
}

void mmUnivCSVDialog::UpdateListItemBackground()
{
    int firstRow = m_spinIgnoreFirstRows_->GetValue();
    int lastRow = m_list_ctrl_->GetItemCount() - m_spinIgnoreLastRows_->GetValue() - 1;
    for (int row = 0; row < m_list_ctrl_->GetItemCount(); row++)
    {
        wxColour color = row >= firstRow && row <= lastRow ? m_list_ctrl_->GetBackgroundColour() : *wxLIGHT_GREY;
        m_list_ctrl_->SetItemBackgroundColour(row, color);
    }
}

bool mmUnivCSVDialog::isIndexPresent(int index) const
{
    for (std::vector<std::pair<int, int>>::const_iterator it = csvFieldOrder_.begin(); it != csvFieldOrder_.end(); ++it)
    {
        if ((*it).first == index) return true;
    }

    return false;
}

bool mmUnivCSVDialog::ShowToolTips()
{
    return true;
}

ITransactionsFile *mmUnivCSVDialog::CreateFileHandler()
{
    // XML
    if (IsXML())
        return new FileXML(this, g_encoding.at(m_choiceEncoding->GetSelection()).second);

    // CSV
    return new FileCSV(this, g_encoding.at(m_choiceEncoding->GetSelection()).first, delimit_);
}

void mmUnivCSVDialog::OnCheckboxClick(wxCommandEvent& event)
{
    auto id = event.GetId();
    if (IsImporter())
    {
        if (id == mmID_COLOR && colorButton_)
            colorButton_->Enable(colorCheckBox_->IsChecked());
        else if (id == mmID_PAYEE)
        {
            payeeMatchAddNotes_->Enable(payeeMatchCheckBox_->IsChecked());
            payeeMatchAddNotes_->SetValue(false);
            refreshTabs(PAYEE_TAB);
        }
    }

    if (id == wxID_DEFAULT)
    {
        wxString preset_name = m_choice_preset_name->GetStringSelection();

        if (m_checkbox_preset_default->IsChecked())
            m_acct_default_preset[m_account_id] = m_preset_id[preset_name];
        else if (m_acct_default_preset[m_account_id] == m_preset_id[preset_name])
            m_acct_default_preset[m_account_id] = "";

        saveAccountPresets();
    }
}

void mmUnivCSVDialog::OnMenuSelected(wxCommandEvent&)
{
    colorButton_->Enable(false);
    colorCheckBox_->SetValue(false);
}

/* Validates the specified string matches the parameters of the target Custom Field.
Cleanses the value and reformats as needed for DB storage
*/
bool mmUnivCSVDialog::validateCustomFieldData(int64 fieldId, wxString& value, wxString& message)
{
    bool is_valid = true;
    long int_val;
    int index;
    double double_val;
    wxDate date;
    wxDateTime time;
    wxArrayString choices;
    wxStringTokenizer tokenizer;

    // Set up valid boolean values
    const wxString bool_true[] = { "True", "T", "1", "Y" };
    const wxArrayString bool_true_array(4, bool_true);
    const wxString bool_false[] = { "False", "F", "0", "N", ""};
    const wxArrayString bool_false_array(4, bool_false);

    if (!value.IsEmpty())
    {
        const Model_CustomField::Data* data = Model_CustomField::instance().get(fieldId);
        wxString type_string = Model_CustomField::TYPE_STR[Model_CustomField::type_id(data)];
        switch (Model_CustomField::type_id(data))
        {
            // Check if string can be read as an integer. Will fail if passed a double.
        case Model_CustomField::TYPE_ID_INTEGER:
            value = cleanseNumberString(value, true);
            if (!value.ToCLong(&int_val))
            {
                message << " " << wxString::Format(_("Value %1$s for custom field '%2$s' is not type %3$s."), value, data->DESCRIPTION, type_string);
                is_valid = false;
            }
            else value = wxString::Format("%i", int_val);
            break;

            // Check if string can be read as a double
        case Model_CustomField::TYPE_ID_DECIMAL:
            value = cleanseNumberString(value, true);
            if (!value.ToCDouble(&double_val))
            {
                message << " " << wxString::Format(_("Value %1$s for custom field '%2$s' is not type %3$s."), value, data->DESCRIPTION, type_string);
                is_valid = false;
            }
            else
            {
                // round to required precision
                int precision = Model_CustomField::getDigitScale(data->PROPERTIES);
                value = wxString::Format("%.*f", precision, double_val);
            }
            break;

            // Check if string can be interpreted as "True" or "False" (case insensitive)    
        case Model_CustomField::TYPE_ID_BOOLEAN:
            if (bool_true_array.Index(value, false) == wxNOT_FOUND)
                if (bool_false_array.Index(value, false) == wxNOT_FOUND)
                {
                    message << " " << wxString::Format(_("Value %1$s for custom field '%2$s' is not type %3$s."), value, data->DESCRIPTION, type_string);
                    is_valid = false;
                }
                else value = "FALSE";
            else value = "TRUE";
            break;

            // Check if string is a valid choice (case insensitive)
        case Model_CustomField::TYPE_ID_SINGLECHOICE:
            choices = Model_CustomField::getChoices(data->PROPERTIES);
            index = choices.Index(value, false);
            if (index == wxNOT_FOUND)
            {
                message << " " << wxString::Format(_("Value %1$s for %2$s custom field '%3$s' is not a valid selection."), value, type_string, data->DESCRIPTION);
                is_valid = false;
            }
            else value = choices[index];
            break;

            // Check if all of the ';' delimited strings are valid choices (case insensitive)
        case Model_CustomField::TYPE_ID_MULTICHOICE:
            choices = Model_CustomField::getChoices(data->PROPERTIES);
            tokenizer = wxStringTokenizer(value, ";");
            value.Clear();
            while (tokenizer.HasMoreTokens())
            {
                wxString token = tokenizer.GetNextToken();
                index = choices.Index(token, false);
                if (index != wxNOT_FOUND)
                {
                    value.Append(choices[index]);
                    if (tokenizer.HasMoreTokens()) value.Append(";");
                }
                else {
                    message << " " << wxString::Format(_("Value %1$s for %2$s custom field '%3$s' is not a valid selection."), token, type_string, data->DESCRIPTION);
                    is_valid = false;
                }
            }
            break;

            // Parse the date using the user specified format. Convert to ISO date
        case Model_CustomField::TYPE_ID_DATE:
            if (!mmParseDisplayStringToDate(date, value, date_format_))
            {
                message << " " << wxString::Format(_("Value %1$s for custom field '%2$s' is not type %3$s."), value, data->DESCRIPTION, type_string) <<
                    " " << wxString::Format(_("Confirm format matches selection %s."), date_format_);
                is_valid = false;
            }
            else value = date.FormatISODate();
            break;

            // Parse the time. Convert to ISO Format
        case Model_CustomField::TYPE_ID_TIME:
            if (!time.ParseTime(value))
            {
                message << " " << wxString::Format(_("Value %1$s for custom field '%2$s' is not type %3$s."), value, data->DESCRIPTION, type_string);
                is_valid = false;
            }
            else value = time.FormatISOTime();
            break;

        default: break;
        }

        // if regex check is enabled, perform regex validation
        const wxString regExStr = Model_CustomField::getRegEx(data->PROPERTIES);
        if (!regExStr.empty())
        {
            wxRegEx regEx(regExStr, wxRE_EXTENDED);

            if (!regEx.Matches(value))
            {
                message << " " << wxString::Format(_("Value %1$s does not match regex %2$s for custom field '%3$s'."), value, regExStr, data->DESCRIPTION);
                is_valid = false;
            }
        }

    }

    return is_valid;
}
