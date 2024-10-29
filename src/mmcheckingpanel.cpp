/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2014 - 2021 Nikolay Akimov
 Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)

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

#include "filtertransdialog.h"
#include "mmcheckingpanel.h"
#include "mmchecking_list.h"
#include "paths.h"
#include "constants.h"
#include "images_list.h"
#include "util.h"
#include "mmex.h"
#include "mmframe.h"
#include "mmTips.h"
#include "mmSimpleDialogs.h"
#include "splittransactionsdialog.h"
#include "transdialog.h"
#include "validators.h"
#include "transactionsupdatedialog.h"
#include "attachmentdialog.h"
#include "model/allmodel.h"
#include "sharetransactiondialog.h"
#include "assetdialog.h"
#include "billsdepositsdialog.h"
#include <wx/clipbrd.h>
#include <float.h>

//----------------------------------------------------------------------------

#include <wx/srchctrl.h>
#include <algorithm>
#include <wx/sound.h>
//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(mmCheckingPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,       mmCheckingPanel::OnNewTransaction)
    EVT_BUTTON(wxID_EDIT,      mmCheckingPanel::OnEditTransaction)
    EVT_BUTTON(wxID_REMOVE,    mmCheckingPanel::OnDeleteTransaction)
    EVT_BUTTON(wxID_DUPLICATE, mmCheckingPanel::OnDuplicateTransaction)
    EVT_BUTTON(wxID_UNDELETE,  mmCheckingPanel::OnRestoreTransaction)
    EVT_BUTTON(wxID_FILE,      mmCheckingPanel::OnOpenAttachment)
    EVT_BUTTON(ID_TRX_FILTER,  mmCheckingPanel::OnMouseLeftDown)
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_FIND, mmCheckingPanel::OnSearchTxtEntered)
    EVT_MENU_RANGE(wxID_HIGHEST + MENU_VIEW_ALLTRANSACTIONS, wxID_HIGHEST + MENU_VIEW_ALLTRANSACTIONS + menu_labels().size()
        , mmCheckingPanel::OnViewPopupSelected)
    EVT_MENU_RANGE(Model_Checking::TYPE_ID_WITHDRAWAL, Model_Checking::TYPE_ID_TRANSFER, mmCheckingPanel::OnNewTransaction)
wxEND_EVENT_TABLE()
//----------------------------------------------------------------------------

mmCheckingPanel::mmCheckingPanel(wxWindow *parent, mmGUIFrame *frame, int accountID, int id)
    : m_AccountID(accountID)
    , isAllAccounts_((-1 == accountID) ? true : false)
    , isTrash_((-2 == accountID) ? true : false)
    , m_frame(frame)
{
    Create(parent, id);
    Fit();
}
//----------------------------------------------------------------------------

/*
    We cannon use OnClose() event because this class deletes
    via DestroyChildren() of its parent.
*/
mmCheckingPanel::~mmCheckingPanel()
{
}

bool mmCheckingPanel::Create(
    wxWindow* parent,
    wxWindowID winid, const wxPoint& pos,
    const wxSize& size, long style, const wxString& name
)
{
    if (isAllAccounts_ || isTrash_) {
        m_currency = Model_Currency::GetBaseCurrency();
    }
    else {
        m_account = Model_Account::instance().get(m_AccountID);
        m_currency = Model_Account::currency(m_account);
    }

    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    if (!wxPanel::Create(parent, winid, pos, size, style, name)) return false;

    this->windowsFreezeThaw();
    CreateControls();
    initViewTransactionsHeader();

    initFilterSettings();
    if (m_transFilterActive) {
        const wxString& def_view = wxString::Format(R"({ "FILTER": "%s" })", Model_Setting::instance().ViewTransactions());
        wxString json = Model_Infotable::instance().GetStringInfo(wxString::Format("CHECK_FILTER_ID_ADV_%d", m_AccountID), def_view);
        m_trans_filter_dlg = new mmFilterTransactionsDialog(parent, m_AccountID, false, json);
        m_bitmapTransFilter->SetToolTip(m_trans_filter_dlg->mmGetDescriptionToolTip());
    }

    RefreshList();
    this->windowsFreezeThaw();

    Model_Usage::instance().pageview(this);
    return true;
}

void mmCheckingPanel::sortTable()
{
    m_listCtrlAccount->sortTable();
}

