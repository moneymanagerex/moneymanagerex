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

#include "stockspanel.h"
#include "stocks_list.h"
#include "attachmentdialog.h"
#include "constants.h"
#include "images_list.h"
#include "mmSimpleDialogs.h"
#include "stockdialog.h"
#include "sharetransactiondialog.h"
#include "util.h"

#include "model/allmodel.h"

enum {
    IDC_PANEL_STOCKS_LISTCTRL = wxID_HIGHEST + 1900,
    MENU_TREEPOPUP_EDIT,
    MENU_TREEPOPUP_ADDTRANS,
    MENU_TREEPOPUP_VIEWTRANS,
    MENU_TREEPOPUP_DELETE,
    MENU_TREEPOPUP_NEW,
    MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS,
};

enum class ico { GAIN, LOSS, ARROW_UP, ARROW_DOWN };

/*******************************************************/

wxBEGIN_EVENT_TABLE(StocksListCtrl, mmListCtrl)
    EVT_LEFT_DOWN(StocksListCtrl::OnListLeftClick)
    EVT_RIGHT_DOWN(StocksListCtrl::OnMouseRightClick)

    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, StocksListCtrl::OnListItemActivated)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,  StocksListCtrl::OnListItemSelected)
    EVT_LIST_KEY_DOWN(wxID_ANY,       StocksListCtrl::OnListKeyDown)

    EVT_MENU(MENU_TREEPOPUP_NEW,                  StocksListCtrl::OnNewStocks)
    EVT_MENU(MENU_TREEPOPUP_EDIT,                 StocksListCtrl::OnEditStocks)
    EVT_MENU(MENU_TREEPOPUP_ADDTRANS,             StocksListCtrl::OnEditStocks)
    EVT_MENU(MENU_TREEPOPUP_VIEWTRANS,            StocksListCtrl::OnEditStocks)
    EVT_MENU(MENU_TREEPOPUP_DELETE,               StocksListCtrl::OnDeleteStocks)
    EVT_MENU(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, StocksListCtrl::OnOrganizeAttachments)
    EVT_MENU(wxID_INDEX,                          StocksListCtrl::OnStockWebPage)
wxEND_EVENT_TABLE()

const std::vector<ListColumnInfo> StocksListCtrl::LIST_INFO = {
    { LIST_ID_ICON,           true, _n("Icon"),                 25,  _FC, false },
    { LIST_ID_ID,             true, _n("ID"),                   _WA, _FL, true },
    { LIST_ID_DATE,           true, _n("*Date"),                _WH, _FL, true },
    { LIST_ID_NAME,           true, _n("Company Name"),         _WH, _FL, true },
    { LIST_ID_SYMBOL,         true, _n("Symbol"),               _WH, _FL, true },
    { LIST_ID_NUMBER,         true, _n("Share Total"),          _WH, _FR, true },
    { LIST_ID_PRICE,          true, _n("Avg Share Price"),      _WH, _FR, true },
    { LIST_ID_VALUE,          true, _n("Total Cost"),           _WH, _FR, true },
    { LIST_ID_REAL_GAIN_LOSS, true, _n("Realized Gain/Loss"),   _WH, _FR, true },
    { LIST_ID_GAIN_LOSS,      true, _n("Unrealized Gain/Loss"), _WH, _FR, true },
    { LIST_ID_CURRENT,        true, _n("Curr. Share Price"),    _WH, _FR, true },
    { LIST_ID_CURRVALUE,      true, _n("Curr. Total Value"),    _WH, _FR, true },
    { LIST_ID_PRICEDATE,      true, _n("Price Date"),           _WH, _FL, true },
    { LIST_ID_COMMISSION,     true, _n("Commission"),           _WH, _FR, true },
    { LIST_ID_NOTES,          true, _n("Notes"),                _WH, _FL, true },
};

