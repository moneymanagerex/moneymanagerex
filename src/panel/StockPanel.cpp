/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2010-2021 Nikolay Akimov
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 Klaus Wich

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

#include "base/defs.h"
#include <wx/clipbrd.h>

#include "base/images_list.h"
#include "util/_simple.h"
#include "util/mmTips.h"
#include "model/_all.h"

#include "StockPanel.h"
#include "dialog/StockDialog.h"
#include "dialog/TransactionShareDialog.h"

class StockPanel;

/*******************************************************/
BEGIN_EVENT_TABLE(StockPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,          StockPanel::OnNewStocks)
    EVT_BUTTON(wxID_EDIT,         StockPanel::OnEditStocks)
    EVT_BUTTON(wxID_ADD,          StockPanel::OnEditStocks)
    EVT_BUTTON(wxID_VIEW_DETAILS, StockPanel::OnEditStocks)
    EVT_BUTTON(wxID_DELETE,       StockPanel::OnDeleteStocks)
    EVT_BUTTON(wxID_MOVE_FRAME,   StockPanel::OnMoveStocks)
    EVT_BUTTON(wxID_FILE,         StockPanel::OnOpenAttachment)
    EVT_BUTTON(wxID_REFRESH,      StockPanel::OnRefreshQuotes)
END_EVENT_TABLE()
/*******************************************************/
StockPanel::StockPanel(int64 accountID
    , mmGUIFrame* frame
    , wxWindow *parent
    , wxWindowID winid)
    : m_account_id(accountID)
    , m_currency()
    , m_frame(frame)
{
    Create(parent, winid);
}

bool StockPanel::Create(wxWindow *parent
    , wxWindowID winid, const wxPoint& pos
    , const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    strLastUpdate_ = InfoModel::instance().getString("STOCKS_LAST_REFRESH_DATETIME", "");
    this->windowsFreezeThaw();

    AccountModel::Data *account = AccountModel::instance().cache_id(m_account_id);
    if (account)
        m_currency = AccountModel::currency(account);
    else
        m_currency = CurrencyModel::GetBaseCurrency();

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    this->windowsFreezeThaw();
    UsageModel::instance().pageview(this);
    mmThemeAutoColour(this);
    return true;
}

StockPanel::~StockPanel()
{
}

void StockPanel::CreateControls()
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
    m_choiceFilter->Append(_t("Non-Zero Shares"));
    m_choiceFilter->SetMinSize(wxSize(150, -1));
    m_choiceFilter->SetSelection(0);

    m_choiceFilter->Bind(wxEVT_CHOICE, [this](wxCommandEvent&)
    {
        RefreshList();
    });

    header_total_ = new wxStaticText(headerPanel, wxID_STATIC, "");

    wxBoxSizer* itemBoxSizerHHeader = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerHHeader->Add(header_text_, 1, wxALIGN_CENTER_VERTICAL | wxALL, 1);

    itemBoxSizerVHeader->Add(itemBoxSizerHHeader, 1, wxEXPAND, 1);
    itemBoxSizerVHeader->Add(m_choiceFilter, g_flagsBorder1V);
    itemBoxSizerVHeader->Add(header_total_, 1, wxALL, 1);

    /* ---------------------- */
    mmSplitterWindow* itemSplitterWindow10 = new mmSplitterWindow(this
        , wxID_ANY, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER, mmThemeMetaColour(meta::COLOR_LISTPANEL));

    m_lc = new StockList(this, itemSplitterWindow10, wxID_ANY);

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

    updateHeader();
}

int StockPanel::AddStockTransaction(int selectedIndex)
{
    StockModel::Data* stock = &m_lc->m_stocks[selectedIndex];
    TransactionShareDialog dlg(this, stock);
    int result = dlg.ShowModal();
    if (result == wxID_OK)
    {
        m_lc->doRefreshItems(dlg.m_stock_id);
        updateExtraStocksData(selectedIndex);
    }
    return result;
}

void StockPanel::OnListItemActivated(int selectedIndex)
{
    call_dialog(selectedIndex);
    updateExtraStocksData(selectedIndex);
}