void mmCheckingPanel::filterTable()
{
    m_listCtrlAccount->m_trans.clear();

    m_account_balance = !isAllAccounts_ && !isTrash_ && m_account ? m_account->INITIALBAL : 0.0;
    m_reconciled_balance = m_account_balance;
    m_filteredBalance = 0.0;
    
    const wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    const wxString splitRefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTIONSPLIT);
    Model_CustomField::TYPE_ID UDFC01_Type = Model_CustomField::getUDFCType(RefType, "UDFC01");
    Model_CustomField::TYPE_ID UDFC02_Type = Model_CustomField::getUDFCType(RefType, "UDFC02");
    Model_CustomField::TYPE_ID UDFC03_Type = Model_CustomField::getUDFCType(RefType, "UDFC03");
    Model_CustomField::TYPE_ID UDFC04_Type = Model_CustomField::getUDFCType(RefType, "UDFC04");
    Model_CustomField::TYPE_ID UDFC05_Type = Model_CustomField::getUDFCType(RefType, "UDFC05");
    int UDFC01_Scale = Model_CustomField::getDigitScale(Model_CustomField::getUDFCProperties(RefType, "UDFC01"));
    int UDFC02_Scale = Model_CustomField::getDigitScale(Model_CustomField::getUDFCProperties(RefType, "UDFC02"));
    int UDFC03_Scale = Model_CustomField::getDigitScale(Model_CustomField::getUDFCProperties(RefType, "UDFC03"));
    int UDFC04_Scale = Model_CustomField::getDigitScale(Model_CustomField::getUDFCProperties(RefType, "UDFC04"));
    int UDFC05_Scale = Model_CustomField::getDigitScale(Model_CustomField::getUDFCProperties(RefType, "UDFC05"));

    auto custom_fields_data = Model_CustomFieldData::instance().get_all(Model_Attachment::TRANSACTION);
    const auto matrix = Model_CustomField::getMatrix(Model_Attachment::TRANSACTION);
    int udfc01_ref_id = matrix.at("UDFC01");
    int udfc02_ref_id = matrix.at("UDFC02");
    int udfc03_ref_id = matrix.at("UDFC03");
    int udfc04_ref_id = matrix.at("UDFC04");
    int udfc05_ref_id = matrix.at("UDFC05");

    bool ignore_future = Option::instance().getIgnoreFutureTransactions();
    const wxString today_date_string = Option::instance().UseTransDateTime() ? wxDateTime::Now().FormatISOCombined() : wxDateTime(23, 59, 59, 999).FormatISOCombined();

    const auto splits = Model_Splittransaction::instance().get_all();
    const auto tags = Model_Taglink::instance().get_all(RefType);
    const auto attachments = Model_Attachment::instance().get_all(Model_Attachment::TRANSACTION);

    const auto i = (isAllAccounts_ || isTrash_) ? Model_Checking::instance().all() : Model_Account::transaction(this->m_account);

    for (const auto& tran : i)
    {
        wxString strDate = Model_Checking::TRANSDATE(tran).FormatISOCombined();

        if (ignore_future && strDate > today_date_string)
            continue;

        double transaction_amount = Model_Checking::amount(tran, m_AccountID);
        if (tran.DELETEDTIME.IsEmpty())
        {
            if (Model_Checking::status_id(tran.STATUS) != Model_Checking::STATUS_ID_VOID)
                m_account_balance += transaction_amount;
            if (Model_Checking::status_id(tran.STATUS) == Model_Checking::STATUS_ID_RECONCILED)
                m_reconciled_balance += transaction_amount;
        }

        Model_Checking::Full_Data full_tran(tran, splits, tags);
        bool expandSplits = false;
        if (m_transFilterActive)
        { 
            int txnMatch = m_trans_filter_dlg->mmIsRecordMatches(tran, splits);
            if (!txnMatch)
                continue;
            else expandSplits = txnMatch < static_cast<int>(full_tran.m_splits.size()) + 1;
        }
        else
        {
            if (m_currentView != MENU_VIEW_ALLTRANSACTIONS)
            {
                if (strDate < m_begin_date)
                    continue;
                if (strDate > m_end_date)
                    continue;
            }
        }

        full_tran.PAYEENAME = full_tran.real_payee_name(m_AccountID);
        full_tran.BALANCE = m_account_balance;
        full_tran.AMOUNT = transaction_amount;
        if (attachments.find(tran.TRANSID) != attachments.end())
        {
            for (const auto& entry : attachments.at(tran.TRANSID))
            {
                full_tran.ATTACHMENT_DESCRIPTION.Add(entry.DESCRIPTION);
            }
        }

        full_tran.UDFC01_Type = Model_CustomField::TYPE_ID_UNKNOWN;
        full_tran.UDFC02_Type = Model_CustomField::TYPE_ID_UNKNOWN;
        full_tran.UDFC03_Type = Model_CustomField::TYPE_ID_UNKNOWN;
        full_tran.UDFC04_Type = Model_CustomField::TYPE_ID_UNKNOWN;
        full_tran.UDFC05_Type = Model_CustomField::TYPE_ID_UNKNOWN;
        full_tran.UDFC01_val = -DBL_MAX;
        full_tran.UDFC02_val = -DBL_MAX;
        full_tran.UDFC03_val = -DBL_MAX;
        full_tran.UDFC04_val = -DBL_MAX;
        full_tran.UDFC05_val = -DBL_MAX;
        if (custom_fields_data.find(tran.TRANSID) != custom_fields_data.end()) {
            const auto& udfcs = custom_fields_data.at(tran.TRANSID);
            for (const auto& udfc : udfcs)
            {
                if (udfc.FIELDID == udfc01_ref_id) {
                    full_tran.UDFC01 = udfc.CONTENT;
                    full_tran.UDFC01_val = cleanseNumberStringToDouble(udfc.CONTENT, UDFC01_Scale);
                    full_tran.UDFC01_Type = UDFC01_Type;
                }
                else if (udfc.FIELDID == udfc02_ref_id) {
                    full_tran.UDFC02 = udfc.CONTENT;
                    full_tran.UDFC02_val = cleanseNumberStringToDouble(udfc.CONTENT, UDFC02_Scale);
                    full_tran.UDFC02_Type = UDFC02_Type;
                }
                else if (udfc.FIELDID == udfc03_ref_id) {
                    full_tran.UDFC03 = udfc.CONTENT;
                    full_tran.UDFC03_val = cleanseNumberStringToDouble(udfc.CONTENT, UDFC03_Scale);
                    full_tran.UDFC03_Type = UDFC03_Type;
                }
                else if (udfc.FIELDID == udfc04_ref_id) {
                    full_tran.UDFC04 = udfc.CONTENT;
                    full_tran.UDFC04_val = cleanseNumberStringToDouble(udfc.CONTENT, UDFC04_Scale);
                    full_tran.UDFC04_Type = UDFC04_Type;
                }
                else if (udfc.FIELDID == udfc05_ref_id) {
                    full_tran.UDFC05 = udfc.CONTENT;
                    full_tran.UDFC05_val = cleanseNumberStringToDouble(udfc.CONTENT, UDFC05_Scale);
                    full_tran.UDFC05_Type = UDFC05_Type;
                }
            }
        }
        if ((isTrash_ && !full_tran.DELETEDTIME.IsEmpty()) || !(isTrash_ || !full_tran.DELETEDTIME.IsEmpty()))
        {
            if (!expandSplits) {
                m_listCtrlAccount->m_trans.push_back(full_tran);
                if (Model_Checking::status_id(tran.STATUS) != Model_Checking::STATUS_ID_VOID && tran.DELETEDTIME.IsEmpty())
                    m_filteredBalance += transaction_amount;
            }
            else
            {
                int splitIndex = 1;
                wxString tranTagnames = full_tran.TAGNAMES;
                for (const auto& split : full_tran.m_splits)
                {
                    full_tran.displayID = (wxString::Format("%i", tran.TRANSID) + "." + wxString::Format("%i", splitIndex++));
                    full_tran.CATEGID = split.CATEGID;
                    full_tran.CATEGNAME = Model_Category::full_name(split.CATEGID);
                    full_tran.TRANSAMOUNT = split.SPLITTRANSAMOUNT;
                    full_tran.NOTES = tran.NOTES;
                    full_tran.TAGNAMES = tranTagnames;
                    Model_Checking::Data splitWithTxnNotes = full_tran;
                    Model_Checking::Data splitWithSplitNotes = full_tran;
                    splitWithSplitNotes.NOTES = split.NOTES;
                    if (m_trans_filter_dlg->mmIsSplitRecordMatches<Model_Splittransaction>(split)
                        && (m_trans_filter_dlg->mmIsRecordMatches<Model_Checking>(splitWithSplitNotes, true)
                            || m_trans_filter_dlg->mmIsRecordMatches<Model_Checking>(splitWithTxnNotes, true)))
                    {
                        full_tran.AMOUNT = Model_Checking::amount(splitWithTxnNotes, m_AccountID);
                        full_tran.NOTES.Append((tran.NOTES.IsEmpty() ? "" : " ") + split.NOTES);
                        wxString tagnames;
                        for (const auto& tag : Model_Taglink::instance().get(splitRefType,split.SPLITTRANSID))
                            tagnames.Append(tag.first + " ");

                        if(!tagnames.IsEmpty())
                            full_tran.TAGNAMES.Append((full_tran.TAGNAMES.IsEmpty() ? "" : ", ") + tagnames.Trim());
                        m_listCtrlAccount->m_trans.push_back(full_tran);
                        if (Model_Checking::status_id(tran.STATUS) != Model_Checking::STATUS_ID_VOID && tran.DELETEDTIME.IsEmpty())
                            m_filteredBalance += full_tran.AMOUNT;
                    }
                }
            }
        }
    }
}

