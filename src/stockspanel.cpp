/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2010-2014 Nikolay

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
#include "stockdialog.h"
#include "attachmentdialog.h"
#include "constants.h"
#include "util.h"
#include "mmsinglechoicedialog.h"
#include "model/Model_Setting.h"
#include "model/Model_Infotable.h"
#include "model/Model_Attachment.h"
#include "model/Model_StockHistory.h"

#include "../resources/uparrow.xpm"
#include "../resources/downarrow_red.xpm"
#include "../resources/downarrow.xpm"
#include "../resources/leds.xpm"
#include "../resources/attachment.xpm"

enum {
    IDC_PANEL_STOCKS_LISTCTRL = wxID_HIGHEST + 1900,
    MENU_TREEPOPUP_EDIT,
    MENU_TREEPOPUP_DELETE,
    MENU_TREEPOPUP_NEW,
    MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS,
};

/*******************************************************/

BEGIN_EVENT_TABLE(StocksListCtrl, mmListCtrl)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,   StocksListCtrl::OnListItemActivated)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,    StocksListCtrl::OnListItemSelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY,  StocksListCtrl::OnListItemDeselected)
    EVT_LIST_COL_END_DRAG(wxID_ANY,     StocksListCtrl::OnItemResize)
    EVT_LIST_COL_CLICK(wxID_ANY,        StocksListCtrl::OnColClick)
    EVT_LIST_KEY_DOWN(wxID_ANY,         StocksListCtrl::OnListKeyDown)
    EVT_MENU(MENU_TREEPOPUP_NEW,     StocksListCtrl::OnNewStocks)
    EVT_MENU(MENU_TREEPOPUP_EDIT,    StocksListCtrl::OnEditStocks)
    EVT_MENU(MENU_TREEPOPUP_DELETE,  StocksListCtrl::OnDeleteStocks)
    EVT_MENU(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, StocksListCtrl::OnOrganizeAttachments)
    EVT_RIGHT_DOWN(StocksListCtrl::OnMouseRightClick)
END_EVENT_TABLE()

StocksListCtrl::~StocksListCtrl()
{
    if (m_imageList) delete m_imageList;
}

StocksListCtrl::StocksListCtrl(mmStocksPanel* cp, wxWindow *parent, wxWindowID winid)
    : mmListCtrl(parent, winid)
    , stock_panel_(cp)
    , m_imageList(0)
{
    ColName_[COL_DATE]      = _("Purchase Date");
    ColName_[COL_NAME]      = _("Share Name");
    ColName_[COL_NUMBER]    = _("Number of Shares");
    ColName_[COL_VALUE]     = _("Value");
    ColName_[COL_GAIN_LOSS] = _("Gain/Loss");
    ColName_[COL_CURRENT]   = _("Current");
    ColName_[COL_PRICEDATE] = _("Price Date");
    ColName_[COL_NOTES]     = _("Notes");

    wxSize imageSize(16, 16);
    m_imageList = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
    m_imageList->Add(wxBitmap(wxImage(uparrow_xpm).Scale(16, 16)));
    m_imageList->Add(wxBitmap(wxImage(downarrow_red_xpm).Scale(16, 16)));
    m_imageList->Add(wxBitmap(wxImage(uparrow_xpm).Scale(16, 16)));
    m_imageList->Add(wxBitmap(wxImage(downarrow_xpm).Scale(16, 16)));

    SetImageList(m_imageList, wxIMAGE_LIST_SMALL);
    wxListItem itemCol;

    for (const auto&column : ColName_)
    {
        itemCol.SetText(column.second);
        InsertColumn(column.first, column.second, (column.first<2 || column.first>5 ? wxLIST_FORMAT_LEFT : wxLIST_FORMAT_RIGHT));

        int col_x = Model_Setting::instance().GetIntSetting(wxString::Format("STOCKS_COL%d_WIDTH", column.first), -2);
        SetColumnWidth(column.first, col_x);
    }

    // load the global variables
    m_selected_col = Model_Setting::instance().GetIntSetting("STOCKS_SORT_COL", 0);
    m_asc = Model_Setting::instance().GetBoolSetting("STOCKS_ASC", true);

    initVirtualListControl(-1, m_selected_col, m_asc);
    if (!m_stocks.empty())
        EnsureVisible(m_stocks.size() - 1);

}

