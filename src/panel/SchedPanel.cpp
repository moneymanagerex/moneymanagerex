/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2014 - 2022 Nikolay Akimov
 Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)

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
#include "model/CategoryModel.h"
#include "model/AttachmentModel.h"
#include "model/UsageModel.h"
#include "SchedPanel.h"
#include "dialog/AttachmentDialog.h"
#include "dialog/SchedDialog.h"

wxBEGIN_EVENT_TABLE(SchedPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,       SchedPanel::onNewBDSeries)
    EVT_BUTTON(wxID_EDIT,      SchedPanel::onEditBDSeries)
    EVT_BUTTON(wxID_DUPLICATE, SchedPanel::onDuplicateBDSeries)
    EVT_BUTTON(wxID_DELETE,    SchedPanel::onDeleteBDSeries)
    EVT_BUTTON(wxID_PASTE,     SchedPanel::onEnterBDTransaction)
    EVT_BUTTON(wxID_IGNORE,    SchedPanel::onSkipBDTransaction)
    EVT_BUTTON(wxID_FILE,      SchedPanel::onOpenAttachment)
    EVT_BUTTON(wxID_FILE2,     SchedPanel::onFilterTransactions)
wxEND_EVENT_TABLE()

SchedPanel::SchedPanel(
    wxWindow* parent_win,
    wxWindowID winid,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
) :
    m_today(mmDate::today())
{
    m_tip_a.Add(_t("MMEX allows regular payments to be set up as transactions. These transactions can also be regular deposits,"
        " or transfers that will occur at some future time. These transactions act as a reminder that an event is about to occur,"
        " and appears on the Dashboard 14 days before the transaction is due."));
    m_tip_a.Add(_t("Tip: These transactions can be set up to activate - allowing the user to adjust any values on the due date."));

    create(parent_win, winid, pos, size, style, name);
    mmThemeAutoColour(this);
}

SchedPanel::~SchedPanel()
{
}

bool SchedPanel::create(
    wxWindow* parent_win,
    wxWindowID winid,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
) {
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent_win, winid, pos, size, style, name);

    createControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    // Set up the transaction filter.
    // The transFilter dialog will be destroyed when the checking panel is destroyed.
    m_filter_active = false;
    w_filter_dlg = new TrxFilterDialog(this, -1, false);

    initList();

    UsageModel::instance().pageview(this);

    return true;
}

