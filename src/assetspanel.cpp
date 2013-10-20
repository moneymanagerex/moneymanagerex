/*******************************************************
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

#include "assetspanel.h"
#include "guiid.h"
#include "assetdialog.h"
#include "constants.h"
#include "model/Model_Setting.h"
#include "model/Model_Asset.h"
#include "model/Model_Currency.h"


enum {IDC_PANEL_ASSETS_LISTCTRL = wxID_HIGHEST + 1
    , IDC_PANEL_ASSET_STATIC_DETAILS
    , IDC_PANEL_ASSET_STATIC_DETAILS_MINI
};
/*******************************************************/
BEGIN_EVENT_TABLE(mmAssetsListCtrl, mmListCtrl)
    EVT_LIST_ITEM_ACTIVATED(IDC_PANEL_ASSETS_LISTCTRL,   mmAssetsListCtrl::OnListItemActivated)
    EVT_RIGHT_DOWN(mmAssetsListCtrl::OnMouseRightClick)
    EVT_LEFT_DOWN(mmAssetsListCtrl::OnMouseLeftClick)
    EVT_LIST_ITEM_SELECTED(IDC_PANEL_ASSETS_LISTCTRL,    mmAssetsListCtrl::OnListItemSelected)
    EVT_LIST_ITEM_DESELECTED(IDC_PANEL_ASSETS_LISTCTRL,  mmAssetsListCtrl::OnListItemDeselected)
    EVT_LIST_COL_END_DRAG(IDC_PANEL_ASSETS_LISTCTRL,     mmAssetsListCtrl::OnItemResize)
    EVT_LIST_COL_CLICK(IDC_PANEL_ASSETS_LISTCTRL,        mmAssetsListCtrl::OnColClick)
    EVT_LIST_END_LABEL_EDIT(IDC_PANEL_ASSETS_LISTCTRL,   mmAssetsListCtrl::OnEndLabelEdit)

    EVT_MENU(MENU_TREEPOPUP_NEW,    mmAssetsListCtrl::OnNewAsset)
    EVT_MENU(MENU_TREEPOPUP_EDIT,   mmAssetsListCtrl::OnEditAsset)
    EVT_MENU(MENU_TREEPOPUP_DELETE, mmAssetsListCtrl::OnDeleteAsset)
    EVT_MENU(MENU_ON_DUPLICATE_TRANSACTION, mmAssetsListCtrl::OnDuplicateAsset)

    EVT_LIST_KEY_DOWN(wxID_ANY, mmAssetsListCtrl::OnListKeyDown)
END_EVENT_TABLE()
/*******************************************************/

mmAssetsListCtrl::mmAssetsListCtrl(mmAssetsPanel* cp, wxWindow *parent, wxWindowID winid)
    : mmListCtrl(parent, winid)
    , cp_(cp)
{}

void mmAssetsListCtrl::OnItemResize(wxListEvent& event)
{
    int i = event.GetColumn();
    int width = this->GetColumnWidth(i);
    Model_Setting::instance().Set(wxString::Format("ASSETS_COL%d_WIDTH", i), width);
}

void mmAssetsListCtrl::OnMouseRightClick(wxMouseEvent& event)
{
    wxMenu menu;
    menu.Append(MENU_TREEPOPUP_NEW, _("&New Asset"));
    menu.AppendSeparator();
    menu.Append(MENU_ON_DUPLICATE_TRANSACTION, _("D&uplicate Asset"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_EDIT, _("&Edit Asset"));
    menu.Append(MENU_TREEPOPUP_DELETE, _("&Delete Asset"));
    if (m_selected_row < 0)
    {
        menu.Enable(MENU_ON_DUPLICATE_TRANSACTION, false);
        menu.Enable(MENU_TREEPOPUP_EDIT, false);
        menu.Enable(MENU_TREEPOPUP_DELETE, false);
    }
    PopupMenu(&menu, event.GetPosition());
}

void mmAssetsListCtrl::OnMouseLeftClick(wxMouseEvent& event)
{
    m_selected_row = -1;
    event.Skip();
}

wxString mmAssetsListCtrl::OnGetItemText(long item, long column) const
{
    return cp_->getItem(item, column);
}

void mmAssetsListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selected_row = event.GetIndex();
    cp_->updateExtraAssetData(m_selected_row);
}