void StocksListCtrl::OnItemResize(wxListEvent& event)
{
    int i = event.GetColumn();
    save_column_width(i);
}

void StocksListCtrl::OnMouseRightClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    m_selected_row = HitTest(wxPoint(event.m_x, event.m_y), Flags);

    if (m_selected_row >= 0)
    {
        SetItemState(m_selected_row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selected_row, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
    }

    bool hide_menu_item = (m_selected_row < 0);

    wxMenu menu;
    menu.Append(MENU_TREEPOPUP_NEW, _("&New Stock Investment"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_EDIT, _("&Edit Stock Investment"));
    menu.Append(MENU_TREEPOPUP_DELETE, _("&Delete Stock Investment"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, _("&Organize Attachments"));

    menu.Enable(MENU_TREEPOPUP_EDIT, !hide_menu_item);
    menu.Enable(MENU_TREEPOPUP_DELETE, !hide_menu_item);
    menu.Enable(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, !hide_menu_item);

    stock_panel_->enableEditDeleteButtons(!hide_menu_item);
    PopupMenu(&menu, event.GetPosition());

    this->SetFocus();
}

wxString StocksListCtrl::OnGetItemText(long item, long column) const
{
    if (column == COL_DATE)         return mmGetDateForDisplay(mmGetStorageStringAsDate(m_stocks[item].PURCHASEDATE));
    if (column == COL_NAME)         return m_stocks[item].STOCKNAME;
    if (column == COL_NUMBER)
    {
        int precision = m_stocks[item].NUMSHARES == floor(m_stocks[item].NUMSHARES) ? 0 : 4;
        return Model_Currency::toString(m_stocks[item].NUMSHARES, stock_panel_->m_currency, precision);
    }
    if (column == COL_GAIN_LOSS)    return Model_Currency::toString(getGainLoss(item), stock_panel_->m_currency /*, 4*/);
    if (column == COL_VALUE)        return Model_Currency::toString(m_stocks[item].VALUE, stock_panel_->m_currency);
    if (column == COL_CURRENT)      return Model_Currency::toString(m_stocks[item].CURRENTPRICE, stock_panel_->m_currency, 4);
    if (column == COL_PRICEDATE)    return mmGetDateForDisplay(mmGetStorageStringAsDate(Model_Stock::instance().lastPriceDate(&m_stocks[item])));
    if (column == COL_NOTES)
    {
        wxString full_notes = m_stocks[item].NOTES;
        if (Model_Attachment::NrAttachments(Model_Attachment::reftype_desc(Model_Attachment::STOCK), m_stocks[item].STOCKID))
            full_notes = full_notes.Prepend(mmAttachmentManage::GetAttachmentNoteSign());
        return full_notes;
    }

    return "";
}

double StocksListCtrl::getGainLoss(long item) const
{
    return m_stocks[item].VALUE - ((m_stocks[item].NUMSHARES * m_stocks[item].PURCHASEPRICE) + m_stocks[item].COMMISSION);
}

void StocksListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selected_row = event.GetIndex();
    stock_panel_->OnListItemSelected(m_selected_row);
}
void mmStocksPanel::OnListItemSelected(int selectedIndex)
{
    updateExtraStocksData(selectedIndex);
    enableEditDeleteButtons(selectedIndex >= 0);
}

void StocksListCtrl::OnListItemDeselected(wxListEvent& /*event*/)
{
    m_selected_row = -1;
    stock_panel_->OnListItemSelected(m_selected_row);
}

int StocksListCtrl::OnGetItemImage(long item) const
{
    /* Returns the icon to be shown for each entry */
    if (getGainLoss(item) > 0) return 0;
    return 1;
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
    mmStockDialog dlg(this, 0, stock_panel_->accountID_);
    if (dlg.ShowModal() == wxID_OK)
    {
        doRefreshItems(dlg.transID_);
    }
}

void StocksListCtrl::OnDeleteStocks(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    wxMessageDialog msgDlg(this, _("Do you really want to delete the stock investment?")
        , _("Confirm Stock Investment Deletion")
        , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        Model_Stock::instance().remove(m_stocks[m_selected_row].STOCKID);
        mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::STOCK), m_stocks[m_selected_row].STOCKID);
        DeleteItem(m_selected_row);
        doRefreshItems(-1);
    }
}

