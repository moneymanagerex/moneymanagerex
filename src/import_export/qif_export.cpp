/*******************************************************
Copyright (C) 2006-2012

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
#include "model/Model_Infotable.h"
#include "model/Model_Account.h"

wxIMPLEMENT_DYNAMIC_CLASS(mmQIFExportDialog, wxDialog);

wxBEGIN_EVENT_TABLE( mmQIFExportDialog, wxDialog)
    EVT_CHECKBOX(wxID_ANY, mmQIFExportDialog::OnCheckboxClick )
    EVT_BUTTON(wxID_OK, mmQIFExportDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmQIFExportDialog::OnCancel)
    EVT_CLOSE(mmQIFExportDialog::OnQuit)
wxEND_EVENT_TABLE()

mmQIFExportDialog::mmQIFExportDialog(wxWindow* parent)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("QIF Export"), wxDefaultPosition, wxSize(500, 300), style);
}

bool mmQIFExportDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
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
    bSelectedAccounts_->SetToolTip(_("All"));

    for (const auto& a : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        if (Model_Account::type(a) != Model_Account::INVESTMENT)
        {
            accounts_name_.Add(a.ACCOUNTNAME);
            selected_accounts_id_.Add(a.ACCOUNTID);
            accounts_id_.Add(a.ACCOUNTID);
        }
    }

    // redirect logs to text control
    //logger_ = wxLog::SetActiveTarget(new wxLogTextCtrl(log_field_));
    //wxLogMessage( "This is the log window" );
}

void mmQIFExportDialog::CreateControls()
{
    int border = 5;
    int fieldWidth = 180;

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

    //
    wxString choices[] = { _("QIF"), _("CSV")};
    int num = sizeof(choices) / sizeof(wxString);
    m_radio_box_ = new wxRadioBox(main_tab, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, num, choices, 2, wxRA_SPECIFY_COLS);
    tab1_sizer->Add(m_radio_box_, wxSizerFlags(g_flagsV).Center());

    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    tab1_sizer->Add(flex_sizer, wxSizerFlags(g_flagsV).Left());

    // Categories -------------------------------------------------
    cCategs_ = new wxCheckBox(main_tab, wxID_ANY
        , _("Categories"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cCategs_->SetValue(FALSE);
    flex_sizer->Add(cCategs_, g_flagsH);
    flex_sizer->AddSpacer(1);

    // Accounts --------------------------------------------
    accountsCheckBox_ = new wxCheckBox( main_tab, wxID_ANY, _("Accounts")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    bSelectedAccounts_ = new wxButton(main_tab, wxID_STATIC, _("All")
        , wxDefaultPosition, wxSize(fieldWidth,-1));
    bSelectedAccounts_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFExportDialog::OnAccountsButton), nullptr, this);
    accountsCheckBox_->SetValue(true);
    flex_sizer->Add(accountsCheckBox_, g_flagsH);
    flex_sizer->Add(bSelectedAccounts_, g_flagsH);

    // From Date --------------------------------------------
    dateFromCheckBox_ = new wxCheckBox(main_tab, wxID_ANY, _("From Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    fromDateCtrl_ = new wxDatePickerCtrl(main_tab, wxID_STATIC, wxDefaultDateTime
        , wxDefaultPosition, wxSize(fieldWidth,-1), wxDP_DROPDOWN);
    fromDateCtrl_->Enable(false);
    flex_sizer->Add(dateFromCheckBox_, g_flagsH);
    flex_sizer->Add(fromDateCtrl_, g_flagsH);

    // To Date --------------------------------------------
    dateToCheckBox_ = new wxCheckBox(main_tab, wxID_ANY, _("To Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    toDateCtrl_ = new wxDatePickerCtrl(main_tab, wxID_STATIC, wxDefaultDateTime
        , wxDefaultPosition, wxSize(fieldWidth,-1), wxDP_DROPDOWN);
    toDateCtrl_->Enable(false);
    flex_sizer->Add(dateToCheckBox_, g_flagsH);
    flex_sizer->Add(toDateCtrl_, g_flagsH);

    // Encoding --------------------------------------------

    // Date Format Settings --------------------------------
    wxString dateFormatStr = Option::instance().DateFormat();

    wxStaticText* dateFormat = new wxStaticText(main_tab, wxID_STATIC, _("Date Format"));
    m_choiceDateFormat = new wxComboBox(main_tab, wxID_ANY);
    for (const auto& i : g_date_formats_map())
    {
        m_choiceDateFormat->Append(i.second, new wxStringClientData(i.first));
        if (dateFormatStr == i.first) m_choiceDateFormat->SetStringSelection(i.second);
    }

    flex_sizer->Add(dateFormat, g_flagsH);
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

void mmQIFExportDialog::OnButtonClear(wxCommandEvent& /*event*/)
{
    log_field_->Clear();
}

