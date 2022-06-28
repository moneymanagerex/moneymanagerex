/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2014 - 2021 Nikolay Akimov
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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
    EVT_BUTTON(wxID_FILE,      mmCheckingPanel::OnOpenAttachment)
    EVT_BUTTON(ID_TRX_FILTER,  mmCheckingPanel::OnMouseLeftDown)
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_FIND, mmCheckingPanel::OnSearchTxtEntered)
    EVT_MENU_RANGE(wxID_HIGHEST + MENU_VIEW_ALLTRANSACTIONS, wxID_HIGHEST + MENU_VIEW_ALLTRANSACTIONS + menu_labels().size()
        , mmCheckingPanel::OnViewPopupSelected)
wxEND_EVENT_TABLE()
//----------------------------------------------------------------------------

mmCheckingPanel::mmCheckingPanel(wxWindow *parent, mmGUIFrame *frame, int accountID, int id) 
    : m_filteredBalance(0.0)
    , m_listCtrlAccount()
    , m_AccountID(accountID)
    , isAllAccounts_((-1 == accountID) ? true : false)
    , m_trans_filter_dlg(nullptr)
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
    if (isAllAccounts_) {
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
        wxString json = Model_Infotable::instance().GetStringInfo(wxString::Format("CHECK_FILTER_ID_%d", m_AccountID), def_view);
        m_trans_filter_dlg = new mmFilterTransactionsDialog(this, m_AccountID, false, json);
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

    m_account_balance = !isAllAccounts_ && m_account ? m_account->INITIALBAL : 0.0;
    m_reconciled_balance = m_account_balance;
    m_filteredBalance = 0.0;

    std::map<int, int> custom_field_type;
    const wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    Model_CustomField::Data_Set custom_fields = Model_CustomField::instance().find(Model_CustomField::DB_Table_CUSTOMFIELD_V1::REFTYPE(RefType));
    for (const auto& entry : custom_fields)
    {
        if (entry.REFTYPE != RefType) continue;
        custom_field_type[entry.FIELDID] = Model_CustomField::all_type().Index(entry.TYPE);
    }

    auto custom_fields_data = Model_CustomFieldData::instance().get_all(Model_Attachment::TRANSACTION);
    const auto matrix = Model_CustomField::getMatrix(Model_Attachment::TRANSACTION);
    int udfc01_ref_id = matrix.at("UDFC01");
    int udfc02_ref_id = matrix.at("UDFC02");
    int udfc03_ref_id = matrix.at("UDFC03");
    int udfc04_ref_id = matrix.at("UDFC04");
    int udfc05_ref_id = matrix.at("UDFC05");

    bool ignore_future = Option::instance().getIgnoreFutureTransactions();
    const wxString today_date_string = wxDate::Today().FormatISODate();

    const auto splits = Model_Splittransaction::instance().get_all();
    const auto attachments = Model_Attachment::instance().get_all(Model_Attachment::TRANSACTION);

    const auto i = isAllAccounts_ ? Model_Checking::instance().all() : Model_Account::transaction(this->m_account);
    for (const auto& tran : i)
    {
        double transaction_amount = Model_Checking::amount(tran, m_AccountID);
        if (Model_Checking::status(tran.STATUS) != Model_Checking::VOID_)
            m_account_balance += transaction_amount;

        if (Model_Checking::status(tran.STATUS) == Model_Checking::RECONCILED)
            m_reconciled_balance += transaction_amount;

        if (ignore_future) {
            if (tran.TRANSDATE > today_date_string) continue;
        }

        if (m_transFilterActive)
        {
            if (!m_trans_filter_dlg->mmIsRecordMatches(tran, splits))
                continue;
        }
        else
        {
            if (m_currentView != MENU_VIEW_ALLTRANSACTIONS)
            {
                if (tran.TRANSDATE < m_begin_date) continue;
                if (tran.TRANSDATE > m_end_date) continue;
            }
        }

        Model_Checking::Full_Data full_tran(tran, splits);
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

        if (Model_Checking::status(tran.STATUS) != Model_Checking::VOID_)
            m_filteredBalance += transaction_amount;

        if (custom_fields_data.find(tran.TRANSID) != custom_fields_data.end()) {
            const auto& udfcs = custom_fields_data.at(tran.TRANSID);
            for (const auto& udfc : udfcs)
            {
                if (udfc.FIELDID == udfc01_ref_id) {
                    full_tran.UDFC01 = udfc.CONTENT;
                    full_tran.UDFC01_Type = custom_field_type.find(udfc.FIELDID) != custom_field_type.end() ? custom_field_type.at(udfc.FIELDID) : -1;
                }
                else if (udfc.FIELDID == udfc02_ref_id) {
                    full_tran.UDFC02 = udfc.CONTENT;
                    full_tran.UDFC02_Type = custom_field_type.find(udfc.FIELDID) != custom_field_type.end() ? custom_field_type.at(udfc.FIELDID) : -1;
                }
                else if (udfc.FIELDID == udfc03_ref_id) {
                    full_tran.UDFC03 = udfc.CONTENT;
                    full_tran.UDFC03_Type = custom_field_type.find(udfc.FIELDID) != custom_field_type.end() ? custom_field_type.at(udfc.FIELDID) : -1;
                }
                else if (udfc.FIELDID == udfc04_ref_id) {
                    full_tran.UDFC04 = udfc.CONTENT;
                    full_tran.UDFC04_Type = custom_field_type.find(udfc.FIELDID) != custom_field_type.end() ? custom_field_type.at(udfc.FIELDID) : -1;
                }
                else if (udfc.FIELDID == udfc05_ref_id) {
                    full_tran.UDFC05 = udfc.CONTENT;
                    full_tran.UDFC05_Type = custom_field_type.find(udfc.FIELDID) != custom_field_type.end() ? custom_field_type.at(udfc.FIELDID) : -1;
                }
            }
        }

        m_listCtrlAccount->m_trans.push_back(full_tran);
    }
}