void StocksListCtrl::OnMoveStocks(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;
    
    const auto& accounts = Model_Account::instance().find(Model_Account::ACCOUNTTYPE(Model_Account::all_type()[Model_Account::INVESTMENT]));
    if (accounts.empty()) return;

    const Model_Account::Data* from_account = Model_Account::instance().get(stock_panel_->accountID_);
    wxString headerMsg = wxString::Format(_("Moving Transaction from %s to..."), from_account->ACCOUNTNAME);
    mmSingleChoiceDialog scd(this, _("Select the destination Account "), headerMsg , accounts);

    int toAccountID = -1;
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
    }

    if (error_code == wxID_OK)
        doRefreshItems(-1);
}

void StocksListCtrl::OnEditStocks(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;

    wxListEvent evt(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxID_ANY);
    AddPendingEvent(evt);
}

void StocksListCtrl::OnOrganizeAttachments(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;

    wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    int RefId = m_stocks[m_selected_row].STOCKID;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();

    doRefreshItems(RefId);
}

void StocksListCtrl::OnOpenAttachment(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;

    wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    int RefId = m_stocks[m_selected_row].STOCKID;

    mmAttachmentManage::OpenAttachmentFromPanelIcon(this, RefType, RefId);
    doRefreshItems(RefId);
}

void StocksListCtrl::OnListItemActivated(wxListEvent& /*event*/)
{
    stock_panel_->OnListItemActivated(m_selected_row);
}
void mmStocksPanel::OnListItemActivated(int selectedIndex)
{
    call_dialog(selectedIndex);
    updateExtraStocksData(selectedIndex);
}

void StocksListCtrl::OnColClick(wxListEvent& event)
{
    if(0 > event.GetColumn() || event.GetColumn() >= getColumnsNumber()) return;

    if (m_selected_col == event.GetColumn()) m_asc = !m_asc;

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(-1);
    SetColumn(m_selected_col, item);

    m_selected_col = event.GetColumn();

    Model_Setting::instance().Set("STOCKS_ASC", m_asc);
    Model_Setting::instance().Set("STOCKS_SORT_COL", m_selected_col);

    int trx_id = -1;
    if (m_selected_row>=0) trx_id = m_stocks[m_selected_row].STOCKID;
    doRefreshItems(trx_id);
    stock_panel_->OnListItemSelected(-1);
}

void StocksListCtrl::doRefreshItems(int trx_id)
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

/*******************************************************/
BEGIN_EVENT_TABLE(mmStocksPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,         mmStocksPanel::OnNewStocks)
    EVT_BUTTON(wxID_EDIT,        mmStocksPanel::OnEditStocks)
    EVT_BUTTON(wxID_DELETE,      mmStocksPanel::OnDeleteStocks)
    EVT_BUTTON(wxID_MOVE_FRAME,  mmStocksPanel::OnMoveStocks)
    EVT_BUTTON(wxID_FILE,        mmStocksPanel::OnOpenAttachment)
    EVT_BUTTON(wxID_REFRESH,     mmStocksPanel::OnRefreshQuotes)
END_EVENT_TABLE()
/*******************************************************/
mmStocksPanel::mmStocksPanel(int accountID,
                             wxWindow *parent,
                             wxWindowID winid, const wxPoint& pos, const wxSize& size, long style,
                             const wxString& name)
: accountID_(accountID)
, m_currency()
{
    Create(parent, winid, pos, size, style, name);
}