void mmCheckingPanel::OnButtonRightDown(wxMouseEvent& event)
{
    int id = event.GetId();
    switch (id)
    {
    case ID_TRX_FILTER:
    {
        wxCommandEvent evt(wxID_ANY, wxID_HIGHEST + MENU_VIEW_FILTER_DIALOG);
        OnViewPopupSelected(evt);
        break;
    }
    case wxID_FILE:
    {
        wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
        auto selected_id = m_listCtrlAccount->getSelectedId();
        if (selected_id.size() == 1) {
            auto i = selected_id[0];
            mmAttachmentDialog dlg(this, RefType, i);
            dlg.ShowModal();
            RefreshList();
        }
        break;
    }
    case wxID_NEW:
    {
        wxMenu menu;
        menu.Append(Model_Checking::TYPE_ID_WITHDRAWAL, _("&New Withdrawal..."));
        menu.Append(Model_Checking::TYPE_ID_DEPOSIT, _("&New Deposit..."));
        menu.Append(Model_Checking::TYPE_ID_TRANSFER, _("&New Transfer..."));
        PopupMenu(&menu);
    }
    default:
        break;
    }
}

void mmCheckingPanel::OnMouseLeftDown(wxCommandEvent& event)
{
    wxMenu menu;
    int id = MENU_VIEW_ALLTRANSACTIONS;
    for (const auto& i : menu_labels())
    {
        if (!isAllAccounts_ || (MENU_VIEW_STATEMENTDATE != id))
            menu.Append(wxID_HIGHEST + id, wxGetTranslation(i));
        id++;
    }
    PopupMenu(&menu);
    m_bitmapTransFilter->Layout();
    event.Skip();
}

//----------------------------------------------------------------------------

void mmCheckingPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    /* ---------------------- */

    wxFlexGridSizer* itemBoxSizerVHeader = new wxFlexGridSizer(0, 1, 0, 0);
    itemBoxSizerVHeader->AddGrowableCol(0, 0);
    itemBoxSizer9->Add(itemBoxSizerVHeader, g_flagsBorder1V);

    m_header_text = new wxStaticText(this, wxID_STATIC, "");
    m_header_text->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizerVHeader->Add(m_header_text, g_flagsExpandBorder1);

    wxBoxSizer* infoPanel = new wxBoxSizer(wxHORIZONTAL);
    m_bitmapTransFilter = new wxButton(this, ID_TRX_FILTER);
    m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
    infoPanel->Add(m_bitmapTransFilter, g_flagsH);
    m_header_sortOrder = new wxStaticText(this, wxID_STATIC, "");
    infoPanel->Add(m_header_sortOrder, g_flagsH);
    itemBoxSizerVHeader->Add(infoPanel, g_flagsBorder1H);

    wxBoxSizer* infoPanel2 = new wxBoxSizer(wxHORIZONTAL);
    m_header_balance = new wxStaticText(this, wxID_STATIC, "");
    infoPanel2->Add(m_header_balance, g_flagsH);
    m_header_credit = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(100,-1));
    infoPanel2->Add(m_header_credit, g_flagsH);
    itemBoxSizerVHeader->Add(infoPanel2, g_flagsBorder1V);

    m_bitmapTransFilter->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(mmCheckingPanel::OnButtonRightDown), nullptr, this);

    /* ---------------------- */

    wxSplitterWindow* itemSplitterWindow10 = new wxSplitterWindow(this
        , wxID_ANY, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER);

    m_images.push_back(mmBitmapBundle(png::UNRECONCILED));
    m_images.push_back(mmBitmapBundle(png::RECONCILED));
    m_images.push_back(mmBitmapBundle(png::VOID_STAT));
    m_images.push_back(mmBitmapBundle(png::FOLLOW_UP));
    m_images.push_back(mmBitmapBundle(png::DUPLICATE_STAT));
    m_images.push_back(mmBitmapBundle(png::UPARROW));
    m_images.push_back(mmBitmapBundle(png::DOWNARROW));

    m_listCtrlAccount = new TransactionListCtrl(this, itemSplitterWindow10);

    m_listCtrlAccount->SetSmallImages(m_images);
    m_listCtrlAccount->SetNormalImages(m_images);

    m_listCtrlAccount->setSortOrder(m_listCtrlAccount->g_asc);
    m_listCtrlAccount->setSortColumn(m_listCtrlAccount->g_sortcol);

    // Get sorted columns list and update the sorted columns with missing columns if needed.
    wxArrayString columnList, sortedColumnList;
    sortedColumnList = m_listCtrlAccount->GetColumnsOrder();
     wxLogDebug("CreateControls: getColumnList() = %s", wxJoin(sortedColumnList, '|'));

    // sort m_columns according to sortedColumnsList
    std::vector<PANEL_COLUMN> sortedColumns = {};
    std::vector<int> sortedRealColumns = {};
    for (const auto& i : sortedColumnList)
    {
        for (unsigned int j = 0; j < m_listCtrlAccount->m_columns.size(); j++)
        {
            auto k = m_listCtrlAccount->m_columns[j];
            auto l = m_listCtrlAccount->m_real_columns[j];
            if (wxString::Format("%d", j) == i)
            {
                sortedColumns.push_back(k);
                sortedRealColumns.push_back(l);
                break;
            }
        }
    }

    m_listCtrlAccount->m_columns = sortedColumns;
    m_listCtrlAccount->m_real_columns = sortedRealColumns;

    m_listCtrlAccount->createColumns(*m_listCtrlAccount);

    // load the global variables
    m_sortSaveTitle = isAllAccounts_ ? "ALLTRANS" : (isTrash_ ? "DELETED" : "CHECK");

    long val = m_listCtrlAccount->COL_DEF_SORT;
    wxString strVal = Model_Setting::instance().GetStringSetting(wxString::Format("%s_SORT_COL", m_sortSaveTitle), wxString() << val);
    if (strVal.ToLong(&val)) m_listCtrlAccount->g_sortcol = m_listCtrlAccount->toEColumn(val);
    val = m_listCtrlAccount->COL_DEF_SORT2;
    strVal = Model_Setting::instance().GetStringSetting(wxString::Format("%s_SORT_COL2", m_sortSaveTitle), wxString() << val);
    if (strVal.ToLong(&val)) m_listCtrlAccount->prev_g_sortcol = m_listCtrlAccount->toEColumn(val);

    val = 1; // asc sorting default
    strVal = Model_Setting::instance().GetStringSetting(wxString::Format("%s_ASC", m_sortSaveTitle), wxString() << val);
    if (strVal.ToLong(&val)) m_listCtrlAccount->g_asc = val != 0;
    val = 1;
    strVal = Model_Setting::instance().GetStringSetting(wxString::Format("%s_ASC2", m_sortSaveTitle), wxString() << val);
    if (strVal.ToLong(&val)) m_listCtrlAccount->prev_g_asc = val != 0;

    // --
    m_listCtrlAccount->setSortColumn(m_listCtrlAccount->g_sortcol);
    m_listCtrlAccount->setSortOrder(m_listCtrlAccount->g_asc);
    m_listCtrlAccount->setColumnImage(m_listCtrlAccount->getSortColumn()
        , m_listCtrlAccount->getSortOrder() ? ICON_ASC : ICON_DESC); // asc\desc sort mark (arrow)

    wxPanel* itemPanel12 = new wxPanel(itemSplitterWindow10, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    mmThemeMetaColour(itemPanel12, meta::COLOR_LISTPANEL);

    itemSplitterWindow10->SplitHorizontally(m_listCtrlAccount, itemPanel12);
    itemSplitterWindow10->SetMinimumPaneSize(100);
    itemSplitterWindow10->SetSashGravity(1.0);

    itemBoxSizer9->Add(itemSplitterWindow10, g_flagsExpandBorder1);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemPanel12->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemButtonsSizer, wxSizerFlags(g_flagsExpandBorder1).Proportion(0));

    m_btnDelete = new wxButton(itemPanel12, wxID_REMOVE, _("&Delete "));
    mmToolTip(m_btnDelete, _("Delete all selected transactions"));

    if (!isTrash_) {
        m_btnNew = new wxButton(itemPanel12, wxID_NEW, _("&New "));
        mmToolTip(m_btnNew, _("New Transaction"));
        itemButtonsSizer->Add(m_btnNew, 0, wxRIGHT, 5);

        m_btnEdit = new wxButton(itemPanel12, wxID_EDIT, _("&Edit "));
        mmToolTip(m_btnEdit, _("Edit all selected transactions"));
        itemButtonsSizer->Add(m_btnEdit, 0, wxRIGHT, 5);
        m_btnEdit->Enable(false);

        itemButtonsSizer->Add(m_btnDelete, 0, wxRIGHT, 5);
        m_btnDelete->Enable(false);

        m_btnDuplicate = new wxButton(itemPanel12, wxID_DUPLICATE, _("D&uplicate "));
        mmToolTip(m_btnDuplicate, _("Duplicate selected transaction"));
        itemButtonsSizer->Add(m_btnDuplicate, 0, wxRIGHT, 5);
        m_btnDuplicate->Enable(false);

        const auto& btnDupSize = m_btnDuplicate->GetSize();
        m_btnAttachment = new wxBitmapButton(itemPanel12, wxID_FILE
            , mmBitmapBundle(png::CLIP), wxDefaultPosition
            , wxSize(btnDupSize.GetY(), btnDupSize.GetY()));
        mmToolTip(m_btnAttachment, _("Open attachments"));
        itemButtonsSizer->Add(m_btnAttachment, 0, wxRIGHT, 5);
        m_btnAttachment->Enable(false);

        m_btnAttachment->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(mmCheckingPanel::OnButtonRightDown), nullptr, this);
        m_btnNew->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(mmCheckingPanel::OnButtonRightDown), nullptr, this);
    }
    else
    {
        m_btnRestore = new wxButton(itemPanel12, wxID_UNDELETE, _("&Restore "));
        mmToolTip(m_btnRestore, _("Restore selected transaction"));
        itemButtonsSizer->Add(m_btnRestore, 0, wxRIGHT, 5);
        m_btnRestore->Enable(false);

        itemButtonsSizer->Add(m_btnDelete, 0, wxRIGHT, 5);
        m_btnDelete->Enable(false);
    }

    wxSearchCtrl* searchCtrl = new wxSearchCtrl(itemPanel12
        , wxID_FIND, wxEmptyString, wxDefaultPosition
        , wxSize(100, m_btnDelete->GetSize().GetHeight())
        , wxTE_NOHIDESEL, wxDefaultValidator);
    searchCtrl->SetDescriptiveText(_("Search"));
    itemButtonsSizer->Add(searchCtrl, g_flagsExpandBorder1);
    mmToolTip(searchCtrl,
        _("Enter any string to find it in the nearest transaction data") + "\n\n" +
        _("Tips: You can use wildcard characters - question mark (?), asterisk (*) - in your search criteria.") + "\n" +
        _("Use the question mark (?) to find any single character - for example, s?t finds 'sat' and 'set'.") + "\n" +
        _("Use the asterisk (*) to find any number of characters - for example, s*d finds 'sad' and 'started'.") + "\n" +
        _("Use the asterisk (*) in the begin to find any string in the middle of the sentence.")
    );

    //Infobar-mini
    m_info_panel_mini = new wxStaticText(itemPanel12, wxID_STATIC, "");
    itemButtonsSizer->Add(m_info_panel_mini, 1, wxGROW | wxTOP | wxLEFT, 5);

    //Infobar
    m_info_panel = new wxStaticText(itemPanel12, wxID_STATIC, ""
        , wxDefaultPosition, wxSize(200, -1), wxTE_MULTILINE | wxTE_WORDWRAP);
    itemBoxSizer4->Add(m_info_panel, g_flagsExpandBorder1);
    //Show tips when no any transaction selected
    showTips();
}

