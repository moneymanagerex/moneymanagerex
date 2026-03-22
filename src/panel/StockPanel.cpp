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
#include "dialog/TrxShareDialog.h"

class StockPanel;

BEGIN_EVENT_TABLE(StockPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,          StockPanel::onNewStocks)
    EVT_BUTTON(wxID_EDIT,         StockPanel::onEditStocks)
    EVT_BUTTON(wxID_ADD,          StockPanel::onEditStocks)
    EVT_BUTTON(wxID_VIEW_DETAILS, StockPanel::onEditStocks)
    EVT_BUTTON(wxID_DELETE,       StockPanel::onDeleteStocks)
    EVT_BUTTON(wxID_MOVE_FRAME,   StockPanel::onMoveStocks)
    EVT_BUTTON(wxID_FILE,         StockPanel::onOpenAttachment)
    EVT_BUTTON(wxID_REFRESH,      StockPanel::onRefreshQuotes)
END_EVENT_TABLE()

StockPanel::StockPanel(
    int64 account_id,
    mmGUIFrame* frame,
    wxWindow* parent_win,
    wxWindowID win_id
) :
    m_account_id(account_id),
    m_currency_n(),
    w_frame(frame)
{
    create(parent_win, win_id);
}

bool StockPanel::create(
    wxWindow* parent_win,
    wxWindowID winid,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
) {
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent_win, winid, pos, size, style, name);

    m_last_update = InfoModel::instance().getString("STOCKS_LAST_REFRESH_DATETIME", "");
    this->windowsFreezeThaw();

    const AccountData* account_n = AccountModel::instance().get_id_data_n(m_account_id);
    if (account_n)
        m_currency_n = AccountModel::instance().get_data_currency_p(*account_n);
    else
        m_currency_n = CurrencyModel::instance().get_base_data_n();

    createControls();
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