bool mmStocksPanel::Create(wxWindow *parent
    , wxWindowID winid, const wxPoint& pos
    , const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    strLastUpdate_ = Model_Infotable::instance().GetStringInfo("STOCKS_LAST_REFRESH_DATETIME", "");
    this->windowsFreezeThaw();

    Model_Account::Data *account = Model_Account::instance().get(accountID_);
    if (account)
        m_currency = Model_Account::currency(account);
    else
        m_currency = Model_Currency::GetBaseCurrency();

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    this->windowsFreezeThaw();
    return TRUE;
}

mmStocksPanel::~mmStocksPanel()
{
}

void StocksListCtrl::save_column_width(int width)
{
    int i = width;
    int col_x = GetColumnWidth(i);
    Model_Setting::instance().Set(wxString::Format("STOCKS_COL%d_WIDTH", i),col_x);
}

void mmStocksPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    /* ---------------------- */
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY
        , wxDefaultPosition , wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    itemBoxSizer9->Add(headerPanel, 0, wxALIGN_CENTER_VERTICAL);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    header_text_ = new wxStaticText(headerPanel, wxID_STATIC, "");
    header_text_->SetFont(this->GetFont().Larger().Bold());

    header_total_ = new wxStaticText(headerPanel, wxID_STATIC, "");

    wxBoxSizer* itemBoxSizerHHeader = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerHHeader->Add(header_text_, 1, wxALIGN_CENTER_VERTICAL | wxALL, 1);

    itemBoxSizerVHeader->Add(itemBoxSizerHHeader, 1, wxEXPAND, 1);
    itemBoxSizerVHeader->Add(header_total_, 1, wxALIGN_CENTER_VERTICAL | wxALL, 1);

    /* ---------------------- */
    wxSplitterWindow* itemSplitterWindow10 = new wxSplitterWindow(this
        , wxID_ANY, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER);

    listCtrlAccount_ = new StocksListCtrl(this, itemSplitterWindow10, wxID_ANY);

    wxPanel* BottomPanel = new wxPanel(itemSplitterWindow10, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);

    itemSplitterWindow10->SplitHorizontally(listCtrlAccount_, BottomPanel);
    itemSplitterWindow10->SetMinimumPaneSize(100);
    itemSplitterWindow10->SetSashGravity(1.0);
    itemBoxSizer9->Add(itemSplitterWindow10, 1, wxGROW | wxALL, 1);

    wxBoxSizer* BoxSizerVBottom = new wxBoxSizer(wxVERTICAL);
    BottomPanel->SetSizer(BoxSizerVBottom);

    wxBoxSizer* BoxSizerHBottom = new wxBoxSizer(wxHORIZONTAL);
    BoxSizerVBottom->Add(BoxSizerHBottom, wxSizerFlags(g_flagsExpand).Border(0));

    wxButton* itemButton6 = new wxButton(BottomPanel, wxID_NEW, _("&New "));
    itemButton6->SetToolTip(_("New Stock Investment"));
    BoxSizerHBottom->Add(itemButton6, g_flags);

    wxButton* itemButton81 = new wxButton(BottomPanel, wxID_EDIT, _("&Edit "));
    itemButton81->SetToolTip(_("Edit Stock Investment"));
    BoxSizerHBottom->Add(itemButton81, g_flags);
    itemButton81->Enable(false);

    wxButton* itemButton7 = new wxButton(BottomPanel, wxID_DELETE, _("&Delete "));
    itemButton7->SetToolTip(_("Delete Stock Investment"));
    BoxSizerHBottom->Add(itemButton7, g_flags);
    itemButton7->Enable(false);

    wxButton* bMove = new wxButton(BottomPanel, wxID_MOVE_FRAME, _("&Move"));
    bMove->SetToolTip(_("Move selected transaction to another account"));
    BoxSizerHBottom->Add(bMove, g_flags);
    bMove->Enable(false);

    attachment_button_ = new wxBitmapButton(BottomPanel
        , wxID_FILE, wxBitmap(attachment_xpm), wxDefaultPosition
        , wxSize(bMove->GetSize().GetY(), bMove->GetSize().GetY()));
    attachment_button_->SetToolTip(_("Open attachments"));
    BoxSizerHBottom->Add(attachment_button_, g_flags);
    attachment_button_->Enable(false);

    refresh_button_ = new wxBitmapButton(BottomPanel
        , wxID_REFRESH, wxBitmap (led_off_xpm), wxDefaultPosition, wxSize(bMove->GetSize().GetY(), bMove->GetSize().GetY()));
    refresh_button_->SetLabelText(_("Refresh"));
    refresh_button_->SetToolTip(_("Refresh Stock Prices from Yahoo"));
    BoxSizerHBottom->Add(refresh_button_, g_flags);

    //Infobar-mini
    stock_details_short_ = new wxStaticText(BottomPanel, wxID_STATIC, strLastUpdate_);
    BoxSizerHBottom->Add(stock_details_short_, 1, wxGROW|wxTOP, 12);
    //Infobar
    stock_details_ = new wxStaticText(BottomPanel, wxID_STATIC, "",
        wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_WORDWRAP);
    BoxSizerVBottom->Add(stock_details_, 1, wxGROW|wxLEFT|wxRIGHT, 14);

    updateExtraStocksData(-1);
}