StocksListCtrl::StocksListCtrl(
    mmStocksPanel* cp, wxWindow *parent, wxWindowID winid
) :
    mmListCtrl(parent, winid),
    m_stock_panel(cp),
    m_attr1(new wxListItemAttr(
            mmThemeMetaColour(meta::COLOR_REPORT_DEBIT),
            mmThemeMetaColour(meta::COLOR_LISTALT0),
            GetFont()
        )),
    m_attr2(new wxListItemAttr(
            mmThemeMetaColour(meta::COLOR_REPORT_DEBIT),
            mmThemeMetaColour(meta::COLOR_LIST),
            GetFont()
        ))
{
    wxVector<wxBitmapBundle> images;
    images.push_back(mmBitmapBundle(png::PROFIT));
    images.push_back(mmBitmapBundle(png::LOSS));
    images.push_back(mmBitmapBundle(png::DOWNARROW));
    images.push_back(mmBitmapBundle(png::UPARROW));

    SetSmallImages(images);
    mmThemeMetaColour(this, meta::COLOR_LISTPANEL);

    m_setting_name = "STOCKS";
    o_col_order_prefix = "STOCKS";
    o_col_width_prefix = "STOCKS_COL";
    o_sort_prefix = "STOCKS";
    m_col_info_id = LIST_INFO;
    m_col_id_nr = ListColumnInfo::getListId(LIST_INFO);
    m_sort_col_id = { col_sort() };
    createColumns();

    initVirtualListControl();
    if (!m_stocks.empty()) {
        EnsureVisible(m_stocks.size() - 1);
    }
}

StocksListCtrl::~StocksListCtrl()
{
}

int StocksListCtrl::getSortIcon(bool asc) const
{
    return asc ? static_cast<int>(ico::ARROW_DOWN) : static_cast<int>(ico::ARROW_UP);
}