//TODO: improve View Stock Transactions
void StockPanel::ViewStockTransactions(int selectedIndex)
{
    StockModel::Data* stock = &m_lc->m_stocks[selectedIndex];

    wxDialog dlg(this, wxID_ANY,
                 _t("View Stock Transactions") + ": "
                 + (m_account_id > -1 ? (AccountModel::cache_id_name(stock->HELDAT) + " - ") : "")
                 + stock->SYMBOL,
                 wxDefaultPosition, wxSize(800, 400),
                 wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    dlg.SetIcon(mmex::getProgramIcon());
    wxWindow* parent = dlg.GetMainWindowOfCompositeControl();
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);

    // Initialize list control
    wxListCtrl* stockTxnListCtrl = InitStockTxnListCtrl(parent);
    topsizer->Add(stockTxnListCtrl, wxSizerFlags(g_flagsExpand).TripleBorder());

    // Load stock transactions
    LoadStockTransactions(stockTxnListCtrl, m_account_id == -1 ? stock->SYMBOL : "", stock->STOCKID);

    // Bind list events
    BindListEvents(stockTxnListCtrl);

    // Add buttons
    wxSizer* buttonSizer = dlg.CreateSeparatedButtonSizer(wxOK);
    if (buttonSizer) {
        topsizer->Add(buttonSizer, wxSizerFlags().Expand().DoubleBorder(wxLEFT | wxRIGHT | wxBOTTOM));
    }

    dlg.SetSizerAndFit(topsizer);
    dlg.SetInitialSize(wxSize(800, 400)); // Set default size
    dlg.Center();
    dlg.ShowModal();
    RefreshList();
}

// Initialize the list control
wxListCtrl* StockPanel::InitStockTxnListCtrl(wxWindow* parent)
{
    wxListCtrl* listCtrl = new wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                          wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_AUTOARRANGE);

    listCtrl->AppendColumn(_t("Date"), wxLIST_FORMAT_LEFT, 100);
    listCtrl->AppendColumn(_t("Lot"), wxLIST_FORMAT_LEFT, 140);
    listCtrl->AppendColumn(_t("Shares"), wxLIST_FORMAT_RIGHT, 100);
    listCtrl->AppendColumn(_t("Trade Type"), wxLIST_FORMAT_LEFT, 80);
    listCtrl->AppendColumn(_t("Price"), wxLIST_FORMAT_RIGHT, 100);
    listCtrl->AppendColumn(_t("Commission"), wxLIST_FORMAT_RIGHT, 100);
    listCtrl->AppendColumn(_t("Total"), wxLIST_FORMAT_RIGHT, 100);

    return listCtrl;
}

// Load stock transactions into the list control
void StockPanel::LoadStockTransactions(wxListCtrl* listCtrl, wxString symbol, int64 stockId)
{
    TransactionLinkModel::Data_Set stock_list;
    TransactionModel::Data_Set checking_list;
    if (symbol.IsEmpty()) {
        stock_list = TransactionLinkModel::TranslinkList<StockModel>(stockId);
    }
    else {  // search for all
        stock_list = TransactionLinkModel::TranslinkListBySymbol(symbol);
    }

    for (const auto& trans : stock_list) {
        auto* checking_entry = TransactionModel::instance().cache_id(trans.CHECKINGACCOUNTID);
        if (checking_entry && checking_entry->DELETEDTIME.IsEmpty()) {
            checking_list.push_back(*checking_entry);
        }
    }
    std::stable_sort(checking_list.begin(), checking_list.end(), TransactionTable::SorterByTRANSDATE());

    int row = 0;
    for (const auto& stock_trans : checking_list)
    {
        auto* share_entry = TransactionShareModel::ShareEntry(stock_trans.TRANSID);
        if (!share_entry || (share_entry->SHARENUMBER <= 0 && share_entry->SHAREPRICE <= 0))
            continue;

        long index = listCtrl->InsertItem(row++, "");
        listCtrl->SetItemData(index, stock_trans.TRANSID.GetValue());
        FillListRow(listCtrl, index, stock_trans, *share_entry);
    }
}

