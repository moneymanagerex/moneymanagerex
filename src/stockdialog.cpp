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
#include "mmtextctrl.h"
#include "attachmentdialog.h"
#include "constants.h"
#include "paths.h"
#include "util.h"
#include "validators.h"
#include <wx/valnum.h>
#include "model/Model_Infotable.h"
#include "model/Model_Account.h"
#include "model/Model_Attachment.h"
#include "model/Model_StockHistory.h"
#include "../resources/update_currency.xpm"
#include "../resources/attachment.xpm"
#include"../resources/uparrow.xpm"
#include "import_export/univcsvdialog.h"

#define wxID_IMPORT 10106
#define wxID_UPDATE 10107

IMPLEMENT_DYNAMIC_CLASS(mmStockDialog, wxDialog)

BEGIN_EVENT_TABLE( mmStockDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmStockDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmStockDialog::OnCancel)
    EVT_BUTTON(wxID_INDEX, mmStockDialog::OnStockPriceButton)
    EVT_BUTTON(wxID_FILE, mmStockDialog::OnAttachments)
    EVT_BUTTON(wxID_IMPORT, mmStockDialog::OnHistoryImportButton)
    EVT_BUTTON(wxID_UPDATE, mmStockDialog::OnHistoryUpdateButton)
    EVT_BUTTON(wxID_DELETE, mmStockDialog::OnHistoryDeleteButton)

    EVT_LIST_ITEM_SELECTED(wxID_ANY, mmStockDialog::OnListItemSelected)
END_EVENT_TABLE()

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
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    wxString heading = _("New Stock Investment");
    if (edit_) heading = _("Edit Stock Investment");
    Create(parent, wxID_ANY, heading, wxDefaultPosition, wxSize(400, 300), style);
}

bool mmStockDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption,
                           const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    fillControls();
    if (edit_) dataToControls();

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
    valueInvestment_->SetLabelText(Model_Account::toCurrency(m_stock->VALUE, account));
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    if (account) currency = Model_Account::currency(account);
    int currency_precision = Model_Currency::precision(currency);
    if (currency_precision < 4) currency_precision = 4;
    purchasePrice_->SetValue(m_stock->PURCHASEPRICE, account, currency_precision);
    currentPrice_->SetValue(m_stock->CURRENTPRICE, account, currency_precision);
    commission_->SetValue(m_stock->COMMISSION, account, currency_precision);
    showStockHistory();
}

void mmStockDialog::fillControls()
{

}

void mmStockDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(itemBoxSizer2);

    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Expand();

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, flags);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, _("Stock Investment Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer4, flags);

    wxPanel* itemPanel5 = new wxPanel( this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemStaticBoxSizer4->Add(itemPanel5, flags);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel5->SetSizer(itemFlexGridSizer6);

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Stock Name")), flags);

    stockName_ = new mmTextCtrl( itemPanel5, ID_TEXTCTRL_STOCKNAME, "");
    itemFlexGridSizer6->Add(stockName_, flagsExpand);
    stockName_->SetToolTip(_("Enter the stock company name"));
    if (!edit_)
        stockName_->SetFocus();

    //Date
    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Date")), flags);

    dpc_ = new wxDatePickerCtrl( itemPanel5, ID_DPC_STOCK_PDATE,
        wxDefaultDateTime, wxDefaultPosition, wxSize(150, -1), wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    itemFlexGridSizer6->Add(dpc_, flags);
    dpc_->SetToolTip(_("Specify the purchase date of the stock investment"));
    //

    //Symbol
    wxStaticText* symbol = new wxStaticText(itemPanel5, wxID_STATIC, _("Symbol"));
    itemFlexGridSizer6->Add(symbol, flags);
    symbol->SetFont(this->GetFont().Bold());

    stockSymbol_ = new mmTextCtrl( itemPanel5, ID_TEXTCTRL_STOCK_SYMBOL,
        "", wxDefaultPosition, wxSize(150, -1), 0 );
    itemFlexGridSizer6->Add(stockSymbol_, flags);
    stockSymbol_->SetToolTip(_("Enter the stock symbol. (Optional) Include exchange. eg: IBM.BE"));

    //Number of Shares
    wxStaticText* number = new wxStaticText(itemPanel5, wxID_STATIC, _("Number of Shares"));
    itemFlexGridSizer6->Add(number, flags);
    number->SetFont(this->GetFont().Bold());
    numShares_ = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_NUMBER_SHARES, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(numShares_, flags);
    numShares_->SetToolTip(_("Enter number of shares held"));
    numShares_->Connect(ID_TEXTCTRL_NUMBER_SHARES, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);

    //Purchase Price
    wxStaticText* pprice = new wxStaticText(itemPanel5, wxID_STATIC, _("Purchase Price"));
    itemFlexGridSizer6->Add(pprice, flags);
    pprice->SetFont(this->GetFont().Bold());
    purchasePrice_ = new mmTextCtrl( itemPanel5, ID_TEXTCTRL_STOCK_PP, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(purchasePrice_, flags);
    purchasePrice_->SetToolTip(_("Enter purchase price for each stock"));
    purchasePrice_->Connect(ID_TEXTCTRL_STOCK_PP, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);

    //
    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Current Price")), flags);
    currentPrice_ = new mmTextCtrl( itemPanel5, ID_TEXTCTRL_STOCK_CP, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(currentPrice_, flags);
    currentPrice_->SetToolTip(_("Enter current stock price"));
    currentPrice_->Connect(ID_TEXTCTRL_STOCK_CP, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);

    //
    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Price Date")), flags);

    priceDate_ = new wxDatePickerCtrl( itemPanel5, ID_DPC_CP_PDATE,
        wxDefaultDateTime, wxDefaultPosition, wxSize(120, -1), wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    itemFlexGridSizer6->Add(priceDate_, flags);
    priceDate_->SetToolTip(_("Specify the stock price date"));

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Commission")), flags);
    commission_ = new mmTextCtrl( itemPanel5, ID_TEXTCTRL_STOCK_COMMISSION, "0"
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    itemFlexGridSizer6->Add(commission_, flags);
    commission_->SetToolTip(_("Enter any commission paid"));
    commission_->Connect(ID_TEXTCTRL_STOCK_COMMISSION, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockDialog::OnTextEntered), nullptr, this);

    //
    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Value")), flags);
    valueInvestment_ = new wxStaticText( itemPanel5, ID_STATIC_STOCK_VALUE, "--");
    itemFlexGridSizer6->Add(valueInvestment_, flags);

    //
    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Notes")), flags);
    bAttachments_ = new wxBitmapButton( itemPanel5, wxID_FILE
        , wxBitmap(attachment_xpm), wxDefaultPosition
        , wxSize(commission_->GetSize().GetY(), commission_->GetSize().GetY()));
    itemFlexGridSizer6->Add(bAttachments_, wxSizerFlags(flags).Align(wxALIGN_RIGHT));
    bAttachments_->SetToolTip(_("Organize attachments of this stock"));

    notes_ = new mmTextCtrl( this, wxID_STATIC, "", wxDefaultPosition, wxSize(200, 90), wxTE_MULTILINE );
    itemStaticBoxSizer4->Add(notes_, flagsExpand);
    itemStaticBoxSizer4->AddSpacer(1);
    notes_->SetToolTip(_("Enter notes associated with this investment"));

    wxPanel* itemPanel27 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer3->Add(itemPanel27, flags.Right());

    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel27->SetSizer(itemBoxSizer28);

    wxButton* itemButtonOK = new wxButton(itemPanel27, wxID_OK);
    wxButton* itemButton30 = new wxButton(itemPanel27, wxID_CANCEL, _("&Cancel "));
    wxBitmapButton* itemButton31 = new wxBitmapButton(itemPanel27, wxID_INDEX, wxNullBitmap
        , wxDefaultPosition, wxSize(itemButtonOK->GetSize().GetHeight(), itemButtonOK->GetSize().GetHeight()));
    itemButton31->SetToolTip(_("Will display the web page for the specified Stock symbol"));
    //TODO: Provide other ico
    itemButton31->SetBitmapLabel(update_currency_xpm);

    if (edit_)
        itemButton30->SetFocus();
    itemBoxSizer28->Add(itemButton31, flags);
    itemBoxSizer28->Add(itemButtonOK, flags);
    itemBoxSizer28->Add(itemButton30, flags);

    wxPanel* itemPanel32 = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer3->Add(itemPanel32, flags.Right());

    wxBoxSizer* itemBoxSizer33 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel32->SetSizer(itemBoxSizer33);

    //Import CSV
    wxBitmapButton* itemButtonImport = new wxBitmapButton(itemPanel32, wxID_IMPORT, wxNullBitmap,
        wxDefaultPosition, wxSize(itemButtonOK->GetSize().GetHeight(), itemButtonOK->GetSize().GetHeight()));
    itemButtonImport->SetToolTip(_("Import Stock Price history (CSV Format)"));
    itemButtonImport->SetBitmapLabel(/*update_currency_xpm*/uparrow_xpm);
    //delete
    wxButton* itemButtonDel = new wxButton(itemPanel32, wxID_DELETE, _("&Delete "));
    //update
    wxButton* itemButtonUpd = new wxButton(itemPanel32, wxID_UPDATE, _("&Update"));
    itemBoxSizer33->Add(itemButtonImport, flags);
    itemBoxSizer33->Add(itemButtonDel, flags);
    itemBoxSizer33->Add(itemButtonUpd, flags);

    priceListBox_ = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxSize(250, 200),
        wxLC_REPORT);
    itemBoxSizer2->Add(priceListBox_, 1, wxGROW|wxALL, 5);
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
}


void mmStockDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmStockDialog::OnAttachments(wxCommandEvent& /*event*/)
{
    wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
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
    wxString stockSymbol = stockSymbol_->GetValue().Trim();

    if (!stockSymbol.IsEmpty())
    {
        wxString stockURL = Model_Infotable::instance().GetStringInfo("STOCKURL", mmex::weblink::DefStockUrl);
        wxString httpString = wxString::Format(stockURL, stockSymbol);
        wxLaunchDefaultBrowser(httpString);
    }
}

void mmStockDialog::OnOk(wxCommandEvent& /*event*/)
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
    Model_Currency::Data *currency = Model_Account::currency(account);
    wxString pdate = dpc_->GetValue().FormatISODate();
    wxString heldAt = account->ACCOUNTNAME; 

    wxString stockName = stockName_->GetValue();
    wxString stockSymbol = stockSymbol_->GetValue();
    wxString notes       = notes_->GetValue();

    wxString numSharesStr = numShares_->GetValue().Trim();
    double numShares = 0;
	if (!Model_Currency::fromString(numSharesStr, numShares, currency) || numShares < 0)
    {
        mmShowErrorMessage(this, _("Invalid number of shares entered "), _("Error"));
        return;
    }

    wxString pPriceStr = purchasePrice_->GetValue().Trim();
    double pPrice;
    if (!Model_Currency::fromString(pPriceStr, pPrice, currency) || pPrice < 0)
    {
        mmShowErrorMessage(this, _("Invalid purchase price entered "), _("Error"));
        return;
    }

    wxString currentPriceStr = currentPrice_->GetValue().Trim();
    double cPrice;
    if (!Model_Currency::fromString(currentPriceStr, cPrice, currency) || cPrice < 0)
    {
        // we assume current price = purchase price
        cPrice = pPrice;
    }

    wxString commissionStr = commission_->GetValue().Trim();
    double commission;
    if (!Model_Currency::fromString(commissionStr, commission, currency) || commission < 0)
    {
        mmShowErrorMessage(this, _("Invalid commission entered "), _("Error"));
        return;
    }

    double cValue = cPrice * numShares;

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

    Model_Stock::instance().save(m_stock);

    if (!edit_)
    {
        transID_ = m_stock->STOCKID;
        wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
        mmAttachmentManage::RelocateAllAttachments(RefType, 0, transID_);
    }
    else
        transID_ = stockID_;

    // update stock history table
    if (!m_stock->SYMBOL.IsEmpty())
        Model_StockHistory::instance().addUpdate(m_stock->SYMBOL, priceDate_->GetValue(), cPrice, Model_StockHistory::MANUAL);

    EndModal(wxID_OK);
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
    wxString fileName = wxFileSelector(_("Choose CSV data file to import"), 
                wxEmptyString, wxEmptyString, wxEmptyString, "*.csv", wxFD_FILE_MUST_EXIST);
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
        wxProgressDialog* progressDlg = new wxProgressDialog(_("Stock History CSV Import"),
            _("Quotes imported from CSV: "), tFile.GetLineCount(),
            NULL, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_CAN_ABORT);
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

            wxString delimiter = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);
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

        wxString msg = wxString::Format(_("Total Lines : %d"), countNumTotal); 
        msg << wxT ("\n");
        msg << wxString::Format(_("Total Imported : %d"), countImported); 
        msg << wxT ("\n");
        msg << wxT ("Date") << wxT ("              ") << wxT ("Price");
        msg << wxT ("\n");
        for (std::vector<wxString>::const_iterator d = rows.begin(); d != rows.end(); ++d)
            msg << *d << wxT ("\n");
        wxString confirmMsg = msg + _("Please confirm saving...");
        if (!canceledbyuser && wxMessageBox(confirmMsg, _("Importing CSV"), wxOK|wxCANCEL|wxICON_INFORMATION) == wxCANCEL)
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
            // and discard the database changes.
        }
    }
}

void mmStockDialog::OnHistoryUpdateButton(wxCommandEvent& /*event*/)
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

    for (long i=0; i<priceListBox_->GetItemCount(); i++)
    {
        if (priceListBox_->GetItemState(i, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
        {
            Model_StockHistory::instance().remove((int)priceListBox_->GetItemData(i));
            priceListBox_->DeleteItem(i);
            if (priceListBox_->GetItemCount())
                priceListBox_->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            break;
        }
    }
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
            wxDate dtdt = Model_StockHistory::DATE(d);
            wxString dispAmount = Model_Account::toString(d.VALUE, account, 4);
            priceListBox_->SetItem(idx, 0, mmGetDateForDisplay(dtdt));
            priceListBox_->SetItem(idx, 1, dispAmount);
            if (idx==0)
            {
                priceDate_->SetValue(dtdt);
                currentPrice_->SetValue(dispAmount);
            }
            dispAmount = Model_Account::toString(d.VALUE-m_stock->PURCHASEPRICE, account, 4);
            priceListBox_->SetItem(idx, 2, dispAmount);
            idx++;
        }
        priceListBox_->RefreshItems(0, --idx);
    }
}