void StocksListCtrl::OnMouseRightClick(wxMouseEvent& event)
{
    if (m_selected_row > -1)
        SetItemState(m_selected_row, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    int Flags = wxLIST_HITTEST_ONITEM;
    m_selected_row = HitTest(wxPoint(event.m_x, event.m_y), Flags);

    if (m_selected_row >= 0)
    {
        SetItemState(m_selected_row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selected_row, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
    }
    m_stock_panel->OnListItemSelected(m_selected_row);

    bool enable_menu_item = m_selected_row > -1 && m_stock_panel->m_account_id > -1;

    wxMenu menu;
    menu.Append(MENU_TREEPOPUP_NEW, _tu("&New Stock Investment…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ADDTRANS, _tu("&Adjust Trade"));
    menu.Append(MENU_TREEPOPUP_VIEWTRANS, _t("&View Stock Transactions"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_EDIT, _tu("&Edit Stock Investment…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_DELETE, _tu("&Delete Stock Investment…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, _tu("&Organize Attachments…"));
    menu.Append(wxID_INDEX, _t("Stock &Web Page"));

    menu.Enable(MENU_TREEPOPUP_NEW, m_stock_panel->m_account_id > -1);
    menu.Enable(MENU_TREEPOPUP_EDIT,  m_selected_row > -1);
    menu.Enable(MENU_TREEPOPUP_ADDTRANS, enable_menu_item && m_stocks[m_selected_row].NUMSHARES > 0);
    menu.Enable(MENU_TREEPOPUP_VIEWTRANS, m_selected_row > -1);
    menu.Enable(MENU_TREEPOPUP_DELETE, enable_menu_item);
    menu.Enable(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, enable_menu_item);
    menu.Enable(wxID_INDEX, m_selected_row > -1);

    PopupMenu(&menu, event.GetPosition());

    this->SetFocus();
}

wxString StocksListCtrl::OnGetItemText(long item, long col_nr) const
{
    int col_id = getColId_Nr(static_cast<int>(col_nr));
    switch (col_id) {
    case LIST_ID_ID:
        return wxString::Format("%lld", m_stocks[item].STOCKID).Trim();
    case LIST_ID_DATE:
        return mmGetDateTimeForDisplay(m_stocks[item].PURCHASEDATE);
    case LIST_ID_NAME:
        return m_stocks[item].STOCKNAME;
    case LIST_ID_SYMBOL:
        return m_stocks[item].SYMBOL;
    case LIST_ID_NUMBER: {
        int precision = m_stocks[item].NUMSHARES == floor(m_stocks[item].NUMSHARES) ? 0 : 4;
        return Model_Currency::toString(m_stocks[item].NUMSHARES, m_stock_panel->m_currency, precision);
    }
    case LIST_ID_PRICE:
        return Model_Currency::toString(m_stocks[item].PURCHASEPRICE, m_stock_panel->m_currency, 4);
    case LIST_ID_VALUE:
        return Model_Currency::toString(m_stocks[item].VALUE, m_stock_panel->m_currency);
    case LIST_ID_REAL_GAIN_LOSS:
        return Model_Currency::toString(GetRealGainLoss(item), m_stock_panel->m_currency);
    case LIST_ID_GAIN_LOSS:
        return Model_Currency::toString(GetGainLoss(item), m_stock_panel->m_currency);
    case LIST_ID_CURRENT:
        return Model_Currency::toString(m_stocks[item].CURRENTPRICE, m_stock_panel->m_currency, 4);
    case LIST_ID_CURRVALUE:
        return Model_Currency::toString(Model_Stock::CurrentValue(m_stocks[item]), m_stock_panel->m_currency);
    case LIST_ID_PRICEDATE:
        return mmGetDateTimeForDisplay(Model_Stock::instance().lastPriceDate(&m_stocks[item]));
    case LIST_ID_COMMISSION:
        return Model_Currency::toString(m_stocks[item].COMMISSION, m_stock_panel->m_currency);
    case LIST_ID_NOTES: {
        wxString full_notes = m_stocks[item].NOTES;
        full_notes.Replace("\n", " ");
        if (Model_Attachment::NrAttachments(Model_Stock::refTypeName, m_stocks[item].STOCKID))
            full_notes.Prepend(mmAttachmentManage::GetAttachmentNoteSign());
        return full_notes;
    }
    default:
        return wxEmptyString;
    }
}

double StocksListCtrl::GetGainLoss(long item) const
{
    return getGainLoss(m_stocks[item]);
}

double StocksListCtrl::getGainLoss(const Model_Stock::Data& stock)
{
    return Model_Stock::CurrentValue(stock) - stock.VALUE;
}

double StocksListCtrl::GetRealGainLoss(long item) const
{
    return getRealGainLoss(m_stocks[item]);
}

double StocksListCtrl::getRealGainLoss(const Model_Stock::Data& stock)
{
    return Model_Stock::RealGainLoss(stock);
}

void StocksListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selected_row = event.GetIndex();
    m_stock_panel->OnListItemSelected(m_selected_row);
}

void mmStocksPanel::OnListItemSelected(int selectedIndex)
{
    updateExtraStocksData(selectedIndex);
    enableEditDeleteButtons(selectedIndex >= 0);
}

void StocksListCtrl::OnListLeftClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    long index = HitTest(wxPoint(event.m_x, event.m_y), Flags);
    if (index == -1)
    {
        m_selected_row = -1;
        m_stock_panel->OnListItemSelected(m_selected_row);
    }
    event.Skip();
}

int StocksListCtrl::OnGetItemImage(long item) const
{
    /* Returns the icon to be shown for each entry */
    double val = GetGainLoss(item);
    if (val > 0.0) return static_cast<int>(ico::GAIN);
    else if (val < 0.0) return static_cast<int>(ico::LOSS);
    else return -1;
}

void StocksListCtrl::OnListKeyDown(wxListEvent& event)
{
    switch (event.GetKeyCode())
    {
        case WXK_DELETE:
            {
                wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED,  MENU_TREEPOPUP_DELETE);
                OnDeleteStocks(evt);
            }
            break;

        default:
            event.Skip();
            break;
    }
}

void StocksListCtrl::OnNewStocks(wxCommandEvent& /*event*/)
{
    mmStockDialog dlg(this, nullptr, m_stock_panel->m_account_id);
    dlg.ShowModal();
    if (Model_Stock::instance().get(dlg.m_stock_id))
    {
        doRefreshItems(dlg.m_stock_id);
        m_stock_panel->m_frame->RefreshNavigationTree();
    }
}

void StocksListCtrl::OnDeleteStocks(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    wxMessageDialog msgDlg(this, _t("Do you want to delete the stock investment?")
        , _t("Confirm Stock Investment Deletion")
        , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        Model_Stock::instance().remove(m_stocks[m_selected_row].STOCKID);
        mmAttachmentManage::DeleteAllAttachments(Model_Stock::refTypeName, m_stocks[m_selected_row].STOCKID);
        Model_Translink::RemoveTransLinkRecords<Model_Stock>(m_stocks[m_selected_row].STOCKID);
        DeleteItem(m_selected_row);
        doRefreshItems(-1);
        m_stock_panel->m_frame->RefreshNavigationTree();
    }
}

void StocksListCtrl::OnMoveStocks(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    const auto& accounts = Model_Account::instance().find(
        Model_Account::ACCOUNTTYPE(Model_Account::TYPE_NAME_INVESTMENT));
    if (accounts.empty()) return;

    const Model_Account::Data* from_account = Model_Account::instance().get(m_stock_panel->m_account_id);
    wxString headerMsg = wxString::Format(_t("Moving Transaction from %s to"), from_account->ACCOUNTNAME);
    mmSingleChoiceDialog scd(this, _t("Select the destination Account "), headerMsg , accounts);

    int64 toAccountID = -1;
    int error_code = scd.ShowModal();
    if (error_code == wxID_OK)
    {
        wxString acctName = scd.GetStringSelection();
        const Model_Account::Data* to_account = Model_Account::instance().get(acctName);
        toAccountID = to_account->ACCOUNTID;
    }

    if ( toAccountID != -1 )
    {
        Model_Stock::Data* stock = Model_Stock::instance().get(m_stocks[m_selected_row].STOCKID);
        stock->HELDAT = toAccountID;
        Model_Stock::instance().save(stock);

        DeleteItem(m_selected_row);
        m_stock_panel->m_frame->RefreshNavigationTree();
    }

    if (error_code == wxID_OK)
        doRefreshItems(-1);
}

void StocksListCtrl::OnEditStocks(wxCommandEvent& event)
{
    if (m_selected_row < 0) return;

    wxListEvent evt(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, event.GetId());
    AddPendingEvent(evt);
}

void StocksListCtrl::OnOrganizeAttachments(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;

    wxString RefType = Model_Stock::refTypeName;
    int64 RefId = m_stocks[m_selected_row].STOCKID;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();

    doRefreshItems(RefId);
}

void StocksListCtrl::OnStockWebPage(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;
    const wxString stockSymbol = m_stocks[m_selected_row].SYMBOL;

    if (!stockSymbol.IsEmpty())
    {
        const wxString& stockURL = Model_Infotable::instance().getString("STOCKURL", mmex::weblink::DefStockUrl);
        const wxString& httpString = wxString::Format(stockURL, stockSymbol);
        wxLaunchDefaultBrowser(httpString);
    }
}

void StocksListCtrl::OnOpenAttachment(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;

    wxString RefType = Model_Stock::refTypeName;
    int64 RefId = m_stocks[m_selected_row].STOCKID;

    mmAttachmentManage::OpenAttachmentFromPanelIcon(this, RefType, RefId);
    doRefreshItems(RefId);
}

void StocksListCtrl::OnListItemActivated(wxListEvent& event)
{
    if ((event.GetId() == wxID_ADD) || (event.GetId() == MENU_TREEPOPUP_ADDTRANS))
    {
        if (m_stock_panel->AddStockTransaction(m_selected_row) == wxID_OK)
        {
            m_stock_panel->m_frame->RefreshNavigationTree();
        }
    }
    else if ((event.GetId() == wxID_VIEW_DETAILS) || (event.GetId() == MENU_TREEPOPUP_VIEWTRANS))
    {
        m_stock_panel->ViewStockTransactions(m_selected_row);
    }
    else
    {
        m_stock_panel->OnListItemActivated(m_selected_row);
    }
}

void StocksListCtrl::OnColClick(wxListEvent& event)
{
    int col_nr = (event.GetId() == MENU_HEADER_SORT) ? m_sel_col_nr : event.GetColumn();
    if (!isValidColNr(col_nr))
        return;
    int col_id = getColId_Nr(col_nr);
    if (!m_col_info_id[col_id].sortable)
        return;

    if (m_sort_col_id[0] != col_id)
        m_sort_col_id[0] = col_id;
    else if (event.GetId() != MENU_HEADER_SORT)
        m_sort_asc[0] = !m_sort_asc[0];
    updateSortIcon();
    savePreferences();

    int64 trx_id = -1;
    if (m_selected_row>=0) trx_id = m_stocks[m_selected_row].STOCKID;
    doRefreshItems(trx_id);
    m_stock_panel->OnListItemSelected(-1);
}

void StocksListCtrl::doRefreshItems(int64 trx_id)
{
    int selectedIndex = initVirtualListControl(trx_id);
    long cnt = static_cast<long>(m_stocks.size());

    if (selectedIndex >= cnt || selectedIndex < 0)
        selectedIndex = getSortAsc() ? cnt - 1 : 0;

    if (cnt>0)
    {
        RefreshItems(0, cnt > 0 ? cnt - 1 : 0);
    }
    else
        selectedIndex = -1;

    if (selectedIndex >= 0 && cnt>0)
    {
        SetItemState(selectedIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(selectedIndex, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        EnsureVisible(selectedIndex);
    }
}

int StocksListCtrl::initVirtualListControl(int64 trx_id)
{
    /* Clear all the records */
    DeleteAllItems();

    // TODO
    if (m_stock_panel->m_account_id > -1 ) {
        m_stocks = Model_Stock::instance().find(
                        Model_Stock::HELDAT(m_stock_panel->m_account_id),
                        Model_Stock::NUMSHARES(0.0, m_stock_panel->getFilter() ? GREATER : GREATER_OR_EQUAL)
                );
    }
    else { // create summary
        m_stocks = Model_Stock::instance().find(
                        Model_Stock::NUMSHARES(0.0, m_stock_panel->getFilter() ? GREATER : GREATER_OR_EQUAL)
                );
        if (!m_stocks.empty())
            createSummary();
    }

    m_stock_panel->updateHeader();
    sortList();

    int cnt = 0, selected_item = -1;
    for (auto& stock : m_stocks)
    {
        if (trx_id == stock.STOCKID)
        {
            selected_item = cnt;
            break;
        }
        if (!stock.PURCHASEPRICE) {
            Model_Stock::UpdatePosition(&stock);
        }
        ++cnt;
    }

    SetItemCount(m_stocks.size());
    return selected_item;
}

void StocksListCtrl::sortList()
{
    std::sort(m_stocks.begin(), m_stocks.end());
    switch (getSortColId())
    {
    case StocksListCtrl::LIST_ID_ID:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterBySTOCKID());
        break;
    case StocksListCtrl::LIST_ID_DATE:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByPURCHASEDATE());
        break;
    case StocksListCtrl::LIST_ID_NAME:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterBySTOCKNAME());
        break;
    case StocksListCtrl::LIST_ID_SYMBOL:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterBySYMBOL());
        break;
    case StocksListCtrl::LIST_ID_NUMBER:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByNUMSHARES());
        break;
    case StocksListCtrl::LIST_ID_PRICE:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByPURCHASEPRICE());
        break;
    case StocksListCtrl::LIST_ID_VALUE:
        std::stable_sort(m_stocks.begin(), m_stocks.end()
            , [](const Model_Stock::Data& x, const Model_Stock::Data& y)
            {
                return x.VALUE < y.VALUE;
            });
        break;
    case StocksListCtrl::LIST_ID_REAL_GAIN_LOSS:
        std::stable_sort(m_stocks.begin(), m_stocks.end()
            , [](const Model_Stock::Data& x, const Model_Stock::Data& y)
        {
            return getRealGainLoss(x) < getRealGainLoss(y);
        });
        break;
    case StocksListCtrl::LIST_ID_GAIN_LOSS:
        std::stable_sort(m_stocks.begin(), m_stocks.end()
            , [](const Model_Stock::Data& x, const Model_Stock::Data& y)
            {
                return getGainLoss(x) < getGainLoss(y);
            });
        break;
    case StocksListCtrl::LIST_ID_CURRENT:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByCURRENTPRICE());
        break;
    case StocksListCtrl::LIST_ID_CURRVALUE:
        std::stable_sort(m_stocks.begin(), m_stocks.end()
            , [](const Model_Stock::Data& x, const Model_Stock::Data& y)
            {
                double valueX = Model_Stock::CurrentValue(x);
                double valueY = Model_Stock::CurrentValue(y);
                return valueX < valueY;
            });
        break;
    case StocksListCtrl::LIST_ID_PRICEDATE:
        //TODO
        break;
    case StocksListCtrl::LIST_ID_COMMISSION:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByCOMMISSION());
        break;
    case StocksListCtrl::LIST_ID_NOTES:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByNOTES());
        break;
    default:
        break;
    }
    if (!getSortAsc()) std::reverse(m_stocks.begin(), m_stocks.end());
}