wxString mmCheckingPanel::GetPanelTitle(const Model_Account::Data& account) const
{
    if (isAllAccounts_)
        return wxString::Format(_("All Transactions"));
    else if (isTrash_)
        return wxString::Format(_("Deleted Transactions"));
    else
        return wxString::Format(_("Account View: %s"), account.ACCOUNTNAME);
}

wxString mmCheckingPanel::BuildPage() const
{
    Model_Account::Data *account = Model_Account::instance().get(m_AccountID);
    return m_listCtrlAccount->BuildPage((account ? GetPanelTitle(*account) : ""));
}

void mmCheckingPanel::setAccountSummary()
{
    Model_Account::Data *account = Model_Account::instance().get(m_AccountID);
    m_header_text->SetLabelText(GetPanelTitle(*account));
    m_header_credit->Hide();

    if (!isAllAccounts_ && !isTrash_)
    {
        bool show_displayed_balance_ = (m_transFilterActive || m_currentView != MENU_VIEW_ALLTRANSACTIONS);
        wxString summaryLine = wxString::Format("%s%s     %s%s     %s%s     %s%s"
            , _("Account Bal: ")
            , Model_Account::toCurrency(m_account_balance, account)
            , _("Reconciled Bal: ")
            , Model_Account::toCurrency(m_reconciled_balance, account)
            , _("Diff: ")
            , Model_Account::toCurrency(m_account_balance - m_reconciled_balance, account)
            , show_displayed_balance_ ? _("Filtered View Bal: ") : ""
            , show_displayed_balance_ ? Model_Account::toCurrency(m_filteredBalance, account) : "");
        if (account->CREDITLIMIT != 0.0) 
        {
            double limit = 100.0 * ((m_account_balance < 0.0) ? -m_account_balance / account->CREDITLIMIT : 0.0);
            summaryLine.Append(wxString::Format("   %s %.1f%%"
                                , _("Credit Limit:")
                                , limit));
           m_header_credit->SetValue(limit);
           m_header_credit->Show(); 
        }
        m_header_balance->SetLabelText(summaryLine);
    }
    this->Layout();
}

