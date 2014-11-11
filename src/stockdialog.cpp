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
#include "attachmentdialog.h"
#include "constants.h"
#include "mmtextctrl.h"
#include "paths.h"
#include "util.h"
#include "validators.h"
#include "import_export/univcsvdialog.h"
#include "model/Model_Infotable.h"
#include "model/Model_Account.h"
#include "model/Model_Attachment.h"
#include "model/Model_StockHistory.h"
#include "../resources/attachment.xpm"
#include "../resources/checkupdate.xpm"
#include"../resources/uparrow.xpm"
#include "../resources/web.xpm"
#include <wx/numdlg.h>
#include <wx/textdlg.h>
#include <wx/valnum.h>

IMPLEMENT_DYNAMIC_CLASS(mmStockDialog, wxDialog)

wxBEGIN_EVENT_TABLE(mmStockDialog, wxDialog)
    EVT_BUTTON(wxID_SAVE, mmStockDialog::OnSave)
    EVT_BUTTON(wxID_CANCEL, mmStockDialog::OnCancel)
    EVT_BUTTON(wxID_INDEX, mmStockDialog::OnStockPriceButton)
    EVT_BUTTON(wxID_FILE, mmStockDialog::OnAttachments)
    EVT_BUTTON(ID_BUTTON_IMPORT, mmStockDialog::OnHistoryImportButton)
    EVT_BUTTON(ID_BUTTON_DOWNLOAD, mmStockDialog::OnHistoryDownloadButton)
    EVT_BUTTON(wxID_ADD, mmStockDialog::OnHistoryAddButton)
    EVT_BUTTON(wxID_DELETE, mmStockDialog::OnHistoryDeleteButton)
    EVT_CHILD_FOCUS(mmStockDialog::onFocusChange)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, mmStockDialog::OnListItemSelected)
wxEND_EVENT_TABLE()

mmStockDialog::mmStockDialog( )
{
}

mmStockDialog::mmStockDialog(wxWindow* parent
    , Model_Stock::Data* stock
    , int accountID)
    : m_stock(stock)
    , edit_(stock ? true: false)
    , accountID_(accountID)
    , skip_attachments_init_(false)
    , stockName_(nullptr)
    , stockSymbol_(nullptr)
    , dpc_(nullptr)
    , numShares_(nullptr)
    , purchasePrice_(nullptr)
    , notes_(nullptr)
    , currentPrice_(nullptr)
    , priceDate_(nullptr)
    , valueInvestment_(nullptr)
    , commission_(nullptr)
    , bAttachments_(nullptr)
    , priceListBox_(nullptr)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, "", wxDefaultPosition, wxSize(400, 300), style);
}

bool mmStockDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    if (edit_) dataToControls();
    updateControls();

    Centre();
    return TRUE;
}

void mmStockDialog::dataToControls()
{
    if (!this->m_stock) return;

    stockID_ = m_stock->STOCKID;

    stockName_->SetValue(m_stock->STOCKNAME);
    stockSymbol_->SetValue(m_stock->SYMBOL);
    notes_->SetValue(m_stock->NOTES);
    dpc_->SetValue(Model_Stock::PURCHASEDATE(m_stock));

    int precision = m_stock->NUMSHARES == floor(m_stock->NUMSHARES) ? 0 : 4;
    numShares_->SetValue(m_stock->NUMSHARES, precision);
    Model_Account::Data* account = Model_Account::instance().get(m_stock->HELDAT);
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    if (account) currency = Model_Account::currency(account);
    int currency_precision = Model_Currency::precision(currency);
    if (currency_precision < 4) currency_precision = 4;
    purchasePrice_->SetValue(m_stock->PURCHASEPRICE, account, currency_precision);
    currentPrice_->SetValue(m_stock->CURRENTPRICE, account, currency_precision);
    commission_->SetValue(m_stock->COMMISSION, account, currency_precision);
    
    showStockHistory();
}