// Fill list row with stock transaction data
void StockPanel::FillListRow(wxListCtrl* listCtrl, long index, const TransactionModel::Data& txn, const TransactionShareModel::Data& share_entry)
{
    listCtrl->SetItem(index, 0, mmGetDateTimeForDisplay(txn.TRANSDATE));
    listCtrl->SetItem(index, 1, share_entry.SHARELOT);

    int precision = share_entry.SHARENUMBER == floor(share_entry.SHARENUMBER) ? 0 : PreferencesModel::instance().getSharePrecision();
    listCtrl->SetItem(index, 2, wxString::FromDouble(share_entry.SHARENUMBER, precision));
    listCtrl->SetItem(index, 3, wxGetTranslation(TransactionModel::trade_type_name(TransactionModel::type_id(txn.TRANSCODE))));
    listCtrl->SetItem(index, 4, wxString::FromDouble(share_entry.SHAREPRICE, PreferencesModel::instance().getSharePrecision()));
    listCtrl->SetItem(index, 5, wxString::FromDouble(share_entry.SHARECOMMISSION, 2));
    double total = share_entry.SHARENUMBER * share_entry.SHAREPRICE + share_entry.SHARECOMMISSION;
    listCtrl->SetItem(index, 6, wxString::FromDouble(total, 2));
}