//----------------------------------------------------------------------------
void mmCheckingPanel::enableTransactionButtons(bool editDelete, bool duplicate, bool attach)
{
    m_btnDelete->Enable(editDelete);

    if (!isTrash_) {
        m_btnEdit->Enable(editDelete);
        m_btnDuplicate->Enable(duplicate);
        m_btnAttachment->Enable(attach);
    }
    else {
        m_btnRestore->Enable(editDelete);
    }

}
//----------------------------------------------------------------------------

void mmCheckingPanel::updateExtraTransactionData(bool single, bool foreign)
{
    if (single)
    {
        enableTransactionButtons(true, !foreign, true);

        long x = -1;
        for (x = 0; x < m_listCtrlAccount->GetItemCount(); x++) {
            if (m_listCtrlAccount->GetItemState(x, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED) {
                break;
            }
        }

        Model_Checking::Full_Data full_tran(m_listCtrlAccount->m_trans[x]);
        wxString miniStr = full_tran.info();
        //Show only first line but full string set as tooltip
        if (miniStr.Find("\n") > 1 && !miniStr.IsEmpty())
        {
            m_info_panel_mini->SetLabelText(miniStr.substr(0, miniStr.Find("\n")) + " ...");
            mmToolTip(m_info_panel_mini, miniStr);
        }
        else
        {
            m_info_panel_mini->SetLabelText(miniStr);
            mmToolTip(m_info_panel_mini, miniStr);
        }

        wxString notesStr = full_tran.NOTES;
        auto splits = Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(full_tran.TRANSID));
        for (const auto& split : splits)
            if (!split.NOTES.IsEmpty())
            {
                notesStr += notesStr.empty() ? "" : "\n";
                notesStr += split.NOTES;
            }

        if (full_tran.has_attachment()) {
            const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
            Model_Attachment::Data_Set attachments = Model_Attachment::instance().FilterAttachments(RefType, full_tran.id());
            for (const auto& i : attachments) {
                notesStr += notesStr.empty() ? "" : "\n";
                notesStr += _("Attachment") + " " + i.DESCRIPTION + " " + i.FILENAME;
            }
        }
        m_info_panel->SetLabelText(notesStr);
    }
    else
    {
        m_info_panel_mini->SetLabelText("");
        const auto s = m_listCtrlAccount->getSelectedId();
        if (s.size() > 0)
        {
            enableTransactionButtons(true, false, false);

            wxString maxDate;
            wxString minDate;
            double balance = 0;
            long item = -1;
            while(true)
            {
                item = m_listCtrlAccount->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
                if (item == -1) break;
                balance += Model_Checking::balance(m_listCtrlAccount->m_trans[item], m_AccountID);
                wxString transdate = m_listCtrlAccount->m_trans[item].TRANSDATE;
                if (minDate > transdate || maxDate.empty()) minDate = transdate;
                if (maxDate < transdate || maxDate.empty()) maxDate = transdate;
            }

            wxDateTime min_date, max_date;
            min_date.ParseISODate(minDate);
            max_date.ParseISODate(maxDate);
            int days = max_date.Subtract(min_date).GetDays();

            Model_Account::Data *account = Model_Account::instance().get(m_AccountID);
            Model_Currency::Data* currency = nullptr;
            if (account) currency = Model_Currency::instance().get(account->CURRENCYID);
            wxString msg;
            msg = wxString::Format(_("Transactions selected: %zu"), s.size());
            msg += "\n";
            if (currency) {
                msg += wxString::Format(_("Selected transactions balance: %s")
                    , Model_Currency::toCurrency(balance, currency));
                msg += "\n";
            }
            msg += wxString::Format(_("Days between selected transactions: %d"), days);
#ifdef __WXMAC__    // See issue #2914
            msg = "";
#endif
            m_info_panel->SetLabelText(msg);
        }
        else
        {
            enableTransactionButtons(false, false, false);
            showTips();
        }
    }
}
//----------------------------------------------------------------------------
void mmCheckingPanel::showTips()
{
    if (Option::instance().getShowMoneyTips())
        m_info_panel->SetLabelText(
            wxGetTranslation(wxString::FromUTF8(
                TIPS[rand() % (sizeof(TIPS) / sizeof(wxString))]
                .ToStdString())
            )
        );
    else
        m_info_panel->SetLabelText("");
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnDeleteTransaction(wxCommandEvent& event)
{
    m_listCtrlAccount->OnDeleteTransaction(event);
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnRestoreTransaction(wxCommandEvent& event)
{
    m_listCtrlAccount->OnRestoreTransaction(event);
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnNewTransaction(wxCommandEvent& event)
{
    m_listCtrlAccount->OnNewTransaction(event);
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnEditTransaction(wxCommandEvent& event)
{
    m_listCtrlAccount->OnEditTransaction(event);
    m_listCtrlAccount->SetFocus();
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnDuplicateTransaction(wxCommandEvent& event)
{
    m_listCtrlAccount->OnDuplicateTransaction(event);
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnMoveTransaction(wxCommandEvent& event)
{
    m_listCtrlAccount->OnMoveTransaction(event);
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnOpenAttachment(wxCommandEvent& event)
{
    m_listCtrlAccount->OnOpenAttachment(event);
    m_listCtrlAccount->SetFocus();
}

//----------------------------------------------------------------------------

void mmCheckingPanel::initViewTransactionsHeader()
{
    const wxString& def_view = wxString::Format("{ \"FILTER\": \"%s\" }", Model_Setting::instance().ViewTransactions());
    const auto& data = Model_Infotable::instance().GetStringInfo(wxString::Format("CHECK_FILTER_ID_%d", m_AccountID), def_view);
    Document j_doc;
    if (j_doc.Parse(data.utf8_str()).HasParseError()) {
        j_doc.Parse("{}");
    }
    Value& j_filter = GetValueByPointerWithDefault(j_doc, "/FILTER", "");
    wxString s_filter = j_filter.IsString() ? wxString::FromUTF8(j_filter.GetString()) : VIEW_TRANS_ALL_STR;

    m_currentView = menu_labels().Index(s_filter);
    if (m_currentView < 0 || m_currentView >= static_cast<int>(menu_labels().size()))
        m_currentView = menu_labels().Index(VIEW_TRANS_ALL_STR);

}
//----------------------------------------------------------------------------
void mmCheckingPanel::initFilterSettings()
{
    m_transFilterActive = false;
    m_bitmapTransFilter->UnsetToolTip();
    wxSharedPtr<mmDateRange> date_range(new mmAllTime);

    m_begin_date = "";
    m_end_date = "";

    switch (m_currentView) {
    case MENU_VIEW_TODAY:
        date_range = new mmToday; break;
    case MENU_VIEW_CURRENTMONTH:
        date_range = new mmCurrentMonth; break;
    case MENU_VIEW_LAST30:
        date_range = new mmLast30Days; break;
    case MENU_VIEW_LAST90:
        date_range = new mmLast90Days; break;
    case MENU_VIEW_LASTMONTH:
        date_range = new mmLastMonth; break;
    case MENU_VIEW_LAST3MONTHS:
        date_range = new mmLast3Months; break;
    case MENU_VIEW_LAST12MONTHS:
        date_range = new mmLast12Months; break;
    case  MENU_VIEW_CURRENTYEAR:
        date_range = new mmCurrentYear; break;
    case  MENU_VIEW_CURRENTFINANCIALYEAR:
        date_range = new mmCurrentFinancialYear(); break;
    case  MENU_VIEW_LASTYEAR:
        date_range = new mmLastYear; break;
    case  MENU_VIEW_LASTFINANCIALYEAR:
        date_range = new mmLastFinancialYear(); break;
    case  MENU_VIEW_STATEMENTDATE:
        date_range = new mmSpecifiedRange(
            Model_Account::DateOf(m_account->STATEMENTDATE).Add(wxDateSpan::Day())
            , wxDateTime::Today()
        );

        if (!Option::instance().getIgnoreFutureTransactions())
            date_range->set_end_date(date_range->future_date());
        
        break;
    case MENU_VIEW_FILTER_DIALOG:
        m_transFilterActive = true;
        break;
    }

    if (m_begin_date.empty()) {
        m_begin_date = date_range->start_date().FormatISOCombined();
    }

    if (m_end_date.empty()) {
        m_end_date = date_range->end_date().FormatISOCombined();
    }

    auto item = m_transFilterActive ? menu_labels()[MENU_VIEW_FILTER_DIALOG] : menu_labels()[m_currentView];
    m_bitmapTransFilter->SetLabel(wxGetTranslation(item));
    m_bitmapTransFilter->SetBitmap(m_transFilterActive ? mmBitmapBundle(png::TRANSFILTER_ACTIVE, mmBitmapButtonSize) : mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));

    //Text field for name of day of the week
    wxSize buttonSize(wxDefaultSize);
    buttonSize.IncTo(GetTextExtent(wxGetTranslation(item)));
    m_bitmapTransFilter->SetMinSize(wxSize(buttonSize.GetWidth() + Option::instance().getIconSize() * 2, -1));

    const wxString& def_view = wxString::Format(R"({ "FILTER": "%s" })"
        , Model_Setting::instance().ViewTransactions());
    wxString json = Model_Infotable::instance().GetStringInfo(wxString::Format("CHECK_FILTER_ID_%d", m_AccountID), def_view);
    Document j_doc;
    if (j_doc.Parse(json.utf8_str()).HasParseError() || !j_doc.IsArray()) {
        j_doc.Parse("{}");
    }

    if (j_doc.HasMember("FILTER")) {
        j_doc["FILTER"].SetString(item.mb_str(), j_doc.GetAllocator());
    }
    else {
        auto& allocator = j_doc.GetAllocator();
        rapidjson::Value value(item.mb_str(), allocator);
        j_doc.AddMember("FILTER", value, allocator);
    }

    json = JSON_PrettyFormated(j_doc);
    Model_Infotable::instance().Set(wxString::Format("CHECK_FILTER_ID_%d", m_AccountID), json);
}

void mmCheckingPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    int oldView = m_currentView;
    m_currentView = event.GetId() - wxID_HIGHEST;

    if (m_currentView == MENU_VIEW_FILTER_DIALOG)
    {
        if (!m_trans_filter_dlg) {
            const wxString& def_view = wxString::Format(R"({ "FILTER": "%s" })", Model_Setting::instance().ViewTransactions());
            wxString json = Model_Infotable::instance().GetStringInfo(wxString::Format("CHECK_FILTER_ID_ADV_%d", m_AccountID), def_view);
            m_trans_filter_dlg.reset(new mmFilterTransactionsDialog(this, m_AccountID, false, json));
        }

        const auto json_settings = m_trans_filter_dlg->mmGetJsonSetings();
        int status =  m_trans_filter_dlg->ShowModal();
        if (oldView == MENU_VIEW_FILTER_DIALOG)
        {
            if (status != wxID_OK)
                m_trans_filter_dlg.reset(new mmFilterTransactionsDialog(this, m_AccountID, false, json_settings));   
        } else
        {
            m_transFilterActive = (status == wxID_OK && m_trans_filter_dlg->mmIsSomethingChecked());
            if (!m_transFilterActive)
                m_currentView = oldView;
        }
    } else
        m_transFilterActive = false;

    initFilterSettings();
    if (m_transFilterActive)
        m_bitmapTransFilter->SetToolTip(m_trans_filter_dlg->mmGetDescriptionToolTip());

    RefreshList();
}

void mmCheckingPanel::OnSearchTxtEntered(wxCommandEvent& event)
{
    const wxString search_string = event.GetString();
    if (search_string.IsEmpty()) return;

    m_listCtrlAccount->doSearchText(search_string);
}

void mmCheckingPanel::DisplaySplitCategories(int transID)
{
    const Model_Checking::Data* tran = Model_Checking::instance().get(transID);
    int transType = Model_Checking::type_id(tran->TRANSCODE);

    Model_Checking::Data *transaction = Model_Checking::instance().get(transID);
    auto splits = Model_Checking::splittransaction(transaction);

    if (splits.empty()) return;

    std::vector<Split> splt;
    for (const auto& entry : splits) {
        Split s;
        s.CATEGID = entry.CATEGID;
        s.SPLITTRANSAMOUNT = entry.SPLITTRANSAMOUNT;
        s.NOTES = entry.NOTES;
        splt.push_back(s);
    }
    mmSplitTransactionDialog splitTransDialog(this, splt, m_AccountID, transType, 0.0, true);

    //splitTransDialog.SetDisplaySplitCategories();
    splitTransDialog.ShowModal();
}

void mmCheckingPanel::RefreshList()
{
    m_listCtrlAccount->refreshVisualList();
}

void mmCheckingPanel::ResetColumnView()
{
    m_listCtrlAccount->DeleteAllColumns();
    m_listCtrlAccount->resetColumns();
    m_listCtrlAccount->createColumns(*m_listCtrlAccount);
    m_listCtrlAccount->refreshVisualList();
}

void mmCheckingPanel::SetSelectedTransaction(int transID)
{
    m_listCtrlAccount->setSelectedID(transID);
    RefreshList();
    m_listCtrlAccount->SetFocus();
}

// Refresh account screen with new details
void mmCheckingPanel::DisplayAccountDetails(int accountID)
{
    wxASSERT (-1 != accountID); // should not be called for all transaction view

    m_listCtrlAccount->setVisibleItemIndex(-1);
    m_AccountID = accountID;
    m_account = Model_Account::instance().get(m_AccountID);
    m_currency = Model_Account::currency(m_account);

    initViewTransactionsHeader();
    initFilterSettings();

    if (m_transFilterActive)
    {
        const wxString& def_view = wxString::Format("{ \"FILTER\": \"%s\" }", Model_Setting::instance().ViewTransactions());
        wxString json = Model_Infotable::instance().GetStringInfo(wxString::Format("CHECK_FILTER_ID_ADV_%d", m_AccountID), def_view);
        m_trans_filter_dlg.reset(new mmFilterTransactionsDialog(this, m_AccountID, false, json));
        m_bitmapTransFilter->SetToolTip(m_trans_filter_dlg->mmGetDescriptionToolTip());
    }

    RefreshList();
    showTips();

    enableTransactionButtons(false, false, false);
}

void mmCheckingPanel::mmPlayTransactionSound()
{
    int play = Model_Setting::instance().GetIntSetting(INIDB_USE_TRANSACTION_SOUND, 0);
    if (play)
    {
        wxString wav_path;
        switch (play) {
        case 2:
            wav_path = mmex::getPathResource(mmex::TRANS_SOUND2);
            break;
        default:
            wav_path = mmex::getPathResource(mmex::TRANS_SOUND1);
            break;
        }

        wxLogDebug("%s", wav_path);
        wxSound registerSound(wav_path);

        if (registerSound.IsOk())
            registerSound.Play(wxSOUND_ASYNC);
    }
}
