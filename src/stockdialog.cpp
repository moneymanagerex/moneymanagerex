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

#include "stockdialog.h"
#include "images_list.h"
#include "attachmentdialog.h"
#include "constants.h"
#include "mmSimpleDialogs.h"
#include "mmTextCtrl.h"
#include "paths.h"
#include "util.h"
#include "validators.h"
#include "accountdialog.h"
#include "sharetransactiondialog.h"
#include "mmframe.h"
#include "Model_StockHistory.h"
#include "Model_Infotable.h"

IMPLEMENT_DYNAMIC_CLASS(mmStockDialog, wxDialog)

wxBEGIN_EVENT_TABLE(mmStockDialog, wxDialog)
EVT_CLOSE(mmStockDialog::OnQuit)
EVT_BUTTON(wxID_SAVE, mmStockDialog::OnSave)
EVT_BUTTON(wxID_CANCEL, mmStockDialog::OnCancel)
EVT_BUTTON(wxID_INDEX, mmStockDialog::OnStockPriceButton)
EVT_BUTTON(wxID_FILE, mmStockDialog::OnAttachments)
EVT_BUTTON(wxID_APPLY, mmStockDialog::OnMagicButton)
EVT_BUTTON(ID_BUTTON_IMPORT, mmStockDialog::OnHistoryImportButton)
EVT_BUTTON(ID_BUTTON_DOWNLOAD, mmStockDialog::OnHistoryDownloadButton)
EVT_CHILD_FOCUS(mmStockDialog::OnFocusChange)
EVT_DATAVIEW_ITEM_EDITING_DONE(wxID_ANY, mmStockDialog::OnListValueEditingDone)
EVT_DATAVIEW_ITEM_VALUE_CHANGED(wxID_ANY, mmStockDialog::OnListValueChanged)
EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, mmStockDialog::OnSelectionChanged)
EVT_BUTTON(wxID_ADD, mmStockDialog::OnHistoryAddUpdateEntry)
EVT_MENU_RANGE(MENU_NEW_ENTRY, MENU_DELETE_ENTRY, mmStockDialog::OnMenuSelected)
wxEND_EVENT_TABLE()

mmStockDialog::mmStockDialog()
{
}

mmStockDialog::mmStockDialog(wxWindow* parent
    , mmGUIFrame* gui_frame
    , Model_Stock::Data* stock
    , int accountID
    , const wxString& name
)
    : m_stock_name_ctrl(nullptr)
    , m_stock_symbol_ctrl(nullptr)
    , m_purchase_date_ctrl(nullptr)
    , m_num_shares_ctrl(nullptr)
    , m_share_price_ctrl(nullptr)
    , m_share_price_txt(nullptr)
    , m_notes_ctrl(nullptr)
    , m_current_price_ctrl(nullptr)
    , m_value_investment(nullptr)
    , m_commission_ctrl(nullptr)
    , m_bAttachments(nullptr)
    , m_price_listbox(nullptr)
    , m_history_date_ctrl(nullptr)
    , m_exchange_text(nullptr)
    , m_stock(stock)
    , m_edit(stock ? true : false)
    , m_account_id(accountID)
    , m_gui_frame(gui_frame)
#ifdef _DEBUG
    , debug_(true)
#else
    , debug_(false)
#endif
{
    if (m_stock)
        m_account_id = m_stock->HELDAT;

    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;

    m_account = Model_Account::instance().get(m_account_id);
    if (m_account) {
        m_currency = Model_Account::currency(m_account);
    }
    else {
        m_currency = Model_Currency::GetBaseCurrency();
    }
    m_decimal_point = m_currency->DECIMAL_POINT;
    m_precision = Option::instance().getSharePrecision();

    Create(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, style, name);
}

bool mmStockDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    if (m_edit) DataToControls();
    UpdateControls();

    m_current_price_ctrl->Connect(ID_TEXTCTRL_STOCK_CP, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);
    m_commission_ctrl->Connect(ID_TEXTCTRL_STOCK_COMMISSION, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);
    m_share_price_ctrl->Connect(ID_TEXTCTRL_STOCK_PP, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);
    m_exchange_text->Connect(ID_TEXTCTRL_HIST_EXCHRATE, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);

    Centre();
    return TRUE;
}

void mmStockDialog::DataToControls()
{
    if (!this->m_stock) return;

    m_stock_id = m_stock->STOCKID;

    m_stock_name_ctrl->SetValue(m_stock->STOCKNAME);
    m_stock_symbol_ctrl->SetValue(m_stock->SYMBOL);
    m_notes_ctrl->SetValue(m_stock->NOTES);
    m_purchase_date_ctrl->SetValue(Model_Stock::PURCHASEDATE(m_stock));

    int precision = m_stock->NUMSHARES == floor(m_stock->NUMSHARES) ? 0 : m_precision;
    m_num_shares_ctrl->SetValue(m_stock->NUMSHARES, precision);

    int currency_precision = Model_Currency::precision(m_currency);
    if (currency_precision < m_precision) {
        currency_precision = m_precision;
    }
    m_share_price_ctrl->SetValue(m_stock->PURCHASEPRICE, m_account, currency_precision);
    m_current_price_ctrl->SetValue(m_stock->CURRENTPRICE, m_account, currency_precision);
    m_commission_ctrl->SetValue(m_stock->COMMISSION, m_account, currency_precision);

    RefreshStockHistory(m_stock->SYMBOL);
}

