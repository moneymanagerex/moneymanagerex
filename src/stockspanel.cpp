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
#include "attachmentdialog.h"
#include "constants.h"
#include "images_list.h"
#include "mmSimpleDialogs.h"
#include "stockdialog.h"
#include "util.h"

#include "model/Model_Attachment.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include "model/Model_StockHistory.h"

static const wxString STOCKTIPS[] = { 
    wxTRANSLATE("Using MMEX it is possible to track stocks/mutual funds investments."),
    wxTRANSLATE("To create new stocks entry the Symbol, Number of shares and Purchase prise should be entered."),
    wxTRANSLATE("Sample of UK (HSBC HLDG) share: HSBA.L"),
    wxTRANSLATE("If the Stock Name field is empty it will be filed when prices updated") };

enum {
    IDC_PANEL_STOCKS_LISTCTRL = wxID_HIGHEST + 1900,
    MENU_TREEPOPUP_EDIT,
    MENU_TREEPOPUP_DELETE,
    MENU_TREEPOPUP_NEW,
    MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS,
    MENU_HEADER_HIDE,
    MENU_HEADER_SORT,
    MENU_HEADER_RESET,
};

/*******************************************************/

wxBEGIN_EVENT_TABLE(StocksListCtrl, mmListCtrl)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, StocksListCtrl::OnListItemActivated)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, StocksListCtrl::OnListItemSelected)
    EVT_LIST_KEY_DOWN(wxID_ANY, StocksListCtrl::OnListKeyDown)
    EVT_MENU(MENU_TREEPOPUP_NEW, StocksListCtrl::OnNewStocks)
    EVT_MENU(MENU_TREEPOPUP_EDIT, StocksListCtrl::OnEditStocks)
    EVT_MENU(MENU_TREEPOPUP_DELETE, StocksListCtrl::OnDeleteStocks)
    EVT_MENU(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, StocksListCtrl::OnOrganizeAttachments)
    EVT_RIGHT_DOWN(StocksListCtrl::OnMouseRightClick)
    EVT_LEFT_DOWN(StocksListCtrl::OnListLeftClick)
wxEND_EVENT_TABLE()

StocksListCtrl::~StocksListCtrl()
{
    if (m_imageList) delete m_imageList;
}