void mmAssetsListCtrl::OnListItemDeselected(wxListEvent& /*event*/)
{
    m_selected_row = -1;
    cp_->updateExtraAssetData(m_selected_row);
}

int mmAssetsListCtrl::OnGetItemImage(long item) const
{
    return Model_Asset::type(cp_->m_assets[item]);
}

void mmAssetsListCtrl::OnListKeyDown(wxListEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE)
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_DELETE);
        OnDeleteAsset(evt);
    }
    else
    {
        event.Skip();
    }
}

void mmAssetsListCtrl::OnNewAsset(wxCommandEvent& /*event*/)
{
    mmAssetDialog dlg(this, 0);
    if (dlg.ShowModal() == wxID_OK)
    {
        doRefreshItems(dlg.m_asset->ASSETID);
    }
}

void mmAssetsListCtrl::doRefreshItems(int trx_id)
{
    int selectedIndex = cp_->initVirtualListControl(trx_id, m_selected_col, m_asc);

    long cnt = static_cast<long>(cp_->m_assets.size());

    if (selectedIndex >= cnt || selectedIndex < 0)
        selectedIndex = m_asc ? cnt - 1 : 0;

    if (cnt>0)
        RefreshItems(0, cnt > 0 ? --cnt : 0);
    else
        selectedIndex = -1;

    if (selectedIndex >= 0 && cnt>0)
    {
        SetItemState(selectedIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(selectedIndex, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        EnsureVisible(selectedIndex);
    }
    m_selected_row = selectedIndex;
}

void mmAssetsListCtrl::OnDeleteAsset(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0)    return;

    wxMessageDialog msgDlg(this
        , _("Do you really want to delete the Asset?")
        , _("Confirm Asset Deletion")
        , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);

    if (msgDlg.ShowModal() == wxID_YES)
    {
        const Model_Asset::Data& asset = cp_->m_assets[m_selected_row];
        Model_Asset::instance().remove(asset.ASSETID);

        cp_->initVirtualListControl(m_selected_row, m_selected_col, m_asc);
        m_selected_row = -1;
        cp_->updateExtraAssetData(m_selected_row);
    }
}

void mmAssetsListCtrl::OnEditAsset(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0)     return;

    wxListEvent evt(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, IDC_PANEL_ASSETS_LISTCTRL);
    AddPendingEvent(evt);
}

void mmAssetsListCtrl::OnDuplicateAsset(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0)     return;

    const Model_Asset::Data& asset = cp_->m_assets[m_selected_row];
    Model_Asset::Data* duplicate_asset = Model_Asset::instance().clone(&asset);

    if (EditAsset(duplicate_asset))
    {
        cp_->initVirtualListControl();
        doRefreshItems(duplicate_asset->ASSETID);
    }
}

void mmAssetsListCtrl::OnListItemActivated(wxListEvent& event)
{
    if (m_selected_row < 0)
    {
        m_selected_row = event.GetIndex();
    }
    EditAsset(&(cp_->m_assets[m_selected_row]));
}

bool mmAssetsListCtrl::EditAsset(Model_Asset::Data* pEntry)
{
    mmAssetDialog dlg(this, pEntry);
    bool edit = true;
    if (dlg.ShowModal() == wxID_OK)
    {
        doRefreshItems(dlg.m_asset->ASSETID);
        cp_->updateExtraAssetData(m_selected_row);
    }
    else edit = false;

    return edit;
}

void mmAssetsListCtrl::OnColClick(wxListEvent& event)
{
    if(0 > event.GetColumn() || event.GetColumn() >= mmAssetsPanel::COL_MAX) return;

    if (m_selected_col == event.GetColumn()) m_asc = !m_asc;

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(-1);
    SetColumn(m_selected_col, item);

    m_selected_col = event.GetColumn();

    item.SetImage(m_asc ? 8 : 7);
    SetColumn(m_selected_col, item);

    int trx_id = -1;
    if (m_selected_row>=0) trx_id = cp_->m_assets[m_selected_row].ASSETID;

    doRefreshItems(trx_id);
}