void mmStockDialog::UpdateControls()
{
    this->SetTitle(m_edit ? _("Edit Stock Investment") : _("New Stock Investment"));
    Model_Account::Data * account = Model_Account::instance().get(m_account_id);

    double numShares = 0, pPrice = 0, commision = 0;
    m_num_shares_ctrl->GetDouble(numShares);
    m_share_price_ctrl->GetDouble(pPrice);
    m_commission_ctrl->GetDouble(commision);

    //Disable history buttons on new stocks
    wxBitmapButton * buttonDownload = static_cast<wxBitmapButton*>(FindWindow(ID_BUTTON_DOWNLOAD));
    buttonDownload->Enable(m_edit);
    wxBitmapButton * buttonImport = static_cast<wxBitmapButton*>(FindWindow(ID_BUTTON_IMPORT));
    buttonImport->Enable(m_edit);
    wxBitmapButton* magicButton = static_cast<wxBitmapButton*>(FindWindow(wxID_APPLY));
    magicButton->Enable(m_edit);

    bool initial_shares = !Model_Translink::HasShares(m_stock_id);
    m_num_shares_ctrl->Enable(!m_edit || initial_shares);
    m_purchase_date_ctrl->Enable(!m_edit || initial_shares);
    m_share_price_ctrl->Enable(!m_edit || initial_shares);
    m_commission_ctrl->Enable(!m_edit || initial_shares);

    if (m_share_price_ctrl->IsEnabled())
        m_share_price_txt->SetToolTip(_("Initial share price.\nRecorded in share transaction"));
    else
        m_share_price_txt->SetToolTip(_("Invalid in this view.\nView Transactions for values"));

    double currPrice;
    if (!m_edit)
    {
        currPrice = numShares * pPrice - commision;
        m_value_investment->SetLabelText(Model_Account::toCurrency(currPrice, account));

        const wxString symbol = m_stock_symbol_ctrl->GetValue().Upper();
        m_stock_symbol_ctrl->SetValue(symbol);
        RefreshStockHistory(symbol);

        Model_Stock::Data_Set entries = Model_Stock::instance().find(Model_Stock::SYMBOL(symbol));
        for (const auto& entry : entries)
        {
            const auto name = entry.STOCKNAME;
            if (!name.empty()) {
                m_stock_name_ctrl->ChangeValue(name);
                break;
            }
        }

        if (m_object_in_focus == m_current_price_ctrl->GetId()) {
            currPrice = currPrice / numShares;
            m_current_price_ctrl->SetValue(currPrice, m_precision);
        }
    }
    else
    {
        m_current_price_ctrl->GetDouble(currPrice);
        currPrice = numShares * currPrice - commision;
        m_value_investment->SetLabelText(Model_Account::toCurrency(currPrice, account));
    }
}

