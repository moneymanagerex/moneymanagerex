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
#include "mmex_settings.h"

/*******************************************************/
BEGIN_EVENT_TABLE(mmAssetsListCtrl, mmListCtrl)
    EVT_LIST_ITEM_ACTIVATED(IDC_PANEL_STOCKS_LISTCTRL,   mmAssetsListCtrl::OnListItemActivated)
    EVT_RIGHT_DOWN(mmAssetsListCtrl::OnMouseRightClick)
    EVT_LEFT_DOWN(mmAssetsListCtrl::OnMouseLeftClick)
    EVT_LIST_ITEM_SELECTED(IDC_PANEL_STOCKS_LISTCTRL,    mmAssetsListCtrl::OnListItemSelected)
    EVT_LIST_ITEM_DESELECTED(IDC_PANEL_STOCKS_LISTCTRL,  mmAssetsListCtrl::OnListItemDeselected)
    EVT_LIST_COL_END_DRAG(IDC_PANEL_STOCKS_LISTCTRL,     mmAssetsListCtrl::OnItemResize)
    EVT_LIST_COL_CLICK(IDC_PANEL_STOCKS_LISTCTRL,        mmAssetsListCtrl::OnColClick)

    EVT_MENU(MENU_TREEPOPUP_NEW,    mmAssetsListCtrl::OnNewAsset)
    EVT_MENU(MENU_TREEPOPUP_EDIT,   mmAssetsListCtrl::OnEditAsset)
    EVT_MENU(MENU_TREEPOPUP_DELETE, mmAssetsListCtrl::OnDeleteAsset)
    EVT_MENU(MENU_ON_DUPLICATE_TRANSACTION, mmAssetsListCtrl::OnDuplicateAsset)

    EVT_LIST_KEY_DOWN(wxID_ANY, mmAssetsListCtrl::OnListKeyDown)
END_EVENT_TABLE()
/*******************************************************/