void StockPanel::createControls()
{
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    /* ---------------------- */
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY
        , wxDefaultPosition , wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    itemBoxSizer9->Add(headerPanel, 0, wxALIGN_LEFT);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    w_header_title = new wxStaticText(headerPanel, wxID_STATIC, "");
    w_header_title->SetFont(this->GetFont().Larger().Bold());

    w_filter_choice = new wxChoice(headerPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr);
    w_filter_choice->Append(_t("All"));
    w_filter_choice->Append(_t("Non-Zero Shares"));
    w_filter_choice->SetMinSize(wxSize(150, -1));
    w_filter_choice->SetSelection(0);

    w_filter_choice->Bind(wxEVT_CHOICE, [this](wxCommandEvent&)
    {
        refreshList();
    });

    w_header_total = new wxStaticText(headerPanel, wxID_STATIC, "");

    wxBoxSizer* itemBoxSizerHHeader = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerHHeader->Add(w_header_title, 1, wxALIGN_CENTER_VERTICAL | wxALL, 1);

    itemBoxSizerVHeader->Add(itemBoxSizerHHeader, 1, wxEXPAND, 1);
    itemBoxSizerVHeader->Add(w_filter_choice, g_flagsBorder1V);
    itemBoxSizerVHeader->Add(w_header_total, 1, wxALL, 1);

    /* ---------------------- */
    mmSplitterWindow* itemSplitterWindow10 = new mmSplitterWindow(this
        , wxID_ANY, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER, mmThemeMetaColour(meta::COLOR_LISTPANEL));

    w_list = new StockList(this, itemSplitterWindow10, wxID_ANY);

    wxPanel* BottomPanel = new wxPanel(itemSplitterWindow10, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    mmThemeMetaColour(BottomPanel, meta::COLOR_LISTPANEL);

    itemSplitterWindow10->SplitHorizontally(w_list, BottomPanel);
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

    w_attachment_btn = new wxBitmapButton(BottomPanel,
        wxID_FILE, mmBitmapBundle(png::CLIP, mmBitmapButtonSize), wxDefaultPosition,
        wxSize(30, bMove->GetSize().GetY())
    );
    mmToolTip(w_attachment_btn, _t("Open attachments"));
    BoxSizerHBottom->Add(w_attachment_btn, 0, wxRIGHT, 5);
    w_attachment_btn->Enable(false);

    w_refresh_btn = new wxBitmapButton(BottomPanel,
        wxID_REFRESH, mmBitmapBundle(png::LED_OFF, mmBitmapButtonSize),
        wxDefaultPosition, wxSize(30, bMove->GetSize().GetY())
    );
    w_refresh_btn->SetLabelText(_t("Refresh"));
    mmToolTip(w_refresh_btn, _t("Refresh Stock Prices from Yahoo"));
    BoxSizerHBottom->Add(w_refresh_btn, 0, wxRIGHT, 5);

    //Infobar-mini
    w_details_short = new wxStaticText(BottomPanel, wxID_STATIC, m_last_update);
    BoxSizerHBottom->Add(w_details_short, 1, wxGROW | wxTOP | wxLEFT, 5);
    //Infobar
    w_details = new wxStaticText(BottomPanel, wxID_STATIC, "",
        wxDefaultPosition, wxSize(200, -1), wxTE_MULTILINE | wxTE_WORDWRAP
    );
    BoxSizerVBottom->Add(w_details, g_flagsExpandBorder1);

    updateExtraStocksData(-1);

    updateHeader();
}

int StockPanel::addStockTransaction(int selectedIndex)
{
    StockData* stock = &w_list->m_stock_a[selectedIndex];
    TrxShareDialog dlg(this, stock);
    int result = dlg.ShowModal();
    if (result == wxID_OK)
    {
        w_list->doRefreshItems(dlg.m_stock_id);
        updateExtraStocksData(selectedIndex);
    }
    return result;
}

void StockPanel::onListItemActivated(int selectedIndex)
{
    call_dialog(selectedIndex);
    updateExtraStocksData(selectedIndex);
}

//TODO: improve View Stock Transactions
void StockPanel::viewStockTransactions(int selectedIndex)
{
    StockData* stock = &w_list->m_stock_a[selectedIndex];

    wxDialog dlg(this, wxID_ANY,
        _t("View Stock Transactions") + ": " + (m_account_id > -1
            ? (AccountModel::instance().get_id_name(stock->m_account_id_n) + " - ")
            : ""
        ) + stock->m_symbol,
        wxDefaultPosition, wxSize(800, 400),
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    );

    dlg.SetIcon(mmex::getProgramIcon());
    wxWindow* parent_win = dlg.GetMainWindowOfCompositeControl();
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);

    // Initialize list control
    wxListCtrl* stockTxnListCtrl = initStockTxnListCtrl(parent_win);
    topsizer->Add(stockTxnListCtrl, wxSizerFlags(g_flagsExpand).TripleBorder());

    // Load stock transactions
    loadStockTransactions(
        stockTxnListCtrl,
        m_account_id == -1 ? stock->m_symbol : "",
        stock->m_id
    );

    // Bind list events
    bindListEvents(stockTxnListCtrl);

    // Add buttons
    wxSizer* buttonSizer = dlg.CreateSeparatedButtonSizer(wxOK);
    if (buttonSizer) {
        topsizer->Add(
            buttonSizer,
            wxSizerFlags().Expand().DoubleBorder(wxLEFT | wxRIGHT | wxBOTTOM)
        );
    }

    dlg.SetSizerAndFit(topsizer);
    dlg.SetInitialSize(wxSize(800, 400)); // Set default size
    dlg.Center();
    dlg.ShowModal();
    refreshList();
}

// Initialize the list control
wxListCtrl* StockPanel::initStockTxnListCtrl(wxWindow* parent_win)
{
    wxListCtrl* listCtrl = new wxListCtrl(
        parent_win, wxID_ANY,
        wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_AUTOARRANGE
    );

    listCtrl->AppendColumn(_t("Date"),       wxLIST_FORMAT_LEFT,  100);
    listCtrl->AppendColumn(_t("Lot"),        wxLIST_FORMAT_LEFT,  140);
    listCtrl->AppendColumn(_t("Shares"),     wxLIST_FORMAT_RIGHT, 100);
    listCtrl->AppendColumn(_t("Trade Type"), wxLIST_FORMAT_LEFT,  80);
    listCtrl->AppendColumn(_t("Price"),      wxLIST_FORMAT_RIGHT, 100);
    listCtrl->AppendColumn(_t("Commission"), wxLIST_FORMAT_RIGHT, 100);
    listCtrl->AppendColumn(_t("Total"),      wxLIST_FORMAT_RIGHT, 100);

    return listCtrl;
}