void mmAssetsListCtrl::OnEndLabelEdit(wxListEvent& event)
{
    if (event.IsEditCancelled()) return;
    Model_Asset::Data* asset = &cp_->m_assets[event.GetIndex()];
    asset->ASSETNAME = event.m_item.m_text;
    Model_Asset::instance().save(asset);
    RefreshItems(event.GetIndex(), event.GetIndex());
}
/*******************************************************/
BEGIN_EVENT_TABLE(mmAssetsPanel, wxPanel)
    EVT_BUTTON(wxID_NEW, mmAssetsPanel::OnNewAsset)
    EVT_BUTTON(wxID_EDIT, mmAssetsPanel::OnEditAsset)
    EVT_BUTTON(wxID_DELETE, mmAssetsPanel::OnDeleteAsset)
    EVT_MENU(wxID_ANY, mmAssetsPanel::OnViewPopupSelected)
END_EVENT_TABLE()
/*******************************************************/

mmAssetsPanel::mmAssetsPanel(wxWindow *parent)
: m_filter_type(Model_Asset::TYPE(-1))
{
    this->tips_ = _("MMEX allows you to track fixed assets like cars, houses, land and others. Each asset can have its value appreciate by a certain rate per year, depreciate by a certain rate per year, or not change in value. The total assets are added to your total financial worth.");
    Create(parent, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, wxPanelNameStr);
}

bool mmAssetsPanel::Create(wxWindow *parent, wxWindowID winid, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    if (!wxPanel::Create(parent, winid, pos, size, style, name)) return false;

    this->windowsFreezeThaw();

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    initVirtualListControl();
    if (!this->m_assets.empty())
        m_listCtrlAssets->EnsureVisible(this->m_assets.size() - 1);

    this->windowsFreezeThaw();

    return true;
}

void mmAssetsPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    /* ---------------------- */
    wxPanel* headerPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition,
        wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer9->Add(headerPanel, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxGROW, 5);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    int font_size = this->GetFont().GetPointSize() + 2;

    wxStaticText* itemStaticText9 = new wxStaticText( headerPanel, wxID_STATIC, _("Assets"));
    itemStaticText9->SetFont(wxFont(font_size, wxSWISS, wxNORMAL, wxBOLD, FALSE, ""));
    itemBoxSizerVHeader->Add(itemStaticText9, 0, wxALL, 1);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerVHeader->Add(itemBoxSizerHHeader2);

    wxBitmap itemStaticBitmap(rightarrow_xpm);
    wxStaticBitmap* itemStaticBitmap3 = new wxStaticBitmap( headerPanel, wxID_STATIC, itemStaticBitmap);
    itemBoxSizerHHeader2->Add(itemStaticBitmap3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    //itemStaticBitmap3->Connect(ID_PANEL_CHECKING_STATIC_BITMAP_VIEW, wxEVT_RIGHT_DOWN, wxMouseEventHandler(mmAssetsPanel::OnFilterResetToViewAll), NULL, this);
    itemStaticBitmap3->Connect(wxID_STATIC, wxEVT_LEFT_DOWN, wxMouseEventHandler(mmAssetsPanel::OnMouseLeftDown), NULL, this);

    itemStaticTextMainFilter_ = new wxStaticText( headerPanel, wxID_STATIC, _("All"));
    itemBoxSizerHHeader2->Add(itemStaticTextMainFilter_, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxGROW, 5);

    header_text_ = new wxStaticText( headerPanel, wxID_STATIC, _("Total:"));
    itemBoxSizerVHeader->Add(header_text_, 0, wxALL, 1);

    /* ---------------------- */

    wxSplitterWindow* itemSplitterWindow10 = new wxSplitterWindow( this, wxID_STATIC,
        wxDefaultPosition, wxSize(200, 200), wxSP_3DBORDER|wxSP_3DSASH|wxNO_BORDER);

    m_listCtrlAssets = new mmAssetsListCtrl(this, itemSplitterWindow10, IDC_PANEL_ASSETS_LISTCTRL);

    wxSize imageSize(16, 16);
    m_imageList.reset(new wxImageList(imageSize.GetWidth(), imageSize.GetHeight()));
    //TODO: Provide better icons
    m_imageList->Add(wxBitmap(wxImage(house_xpm).Scale(16, 16)));           // Property
    m_imageList->Add(wxBitmap(wxImage(car_xpm).Scale(16, 16)));             // Automobile
    m_imageList->Add(wxBitmap(wxImage(clock_xpm).Scale(16, 16)));           // Household Object
    m_imageList->Add(wxBitmap(wxImage(art_xpm).Scale(16, 16)));             // Art
    m_imageList->Add(wxBitmap(wxImage(assets_xpm).Scale(16, 16)));      // Jewellery
    m_imageList->Add(wxBitmap(wxImage(coin_xpm).Scale(16, 16)));    // Cash
    m_imageList->Add(wxBitmap(wxImage(rubik_cube_xpm).Scale(16, 16)));          // Other
    m_imageList->Add(wxBitmap(wxImage(uparrow_xpm).Scale(16, 16)));
    m_imageList->Add(wxBitmap(wxImage(downarrow_xpm).Scale(16, 16)));

    m_listCtrlAssets->SetImageList(m_imageList.get(), wxIMAGE_LIST_SMALL);
    m_listCtrlAssets->InsertColumn(COL_NAME, _("Name"));

    wxListItem itemCol;
    itemCol.SetImage(-1);
    itemCol.SetAlign(wxLIST_FORMAT_LEFT);
    itemCol.SetText(_("Type"));
    m_listCtrlAssets->InsertColumn(COL_TYPE, itemCol);

    itemCol.SetAlign(wxLIST_FORMAT_RIGHT);
    itemCol.SetText(_("Initial Value"));
    m_listCtrlAssets->InsertColumn(COL_VALUE_INITIAL, itemCol);

    itemCol.SetAlign(wxLIST_FORMAT_RIGHT);
    itemCol.SetText(_("Current Value"));
    m_listCtrlAssets->InsertColumn(COL_VALUE_CURRENT, itemCol);

    itemCol.SetAlign(wxLIST_FORMAT_RIGHT);
    itemCol.SetText(_("Date"));
    m_listCtrlAssets->InsertColumn(COL_DATE, itemCol);

    itemCol.SetAlign(wxLIST_FORMAT_LEFT);
    itemCol.SetText(_("Notes"));
    m_listCtrlAssets->InsertColumn(COL_NOTES, itemCol);

    /* See if we can get data from inidb */
    int col0 = Model_Setting::instance().GetIntSetting("ASSETS_COL0_WIDTH", 150);
    int col1 = Model_Setting::instance().GetIntSetting("ASSETS_COL1_WIDTH", -2);
    int col2 = Model_Setting::instance().GetIntSetting("ASSETS_COL2_WIDTH", -2);
    int col3 = Model_Setting::instance().GetIntSetting("ASSETS_COL3_WIDTH", -2);
    int col4 = Model_Setting::instance().GetIntSetting("ASSETS_COL4_WIDTH", 450);

    m_listCtrlAssets->SetColumnWidth(COL_NAME, col0);
    m_listCtrlAssets->SetColumnWidth(COL_DATE, col1);
    m_listCtrlAssets->SetColumnWidth(COL_TYPE, col2);
    m_listCtrlAssets->SetColumnWidth(COL_VALUE_INITIAL, col3);
    m_listCtrlAssets->SetColumnWidth(COL_VALUE_CURRENT, col3);
    m_listCtrlAssets->SetColumnWidth(COL_NOTES, col4);

    wxPanel* assets_panel = new wxPanel( itemSplitterWindow10, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );

    itemSplitterWindow10->SplitHorizontally(m_listCtrlAssets, assets_panel);
    itemSplitterWindow10->SetMinimumPaneSize(100);
    itemSplitterWindow10->SetSashGravity(1.0);
    itemBoxSizer9->Add(itemSplitterWindow10, 1, wxGROW|wxALL, 1);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    assets_panel->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxALIGN_LEFT|wxALL, 3);

    wxButton* itemButton6 = new wxButton( assets_panel, wxID_NEW);
    itemButton6->SetToolTip(_("New Asset"));
    itemBoxSizer5->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 4);

    wxButton* itemButton81 = new wxButton( assets_panel, wxID_EDIT);
    itemButton81->SetToolTip(_("Edit Asset"));
    itemBoxSizer5->Add(itemButton81, 0, wxALIGN_CENTER_VERTICAL|wxALL, 4);
    itemButton81->Enable(false);

    wxButton* itemButton7 = new wxButton( assets_panel, wxID_DELETE);
    itemButton7->SetToolTip(_("Delete Asset"));
    itemBoxSizer5->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 4);
    itemButton7->Enable(false);

    //Infobar-mini
    wxStaticText* itemStaticText44 = new wxStaticText( assets_panel, IDC_PANEL_ASSET_STATIC_DETAILS_MINI, "");
    itemBoxSizer5->Add(itemStaticText44, 1, wxGROW|wxTOP, 12);

    //Infobar
    wxStaticText* itemStaticText33 = new wxStaticText( assets_panel,
        IDC_PANEL_ASSET_STATIC_DETAILS, "", wxDefaultPosition, wxSize(200,-1), wxTE_MULTILINE|wxTE_WORDWRAP);
    itemBoxSizer4->Add(itemStaticText33, 1, wxGROW|wxLEFT|wxRIGHT, 14);

    updateExtraAssetData(-1);
}