void StocksListCtrl::sortTable()
{
    std::sort(m_stocks.begin(), m_stocks.end());
    switch (m_selected_col)
    {
    case StocksListCtrl::COL_DATE:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByPURCHASEDATE());
        break;
    case StocksListCtrl::COL_NAME:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterBySTOCKNAME());
        break;
    case StocksListCtrl::COL_NUMBER:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByNUMSHARES());
        break;
    case StocksListCtrl::COL_VALUE:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByVALUE());
        break;
    case StocksListCtrl::COL_GAIN_LOSS:
        std::stable_sort(m_stocks.begin(), m_stocks.end()
            , [](const Model_Stock::Data& x, const Model_Stock::Data& y)
        {
            double valueX = x.VALUE - ((x.NUMSHARES * x.PURCHASEPRICE) + x.COMMISSION);
            double valueY = y.VALUE - ((y.NUMSHARES * y.PURCHASEPRICE) + y.COMMISSION);
            return valueX < valueY;
        });
        break;
    case StocksListCtrl::COL_CURRENT:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByCURRENTPRICE());
        break;
    case StocksListCtrl::COL_NOTES:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByNOTES());
        break;
    default:
        break;
    }
    if (!m_asc) std::reverse(m_stocks.begin(), m_stocks.end());
}

int StocksListCtrl::initVirtualListControl(int id, int col, bool asc)
{
    stock_panel_->updateHeader();
    /* Clear all the records */
    DeleteAllItems();

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(asc ? 3 : 2);
    SetColumn(col, item);

    m_stocks = Model_Stock::instance().find(Model_Stock::HELDAT(stock_panel_->accountID_));
    sortTable();

    int cnt = 0, selected_item = -1;
    for (const auto& stock: m_stocks)
    {
        if (id == stock.STOCKID)
        {
            selected_item = cnt;
            break;
        }
        ++cnt;
    }

    SetItemCount(m_stocks.size());
    return selected_item;
}

wxString mmStocksPanel::GetPanelTitle(const Model_Account::Data& account) const
{
    return wxString::Format(_("Stock Investments: %s"), account.ACCOUNTNAME);
}

wxString mmStocksPanel::BuildPage() const
{ 
    const Model_Account::Data* account = Model_Account::instance().get(accountID_);
    return listCtrlAccount_->BuildPage((account ? GetPanelTitle(*account) : ""));
}

const wxString mmStocksPanel::Total_Shares()
{
    double total_shares = 0;
    for (const auto& stock : Model_Stock::instance().find(Model_Stock::HELDAT(accountID_)))
    {
        total_shares += stock.NUMSHARES;
    }

    int precision = (total_shares - static_cast<int>(total_shares) != 0) ? 2 : 0;
    return Model_Currency::toString(total_shares, m_currency, precision);
}