void mmStockDialog::CreateControls()
{
    bool initial_stock_transaction = true;
    if (m_stock)
    {
        if (!Model_Translink::TranslinkList(Model_Attachment::STOCK, m_stock->STOCKID).empty())
        {
            initial_stock_transaction = false;
        }
    }

    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(mainBoxSizer);

    wxBoxSizer* leftBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(leftBoxSizer, g_flagsH);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, _("Stock Investment Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    leftBoxSizer->Add(itemStaticBoxSizer4, g_flagsExpand);

    wxPanel* itemPanel5 = new wxPanel(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemStaticBoxSizer4->Add(itemPanel5, g_flagsV);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel5->SetSizer(itemFlexGridSizer6);

    //Symbol
    wxStaticText* symbol = new wxStaticText(itemPanel5, wxID_STATIC, _("Stock Symbol"));
    itemFlexGridSizer6->Add(symbol, g_flagsH);
    symbol->SetFont(this->GetFont().Bold());

    m_stock_symbol_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_SYMBOL
        , "", wxDefaultPosition, wxDefaultSize, 0);
    itemFlexGridSizer6->Add(m_stock_symbol_ctrl, g_flagsExpand);

    // Name
    wxStaticText* company_name_txt = new wxStaticText(itemPanel5, wxID_STATIC, _("Company Name"));
    itemFlexGridSizer6->Add(company_name_txt, g_flagsH);
    m_stock_name_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCKNAME, "");
    itemFlexGridSizer6->Add(m_stock_name_ctrl, g_flagsExpand);

    //Date
    wxStaticText * date_txt = new wxStaticText(itemPanel5, wxID_STATIC, _("*Date"));
    itemFlexGridSizer6->Add(date_txt, g_flagsH);
    m_purchase_date_ctrl = new wxDatePickerCtrl(itemPanel5, ID_DPC_STOCK_PDATE
        , wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);

    itemFlexGridSizer6->Add(m_purchase_date_ctrl, g_flagsH);

    //Number of Shares
    wxStaticText * number_txt = new wxStaticText(itemPanel5, wxID_STATIC, _("*Share Number"));
    itemFlexGridSizer6->Add(number_txt, g_flagsH);
    number_txt->SetFont(this->GetFont().Bold());
    m_num_shares_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_NUMBER_SHARES, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(m_num_shares_ctrl, g_flagsExpand);

    //Purchase Price
    m_share_price_txt = new wxStaticText(itemPanel5, wxID_STATIC, _("*Share Price"));
    // m_share_price_txt ToolTip is set in method: mmStockDialog::UpdateControls()
    itemFlexGridSizer6->Add(m_share_price_txt, g_flagsH);
    m_share_price_txt->SetFont(this->GetFont().Bold());
    m_share_price_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_PP, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(m_share_price_ctrl, g_flagsExpand);

    //Commission
    wxStaticText * commission_txt = new wxStaticText(itemPanel5, wxID_STATIC, _("*Commission"));
    itemFlexGridSizer6->Add(commission_txt, g_flagsH);
    m_commission_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_COMMISSION, "0", wxDefaultPosition
        , wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator(), m_currency, "commission_ctrl");
    itemFlexGridSizer6->Add(m_commission_ctrl, g_flagsExpand);

    //Current Price
    wxStaticText * current_price_txt = new wxStaticText(itemPanel5, wxID_STATIC, _("Current Price"));
    itemFlexGridSizer6->Add(current_price_txt, g_flagsH);
    m_current_price_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_CP, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(m_current_price_ctrl, g_flagsExpand);

    //Current Value
    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Current Value")), g_flagsH);
    m_value_investment = new wxStaticText(itemPanel5, ID_STATIC_STOCK_VALUE, "--");
    itemFlexGridSizer6->Add(m_value_investment, g_flagsH);

    //
    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Notes")), g_flagsH);
    wxBoxSizer * iconsSizer = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer6->Add(iconsSizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));
    m_bAttachments = new wxBitmapButton(itemPanel5, wxID_FILE, mmBitmap(png::CLIP));
    wxBitmapButton * itemButton31 = new wxBitmapButton(itemPanel5, wxID_INDEX, mmBitmap(png::WEB));
    iconsSizer->Add(m_bAttachments, g_flagsH);
    iconsSizer->Add(itemButton31, g_flagsH);

    m_notes_ctrl = new mmTextCtrl(this, wxID_STATIC, "", wxDefaultPosition, wxSize(200, 90), wxTE_MULTILINE);
    itemStaticBoxSizer4->Add(m_notes_ctrl, g_flagsExpand);
    itemStaticBoxSizer4->AddSpacer(1);

    leftBoxSizer->AddSpacer(20);

    //History Panel
    wxBoxSizer * rightBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(rightBoxSizer, g_flagsExpand);

    wxStaticBox * historyStaticBox = new wxStaticBox(this, wxID_ANY, _("Stock History Options"));
    wxStaticBoxSizer * historyStaticBoxSizer = new wxStaticBoxSizer(historyStaticBox, wxVERTICAL);
    rightBoxSizer->Add(historyStaticBoxSizer, g_flagsExpand);

    m_price_listbox = new wxDataViewListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_MULTIPLE);
    m_price_listbox->SetMinSize(wxSize(250, 150));
    historyStaticBoxSizer->Add(m_price_listbox, g_flagsExpand);

    if (debug_) m_price_listbox->AppendTextColumn(_("#"), wxDATAVIEW_CELL_INERT, 30);
    m_price_listbox->AppendTextColumn(_("Date"));
    m_price_listbox->AppendTextColumn(_("Price"), wxDATAVIEW_CELL_EDITABLE);
    m_price_listbox->AppendTextColumn("");

    //Popup dialog
    wxPanel * hist_buttons_panel = new wxPanel(this, ID_HIST_PANEL);
    historyStaticBoxSizer->Add(hist_buttons_panel, g_flagsV);

    m_history_date_ctrl = new wxDatePickerCtrl(hist_buttons_panel, wxID_ANY);
    m_exchange_text = new mmTextCtrl(hist_buttons_panel, ID_TEXTCTRL_HIST_EXCHRATE, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    wxBitmapButton * buttonAdd = new wxBitmapButton(hist_buttons_panel, wxID_ADD, mmBitmap(png::SAVE));

    wxBoxSizer * hist_sizer = new wxBoxSizer(wxHORIZONTAL);

    hist_sizer->Add(m_history_date_ctrl, g_flagsH);
    hist_sizer->Add(m_exchange_text, g_flagsExpand);
    hist_sizer->Add(buttonAdd, g_flagsH);

    hist_buttons_panel->SetSizer(hist_sizer);
    hist_buttons_panel->Show(false);

    //History Buttons
    wxPanel * buttons_panel = new wxPanel(this, wxID_ANY);
    historyStaticBoxSizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Centre());
    wxStdDialogButtonSizer * buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxBitmapButton * buttonDownload = new wxBitmapButton(buttons_panel, ID_BUTTON_DOWNLOAD, mmBitmap(png::CURRATES));
    wxBitmapButton * buttonImport = new wxBitmapButton(buttons_panel, ID_BUTTON_IMPORT, mmBitmap(png::IMPORT));
    wxBitmapButton * magicButton = new wxBitmapButton(buttons_panel, wxID_APPLY, mmBitmap(png::RUN));

    buttons_sizer->Add(buttonDownload, g_flagsH);
    buttons_sizer->Add(buttonImport, g_flagsH);
    buttons_sizer->Add(magicButton, g_flagsH);


    //OK & Cancel buttons
    wxStdDialogButtonSizer * buttonsOK_CANCEL_sizer = new wxStdDialogButtonSizer;
    leftBoxSizer->Add(buttonsOK_CANCEL_sizer, wxSizerFlags(g_flagsV).Centre());

    wxButton * itemButtonOK = new wxButton(this, wxID_SAVE, _("&Save "));
    wxButton * itemButton30 = new wxButton(this, wxID_CANCEL, wxGetTranslation(g_CloseLabel));

    if (m_edit) {
        itemButton30->SetFocus();
    }
    buttonsOK_CANCEL_sizer->Add(itemButtonOK, g_flagsH);
    buttonsOK_CANCEL_sizer->Add(itemButton30, g_flagsH);

    m_purchase_date_ctrl->Enable(initial_stock_transaction);
    m_num_shares_ctrl->Enable(initial_stock_transaction);
    m_share_price_ctrl->Enable(initial_stock_transaction);
    m_commission_ctrl->Enable(initial_stock_transaction);

    company_name_txt->SetToolTip(_("Name of the company issuing Shares"));
    m_num_shares_ctrl->SetToolTip(_("Enter the initial number of shares."));
    m_stock_symbol_ctrl->SetToolTip(_("Enter the Company's stock exchange symbol.\n(Optional) Include exchange. eg: IBM.BE"));
    m_stock_name_ctrl->SetToolTip(_("Enter the associated Share Account name\nAn account can be created on Save"));
    date_txt->SetToolTip(_("Initial purchase date of shares.\nRecorded in share transaction"));
    m_purchase_date_ctrl->SetToolTip(_("Specify the date of this initial share purchase"));
    number_txt->SetToolTip(_("Total number of shares.\nRecorded in share transaction"));
    m_share_price_ctrl->SetToolTip(_("Enter the initial price per share."));
    commission_txt->SetToolTip(_("Remainder of funds from share investment.\nRecorded in share transaction"));
    m_commission_ctrl->SetToolTip(_("Enter any commission paid or left over money from the purchase of shares."));
    current_price_txt->SetToolTip(_("Share price after the initial share purchase"));
    m_current_price_ctrl->SetToolTip(_("Enter current share price."));
    m_bAttachments->SetToolTip(_("Organize attachments of this stock"));
    itemButton31->SetToolTip(_("Display the web page for the specified Stock symbol"));
    m_notes_ctrl->SetToolTip(_("Enter notes associated with this investment"));
    m_price_listbox->SetToolTip(_("Stock Price History"));
    buttonDownload->SetToolTip(_("Download Stock Price history"));
    buttonImport->SetToolTip(_("Import Stock Price history (CSV Format)"));
    magicButton->SetToolTip(_("Other tools"));
}

