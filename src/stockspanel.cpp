/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2010-2021 Nikolay Akimov
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "stockspanel.h"
#include "images_list.h"
#include "mmSimpleDialogs.h"
#include "mmTips.h"
#include "stockdialog.h"
#include "sharetransactiondialog.h"

#include "model/allmodel.h"

#include <wx/clipbrd.h>

class mmStocksPanel;

/*******************************************************/
BEGIN_EVENT_TABLE(mmStocksPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,          mmStocksPanel::OnNewStocks)
    EVT_BUTTON(wxID_EDIT,         mmStocksPanel::OnEditStocks)
    EVT_BUTTON(wxID_ADD,          mmStocksPanel::OnEditStocks)
    EVT_BUTTON(wxID_VIEW_DETAILS, mmStocksPanel::OnEditStocks)
    EVT_BUTTON(wxID_DELETE,       mmStocksPanel::OnDeleteStocks)
    EVT_BUTTON(wxID_MOVE_FRAME,   mmStocksPanel::OnMoveStocks)
    EVT_BUTTON(wxID_FILE,         mmStocksPanel::OnOpenAttachment)
    EVT_BUTTON(wxID_REFRESH,      mmStocksPanel::OnRefreshQuotes)
END_EVENT_TABLE()
/*******************************************************/
mmStocksPanel::mmStocksPanel(int64 accountID
    , mmGUIFrame* frame
    , wxWindow *parent
    , wxWindowID winid)
    : m_account_id(accountID)
    , m_currency()
    , m_frame(frame)
{
    Create(parent, winid);
}

bool mmStocksPanel::Create(wxWindow *parent
    , wxWindowID winid, const wxPoint& pos
    , const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    strLastUpdate_ = Model_Infotable::instance().getString("STOCKS_LAST_REFRESH_DATETIME", "");
    this->windowsFreezeThaw();

    Model_Account::Data *account = Model_Account::instance().get(m_account_id);
    if (account)
        m_currency = Model_Account::currency(account);
    else
        m_currency = Model_Currency::GetBaseCurrency();

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    this->windowsFreezeThaw();
    Model_Usage::instance().pageview(this);
    return true;
}

mmStocksPanel::~mmStocksPanel()
{
}

void mmStocksPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    /* ---------------------- */
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY
        , wxDefaultPosition , wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    itemBoxSizer9->Add(headerPanel, 0, wxALIGN_LEFT);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    header_text_ = new wxStaticText(headerPanel, wxID_STATIC, "");
    header_text_->SetFont(this->GetFont().Larger().Bold());

    m_choiceFilter = new wxChoice(headerPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr);
    m_choiceFilter->Append(_t("All"));
    m_choiceFilter->Append(_T("Non-Zero Shares"));
    m_choiceFilter->SetMinSize(wxSize(150, -1));
    m_choiceFilter->SetSelection(0);

    m_choiceFilter->Bind(wxEVT_CHOICE, [this](wxCommandEvent& event)
    {
        RefreshList();
    });

    header_total_ = new wxStaticText(headerPanel, wxID_STATIC, "");

    wxBoxSizer* itemBoxSizerHHeader = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerHHeader->Add(header_text_, 1, wxALIGN_CENTER_VERTICAL | wxALL, 1);

    itemBoxSizerVHeader->Add(itemBoxSizerHHeader, 1, wxEXPAND, 1);
    itemBoxSizerVHeader->Add(m_choiceFilter, g_flagsBorder1H);
    itemBoxSizerVHeader->Add(header_total_, 1, wxALL, 1);

    /* ---------------------- */
    wxSplitterWindow* itemSplitterWindow10 = new wxSplitterWindow(this
        , wxID_ANY, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER);

    m_lc = new StocksListCtrl(this, itemSplitterWindow10, wxID_ANY);

    wxPanel* BottomPanel = new wxPanel(itemSplitterWindow10, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    mmThemeMetaColour(BottomPanel, meta::COLOR_LISTPANEL);

    itemSplitterWindow10->SplitHorizontally(m_lc, BottomPanel);
    itemSplitterWindow10->SetMinimumPaneSize(100);
    itemSplitterWindow10->SetSashGravity(1.0);
    itemBoxSizer9->Add(itemSplitterWindow10, g_flagsExpandBorder1);

    wxBoxSizer* BoxSizerVBottom = new wxBoxSizer(wxVERTICAL);
    BottomPanel->SetSizer(BoxSizerVBottom);

    wxBoxSizer* BoxSizerHBottom = new wxBoxSizer(wxHORIZONTAL);
    BoxSizerVBottom->Add(BoxSizerHBottom, g_flagsBorder1V);

    wxButton* itemButton6 = new wxButton(BottomPanel, wxID_NEW, _t("&New "));
    mmToolTip(itemButton6, _t("New Stock Investment"));
    BoxSizerHBottom->Add(itemButton6, 0, wxRIGHT, 5);

    wxButton* add_trans_btn = new wxButton(BottomPanel, wxID_ADD, _t("&Adjust Trade "));
    mmToolTip(add_trans_btn, _t("Add Stock Transactions"));
    BoxSizerHBottom->Add(add_trans_btn, 0, wxRIGHT, 5);
    add_trans_btn->Enable(false);

    wxButton* view_trans_btn = new wxButton(BottomPanel, wxID_VIEW_DETAILS, _t("&View Trans "));
    mmToolTip(view_trans_btn, _t("View Stock Transactions"));
    BoxSizerHBottom->Add(view_trans_btn, 0, wxRIGHT, 5);
    view_trans_btn->Enable(false);

    wxButton* itemButton81 = new wxButton(BottomPanel, wxID_EDIT, _t("&Edit "));
    mmToolTip(itemButton81, _t("Edit Stock Investment"));
    BoxSizerHBottom->Add(itemButton81, 0, wxRIGHT, 5);
    itemButton81->Enable(false);

    wxButton* itemButton7 = new wxButton(BottomPanel, wxID_DELETE, _t("&Delete "));
    mmToolTip(itemButton7, _t("Delete Stock Investment"));
    BoxSizerHBottom->Add(itemButton7, 0, wxRIGHT, 5);
    itemButton7->Enable(false);

    wxButton* bMove = new wxButton(BottomPanel, wxID_MOVE_FRAME, _t("&Move"));
    mmToolTip(bMove, _t("Move selected transaction to another account"));
    BoxSizerHBottom->Add(bMove, 0, wxRIGHT, 5);
    bMove->Enable(false);

    attachment_button_ = new wxBitmapButton(BottomPanel
        , wxID_FILE, mmBitmapBundle(png::CLIP, mmBitmapButtonSize), wxDefaultPosition
        , wxSize(30, bMove->GetSize().GetY()));
    mmToolTip(attachment_button_, _t("Open attachments"));
    BoxSizerHBottom->Add(attachment_button_, 0, wxRIGHT, 5);
    attachment_button_->Enable(false);

    refresh_button_ = new wxBitmapButton(BottomPanel
        , wxID_REFRESH, mmBitmapBundle(png::LED_OFF, mmBitmapButtonSize), wxDefaultPosition, wxSize(30, bMove->GetSize().GetY()));
    refresh_button_->SetLabelText(_t("Refresh"));
    mmToolTip(refresh_button_, _t("Refresh Stock Prices from Yahoo"));
    BoxSizerHBottom->Add(refresh_button_, 0, wxRIGHT, 5);

    //Infobar-mini
    stock_details_short_ = new wxStaticText(BottomPanel, wxID_STATIC, strLastUpdate_);
    BoxSizerHBottom->Add(stock_details_short_, 1, wxGROW | wxTOP | wxLEFT, 5);
    //Infobar
    stock_details_ = new wxStaticText(BottomPanel, wxID_STATIC, ""
        , wxDefaultPosition, wxSize(200, -1), wxTE_MULTILINE | wxTE_WORDWRAP);
    BoxSizerVBottom->Add(stock_details_, g_flagsExpandBorder1);

    updateExtraStocksData(-1);
}

void mmStocksPanel::AddStockTransaction(int selectedIndex)
{
    Model_Stock::Data* stock = &m_lc->m_stocks[selectedIndex];
    ShareTransactionDialog dlg(this, stock);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_lc->doRefreshItems(dlg.m_stock_id);
        updateExtraStocksData(selectedIndex);
    }
}