void mmCheckingPanel::OnButtonRightDown(wxMouseEvent& event)
{
    wxCommandEvent evt(wxID_ANY, wxID_HIGHEST + MENU_VIEW_FILTER_DIALOG);
    OnViewPopupSelected(evt);
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

    m_header_text = new wxStaticText( this, wxID_STATIC, "");
    m_header_text->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizerVHeader->Add(m_header_text, g_flagsExpandBorder1);

    m_bitmapTransFilter = new wxButton(this, ID_TRX_FILTER);
    m_bitmapTransFilter->SetBitmap(mmBitmap(png::TRANSFILTER, mmBitmapButtonSize));
    itemBoxSizerVHeader->Add(m_bitmapTransFilter, g_flagsBorder1H);

    m_header_balance = new wxStaticText(this, wxID_STATIC, "");
    itemBoxSizerVHeader->Add(m_header_balance, g_flagsBorder1V);

    m_bitmapTransFilter->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(mmCheckingPanel::OnButtonRightDown), NULL, this);

    /* ---------------------- */

    wxSplitterWindow* itemSplitterWindow10 = new wxSplitterWindow(this
        , wxID_ANY, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER);

    m_imageList.reset(createImageList());
    m_imageList->Add(mmBitmap(png::UNRECONCILED));
    m_imageList->Add(mmBitmap(png::RECONCILED));
    m_imageList->Add(mmBitmap(png::VOID_STAT));
    m_imageList->Add(mmBitmap(png::FOLLOW_UP));
    m_imageList->Add(mmBitmap(png::DUPLICATE_STAT));
    m_imageList->Add(mmBitmap(png::UPARROW));
    m_imageList->Add(mmBitmap(png::DOWNARROW));

    m_listCtrlAccount = new TransactionListCtrl(this, itemSplitterWindow10);

    m_listCtrlAccount->SetImageList(m_imageList.get(), wxIMAGE_LIST_SMALL);
    m_listCtrlAccount->setSortOrder(m_listCtrlAccount->g_asc);
    m_listCtrlAccount->setSortColumn(m_listCtrlAccount->g_sortcol);

    m_listCtrlAccount->createColumns(*m_listCtrlAccount);

    // load the global variables
    m_sortSaveTitle = isAllAccounts_ ? "ALLTRANS" : "CHECK";

    long val = m_listCtrlAccount->COL_DEF_SORT;
    wxString strVal = Model_Setting::instance().GetStringSetting(wxString::Format("%s_SORT_COL", m_sortSaveTitle), wxString() << val);
    if (strVal.ToLong(&val)) m_listCtrlAccount->g_sortcol = m_listCtrlAccount->toEColumn(val);
    // --
    val = 1; // asc sorting default
    strVal = Model_Setting::instance().GetStringSetting(wxString::Format("%s_ASC", m_sortSaveTitle), wxString() << val);
    if (strVal.ToLong(&val)) m_listCtrlAccount->g_asc = val != 0;

    // --
    m_listCtrlAccount->setSortColumn(m_listCtrlAccount->g_sortcol);
    m_listCtrlAccount->setSortOrder(m_listCtrlAccount->g_asc);
    m_listCtrlAccount->setColumnImage(m_listCtrlAccount->getSortColumn()
        , m_listCtrlAccount->getSortOrder() ? ICON_ASC : ICON_DESC); // asc\desc sort mark (arrow)

    wxPanel *itemPanel12 = new wxPanel(itemSplitterWindow10, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL);
    mmThemeMetaColour(itemPanel12, meta::COLOR_LISTPANEL);

    itemSplitterWindow10->SplitHorizontally(m_listCtrlAccount, itemPanel12);
    itemSplitterWindow10->SetMinimumPaneSize(100);
    itemSplitterWindow10->SetSashGravity(1.0);

    itemBoxSizer9->Add(itemSplitterWindow10, g_flagsExpandBorder1);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemPanel12->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemButtonsSizer, g_flagsBorder1V);

    m_btnNew = new wxButton(itemPanel12, wxID_NEW, _("&New "));
    mmToolTip(m_btnNew, _("New Transaction"));
    itemButtonsSizer->Add(m_btnNew, 0, wxRIGHT, 5);

    m_btnEdit = new wxButton(itemPanel12, wxID_EDIT, _("&Edit "));
    mmToolTip(m_btnEdit, _("Edit selected transaction"));
    itemButtonsSizer->Add(m_btnEdit, 0, wxRIGHT, 5);
    m_btnEdit->Enable(false);

    m_btnDelete = new wxButton(itemPanel12, wxID_REMOVE, _("&Delete "));
    mmToolTip(m_btnDelete, _("Delete selected transaction"));
    itemButtonsSizer->Add(m_btnDelete, 0, wxRIGHT, 5);
    m_btnDelete->Enable(false);

    m_btnDuplicate = new wxButton(itemPanel12, wxID_DUPLICATE, _("D&uplicate "));
    mmToolTip(m_btnDuplicate, _("Duplicate selected transaction"));
    itemButtonsSizer->Add(m_btnDuplicate, 0, wxRIGHT, 5);
    m_btnDuplicate->Enable(false);

    m_btnAttachment = new wxBitmapButton(itemPanel12, wxID_FILE
        , mmBitmap(png::CLIP, mmBitmapButtonSize), wxDefaultPosition
        , wxSize(30, m_btnDuplicate->GetSize().GetY()));
    mmToolTip(m_btnAttachment, _("Open attachments"));
    itemButtonsSizer->Add(m_btnAttachment, 0, wxRIGHT, 5);
    m_btnAttachment->Enable(false);

    wxSearchCtrl* searchCtrl = new wxSearchCtrl(itemPanel12
        , wxID_FIND, wxEmptyString, wxDefaultPosition
        , wxSize(100, m_btnDuplicate->GetSize().GetHeight())
        , wxTE_NOHIDESEL, wxDefaultValidator);
    searchCtrl->SetDescriptiveText(_("Search"));
    itemButtonsSizer->Add(searchCtrl, 0, wxCENTER, 1);
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
        return wxString::Format(_("Full Transactions Report"));
    else
        return wxString::Format(_("Account View : %s"), account.ACCOUNTNAME);
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

    if (!isAllAccounts_)
    {
        bool show_displayed_balance_ = (m_transFilterActive || m_currentView != MENU_VIEW_ALLTRANSACTIONS);
        const wxString summaryLine = wxString::Format("%s%s     %s%s     %s%s     %s%s"
            , _("Account Bal: ")
            , Model_Account::toCurrency(m_account_balance, account)
            , _("Reconciled Bal: ")
            , Model_Account::toCurrency(m_reconciled_balance, account)
            , _("Diff: ")
            , Model_Account::toCurrency(m_account_balance - m_reconciled_balance, account)
            , show_displayed_balance_ ? _("Filtered View Bal: ") : ""
            , show_displayed_balance_ ? Model_Account::toCurrency(m_filteredBalance, account) : "");
        m_header_balance->SetLabelText(summaryLine);
    }
    this->Layout();
}