// Load stock transactions into the list control
void StockPanel::loadStockTransactions(wxListCtrl* listCtrl, wxString symbol, int64 stockId)
{
    TrxLinkModel::DataA tl_a;
    TrxModel::DataA trx_a;
    if (symbol.IsEmpty()) {
        tl_a = TrxLinkModel::instance().find_ref_data_a(StockModel::s_ref_type, stockId);
    }
    else {
        // search for all
        tl_a = TrxLinkModel::instance().find_symbol_data_a(symbol);
    }

    for (const auto& tl_d : tl_a) {
        const TrxData* trx_n = TrxModel::instance().get_id_data_n(tl_d.m_trx_id);
        if (trx_n && !trx_n->is_deleted()) {
            trx_a.push_back(*trx_n);
        }
    }
    std::stable_sort(trx_a.begin(), trx_a.end(), TrxData::SorterByDateTime());

    int row = 0;
    for (const auto& trx_d : trx_a) {
        const TrxShareData* ts_n = TrxShareModel::instance().get_trxId_data_n(trx_d.m_id);
        if (!ts_n || (ts_n->m_number <= 0 && ts_n->m_price <= 0))
            continue;
        long index = listCtrl->InsertItem(row++, "");
        listCtrl->SetItemData(index, trx_d.m_id.GetValue());
        fillListRow(listCtrl, index, trx_d, *ts_n);
    }
}

// Fill list row with stock transaction data
void StockPanel::fillListRow(
    wxListCtrl* listCtrl,
    long index,
    const TrxData& trx_d,
    const TrxShareData& ts_d
) {
    listCtrl->SetItem(index, 0, mmGetDateTimeForDisplay(trx_d.m_date_time.isoDateTime()));
    listCtrl->SetItem(index, 1, ts_d.m_lot);

    int precision = ts_d.m_number == floor(ts_d.m_number) ? 0 : PrefModel::instance().getSharePrecision();
    listCtrl->SetItem(index, 2, wxString::FromDouble(ts_d.m_number, precision));
    listCtrl->SetItem(index, 3, wxGetTranslation(trx_d.m_type.trade_name()));
    listCtrl->SetItem(index, 4, wxString::FromDouble(ts_d.m_price, PrefModel::instance().getSharePrecision()));
    listCtrl->SetItem(index, 5, wxString::FromDouble(ts_d.m_commission, 2));
    double total = ts_d.m_number * ts_d.m_price + ts_d.m_commission;
    listCtrl->SetItem(index, 6, wxString::FromDouble(total, 2));
}