void mmStocksPanel::updateHeader()
{
    const Model_Account::Data* account = Model_Account::instance().get(accountID_);
    double initVal = 0;
    // + Transfered from other accounts - Transfered to other accounts

    //Get Stock Investment Account Balance as Init Amount + sum (Value) - sum (Purchase Price)
    std::pair<double, double> investment_balance;
    if (account)
    {
        header_text_->SetLabel(GetPanelTitle(*account));
        //Get Init Value of the account
        initVal = account->INITIALBAL;
        investment_balance = Model_Account::investment_balance(account);
    }
    double originalVal = investment_balance.second;
    double total = investment_balance.first; 

    wxString balance = Model_Currency::toCurrency(total + initVal, m_currency);
    wxString original = Model_Currency::toCurrency(originalVal, m_currency);
    wxString diffStr = Model_Currency::toCurrency(total > originalVal ? total - originalVal : originalVal - total, m_currency);

    wxString lbl;
    lbl << _("Total Shares: ") << Total_Shares() << "     " << _("Total: ") << balance << "     " << _("Invested: ") << original;

    //Percent
    if (originalVal != 0.0) {
        if (total > originalVal)
            lbl << "     " << _("Gain: ");
        else
            lbl << "     " << _("Loss: ");
        double diffPercents = (total > originalVal ? total/originalVal*100.0-100.0 : -(total/originalVal*100.0-100.0));
        lbl << diffStr << "  ( " << Model_Currency::toString(diffPercents, m_currency, 2) << " %)";
    }

    header_total_->SetLabel(lbl);
}

void mmStocksPanel::OnDeleteStocks(wxCommandEvent& event)
{
    listCtrlAccount_->OnDeleteStocks(event);
}

void mmStocksPanel::OnMoveStocks(wxCommandEvent& event)
{
    listCtrlAccount_->OnMoveStocks(event);
}

void mmStocksPanel::OnNewStocks(wxCommandEvent& event)
{
    listCtrlAccount_->OnNewStocks(event);
}

void mmStocksPanel::OnEditStocks(wxCommandEvent& event)
{
    listCtrlAccount_->OnEditStocks(event);
}

void mmStocksPanel::OnOpenAttachment(wxCommandEvent& event)
{
    listCtrlAccount_->OnOpenAttachment(event);
}

void mmStocksPanel::OnRefreshQuotes(wxCommandEvent& WXUNUSED(event))
{
    wxString sError = "";
    if (onlineQuoteRefresh(sError))
    {
        const wxString header = _("Stock prices successfully updated");
        stock_details_->SetLabel(header);
        stock_details_short_->SetLabel(wxString::Format(_("Last updated %s"), strLastUpdate_));
        wxMessageDialog msgDlg(this, sError, header);
        msgDlg.ShowModal();
    }
    else
    {
        refresh_button_->SetBitmapLabel(wxBitmap(wxImage(led_red_xpm).Scale(16,16)));
        stock_details_->SetLabel(sError);
        stock_details_short_->SetLabel(_("Error"));
        mmShowErrorMessage(this, sError, _("Error"));
    }
}

