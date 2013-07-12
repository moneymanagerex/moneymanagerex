/*******************************************************
 ********************************************************/
#include "constants.h"
#include "qif_export.h"
#include "util.h"
#include "paths.h"
#include "export.h"
#include "model/Model_Infotable.h"

IMPLEMENT_DYNAMIC_CLASS( mmQIFExportDialog, wxDialog )

BEGIN_EVENT_TABLE( mmQIFExportDialog, wxDialog )
    EVT_CHECKBOX(wxID_ANY, mmQIFExportDialog::OnCheckboxClick )
    EVT_BUTTON(wxID_OK, mmQIFExportDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmQIFExportDialog::OnCancel)
    EVT_CLOSE(mmQIFExportDialog::OnQuit)
END_EVENT_TABLE()

mmQIFExportDialog::mmQIFExportDialog(
    mmCoreDB* core,
    wxWindow* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos,
    const wxSize& size, long style
) :
    core_(core),
    parent_(parent)
{
    Create(parent, id, caption, pos, size, style);
}

bool mmQIFExportDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption,
                           const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    SetIcon(mmex::getProgramIcon());
    Centre();
    Fit();

    fillControls();

    return TRUE;
}

void mmQIFExportDialog::fillControls()
{
    wxArrayString accounts_type;
    accounts_type.Add(ACCOUNT_TYPE_BANK);
    accounts_type.Add(ACCOUNT_TYPE_TERM);
    accounts_id_ = core_->accountList_.getAccountsID(accounts_type);
    bSelectedAccounts_->SetLabel(_("All"));
    bSelectedAccounts_->SetToolTip(_("All"));

    for (const auto &entry : accounts_id_)
    {
        accounts_name_.Add(core_->accountList_.GetAccountName(entry));
        items_index_.Add(entry);
    }

    // redirect logs to text control
    //logger_ = wxLog::SetActiveTarget(new wxLogTextCtrl(log_field_));
    //wxLogMessage( "This is the log window" );
}