mmAssetsListCtrl::mmAssetsListCtrl(mmAssetsPanel* cp, wxWindow *parent,
const wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: mmListCtrl(parent, id, pos, size, style)
, cp_(cp)
, selectedIndex_(-1)
{}

void mmAssetsListCtrl::OnItemResize(wxListEvent& event)
{
    int i = event.GetColumn();
    int width = cp_->GetListCtrlWidth(i);
    cp_->core_->iniSettings_->SetSetting(wxString::Format("ASSETS_COL%d_WIDTH", i), width);
}

void mmAssetsListCtrl::InitVariables()
{
    m_selected_col = 0;
    m_asc = true;
    cp_->SetFilter(" 'Property','Automobile','Household Object','Art','Jewellery','Cash','Other' ");
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
    if (selectedIndex_ < 0)
    {
        menu.Enable(MENU_ON_DUPLICATE_TRANSACTION, false);
        menu.Enable(MENU_TREEPOPUP_EDIT, false);
        menu.Enable(MENU_TREEPOPUP_DELETE, false);
    }
    PopupMenu(&menu, event.GetPosition());
}

void mmAssetsListCtrl::OnMouseLeftClick(wxMouseEvent& event)
{
    selectedIndex_ = -1;
    event.Skip();
}

wxString mmAssetsListCtrl::OnGetItemText(long item, long column) const
{
    return cp_->getItem(item, column);
}

void mmAssetsListCtrl::OnListItemSelected(wxListEvent& event)
{
    selectedIndex_ = event.GetIndex();
    cp_->updateExtraAssetData(selectedIndex_);
}

void mmAssetsListCtrl::OnListItemDeselected(wxListEvent& /*event*/)
{
    selectedIndex_ = -1;
    cp_->updateExtraAssetData(selectedIndex_);
}

int mmAssetsListCtrl::OnGetItemImage(long item) const
{
    int image_id = 0;
    size_t size = sizeof(ASSET_TYPE_DEF)/sizeof(wxString);
    for(size_t i = 0; i < size; ++i)
    {
        if (ASSET_TYPE_DEF[i] == cp_->AssetList().entrylist_[item]->type_)
            image_id = i;
    }

    return image_id;
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
    mmAssetDialog dlg(this, cp_->core_, cp_, NULL, false);
    if (dlg.ShowModal() == wxID_OK)
    {
        doRefreshItems(dlg.GetAssetID());
    }
}

void mmAssetsListCtrl::doRefreshItems(int trx_id)
{
    int selectedIndex = cp_->initVirtualListControl(trx_id, m_selected_col, m_asc);

    long cnt = static_cast<long>(cp_->AssetList().entrylist_.size());

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
    selectedIndex_ = selectedIndex;
}

void mmAssetsListCtrl::OnDeleteAsset(wxCommandEvent& /*event*/)
{
    if (selectedIndex_ < 0)    return;

    wxMessageDialog msgDlg(this
        , _("Do you really want to delete the Asset?")
        , _("Confirm Asset Deletion")
        , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);

    if (msgDlg.ShowModal() == wxID_YES)
    {
        cp_->AssetList().DeleteEntry(cp_->AssetList().GetIndexedEntryPtr(selectedIndex_)->GetId());

        cp_->initVirtualListControl(selectedIndex_, m_selected_col, m_asc);
        selectedIndex_ = -1;
        cp_->updateExtraAssetData(selectedIndex_);
    }
}

void mmAssetsListCtrl::OnEditAsset(wxCommandEvent& /*event*/)
{
    if (selectedIndex_ < 0)     return;

    wxListEvent evt(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, IDC_PANEL_STOCKS_LISTCTRL);
    AddPendingEvent(evt);
}

void mmAssetsListCtrl::OnDuplicateAsset(wxCommandEvent& /*event*/)
{
    if (selectedIndex_ < 0)     return;

    int original_index = selectedIndex_;

    // Duplicate the asset entry
    TAssetEntry* pNewEntry = new TAssetEntry(cp_->AssetList().entrylist_[selectedIndex_]);
    int new_asset_id = cp_->AssetList().AddEntry(pNewEntry);
    // Locate new entry in the visual list.
    cp_->AssetList().GetEntryPtr(new_asset_id);
    selectedIndex_ = cp_->AssetList().GetCurrentIndex();

    if (! EditAsset(pNewEntry))
    {
        // remove the duplicate asset canceled by user.
        cp_->AssetList().DeleteEntry(new_asset_id);
        selectedIndex_ = original_index;
        cp_->updateExtraAssetData(selectedIndex_);
    }
}

void mmAssetsListCtrl::OnListItemActivated(wxListEvent& event)
{
    selectedIndex_ = event.GetIndex();
    EditAsset(cp_->AssetList().entrylist_[selectedIndex_]);
}

bool mmAssetsListCtrl::EditAsset(TAssetEntry* pEntry)
{
    mmAssetDialog dlg(this, cp_->core_, cp_, pEntry, true);
    bool edit = true;
    if (dlg.ShowModal() == wxID_OK)
    {
        doRefreshItems(dlg.GetAssetID());
        cp_->updateExtraAssetData(selectedIndex_);
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
    cp_->SetListCtrlColumn(m_selected_col, item);

    m_selected_col = event.GetColumn();

    item.SetImage(m_asc ? 8 : 7);
    SetColumn(m_selected_col, item);

    int trx_id = -1;
    if (selectedIndex_>=0) trx_id = cp_->AssetList().entrylist_[selectedIndex_]->GetId();

    doRefreshItems(trx_id);
}

/*******************************************************/
BEGIN_EVENT_TABLE(mmAssetsPanel, wxPanel)
    EVT_BUTTON(wxID_NEW, mmAssetsPanel::OnNewAsset)
    EVT_BUTTON(wxID_EDIT, mmAssetsPanel::OnEditAsset)
    EVT_BUTTON(wxID_DELETE, mmAssetsPanel::OnDeleteAsset)
    EVT_MENU(wxID_ANY, mmAssetsPanel::OnViewPopupSelected)
END_EVENT_TABLE()
/*******************************************************/

mmAssetsPanel::mmAssetsPanel(wxWindow *parent, mmCoreDB* core)
: mmPanelBase(core)
, asset_list_(core->db_.get(), false) // don't load entries at this point.
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

    m_listCtrlAssets->InitVariables();
    initVirtualListControl();
    if (!asset_list_.entrylist_.empty())
        m_listCtrlAssets->EnsureVisible(static_cast<long>(asset_list_.entrylist_.size()) - 1);

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

    m_listCtrlAssets = new mmAssetsListCtrl( this, itemSplitterWindow10,
        IDC_PANEL_STOCKS_LISTCTRL, wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_VIRTUAL | wxLC_SINGLE_SEL);

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
    itemCol.SetText(_("Value"));
    m_listCtrlAssets->InsertColumn(COL_VALUE, itemCol);

    itemCol.SetAlign(wxLIST_FORMAT_RIGHT);
    itemCol.SetText(_("Date"));
    m_listCtrlAssets->InsertColumn(COL_DATE, itemCol);

    itemCol.SetAlign(wxLIST_FORMAT_LEFT);
    itemCol.SetText(_("Notes"));
    m_listCtrlAssets->InsertColumn(COL_NOTES, itemCol);

    /* See if we can get data from inidb */
    int col0 = core_->iniSettings_->GetIntSetting("ASSETS_COL0_WIDTH", 150);
    int col1 = core_->iniSettings_->GetIntSetting("ASSETS_COL1_WIDTH", -2);
    int col2 = core_->iniSettings_->GetIntSetting("ASSETS_COL2_WIDTH", -2);
    int col3 = core_->iniSettings_->GetIntSetting("ASSETS_COL3_WIDTH", -2);
    int col4 = core_->iniSettings_->GetIntSetting("ASSETS_COL4_WIDTH", 450);

    m_listCtrlAssets->SetColumnWidth(COL_NAME, col0);
    m_listCtrlAssets->SetColumnWidth(COL_DATE, col1);
    m_listCtrlAssets->SetColumnWidth(COL_TYPE, col2);
    m_listCtrlAssets->SetColumnWidth(COL_VALUE, col3);
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

int mmAssetsPanel::initVirtualListControl(int id, int col, bool asc)
{
    /* Clear all the records */
    m_listCtrlAssets->DeleteAllItems();

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(asc ? 8 : 7);
    m_listCtrlAssets->SetColumn(col, item);

    core_->currencyList_.LoadBaseCurrencySettings();

    const wxString sql = wxString::FromUTF8(SELECT_ALL_FROM_ASSETS_V1)
        + wxString::Format(" where ASSETTYPE in ( %s ) ", filter_)
        + " order by " + (wxString() << col + 1)
        + (!asc ? " desc" : " ");
    asset_list_.LoadEntriesUsing(sql);

    m_listCtrlAssets->SetItemCount(asset_list_.entrylist_.size());
    header_text_->SetLabel(wxString::Format(_("Total: %s")
        , asset_list_.GetAssetBalanceCurrencyEditFormat()));

    int selected_item = -1;
    for (int i = 0; i < asset_list_.CurrentListSize(); ++i)
    {
        if (id == asset_list_.GetIndexedEntryPtr(i)->GetId())
        {
            selected_item = i;
            break;
        }
    }

    return selected_item;
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
    wxString item_data = wxGetEmptyString();
    if (column == COL_NAME)  item_data = asset_list_.entrylist_[item]->name_;
    else if (column == COL_TYPE)  item_data = wxGetTranslation(asset_list_.entrylist_[item]->type_);
    else if (column == COL_VALUE) item_data = wxString() << asset_list_.entrylist_[item]->GetValueCurrencyEditFormat();
    else if (column == COL_DATE)  item_data = asset_list_.entrylist_[item]->DisplayDate();
    else if (column == COL_NOTES) item_data = asset_list_.entrylist_[item]->notes_;

    return item_data;
}

void mmAssetsPanel::SetFilter(const wxString& filter)
{
    filter_ = filter;
}

int mmAssetsPanel::GetListCtrlWidth(int id)
{
    return m_listCtrlAssets->GetColumnWidth(id);
}

void mmAssetsPanel::SetListCtrlColumn(int m_selected_col, wxListItem item)
{
    m_listCtrlAssets->SetColumn(m_selected_col, item);
}

void mmAssetsPanel::updateExtraAssetData(int selIndex)
{
    wxStaticText* st = (wxStaticText*)FindWindow(IDC_PANEL_ASSET_STATIC_DETAILS);
    wxStaticText* stm = (wxStaticText*)FindWindow(IDC_PANEL_ASSET_STATIC_DETAILS_MINI);
    if (selIndex > -1)
    {
        enableEditDeleteButtons(true);
        wxString miniInfo;

        miniInfo << "\t" << _("Change in Value") << ": "
        << wxGetTranslation(asset_list_.entrylist_[selIndex]->rate_type_);

        if (asset_list_.entrylist_[selIndex]->rate_type_ != ASSET_RATE_DEF[TAssetEntry::RATE_NONE])
            miniInfo<< " = " << asset_list_.entrylist_[selIndex]->rate_value_ << "%";

        st->SetLabel(asset_list_.entrylist_[selIndex]->notes_);
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
    wxMenu* menu = new wxMenu;
    menu->Append(new wxMenuItem(menu, 0, wxGetTranslation(wxTRANSLATE("All"))));

    size_t size = sizeof(ASSET_TYPE_DEF)/sizeof(wxString);
    for(size_t i = 0; i < size; ++i)
    {
        wxMenuItem* menuItem = new wxMenuItem(menu, i+1, wxGetTranslation(ASSET_TYPE_DEF[i]));
        menu->Append(menuItem);
    }
    PopupMenu(menu);
    delete menu;

    event.Skip();
}

void mmAssetsPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    int evt =  event.GetId();

    filter_ = "";
    wxString label;

    int size = sizeof(ASSET_TYPE_DEF)/sizeof(wxString);
    for(int i = 0; i < size; ++i)
    {
        if (evt == 0 || evt == i+1)
        {
            filter_ << "'" << ASSET_TYPE_DEF[i] << "'" << ",";
            if (evt == i+1) label = ASSET_TYPE_DEF[i];
        }
    }
    filter_.RemoveLast(1);

    if (evt == 0)
        itemStaticTextMainFilter_->SetLabel(_("All"));
    else
        itemStaticTextMainFilter_->SetLabel(wxGetTranslation(label));

    int trx_id = -1;
    m_listCtrlAssets->doRefreshItems(trx_id);
    updateExtraAssetData(trx_id);
}