//----------------------------------------------------------------------------
void mmCheckingPanel::enableTransactionButtons(bool editDelete, bool duplicate, bool attach)
{
 
    m_btnEdit->Enable(editDelete);
    m_btnDelete->Enable(editDelete);

    m_btnDuplicate->Enable(duplicate);

    m_btnAttachment->Enable(attach);

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
            for (const auto& i : s)
            {
                const Model_Checking::Data* trx = Model_Checking::instance().get(i);
                balance += Model_Checking::balance(trx, m_AccountID);
                if (minDate > trx->TRANSDATE || maxDate.empty()) minDate = trx->TRANSDATE;
                if (maxDate < trx->TRANSDATE || maxDate.empty()) maxDate = trx->TRANSDATE;
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
        m_info_panel->SetLabelText(wxGetTranslation(TIPS[rand() % (sizeof(TIPS) / sizeof(wxString))]));
    else
        m_info_panel->SetLabelText("");
}
//----------------------------------------------------------------------------

void mmCheckingPanel::OnDeleteTransaction(wxCommandEvent& event)
{
    m_listCtrlAccount->OnDeleteTransaction(event);
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
        if (Model_Account::BoolOf(m_account->STATEMENTLOCKED))
        {
            date_range = new mmSpecifiedRange(
                Model_Account::DateOf(m_account->STATEMENTDATE).Add(wxDateSpan::Day())
                , wxDateTime::Today()
            );

            if (!Option::instance().getIgnoreFutureTransactions())
                date_range->set_end_date(date_range->future_date());
        }
        break;
    case MENU_VIEW_FILTER_DIALOG:
        m_transFilterActive = true;
        break;
    }

    if (m_begin_date.empty()) {
        m_begin_date = date_range->start_date().FormatISODate();
    }

    if (m_end_date.empty()) {
        m_end_date = date_range->end_date().FormatISODate();
    }

    auto item = m_transFilterActive ? menu_labels()[MENU_VIEW_FILTER_DIALOG] : menu_labels()[m_currentView];
    m_bitmapTransFilter->SetLabel(wxGetTranslation(item));
    m_bitmapTransFilter->SetBitmap(m_transFilterActive ? mmBitmap(png::TRANSFILTER_ACTIVE, mmBitmapButtonSize) : mmBitmap(png::TRANSFILTER, mmBitmapButtonSize));

    //Text field for name of day of the week
    wxSize buttonSize(wxDefaultSize);
    buttonSize.IncTo(GetTextExtent(wxGetTranslation(item)));
    m_bitmapTransFilter->SetMinSize(wxSize(buttonSize.GetWidth() + Option::instance().getIconSize() * 2, -1));

    const wxString& def_view = wxString::Format(R"({ "FILTER": "%s" })"
        , Model_Setting::instance().ViewTransactions());
    wxString json = Model_Infotable::instance().GetStringInfo(wxString::Format("CHECK_FILTER_ID_%d", m_AccountID), def_view);
    Document j_doc;
    if (j_doc.Parse(json.utf8_str()).HasParseError()) {
        j_doc.Parse("{}");
    }

    if (j_doc.HasMember("FILTER")) {
        Value::MemberIterator v_name = j_doc.FindMember("FILTER");
        j_doc["FILTER"].SetString(item.mb_str(), j_doc.GetAllocator());
    }
    else
    {
        auto& allocator = j_doc.GetAllocator();
        rapidjson::Value value(item.mb_str(), allocator);
        j_doc.AddMember("FILTER", value, allocator);
    }

    json = JSON_PrettyFormated(j_doc);
    Model_Infotable::instance().Set(wxString::Format("CHECK_FILTER_ID_%d", m_AccountID), json);
}

void mmCheckingPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    m_currentView = event.GetId() - wxID_HIGHEST;
    m_transFilterActive = false;

    if (m_currentView == MENU_VIEW_FILTER_DIALOG)
    {
        if (!m_trans_filter_dlg) {
            const wxString& def_view = wxString::Format(R"({ "FILTER": "%s" })", Model_Setting::instance().ViewTransactions());
            wxString json = Model_Infotable::instance().GetStringInfo(wxString::Format("CHECK_FILTER_ID_%d", m_AccountID), def_view);
            m_trans_filter_dlg.reset(new mmFilterTransactionsDialog(this, m_AccountID, false, json));
        }

        const auto json_settings = m_trans_filter_dlg->mmGetJsonSetings();
        m_transFilterActive = (m_trans_filter_dlg->ShowModal() == wxID_OK
            && m_trans_filter_dlg->mmIsSomethingChecked());
        if (!m_transFilterActive)
            m_currentView = MENU_VIEW_ALLTRANSACTIONS;
    }

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
    int transType = Model_Checking::type(tran->TRANSCODE);

    Model_Checking::Data *transaction = Model_Checking::instance().get(transID);
    auto splits = Model_Checking::splittransaction(transaction);

    if (splits.empty()) return;

    std::vector<Split> splt;
    for (const auto& entry : splits) {
        Split s;
        s.CATEGID = entry.CATEGID;
        s.SUBCATEGID = entry.SUBCATEGID;
        s.SPLITTRANSAMOUNT = entry.SPLITTRANSAMOUNT;
        splt.push_back(s);
    }
    mmSplitTransactionDialog splitTransDialog(this
        , splt, m_AccountID, transType, 0.0, true);

    //splitTransDialog.SetDisplaySplitCategories();
    splitTransDialog.ShowModal();
}

void mmCheckingPanel::RefreshList()
{
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
        wxString json = Model_Infotable::instance().GetStringInfo(wxString::Format("CHECK_FILTER_ID_%d", m_AccountID), def_view);
        m_trans_filter_dlg.reset(new mmFilterTransactionsDialog(this, m_AccountID, false, json));
        m_bitmapTransFilter->SetToolTip(m_trans_filter_dlg->mmGetDescriptionToolTip());
    }

    RefreshList();
    showTips();

    enableTransactionButtons(false, false, false);
}

void mmCheckingPanel::mmPlayTransactionSound()
{
    bool play = Model_Setting::instance().GetBoolSetting(INIDB_USE_TRANSACTION_SOUND, true);
    if (play)
    {
        wxString wav_path = mmex::getPathResource(mmex::TRANS_SOUND);
        wxLogDebug("%s", wav_path);
        wxSound registerSound(wav_path);

        if (registerSound.IsOk())
            registerSound.Play(wxSOUND_ASYNC);
    }
}