void mmStocksPanel::OnListItemActivated(int selectedIndex)
{
    call_dialog(selectedIndex);
    updateExtraStocksData(selectedIndex);
}

//TODO: improve View Stock Transactions
void mmStocksPanel::ViewStockTransactions(int selectedIndex)
{
    Model_Stock::Data* stock = &m_lc->m_stocks[selectedIndex];

    wxDialog dlg(this, wxID_ANY,
                 _t("View Stock Transactions") + ": " +
                 Model_Account::get_account_name(stock->HELDAT) + " - " + stock->SYMBOL,
                 wxDefaultPosition, wxDefaultSize,
                 wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    dlg.SetIcon(mmex::getProgramIcon());
    wxWindow* parent = dlg.GetMainWindowOfCompositeControl();
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);

    // Initialize list control
    wxListCtrl* stockTxnListCtrl = InitStockTxnListCtrl(parent);
    topsizer->Add(stockTxnListCtrl, wxSizerFlags(g_flagsExpand).TripleBorder());

    // Load stock transactions
    LoadStockTransactions(stockTxnListCtrl, stock->STOCKID);

    // Bind list events
    BindListEvents(stockTxnListCtrl);

    // Add buttons
    wxSizer* buttonSizer = dlg.CreateSeparatedButtonSizer(wxOK);
    if (buttonSizer) {
        topsizer->Add(buttonSizer, wxSizerFlags().Expand().DoubleBorder(wxLEFT | wxRIGHT | wxBOTTOM));
    }

    dlg.SetSizerAndFit(topsizer);
    dlg.SetInitialSize(wxSize(600, 400)); // Set default size
    dlg.Center();
    dlg.ShowModal();
    RefreshList();
}

// Initialize the list control
wxListCtrl* mmStocksPanel::InitStockTxnListCtrl(wxWindow* parent)
{
    wxListCtrl* listCtrl = new wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
                                          wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_AUTOARRANGE);

    listCtrl->AppendColumn(_t("Date"), wxLIST_FORMAT_LEFT, 100);
    listCtrl->AppendColumn(_t("Lot"), wxLIST_FORMAT_LEFT, 140);
    listCtrl->AppendColumn(_t("Shares"), wxLIST_FORMAT_RIGHT, 100);
    listCtrl->AppendColumn(_t("Trade Type"), wxLIST_FORMAT_LEFT, 80);
    listCtrl->AppendColumn(_t("Price"), wxLIST_FORMAT_RIGHT, 100);
    listCtrl->AppendColumn(_t("Commission"), wxLIST_FORMAT_RIGHT, 100);

    return listCtrl;
}

// Load stock transactions into the list control
void mmStocksPanel::LoadStockTransactions(wxListCtrl* listCtrl, int64 stockId)
{
    Model_Translink::Data_Set stock_list = Model_Translink::TranslinkList<Model_Stock>(stockId);
    Model_Checking::Data_Set checking_list;

    for (const auto& trans : stock_list) {
        auto* checking_entry = Model_Checking::instance().get(trans.CHECKINGACCOUNTID);
        if (checking_entry && checking_entry->DELETEDTIME.IsEmpty()) {
            checking_list.push_back(*checking_entry);
        }
    }
    std::stable_sort(checking_list.begin(), checking_list.end(), SorterByTRANSDATE());

    int row = 0;
    for (const auto& stock_trans : checking_list)
    {
        auto* share_entry = Model_Shareinfo::ShareEntry(stock_trans.TRANSID);
        if (!share_entry || (share_entry->SHARENUMBER <= 0 && share_entry->SHAREPRICE <= 0))
            continue;

        long index = listCtrl->InsertItem(row++, "");
        listCtrl->SetItemData(index, stock_trans.TRANSID.GetValue());
        FillListRow(listCtrl, index, stock_trans, *share_entry);
    }
}