void mmQIFExportDialog::OnAccountsButton(wxCommandEvent& /*event*/)
{
    selected_accounts_id_.clear();
    bSelectedAccounts_->UnsetToolTip();
    wxMultiChoiceDialog s_acc(this, _("Choose Account to Export from:")
        , _("QIF Export"), accounts_name_);
    wxButton* ok = static_cast<wxButton*>(s_acc.FindWindow(wxID_OK));
    if (ok) ok->SetLabel(_("&OK "));
    wxButton* ca = static_cast<wxButton*>(s_acc.FindWindow(wxID_CANCEL));
    if (ca) ca->SetLabel(wxGetTranslation(g_CancelLabel));

    wxString baloon = "";
    wxArrayInt selected_items;
    if (s_acc.ShowModal() == wxID_OK)
    {
        selected_items = s_acc.GetSelections();
        for (const auto &entry : selected_items)
        {
            int index = entry;
            const wxString accounts_name = accounts_name_[index];
            const Model_Account::Data* account = Model_Account::instance().get(accounts_name);
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
        bSelectedAccounts_->SetToolTip(baloon);
    }

}

void mmQIFExportDialog::OnFileSearch(wxCommandEvent& /*event*/)
{
    wxString fileName = m_text_ctrl_->GetValue();
    const bool qif_csv = m_radio_box_->GetSelection() == 0;

    const wxString choose_ext = qif_csv ? _("QIF Files") : _("CSV Files");
    fileName = wxFileSelector(
        (qif_csv
            ? _("Choose QIF data file to Export")
            : _("Choose CSV data file to Export"))
        , wxEmptyString, fileName, wxEmptyString
        , choose_ext + (qif_csv ? " (*.qif)|*.qif;*.QIF" : " (*.csv)|*.csv;*.CSV")
        , wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (!fileName.IsEmpty())
        correctEmptyFileExt(qif_csv ? "qif":"csv" , fileName);
    m_text_ctrl_->SetValue(fileName);
}

void mmQIFExportDialog::OnOk(wxCommandEvent& /*event*/)
{
    if (toFileCheckBox_->GetValue() && m_text_ctrl_->GetValue().IsEmpty())
        return mmErrorDialogs::InvalidFile(m_text_ctrl_);

    bool bCorrect = false;
    wxString sErrorMsg = "";
    if (Model_Account::instance().all().empty() && accountsCheckBox_->GetValue())
        sErrorMsg =_("No Account available for export");
    else if (selected_accounts_id_.Count() < 1 && accountsCheckBox_->GetValue())
        sErrorMsg =_("No Accounts selected for export");
    else if (dateToCheckBox_->GetValue() && dateFromCheckBox_->GetValue() && fromDateCtrl_->GetValue() > toDateCtrl_->GetValue())
        sErrorMsg =_("To Date less than From Date");
    else
        bCorrect = true;

    if (bCorrect)
        mmExportQIF();
    else
        wxMessageBox(sErrorMsg, _("QIF Export"), wxOK|wxICON_WARNING);
}

void mmQIFExportDialog::OnCancel(wxCommandEvent& /*event*/)
{

    EndModal(wxID_CANCEL);
}

void mmQIFExportDialog::OnQuit(wxCloseEvent& /*event*/)
{
        EndModal(wxID_CANCEL);
}
void mmQIFExportDialog::OnCheckboxClick( wxCommandEvent& /*event*/ )
{
    bSelectedAccounts_->Enable(accountsCheckBox_->GetValue());
    if (dateFromCheckBox_->GetValue()) dateFromCheckBox_->SetValue(accountsCheckBox_->GetValue());
    if (dateToCheckBox_->GetValue()) dateToCheckBox_->SetValue(accountsCheckBox_->GetValue());
    fromDateCtrl_->Enable(dateFromCheckBox_->GetValue());
    toDateCtrl_->Enable(dateToCheckBox_->GetValue());
    button_search_->Enable(toFileCheckBox_->GetValue());
    file_name_label_->Enable(toFileCheckBox_->GetValue());
    m_text_ctrl_->Enable(toFileCheckBox_->GetValue());

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
    bool qif_csv = m_radio_box_->GetSelection() == 0;
    bool exp_categ = cCategs_->GetValue();
    bool exp_transactions = (accountsCheckBox_->GetValue() && selected_accounts_id_.GetCount() > 0);
    bool write_to_file = toFileCheckBox_->GetValue();
    wxString sErrorMsg;
    wxString buffer;
    int numRecords = 0;

    wxStringClientData* data_obj = (wxStringClientData*)m_choiceDateFormat->GetClientObject(m_choiceDateFormat->GetSelection());
    const wxString dateMask = data_obj->GetData();

    //Export categories
    if (exp_categ)
    {
        if (qif_csv)
            buffer << mmExportTransaction::getCategoriesQIF();
         else
             buffer << mmExportTransaction::getCategoriesCSV();
        sErrorMsg << _("Categories exported") << "\n";
    }

    if (exp_transactions)
    {
        /* Array with transfer transactions */
        std::map <int, wxString> transferTransactions;

        wxProgressDialog progressDlg(_("Please wait"), _("Exporting")
            , 100, this, wxPD_APP_MODAL | wxPD_CAN_ABORT);

        const auto splits = Model_Splittransaction::instance().get_all();
        for (const auto &account_id : selected_accounts_id_)
        {
            if (qif_csv)
            {
                buffer << mmExportTransaction::getAccountHeaderQIF(account_id);
            }

            const wxString begin_date = fromDateCtrl_->GetValue().FormatISODate();
            const wxString end_date = toDateCtrl_->GetValue().FormatISODate();
            for (const auto& transaction : Model_Checking::instance().find_or(Model_Checking::ACCOUNTID(account_id)
                , Model_Checking::TOACCOUNTID(account_id)))
            {
                //Filtering
                if (dateFromCheckBox_->IsChecked() && transaction.TRANSDATE < begin_date)
                    continue;
                if (dateToCheckBox_->IsChecked() && transaction.TRANSDATE > end_date)
                    continue;

                if (!progressDlg.Pulse(wxString::Format(_("Exporting transaction %i"), ++numRecords))) // if cancel clicked
                    break; // abort processing

                Model_Checking::Full_Data full_tran(transaction, splits);
                if (qif_csv)
                    buffer << mmExportTransaction::getTransactionQIF(full_tran, account_id, dateMask);
                else
                    buffer << mmExportTransaction::getTransactionCSV(full_tran, account_id, dateMask);

                if (Model_Checking::type(transaction) == Model_Checking::TRANSFER)
                {
                    int index = transaction.ACCOUNTID == account_id ? transaction.TOACCOUNTID : transaction.ACCOUNTID;
                    if (transaction.ACCOUNTID != account_id) numRecords--;
                    if (selected_accounts_id_.Index(index) == wxNOT_FOUND)
                    {
                        //get second part of transfer transaction
                        wxString second_part = "";
                        if (qif_csv)
                            second_part = mmExportTransaction::getTransactionQIF(full_tran, account_id, dateMask);
                        else
                            second_part = mmExportTransaction::getTransactionCSV(full_tran, account_id, dateMask);
                        transferTransactions[index] += second_part;
                    }
                }
                //
            }
        }
        //Export second part of transfer transactions
        for (const auto &entry : transferTransactions)
        {
            if (qif_csv)
            {
                buffer << mmExportTransaction::getAccountHeaderQIF(entry.first);
            }
            buffer << entry.second;
        }
    }

    if (write_to_file)
    {
        wxString fileName = m_text_ctrl_->GetValue();
        wxFileOutputStream output(fileName);
        wxTextOutputStream text(output);
        text << buffer;
        output.Close();
    }
    else
        *log_field_ << buffer;

    wxMessageDialog msgDlg(this
        , wxString::Format(_("Number of transactions exported: %ld"), numRecords)
        , _("Export to QIF"), wxOK | wxICON_INFORMATION);

    wxButton* ok = wxStaticCast(FindWindow(wxID_OK), wxButton);
    ok->Disable();

    msgDlg.ShowModal();
}