void mmStockDialog::OnQuit(wxCloseEvent & WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    if (!m_edit)
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    EndModal(wxID_CANCEL);
}

void mmStockDialog::OnCancel(wxCommandEvent & WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    if (m_stock_id <= 0)
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    EndModal(wxID_CANCEL);
}

void mmStockDialog::OnAttachments(wxCommandEvent & WXUNUSED(event))
{
    const wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    int RefId = m_stock_id;

    if (RefId < 0)
        RefId = 0;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();
}

void mmStockDialog::OnStockPriceButton(wxCommandEvent & WXUNUSED(event))
{
    const wxString stockSymbol = m_stock_symbol_ctrl->GetValue().Trim();

    if (!stockSymbol.IsEmpty())
    {
        const wxString& stockURL = Model_Infotable::instance().GetStringInfo("STOCKURL", mmex::weblink::DefStockUrl);
        const wxString& httpString = wxString::Format(stockURL, stockSymbol);
        wxLaunchDefaultBrowser(httpString);
    }
}

void mmStockDialog::OnSave(wxCommandEvent & WXUNUSED(event))
{

    Model_Account::Data* account = Model_Account::instance().get(m_account_id);
    if (!account) {
        return mmErrorDialogs::MessageInvalid(this, _("Held At"));
    }

    const wxString& stockSymbol = m_stock_symbol_ctrl->GetValue();
    if (stockSymbol.empty()) {
        return mmErrorDialogs::MessageInvalid(this, _("Symbol"));
    }

    const wxString pdate = m_purchase_date_ctrl->GetValue().FormatISODate();
    const wxString stockName = m_stock_name_ctrl->GetValue();
    const wxString notes = m_notes_ctrl->GetValue();

    double numShares = 0;
    if (!m_num_shares_ctrl->checkValue(numShares))
        return;

    double initPrice;
    if (!m_share_price_ctrl->checkValue(initPrice))
        return;

    double currentPrice;
    if (!m_current_price_ctrl->GetDouble(currentPrice))
    {
        // assume current price = purchase price on stock creation only
        if (!m_stock)
        {
            currentPrice = initPrice;
            m_current_price_ctrl->SetValue(m_share_price_ctrl->GetValue());
        }
    }

    double commission = 0;
    m_commission_ctrl->GetDouble(commission);

    double initValue = 0;
    // maintain initial value for records with share accounts
    m_stock&& initPrice == 0
        ? initValue = m_stock->VALUE
        : initValue = initPrice * numShares;

    if (!m_stock) m_stock = Model_Stock::instance().create();

    m_stock->HELDAT = m_account_id;
    m_stock->PURCHASEDATE = pdate;
    m_stock->STOCKNAME = stockName;
    m_stock->SYMBOL = stockSymbol;
    m_stock->NUMSHARES = numShares;
    m_stock->PURCHASEPRICE = initPrice;
    m_stock->NOTES = notes;
    m_stock->CURRENTPRICE = currentPrice;
    m_stock->VALUE = initValue;
    m_stock->COMMISSION = commission;

    m_stock_id = Model_Stock::instance().save(m_stock);

    if (!m_edit)
    {
        const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
        mmAttachmentManage::RelocateAllAttachments(RefType, 0, m_stock->STOCKID);
    }

    if (!stockName.empty())
    {
        Model_Account::Data* share_account = Model_Account::instance().get(stockName);
        if (!share_account && !m_edit)
        {
            if (wxMessageBox(_(
                "A Share Account as the Company name has not been found.\n\n"
                "Do you want to create one?")
                , _("New Stock Investment"), wxYES_NO | wxICON_INFORMATION) == wxYES)
            {
                CreateShareAccount(account, stockName);
            }
        }
        else if (!share_account)
        {
            if (wxMessageBox(_(
                "The Company name does not have an associated Share Account.\n\n"
                "You may want to readjust the Company Name to an existing Share Account with the same name. "
                "If this is an existing Stock without a Share Account, it is recommended that a Share Account is created.\n\n"
                "Do you want to create a new Share Acccount?\n")
                , _("Edit Stock Investment"), wxYES_NO | wxICON_WARNING) == wxYES)
            {
                CreateShareAccount(account, stockName);
            }
        }
    }

    m_edit = true;
    UpdateControls();
}