/*** Trigger a quote download ***/
bool mmStocksPanel::onlineQuoteRefresh(wxString& sError)
{
    if(listCtrlAccount_->m_stocks.size() < 1)
    {
        sError = _("Nothing to update");
        return false;
    }

    //Symbol, (Amount, Name)
    std::map<wxString, std::pair<double, wxString> > stocks_data;
    wxString site = "";

    for (const auto &stock : listCtrlAccount_->m_stocks)
    {
        const wxString symbol = stock.SYMBOL.Upper();
        if (!symbol.IsEmpty())
        {
            if (stocks_data.find(symbol) == stocks_data.end())
            {
                stocks_data[symbol] = std::make_pair(stock.CURRENTPRICE, "");
                site << symbol << "+";
            }
        }
    }
    if (site.Right(1).Contains("+")) site.RemoveLast(1);

    //Sample : http://finance.yahoo.com/d/quotes.csv?s=SBER.ME+GAZP.ME&f=sl1n&e=.csv
    site = wxString::Format("http://download.finance.yahoo.com/d/quotes.csv?s=%s&f=sl1n&e=.csv"
        , site);

    refresh_button_->SetBitmapLabel(wxBitmap(wxImage(led_yellow_xpm).Scale(16,16)));
    stock_details_->SetLabel(_("Connecting..."));
    wxString sOutput;

    int err_code = site_content(site, sOutput);
    if (err_code != wxURL_NOERR)
    {
        sError = sOutput;
        return false;
    }

    //--//
    wxString StockSymbolWithSuffix, sName;
    double dPrice = 0.0;

    wxStringTokenizer tkz(sOutput, "\r\n");
    while (tkz.HasMoreTokens())
    {
        const wxString csvline = tkz.GetNextToken();
        StockSymbolWithSuffix = "";
        wxRegEx pattern("\"([^\"]+)\",([^,][0-9.]+),\"([^\"]*)\"");
        if (pattern.Matches(csvline))
        {
            StockSymbolWithSuffix = pattern.GetMatch(csvline, 1);
            pattern.GetMatch(csvline, 2).ToDouble(&dPrice);
            sName = pattern.GetMatch(csvline, 3);
        }

        bool updated = !StockSymbolWithSuffix.IsEmpty();

        //**** HACK HACK HACK
        // Note:
        // 1. If the share is a UK share (e.g. HSBA.L), its downloaded value in pence
        // 2. If the share is not a UK share (e.g. 0005.HK) while we are using UK Yahoo finance, we do not need
        //    to modify the price

        //// UK finance apparently downloads values in pence
        //if (!yahoo_->Server_.CmpNoCase("uk.finance.yahoo.com"))
        //    dPrice = dPrice / 100;
        //// ------------------
        if (updated && dPrice > 0)
        {
            //HACK seems outdated http://sourceforge.net/p/moneymanagerex/bugs/360/
            //if(StockSymbolWithSuffix.EndsWith(".L"))
            //    dPrice = dPrice / 100;
            stocks_data[StockSymbolWithSuffix].first = dPrice;
            stocks_data[StockSymbolWithSuffix].second = sName;
            sError << wxString::Format(_("%s\t -> %s\n")
                , StockSymbolWithSuffix, wxString::Format("%0.4f", dPrice));
        }
    }

    for (auto &s: listCtrlAccount_->m_stocks)
    {
        std::map<wxString, std::pair<double, wxString> >::const_iterator it = stocks_data.find(s.SYMBOL.Upper());
        if (it == stocks_data.end()) continue;
        dPrice = it->second.first;

        s.CURRENTPRICE = dPrice;
        s.VALUE = dPrice * s.NUMSHARES;
        if (s.STOCKNAME.empty()) s.STOCKNAME = it->second.second;
        Model_Stock::instance().save(&s);

        Model_StockHistory::instance().addUpdate(s.id(), wxDate::Now(), dPrice, Model_StockHistory::ONLINE);
    }

    // Now refresh the display
    int selected_id = -1;
    if (listCtrlAccount_->get_selectedIndex() > -1)
        selected_id = listCtrlAccount_->m_stocks[listCtrlAccount_->get_selectedIndex()].STOCKID;
    listCtrlAccount_->doRefreshItems(selected_id);

    // We are done!
    LastRefreshDT_       = wxDateTime::Now();
    StocksRefreshStatus_ = true;
    refresh_button_->SetBitmapLabel(wxBitmap(wxImage(led_green_xpm).Scale(16,16)));

    strLastUpdate_.Printf(_("%s on %s"), LastRefreshDT_.FormatTime()
        , LastRefreshDT_.FormatDate());
    Model_Infotable::instance().Set("STOCKS_LAST_REFRESH_DATETIME", strLastUpdate_);

    return true;
}