void StocksListCtrl::createSummary()
{
    Model_Stock::Data_Set stocks_summary;
    Model_Stock::Data prevStock;
    wxString prevSymbol = "";
    m_investedVal = 0;
    m_marketVal = 0;

    std::sort(m_stocks.begin(), m_stocks.end());
    std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterBySYMBOL());

    for (Model_Stock::Data stock : m_stocks) {
        if (stock.SYMBOL != prevSymbol) {
            if (!prevSymbol.IsEmpty()) {
                stocks_summary.push_back(prevStock);
            }
            prevSymbol = stock.SYMBOL;
            prevStock = stock;
        }
        else {
            prevStock.NUMSHARES += stock.NUMSHARES;
            prevStock.VALUE += stock.VALUE;
            prevStock.COMMISSION += stock.COMMISSION;
            prevStock.PURCHASEPRICE = (prevStock.VALUE - prevStock.COMMISSION)/ prevStock.NUMSHARES;
            prevStock.NOTES += (prevStock.NOTES.IsEmpty() ? "" : " - ") + stock.NOTES;
        }
        m_investedVal += stock.VALUE + stock.COMMISSION;
        m_marketVal += stock.CURRENTPRICE * stock.NUMSHARES;

    }
    stocks_summary.push_back(prevStock);

    m_stocks = stocks_summary;
}

void StocksListCtrl::getInvestmentBalance(double& invested, double& current)
{
    invested = m_investedVal;
    current = m_marketVal;
}

wxListItemAttr* StocksListCtrl::OnGetItemAttr(long item) const
{
    return (item % 2) ? (GetGainLoss(item) < 0 ? m_attr2.get() : attr2_.get()) : GetGainLoss(item) < 0 ? m_attr1.get() : attr1_.get();
}