void mmQIFExportDialog::CreateControls()
{
    int border = 5;
    int fieldWidth = 180;
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, border);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, border).Expand().Proportion(1);

    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(main_sizer);
    wxBoxSizer* box_sizer1 = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(box_sizer1, flagsExpand);

    wxNotebook* export_notebook = new wxNotebook(this,
        wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE );
    wxPanel* main_tab = new wxPanel(export_notebook, wxID_ANY);
    export_notebook->AddPage(main_tab, _("Parameters"));
    wxBoxSizer *tab1_sizer = new wxBoxSizer(wxVERTICAL);
    main_tab->SetSizer(tab1_sizer);

    wxPanel* log_tab = new wxPanel(export_notebook, wxID_ANY);
    export_notebook->AddPage(log_tab, _("Log"));
    wxBoxSizer *tab2_sizer = new wxBoxSizer(wxVERTICAL);
    log_tab->SetSizer(tab2_sizer);

    box_sizer1->Add(export_notebook, flagsExpand);

    //
    wxString choices[] = { _("QIF"), _("CSV")};
    int num = sizeof(choices) / sizeof(wxString);
    m_radio_box_ = new wxRadioBox(main_tab, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, num, choices, 2, wxRA_SPECIFY_COLS);
    tab1_sizer->Add(m_radio_box_, flags.Center());

    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    tab1_sizer->Add(flex_sizer, flags.Left());

    // Categories -------------------------------------------------
    cCategs_ = new wxCheckBox(main_tab, wxID_ANY,
        _("Categories"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    cCategs_->SetValue(FALSE);
    flex_sizer->Add(cCategs_, flags);
    flex_sizer->AddSpacer(1);

    // Accounts --------------------------------------------
    accountsCheckBox_ = new wxCheckBox( main_tab, wxID_ANY, _("Accounts")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    bSelectedAccounts_ = new wxButton(main_tab, wxID_STATIC, _("All")
        , wxDefaultPosition, wxSize(fieldWidth,-1));
    bSelectedAccounts_ -> Connect(wxID_ANY,
        wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmQIFExportDialog::OnAccountsButton), NULL, this);
    accountsCheckBox_->SetValue(true);
    flex_sizer->Add(accountsCheckBox_, flags);
    flex_sizer->Add(bSelectedAccounts_, flags);

    // From Date --------------------------------------------
    dateFromCheckBox_ = new wxCheckBox( main_tab, wxID_ANY, _("From Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    fromDateCtrl_ = new wxDatePickerCtrl( main_tab, wxID_STATIC, wxDefaultDateTime
        , wxDefaultPosition, wxSize(fieldWidth,-1), wxDP_DROPDOWN);
    fromDateCtrl_->Enable(false);
    flex_sizer->Add(dateFromCheckBox_, flags);
    flex_sizer->Add(fromDateCtrl_, flags);

    // To Date --------------------------------------------
    dateToCheckBox_ = new wxCheckBox( main_tab, wxID_ANY, _("To Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    toDateCtrl_ = new wxDatePickerCtrl( main_tab, wxID_STATIC, wxDefaultDateTime
        , wxDefaultPosition, wxSize(fieldWidth,-1), wxDP_DROPDOWN);
    toDateCtrl_->Enable(false);
    flex_sizer->Add(dateToCheckBox_, flags);
    flex_sizer->Add(toDateCtrl_, flags);

    // Encoding --------------------------------------------

    // File Name --------------------------------------------
    toFileCheckBox_ = new wxCheckBox( main_tab, wxID_ANY, _("Write to File")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    toFileCheckBox_->SetValue(true);
    file_name_label_ = new wxStaticText(main_tab, wxID_ANY, _("File Name:"));
    button_search_ = new wxButton(main_tab, wxID_SAVE, _("Choose &file"));
    button_search_->Connect(wxID_SAVE, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFExportDialog::OnFileSearch), NULL, this);

    m_text_ctrl_ = new wxTextCtrl(main_tab, wxID_FILE, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    m_text_ctrl_->Connect(wxID_FILE, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(mmQIFExportDialog::OnFileNameChanged), NULL, this);
    m_text_ctrl_->Connect(wxID_FILE, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmQIFExportDialog::OnFileNameEntered), NULL, this);

    flex_sizer->Add(toFileCheckBox_, flags);
    flex_sizer->AddSpacer(1);
    flex_sizer->Add(file_name_label_, flags);
    flex_sizer->Add(button_search_, flags);
    tab1_sizer->Add(m_text_ctrl_, 0, wxALL|wxGROW, border);

    //Log viewer
    log_field_ = new wxTextCtrl( log_tab, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxHSCROLL );
    tab2_sizer->Add(log_field_, 1, wxGROW|wxALL, border);

    wxButton* itemClearButton = new wxButton(log_tab, wxID_CLEAR, _("Clear"));
    tab2_sizer->Add(itemClearButton, flags.Center());
    itemClearButton->Connect(wxID_CLEAR, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFExportDialog::OnButtonClear), NULL, this);

    /**********************************************************************************************
     Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    main_sizer->Add(buttons_panel, flags.Center().Border(wxALL, 0));

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxButton* itemButtonOK = new wxButton( buttons_panel, wxID_OK, _("&OK"));
    wxButton* itemButtonCancel_ = new wxButton( buttons_panel, wxID_CANCEL, _("&Cancel"));
    itemButtonOK->Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFExportDialog::OnOk), NULL, this);

    buttons_sizer->Add(itemButtonOK, flags.Border(wxBOTTOM|wxRIGHT, 10));
    buttons_sizer->Add(itemButtonCancel_, flags);

    buttons_sizer->Realize();

}

void mmQIFExportDialog::OnButtonClear(wxCommandEvent& /*event*/)
{
    log_field_->Clear();
}

void mmQIFExportDialog::OnAccountsButton(wxCommandEvent& /*event*/)
{
    items_index_.clear();
    bSelectedAccounts_->UnsetToolTip();
    wxMultiChoiceDialog s_acc(this, _("Choose Account to Export from:")
        , _("QIF Export"), accounts_name_);

    wxString baloon = "";
    wxArrayInt selected_items;
    if (s_acc.ShowModal() == wxID_OK)
    {
        selected_items = s_acc.GetSelections();
        for (const auto &entry : selected_items)
        {
            int index = entry;
            const wxString accounts_name = accounts_name_[index];
            int account_id =  core_->accountList_.GetAccountId(accounts_name);
            items_index_.Add(account_id);
            baloon += accounts_name + "\n";
        }
    }
    *log_field_ << baloon;

    if (items_index_.GetCount() == 0)
    {
        fillControls();
    }
    else if (items_index_.GetCount() == 1)
    {
        int account_id = accounts_id_[selected_items[0]];
        const wxString account_name = core_->accountList_.GetAccountName(account_id);
        bSelectedAccounts_->SetLabel(account_name);
    }
    else if (items_index_.GetCount() > 1)
    {
        bSelectedAccounts_->SetLabel("...");
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
    bool bCorrect = false;
    wxString sErrorMsg = "";
    if (core_->accountList_.getNumAccounts() == 0 && accountsCheckBox_->GetValue())
    {
        sErrorMsg =_("No Account available for export");
    }
    else if (items_index_.Count() < 1 && accountsCheckBox_->GetValue())
    {
        sErrorMsg =_("No Accounts selected for export");
    }
    else if (toFileCheckBox_->GetValue() && m_text_ctrl_->GetValue().IsEmpty())
    {
        sErrorMsg =_("File name is empty");
    }
    else if (dateToCheckBox_->GetValue() && dateFromCheckBox_->GetValue() && fromDateCtrl_->GetValue() > toDateCtrl_->GetValue())
    {
        sErrorMsg =_("To Date less than From Date");
    }
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
    const bool qif_csv = m_radio_box_->GetSelection() == 0;
    const bool exp_categ = cCategs_->GetValue();
    const bool exp_transactions = (accountsCheckBox_->GetValue() && items_index_.GetCount()>0);
    const bool write_to_file = toFileCheckBox_->GetValue();
    wxString sErrorMsg;
    wxString buffer;
    long numRecords = 0;

    //Export categories
    if (exp_categ)
    {
        mmExportTransaction categories(core_);
        if (qif_csv)
            buffer << categories.getCategoriesQIF();
         else
            buffer << categories.getCategoriesCSV();
        sErrorMsg << _("Categories exported") << "\n";
    }

    //Export transactions
    numRecords = 0;
    wxArrayInt transferTrxId;

    if (exp_transactions)
    {
        for (const auto &entry : items_index_)
        {
            mmExportTransaction header(core_, entry);
            if (qif_csv)
                buffer << header.getAccountHeaderQIF();

            double account_balance = 0.0, reconciled_balance = 0.0;
            core_->bTransactionList_.LoadAccountTransactions(entry, account_balance, reconciled_balance);

            for (const auto& transaction : core_->bTransactionList_.accountTransactions_)
            {
                //Filtering
                if (dateFromCheckBox_->IsChecked() && transaction->date_ < fromDateCtrl_->GetValue() )
                    continue;
                if (dateToCheckBox_->IsChecked() && transaction->date_ > toDateCtrl_->GetValue() )
                    continue;

                if (transferTrxId.Index(transaction->transactionID()) == wxNOT_FOUND)
                    numRecords++;

                if (transaction->transType_ == TRANS_TYPE_TRANSFER_STR)
                {
                    if (items_index_.Index(transaction->toAccountID_) == wxNOT_FOUND)
                        transferTrxId.Add(transaction->transactionID());
                }

                mmExportTransaction data(core_, transaction);
                if (qif_csv)
                    buffer << data.getTransactionQIF();
                else
                    buffer << data.getTransactionCSV();
            }
        }
    }

    if (write_to_file)
    {
        wxString fileName = m_text_ctrl_->GetValue();
        wxFileOutputStream output(fileName);
        wxTextOutputStream text(output);
        text << buffer;
    }

    *log_field_ << buffer;
    sErrorMsg << wxTRANSLATE("Number of transactions exported: %ld");
    const wxString msg = wxString::Format(sErrorMsg, numRecords);
    wxMessageDialog msgDlg(parent_, wxGetTranslation(msg)
        , _("Export to QIF"), wxOK|wxICON_INFORMATION);

    //FIXME: Can't close this dialog
    msgDlg.ShowModal();

}