void SchedPanel::createControls()
{
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    wxPanel* headerPanel = new wxPanel(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize,
        wxNO_BORDER | wxTAB_TRAVERSAL
    );
    itemBoxSizer9->Add(headerPanel, g_flagsBorder1V);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    wxStaticText* itemStaticText9 = new wxStaticText(headerPanel, wxID_ANY,
        _t("Scheduled Transactions")
    );
    itemStaticText9->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizerVHeader->Add(itemStaticText9, g_flagsBorder1V);

    /* Disable feature to judge reaction :-)
       https://github.com/moneymanagerex/moneymanagerex/issues/5281

        wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
        itemBoxSizerVHeader->Add(itemBoxSizerHHeader2);

        w_filter_btn = new wxButton(headerPanel, wxID_FILE2);
        w_filter_btn->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
        w_filter_btn->SetLabel(_t("Transaction Filter"));
        itemBoxSizerHHeader2->Add(w_filter_btn, g_flagsBorder1H);
    */

    mmSplitterWindow* itemSplitterWindowBillsDeposit = new mmSplitterWindow(this,
        wxID_ANY, wxDefaultPosition, wxSize(200, 200),
        wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER,
        mmThemeMetaColour(meta::COLOR_LISTPANEL)
    );

    wxVector<wxBitmapBundle> images;
    images.push_back(mmBitmapBundle(png::FOLLOW_UP));
    images.push_back(mmBitmapBundle(png::RUN_AUTO));
    images.push_back(mmBitmapBundle(png::RUN));
    images.push_back(mmBitmapBundle(png::UPARROW));
    images.push_back(mmBitmapBundle(png::DOWNARROW));

    w_list = new SchedList(this, itemSplitterWindowBillsDeposit);
    
    w_list->SetSmallImages(images);

    wxPanel* bdPanel = new wxPanel(itemSplitterWindowBillsDeposit, wxID_ANY,
        wxDefaultPosition, wxDefaultSize,
        wxNO_BORDER | wxTAB_TRAVERSAL
    );

    itemSplitterWindowBillsDeposit->SplitHorizontally(w_list, bdPanel);
    itemSplitterWindowBillsDeposit->SetMinimumPaneSize(100);
    itemSplitterWindowBillsDeposit->SetSashGravity(1.0);
    itemBoxSizer9->Add(itemSplitterWindowBillsDeposit, g_flagsExpandBorder1);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    bdPanel->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer5, g_flagsBorder1V);

    wxButton* itemButtonNew = new wxButton(bdPanel, wxID_NEW, _t("&New "));
    mmToolTip(itemButtonNew, _t("New Scheduled Transaction"));
    itemBoxSizer5->Add(itemButtonNew, 0, wxRIGHT, 2);

    wxButton* itemButton81 = new wxButton(bdPanel, wxID_EDIT, _t("&Edit "));
    mmToolTip(itemButton81, _t("Edit Scheduled Transaction"));
    itemBoxSizer5->Add(itemButton81, 0, wxRIGHT, 2);
    itemButton81->Enable(false);

    wxButton* itemButton82 = new wxButton(bdPanel, wxID_DUPLICATE, _t("D&uplicate "));
    mmToolTip(itemButton82, _t("Duplicate Scheduled Transaction"));
    itemBoxSizer5->Add(itemButton82, 0, wxRIGHT, 2);
    itemButton82->Enable(false);

    wxButton* itemButton7 = new wxButton(bdPanel, wxID_DELETE, _t("&Delete "));
    mmToolTip(itemButton7, _t("Delete Scheduled Transaction"));
    itemBoxSizer5->Add(itemButton7, 0, wxRIGHT, 2);
    itemButton7->Enable(false);

    wxButton* itemButton8 = new wxButton(bdPanel, wxID_PASTE, _t("Ente&r"));
    mmToolTip(itemButton8, _t("Enter Next Scheduled Transaction Occurrence"));
    itemBoxSizer5->Add(itemButton8, 0, wxRIGHT, 2);
    itemButton8->Enable(false);

    wxButton* buttonSkipTrans = new wxButton(bdPanel, wxID_IGNORE, _t("&Skip"));
    mmToolTip(buttonSkipTrans, _t("Skip Next Scheduled Transaction Occurrence"));
    itemBoxSizer5->Add(buttonSkipTrans, 0, wxRIGHT, 2);
    buttonSkipTrans->Enable(false);

    wxBitmapButton* btnAttachment_ = new wxBitmapButton(bdPanel, wxID_FILE,
        mmBitmapBundle(png::CLIP, mmBitmapButtonSize), wxDefaultPosition,
        wxSize(30, itemButton8->GetSize().GetY())
    );
    mmToolTip(btnAttachment_, _t("Open attachments"));
    itemBoxSizer5->Add(btnAttachment_, 0, wxRIGHT, 2);
    btnAttachment_->Enable(false);

    // Infobar-mini
    w_mini_text = new wxStaticText(bdPanel, wxID_STATIC, "");
    itemBoxSizer5->Add(w_mini_text, 1, wxGROW | wxTOP | wxLEFT, 5);

    //Infobar
    w_info_text = new wxStaticText(bdPanel, wxID_ANY,
        "",
        wxPoint(-1, -1), wxSize(200, -1),
        wxNO_BORDER | wxTE_MULTILINE | wxTE_WORDWRAP | wxST_NO_AUTORESIZE
    );
    itemBoxSizer4->Add(w_info_text, g_flagsExpandBorder1);

    SchedPanel::updateBottomPanelData(-1);
}

