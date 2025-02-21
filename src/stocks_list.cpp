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
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, StocksListCtrl::OnListItemActivated)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, StocksListCtrl::OnListItemSelected)
    EVT_LIST_KEY_DOWN(wxID_ANY, StocksListCtrl::OnListKeyDown)
    EVT_MENU(MENU_TREEPOPUP_NEW, StocksListCtrl::OnNewStocks)
    EVT_MENU(MENU_TREEPOPUP_EDIT, StocksListCtrl::OnEditStocks)
    EVT_MENU(MENU_TREEPOPUP_ADDTRANS, StocksListCtrl::OnEditStocks)
    EVT_MENU(MENU_TREEPOPUP_VIEWTRANS, StocksListCtrl::OnEditStocks)
    EVT_MENU(MENU_TREEPOPUP_DELETE, StocksListCtrl::OnDeleteStocks)
    EVT_MENU(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, StocksListCtrl::OnOrganizeAttachments)
    EVT_MENU(wxID_INDEX, StocksListCtrl::OnStockWebPage)
    EVT_RIGHT_DOWN(StocksListCtrl::OnMouseRightClick)
    EVT_LEFT_DOWN(StocksListCtrl::OnListLeftClick)
wxEND_EVENT_TABLE()

const std::vector<ListColumnInfo> StocksListCtrl::col_info_all()
{
    return {
        { " ",                       25,                        wxLIST_FORMAT_LEFT,  false },
        { _t("ID"),                   wxLIST_AUTOSIZE,           wxLIST_FORMAT_RIGHT, true },
        { _t("*Date"),                wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
        { _t("Company Name"),         wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
        { _t("Symbol"),               wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
        { _t("Share Total"),          wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT, true },
        { _t("Avg Share Price"),      wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT, true },
        { _t("Total Cost"),           wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT, true },
        { _t("Realized Gain/Loss"),   wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT, true },
        { _t("Unrealized Gain/Loss"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT, true },
        { _t("Curr. Share Price"),    wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT, true },
        { _t("Curr. Total Value"),    wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT, true },
        { _t("Price Date"),           wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
        { _t("Commission"),           wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT, true },
        { _t("Notes"),                wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT,  true },
    };
}

StocksListCtrl::~StocksListCtrl()
{
}

StocksListCtrl::StocksListCtrl(mmStocksPanel* cp, wxWindow *parent, wxWindowID winid)
    : mmListCtrl(parent, winid)
    , m_stock_panel(cp)
{
    wxVector<wxBitmapBundle> images;
    images.push_back(mmBitmapBundle(png::PROFIT));
    images.push_back(mmBitmapBundle(png::LOSS));
    images.push_back(mmBitmapBundle(png::DOWNARROW));
    images.push_back(mmBitmapBundle(png::UPARROW));

    SetSmallImages(images);
    mmThemeMetaColour(this, meta::COLOR_LISTPANEL);

    // load the global variables
    m_selected_col = Model_Setting::instance().getInt("STOCKS_SORT_COL", col_sort());
    m_asc = Model_Setting::instance().getBool("STOCKS_ASC", true);

    m_columns = col_info_all();
    for (int i = 0; i < LIST_COL_size; ++i)
        m_column_order.push_back(i);
    m_col_width_fmt = "STOCKS_COL%d_WIDTH";
    m_col_type_str = "STOCKS";
    m_default_sort_column = col_sort();

    createColumns();

    initVirtualListControl(-1, m_selected_col, m_asc);
    if (!m_stocks.empty())
        EnsureVisible(m_stocks.size() - 1);

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

    bool hide_menu_item = (m_selected_row < 0);

    wxMenu menu;
    menu.Append(MENU_TREEPOPUP_NEW, _tu("&New Stock Investment…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ADDTRANS, _tu("&Add Stock Transactions…"));
    menu.Append(MENU_TREEPOPUP_VIEWTRANS, _t("&View Stock Transactions"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_EDIT, _tu("&Edit Stock Investment…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_DELETE, _tu("&Delete Stock Investment…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, _tu("&Organize Attachments…"));
    menu.Append(wxID_INDEX, _t("Stock &Web Page"));

    menu.Enable(MENU_TREEPOPUP_EDIT, !hide_menu_item);
    menu.Enable(MENU_TREEPOPUP_ADDTRANS, !hide_menu_item);
    menu.Enable(MENU_TREEPOPUP_VIEWTRANS, !hide_menu_item);
    menu.Enable(MENU_TREEPOPUP_DELETE, !hide_menu_item);
    menu.Enable(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, !hide_menu_item);
    menu.Enable(wxID_INDEX, !hide_menu_item);

    PopupMenu(&menu, event.GetPosition());

    this->SetFocus();
}

wxString StocksListCtrl::OnGetItemText(long item, long column) const
{
    switch (m_column_order[column]) {
    case LIST_COL_ID:
        return wxString::Format("%lld", m_stocks[item].STOCKID).Trim();
    case LIST_COL_DATE:
        return mmGetDateTimeForDisplay(m_stocks[item].PURCHASEDATE);
    case LIST_COL_NAME:
        return m_stocks[item].STOCKNAME;
    case LIST_COL_SYMBOL:
        return m_stocks[item].SYMBOL;
    case LIST_COL_NUMBER: {
        int precision = m_stocks[item].NUMSHARES == floor(m_stocks[item].NUMSHARES) ? 0 : 4;
        return Model_Currency::toString(m_stocks[item].NUMSHARES, m_stock_panel->m_currency, precision);
    }
    case LIST_COL_PRICE:
        return Model_Currency::toString(m_stocks[item].PURCHASEPRICE, m_stock_panel->m_currency, 4);
    case LIST_COL_VALUE:
        return Model_Currency::toString(m_stocks[item].VALUE, m_stock_panel->m_currency);
    case LIST_COL_REAL_GAIN_LOSS:
        return Model_Currency::toString(GetRealGainLoss(item), m_stock_panel->m_currency);
    case LIST_COL_GAIN_LOSS:
        return Model_Currency::toString(GetGainLoss(item), m_stock_panel->m_currency);
    case LIST_COL_CURRENT:
        return Model_Currency::toString(m_stocks[item].CURRENTPRICE, m_stock_panel->m_currency, 4);
    case LIST_COL_CURRVALUE:
        return Model_Currency::toString(Model_Stock::CurrentValue(m_stocks[item]), m_stock_panel->m_currency);
    case LIST_COL_PRICEDATE:
        return mmGetDateTimeForDisplay(Model_Stock::instance().lastPriceDate(&m_stocks[item]));
    case LIST_COL_COMMISSION:
        return Model_Currency::toString(m_stocks[item].COMMISSION, m_stock_panel->m_currency);
    case LIST_COL_NOTES: {
        wxString full_notes = m_stocks[item].NOTES;
        full_notes.Replace("\n", " ");
        if (Model_Attachment::NrAttachments(Model_Attachment::REFTYPE_NAME_STOCK, m_stocks[item].STOCKID))
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
    mmStockDialog dlg(this, m_stock_panel->m_frame, nullptr, m_stock_panel->m_account_id);
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
        mmAttachmentManage::DeleteAllAttachments(Model_Attachment::REFTYPE_NAME_STOCK, m_stocks[m_selected_row].STOCKID);
        Model_Translink::RemoveTransLinkRecords(Model_Attachment::REFTYPE_ID_STOCK, m_stocks[m_selected_row].STOCKID);
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

    wxString RefType = Model_Attachment::REFTYPE_NAME_STOCK;
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

    wxString RefType = Model_Attachment::REFTYPE_NAME_STOCK;
    int64 RefId = m_stocks[m_selected_row].STOCKID;

    mmAttachmentManage::OpenAttachmentFromPanelIcon(this, RefType, RefId);
    doRefreshItems(RefId);
}

void StocksListCtrl::OnListItemActivated(wxListEvent& event)
{
    if ((event.GetId() == wxID_ADD) || (event.GetId() == MENU_TREEPOPUP_ADDTRANS))
    {
        m_stock_panel->AddStockTransaction(m_selected_row);
        m_stock_panel->m_frame->RefreshNavigationTree();
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
    int ColumnNr;
    if (event.GetId() != MENU_HEADER_SORT && event.GetId() != MENU_HEADER_RESET)
        ColumnNr = event.GetColumn();
    else
        ColumnNr = m_ColumnHeaderNbr;
    if (0 >= ColumnNr || ColumnNr >= getColumnsNumber()) return;

    if (m_selected_col == ColumnNr &&
        event.GetId() != MENU_HEADER_SORT && event.GetId() != MENU_HEADER_RESET
    )
        m_asc = !m_asc;

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(-1);
    SetColumn(m_selected_col, item);

    m_selected_col = ColumnNr;

    Model_Setting::instance().setBool("STOCKS_ASC", m_asc);
    Model_Setting::instance().setInt("STOCKS_SORT_COL", m_selected_col);

    int64 trx_id = -1;
    if (m_selected_row>=0) trx_id = m_stocks[m_selected_row].STOCKID;
    doRefreshItems(trx_id);
    m_stock_panel->OnListItemSelected(-1);
}

void StocksListCtrl::doRefreshItems(int64 trx_id)
{
    int selectedIndex = initVirtualListControl(trx_id, m_selected_col, m_asc);
    long cnt = static_cast<long>(m_stocks.size());

    if (selectedIndex >= cnt || selectedIndex < 0)
        selectedIndex = m_asc ? cnt - 1 : 0;

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

int StocksListCtrl::initVirtualListControl(int64 trx_id, int col, bool asc)
{
    m_stock_panel->updateHeader();
    /* Clear all the records */
    DeleteAllItems();

    if (col > 0)
    {
        wxListItem item;
        item.SetMask(wxLIST_MASK_IMAGE);
        item.SetImage(asc ? static_cast<int>(ico::ARROW_DOWN) : static_cast<int>(ico::ARROW_UP));
        SetColumn(col, item);
    }

    m_stocks = Model_Stock::instance().find(Model_Stock::HELDAT(m_stock_panel->m_account_id));
    sortTable();

    int cnt = 0, selected_item = -1;
    for (auto& stock : m_stocks)
    {
        if (trx_id == stock.STOCKID)
        {
            selected_item = cnt;
            break;
        }
        if (!stock.PURCHASEPRICE) {
            Model_Translink::UpdateStockValue(&stock);
        }
        ++cnt;
    }

    SetItemCount(m_stocks.size());
    return selected_item;
}

void StocksListCtrl::sortTable()
{
    std::sort(m_stocks.begin(), m_stocks.end());
    switch (m_selected_col)
    {
    case StocksListCtrl::LIST_COL_ID:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterBySTOCKID());
        break;
    case StocksListCtrl::LIST_COL_DATE:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByPURCHASEDATE());
        break;
    case StocksListCtrl::LIST_COL_NAME:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterBySTOCKNAME());
        break;
    case StocksListCtrl::LIST_COL_SYMBOL:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterBySYMBOL());
        break;
    case StocksListCtrl::LIST_COL_NUMBER:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByNUMSHARES());
        break;
    case StocksListCtrl::LIST_COL_PRICE:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByPURCHASEPRICE());
        break;
    case StocksListCtrl::LIST_COL_VALUE:
        std::stable_sort(m_stocks.begin(), m_stocks.end()
            , [](const Model_Stock::Data& x, const Model_Stock::Data& y)
            {
                return x.VALUE < y.VALUE;
            });
        break;
    case StocksListCtrl::LIST_COL_REAL_GAIN_LOSS:
        std::stable_sort(m_stocks.begin(), m_stocks.end()
            , [](const Model_Stock::Data& x, const Model_Stock::Data& y)
        {
            return getRealGainLoss(x) < getRealGainLoss(y);
        });
        break;
    case StocksListCtrl::LIST_COL_GAIN_LOSS:
        std::stable_sort(m_stocks.begin(), m_stocks.end()
            , [](const Model_Stock::Data& x, const Model_Stock::Data& y)
            {
                return getGainLoss(x) < getGainLoss(y);
            });
        break;
    case StocksListCtrl::LIST_COL_CURRENT:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByCURRENTPRICE());
        break;
    case StocksListCtrl::LIST_COL_CURRVALUE:
        std::stable_sort(m_stocks.begin(), m_stocks.end()
            , [](const Model_Stock::Data& x, const Model_Stock::Data& y)
            {
                double valueX = Model_Stock::CurrentValue(x);
                double valueY = Model_Stock::CurrentValue(y);
                return valueX < valueY;
            });
        break;
    case StocksListCtrl::LIST_COL_PRICEDATE:
        //TODO
        break;
    case StocksListCtrl::LIST_COL_COMMISSION:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByCOMMISSION());
        break;
    case StocksListCtrl::LIST_COL_NOTES:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByNOTES());
        break;
    default:
        break;
    }
    if (!m_asc) std::reverse(m_stocks.begin(), m_stocks.end());
}