void mmStocksPanel::updateExtraStocksData(int selectedIndex)
{
    enableEditDeleteButtons(selectedIndex >= 0);
    if (selectedIndex >= 0)
    {
        const wxString additionInfo = listCtrlAccount_->getStockInfo(selectedIndex);
        stock_details_->SetLabel(additionInfo);
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
        stockavgPurchasePrice += s.PURCHASEPRICE * s.NUMSHARES;
    }
    stockavgPurchasePrice /= stocktotalnumShares;

    double sNumShares = m_stocks[selectedIndex].NUMSHARES;
    wxString sTotalNumShares;
    if ((stocktotalnumShares - static_cast<long>(stocktotalnumShares)) != 0.0)
        sTotalNumShares = wxString::Format("%.4f", stocktotalnumShares);
    else
        sTotalNumShares <<  static_cast<long>(stocktotalnumShares);
    wxString sGainLoss = Model_Currency::toCurrency(getGainLoss(selectedIndex));

    double stockPurchasePrice = m_stocks[selectedIndex].PURCHASEPRICE;
    double stockCurrentPrice = m_stocks[selectedIndex].CURRENTPRICE;
    double stockDifference = stockCurrentPrice - stockPurchasePrice;

    double stocktotalDifference = stockCurrentPrice - stockavgPurchasePrice;
    //Commision don't calculates here
    double stockPercentage = (stockCurrentPrice / stockPurchasePrice - 1.0)*100.0;
    double stocktotalPercentage = (stockCurrentPrice / stockavgPurchasePrice - 1.0)*100.0;
    double stocktotalgainloss = stocktotalDifference * stocktotalnumShares;

    wxString sPurchasePrice = Model_Currency::toCurrency(stockPurchasePrice);
    wxString sAvgPurchasePrice = Model_Currency::toCurrency(stockavgPurchasePrice);
    wxString sCurrentPrice = Model_Currency::toCurrency(stockCurrentPrice);
    wxString sDifference = Model_Currency::toCurrency(stockDifference);
    wxString sTotalDifference = Model_Currency::toCurrency(stocktotalDifference);
    wxString sPercentage = wxNumberFormatter::ToString(stockPercentage, 2);
    wxString sTotalPercentage = wxNumberFormatter::ToString(stocktotalPercentage, 2);
    wxString sTotalGainLoss = Model_Currency::toCurrency(stocktotalgainloss);

    wxString miniInfo = "";
    if (m_stocks[selectedIndex].SYMBOL != "")
        miniInfo << "\t" << _("Symbol: ") << m_stocks[selectedIndex].SYMBOL << "\t\t";
    miniInfo << _ ("Total:") << " (" << stocktotalnumShares << ") ";
    stock_panel_->stock_details_short_->SetLabel(miniInfo);

    wxString additionInfo = "";
    //Selected share info
    additionInfo
    << "|" << sCurrentPrice << " - " << sPurchasePrice << "|" << " = " << sDifference
    << " * " << sNumShares << " = " << sGainLoss << " ( " << sPercentage << "%"
    << " )" << "\n";
    //Summary for account for selected symbol
    if (purchasedTime > 1)
    {
        additionInfo << "|" << sCurrentPrice << " - " << sAvgPurchasePrice << "|" << " = " << sTotalDifference
        << " * " << sTotalNumShares << " = " << sTotalGainLoss << " ( " << sTotalPercentage << "%"
        << " )"
        << "\n" << OnGetItemText(selectedIndex, (long)COL_NOTES);
    }
    return additionInfo;
}
void mmStocksPanel::enableEditDeleteButtons(bool en)
{
    wxButton* bE = (wxButton*)FindWindow(wxID_EDIT);
    wxButton* bD = (wxButton*)FindWindow(wxID_DELETE);
    wxButton* bM = (wxButton*)FindWindow(wxID_MOVE_FRAME);
    if (bE) bE->Enable(en);
    if (bD) bD->Enable(en);
    if (bM) bM->Enable(en);
    attachment_button_->Enable(en);
    if (!en)
    {
        stock_details_->SetLabel(STOCKTIPS[rand() % sizeof(STOCKTIPS) / sizeof(wxString)]);
        stock_details_short_->SetLabel(wxString::Format(_("Last updated %s"), strLastUpdate_));
    }
}

void mmStocksPanel::call_dialog(int selectedIndex)
{
    Model_Stock::Data* stock = &listCtrlAccount_->m_stocks[selectedIndex];
    mmStockDialog dlg(this, stock, accountID_);
    if (dlg.ShowModal() == wxID_OK)
    {
        listCtrlAccount_->doRefreshItems(dlg.transID_);
    }
}