int SchedPanel::initList(int64 sched_id_n)
{
    w_list->DeleteAllItems();

    m_sched_xa.clear();
    const auto schedId_qpA_m = SchedSplitModel::instance().find_all_mSchedId();
    for (const SchedData& data : SchedModel::instance().find_all(
        SchedCol::COL_ID_NEXTOCCURRENCEDATE
    )) {
        if (m_filter_active && !w_filter_dlg->mmIsRecordMatches(data, schedId_qpA_m))
            continue;
        SchedModel::DataExt r(data);
        m_sched_xa.push_back(r);
    }

    sortList();

    int item = 0;
    int selected_item = -1;
    for (const auto& sched_dx : m_sched_xa) {
        if (sched_dx.m_id == sched_id_n) {
            selected_item = item;
            break;
        }
        ++item;
    }

    w_list->SetItemCount(static_cast<long>(m_sched_xa.size()));
    return selected_item;
}

wxString SchedPanel::getItem(long item, int col_id)
{
    const SchedModel::DataExt& sched_dx = this->m_sched_xa.at(item);

    switch (col_id)
    {
    case SchedList::LIST_ID_ID:
        return wxString::Format("%lld", sched_dx.m_id).Trim();
    case SchedList::LIST_ID_PAYMENT_DATE:
        return mmGetDateTimeForDisplay(sched_dx.m_date_time.isoDateTime());
    case SchedList::LIST_ID_DUE_DATE:
        return mmGetDateTimeForDisplay(sched_dx.m_due_date.isoDate());
    case SchedList::LIST_ID_ACCOUNT:
        return sched_dx.ACCOUNTNAME;
    case SchedList::LIST_ID_PAYEE:
        return sched_dx.real_payee_name();
    case SchedList::LIST_ID_STATUS:
        return sched_dx.m_status.key();
    case SchedList::LIST_ID_CATEGORY:
        return sched_dx.CATEGNAME;
    case SchedList::LIST_ID_TAGS:
        return sched_dx.TAGNAMES;
    case SchedList::LIST_ID_WITHDRAWAL: {
        wxString value = wxEmptyString;
        int64 accountid;
        double transamount;
        if (sched_dx.is_withdrawal()) {
            accountid = sched_dx.m_account_id; transamount = sched_dx.m_amount;
        }
        else if (sched_dx.is_transfer()) {
            accountid = sched_dx.m_account_id; transamount = sched_dx.m_amount;
        }
        else
            return value;
        const AccountData* account = AccountModel::instance().get_id_data_n(accountid);
        const CurrencyData* currency = account ?
            CurrencyModel::instance().get_id_data_n(account->m_currency_id) : nullptr;
        if (currency)
            value = CurrencyModel::instance().toCurrency(transamount, currency);
        if (!value.IsEmpty() && sched_dx.is_void())
            value = "* " + value;
        return value;
    }
    case SchedList::LIST_ID_DEPOSIT: {
        wxString value = wxEmptyString;
        int64 accountid;
        double transamount;
        if (sched_dx.is_deposit()) {
            accountid = sched_dx.m_account_id; transamount = sched_dx.m_amount;
        }
        else if (sched_dx.is_transfer()) {
            accountid = sched_dx.m_to_account_id_n; transamount = sched_dx.m_to_amount;
        }
        else
            return value;
        const AccountData* account = AccountModel::instance().get_id_data_n(accountid);
        const CurrencyData* currency = account ?
            CurrencyModel::instance().get_id_data_n(account->m_currency_id) : nullptr;
        if (currency)
            value = CurrencyModel::instance().toCurrency(transamount, currency);
        if (!value.IsEmpty() && sched_dx.is_void())
            value = "* " + value;
        return value;
    }
    case SchedList::LIST_ID_FREQUENCY: {
        wxString name = wxGetTranslation(sched_dx.m_repeat.m_freq.name());
        if (sched_dx.m_repeat.m_freq.has_x())
            name.Replace("%s", wxString::Format("%i", sched_dx.m_repeat.m_x));
        return name;
    }
    case SchedList::LIST_ID_REPEATS: {
        return sched_dx.m_repeat.m_num == -1
            ? L"\x221E" /* INFITITY */
            : wxString::Format("%i", sched_dx.m_repeat.m_num);
    }
    case SchedList::LIST_ID_AUTO: {
        return wxGetTranslation(sched_dx.m_repeat.m_mode.name());
    }
    case SchedList::LIST_ID_REMAINING:
        return getRemainingDays(sched_dx);
    case SchedList::LIST_ID_NUMBER:
        return sched_dx.m_number;
    case SchedList::LIST_ID_NOTES: {
        wxString value = sched_dx.m_notes;
        value.Replace("\n", " ");
        if (AttachmentModel::instance().find_ref_c(SchedModel::s_ref_type, sched_dx.m_id))
            value.Prepend(mmAttachmentManage::GetAttachmentNoteSign());
        return value;
    }
    default:
        return wxEmptyString;
    }
}