StocksListCtrl::StocksListCtrl(mmStocksPanel* cp, wxWindow *parent, wxWindowID winid)
    : mmListCtrl(parent, winid)
    , stock_panel_(cp)
    , m_imageList(0)
{
    int x = mmIniOptions::instance().ico_size_;
    m_imageList = new wxImageList(x, x);
    m_imageList->Add(mmBitmap(png::PROFIT));
    m_imageList->Add(mmBitmap(png::LOSS));
    m_imageList->Add(mmBitmap(png::UPARROW));
    m_imageList->Add(mmBitmap(png::DOWNARROW));

    SetImageList(m_imageList, wxIMAGE_LIST_SMALL);

    // load the global variables
    m_selected_col = Model_Setting::instance().GetIntSetting("STOCKS_SORT_COL", col_sort());
    m_asc = Model_Setting::instance().GetBoolSetting("STOCKS_ASC", true);

    m_columns.push_back(std::make_tuple(" ", 25, wxLIST_FORMAT_LEFT));
    m_columns.push_back(std::make_tuple(_("ID"), wxLIST_AUTOSIZE, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(std::make_tuple(_("Purchase Date"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(std::make_tuple(_("Share Name"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(std::make_tuple(_("Share Symbol"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(std::make_tuple(_("Number of Shares"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(std::make_tuple(_("Unit Price"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(std::make_tuple(_("Total Price"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(std::make_tuple(_("Gain/Loss"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(std::make_tuple(_("Curr. unit value"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(std::make_tuple(_("Curr. total value"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(std::make_tuple(_("Value Date"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(std::make_tuple(_("Commission"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(std::make_tuple(_("Notes"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));

    m_col_width = "STOCKS_COL%d_WIDTH";
    m_default_sort_column = col_sort();

    for (const auto& entry : m_columns)
    {
        long count = GetColumnCount();
        InsertColumn(count
            , std::get<HEADER>(entry)
            , std::get<FORMAT>(entry)
            , Model_Setting::instance().GetIntSetting(wxString::Format(m_col_width, count), std::get<WIDTH>(entry)));
    }

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
    stock_panel_->OnListItemSelected(m_selected_row);

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

    PopupMenu(&menu, event.GetPosition());

    this->SetFocus();
}

wxString StocksListCtrl::OnGetItemText(long item, long column) const
{
    if (column == COL_ID)           return wxString::Format("%i", m_stocks[item].STOCKID).Trim();
    if (column == COL_DATE)         return mmGetDateForDisplay(mmGetStorageStringAsDate(m_stocks[item].PURCHASEDATE));
    if (column == COL_NAME)         return m_stocks[item].STOCKNAME;
    if (column == COL_SYMBOL)       return m_stocks[item].SYMBOL;
    if (column == COL_NUMBER)
    {
        int precision = m_stocks[item].NUMSHARES == floor(m_stocks[item].NUMSHARES) ? 0 : 4;
        return Model_Currency::toString(m_stocks[item].NUMSHARES, stock_panel_->m_currency, precision);
    }
    if (column == COL_PRICE)        return Model_Currency::toString(m_stocks[item].PURCHASEPRICE, stock_panel_->m_currency, 4);
    if (column == COL_VALUE)        return Model_Currency::toString(m_stocks[item].PURCHASEPRICE*m_stocks[item].NUMSHARES, stock_panel_->m_currency);
    if (column == COL_GAIN_LOSS)    return Model_Currency::toString(getGainLoss(item), stock_panel_->m_currency);
    if (column == COL_CURRENT)      return Model_Currency::toString(m_stocks[item].CURRENTPRICE, stock_panel_->m_currency, 4);
    if (column == COL_CURRVALUE)    return Model_Currency::toString(m_stocks[item].CURRENTPRICE*m_stocks[item].NUMSHARES, stock_panel_->m_currency);
    if (column == COL_PRICEDATE)    return mmGetDateForDisplay(mmGetStorageStringAsDate(Model_Stock::instance().lastPriceDate(&m_stocks[item])));
    if (column == COL_COMMISSION)   return Model_Currency::toString(m_stocks[item].COMMISSION, stock_panel_->m_currency);
    if (column == COL_NOTES)
    {
        wxString full_notes = m_stocks[item].NOTES;
        if (Model_Attachment::NrAttachments(Model_Attachment::reftype_desc(Model_Attachment::STOCK), m_stocks[item].STOCKID))
            full_notes = full_notes.Prepend(mmAttachmentManage::GetAttachmentNoteSign());
        return full_notes;
    }

    return wxEmptyString;
}

double StocksListCtrl::getGainLoss(long item) const
{
    return m_stocks[item].NUMSHARES * m_stocks[item].CURRENTPRICE - ((m_stocks[item].NUMSHARES * m_stocks[item].PURCHASEPRICE) + m_stocks[item].COMMISSION);
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

void StocksListCtrl::OnListLeftClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    long index = HitTest(wxPoint(event.m_x, event.m_y), Flags);
    if (index == -1)
    {
        m_selected_row = -1;
        stock_panel_->OnListItemSelected(m_selected_row);
    }
    event.Skip();
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
    mmStockDialog dlg(this, nullptr, stock_panel_->accountID_);
    dlg.ShowModal();
    if (Model_Stock::instance().get(dlg.stockID_))
    {
        doRefreshItems(dlg.stockID_);
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
    int ColumnNr;
    if (event.GetId() != MENU_HEADER_SORT)
        ColumnNr = event.GetColumn();
    else
        ColumnNr = m_ColumnHeaderNbr;
    if (0 >= ColumnNr || ColumnNr >= getColumnsNumber()) return;

    if (m_selected_col == ColumnNr && event.GetId() != MENU_HEADER_SORT) m_asc = !m_asc;

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(-1);
    SetColumn(m_selected_col, item);

    m_selected_col = ColumnNr;

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
mmStocksPanel::mmStocksPanel(int accountID
    , wxWindow *parent
    , wxWindowID winid, const wxPoint& pos, const wxSize& size, long style
    , const wxString& name)
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
    itemBoxSizer9->Add(itemSplitterWindow10, g_flagsExpandBorder1);

    wxBoxSizer* BoxSizerVBottom = new wxBoxSizer(wxVERTICAL);
    BottomPanel->SetSizer(BoxSizerVBottom);

    wxBoxSizer* BoxSizerHBottom = new wxBoxSizer(wxHORIZONTAL);
    BoxSizerVBottom->Add(BoxSizerHBottom, g_flagsBorder1);

    wxButton* itemButton6 = new wxButton(BottomPanel, wxID_NEW, _("&New "));
    itemButton6->SetToolTip(_("New Stock Investment"));
    BoxSizerHBottom->Add(itemButton6, 0, wxRIGHT, 5);

    wxButton* itemButton81 = new wxButton(BottomPanel, wxID_EDIT, _("&Edit "));
    itemButton81->SetToolTip(_("Edit Stock Investment"));
    BoxSizerHBottom->Add(itemButton81, 0, wxRIGHT, 5);
    itemButton81->Enable(false);

    wxButton* itemButton7 = new wxButton(BottomPanel, wxID_DELETE, _("&Delete "));
    itemButton7->SetToolTip(_("Delete Stock Investment"));
    BoxSizerHBottom->Add(itemButton7, 0, wxRIGHT, 5);
    itemButton7->Enable(false);

    wxButton* bMove = new wxButton(BottomPanel, wxID_MOVE_FRAME, _("&Move"));
    bMove->SetToolTip(_("Move selected transaction to another account"));
    BoxSizerHBottom->Add(bMove, 0, wxRIGHT, 5);
    bMove->Enable(false);

    attachment_button_ = new wxBitmapButton(BottomPanel
        , wxID_FILE, mmBitmap(png::CLIP), wxDefaultPosition
        , wxSize(30, bMove->GetSize().GetY()));
    attachment_button_->SetToolTip(_("Open attachments"));
    BoxSizerHBottom->Add(attachment_button_, 0, wxRIGHT, 5);
    attachment_button_->Enable(false);

    refresh_button_ = new wxBitmapButton(BottomPanel
        , wxID_REFRESH, mmBitmap (png::LED_OFF), wxDefaultPosition, wxSize(30, bMove->GetSize().GetY()));
    refresh_button_->SetLabelText(_("Refresh"));
    refresh_button_->SetToolTip(_("Refresh Stock Prices from Yahoo"));
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

void StocksListCtrl::sortTable()
{
    std::sort(m_stocks.begin(), m_stocks.end());
    switch (m_selected_col)
    {
    case StocksListCtrl::COL_ID:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterBySTOCKID());
        break;
    case StocksListCtrl::COL_DATE:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByPURCHASEDATE());
        break;
    case StocksListCtrl::COL_NAME:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterBySTOCKNAME());
        break;
    case StocksListCtrl::COL_SYMBOL:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterBySYMBOL());
        break;
    case StocksListCtrl::COL_NUMBER:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByNUMSHARES());
        break;
    case StocksListCtrl::COL_PRICE:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByPURCHASEPRICE());
        break;
    case StocksListCtrl::COL_VALUE:
        std::stable_sort(m_stocks.begin(), m_stocks.end()
            , [](const Model_Stock::Data& x, const Model_Stock::Data& y)
        {
            double valueX = x.PURCHASEPRICE * x.NUMSHARES;
            double valueY = y.PURCHASEPRICE * y.NUMSHARES;
            return valueX < valueY;
        });
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
    case StocksListCtrl::COL_CURRVALUE:
        std::stable_sort(m_stocks.begin(), m_stocks.end()
            , [](const Model_Stock::Data& x, const Model_Stock::Data& y)
        {
            double valueX = x.CURRENTPRICE * x.NUMSHARES;
            double valueY = y.CURRENTPRICE * y.NUMSHARES;
            return valueX < valueY;
        });
        break;
    case StocksListCtrl::COL_PRICEDATE:
        //TODO
        break;
    case StocksListCtrl::COL_COMMISSION:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByCOMMISSION());
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

    int precision = (total_shares - static_cast<int>(total_shares) != 0) ? 4 : 0;
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
        header_text_->SetLabelText(GetPanelTitle(*account));
        //Get Init Value of the account
        initVal = account->INITIALBAL;
        investment_balance = Model_Account::investment_balance(account);
    }
    double originalVal = investment_balance.second;
    double total = investment_balance.first; 

    const wxString& diffStr = Model_Currency::toCurrency(total > originalVal ? total - originalVal : originalVal - total, m_currency);
    double diffPercents = diffPercents = (total > originalVal ? total / originalVal*100.0 - 100.0 : -(total / originalVal*100.0 - 100.0));
    const wxString lbl = wxString::Format("%s     %s     %s     %s (%s %%)"
        , wxString::Format(_("Total Shares: %s"), Total_Shares())
        , wxString::Format(_("Total: %s"), Model_Currency::toCurrency(total + initVal, m_currency))
        , wxString::Format(_("Invested: %s"), Model_Currency::toCurrency(originalVal, m_currency))
        , wxString::Format(total > originalVal ? _("Gain: %s") : _("Loss: %s"), diffStr)
        , Model_Currency::toString(diffPercents, m_currency, 2));

    header_total_->SetLabelText(lbl);
    this->Layout();
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
        stock_details_->SetLabelText(header);
        stock_details_short_->SetLabelText(wxString::Format(_("Last updated %s"), strLastUpdate_));
        wxMessageDialog msgDlg(this, sError, header);
        msgDlg.ShowModal();
    }
    else
    {
        refresh_button_->SetBitmapLabel(mmBitmap(png::LED_RED));
        stock_details_->SetLabelText(sError);
        stock_details_short_->SetLabelText(_("Error"));
        mmErrorDialogs::MessageError(this, sError, _("Error"));
    }
}

/*** Trigger a quote download ***/
bool mmStocksPanel::onlineQuoteRefresh(wxString& sError)
{
    if(listCtrlAccount_->m_stocks.empty())
    {
        sError = _("Nothing to update");
        return false;
    }

    //Symbol, (Amount, Name)
    std::map<wxString, std::pair<double, wxString> > stocks_data;
    wxString site = "";

    for (const auto &stock : Model_Stock::instance().all())
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

    //Sample: http://finance.yahoo.com/d/quotes.csv?s=SBER.ME+GAZP.ME&f=sl1c4n&e=.csv
    //Sample CSV: "SBER.ME",85.49,"RUB","SBERBANK"
    site = wxString::Format(mmex::weblink::YahooQuotes, site);

    refresh_button_->SetBitmapLabel(mmBitmap(png::LED_YELLOW));
    stock_details_->SetLabelText(_("Connecting..."));
    wxString sOutput;

    int err_code = site_content(site, sOutput);
    if (err_code != wxURL_NOERR)
    {
        sError = sOutput;
        return false;
    }

    //--//
    wxString StockSymbolWithSuffix, sName, StockQuoteCurrency;
    double dPrice = 0.0;
    int count = 0;

    wxStringTokenizer tkz(sOutput, "\r\n");
    while (tkz.HasMoreTokens())
    {
        const wxString csvline = tkz.GetNextToken();
        StockSymbolWithSuffix = "";
        StockQuoteCurrency = "";
        wxRegEx pattern("\"([^\"]+)\",([^,][0-9.]+),\"([^\"]*)\",\"([^\"]*)\"");
        if (pattern.Matches(csvline))
        {
            StockSymbolWithSuffix = pattern.GetMatch(csvline, 1);
            pattern.GetMatch(csvline, 2).ToDouble(&dPrice);
            StockQuoteCurrency = pattern.GetMatch(csvline, 3);
            sName = pattern.GetMatch(csvline, 4);
        }

        bool updated = !StockSymbolWithSuffix.IsEmpty();

        /* HACK FOR GBP
        http://sourceforge.net/p/moneymanagerex/bugs/414/
        http://sourceforge.net/p/moneymanagerex/bugs/360/
        1. If the share has GBp as currency, its downloaded value in pence
        2. If the share has another currency, we don't need to modify the price
        */

        if (updated && dPrice > 0)
        {
            if (StockQuoteCurrency == "GBp")
                dPrice = dPrice / 100;
            stocks_data[StockSymbolWithSuffix].first = dPrice;
            stocks_data[StockSymbolWithSuffix].second = sName;
            sError << wxString::Format(_("%s\t -> %s\n")
                , StockSymbolWithSuffix, wxString::Format("%0.4f", dPrice));
            count++;
        }
    }

    if (count == 0)
    {
        sError = _("Quotes not found");
        return false;
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

        Model_StockHistory::instance().addUpdate(s.SYMBOL, wxDate::Now(), dPrice, Model_StockHistory::ONLINE);
    }

    // Now refresh the display
    int selected_id = -1;
    if (listCtrlAccount_->get_selectedIndex() > -1)
        selected_id = listCtrlAccount_->m_stocks[listCtrlAccount_->get_selectedIndex()].STOCKID;
    listCtrlAccount_->doRefreshItems(selected_id);

    // We are done!
    LastRefreshDT_       = wxDateTime::Now();
    StocksRefreshStatus_ = true;
    refresh_button_->SetBitmapLabel(mmBitmap(png::LED_GREEN));

    strLastUpdate_.Printf(_("%s on %s"), LastRefreshDT_.FormatTime()
        , mmGetDateForDisplay(LastRefreshDT_));
    Model_Infotable::instance().Set("STOCKS_LAST_REFRESH_DATETIME", strLastUpdate_);

    return true;
}

void mmStocksPanel::updateExtraStocksData(int selectedIndex)
{
    enableEditDeleteButtons(selectedIndex >= 0);
    if (selectedIndex >= 0)
    {
        const wxString additionInfo = listCtrlAccount_->getStockInfo(selectedIndex);
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
        stockavgPurchasePrice += s.PURCHASEPRICE * s.NUMSHARES;
    }
    stockavgPurchasePrice /= stocktotalnumShares;

    double numShares = m_stocks[selectedIndex].NUMSHARES;
    wxString sNumShares = wxString::Format("%i", (int)numShares);
    if (numShares - static_cast<long>(numShares) != 0.0)
        sNumShares = wxString::Format("%.4f", numShares);

    wxString sTotalNumShares = wxString::Format("%i", (int)stocktotalnumShares);
    if ((stocktotalnumShares - static_cast<long>(stocktotalnumShares)) != 0.0)
        sTotalNumShares = wxString::Format("%.4f", stocktotalnumShares);

    double stockPurchasePrice = m_stocks[selectedIndex].PURCHASEPRICE;
    double stockCurrentPrice = m_stocks[selectedIndex].CURRENTPRICE;
    double stockDifference = stockCurrentPrice - stockPurchasePrice;

    double stocktotalDifference = stockCurrentPrice - stockavgPurchasePrice;
    //Commision don't calculates here
    double stockPercentage = (stockCurrentPrice / stockPurchasePrice - 1.0)*100.0;
    double stocktotalPercentage = (stockCurrentPrice / stockavgPurchasePrice - 1.0)*100.0;
    double stocktotalgainloss = stocktotalDifference * stocktotalnumShares;

    const wxString& sPurchasePrice = Model_Currency::toCurrency(stockPurchasePrice, stock_panel_->m_currency, 4);
    const wxString& sAvgPurchasePrice = Model_Currency::toCurrency(stockavgPurchasePrice, stock_panel_->m_currency, 4);
    const wxString& sCurrentPrice = Model_Currency::toCurrency(stockCurrentPrice, stock_panel_->m_currency, 4);
    const wxString& sDifference = Model_Currency::toCurrency(stockDifference, stock_panel_->m_currency, 4);
    const wxString& sTotalDifference = Model_Currency::toCurrency(stocktotalDifference);

    wxString miniInfo = "";
    if (m_stocks[selectedIndex].SYMBOL != "")
        miniInfo << "\t" << wxString::Format(_("Symbol: %s"), m_stocks[selectedIndex].SYMBOL) << "\t\t";
    miniInfo << wxString::Format(_("Total: %s"), " (" + sTotalNumShares + ") ");
    stock_panel_->stock_details_short_->SetLabelText(miniInfo);

    //Selected share info
    wxString additionInfo = wxString::Format("|%s - %s| = %s, %s * %s = %s ( %s %% )\n"
        , sCurrentPrice, sPurchasePrice, sDifference
        , sDifference, sNumShares
        , Model_Currency::toCurrency(getGainLoss(selectedIndex))
        , wxNumberFormatter::ToString(stockPercentage, 2));

    //Summary for account for selected symbol
    if (purchasedTime > 1)
    {
        additionInfo += wxString::Format( "|%s - %s| = %s, %s * %s = %s ( %s %% )\n%s"
            ,  sCurrentPrice, sAvgPurchasePrice, sTotalDifference
            , sTotalDifference, sTotalNumShares
            , Model_Currency::toCurrency(stocktotalgainloss)
            , wxNumberFormatter::ToString(stocktotalPercentage, 2)
            , OnGetItemText(selectedIndex, (long)COL_NOTES));
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
        stock_details_->SetLabelText(_(STOCKTIPS[rand() % (sizeof(STOCKTIPS) / sizeof(wxString))]));
        stock_details_short_->SetLabelText(wxString::Format(_("Last updated %s"), strLastUpdate_));
    }
}

void mmStocksPanel::call_dialog(int selectedIndex)
{
    Model_Stock::Data* stock = &listCtrlAccount_->m_stocks[selectedIndex];
    mmStockDialog dlg(this, stock, accountID_);
    dlg.ShowModal();
    listCtrlAccount_->doRefreshItems(dlg.stockID_);
}