void mmStockDialog::updateControls()
{
    this->SetTitle(edit_ ? _("Edit Stock Investment") : _("New Stock Investment"));
    Model_Account::Data* account = Model_Account::instance().get(accountID_);
    Model_Currency::Data *currency = Model_Account::currency(account);

    double numShares = 0, pPrice = 0;
    numShares_->GetDouble(numShares, currency);
    currentPrice_->GetDouble(pPrice, currency);
    valueInvestment_->SetLabelText(Model_Account::toCurrency(numShares*pPrice, account));

    //Disable history buttons on new stocks

    wxBitmapButton* buttonDownload = (wxBitmapButton*) FindWindow(ID_BUTTON_DOWNLOAD);
    buttonDownload->Enable(edit_);
    wxBitmapButton* buttonImport = (wxBitmapButton*) FindWindow(ID_BUTTON_IMPORT);
    buttonImport->Enable(edit_);
    wxBitmapButton* buttonDel = (wxBitmapButton*) FindWindow(wxID_DELETE);
    buttonDel->Enable(edit_);
    wxBitmapButton* buttonAdd = (wxBitmapButton*) FindWindow(wxID_ADD);
    buttonAdd->Enable(edit_);
    bAttachments_->Enable(edit_);

    stockSymbol_->SetValue(stockSymbol_->GetValue().Upper());
}

void mmStockDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(mainBoxSizer);

    wxBoxSizer* leftBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(leftBoxSizer, g_flags);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, _("Stock Investment Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    leftBoxSizer->Add(itemStaticBoxSizer4, g_flagsExpand);

    wxPanel* itemPanel5 = new wxPanel(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemStaticBoxSizer4->Add(itemPanel5, g_flags);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel5->SetSizer(itemFlexGridSizer6);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Stock Name")), g_flags);

    stockName_ = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCKNAME, "");
    itemFlexGridSizer6->Add(stockName_, g_flagsExpand);
    stockName_->SetToolTip(_("Enter the stock company name"));

    //Date
    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Date")), g_flags);

    dpc_ = new wxDatePickerCtrl(itemPanel5, ID_DPC_STOCK_PDATE
        , wxDefaultDateTime, wxDefaultPosition, wxSize(150, -1), wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    itemFlexGridSizer6->Add(dpc_, g_flags);
    dpc_->SetToolTip(_("Specify the purchase date of the stock investment"));

    //Symbol
    wxStaticText* symbol = new wxStaticText(itemPanel5, wxID_STATIC, _("Symbol"));
    itemFlexGridSizer6->Add(symbol, g_flags);
    symbol->SetFont(this->GetFont().Bold());

    stockSymbol_ = new mmTextCtrl( itemPanel5, ID_TEXTCTRL_STOCK_SYMBOL
        , "", wxDefaultPosition, wxSize(150, -1), 0);
    itemFlexGridSizer6->Add(stockSymbol_, g_flags);
    stockSymbol_->SetToolTip(_("Enter the stock symbol. (Optional) Include exchange. eg: IBM.BE"));

    //Number of Shares
    wxStaticText* number = new wxStaticText(itemPanel5, wxID_STATIC, _("Number of Shares"));
    itemFlexGridSizer6->Add(number, g_flags);
    number->SetFont(this->GetFont().Bold());
    numShares_ = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_NUMBER_SHARES, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(numShares_, g_flags);
    numShares_->SetToolTip(_("Enter number of shares held"));
    numShares_->Connect(ID_TEXTCTRL_NUMBER_SHARES, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);

    //Purchase Price
    wxStaticText* pprice = new wxStaticText(itemPanel5, wxID_STATIC, _("Purchase Price"));
    itemFlexGridSizer6->Add(pprice, g_flags);
    pprice->SetFont(this->GetFont().Bold());
    purchasePrice_ = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_PP, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(purchasePrice_, g_flags);
    purchasePrice_->SetToolTip(_("Enter purchase price for each stock"));
    purchasePrice_->Connect(ID_TEXTCTRL_STOCK_PP, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);

    //
    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Current Price")), g_flags);
    currentPrice_ = new mmTextCtrl( itemPanel5, ID_TEXTCTRL_STOCK_CP, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(currentPrice_, g_flags);
    currentPrice_->SetToolTip(_("Enter current stock price"));
    currentPrice_->Connect(ID_TEXTCTRL_STOCK_CP, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);

    //
    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Price Date")), g_flags);
    priceDate_ = new wxDatePickerCtrl(itemPanel5, ID_DPC_CP_PDATE
        , wxDefaultDateTime, wxDefaultPosition, wxSize(150, -1), wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    itemFlexGridSizer6->Add(priceDate_, g_flags);
    priceDate_->SetToolTip(_("Specify the stock price date"));

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Commission")), g_flags);
    commission_ = new mmTextCtrl( itemPanel5, ID_TEXTCTRL_STOCK_COMMISSION, "0"
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(commission_, g_flags);
    commission_->SetToolTip(_("Enter any commission paid"));
    commission_->Connect(ID_TEXTCTRL_STOCK_COMMISSION, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);

    //
    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Value")), g_flags);
    valueInvestment_ = new wxStaticText(itemPanel5, ID_STATIC_STOCK_VALUE, "--");
    itemFlexGridSizer6->Add(valueInvestment_, g_flags);

    //
    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Notes")), g_flags);
    wxBoxSizer* iconsSizer = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer6->Add(iconsSizer, wxSizerFlags(g_flags).Align(wxALIGN_RIGHT));
    bAttachments_ = new wxBitmapButton(itemPanel5, wxID_FILE, wxBitmap(attachment_xpm), wxDefaultPosition
        , wxSize(commission_->GetSize().GetY(), commission_->GetSize().GetY()));
    bAttachments_->SetToolTip(_("Organize attachments of this stock"));
    wxBitmapButton* itemButton31 = new wxBitmapButton(itemPanel5, wxID_INDEX, wxBitmap(web_xpm)
        , wxDefaultPosition, wxSize(commission_->GetSize().GetY(), commission_->GetSize().GetY()));
    itemButton31->SetToolTip(_("Display the web page for the specified Stock symbol"));
    iconsSizer->Add(bAttachments_, g_flags);
    iconsSizer->Add(itemButton31, g_flags);

    notes_ = new mmTextCtrl(this, wxID_STATIC, "", wxDefaultPosition, wxSize(200, 90), wxTE_MULTILINE);
    itemStaticBoxSizer4->Add(notes_, g_flagsExpand);
    itemStaticBoxSizer4->AddSpacer(1);
    notes_->SetToolTip(_("Enter notes associated with this investment"));

    leftBoxSizer->AddSpacer(20);

    //History Panel
    wxBoxSizer* rightBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(rightBoxSizer, g_flagsExpand);

    wxStaticBox* historyStaticBox = new wxStaticBox(this, wxID_ANY, _("Stock History Options"));
    wxStaticBoxSizer* historyStaticBoxSizer = new wxStaticBoxSizer(historyStaticBox, wxVERTICAL);
    rightBoxSizer->Add(historyStaticBoxSizer, g_flagsExpand);

    priceListBox_ = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(250, 150)
        , wxLC_REPORT);
    historyStaticBoxSizer->Add(priceListBox_, g_flagsExpand);
    priceListBox_->SetToolTip(_("Stock Price History"));

    // Add first column
    wxListItem col0;
    col0.SetId(0);
    col0.SetText( _("Date") );
    col0.SetWidth(90);
    priceListBox_->InsertColumn(0, col0);

    // Add second column
    wxListItem col1;
    col1.SetId(1);
    col1.SetText( _("Price") );
    col1.SetWidth(80);
    priceListBox_->InsertColumn(1, col1);

    // Add third column
    wxListItem col2;
    col2.SetId(2);
    col2.SetText( _("Diff.") );
    col2.SetWidth(80);
    priceListBox_->InsertColumn(2, col2);

    //History Buttons
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    historyStaticBoxSizer->Add(buttons_panel, wxSizerFlags(g_flags).Centre());
    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxBitmapButton* buttonDownload = new wxBitmapButton(buttons_panel, ID_BUTTON_DOWNLOAD, wxBitmap(checkupdate_xpm)
        , wxDefaultPosition, wxSize(commission_->GetSize().GetY(), commission_->GetSize().GetY()));
    buttonDownload->SetToolTip(_("Download Stock Price history"));
    wxBitmapButton* buttonImport = new wxBitmapButton(buttons_panel, ID_BUTTON_IMPORT, wxBitmap(uparrow_xpm)
        , wxDefaultPosition, wxSize(commission_->GetSize().GetY(), commission_->GetSize().GetY()));
    buttonImport->SetToolTip(_("Import Stock Price history (CSV Format)"));
    wxButton* buttonDel = new wxButton(buttons_panel, wxID_DELETE, _("&Delete "));
    buttonDel->SetToolTip(_("Delete selected Stock Price"));
    wxButton* buttonAdd = new wxButton(buttons_panel, wxID_ADD, _("&Add "));
    buttonAdd->SetToolTip(_("Add Stock Price to history"));
    buttons_sizer->Add(buttonDownload, g_flags);
    buttons_sizer->Add(buttonImport, g_flags);
    buttons_sizer->Add(buttonDel, g_flags);
    buttons_sizer->Add(buttonAdd, g_flags);

    //OK & Cancel buttons
    wxStdDialogButtonSizer*  buttonsOK_CANCEL_sizer = new wxStdDialogButtonSizer;
    leftBoxSizer->Add(buttonsOK_CANCEL_sizer, wxSizerFlags(g_flags).Centre());

    wxButton* itemButtonOK = new wxButton(this, wxID_SAVE, _("&Save "));
    wxButton* itemButton30 = new wxButton(this, wxID_CANCEL, _("&Close "));

    if (edit_)
        itemButton30->SetFocus();
    buttonsOK_CANCEL_sizer->Add(itemButtonOK, g_flags);
    buttonsOK_CANCEL_sizer->Add(itemButton30, g_flags);
}


void mmStockDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmStockDialog::OnAttachments(wxCommandEvent& /*event*/)
{
    const wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    int RefId = stockID_;

    if (RefId < 0)
        RefId = 0;

    if (RefId == 0 && !skip_attachments_init_)
    {
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
        skip_attachments_init_ = true;
    }

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();
}

void mmStockDialog::OnStockPriceButton(wxCommandEvent& /*event*/)
{
    const wxString stockSymbol = stockSymbol_->GetValue().Trim();

    if (!stockSymbol.IsEmpty())
    {
        const wxString& stockURL = Model_Infotable::instance().GetStringInfo("STOCKURL", mmex::weblink::DefStockUrl);
        const wxString& httpString = wxString::Format(stockURL, stockSymbol);
        wxLaunchDefaultBrowser(httpString);
    }
}

void mmStockDialog::OnSave(wxCommandEvent& /*event*/)
{
    if (priceListBox_->GetItemCount())
    {
        for (long i=0; i<priceListBox_->GetItemCount(); i++)
        {
            if (priceListBox_->GetItemState(0, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
            {
                priceListBox_->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                break;
            }
        }
    }

    Model_Account::Data* account = Model_Account::instance().get(accountID_);
    if (!account)
    {
        mmShowErrorMessageInvalid(this, _("Held At"));
        return;
    }
    
    const wxString& stockSymbol = stockSymbol_->GetValue();
    if (stockSymbol.empty()) {
        mmShowErrorMessageInvalid(this, _("Symbol"));
        return;
    }
        
    Model_Currency::Data *currency = Model_Account::currency(account);
    const wxString& pdate = dpc_->GetValue().FormatISODate();
    const wxString& stockName = stockName_->GetValue();
    const wxString& notes = notes_->GetValue();

    double numShares = 0;
    if (!numShares_->checkValue(numShares, currency))
        return;

    double pPrice;
    if (!purchasePrice_->checkValue(pPrice, currency))
        return;

    double cPrice;
    if (!currentPrice_->GetDouble(cPrice, currency))
    {
        // we assume current price = purchase price
        cPrice = pPrice;
    }

    double commission = 0;
    commission_->GetDouble(commission);

    double cValue = cPrice * numShares; //TODO: what about commision?

    if (!m_stock) m_stock = Model_Stock::instance().create();

    m_stock->HELDAT = accountID_;
    m_stock->PURCHASEDATE = pdate;
    m_stock->STOCKNAME = stockName;
    m_stock->SYMBOL = stockSymbol;
    m_stock->NUMSHARES = numShares;
    m_stock->PURCHASEPRICE = pPrice;
    m_stock->NOTES = notes;
    m_stock->CURRENTPRICE = cPrice;
    m_stock->VALUE = cValue;
    m_stock->COMMISSION = commission;
    if (edit_) m_stock->STOCKID = stockID_;

    stockID_ = Model_Stock::instance().save(m_stock);

    // update stock history table and stock items price/values with same symbol code
    if (!m_stock->SYMBOL.IsEmpty())
    {
        for (auto st : Model_Stock::instance().find(Model_Stock::SYMBOL(m_stock->SYMBOL)))
        {
            if (st.STOCKID != m_stock->STOCKID)
            {
                st.CURRENTPRICE = m_stock->CURRENTPRICE;
                st.VALUE = st.CURRENTPRICE * st.NUMSHARES;
                Model_Stock::instance().save(&st);
            }
            Model_StockHistory::instance().addUpdate(st.SYMBOL, priceDate_->GetValue(), st.CURRENTPRICE, Model_StockHistory::MANUAL);
        }
    }

    edit_ = true;
    updateControls();
}

void mmStockDialog::OnTextEntered(wxCommandEvent& event)
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    Model_Account::Data *account = Model_Account::instance().get(accountID_);
    if (account) currency = Model_Account::currency(account);
    int currency_precision = Model_Currency::precision(currency);
    if (currency_precision < 4) currency_precision = 4;

    if (event.GetId() == numShares_->GetId())
    {
        numShares_->Calculate(currency, 4);
    }
    else if (event.GetId() == purchasePrice_->GetId())
    {
        purchasePrice_->Calculate(currency, currency_precision);
    }
    else if (event.GetId() == currentPrice_->GetId())
    {
        currentPrice_->Calculate(currency, currency_precision);
    }
    else if (event.GetId() == commission_->GetId())
    {
        commission_->Calculate(currency, currency_precision);
    }
}

void mmStockDialog::OnListItemSelected(wxListEvent& event)
{
    long selectedIndex = event.GetIndex();
    long histId = priceListBox_->GetItemData(selectedIndex);
    Model_Account::Data* account = Model_Account::instance().get(m_stock->HELDAT);
    Model_StockHistory::Data *histData = Model_StockHistory::instance().get(histId);

    if (histData->HISTID > 0)
    {
        priceDate_->SetValue(Model_StockHistory::DATE(*histData));
        currentPrice_->SetValue(Model_Account::toString(histData->VALUE, account, 4));
    }
}

void mmStockDialog::OnHistoryImportButton(wxCommandEvent& /*event*/)
{
    if (m_stock->SYMBOL.IsEmpty())
        return;

    bool canceledbyuser = false;
    const wxString fileName = wxFileSelector(_("Choose CSV data file to import")
        , wxEmptyString, wxEmptyString, wxEmptyString, "*.csv", wxFD_FILE_MUST_EXIST);
    Model_Account::Data *account = Model_Account::instance().get(m_stock->HELDAT);
    Model_Currency::Data *currency = Model_Account::currency(account);

    if (!fileName.IsEmpty())
    {
        wxFileName csv_file(fileName);
        if (fileName.IsEmpty() || !csv_file.FileExists())
            return;
        wxTextFile tFile(fileName);
        if (!tFile.Open())
            return;
        wxProgressDialog* progressDlg = new wxProgressDialog(_("Stock History CSV Import")
            , _("Quotes imported from CSV: "), tFile.GetLineCount()
            , NULL, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_CAN_ABORT);
        long countNumTotal = 0;
        long countImported = 0;
        double price;
        wxDateTime dt;
        wxString dateStr, priceStr;
        Model_StockHistory::Data *data;
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
            mmUnivCSVDialog::csv2tab_separated_values(line, delimiter);
            wxStringTokenizer tkz(line, "\t", wxTOKEN_RET_EMPTY_ALL);  
            if ((int)tkz.CountTokens() < 2)
                continue;
            
            std::vector<wxString> tokens;
            while (tkz.HasMoreTokens())
            {
                wxString token = tkz.GetNextToken();
                tokens.push_back(token);
            }

            // date
            dateStr = tokens[0];
            mmParseDisplayStringToDate(dt, dateStr, mmOptions::instance().dateFormat_);
            dateStr = dt.FormatISODate();
            // price
            priceStr = tokens[1];
            priceStr.Replace(" ", wxEmptyString);
            if (!Model_Currency::fromString(priceStr, price, currency) || price <= 0.0)
                continue;

            data = Model_StockHistory::instance().create();
            data->SYMBOL = m_stock->SYMBOL;
            data->DATE = dateStr;
            data->VALUE = price;
            data->UPDTYPE = 2;
            stockData.push_back(data);

            if (rows.size()<10)
            {
                dateStr <<  wxT ("  ") << priceStr;
                rows.push_back(dateStr);
            }
            countImported++;
        }

        progressDlg->Destroy();       

        wxString msg = wxString::Format(_("Total Lines : %ld"), countNumTotal); 
        msg << "\n";
        msg << wxString::Format(_("Total Imported : %ld"), countImported); 
        msg << "\n";
        msg << _("Date") << wxT ("              ") << _("Price");
        msg << "\n";
        for (std::vector<wxString>::const_iterator d = rows.begin(); d != rows.end(); ++d)
            msg << *d << wxT ("\n");
        wxString confirmMsg = msg + _("Please confirm saving...");
        if (!canceledbyuser && wxMessageBox(confirmMsg
            , _("Importing CSV"), wxOK | wxCANCEL | wxICON_INFORMATION) == wxCANCEL)
                canceledbyuser = true;
 
        // Since all database transactions are only in memory,
        if (!canceledbyuser)
        {
            // we need to save them to the database. 
            for (auto &d : stockData)
                Model_StockHistory::instance().save(d);
            // show the data
            showStockHistory();
        }
        else 
        {
            //TODO: and discard the database changes.
        }
    }
}

void mmStockDialog::OnHistoryDownloadButton(wxCommandEvent& /*event*/)
{
    /*
    Example stock history download:
    https://code.google.com/p/yahoo-finance-managed/wiki/csvHistQuotesDownload
    */

    if (m_stock->SYMBOL.IsEmpty())
        return;

    const wxDateTime& StartDate = Model_Stock::PURCHASEDATE(m_stock);
    wxDateTime EndDate = wxDate::Today();
    const wxTimeSpan time = EndDate - StartDate;
    long intervalMonths = EndDate.GetMonth() - StartDate.GetMonth()
        + 12 * (EndDate.GetYear() - StartDate.GetYear())
        - (EndDate.GetDay() < StartDate.GetDay());

    //Define frequency
    enum { DAILY, WEEKLY, MONTHLY };
    wxArrayString FreqStrs;
    FreqStrs.Add(_("Days"));
    FreqStrs.Add(_("Weeks"));
    if (intervalMonths > 0) FreqStrs.Add(_("Months"));

    int freq = wxGetSingleChoiceIndex(_("Specify type frequency of stock history")
        , _("Stock History Update"), FreqStrs);

    long interval = 0;
    switch (freq)
    {
    case DAILY: interval = time.GetDays(); break;
    case WEEKLY: interval = time.GetWeeks(); break;
    case MONTHLY: interval = intervalMonths; break;
    default: return;
    }

    int nrPrices = (int) wxGetNumberFromUser(_("Specify how many stock history prices download from purchase date")
        , wxString::Format(_("Number of %s:"), FreqStrs.Item(freq).Lower()), _("Stock History Update")
        , interval, 1L, 9999L, this, wxDefaultPosition);

    if (nrPrices <= 0)
    {
        mmShowErrorMessageInvalid(this, FreqStrs[freq]);
        return;
    }
    else
    {
        switch (freq)
        {
        case DAILY: EndDate = wxDate(StartDate).Add(wxDateSpan::Days(nrPrices)); break;
        case WEEKLY: EndDate = wxDate(StartDate).Add(wxDateSpan::Weeks(nrPrices)); break;
        case MONTHLY: EndDate = wxDate(StartDate).Add(wxDateSpan::Months(nrPrices)); break;
        default: break;
        }
    }

    if (EndDate > wxDate::Today())
    {
        mmShowWarningMessage(this, _("End date is in the future\nQuotes will be updated until today")
            , _("Stock History Error"));
        EndDate = wxDate::Today();
    }

    wxString CSVQuotes;
    wxString URL = mmex::weblink::YahooQuotesHistory;
    URL += m_stock->SYMBOL;
    URL += wxString::Format("&a=%i", StartDate.GetMonth());
    URL += wxString::Format("&b=%i", StartDate.GetDay());
    URL += wxString::Format("&c=%i", StartDate.GetYear());
    URL += wxString::Format("&d=%i", EndDate.GetMonth());
    URL += wxString::Format("&e=%i", EndDate.GetDay());
    URL += wxString::Format("&f=%i", EndDate.GetYear());
    switch (freq)
    {
    case DAILY: URL += "&g=d"; break;
    case WEEKLY: URL += "&g=w"; break;
    case MONTHLY: URL += "&g=m"; break;
        default: break;
    }
    URL += "&ignore=.csv";
    wxLogDebug("Start Date:%s End Date:%s URL:%s", StartDate.FormatISODate(), EndDate.FormatISODate(), URL);

    int err_code = site_content(URL, CSVQuotes);
    if (err_code != wxURL_NOERR)
    {
        if (err_code == -1) CSVQuotes = _("Stock history not found!");
        mmShowErrorMessage(this, CSVQuotes, _("Stock History Error"));
        return;
    }

    double dPrice;
    wxString dateStr;
    Model_StockHistory::Data *data;

    wxStringTokenizer tkz(CSVQuotes, "\r\n");
    Model_StockHistory::instance().Begin();
    while (tkz.HasMoreTokens())
    {
        wxStringTokenizer tkzSingleLine(tkz.GetNextToken(), ",");
        std::vector<wxString> tokens;
        while (tkzSingleLine.HasMoreTokens())
        {
            const wxString& token = tkzSingleLine.GetNextToken();
            tokens.push_back(token);
        }

        if (tokens[0].Contains("-"))
        {
            dateStr = tokens[0];
            tokens[6].ToDouble(&dPrice);

            if (Model_StockHistory::instance().find(
                    Model_StockHistory::SYMBOL(m_stock->SYMBOL),
                    Model_StockHistory::DB_Table_STOCKHISTORY_V1::DATE(dateStr)
                ).size() == 0
                && dPrice > 0)
            {
                data = Model_StockHistory::instance().create();
                data->SYMBOL = m_stock->SYMBOL;
                data->DATE = dateStr;
                data->VALUE = dPrice;
                data->UPDTYPE = Model_StockHistory::ONLINE;
                Model_StockHistory::instance().save(data);
            }
        }
    }
    Model_StockHistory::instance().Commit();
    showStockHistory();
}

void mmStockDialog::OnHistoryAddButton(wxCommandEvent& /*event*/)
{
    if (m_stock->SYMBOL.IsEmpty())
        return;

    wxString listStr;
    wxDateTime dt;
    long i, histID;
    double dPrice = 0.0;
    Model_Account::Data* account = Model_Account::instance().get(m_stock->HELDAT);
	Model_Currency::Data* currency = Model_Account::currency(account);
    wxString currentPriceStr = currentPrice_->GetValue().Trim();
    if (!Model_Currency::fromString(currentPriceStr, dPrice, currency) || (dPrice < 0.0))
        return;
    histID = Model_StockHistory::instance().addUpdate(m_stock->SYMBOL, priceDate_->GetValue(), dPrice, Model_StockHistory::MANUAL);

    for (i=0; i<priceListBox_->GetItemCount(); i++)
    {
        listStr = priceListBox_->GetItemText(i, 0);
        mmParseDisplayStringToDate(dt, listStr, mmOptions::instance().dateFormat_);
        if (dt.FormatISODate() == priceDate_->GetValue().FormatISODate())
            break;
    }
    if (i == priceListBox_->GetItemCount())
    {
        //add
        for (i=0; i<priceListBox_->GetItemCount(); i++)
        {
            listStr = priceListBox_->GetItemText(i, 0);
            mmParseDisplayStringToDate(dt, listStr, mmOptions::instance().dateFormat_);
            if (dt.FormatISODate() < priceDate_->GetValue().FormatISODate())
                break;
        }
        wxListItem item;
        item.SetId(i);
        item.SetData(histID);
        priceListBox_->InsertItem( item );
    }
    if (i != priceListBox_->GetItemCount())
    {
        listStr = Model_Account::toString(dPrice, account, 4);
        priceListBox_->SetItem(i, 0, mmGetDateForDisplay(priceDate_->GetValue()));
        priceListBox_->SetItem(i, 1, listStr);
        listStr = Model_Account::toString(dPrice - m_stock->PURCHASEPRICE, account, 4);
        priceListBox_->SetItem(i, 2, listStr);
    }
}

void mmStockDialog::OnHistoryDeleteButton(wxCommandEvent& /*event*/)
{
    if (priceListBox_->GetSelectedItemCount() <= 0)
        return;

    long item = -1;
    Model_StockHistory::instance().Begin();
    for (;;)
    {
        item = priceListBox_->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

        if (item == -1)
            break;
        Model_StockHistory::instance().remove((int) priceListBox_->GetItemData(item));
    }
    Model_StockHistory::instance().Commit();
    showStockHistory();
}

void mmStockDialog::showStockHistory()
{
    priceListBox_->DeleteAllItems();
    if (m_stock->SYMBOL.IsEmpty())
        return;

    Model_Account::Data* account = Model_Account::instance().get(m_stock->HELDAT);
    Model_StockHistory::Data_Set histData = Model_StockHistory::instance().find(Model_StockHistory::SYMBOL(m_stock->SYMBOL));
    std::stable_sort(histData.begin(), histData.end(), SorterByDATE());
    std::reverse(histData.begin(), histData.end());
    if (histData.size()>300)
        histData.resize(300);
    if (!histData.empty())
    {
        int idx=0;
        for (const auto &d : histData)
        {
            wxListItem item;
            item.SetId(idx);
            item.SetData(d.HISTID);
            priceListBox_->InsertItem( item );
            const wxDate dtdt = Model_StockHistory::DATE(d);
            const wxString dispAmount = Model_Account::toString(d.VALUE, account, 4);
            priceListBox_->SetItem(idx, 0, mmGetDateForDisplay(dtdt));
            priceListBox_->SetItem(idx, 1, dispAmount);
            if (idx == 0)
            {
                priceDate_->SetValue(dtdt);
                currentPrice_->SetValue(dispAmount);
            }
            const wxString& priceAmount = Model_Account::toString(d.VALUE - m_stock->PURCHASEPRICE, account, 4);
            priceListBox_->SetItem(idx, 2, priceAmount);
            idx++;
        }
        priceListBox_->RefreshItems(0, --idx);
    }
}

void mmStockDialog::onFocusChange(wxChildFocusEvent& event)
{
    updateControls();
    event.Skip();
}