const wxString SchedPanel::getRemainingDays(const SchedData& sched_d) const
{
    int payment_days = sched_d.m_date().daysSince(m_today);
    int due_days = sched_d.m_due_date.daysSince(m_today);

    // add a warning marker (*) in front, such that it is visible
    // to the user even when the Remaining column is too narrow.
    return (due_days < 0)
        ? "*" + wxString::Format(
            wxPLURAL( "%d day overdue", "%d days overdue", -due_days),
            -due_days
        )
        : (payment_days < 0)
        ? "*" + wxString::Format(
            wxPLURAL("%d day delay", "%d days delay", -payment_days),
            -payment_days
        )
        : wxString::Format(
            wxPLURAL("%d day", "%d days", payment_days),
            payment_days
        );
}

void SchedPanel::updateBottomPanelData(int selIndex)
{
    enableEditDeleteButtons(selIndex >= 0);
    if (selIndex != -1) {
        w_mini_text->SetLabelText(CategoryModel::instance().get_id_fullname(
            m_sched_xa[selIndex].m_category_id_n
        ));
        w_info_text->SetLabelText(m_sched_xa[selIndex].m_notes);
    }
}

void SchedPanel::enableEditDeleteButtons(bool en)
{
    wxButton* edit_btn = static_cast<wxButton*>(FindWindow(wxID_EDIT));
    if (edit_btn)
        edit_btn->Enable(en);

    wxButton* delete_btn = static_cast<wxButton*>(FindWindow(wxID_DELETE));
    if (delete_btn)
        delete_btn->Enable(en);

    wxButton* dup_btn = static_cast<wxButton*>(FindWindow(wxID_DUPLICATE));
    if (dup_btn)
        dup_btn->Enable(en);

    wxButton* paste_btn = static_cast<wxButton*>(FindWindow(wxID_PASTE));
    if (paste_btn)
        paste_btn->Enable(en);

    wxButton* ignore_btn = static_cast<wxButton*>(FindWindow(wxID_IGNORE));
    if (ignore_btn)
        ignore_btn->Enable(en);

    wxButton* file_btn = static_cast<wxButton*>(FindWindow(wxID_FILE));
    if (file_btn)
        file_btn->Enable(en);

    w_info_text->SetLabelText(this->getRandomTip());
    w_mini_text->ClearBackground();
}