// Bind list control events
void StockPanel::BindListEvents(wxListCtrl* listCtrl)
{
    listCtrl->Bind(wxEVT_LIST_ITEM_ACTIVATED, [listCtrl, this](wxListEvent& event) {
        long index = event.GetIndex();
        auto* txn = TransactionModel::instance().cache_id(event.GetData());
        if (!txn) return;

        auto link = TransactionLinkModel::TranslinkRecord(txn->TRANSID);
        TransactionShareDialog dlg(listCtrl, &link, txn);
        dlg.ShowModal();

        // Update the modified row
        auto* share_entry = TransactionShareModel::ShareEntry(txn->TRANSID);
        if (share_entry) {
            this->FillListRow(listCtrl, index, *txn, *share_entry);
        }

        // Re-sort the list
        listCtrl->SortItems([](wxIntPtr item1, wxIntPtr item2, wxIntPtr) -> int {
            auto date1 = TransactionModel::getTransDateTime(TransactionModel::instance().cache_id(item1));
            auto date2 = TransactionModel::getTransDateTime(TransactionModel::instance().cache_id(item2));
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
void StockPanel::CopySelectedRowsToClipboard(wxListCtrl* listCtrl)
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

wxString StockPanel::GetPanelTitle(const AccountModel::Data& account) const
{
    return wxString::Format(_t("Stock Portfolio: %s"), account.ACCOUNTNAME);
}

wxString StockPanel::BuildPage() const
{
    const AccountModel::Data* account = AccountModel::instance().cache_id(m_account_id);
    return m_lc->BuildPage((account ? GetPanelTitle(*account) : ""));
}

const wxString StockPanel::Total_Shares()
{
    double total_shares = 0;
    for (const auto& stock : StockModel::instance().find(StockModel::HELDAT(m_account_id)))
    {
        total_shares += stock.NUMSHARES;
    }

    int precision = (total_shares - static_cast<int>(total_shares) != 0) ? 4 : 0;
    return CurrencyModel::toString(total_shares, m_currency, precision);
}

void StockPanel::updateHeader()
{
    double cashBalance = 0;
    double marketValue = 0;
    double InvestedVal = 0;
    // + Transfered from other accounts - Transfered to other accounts

    //Get Stock Investment Account Balance as Init Amount + sum (Value) - sum (Purchase Price)
    wxString lbl;

    if (m_account_id > -1) {
        const AccountModel::Data* account = AccountModel::instance().cache_id(m_account_id);
        if (account) {
            header_text_->SetLabelText(GetPanelTitle(*account));
            cashBalance = AccountModel::balance(account);
            std::pair<double, double> investment_balance = AccountModel::investment_balance(account);
            marketValue = investment_balance.first;
            InvestedVal = investment_balance.second;
        }
    }
    else {
        header_text_->SetLabelText(wxString::Format(_t("Stock Portfolios Overview")));
        if (m_lc) {
            m_lc->getInvestmentBalance(InvestedVal, marketValue);
        }
    }

    const wxString& diffStr = CurrencyModel::toCurrency(marketValue > InvestedVal ? marketValue - InvestedVal : InvestedVal - marketValue, m_currency);
    double diffPercents = InvestedVal != 0.0
        ? (marketValue > InvestedVal ? marketValue / InvestedVal*100.0 - 100.0 : -(marketValue / InvestedVal*100.0 - 100.0))
        : 0.0;
    lbl = wxString::Format("%s     %s     %s     %s     %s (%s %%)"
        , wxString::Format(_t("Total: %s"), CurrencyModel::toCurrency(marketValue + cashBalance, m_currency))
        , wxString::Format(_t("Cash Balance: %s"), CurrencyModel::toCurrency(cashBalance, m_currency))
        , wxString::Format(_t("Market Value: %s"), CurrencyModel::toCurrency(marketValue, m_currency))
        , wxString::Format(_t("Invested: %s"), CurrencyModel::toCurrency(InvestedVal, m_currency))
        , wxString::Format(marketValue > InvestedVal ? _t("Gain: %s") : _t("Loss: %s"), diffStr)
        , CurrencyModel::toString(diffPercents, m_currency, 2));

    header_total_->SetLabelText(lbl);
    this->Layout();
}

void StockPanel::OnDeleteStocks(wxCommandEvent& event)
{
    m_lc->OnDeleteStocks(event);
}

void StockPanel::OnMoveStocks(wxCommandEvent& event)
{
    m_lc->OnMoveStocks(event);
}

void StockPanel::OnNewStocks(wxCommandEvent& event)
{
    m_lc->OnNewStocks(event);
}

void StockPanel::OnEditStocks(wxCommandEvent& event)
{
    m_lc->OnEditStocks(event);
}

void StockPanel::OnOpenAttachment(wxCommandEvent& event)
{
    m_lc->OnOpenAttachment(event);
}

void StockPanel::OnRefreshQuotes(wxCommandEvent& WXUNUSED(event))
{
    wxString sError = "";
    bool ok = onlineQuoteRefresh(sError);
    if (ok)
    {
        const wxString header = _t("Stock prices updated successfully");
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
bool StockPanel::onlineQuoteRefresh(wxString& msg)
{
    wxString base_currency_symbol;
    if (!CurrencyModel::GetBaseCurrencySymbol(base_currency_symbol))
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
    StockModel::Data_Set stock_list = StockModel::instance().get_all();
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

    StockHistoryModel::instance().Savepoint();
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
            StockModel::instance().save(&s);
            StockHistoryModel::instance().addUpdate(s.SYMBOL
                , wxDate::Now(), dPrice, StockHistoryModel::ONLINE);
        }
    }
    StockHistoryModel::instance().ReleaseSavepoint();

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
    InfoModel::instance().setString("STOCKS_LAST_REFRESH_DATETIME", strLastUpdate_);

    return true;
}

void StockPanel::updateExtraStocksData(int selectedIndex)
{
    enableEditDeleteButtons(selectedIndex >= 0);
    stock_details_->SetLabelText(selectedIndex >= 0 ? m_lc->getStockInfo(selectedIndex, m_account_id > -1) : "");
}

wxString StockList::getStockInfo(int selectedIndex, bool addtotal) const
{
    int purchasedTime = 0;
    double stocktotalnumShares = 0;
    double stockavgPurchasePrice = 0;
    for (const auto& s: StockModel::instance().find(StockModel::SYMBOL(m_stocks[selectedIndex].SYMBOL)))
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
        ? wxString::Format("(%s %%)", CurrencyModel::toStringNoFormatting(
            ((stockCurrentPrice / stockPurchasePrice - 1.0) * 100.0), nullptr, 2))
        : "";
    double stocktotalPercentage = (stockCurrentPrice / stockavgPurchasePrice - 1.0)*100.0;
    double stocktotalgainloss = stocktotalDifference * stocktotalnumShares;

    const wxString& sPurchasePrice = CurrencyModel::toCurrency(stockPurchasePrice, m_stock_panel->m_currency, 4);
    const wxString& sAvgPurchasePrice = CurrencyModel::toCurrency(stockavgPurchasePrice, m_stock_panel->m_currency, 4);
    const wxString& sCurrentPrice = CurrencyModel::toCurrency(stockCurrentPrice, m_stock_panel->m_currency, 4);
    const wxString& sDifference = CurrencyModel::toCurrency(stockDifference, m_stock_panel->m_currency, 4);
    const wxString& sTotalDifference = CurrencyModel::toCurrency(stocktotalDifference);

    wxString miniInfo = "";
    if (m_stocks[selectedIndex].SYMBOL != "")
        miniInfo << "\t" << wxString::Format(_t("Symbol: %s"), m_stocks[selectedIndex].SYMBOL) << "\t\t";
    miniInfo << wxString::Format(_t("Total: %s"), " (" + sTotalNumShares + ") ");
    m_stock_panel->stock_details_short_->SetLabelText(miniInfo);

    //Selected share info
    wxString additionInfo = wxString::Format("This Account: |%s - %s| = %s, %s * %s = %s %s\n"
        , sCurrentPrice, sPurchasePrice, sDifference
        , sDifference, sNumShares
        , CurrencyModel::toCurrency(GetGainLoss(selectedIndex), m_stock_panel->m_currency)
        , stockPercentage);

    //Summary for account for selected symbol
    if (addtotal && purchasedTime > 1)
    {
        additionInfo += wxString::Format( "All Accounts: |%s - %s| = %s, %s * %s = %s ( %s %% )\n%s"
            ,  sCurrentPrice, sAvgPurchasePrice, sTotalDifference
            , sTotalDifference, sTotalNumShares
            , CurrencyModel::toCurrency(stocktotalgainloss)
            , CurrencyModel::toStringNoFormatting(stocktotalPercentage, nullptr, 2)
            , OnGetItemText(selectedIndex, static_cast<long>(LIST_ID_NOTES)));
    }
    return additionInfo;
}

void StockPanel::enableEditDeleteButtons(bool en)
{
    wxButton* bN = static_cast<wxButton*>(FindWindow(wxID_NEW));
    wxButton* bE = static_cast<wxButton*>(FindWindow(wxID_EDIT));
    wxButton* bA = static_cast<wxButton*>(FindWindow(wxID_ADD));
    wxButton* bV = static_cast<wxButton*>(FindWindow(wxID_VIEW_DETAILS));
    wxButton* bD = static_cast<wxButton*>(FindWindow(wxID_DELETE));
    wxButton* bM = static_cast<wxButton*>(FindWindow(wxID_MOVE_FRAME));
    bool isaccountview = m_account_id > -1;
    if (bN) bN->Enable(!en && isaccountview);
    if (bE) bE->Enable(en);
    if (bA) bA->Enable(en && isaccountview);
    if (bV) bV->Enable(en);
    if (bD) bD->Enable(en && isaccountview);
    if (bM) bM->Enable(en && isaccountview);
    attachment_button_->Enable(en);
    if (!en)
    {
        if (PreferencesModel::instance().getShowMoneyTips())
            stock_details_->SetLabelText(wxGetTranslation(mmStockTips[rand() % (sizeof(mmStockTips) / sizeof(wxString))]));
        stock_details_short_->SetLabelText(wxString::Format(_t("Last updated %s"), strLastUpdate_));
    }
}

void StockPanel::call_dialog(int selectedIndex)
{
    StockModel::Data* stock = &m_lc->m_stocks[selectedIndex];
    StockDialog dlg(this, stock, m_account_id);
    dlg.ShowModal();
    m_lc->doRefreshItems(dlg.m_stock_id);
}

void StockPanel::DisplayAccountDetails(int64 accountID)
{

    m_account_id = accountID;

    if (m_account_id > -1){
        AccountModel::Data* account = AccountModel::instance().cache_id(m_account_id);
        m_currency = AccountModel::currency(account);
    }

    enableEditDeleteButtons(false);
    m_lc->initVirtualListControl();

}

void StockPanel::RefreshList()
{
    int64 selected_id = -1;
    if (m_lc->get_selectedIndex() > -1)
        selected_id = m_lc->m_stocks[m_lc->get_selectedIndex()].STOCKID;
    m_lc->doRefreshItems(selected_id);
}

int StockPanel::getFilter()
{
    return m_choiceFilter->GetSelection();
}