void mmAssetsPanel::sortTable()
{
    std::sort(this->m_assets.begin(), this->m_assets.end());
    switch (this->m_listCtrlAssets->m_selected_col)
    {
    case COL_NAME:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end()
            , [] (const Model_Asset::Data& x, const Model_Asset::Data& y)
            {
                if (x.ASSETNAME != y.ASSETNAME) return x.ASSETNAME < y.ASSETNAME;
                else return x.STARTDATE < y.STARTDATE;
            });
        break;
    case COL_TYPE:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end()
            , [](const Model_Asset::Data& x, const Model_Asset::Data& y)
            {
                if (x.ASSETTYPE != y.ASSETTYPE) return x.ASSETTYPE < y.ASSETTYPE;
                else return x.STARTDATE < y.STARTDATE;
            });
        break;
    case COL_VALUE_INITIAL:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end()
            , [](const Model_Asset::Data& x, const Model_Asset::Data& y)
            {
                if (x.VALUE != y.VALUE) return x.VALUE < y.VALUE;
                else return x.STARTDATE < y.STARTDATE;
            });
        break;
    case COL_VALUE_CURRENT:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end()
            , [](const Model_Asset::Data& x, const Model_Asset::Data& y)
            {
                if (Model_Asset::value(x) != Model_Asset::value(y)) return Model_Asset::value(x) < Model_Asset::value(y);
                else return x.STARTDATE < y.STARTDATE;
            });
        break;
    case COL_DATE:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end()
            , [](const Model_Asset::Data& x, const Model_Asset::Data& y)
            {
                if (x.STARTDATE != y.STARTDATE) return x.STARTDATE < y.STARTDATE;
                else return x.STARTDATE < y.STARTDATE;
            });
        break;
    case COL_NOTES:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end()
            , [](const Model_Asset::Data& x, const Model_Asset::Data& y)
            {
                if (x.NOTES != y.NOTES) return x.NOTES < y.NOTES;
                else return x.STARTDATE < y.STARTDATE;
            });
    default:
        break;
    }

    if (!this->m_listCtrlAssets->m_asc) std::reverse(this->m_assets.begin(), this->m_assets.end());
}