void mmStockDialog::CreateShareAccount(Model_Account::Data * stock_account, const wxString& name)
{
    if (name.empty()) return;
    Model_Account::Data* share_account = Model_Account::instance().create();
    share_account->ACCOUNTNAME = name;
    share_account->ACCOUNTTYPE = Model_Account::all_type()[Model_Account::SHARES];

    share_account->FAVORITEACCT = "TRUE";
    share_account->STATUS = Model_Account::all_status()[Model_Account::OPEN];
    share_account->INITIALBAL = 0;
    share_account->CURRENCYID = stock_account->CURRENCYID;
    Model_Account::instance().save(share_account);

    mmNewAcctDialog account_dialog(share_account, this);
    account_dialog.ShowModal();

    ShareTransactionDialog share_dialog(this, m_stock);
    share_dialog.ShowModal();
    m_gui_frame->RefreshNavigationTree();
    EndModal(wxID_OK);
}

void mmStockDialog::OnHistoryImportButton(wxCommandEvent & WXUNUSED(event))
{
    if (m_stock->SYMBOL.IsEmpty())
        return;

    bool canceledbyuser = false;
    const wxString fileName = wxFileSelector(_("Choose CSV data file to import")
        , wxEmptyString, wxEmptyString, wxEmptyString, _("CSV Files (*.csv)") + "|*.csv;*.CSV", wxFD_FILE_MUST_EXIST);

    if (!fileName.IsEmpty())
    {
        wxFileName csv_file(fileName);
        if (fileName.IsEmpty() || !csv_file.FileExists())
            return;
        wxTextFile tFile(fileName);
        if (!tFile.Open())
            return;
        wxProgressDialog * progressDlg = new wxProgressDialog(_("Stock History CSV Import")
            , _("Quotes imported from CSV: "), tFile.GetLineCount()
            , NULL, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_CAN_ABORT);
        long countNumTotal = 0;
        long countImported = 0;
        double price;
        wxString dateStr, priceStr;
        Model_StockHistory::Data * data;
        Model_StockHistory::Cache stockData;

        wxString line;
        std::vector<wxString> rows;
        for (line = tFile.GetFirstLine(); !tFile.Eof(); line = tFile.GetNextLine())
        {
            wxString progressMsg;
            progressMsg << _("Quotes imported from CSV: ") << countImported;
            if (!progressDlg->Update(countImported, progressMsg))
            {
                canceledbyuser = true;
                break; // abort processing
            }

            if (!line.IsEmpty())
                ++countNumTotal;
            else
                continue;

            dateStr.clear();
            priceStr.clear();

            const wxString& delimiter = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);
            csv2tab_separated_values(line, delimiter);
            wxStringTokenizer tkz(line, "\t", wxTOKEN_RET_EMPTY_ALL);
            if (tkz.CountTokens() < 2)
                continue;

            std::vector<wxString> tokens;
            while (tkz.HasMoreTokens())
            {
                wxString token = tkz.GetNextToken();
                tokens.push_back(token);
            }

            // date
            wxDateTime dt;
            dateStr = tokens[0];
            mmParseDisplayStringToDate(dt, dateStr, Option::instance().getDateFormat());
            dateStr = dt.FormatISODate();
            // price
            priceStr = tokens[1];
            priceStr.Replace(" ", wxEmptyString);
            if (!Model_Currency::fromString(priceStr, price, m_currency) || price <= 0.0)
                continue;

            data = Model_StockHistory::instance().create();
            data->SYMBOL = m_stock->SYMBOL;
            data->DATE = dateStr;
            data->VALUE = price;
            data->UPDTYPE = Model_StockHistory::MANUAL;
            stockData.push_back(data);

            if (rows.size() < 10)
            {
                dateStr << "  " << priceStr;
                rows.push_back(dateStr);
            }
            countImported++;
        }

        progressDlg->Destroy();

        wxString msg = wxString::Format(_("Total Lines: %ld"), countNumTotal);
        msg << "\n";
        msg << wxString::Format(_("Total Imported: %ld"), countImported);
        msg << "\n";
        msg << _("Date") << "              " << _("Price");
        msg << "\n";
        for (std::vector<wxString>::const_iterator d = rows.begin(); d != rows.end(); ++d)
            msg << *d << "\n";
        wxString confirmMsg = msg + _("Please confirm saving...");
        if (!canceledbyuser && wxMessageBox(confirmMsg
            , _("Importing CSV"), wxOK | wxCANCEL | wxICON_INFORMATION) == wxCANCEL)
        {
            canceledbyuser = true;
        }

        // Since all database transactions are only in memory,
        if (!canceledbyuser)
        {
            // we need to save them to the database.
            for (auto& d : stockData)
                Model_StockHistory::instance().save(d);
            // show the data
            RefreshStockHistory(m_stock->SYMBOL);
        }
        else
        {
            //TODO: and discard the database changes.
        }
    }
}

