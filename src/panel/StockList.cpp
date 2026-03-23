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

#include "base/constants.h"
#include "base/images_list.h"
#include "util/_util.h"
#include "util/_simple.h"

#include "model/_all.h"

#include "StockPanel.h"
#include "StockList.h"

#include "dialog/AttachmentDialog.h"
#include "dialog/StockDialog.h"
#include "dialog/TrxShareDialog.h"

enum {
    IDC_PANEL_STOCKS_LISTCTRL = wxID_HIGHEST + 1900,
    MENU_TREEPOPUP_EDIT,
    MENU_TREEPOPUP_ADDTRANS,
    MENU_TREEPOPUP_VIEWTRANS,
    MENU_TREEPOPUP_DELETE,
    MENU_TREEPOPUP_NEW,
    MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS,
};

enum class ico
{
    GAIN,
    LOSS,
    ARROW_UP,
    ARROW_DOWN
};

wxBEGIN_EVENT_TABLE(StockList, ListBase)
    EVT_LEFT_DOWN(                                StockList::onListLeftClick)
    EVT_RIGHT_DOWN(                               StockList::onMouseRightClick)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,             StockList::onListItemActivated)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,              StockList::onListItemSelected)
    EVT_LIST_KEY_DOWN(wxID_ANY,                   StockList::onListKeyDown)
    EVT_MENU(MENU_TREEPOPUP_NEW,                  StockList::onNewStocks)
    EVT_MENU(MENU_TREEPOPUP_EDIT,                 StockList::onEditStocks)
    EVT_MENU(MENU_TREEPOPUP_ADDTRANS,             StockList::onEditStocks)
    EVT_MENU(MENU_TREEPOPUP_VIEWTRANS,            StockList::onEditStocks)
    EVT_MENU(MENU_TREEPOPUP_DELETE,               StockList::onDeleteStocks)
    EVT_MENU(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, StockList::onOrganizeAttachments)
    EVT_MENU(wxID_INDEX,                          StockList::onStockWebPage)
wxEND_EVENT_TABLE()