// Bind list control events
void StockPanel::bindListEvents(wxListCtrl* listCtrl)
{
    listCtrl->Bind(wxEVT_LIST_ITEM_ACTIVATED, [listCtrl, this](wxListEvent& event) {
        long index = event.GetIndex();
        TrxData* trx_n = TrxModel::instance().unsafe_get_id_data_n(event.GetData());
        if (!trx_n)
            return;

        const TrxLinkData* tl_n = TrxLinkModel::instance().get_trx_data_n(trx_n->m_id);
        TrxLinkData tl_d = tl_n ? *tl_n : TrxLinkData();
        TrxShareDialog dlg(listCtrl, &tl_d, trx_n);
        dlg.ShowModal();

        // Update the modified row
        const TrxShareData* ts_n = TrxShareModel::instance().get_trxId_data_n(trx_n->m_id);
        if (ts_n) {
            this->fillListRow(listCtrl, index, *trx_n, *ts_n);
        }

        // Re-sort the list
        // FIXME: change type to int64
        listCtrl->SortItems([](wxIntPtr item1, wxIntPtr item2, wxIntPtr) -> int {
            auto date1 = TrxModel::instance().get_id_data_n(item1)->m_date_time.getDateTime();
            auto date2 = TrxModel::instance().get_id_data_n(item2)->m_date_time.getDateTime();
            return date1.IsEarlierThan(date2) ? -1 : (date1.IsLaterThan(date2) ? 1 : 0);
        }, 0);
    });

    listCtrl->Bind(wxEVT_CHAR, [listCtrl, this](wxKeyEvent& event) {
        if (event.GetKeyCode() == WXK_CONTROL_C) {
            copySelectedRowsToClipboard(listCtrl);
        } else if (event.GetKeyCode() == WXK_CONTROL_A) {
            for (int row = 0; row < listCtrl->GetItemCount(); row++)
                listCtrl->SetItemState(row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
    });
}

// Copy selected rows to the clipboard
void StockPanel::copySelectedRowsToClipboard(wxListCtrl* listCtrl)
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

wxString StockPanel::getPanelTitle(const AccountData& account) const
{
    return wxString::Format(_t("Stock Portfolio: %s"), account.m_name);
}

wxString StockPanel::buildPage() const
{
    const AccountData* account = AccountModel::instance().get_id_data_n(m_account_id);
    return w_list->buildPage((account ? getPanelTitle(*account) : ""));
}

const wxString StockPanel::totalShares()
{
    double total_shares = 0;
    for (const auto& stock : StockModel::instance().find(StockCol::HELDAT(m_account_id))) {
        total_shares += stock.m_num_shares;
    }

    int precision = (total_shares - static_cast<int>(total_shares) != 0) ? 4 : 0;
    return CurrencyModel::instance().toString(total_shares, m_currency_n, precision);
}

void StockPanel::updateHeader()
{
    double cashBalance = 0;
    double marketValue = 0;
    double InvestedVal = 0;
    bool today = PrefModel::instance().getIgnoreFutureTransactionsHomePage();
    // + Transfered from other accounts - Transfered to other accounts

    //Get Stock Investment Account Balance as Init Amount + sum (Value) - sum (Purchase Price)
    wxString lbl;

    if (m_account_id > -1) {
        const AccountData* account_n = AccountModel::instance().get_id_data_n(m_account_id);
        if (account_n) {
            w_header_title->SetLabelText(getPanelTitle(*account_n));

            if (today) {
                cashBalance = AccountModel::instance().get_data_balance_to_date(*account_n, mmDate::today());
            }
            else {
                cashBalance = AccountModel::instance().get_data_balance(*account_n);
            }
            std::pair<double, double> investment_balance = AccountModel::instance().get_data_investment_balance(*account_n);
            marketValue = investment_balance.first;
            InvestedVal = investment_balance.second;
        }
    }
    else {
        w_header_title->SetLabelText(wxString::Format(_t("Stock Portfolios Overview")));
        if (w_list) {
            w_list->getInvestmentBalance(InvestedVal, marketValue);
        }
    }

    const wxString& diffStr = CurrencyModel::instance().toCurrency(marketValue > InvestedVal ? marketValue - InvestedVal : InvestedVal - marketValue, m_currency_n);
    double diffPercents = InvestedVal != 0.0
        ? (marketValue > InvestedVal ? marketValue / InvestedVal*100.0 - 100.0 : -(marketValue / InvestedVal*100.0 - 100.0))
        : 0.0;
    lbl = wxString::Format("%s     %s     %s     %s     %s (%s %%)"
        , wxString::Format(_t("Total: %s"), CurrencyModel::instance().toCurrency(marketValue + cashBalance, m_currency_n))
        , wxString::Format(_t("Cash Balance: %s"), CurrencyModel::instance().toCurrency(cashBalance, m_currency_n))
        , wxString::Format(_t("Market Value: %s"), CurrencyModel::instance().toCurrency(marketValue, m_currency_n))
        , wxString::Format(_t("Invested: %s"), CurrencyModel::instance().toCurrency(InvestedVal, m_currency_n))
        , wxString::Format(marketValue > InvestedVal ? _t("Gain: %s") : _t("Loss: %s"), diffStr)
        , CurrencyModel::instance().toString(diffPercents, m_currency_n, 2));

    w_header_total->SetLabelText(lbl);
    this->Layout();
}

void StockPanel::onRefreshQuotes(wxCommandEvent& WXUNUSED(event))
{
    wxString sError = "";
    bool ok = onlineQuoteRefresh(sError);
    if (ok) {
        const wxString header = _t("Stock prices updated successfully");
        w_details->SetLabelText(header);
        w_details_short->SetLabelText(wxString::Format(_t("Last updated %s"), m_last_update));
        wxMessageDialog msgDlg(this, sError, header);
        msgDlg.ShowModal();
        w_refresh_btn->SetBitmapLabel(mmBitmapBundle(png::LED_GREEN, mmBitmapButtonSize));
    }
    else {
        w_refresh_btn->SetBitmapLabel(mmBitmapBundle(png::LED_RED, mmBitmapButtonSize));
        w_details->SetLabelText(sError);
        w_details_short->SetLabelText(_t("Error"));
        mmErrorDialogs::MessageError(this, sError, _t("Error"));
    }
}

// Trigger a quote download
bool StockPanel::onlineQuoteRefresh(wxString& msg)
{
    wxString base_currency_symbol;
    if (!CurrencyModel::instance().get_base_symbol(base_currency_symbol)) {
        msg = _t("Unable to find base currency symbol!");
        return false;
    }

    if (w_list->m_stock_a.empty()) {
        msg = _t("Nothing to update");
        return false;
    }

    std::map<wxString, double> symbols;
    StockModel::DataA stock_a = StockModel::instance().find_all();
    for (const auto& stock_d : stock_a) {
        const wxString symbol = stock_d.m_symbol.Upper();
        if (symbol.IsEmpty()) continue;
        symbols[symbol] = stock_d.m_purchase_value;
    }

    w_refresh_btn->SetBitmapLabel(mmBitmapBundle(png::LED_YELLOW, mmBitmapButtonSize));
    w_details->SetLabelText(_tu("Connecting…"));

    std::map<wxString, double > stocks_data;
    bool ok = get_yahoo_prices(
        symbols, stocks_data, base_currency_symbol, msg, yahoo_price_type::SHARES
    );
    if (!ok) {
        return false;
    }

    std::map<wxString, double> nonYahooSymbols;

    StockHistoryModel::instance().db_savepoint();
    for (auto& stock_d : stock_a) {
        std::map<wxString, double>::const_iterator it = stocks_data.find(stock_d.m_symbol.Upper());
        if (it == stocks_data.end()) {
            nonYahooSymbols[stock_d.m_symbol.Upper()] = 0;
            continue;
        }

        double dPrice = it->second;

        if (dPrice != 0) {
            msg += wxString::Format("%s\t: %0.6f -> %0.6f\n",
                stock_d.m_symbol, stock_d.m_current_price, dPrice
            );
            stock_d.m_current_price = dPrice;
            StockModel::instance().save_data_n(stock_d);
            StockHistoryModel::instance().save_record(
                stock_d.m_symbol,
                mmDate::today(),
                dPrice,
                UpdateType(UpdateType::e_online)
            );
        }
    }
    StockHistoryModel::instance().db_release_savepoint();

    for (const auto& entry : nonYahooSymbols) {
        msg += wxString::Format("%s\t: %s\n", entry.first, _t("Missing"));
    }

    // Now refresh the display
    refreshList();

    // We are done!
    m_last_refresh = wxDateTime::Now();
    m_refresh_status = true;

    m_last_update.Printf(_t("%1$s on %2$s"),
        m_last_refresh.FormatTime(),
        mmGetDateTimeForDisplay(m_last_refresh.FormatISODate())
    );
    InfoModel::instance().setString("STOCKS_LAST_REFRESH_DATETIME", m_last_update);

    return true;
}

void StockPanel::updateExtraStocksData(int selectedIndex)
{
    enableEditDeleteButtons(selectedIndex >= 0);
    w_details->SetLabelText(selectedIndex >= 0 ? w_list->getStockInfo(selectedIndex, m_account_id > -1) : "");
}

void StockPanel::enableEditDeleteButtons(bool en)
{
    wxButton* bN = static_cast<wxButton*>(FindWindow(wxID_NEW));
    wxButton* bE = static_cast<wxButton*>(FindWindow(wxID_EDIT));
    wxButton* bA = static_cast<wxButton*>(FindWindow(wxID_ADD));
    wxButton* bV = static_cast<wxButton*>(FindWindow(wxID_VIEW_DETAILS));
    wxButton* bD = static_cast<wxButton*>(FindWindow(wxID_DELETE));
    wxButton* bM = static_cast<wxButton*>(FindWindow(wxID_MOVE_FRAME));
    bool isaccountview = (m_account_id > -1);

    if (bN)
        bN->Enable(!en && isaccountview);
    if (bE)
        bE->Enable(en);
    if (bA)
        bA->Enable(en && isaccountview);
    if (bV)
        bV->Enable(en);
    if (bD)
        bD->Enable(en && isaccountview);
    if (bM)
        bM->Enable(en && isaccountview);

    w_attachment_btn->Enable(en);
    if (!en) {
        if (PrefModel::instance().getShowMoneyTips())
            w_details->SetLabelText(wxGetTranslation(
                mmStockTips[rand() % (sizeof(mmStockTips) / sizeof(wxString))]
            ));
        w_details_short->SetLabelText(wxString::Format(_t("Last updated %s"),
            m_last_update
        ));
    }
}

void StockPanel::call_dialog(int selectedIndex)
{
    StockData* stock = &w_list->m_stock_a[selectedIndex];
    StockDialog dlg(this, stock, m_account_id);
    dlg.ShowModal();
    w_list->doRefreshItems(dlg.m_stock_id);
}

void StockPanel::displayAccountDetails(int64 account_id)
{

    m_account_id = account_id;

    if (m_account_id > -1){
        const AccountData* account_n = AccountModel::instance().get_id_data_n(m_account_id);
        m_currency_n = AccountModel::instance().get_data_currency_p(*account_n);
    }

    enableEditDeleteButtons(false);
    w_list->initVirtualListControl();

}

void StockPanel::refreshList()
{
    int64 selected_id = -1;
    if (w_list->get_selectedIndex() > -1)
        selected_id = w_list->m_stock_a[w_list->get_selectedIndex()].m_id;
    w_list->doRefreshItems(selected_id);
}

int StockPanel::getFilter()
{
    return w_filter_choice->GetSelection();
}