void mmStockDialog::OnHistoryDownloadButton(wxCommandEvent & WXUNUSED(event))
{

    if (m_stock->SYMBOL.IsEmpty())
        return;

    /*"ValidRanges":["1d","5d","1mo","3mo","6mo","1y","2y","5y","10y","ytd","max"]*/
    enum { DAY5, MON, MON3, MON6, YEAR, YEAR2, YEAR5, YEAR10, YTD, MAX };
    const wxString ranges[] = { "5d","1mo","3mo","6mo","1y","2y","5y","10y","ytd","max" };
    const std::vector<std::pair<int, wxString> > RANGE_PAIRS =
    {
        { DAY5, _("5 Days") }
        ,{ MON, _("1 Month") }
        ,{ MON3, _("3 Months") }
        ,{ MON6, _("6 Months") }
        ,{ YEAR, _("1 Year") }
        ,{ YEAR2, _("2 Years") }
        ,{ YEAR5, _("5 Years") }
        ,{ YEAR10, _("10 Years") }
        ,{ YTD, _("Current Year to Date") }
        ,{ MAX, _("Max") }
    };

    wxArrayString items;
    for (const auto& entry : RANGE_PAIRS) { items.Add(entry.second); }

    int range_menu_item_no = wxGetSingleChoiceIndex(_("Specify type frequency of stock history")
        , _("Stock History Update"), items);

    if (range_menu_item_no < 0) return;
    const wxString range = ranges[range_menu_item_no];

    wxString interval = "1d";
    if (range != "5d")
    {
        /* Valid intervals : [1m, 2m, 5m, 15m, 30m, 60m, 90m, 1h, 1d, 5d, 1wk, 1mo, 3mo] */
        enum { IDAILY, IDAY5, IWEEK, IMON, IMON3 };
        const wxString intervals[] = { "1d","5d","1wk","1mo","3mo" };
        const std::vector<std::pair<int, wxString> > INTERVAL_PAIRS =
        {
            { IDAILY, _("1 Day") }
            ,{ IDAY5, _("5 Days") }
            ,{ IWEEK, _("1 Week") }
            ,{ IMON, _("1 Month") }
            ,{ IMON3, _("3 Months") }
        };

        items.clear();
        for (const auto& entry : INTERVAL_PAIRS) { items.Add(entry.second); }

        int interval_menu_item_no = wxGetSingleChoiceIndex(_("Specify interval of stock history")
            , _("Stock History Update"), items);

        if (interval_menu_item_no < 0) return;
        interval = intervals[interval_menu_item_no];
    }
    const wxString URL = wxString::Format(mmex::weblink::YahooQuotesHistory
        , m_stock->SYMBOL, range, interval);

    wxString json_data, sOutput;
    auto err_code = http_get_data(URL, json_data);
    if (err_code != CURLE_OK)
    {
        if (sOutput == wxEmptyString) sOutput = _("Stock history not found!"); //TODO: ?
        mmErrorDialogs::MessageError(this, sOutput, _("Stock History Error"));
        return;
    }

    Document json_doc;
    if (json_doc.Parse(json_data.c_str()).HasParseError())
    {
        return;
    }

    if (!json_doc.HasMember("chart") || !json_doc["chart"].IsObject())
        return;
    Value chart = json_doc["chart"].GetObject();

    wxASSERT(chart.HasMember("error"));
    if (!chart.HasMember("error") || !chart["error"].IsNull())
        return;

    if (!chart.HasMember("result") || !chart["result"].IsArray())
        return;
    Value result = chart["result"].GetArray();
    if (!result.IsArray() || !result.Begin()->IsObject())
        return;
    Value data = result.Begin()->GetObject();

    if (!data.HasMember("meta") || !data["meta"].IsObject())
        return;
    Value meta = data["meta"].GetObject();

    if (!meta.HasMember("currency") || !meta["currency"].IsString())
        return;
    const auto currency = wxString::FromUTF8(meta["currency"].GetString());
    float k = currency == "GBp" ? 100 : 1;

    if (!data.HasMember("timestamp") || !data["timestamp"].IsArray())
        return;
    Value timestamp = data["timestamp"].GetArray();

    if (!data.HasMember("indicators") || !data.IsObject())
        return;
    Value indicators = data["indicators"].GetObject();

    if (!indicators.HasMember("adjclose") || !indicators["adjclose"].IsArray())
        return;
    Value quote_array = indicators["adjclose"].GetArray();
    Value quotes = quote_array.Begin()->GetObject();
    if (!quotes.HasMember("adjclose") || !quotes["adjclose"].IsArray())
        return;
    Value quotes_closed = quotes["adjclose"].GetArray();

    if (timestamp.Size() != quotes_closed.Size())
        return;

    std::map<time_t, float> history;
    for (rapidjson::SizeType i = 0; i < timestamp.Size(); i++)
    {
        if (!timestamp[i].IsInt()) continue;
        time_t time = timestamp[i].GetInt();
        if (!quotes_closed[i].IsFloat()) continue;
        float rate = quotes_closed[i].GetFloat() / k;
        history[time] = rate;
    }

    const wxString today = wxDate::Today().FormatISODate();
    Model_StockHistory::instance().Savepoint();
    for (const auto& entry : history)
    {
        float dPrice = entry.second;
        const wxString date_str = wxDateTime(static_cast<time_t>(entry.first)).FormatISODate();
        if (date_str == today) {
            continue;
        }

        if (Model_StockHistory::instance()
            .find(
                Model_StockHistory::SYMBOL(m_stock->SYMBOL)
                , Model_StockHistory::DB_Table_STOCKHISTORY::DATE(date_str)
            ).empty()
            && dPrice > 0
            )
        {
            Model_StockHistory::Data* ndata = Model_StockHistory::instance().create();
            ndata->SYMBOL = m_stock->SYMBOL;
            ndata->DATE = date_str;
            ndata->VALUE = dPrice;
            ndata->UPDTYPE = Model_StockHistory::ONLINE;
            Model_StockHistory::instance().save(ndata);
        }
    }
    Model_StockHistory::instance().ReleaseSavepoint();
    RefreshStockHistory(m_stock->SYMBOL);
}