const std::vector<ListColumnInfo> StockList::LIST_INFO = {
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

StockList::StockList(
    StockPanel* cp,
    wxWindow* parent_win,
    wxWindowID win_id
) :
    ListBase(parent_win, win_id),
    w_panel(cp),
    w_loss_attr1(new wxListItemAttr(
        mmThemeMetaColour(meta::COLOR_REPORT_DEBIT),
        mmThemeMetaColour(meta::COLOR_LISTALT0),
        GetFont()
    )),
    w_loss_attr2(new wxListItemAttr(
        mmThemeMetaColour(meta::COLOR_REPORT_DEBIT),
        mmThemeMetaColour(meta::COLOR_LIST),
        GetFont()
    ))
{
    wxVector<wxBitmapBundle> image_a;
    image_a.push_back(mmBitmapBundle(png::PROFIT));
    image_a.push_back(mmBitmapBundle(png::LOSS));
    image_a.push_back(mmBitmapBundle(png::DOWNARROW));
    image_a.push_back(mmBitmapBundle(png::UPARROW));

    SetSmallImages(image_a);
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
    if (!m_stock_a.empty()) {
        EnsureVisible(m_stock_a.size() - 1);
    }
}

StockList::~StockList()
{
}

int StockList::getSortIcon(bool asc) const
{
    return asc
        ? static_cast<int>(ico::ARROW_DOWN)
        : static_cast<int>(ico::ARROW_UP);
}

void StockList::onMouseRightClick(wxMouseEvent& event)
{
    if (m_select_n >= 0)
        SetItemState(m_select_n, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    int Flags = wxLIST_HITTEST_ONITEM;
    m_select_n = HitTest(wxPoint(event.m_x, event.m_y), Flags);

    if (m_select_n >= 0) {
        SetItemState(m_select_n, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_select_n, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
    }
    w_panel->onListItemSelected(m_select_n);

    bool enable_menu_item = (m_select_n > -1 && w_panel->m_account_id > -1);

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

    menu.Enable(MENU_TREEPOPUP_NEW,
        w_panel->m_account_id > -1
    );
    menu.Enable(MENU_TREEPOPUP_EDIT,
        m_select_n > -1
    );
    menu.Enable(MENU_TREEPOPUP_ADDTRANS,
        enable_menu_item && m_stock_a[m_select_n].m_num_shares > 0
    );
    menu.Enable(MENU_TREEPOPUP_VIEWTRANS,
        m_select_n > -1
    );
    menu.Enable(MENU_TREEPOPUP_DELETE,
        enable_menu_item
    );
    menu.Enable(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS,
        enable_menu_item
    );
    menu.Enable(wxID_INDEX,
        m_select_n > -1
    );

    PopupMenu(&menu, event.GetPosition());

    this->SetFocus();
}

wxString StockList::OnGetItemText(long item, long col_nr) const
{
    int col_id = getColId_Nr(static_cast<int>(col_nr));
    switch (col_id) {
    case LIST_ID_ID:
        return wxString::Format("%lld", m_stock_a[item].m_id).Trim();
    case LIST_ID_DATE:
        return mmGetDateTimeForDisplay(m_stock_a[item].m_purchase_date.isoDate());
    case LIST_ID_NAME:
        return m_stock_a[item].m_name;
    case LIST_ID_SYMBOL:
        return m_stock_a[item].m_symbol;
    case LIST_ID_NUMBER: {
        int precision = m_stock_a[item].m_num_shares == floor(m_stock_a[item].m_num_shares)
            ? 0
            : 4;
        return CurrencyModel::instance().toString(
            m_stock_a[item].m_num_shares,
            w_panel->m_currency_n, precision
        );
    }
    case LIST_ID_PRICE:
        return CurrencyModel::instance().toString(
            m_stock_a[item].m_purchase_price,
            w_panel->m_currency_n, 4
        );
    case LIST_ID_VALUE:
        return CurrencyModel::instance().toString(
            m_stock_a[item].m_purchase_value,
            w_panel->m_currency_n
        );
    case LIST_ID_REAL_GAIN_LOSS:
        return CurrencyModel::instance().toString(
            getRealGainLoss(item),
            w_panel->m_currency_n
        );
    case LIST_ID_GAIN_LOSS:
        return CurrencyModel::instance().toString(
            getGainLoss(item),
            w_panel->m_currency_n
        );
    case LIST_ID_CURRENT:
        return CurrencyModel::instance().toString(
            m_stock_a[item].m_current_price,
            w_panel->m_currency_n, 4
        );
    case LIST_ID_CURRVALUE:
        return CurrencyModel::instance().toString(
            m_stock_a[item].current_value(),
            w_panel->m_currency_n
        );
    case LIST_ID_PRICEDATE:
        return mmGetDateTimeForDisplay(
            StockModel::instance().find_last_hist_date(m_stock_a[item]).isoDate()
        );
    case LIST_ID_COMMISSION:
        return CurrencyModel::instance().toString(
            m_stock_a[item].m_commission,
            w_panel->m_currency_n
        );
    case LIST_ID_NOTES: {
        wxString full_notes = m_stock_a[item].m_notes;
        full_notes.Replace("\n", " ");
        if (AttachmentModel::instance().find_ref_c(
            StockModel::s_ref_type, m_stock_a[item].m_id
        )) {
            full_notes.Prepend(mmAttachmentManage::GetAttachmentNoteSign());
        }
        return full_notes;
    }
    default:
        return wxEmptyString;
    }
}

double StockList::getRealGainLoss(const StockData& stock_d)
{
    return StockModel::instance().calculate_realized_gain(stock_d);
}

void StockList::onListItemSelected(wxListEvent& event)
{
    m_select_n = event.GetIndex();
    w_panel->onListItemSelected(m_select_n);
}

void StockPanel::onListItemSelected(int selectedIndex)
{
    updateExtraStocksData(selectedIndex);
    enableEditDeleteButtons(selectedIndex >= 0);
}

void StockList::onListLeftClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    long index = HitTest(wxPoint(event.m_x, event.m_y), Flags);
    if (index == -1) {
        m_select_n = -1;
        w_panel->onListItemSelected(m_select_n);
    }
    event.Skip();
}

int StockList::OnGetItemImage(long item) const
{
    /* Returns the icon to be shown for each entry */
    double val = getGainLoss(item);
    if (val > 0.0) return static_cast<int>(ico::GAIN);
    else if (val < 0.0) return static_cast<int>(ico::LOSS);
    else return -1;
}

void StockList::onListKeyDown(wxListEvent& event)
{
    switch (event.GetKeyCode())
    {
    case WXK_DELETE: {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED,  MENU_TREEPOPUP_DELETE);
        onDeleteStocks(evt);
        break;
    }
    default:
        event.Skip();
        break;
    }
}

void StockList::onNewStocks(wxCommandEvent& /*event*/)
{
    StockDialog dlg(this, nullptr, w_panel->m_account_id);
    dlg.ShowModal();
    if (StockModel::instance().get_id_data_n(dlg.m_stock_id)) {
        doRefreshItems(dlg.m_stock_id);
        w_panel->w_frame->RefreshNavigationTree();
    }
}

void StockList::onDeleteStocks(wxCommandEvent& /*event*/)
{
    if (m_select_n == -1)
        return;

    wxMessageDialog msgDlg(this,
        _t("Do you want to delete the stock investment?"),
        _t("Confirm Stock Investment Deletion"),
        wxYES_NO | wxNO_DEFAULT | wxICON_ERROR
    );
    if (msgDlg.ShowModal() == wxID_YES) {
        StockModel::instance().purge_id(m_stock_a[m_select_n].m_id);
        mmAttachmentManage::DeleteAllAttachments(
            StockModel::s_ref_type, m_stock_a[m_select_n].m_id
        );
        TrxLinkModel::instance().purge_ref(
            StockModel::s_ref_type, m_stock_a[m_select_n].m_id
        );
        DeleteItem(m_select_n);
        doRefreshItems(-1);
        w_panel->w_frame->RefreshNavigationTree();
    }
}

void StockList::onMoveStocks(wxCommandEvent& /*event*/)
{
    if (m_select_n == -1)
        return;

    const auto& account_a = AccountModel::instance().find(
        AccountCol::ACCOUNTTYPE(NavigatorTypes::instance().getInvestmentAccountStr())
    );
    if (account_a.empty())
        return;

    const AccountData* from_account_n = AccountModel::instance().get_id_data_n(
        w_panel->m_account_id
    );
    wxString headerMsg = wxString::Format(_t("Moving Transaction from %s to"),
        from_account_n->m_name
    );
    mmSingleChoiceDialog scd(this, _t("Select the destination Account "),
        headerMsg, account_a
    );

    int64 to_account_id_n = -1;
    int error_code = scd.ShowModal();
    if (error_code == wxID_OK) {
        wxString acctName = scd.GetStringSelection();
        to_account_id_n = AccountModel::instance().get_name_data_n(acctName)->m_id;
    }

    if (to_account_id_n != -1 ) {
        StockData* stock_n = StockModel::instance().unsafe_get_id_data_n(
            m_stock_a[m_select_n].m_id
        );
        stock_n->m_account_id_n = to_account_id_n;
        StockModel::instance().unsafe_update_data_n(stock_n);

        DeleteItem(m_select_n);
        w_panel->w_frame->RefreshNavigationTree();
    }

    if (error_code == wxID_OK)
        doRefreshItems(-1);
}

void StockList::onEditStocks(wxCommandEvent& event)
{
    if (m_select_n < 0)
        return;

    wxListEvent evt(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, event.GetId());
    AddPendingEvent(evt);
}

void StockList::onOrganizeAttachments(wxCommandEvent& /*event*/)
{
    if (m_select_n < 0)
        return;

    int64 ref_id = m_stock_a[m_select_n].m_id;

    AttachmentDialog dlg(this, StockModel::s_ref_type, ref_id);
    dlg.ShowModal();

    doRefreshItems(ref_id);
}

void StockList::onStockWebPage(wxCommandEvent& /*event*/)
{
    if (m_select_n < 0)
        return;

    const wxString stockSymbol = m_stock_a[m_select_n].m_symbol;

    if (!stockSymbol.IsEmpty()) {
        const wxString& stockURL = InfoModel::instance().getString(
            "STOCKURL", mmex::weblink::DefStockUrl
        );
        const wxString& httpString = wxString::Format(stockURL, stockSymbol);
        wxLaunchDefaultBrowser(httpString);
    }
}

void StockList::onOpenAttachment(wxCommandEvent& /*event*/)
{
    if (m_select_n < 0)
        return;

    int64 ref_id = m_stock_a[m_select_n].m_id;
    mmAttachmentManage::OpenAttachmentFromPanelIcon(this, StockModel::s_ref_type, ref_id);
    doRefreshItems(ref_id);
}

void StockList::onListItemActivated(wxListEvent& event)
{
    if (event.GetId() == wxID_ADD || event.GetId() == MENU_TREEPOPUP_ADDTRANS) {
        if (w_panel->addStockTransaction(m_select_n) == wxID_OK) {
            w_panel->w_frame->RefreshNavigationTree();
        }
    }
    else if (event.GetId() == wxID_VIEW_DETAILS || event.GetId() == MENU_TREEPOPUP_VIEWTRANS) {
        w_panel->viewStockTransactions(m_select_n);
    }
    else {
        w_panel->onListItemActivated(m_select_n);
    }
}

void StockList::onColClick(wxListEvent& event)
{
    int col_nr = (event.GetId() == MENU_HEADER_SORT)
        ? m_sel_col_nr
        : event.GetColumn();
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
    savePref();

    int64 trx_id = -1;
    if (m_select_n >= 0)
        trx_id = m_stock_a[m_select_n].m_id;
    doRefreshItems(trx_id);
    w_panel->onListItemSelected(-1);
}

void StockList::doRefreshItems(int64 trx_id)
{
    int selectedIndex = initVirtualListControl(trx_id);
    long cnt = static_cast<long>(m_stock_a.size());

    if (selectedIndex >= cnt || selectedIndex < 0)
        selectedIndex = getSortAsc() ? cnt - 1 : 0;

    if (cnt>0) {
        RefreshItems(0, cnt > 0 ? cnt - 1 : 0);
    }
    else
        selectedIndex = -1;

    if (selectedIndex >= 0 && cnt>0) {
        SetItemState(selectedIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(selectedIndex, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        EnsureVisible(selectedIndex);
    }
}

int StockList::initVirtualListControl(int64 trx_id)
{
    /* Clear all the records */
    DeleteAllItems();

    // TODO
    if (w_panel->m_account_id > -1 ) {
        m_stock_a = StockModel::instance().find(
            StockCol::HELDAT(w_panel->m_account_id),
            StockCol::NUMSHARES(w_panel->getFilter() ? OP_GT : OP_GE, 0.0)
        );
    }
    // create summary
    else {
        m_stock_a = StockModel::instance().find(
            StockCol::NUMSHARES(w_panel->getFilter() ? OP_GT : OP_GE, 0.0)
        );
        if (!m_stock_a.empty())
            createSummary();
    }

    w_panel->updateHeader();
    sortList();

    int cnt = 0, selected_item = -1;
    for (auto& stock_d : m_stock_a) {
        if (trx_id == stock_d.m_id) {
            selected_item = cnt;
            break;
        }
        if (!stock_d.m_purchase_price) {
            StockModel::instance().update_data_position(&stock_d);
        }
        ++cnt;
    }

    SetItemCount(m_stock_a.size());
    return selected_item;
}

void StockList::sortList()
{
    std::sort(m_stock_a.begin(), m_stock_a.end());
    switch (getSortColId())
    {
    case StockList::LIST_ID_ID:
        std::stable_sort(m_stock_a.begin(), m_stock_a.end(), StockData::SorterBySTOCKID());
        break;
    case StockList::LIST_ID_DATE:
        std::stable_sort(m_stock_a.begin(), m_stock_a.end(), StockData::SorterByPURCHASEDATE());
        break;
    case StockList::LIST_ID_NAME:
        std::stable_sort(m_stock_a.begin(), m_stock_a.end(), StockData::SorterBySTOCKNAME());
        break;
    case StockList::LIST_ID_SYMBOL:
        std::stable_sort(m_stock_a.begin(), m_stock_a.end(), StockData::SorterBySYMBOL());
        break;
    case StockList::LIST_ID_NUMBER:
        std::stable_sort(m_stock_a.begin(), m_stock_a.end(), StockData::SorterByNUMSHARES());
        break;
    case StockList::LIST_ID_PRICE:
        std::stable_sort(m_stock_a.begin(), m_stock_a.end(), StockData::SorterByPURCHASEPRICE());
        break;
    case StockList::LIST_ID_VALUE:
        std::stable_sort(m_stock_a.begin(), m_stock_a.end(),
            [](const StockData& x, const StockData& y) {
                return x.m_purchase_value < y.m_purchase_value;
            }
        );
        break;
    case StockList::LIST_ID_REAL_GAIN_LOSS:
        std::stable_sort(m_stock_a.begin(), m_stock_a.end(),
            [](const StockData& x, const StockData& y) {
                return getRealGainLoss(x) < getRealGainLoss(y);
            }
        );
        break;
    case StockList::LIST_ID_GAIN_LOSS:
        std::stable_sort(m_stock_a.begin(), m_stock_a.end(),
            [](const StockData& x, const StockData& y) {
                return x.current_gain() < y.current_gain();
            }
        );
        break;
    case StockList::LIST_ID_CURRENT:
        std::stable_sort(m_stock_a.begin(), m_stock_a.end(), StockData::SorterByCURRENTPRICE());
        break;
    case StockList::LIST_ID_CURRVALUE:
        std::stable_sort(m_stock_a.begin(), m_stock_a.end(),
            [](const StockData& x, const StockData& y) {
                return x.current_value() < y.current_value();
            }
        );
        break;
    case StockList::LIST_ID_PRICEDATE:
        //TODO
        break;
    case StockList::LIST_ID_COMMISSION:
        std::stable_sort(m_stock_a.begin(), m_stock_a.end(), StockData::SorterByCOMMISSION());
        break;
    case StockList::LIST_ID_NOTES:
        std::stable_sort(m_stock_a.begin(), m_stock_a.end(), StockData::SorterByNOTES());
        break;
    default:
        break;
    }

    if (!getSortAsc())
        std::reverse(m_stock_a.begin(), m_stock_a.end());
}

void StockList::createSummary()
{
    StockModel::DataA stock_a;
    StockData prev_stock_d;
    wxString prev_symbol = "";
    m_invest_value = 0;
    m_market_value = 0;

    std::sort(m_stock_a.begin(), m_stock_a.end());
    std::stable_sort(m_stock_a.begin(), m_stock_a.end(), StockData::SorterBySYMBOL());

    for (StockData stock_d : m_stock_a) {
        if (stock_d.m_symbol != prev_symbol) {
            if (!prev_symbol.IsEmpty()) {
                // FIXME: m_purchase_value includes m_commission here
                prev_stock_d.m_purchase_price = (
                    prev_stock_d.m_purchase_value - prev_stock_d.m_commission
                ) / prev_stock_d.m_num_shares;
                stock_a.push_back(prev_stock_d);
            }
            prev_symbol = stock_d.m_symbol;
            prev_stock_d = stock_d;
        }
        else {
            prev_stock_d.m_num_shares     += stock_d.m_num_shares;
            prev_stock_d.m_purchase_value += stock_d.m_purchase_value;
            prev_stock_d.m_commission     += stock_d.m_commission;
            prev_stock_d.m_notes          += (prev_stock_d.m_notes.IsEmpty() ? "" : " - ") + stock_d.m_notes;
        }
        // FIXME: m_purchase_value does not include m_commission here
        m_invest_value += stock_d.m_purchase_value + stock_d.m_commission;
        m_market_value += stock_d.m_current_price * stock_d.m_num_shares;

    }
    stock_a.push_back(prev_stock_d);

    m_stock_a = stock_a;
}

void StockList::getInvestmentBalance(double& invest_value, double& market_value)
{
    invest_value = m_invest_value;
    market_value = m_market_value;
}

wxListItemAttr* StockList::OnGetItemAttr(long item) const
{
    return (item % 2 == 0)
        ? (getGainLoss(item) < 0 ? w_loss_attr1.get() : w_attr1.get())
        : (getGainLoss(item) < 0 ? w_loss_attr2.get() : w_attr2.get());
}

wxString StockList::getStockInfo(int selectedIndex, bool with_symbol) const
{
    const StockData& stock_d = m_stock_a[selectedIndex];

    // Short symbols
    // pur_m  : times purchased
    // pur_n  : number of shares
    //
    // pur_p  : purchase price per share (not including commission)
    // cur_p  : current price per share
    // diff_p : price difference per share (not including commission)
    // gain_p : price change in % (not including commission)
    //
    // pur_q  : purchase value per share (including commission)
    // diff_q : value difference per share (including commission)
    // gain_q : price change in % (including commission)
    //
    // pur_v  : purchase value (including commission)
    // cur_v  : current value
    // diff_v : value difference (including commission)

    // Selected share
    double stock_pur_n  = stock_d.m_num_shares;
    double stock_pur_p  = stock_d.m_purchase_price;
    double stock_cur_p  = stock_d.m_current_price;
    double stock_diff_p = stock_cur_p - stock_pur_p;
    double stock_gain_p = (stock_cur_p / stock_pur_p - 1.0) * 100.0;
    double stock_pur_v  = stock_d.m_purchase_value;
    double stock_cur_v  = stock_pur_n * stock_cur_p;
    double stock_diff_v = stock_cur_v - stock_pur_v;

    wxString stock_pur_n_str = wxString::Format("%i", static_cast<int>(stock_pur_n));
    if (stock_pur_n - static_cast<long>(stock_pur_n) != 0.0)
        stock_pur_n_str = wxString::Format("%.4f", stock_pur_n);
    const wxString& stock_pur_p_str = CurrencyModel::instance().toCurrency(
        stock_pur_p, w_panel->m_currency_n, 4
    );
    const wxString& stock_cur_p_str = CurrencyModel::instance().toCurrency(
        stock_cur_p, w_panel->m_currency_n, 4
    );
    const wxString& stock_diff_p_str = CurrencyModel::instance().toCurrency(
        stock_diff_p, w_panel->m_currency_n, 4
    );
    const wxString& stock_gain_p_str = (stock_d.m_purchase_price != 0.0)
        ? wxString::Format("(%s %%)",
            CurrencyModel::instance().toStringNoFormatting(stock_gain_p, nullptr, 2)
        ) : "";

    // Summary for selected symbol
    int    symbol_pur_m = 0;
    double symbol_pur_n = 0;
    double symbol_pur_v = 0;
    for (const auto& symbol_stock_d: StockModel::instance().find(
        StockCol::SYMBOL(stock_d.m_symbol)
    )) {
        symbol_pur_m += 1;
        symbol_pur_n += symbol_stock_d.m_num_shares;
        symbol_pur_v += symbol_stock_d.m_purchase_value;
    }
    double symbol_pur_q  = symbol_pur_v / symbol_pur_n;
    double symbol_cur_p  = stock_cur_p;
    double symbol_diff_q = symbol_cur_p - symbol_pur_q;
    double symbol_gain_q = (symbol_cur_p / symbol_pur_q - 1.0) * 100.0;
    double symbol_diff_v = symbol_diff_q * symbol_pur_n;

    wxString symbol_pur_n_str = wxString::Format("%i", static_cast<int>(symbol_pur_n));
    if (symbol_pur_n - static_cast<long>(symbol_pur_n) != 0.0)
        symbol_pur_n_str = wxString::Format("%.4f", symbol_pur_n);
    const wxString& symbol_pur_q_str = CurrencyModel::instance().toCurrency(
        symbol_pur_q, w_panel->m_currency_n, 4
    );
    const wxString& symbol_diff_q_str = CurrencyModel::instance().toCurrency(symbol_diff_q);

    wxString miniInfo = "";
    if (stock_d.m_symbol != "")
        miniInfo << "\t" << wxString::Format(_t("Symbol: %s"), stock_d.m_symbol) << "\t\t";
    miniInfo << wxString::Format(_t("Total: %s"), " (" + symbol_pur_n_str + ") ");
    w_panel->w_details_short->SetLabelText(miniInfo);

    // Selected share info
    wxString info_str = wxString::Format("This Account: |%s - %s| = %s, %s * %s = %s %s\n",
        stock_cur_p_str, stock_pur_p_str, stock_diff_p_str,
        stock_diff_p_str, stock_pur_n_str,
        // CHECK: stock_diff_v includes commission; all other do not include commission
        CurrencyModel::instance().toCurrency(stock_diff_v, w_panel->m_currency_n),
        stock_gain_p_str
    );

    // Summary for selected symbol
    if (with_symbol && symbol_pur_m > 1) {
        info_str += wxString::Format("All Accounts: |%s - %s| = %s, %s * %s = %s ( %s %% )\n%s",
            stock_cur_p_str, symbol_pur_q_str, symbol_diff_q_str,
            symbol_diff_q_str, symbol_pur_n_str,
            CurrencyModel::instance().toCurrency(symbol_diff_v),
            CurrencyModel::instance().toStringNoFormatting(symbol_gain_q, nullptr, 2),
            OnGetItemText(selectedIndex, static_cast<long>(LIST_ID_NOTES))
        );
    }
    return info_str;
}