void SchedPanel::sortList()
{
    std::sort(m_sched_xa.begin(), m_sched_xa.end());
    switch (w_list->getSortColId()) {
    case SchedList::LIST_ID_ID:
        std::stable_sort(m_sched_xa.begin(), m_sched_xa.end(), SchedData::SorterById());
        break;
    case SchedList::LIST_ID_PAYMENT_DATE:
        std::stable_sort(m_sched_xa.begin(), m_sched_xa.end(), SchedData::SorterByDateTime());
        break;
    case SchedList::LIST_ID_DUE_DATE:
        std::stable_sort(m_sched_xa.begin(), m_sched_xa.end(), SchedData::SorterByDueDate());
        break;
    case SchedList::LIST_ID_ACCOUNT:
        std::stable_sort(m_sched_xa.begin(), m_sched_xa.end(), SchedModel::SorterByACCOUNTNAME());
        break;
    case SchedList::LIST_ID_PAYEE:
        std::stable_sort(m_sched_xa.begin(), m_sched_xa.end(), SchedModel::SorterByPAYEENAME());
        break;
    case SchedList::LIST_ID_STATUS:
        std::stable_sort(m_sched_xa.begin(), m_sched_xa.end(), SchedData::SorterByStatus());
        break;
    case SchedList::LIST_ID_CATEGORY:
        std::stable_sort(m_sched_xa.begin(), m_sched_xa.end(), SchedModel::SorterByCATEGNAME());
        break;
    case SchedList::LIST_ID_WITHDRAWAL:
        std::stable_sort(m_sched_xa.begin(), m_sched_xa.end(), SchedModel::SorterByWITHDRAWAL());
        break;
    case SchedList::LIST_ID_DEPOSIT:
        std::stable_sort(m_sched_xa.begin(), m_sched_xa.end(), SchedModel::SorterByDEPOSIT());
        break;
    case SchedList::LIST_ID_FREQUENCY:
        std::stable_sort(m_sched_xa.begin(), m_sched_xa.end(), SchedData::SorterByRepeatFreq());
        break;
    case SchedList::LIST_ID_REPEATS:
        std::stable_sort(m_sched_xa.begin(), m_sched_xa.end(), SchedData::SorterByRepeatNum());
        break;
    case SchedList::LIST_ID_AUTO:
        std::stable_sort(m_sched_xa.begin(), m_sched_xa.end(), SchedData::SorterByRepeatMode());
        break;
    case SchedList::LIST_ID_REMAINING:
        // in almost all cases, sorting by remaining days is equivalent to sorting by TRANSDATE
        std::stable_sort(m_sched_xa.begin(), m_sched_xa.end(), SchedData::SorterByDateTime());
        break;
    case SchedList::LIST_ID_NOTES:
        std::stable_sort(m_sched_xa.begin(), m_sched_xa.end(), SchedData::SorterByNotes());
        break;
    default:
        break;
    }
    if (!w_list->getSortAsc())
        std::reverse(m_sched_xa.begin(), m_sched_xa.end());
}

wxString SchedPanel::getRandomTip()
{
    return this->m_tip_a[rand() % this->m_tip_a.GetCount()];
}

void SchedList::refreshVisualList(int selected_index)
{

    if (selected_index >= static_cast<long>(w_panel->m_sched_xa.size()) || selected_index < 0)
        selected_index = - 1;

    if (!w_panel->m_sched_xa.empty()) {
        RefreshItems(0, w_panel->m_sched_xa.size() - 1);
    }
    else {
        selected_index = -1;
    }

    if (selected_index >= 0 && !w_panel->m_sched_xa.empty()) {
        SetItemState(selected_index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(selected_index, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        EnsureVisible(selected_index);
    }
    m_select_n = selected_index;
    w_panel->updateBottomPanelData(selected_index);
}

void SchedPanel::onFilterTransactions(wxCommandEvent& WXUNUSED(event))
{

    if (w_filter_dlg->ShowModal() == wxID_OK && w_filter_dlg->mmIsSomethingChecked()) {
        m_filter_active = true;
        w_filter_btn->SetBitmap(mmBitmapBundle(png::TRANSFILTER_ACTIVE, mmBitmapButtonSize));
    }
    else {
        m_filter_active = false;
        w_filter_btn->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
    }

    initList();
}