void mmStockDialog::OnMenuAddSelected()
{
    wxPanel* hist_buttons_panel = static_cast<wxPanel*>(FindWindow(ID_HIST_PANEL));
    hist_buttons_panel->Show();
    this->Layout();
}

void mmStockDialog::OnHistoryAddUpdateEntry(wxCommandEvent & WXUNUSED(event))
{
    if (m_stock->SYMBOL.IsEmpty())
        return;

    double price;
    if (!m_exchange_text->checkValue(price))
        return;

    wxDateTime date = m_history_date_ctrl->GetValue().GetDateOnly();

    Model_StockHistory::Data_Set entries = Model_StockHistory::instance().find(Model_StockHistory::DATE(date)
        , Model_StockHistory::SYMBOL(m_stock->SYMBOL));
    if (entries.empty())
    {
        Model_StockHistory::instance().addUpdate(m_stock->SYMBOL, date, price, Model_StockHistory::MANUAL);
    }
    else
    {
        Model_StockHistory::Data* stockHistyEntry = Model_StockHistory::instance().get(entries.begin()->SYMBOL, date);
        stockHistyEntry->DATE = date.FormatISODate();
        stockHistyEntry->VALUE = price;
        stockHistyEntry->UPDTYPE = Model_StockHistory::MANUAL;
        Model_StockHistory::instance().save(stockHistyEntry);
    }

    //Hide fields afrer usage
    wxPanel* hist_buttons_panel = static_cast<wxPanel*>(FindWindow(ID_HIST_PANEL));
    hist_buttons_panel->Show(false);
    this->Layout();

    m_history_date_ctrl->SetValue(wxDate::Today());
    m_exchange_text->ChangeValue("");

    RefreshStockHistory(m_stock->SYMBOL);

}

void mmStockDialog::OnHistoryDelete()
{
    wxDataViewItemArray array;
    m_price_listbox->GetSelections(array);

    if (!array.empty())
    {
        Model_StockHistory::instance().Savepoint();
        for (const auto entry : array)
        {
            int hist_id = static_cast<int>(m_price_listbox->GetItemData(entry));
            Model_StockHistory::instance().remove(hist_id);
        }

        Model_StockHistory::instance().ReleaseSavepoint();
        RefreshStockHistory(m_stock->SYMBOL);
    }
}