// Fill list row with stock transaction data
void mmStocksPanel::FillListRow(wxListCtrl* listCtrl, long index, const Model_Checking::Data& txn, const Model_Shareinfo::Data& share_entry)
{
    listCtrl->SetItem(index, 0, mmGetDateTimeForDisplay(txn.TRANSDATE));
    listCtrl->SetItem(index, 1, share_entry.SHARELOT);

    int precision = share_entry.SHARENUMBER == floor(share_entry.SHARENUMBER) ? 0 : Option::instance().getSharePrecision();
    listCtrl->SetItem(index, 2, wxString::FromDouble(share_entry.SHARENUMBER, precision));
    listCtrl->SetItem(index, 3, Model_Checking::trade_type_name(Model_Checking::type_id(txn.TRANSCODE)));
    listCtrl->SetItem(index, 4, wxString::FromDouble(share_entry.SHAREPRICE, Option::instance().getSharePrecision()));
    listCtrl->SetItem(index, 5, wxString::FromDouble(share_entry.SHARECOMMISSION, 2));
}

// Bind list control events
void mmStocksPanel::BindListEvents(wxListCtrl* listCtrl)
{
    listCtrl->Bind(wxEVT_LIST_ITEM_ACTIVATED, [listCtrl, this](wxListEvent& event) {
        long index = event.GetIndex();
        auto* txn = Model_Checking::instance().get(event.GetData());
        if (!txn) return;

        auto link = Model_Translink::TranslinkRecord(txn->TRANSID);
        ShareTransactionDialog dlg(listCtrl, &link, txn);
        dlg.ShowModal();

        // Update the modified row
        auto* share_entry = Model_Shareinfo::ShareEntry(txn->TRANSID);
        if (share_entry) {
            this->FillListRow(listCtrl, index, *txn, *share_entry);
        }

        // Re-sort the list
        listCtrl->SortItems([](wxIntPtr item1, wxIntPtr item2, wxIntPtr) -> int {
            auto date1 = Model_Checking::TRANSDATE(Model_Checking::instance().get(item1));
            auto date2 = Model_Checking::TRANSDATE(Model_Checking::instance().get(item2));
            return date1.IsEarlierThan(date2) ? -1 : (date1.IsLaterThan(date2) ? 1 : 0);
        }, 0);
    });

    listCtrl->Bind(wxEVT_CHAR, [listCtrl, this](wxKeyEvent& event) {
        if (event.GetKeyCode() == WXK_CONTROL_C) {
            CopySelectedRowsToClipboard(listCtrl);
        } else if (event.GetKeyCode() == WXK_CONTROL_A) {
            for (int row = 0; row < listCtrl->GetItemCount(); row++)
                listCtrl->SetItemState(row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
    });
}

// Copy selected rows to the clipboard
void mmStocksPanel::CopySelectedRowsToClipboard(wxListCtrl* listCtrl)
{
    if (!wxTheClipboard->Open()) return;

    wxString data;
    const wxString separator = "\t";

    for (int row = 0; row < listCtrl->GetItemCount(); row++) {
        if (listCtrl->GetItemState(row, wxLIST_STATE_SELECTED) != wxLIST_STATE_SELECTED)
            continue;

        for (int col = 0; col < listCtrl->GetColumnCount(); col++) {
            if (listCtrl->GetColumnWidth(col) > 0) {
                data += listCtrl->GetItemText(row, col) + separator;
            }
        }
        data += "\n";
    }

    wxTheClipboard->SetData(new wxTextDataObject(data));
    wxTheClipboard->Close();
}

wxString mmStocksPanel::GetPanelTitle(const Model_Account::Data& account) const
{
    return wxString::Format(_t("Stock Portfolio: %s"), account.ACCOUNTNAME);
}

wxString mmStocksPanel::BuildPage() const
{ 
    const Model_Account::Data* account = Model_Account::instance().get(m_account_id);
    return m_lc->BuildPage((account ? GetPanelTitle(*account) : ""));
}

const wxString mmStocksPanel::Total_Shares()
{
    double total_shares = 0;
    for (const auto& stock : Model_Stock::instance().find(Model_Stock::HELDAT(m_account_id)))
    {
        total_shares += stock.NUMSHARES;
    }

    int precision = (total_shares - static_cast<int>(total_shares) != 0) ? 4 : 0;
    return Model_Currency::toString(total_shares, m_currency, precision);
}

void mmStocksPanel::updateHeader()
{
    const Model_Account::Data* account = Model_Account::instance().get(m_account_id);
    double cashBalance = 0;
    // + Transfered from other accounts - Transfered to other accounts

    //Get Stock Investment Account Balance as Init Amount + sum (Value) - sum (Purchase Price)
    std::pair<double, double> investment_balance;
    if (account)
    {
        header_text_->SetLabelText(GetPanelTitle(*account));
        cashBalance = Model_Account::balance(account);
        investment_balance = Model_Account::investment_balance(account);
    }

    auto [marketValue, InvestedVal] = investment_balance;

    const wxString& diffStr = Model_Currency::toCurrency(marketValue > InvestedVal ? marketValue - InvestedVal : InvestedVal - marketValue, m_currency);
    double diffPercents = InvestedVal != 0.0
        ? (marketValue > InvestedVal ? marketValue / InvestedVal*100.0 - 100.0 : -(marketValue / InvestedVal*100.0 - 100.0))
        : 0.0;
    const wxString lbl = wxString::Format("%s     %s     %s     %s     %s (%s %%)"
        , wxString::Format(_t("Total: %s"), Model_Currency::toCurrency(marketValue + cashBalance, m_currency))
        , wxString::Format(_t("Cash Balance: %s"), Model_Currency::toCurrency(cashBalance, m_currency))
        , wxString::Format(_t("Market Value: %s"), Model_Currency::toCurrency(marketValue, m_currency))
        , wxString::Format(_t("Invested: %s"), Model_Currency::toCurrency(InvestedVal, m_currency))
        , wxString::Format(marketValue > InvestedVal ? _t("Gain: %s") : _t("Loss: %s"), diffStr)
        , Model_Currency::toString(diffPercents, m_currency, 2));

    header_total_->SetLabelText(lbl);
    this->Layout();
}

void mmStocksPanel::OnDeleteStocks(wxCommandEvent& event)
{
    m_lc->OnDeleteStocks(event);
}

void mmStocksPanel::OnMoveStocks(wxCommandEvent& event)
{
    m_lc->OnMoveStocks(event);
}

void mmStocksPanel::OnNewStocks(wxCommandEvent& event)
{
    m_lc->OnNewStocks(event);
}

void mmStocksPanel::OnEditStocks(wxCommandEvent& event)
{
    m_lc->OnEditStocks(event);
}

void mmStocksPanel::OnOpenAttachment(wxCommandEvent& event)
{
    m_lc->OnOpenAttachment(event);
}

void mmStocksPanel::OnRefreshQuotes(wxCommandEvent& WXUNUSED(event))
{
    wxString sError = "";
    bool ok = onlineQuoteRefresh(sError);
    if (ok)
    {
        const wxString header = _t("Stock prices successfully updated");
        stock_details_->SetLabelText(header);
        stock_details_short_->SetLabelText(wxString::Format(_t("Last updated %s"), strLastUpdate_));
        wxMessageDialog msgDlg(this, sError, header);
        msgDlg.ShowModal();
        refresh_button_->SetBitmapLabel(mmBitmapBundle(png::LED_GREEN, mmBitmapButtonSize));
    }
    else
    {
        refresh_button_->SetBitmapLabel(mmBitmapBundle(png::LED_RED, mmBitmapButtonSize));
        stock_details_->SetLabelText(sError);
        stock_details_short_->SetLabelText(_t("Error"));
        mmErrorDialogs::MessageError(this, sError, _t("Error"));
    }
}

/*** Trigger a quote download ***/
bool mmStocksPanel::onlineQuoteRefresh(wxString& msg)
{
    wxString base_currency_symbol;
    if (!Model_Currency::GetBaseCurrencySymbol(base_currency_symbol))
    {
        msg = _t("Unable to find base currency symbol!");
        return false;
    }

    if (m_lc->m_stocks.empty())
    {
        msg = _t("Nothing to update");
        return false;
    }

    std::map<wxString, double> symbols;
    Model_Stock::Data_Set stock_list = Model_Stock::instance().all();
    for (const auto &stock : stock_list)
    {
        const wxString symbol = stock.SYMBOL.Upper();
        if (symbol.IsEmpty()) continue;
        symbols[symbol] = stock.VALUE;
    }

    refresh_button_->SetBitmapLabel(mmBitmapBundle(png::LED_YELLOW, mmBitmapButtonSize));
    stock_details_->SetLabelText(_tu("Connecting…"));

    std::map<wxString, double > stocks_data;
    bool ok = get_yahoo_prices(symbols, stocks_data, base_currency_symbol, msg, yahoo_price_type::SHARES);
    if (!ok) {
        return false;
    }

    std::map<wxString, double> nonYahooSymbols;

    Model_StockHistory::instance().Savepoint();
    for (auto &s : stock_list)
    {
        std::map<wxString, double>::const_iterator it = stocks_data.find(s.SYMBOL.Upper());
        if (it == stocks_data.end()) {
            nonYahooSymbols[s.SYMBOL.Upper()] = 0;
            continue;
        }

        double dPrice = it->second;

        if (dPrice != 0)
        {
            msg += wxString::Format("%s\t: %0.6f -> %0.6f\n", s.SYMBOL, s.CURRENTPRICE, dPrice);
            s.CURRENTPRICE = dPrice;
            Model_Stock::instance().save(&s);
            Model_StockHistory::instance().addUpdate(s.SYMBOL
                , wxDate::Now(), dPrice, Model_StockHistory::ONLINE);
        }
    }
    Model_StockHistory::instance().ReleaseSavepoint();

    for (const auto& entry : nonYahooSymbols)
    {
        msg += wxString::Format("%s\t: %s\n", entry.first, _t("Missing"));
    }

    // Now refresh the display
    RefreshList();

    // We are done!
    LastRefreshDT_ = wxDateTime::Now();
    StocksRefreshStatus_ = true;

    strLastUpdate_.Printf(_t("%1$s on %2$s"), LastRefreshDT_.FormatTime()
        , mmGetDateTimeForDisplay(LastRefreshDT_.FormatISODate()));
    Model_Infotable::instance().setString("STOCKS_LAST_REFRESH_DATETIME", strLastUpdate_);

    return true;
}

void mmStocksPanel::updateExtraStocksData(int selectedIndex)
{
    enableEditDeleteButtons(selectedIndex >= 0);
    if (selectedIndex >= 0)
    {
        const wxString additionInfo = m_lc->getStockInfo(selectedIndex);
        stock_details_->SetLabelText(additionInfo);
    }
}

wxString StocksListCtrl::getStockInfo(int selectedIndex) const
{
    int purchasedTime = 0;
    double stocktotalnumShares = 0;
    double stockavgPurchasePrice = 0;
    for (const auto& s: Model_Stock::instance().find(Model_Stock::SYMBOL(m_stocks[selectedIndex].SYMBOL)))
    {
        purchasedTime++;
        stocktotalnumShares += s.NUMSHARES;
        stockavgPurchasePrice += s.VALUE;
    }
    stockavgPurchasePrice /= stocktotalnumShares;

    double numShares = m_stocks[selectedIndex].NUMSHARES;
    wxString sNumShares = wxString::Format("%i", static_cast<int>(numShares));
    if (numShares - static_cast<long>(numShares) != 0.0)
        sNumShares = wxString::Format("%.4f", numShares);

    wxString sTotalNumShares = wxString::Format("%i", static_cast<int>(stocktotalnumShares));
    if ((stocktotalnumShares - static_cast<long>(stocktotalnumShares)) != 0.0)
        sTotalNumShares = wxString::Format("%.4f", stocktotalnumShares);

    double stockPurchasePrice = m_stocks[selectedIndex].PURCHASEPRICE;
    double stockCurrentPrice = m_stocks[selectedIndex].CURRENTPRICE;
    double stockDifference = stockCurrentPrice - stockPurchasePrice;

    double stocktotalDifference = stockCurrentPrice - stockavgPurchasePrice;
    // Commission don't calculates here
    const wxString& stockPercentage = (stockPurchasePrice != 0.0)
        ? wxString::Format("(%s %%)", Model_Currency::toStringNoFormatting(
            ((stockCurrentPrice / stockPurchasePrice - 1.0) * 100.0), nullptr, 2))
        : "";
    double stocktotalPercentage = (stockCurrentPrice / stockavgPurchasePrice - 1.0)*100.0;
    double stocktotalgainloss = stocktotalDifference * stocktotalnumShares;

    const wxString& sPurchasePrice = Model_Currency::toCurrency(stockPurchasePrice, m_stock_panel->m_currency, 4);
    const wxString& sAvgPurchasePrice = Model_Currency::toCurrency(stockavgPurchasePrice, m_stock_panel->m_currency, 4);
    const wxString& sCurrentPrice = Model_Currency::toCurrency(stockCurrentPrice, m_stock_panel->m_currency, 4);
    const wxString& sDifference = Model_Currency::toCurrency(stockDifference, m_stock_panel->m_currency, 4);
    const wxString& sTotalDifference = Model_Currency::toCurrency(stocktotalDifference);

    wxString miniInfo = "";
    if (m_stocks[selectedIndex].SYMBOL != "")
        miniInfo << "\t" << wxString::Format(_t("Symbol: %s"), m_stocks[selectedIndex].SYMBOL) << "\t\t";
    miniInfo << wxString::Format(_t("Total: %s"), " (" + sTotalNumShares + ") ");
    m_stock_panel->stock_details_short_->SetLabelText(miniInfo);

    //Selected share info
    wxString additionInfo = wxString::Format("This Account: |%s - %s| = %s, %s * %s = %s %s\n"
        , sCurrentPrice, sPurchasePrice, sDifference
        , sDifference, sNumShares
        , Model_Currency::toCurrency(GetGainLoss(selectedIndex), m_stock_panel->m_currency)
        , stockPercentage);

    //Summary for account for selected symbol
    if (purchasedTime > 1)
    {
        additionInfo += wxString::Format( "All Accounts: |%s - %s| = %s, %s * %s = %s ( %s %% )\n%s"
            ,  sCurrentPrice, sAvgPurchasePrice, sTotalDifference
            , sTotalDifference, sTotalNumShares
            , Model_Currency::toCurrency(stocktotalgainloss)
            , Model_Currency::toStringNoFormatting(stocktotalPercentage, nullptr, 2)
            , OnGetItemText(selectedIndex, static_cast<long>(LIST_ID_NOTES)));
    }
    return additionInfo;
}
void mmStocksPanel::enableEditDeleteButtons(bool en)
{
    wxButton* bN = static_cast<wxButton*>(FindWindow(wxID_NEW));
    wxButton* bE = static_cast<wxButton*>(FindWindow(wxID_EDIT));
    wxButton* bA = static_cast<wxButton*>(FindWindow(wxID_ADD));
    wxButton* bV = static_cast<wxButton*>(FindWindow(wxID_VIEW_DETAILS));
    wxButton* bD = static_cast<wxButton*>(FindWindow(wxID_DELETE));
    wxButton* bM = static_cast<wxButton*>(FindWindow(wxID_MOVE_FRAME));
    if (bN) bN->Enable(!en);
    if (bE) bE->Enable(en);
    if (bA) bA->Enable(en);
    if (bV) bV->Enable(en);
    if (bD) bD->Enable(en);
    if (bM) bM->Enable(en);
    attachment_button_->Enable(en);
    if (!en)
    {
        if (Option::instance().getShowMoneyTips())
            stock_details_->SetLabelText(wxGetTranslation(STOCKTIPS[rand() % (sizeof(STOCKTIPS) / sizeof(wxString))]));
        stock_details_short_->SetLabelText(wxString::Format(_t("Last updated %s"), strLastUpdate_));
    }
}

void mmStocksPanel::call_dialog(int selectedIndex)
{
    Model_Stock::Data* stock = &m_lc->m_stocks[selectedIndex];
    mmStockDialog dlg(this, m_frame, stock, m_account_id);
    dlg.ShowModal();
    m_lc->doRefreshItems(dlg.m_stock_id);
}

void mmStocksPanel::DisplayAccountDetails(int64 accountID)
{

    m_account_id = accountID;

    Model_Account::Data* account = Model_Account::instance().get(m_account_id);
    m_currency = Model_Account::currency(account);

    updateHeader();
    enableEditDeleteButtons(false);
    m_lc->initVirtualListControl();

}

void mmStocksPanel::RefreshList()
{
    int64 selected_id = -1;
    if (m_lc->get_selectedIndex() > -1)
        selected_id = m_lc->m_stocks[m_lc->get_selectedIndex()].STOCKID;
    m_lc->doRefreshItems(selected_id);
}

int mmStocksPanel::getFilter()
{
    return m_choiceFilter->GetSelection();
}