int mmAssetsPanel::initVirtualListControl(int id, int col, bool asc)
{
    /* Clear all the records */
    m_listCtrlAssets->DeleteAllItems();

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(asc ? 8 : 7);
    m_listCtrlAssets->SetColumn(col, item);

    if (this->m_filter_type == Model_Asset::TYPE(-1)) // ALL
        this->m_assets = Model_Asset::instance().all();
    else
        this->m_assets = Model_Asset::instance().find(Model_Asset::ASSETTYPE(m_filter_type));
    this->sortTable();

    m_listCtrlAssets->SetItemCount(this->m_assets.size());

    double balance = 0.0;
    for (const auto& asset: this->m_assets) balance += Model_Asset::value(asset); 
    header_text_->SetLabel(Model_Currency::toString(balance)); // balance

    int selected_item = 0;
    for (const auto& asset: this->m_assets)
    {
        if (asset.ASSETID == id) return selected_item;
        ++ selected_item;
    }
    return -1;
}

void mmAssetsPanel::OnDeleteAsset(wxCommandEvent& event)
{
    m_listCtrlAssets->OnDeleteAsset(event);
}

void mmAssetsPanel::OnNewAsset(wxCommandEvent& event)
{
    m_listCtrlAssets->OnNewAsset(event);
}

void mmAssetsPanel::OnEditAsset(wxCommandEvent& event)
{
    m_listCtrlAssets->OnEditAsset(event);
}

wxString mmAssetsPanel::getItem(long item, long column)
{
    const Model_Asset::Data& asset = this->m_assets[item];
    switch (column)
    {
    case COL_NAME:
        return asset.ASSETNAME;
    case COL_TYPE:
        return wxGetTranslation(asset.ASSETTYPE);
    case COL_VALUE_INITIAL:
        return Model_Currency::toString(asset.VALUE);
    case COL_VALUE_CURRENT:
        return Model_Currency::toString(Model_Asset::value(asset));
    case COL_DATE:
        return mmGetDateForDisplay(Model_Asset::STARTDATE(asset));
    case COL_NOTES:
        return asset.NOTES;
    default:
        return "";
    }
}

void mmAssetsPanel::updateExtraAssetData(int selIndex)
{
    wxStaticText* st = (wxStaticText*)FindWindow(IDC_PANEL_ASSET_STATIC_DETAILS);
    wxStaticText* stm = (wxStaticText*)FindWindow(IDC_PANEL_ASSET_STATIC_DETAILS_MINI);
    if (selIndex > -1)
    {
        const Model_Asset::Data& asset = this->m_assets[selIndex];
        enableEditDeleteButtons(true);
        wxString miniInfo;
        miniInfo << "\t" << _("Change in Value") << ": "<< wxGetTranslation(asset.VALUECHANGE);
        if (Model_Asset::rate(asset) != Model_Asset::RATE_NONE)
            miniInfo<< " = " << asset.VALUECHANGERATE<< "%";

        st->SetLabel(asset.NOTES);
        stm->SetLabel(miniInfo);
    }
    else
    {
        stm -> SetLabel("");
        st->SetLabel(this->tips_);
        enableEditDeleteButtons(false);
    }
}

void mmAssetsPanel::enableEditDeleteButtons(bool enable)
{
    wxButton* btn = static_cast<wxButton*>(FindWindow(wxID_EDIT));
    wxASSERT(btn);
    btn->Enable(enable);

    btn = static_cast<wxButton*>(FindWindow(wxID_DELETE));
    wxASSERT(btn);
    btn->Enable(enable);
}

void mmAssetsPanel::OnMouseLeftDown ( wxMouseEvent& event )
{
    int i = 0;
    wxMenu menu;
    menu.Append(++i, wxGetTranslation(wxTRANSLATE("All")));

    for (const auto& type: Model_Asset::all_type())
    {
        menu.Append(++i, wxGetTranslation(type));
    }
    PopupMenu(&menu);

    event.Skip();
}

void mmAssetsPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    int evt = std::max(event.GetId() - 1, 0);

    if (evt == 0)
    {
        itemStaticTextMainFilter_->SetLabel(_("All"));
        this->m_filter_type = Model_Asset::TYPE(-1);
    }
    else
    {
        this->m_filter_type = Model_Asset::TYPE(evt - 1);
        itemStaticTextMainFilter_->SetLabel(wxGetTranslation(Model_Asset::all_type()[evt - 1]));
    }

    int trx_id = -1;
    m_listCtrlAssets->doRefreshItems(trx_id);
    updateExtraAssetData(trx_id);
}