void mmStockDialog::RefreshStockHistory(const wxString& symbol)
{
    m_price_listbox->DeleteAllItems();
    if (symbol.empty())
        return;

    Model_StockHistory::Data_Set histData = Model_StockHistory::instance().find(Model_StockHistory::SYMBOL(symbol));
    std::stable_sort(histData.begin(), histData.end(), SorterByDATE());
    std::reverse(histData.begin(), histData.end());
    if (histData.size() > 300) {
        histData.resize(300);
    }

    if (!histData.empty())
    {
        for (const auto& d : histData)
        {
            const wxDate dtdt = Model_StockHistory::DATE(d);
            const wxString dispAmount = Model_Account::toString(d.VALUE, m_account, m_precision);
            const wxString manual = d.UPDTYPE == Model_StockHistory::MANUAL ? "*" : "";
            wxVector<wxVariant> data;
            if (debug_) data.push_back(wxVariant(wxString::Format("%i", d.HISTID)));
            data.push_back(wxVariant(mmGetDateForDisplay(dtdt.FormatISODate())));
            data.push_back(wxVariant(dispAmount));
            data.push_back(wxVariant(manual));
            m_price_listbox->AppendItem(data, static_cast<wxUIntPtr>(d.HISTID));
        }

        wxDateTime dt = mmParseISODate(histData.begin()->DATE);
        double amount = histData.begin()->VALUE;
        //wxLogDebug("Date: %s | %.2f", dt.FormatISODate(), amount);
        if (dt > m_purchase_date_ctrl->GetValue().GetDateOnly())
        {
            m_current_price_ctrl->SetValue(amount, m_precision);
        }
    }

}

void mmStockDialog::OnFocusChange(wxChildFocusEvent & event)
{
    m_object_in_focus = -1;
    wxWindow* w = event.GetWindow();
    if (w) {
        m_object_in_focus = w->GetId();
    }

    UpdateControls();
    event.Skip();
}

void mmStockDialog::OnSelectionChanged(wxDataViewEvent& event)
{
    m_history_date_ctrl->SetValue(wxDate::Today());
    m_exchange_text->ChangeValue("");

    wxDataViewItem item = event.GetItem();
    if (!item.IsOk())
        return;

    int hist_id = static_cast<int>(m_price_listbox->GetItemData(item));
    if (hist_id > -1)
    {
        Model_StockHistory::Data* entry = Model_StockHistory::instance().get(hist_id);
        wxDate date = mmParseISODate(entry->DATE);
        m_history_date_ctrl->SetValue(date);
        m_exchange_text->SetValue(entry->VALUE, m_precision);
    }
}

void  mmStockDialog::OnListValueEditingDone(wxDataViewEvent & event)
{
    wxDataViewItem item = event.GetItem();

    if (m_stock && item.IsOk())
    {
        int row = m_price_listbox->ItemToRow(item);
        int col = event.GetColumn();
        wxVariant value;
        m_price_listbox->GetValue(value, static_cast<unsigned int>(row), static_cast<unsigned int>(col));
        m_current_value = value.GetString();
        m_price_listbox->SelectRow(row);
    }
    else
    {
        event.Veto();
    }
}

void  mmStockDialog::OnListValueChanged(wxDataViewEvent & event)
{
    int col = event.GetColumn();
    int row = m_price_listbox->GetSelectedRow();

    wxVariant value;
    m_price_listbox->GetValue(value, static_cast<unsigned int>(row), static_cast<unsigned int>(col));

    wxString amount = mmTrimAmount(value.GetString(), m_decimal_point);
    amount = Model_Currency::fromString2Default(amount, m_currency);
    double price;
    if (!Model_Currency::fromString(amount, price, m_currency) || price <= 0.0)
    {
        m_price_listbox->SetValue(wxVariant(m_current_value), row, col);
    }
    else
    {
        amount = Model_Currency::toString(price, m_currency, m_precision);

        if (!amount.empty() && amount != m_current_value)
        {
            m_current_value = amount;
            m_price_listbox->SetValue(wxVariant(amount), row, col);
            m_price_listbox->SetValue(wxVariant("*"), row, col);

            wxDataViewItem item = m_price_listbox->GetSelection();
            int hist_id = static_cast<int>(m_price_listbox->GetItemData(item));
            if (hist_id > -1)
            {
                Model_StockHistory::Data* entry = Model_StockHistory::instance().get(hist_id);
                wxDate date = mmParseISODate(entry->DATE);
                Model_StockHistory::instance().addUpdate(m_stock->SYMBOL, date, price, Model_StockHistory::MANUAL);
            }
        }
        m_exchange_text->SetValue(amount);
    }
}

void mmStockDialog::OnTextEntered(wxCommandEvent & WXUNUSED(event))
{
    m_current_price_ctrl->Calculate(m_precision);
    m_commission_ctrl->Calculate(m_precision);
    m_share_price_ctrl->Calculate(m_precision);
    m_exchange_text->Calculate(m_precision);
}

void mmStockDialog::OnMagicButton(wxCommandEvent & WXUNUSED(event))
{
    wxDataViewEvent evt;
    OnItemRightClick(evt);
}

void mmStockDialog::OnItemRightClick(wxDataViewEvent & event)
{
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, wxID_ANY);
    evt.SetEventObject(this);

    wxMenu* mainMenu = new wxMenu;
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_NEW_ENTRY, _("Add/Update Stock Price history")));
    mainMenu->Append(new wxMenuItem(mainMenu, MENU_DELETE_ENTRY, _("Delete selected Stock Prices")));

    PopupMenu(mainMenu);
    delete mainMenu;
    event.Skip();
}

void mmStockDialog::OnMenuSelected(wxCommandEvent & event)
{
    switch (event.GetId())
    {
    case MENU_NEW_ENTRY: OnMenuAddSelected(); break;
    case MENU_DELETE_ENTRY: OnHistoryDelete(); break;
    default: break;
    }
}
