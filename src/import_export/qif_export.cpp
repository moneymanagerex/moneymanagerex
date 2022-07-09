/*******************************************************
Copyright (C) 2014-2017 Nikolay Akimov

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
#include "qif_export.h"
#include "util.h"
#include "paths.h"
#include "export.h"
#include "mmSimpleDialogs.h"
#include "option.h"
#include "model/Model_Infotable.h"
#include "model/Model_Account.h"
#include "model/Model_Category.h"
#include "model/Model_Attachment.h"
#include "model/Model_CustomFieldData.h"

wxIMPLEMENT_DYNAMIC_CLASS(mmQIFExportDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmQIFExportDialog, wxDialog)
    EVT_CHECKBOX(wxID_ANY, mmQIFExportDialog::OnCheckboxClick )
    EVT_CHOICE(wxID_ANY, mmQIFExportDialog::OnChoiceType)
    EVT_BUTTON(wxID_OK, mmQIFExportDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmQIFExportDialog::OnCancel)
    EVT_CLOSE(mmQIFExportDialog::OnQuit)
wxEND_EVENT_TABLE()

mmQIFExportDialog::mmQIFExportDialog(wxWindow *parent, int type)
{
    m_type = type;
    wxString type_name;
    switch (type)
    {
    case (QIF): type_name = _("QIF Export"); break;
    case (JSON): type_name = _("JSON Export"); break;
    case (CSV): type_name = _("CSV Export"); break;
    }
    Create(parent, type_name);

}

bool mmQIFExportDialog::Create(wxWindow* parent, const wxString& caption, wxWindowID id
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    fillControls();

    SetIcon(mmex::getProgramIcon());
    this->SetMinSize(wxSize(350, 450));
    this->Fit();

    return TRUE;
}

void mmQIFExportDialog::fillControls()
{
    bSelectedAccounts_->SetLabelText(_("All"));
    mmToolTip(bSelectedAccounts_, _("All"));

    m_accounts_name.clear();
    selected_accounts_id_.clear();
    accounts_id_.clear();

    Model_Account::Data_Set selected_accounts = Model_Account::instance().find(
            Model_Account::ACCOUNTTYPE(Model_Account::all_type()[Model_Account::INVESTMENT], NOT_EQUAL)
    );

    for (const auto& a : selected_accounts)
    {
        m_accounts_name.Add(a.ACCOUNTNAME);
        selected_accounts_id_.Add(a.ACCOUNTID);
        accounts_id_.Add(a.ACCOUNTID);
    }

    // redirect logs to text control
    //logger_ = wxLog::SetActiveTarget(new wxLogTextCtrl(log_field_));
    //wxLogMessage( "This is the log window" );
}

void mmQIFExportDialog::CreateControls()
{
    int border = 5;
    const auto min_size = wxSize(180, -1);

    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(main_sizer);
    wxBoxSizer* box_sizer1 = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(box_sizer1, g_flagsExpand);

    wxNotebook* export_notebook = new wxNotebook(this
        , wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE );
    wxPanel* main_tab = new wxPanel(export_notebook, wxID_ANY);
    export_notebook->AddPage(main_tab, _("Parameters"));
    wxBoxSizer *tab1_sizer = new wxBoxSizer(wxVERTICAL);
    main_tab->SetSizer(tab1_sizer);

    wxPanel* log_tab = new wxPanel(export_notebook, wxID_ANY);
    export_notebook->AddPage(log_tab, _("Log"));
    wxBoxSizer *tab2_sizer = new wxBoxSizer(wxVERTICAL);
    log_tab->SetSizer(tab2_sizer);

    box_sizer1->Add(export_notebook, g_flagsExpand);

    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    tab1_sizer->Add(flex_sizer, wxSizerFlags(g_flagsV).Left());

    // Type -------------------------------------------------
    wxStaticText* type = new wxStaticText(main_tab, wxID_STATIC, _("Type"));
    wxChoice* typeCheckBox = new wxChoice(main_tab, wxID_ANY);
    typeCheckBox->AppendString(_("QIF"));
    typeCheckBox->AppendString(_("JSON"));
    typeCheckBox->AppendString(_("CSV"));
    typeCheckBox->SetSelection(m_type);
    typeCheckBox->SetMinSize(min_size);
    flex_sizer->Add(type, g_flagsH);
    flex_sizer->Add(typeCheckBox, g_flagsH);

    // Categories -------------------------------------------------
    cCategs_ = new wxCheckBox(main_tab, wxID_ANY
        , _("Categories"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cCategs_->SetValue(FALSE);
    flex_sizer->Add(cCategs_, g_flagsH);
    flex_sizer->AddSpacer(1);

    // Accounts --------------------------------------------
    accountsCheckBox_ = new wxCheckBox( main_tab, wxID_ANY, _("Accounts")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    bSelectedAccounts_ = new wxButton(main_tab, wxID_STATIC, _("All"));
    bSelectedAccounts_->SetMinSize(min_size);
    bSelectedAccounts_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFExportDialog::OnAccountsButton), nullptr, this);
    accountsCheckBox_->SetValue(true);
    flex_sizer->Add(accountsCheckBox_, g_flagsH);
    flex_sizer->Add(bSelectedAccounts_, g_flagsH);

    // From Date --------------------------------------------
    dateFromCheckBox_ = new wxCheckBox(main_tab, wxID_ANY, _("From Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    fromDateCtrl_ = new mmDatePickerCtrl(main_tab, wxID_STATIC, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    fromDateCtrl_->SetMinSize(min_size);
    fromDateCtrl_->Enable(false);
    flex_sizer->Add(dateFromCheckBox_, g_flagsH);
    flex_sizer->Add(fromDateCtrl_, g_flagsH);

    // To Date --------------------------------------------
    dateToCheckBox_ = new wxCheckBox(main_tab, wxID_ANY, _("To Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    toDateCtrl_ = new mmDatePickerCtrl(main_tab, wxID_STATIC, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    toDateCtrl_->SetMinSize(min_size);
    toDateCtrl_->Enable(false);
    flex_sizer->Add(dateToCheckBox_, g_flagsH);
    flex_sizer->Add(toDateCtrl_, g_flagsH);

    // Encoding --------------------------------------------

    // Date Format Settings --------------------------------
    wxString dateFormatStr = Option::instance().getDateFormat();

    choiceDateFormat_label_ = new wxStaticText(main_tab, wxID_STATIC, _("Date Format"));
    m_choiceDateFormat = new wxComboBox(main_tab, wxID_ANY);
    for (const auto& i : g_date_formats_map())
    {
        m_choiceDateFormat->Append(i.second, new wxStringClientData(i.first));
        if (dateFormatStr == i.first) m_choiceDateFormat->SetStringSelection(i.second);
    }

    flex_sizer->Add(choiceDateFormat_label_, g_flagsH);
    flex_sizer->Add(m_choiceDateFormat, g_flagsH);

    // File Name --------------------------------------------
    toFileCheckBox_ = new wxCheckBox(main_tab, wxID_ANY, _("Write to File")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    toFileCheckBox_->SetValue(true);
    file_name_label_ = new wxStaticText(main_tab, wxID_ANY, _("File Name:"));
    button_search_ = new wxButton(main_tab, wxID_SAVE, _("File"));
    button_search_->Connect(wxID_SAVE, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFExportDialog::OnFileSearch), nullptr, this);

    m_text_ctrl_ = new wxTextCtrl(main_tab, wxID_FILE, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    m_text_ctrl_->Connect(wxID_FILE, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(mmQIFExportDialog::OnFileNameChanged), nullptr, this);
    m_text_ctrl_->Connect(wxID_FILE, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmQIFExportDialog::OnFileNameEntered), nullptr, this);

    flex_sizer->Add(toFileCheckBox_, g_flagsH);
    flex_sizer->AddSpacer(1);
    flex_sizer->Add(file_name_label_, g_flagsH);
    flex_sizer->Add(button_search_, g_flagsH);
    tab1_sizer->Add(m_text_ctrl_, 0, wxALL|wxGROW, border);

    //Log viewer
    log_field_ = new wxTextCtrl(log_tab, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxHSCROLL);
    tab2_sizer->Add(log_field_, 1, wxGROW|wxALL, border);

    wxButton* itemClearButton = new wxButton(log_tab, wxID_CLEAR, _("Clear"));
    tab2_sizer->Add(itemClearButton, wxSizerFlags(g_flagsV).Center());
    itemClearButton->Connect(wxID_CLEAR, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFExportDialog::OnButtonClear), nullptr, this);

    /**********************************************************************************************
     Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    main_sizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 0));

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxButton* itemButtonOK = new wxButton(buttons_panel, wxID_OK, _("&OK "));
    wxButton* itemButtonCancel_ = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    itemButtonOK->Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFExportDialog::OnOk), nullptr, this);

    buttons_sizer->Add(itemButtonOK, g_flagsH);
    buttons_sizer->Add(itemButtonCancel_, g_flagsH);

    buttons_sizer->Realize();
}

void mmQIFExportDialog::OnButtonClear(wxCommandEvent& WXUNUSED(event))
{
    log_field_->Clear();
}

void mmQIFExportDialog::OnAccountsButton(wxCommandEvent& WXUNUSED(event))
{
    selected_accounts_id_.clear();
    bSelectedAccounts_->UnsetToolTip();
    mmMultiChoiceDialog s_acc(this, _("Choose Account to Export from:")
        , _("QIF Export"), m_accounts_name);


    wxString baloon = "";
    wxArrayInt selected_items;
    if (s_acc.ShowModal() == wxID_OK)
    {
        selected_items = s_acc.GetSelections();
        for (const auto &entry : selected_items)
        {
            int index = entry;
            const wxString accounts_name = m_accounts_name[index];
            const auto account = Model_Account::instance().get(accounts_name);
            if (account) selected_accounts_id_.Add(account->ACCOUNTID);
            baloon += accounts_name + "\n";
        }
    }
    *log_field_ << baloon;

    if (selected_accounts_id_.GetCount() == 0)
    {
        fillControls();
    }
    else if (selected_accounts_id_.GetCount() == 1)
    {
        int account_id = accounts_id_[selected_items[0]];
        const Model_Account::Data* account = Model_Account::instance().get(account_id);
        if (account) bSelectedAccounts_->SetLabelText(account->ACCOUNTNAME);
    }
    else if (selected_accounts_id_.GetCount() > 1)
    {
        bSelectedAccounts_->SetLabelText("...");
        mmToolTip(bSelectedAccounts_, baloon);
    }

}

void mmQIFExportDialog::OnFileSearch(wxCommandEvent& WXUNUSED(event))
{
    wxString fileName = m_text_ctrl_->GetValue();

    switch (m_type)
    {
    case QIF:
        fileName = wxFileSelector(_("Choose QIF data file to Export")
            , wxEmptyString, fileName, wxEmptyString
            , _("QIF Files (*.qif)") + "|*.qif;*.QIF"
            , wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (!fileName.IsEmpty())
            correctEmptyFileExt("qif", fileName);
        break;
    case JSON:
        fileName = wxFileSelector(_("Choose JSON data file to Export")
            , wxEmptyString, fileName, wxEmptyString
            , _("JSON Files (*.json)") + "|*.json;*.JSON"
            , wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (!fileName.IsEmpty())
            correctEmptyFileExt("json", fileName);
        break;
    case CSV:
        fileName = wxFileSelector(_("Choose QIF data file to Export")
            , wxEmptyString, fileName, wxEmptyString
            , _("CSV Files (*.csv)") + "|*.csv;*.CSV"
            , wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (!fileName.IsEmpty())
            correctEmptyFileExt("csv", fileName);
        break;
    }

    m_text_ctrl_->SetValue(fileName);
}

void mmQIFExportDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    if (toFileCheckBox_->IsChecked() && m_text_ctrl_->IsEmpty())
        return mmErrorDialogs::InvalidFile(m_text_ctrl_);

    bool bCorrect = false;
    wxString sErrorMsg = "";
    if (Model_Account::instance().all().empty() && accountsCheckBox_->IsChecked())
        sErrorMsg =_("No Account available for export");
    else if (selected_accounts_id_.Count() < 1 && accountsCheckBox_->IsChecked())
        sErrorMsg =_("No Accounts selected for export");
    else if (dateToCheckBox_->IsChecked() && dateFromCheckBox_->IsChecked() && fromDateCtrl_->GetValue() > toDateCtrl_->GetValue())
        sErrorMsg =_("To Date less than From Date");
    else
        bCorrect = true;

    if (bCorrect)
        mmExportQIF();
    else
        wxMessageBox(sErrorMsg, _("QIF Export"), wxOK|wxICON_WARNING);
}

void mmQIFExportDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void mmQIFExportDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void mmQIFExportDialog::OnChoiceType(wxCommandEvent& event)
{
    m_type = event.GetInt();
    if (m_type < QIF || m_type > CSV) m_type = QIF;
}

void mmQIFExportDialog::OnCheckboxClick( wxCommandEvent& WXUNUSED(event) )
{
    bSelectedAccounts_->Enable(accountsCheckBox_->IsChecked());
    dateFromCheckBox_->Enable(accountsCheckBox_->IsChecked());
    dateToCheckBox_->Enable(accountsCheckBox_->IsChecked());
    fromDateCtrl_->Enable(dateFromCheckBox_->IsEnabled() && dateFromCheckBox_->IsChecked());
    toDateCtrl_->Enable(dateToCheckBox_->IsEnabled() && dateToCheckBox_->IsChecked());
    button_search_->Enable(toFileCheckBox_->IsChecked());
    file_name_label_->Enable(toFileCheckBox_->IsChecked());
    m_text_ctrl_->Enable(toFileCheckBox_->IsChecked());
    choiceDateFormat_label_->Enable(accountsCheckBox_->IsChecked());
    m_choiceDateFormat->Enable(accountsCheckBox_->IsChecked());
}

void mmQIFExportDialog::OnFileNameChanged(wxCommandEvent& event)
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

}
void mmQIFExportDialog::OnFileNameEntered(wxCommandEvent& event)
{
    wxString file_name = m_text_ctrl_->GetValue();
    file_name.Trim();

    event.Skip();
    wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, wxID_SAVE);
    this->GetEventHandler()->AddPendingEvent(evt);
}

void mmQIFExportDialog::mmExportQIF()
{
    bool write_to_file = toFileCheckBox_->IsChecked();
    wxString fileName = m_text_ctrl_->GetValue();

    bool exp_categ = cCategs_->IsChecked();
    bool exp_transactions = (accountsCheckBox_->IsChecked() && selected_accounts_id_.GetCount() > 0);

    const wxString delimiter = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);

    wxString sErrorMsg;
    size_t numRecords = 0, numCategories = 0;

    wxStringClientData* data_obj = static_cast<wxStringClientData*>(m_choiceDateFormat->GetClientObject(m_choiceDateFormat->GetSelection()));
    const wxString dateMask = data_obj->GetData();

    wxString buffer;
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();

    //Export categories
    if (m_type == QIF && exp_categ)
    {
        buffer << mmExportTransaction::getCategoriesQIF();
        numCategories = Model_Category::instance().all_categories().size();
        sErrorMsg << _("Categories exported") << "\n";
    }
    else if (m_type == JSON)
    {
        if (exp_categ) {
            mmExportTransaction::getCategoriesJSON(json_writer);
            numCategories = Model_Category::instance().all_categories().size();
        }
        else {
            mmExportTransaction::getUsedCategoriesJSON(json_writer);
        }
        sErrorMsg << _("Categories exported") << "\n";
    }

    std::unordered_map <int /*account ID*/, wxString> allAccounts4Export;
    wxArrayInt allPayees4Export;
    const wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    wxArrayInt allAttachments4Export;
    wxArrayInt allCustomFields4Export;

    const auto transactions = Model_Checking::instance().find(
        Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL));

    if (exp_transactions && !transactions.empty())
    {
        json_writer.Key("transactions");
        json_writer.StartArray();

        /* Array to store QIF tarts for selected accounts */
        std::unordered_map <int /*account ID*/, wxString> extraTransfers;

        wxProgressDialog progressDlg(_("Please wait"), _("Exporting")
            , 100, this, wxPD_APP_MODAL | wxPD_CAN_ABORT);

        const auto splits = Model_Splittransaction::instance().get_all();

        const wxString begin_date = fromDateCtrl_->GetValue().FormatISODate();
        const wxString end_date = toDateCtrl_->GetValue().FormatISODate();

        for (const auto& transaction : transactions)
        {
            //Filtering
            if (dateFromCheckBox_->IsChecked() && transaction.TRANSDATE < begin_date)
                continue;
            if (dateToCheckBox_->IsChecked() && transaction.TRANSDATE > end_date)
                continue;
            if (!Model_Checking::is_transfer(transaction.TRANSCODE)
                && (selected_accounts_id_.Index(transaction.ACCOUNTID) == wxNOT_FOUND))
                continue;
            if (Model_Checking::is_transfer(transaction.TRANSCODE)
                && (selected_accounts_id_.Index(transaction.ACCOUNTID) == wxNOT_FOUND)
                && (selected_accounts_id_.Index(transaction.TOACCOUNTID) == wxNOT_FOUND))
                continue;
            //

            // if Cancel clicked
            if (!progressDlg.Pulse(wxString::Format(_("Exporting transaction %zu"), ++numRecords)))
                break; // abort processing

            bool is_reverce = false;
            wxString trx_str;
            Model_Checking::Full_Data full_tran(transaction, splits);
            int account_id = transaction.ACCOUNTID;

            switch (m_type)
            {
            case JSON:
                mmExportTransaction::getTransactionJSON(json_writer, full_tran);
                allAccounts4Export[account_id] = "";
                if (allPayees4Export.Index(full_tran.PAYEEID) == wxNOT_FOUND
                    && full_tran.TRANSCODE != Model_Checking::all_type()[Model_Checking::TRANSFER]) {
                    allPayees4Export.Add(full_tran.PAYEEID);
                }

                if (!Model_Attachment::instance().FilterAttachments(RefType, full_tran.id()).empty()
                    && allAttachments4Export.Index(full_tran.TRANSID) == wxNOT_FOUND) {
                    allAttachments4Export.Add(full_tran.TRANSID);
                }

                for (const auto & entry : Model_CustomFieldData::instance().find(Model_CustomFieldData::REFID(full_tran.id())))
                {
                    if (allCustomFields4Export.Index(entry.FIELDATADID) == wxNOT_FOUND) {
                        allCustomFields4Export.Add(entry.FIELDATADID);
                    }
                }
                break;

            case QIF:

                if (Model_Checking::is_transfer(transaction.TRANSCODE))
                {
                    if (selected_accounts_id_.Index(transaction.ACCOUNTID) == wxNOT_FOUND) {
                        is_reverce = true;
                        account_id = transaction.TOACCOUNTID;
                    }

                    if (transaction.TRANSAMOUNT != transaction.TOTRANSAMOUNT) {
                        const auto trx2_str = mmExportTransaction::getTransactionQIF(full_tran, dateMask, !is_reverce);
                        extraTransfers[is_reverce ? transaction.ACCOUNTID : transaction.TOACCOUNTID] += trx2_str;
                    }
                }

                trx_str = mmExportTransaction::getTransactionQIF(full_tran, dateMask, is_reverce);
                allAccounts4Export[account_id] += trx_str;
                break;

            case CSV:

                if (Model_Checking::is_transfer(transaction.TRANSCODE))
                {
                    if (selected_accounts_id_.Index(transaction.ACCOUNTID) == wxNOT_FOUND) {
                        is_reverce = true;
                        account_id = transaction.TOACCOUNTID;
                    }
                    const auto trx2_str = mmExportTransaction::getTransactionCSV(full_tran, dateMask, !is_reverce);
                    extraTransfers[is_reverce ? transaction.ACCOUNTID : transaction.TOACCOUNTID] += trx2_str;
                }

                trx_str = mmExportTransaction::getTransactionCSV(full_tran, dateMask, is_reverce);
                allAccounts4Export[account_id] += trx_str;
                break;
            }
        }
        json_writer.EndArray();

        switch (m_type)
        {
        case QIF:
            //Export accounts
            for (const auto &entry : allAccounts4Export) {
                buffer << mmExportTransaction::getAccountHeaderQIF(entry.first);
                buffer << entry.second;
            }

            //Append extra transters
            for (const auto &entry : extraTransfers) {
                buffer << mmExportTransaction::getAccountHeaderQIF(entry.first);
                buffer << entry.second;
            }
            break;

        case JSON:
            mmExportTransaction::getAccountsJSON(json_writer, allAccounts4Export);
            mmExportTransaction::getPayeesJSON(json_writer, allPayees4Export);
            mmExportTransaction::getAttachmentsJSON(json_writer, allAttachments4Export);
            mmExportTransaction::getCustomFieldsJSON(json_writer, allCustomFields4Export);
            break;

        case CSV:
            buffer
                << _("ID") << delimiter
                << _("Date") << delimiter
                << _("Status") << delimiter
                << _("Type") << delimiter
                << _("Account") << delimiter
                << _("Payee") << delimiter
                << _("Category") << delimiter
                << _("Amount") << delimiter
                << _("Currency") << delimiter
                << _("Number") << delimiter
                << _("Notes")
                << "\n";

            //Export accounts
            for (const auto &entry : allAccounts4Export) {
                buffer << entry.second;
            }

            //Append extra transters
            for (const auto &entry : extraTransfers) {
                buffer << entry.second;
            }
            break;
        }
    }
    json_writer.EndObject();

    switch (m_type)
    {
    case JSON:
        buffer << wxString::FromUTF8(json_buffer.GetString()); break;
    case QIF:
        break;
    }

    if (write_to_file)
    {
        wxFileOutputStream output(fileName);
        wxTextOutputStream text(output);
        text << buffer;
        output.Close();
        if (numCategories || numRecords || allAccounts4Export.size())
            m_text_ctrl_->Clear();
    }
    else {
        *log_field_ << buffer;
    }

    wxString msg = "";
    if (numCategories > 0) {
        msg += wxString::Format(_("Number of categories exported: %zu \n"), numCategories);
    }
    msg += wxString::Format(_("Number of transactions exported: %zu \n"), numRecords);
    msg += wxString::Format(_("Number of accounts exported: %zu"), allAccounts4Export.size());

    wxMessageDialog msgDlg(this, msg, _("Export to QIF"), wxOK | wxICON_INFORMATION);

    msgDlg.ShowModal();
}
