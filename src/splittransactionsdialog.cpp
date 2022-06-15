/*******************************************************
Copyright (C) 2006-2012 Madhan Kanagavel
Copyright (C) 2013 - 2016, 2020 -2022 Nikolay Akimov

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

#include "splittransactionsdialog.h"
#include "constants.h"
#include "mmSimpleDialogs.h"
#include "util.h"
#include "paths.h"
#include "splitdetailsdialog.h"
#include "validators.h"

#include "model/Model_Account.h"
#include "model/Model_Category.h"

#include <wx/statline.h>

#define STATIC_SPLIT_NUM 7

 wxBEGIN_EVENT_TABLE(mmSplitTransactionDialog, wxDialog)
     EVT_CHILD_FOCUS(mmSplitTransactionDialog::OnFocusChange)
     EVT_BUTTON(wxID_OK, mmSplitTransactionDialog::OnOk)
     EVT_TEXT_ENTER(wxID_ANY, mmSplitTransactionDialog::OnTextEntered)
     EVT_CHECKBOX(wxID_ANY, mmSplitTransactionDialog::OnCheckBox)
 wxEND_EVENT_TABLE()

mmSplitTransactionDialog::mmSplitTransactionDialog( )
{
}

mmSplitTransactionDialog::~mmSplitTransactionDialog()
{
     Model_Infotable::instance().Set("SPLITTRANSACTION_DIALOG_SIZE", GetRect());
}

mmSplitTransactionDialog::mmSplitTransactionDialog(wxWindow* parent
    , std::vector<Split>& split
    , int accountID
    , int transType
    , double totalAmount
    , const wxString& name
)
    : m_splits(split)
    , totalAmount_(totalAmount)
    , accountID_(accountID)
    , transType_(transType)
    , isItemsChanged_(false)
{
    for (const auto& item : m_splits)
        m_local_splits.push_back(item);

    Create(parent);
}

bool mmSplitTransactionDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style
    , const wxString& name
    )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style, name);

    CreateControls();
    UpdateSplitTotal();

    mmSetSize(this);
    SetIcon(mmex::getProgramIcon());

    return TRUE;
}

void mmSplitTransactionDialog::CreateControls()
{
    SetEvtHandlerEnabled(false);
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainSizer);

    wxStaticText* headingText = new wxStaticText(this, wxID_STATIC, _("Split Category Details"));
    mainSizer->Add(headingText, g_flagsV);

    slider_ = new wxScrolledWindow(this, wxNewId(), wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    mainSizer->Add(slider_, g_flagsExpandBorder1);
    slider_->SetMinSize(wxSize(350, 400));

    Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
    Model_Account::Data* account = Model_Account::instance().get(accountID_);
    if (account) {
        currency = Model_Account::currency(account);
    }

    wxBoxSizer* dialogMainSizerV = new wxBoxSizer(wxVERTICAL);
    slider_->SetSizer(dialogMainSizerV);

    flexGridSizer_ = new wxFlexGridSizer(0, 3, 0, 0);
    flexGridSizer_->AddGrowableCol(1, 0);
    dialogMainSizerV->Add(flexGridSizer_, g_flagsExpand);

    int size = static_cast<int>(m_local_splits.size()) + 1;
    if (size < STATIC_SPLIT_NUM) size = STATIC_SPLIT_NUM;
    for (int i = 0; i < size; i++)
    {
        wxCheckBox* cb = new wxCheckBox(slider_, wxID_HIGHEST + i, ""
            , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator
            , wxString::Format("check_box%i", i));
        mmComboBoxCategory* cbc = new mmComboBoxCategory(slider_, wxID_HIGHEST + i);
        cbc->SetName(wxString::Format("category_box%i", i));
        mmTextCtrl* val = new mmTextCtrl(slider_, wxID_HIGHEST + i, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
        val->SetMinSize(wxSize(100,-1));
        val->SetName(wxString::Format("value_box%i", i));
        flexGridSizer_->Add(cb, g_flagsH);
        flexGridSizer_->Add(cbc, g_flagsExpand);
        flexGridSizer_->Add(val, g_flagsH);

        if (i < m_local_splits.size())
        {
            cb->SetValue(true);
            const auto categ = Model_Category::full_name(m_local_splits.at(i).CATEGID, m_local_splits.at(i).SUBCATEGID);
            cbc->SetValue(categ);
            val->SetValue(m_local_splits.at(i).SPLITTRANSAMOUNT, currency);
        }

        cb->Enable(i <= m_local_splits.size());
        cbc->Enable(i <= m_local_splits.size());
        val->Enable(i <= m_local_splits.size());
        if (i == static_cast<int>(m_local_splits.size())) {
            cbc->SetFocusFromKbd();
        }
    }

    slider_->GetBestVirtualSize();
    slider_->FitInside();
    slider_->SetScrollRate(1, 1);

    wxBoxSizer* totalAmountSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* transAmountText = new wxStaticText(this, wxID_STATIC, _("Total:"));
    transAmount_ = new wxStaticText(this, wxID_STATIC, wxEmptyString);
    totalAmountSizer->Add(transAmountText, g_flagsH);
    totalAmountSizer->Add(transAmount_, g_flagsExpand);
    mainSizer->Add(totalAmountSizer, g_flagsV);

    // OK Cancel buttons
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    mainSizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 5));
    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxSizerFlags flagsH = wxSizerFlags(g_flagsH).Border(wxLEFT | wxRIGHT | wxBOTTOM, 5).Center();
    wxSizerFlags flagsV = wxSizerFlags(g_flagsV).Border(wxLEFT | wxRIGHT | wxBOTTOM, 5).Center();
    wxBoxSizer* mainButtonSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* topRowButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* bottomRowButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    mainButtonSizer->Add(topRowButtonSizer, flagsV);
    mainButtonSizer->Add(bottomRowButtonSizer, flagsV);
    buttons_sizer->Add(mainButtonSizer);

    itemButtonOK_ = new wxButton(buttons_panel, wxID_OK, _("&OK "));
    wxButton* itemButtonCancel = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));

    bottomRowButtonSizer->Add(itemButtonOK_, g_flagsH);
    bottomRowButtonSizer->Add(itemButtonCancel, g_flagsH);

    SetEvtHandlerEnabled(true);
}

void mmSplitTransactionDialog::OnOk( wxCommandEvent& /*event*/ )
{
    for (int i = static_cast<int>(m_local_splits.size()) - 1; i >= 0; --i)
    {
        auto name = wxString::Format("check_box%i", i);
        auto cb = static_cast<wxCheckBox*>(FindWindowByName(name));
        if (cb && !cb->GetValue()) {
            m_local_splits.erase(m_local_splits.begin() + i);
        }
    }

    //Check total amount - should be positive
    totalAmount_ = 0;
    for (const auto& entry : m_local_splits)
        totalAmount_ += entry.SPLITTRANSAMOUNT;
    if (totalAmount_ < 0)
    {
        return mmErrorDialogs::MessageError(this, _("Invalid Total Amount"), _("Error"));
    }

    m_splits.swap(m_local_splits);
    EndModal(wxID_OK);
}


void mmSplitTransactionDialog::UpdateSplitTotal()
{
    Model_Account::Data *account = Model_Account::instance().get(accountID_);
    const auto curr = account ? Model_Account::currency(account) : Model_Currency::GetBaseCurrency();
    double total = 0;

    for (int i = static_cast<int>(m_local_splits.size()) - 1; i >= 0; --i)
    {
        auto name = wxString::Format("check_box%i", i);
        auto cb = static_cast<wxCheckBox*>(FindWindowByName(name));
        if (cb && cb->GetValue())
            total += m_local_splits.at(i).SPLITTRANSAMOUNT;
    }
    totalAmount_ = total;

    wxString total_text = Model_Currency::toCurrency(total, curr);
    transAmount_->SetLabelText(total_text);
}

void mmSplitTransactionDialog::mmDoEnableLineById(int id, bool value)
{
    if (id < static_cast<int>(m_local_splits.size()) || id < STATIC_SPLIT_NUM)
    {
        auto name = wxString::Format("check_box%i", id);
        auto cb = static_cast<wxCheckBox*>(FindWindowByName(name));
        if (cb) {
            cb->Enable(true);
            cb->SetValue(value);
        }

        name = wxString::Format("category_box%i", id);
        auto cbc = static_cast<mmComboBoxCategory*>(FindWindowByName(name));
        if (cbc) cbc->Enable(true);
        if (cbc) cbc->SetFocus();

        name = wxString::Format("value_box%i", id);
        auto val = static_cast<mmTextCtrl*>(FindWindowByName(name));
        if (val) val->Enable(true);
    }
    else
    {
        int i = static_cast<int>(m_local_splits.size());
        wxCheckBox* cb = new wxCheckBox(slider_, wxID_HIGHEST + i, ""
            , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator
            , wxString::Format("check_box%i", i));
        mmComboBoxCategory* cbc = new mmComboBoxCategory(slider_, wxID_HIGHEST + i);
        cbc->SetName(wxString::Format("category_box%i", i));
        mmTextCtrl* val = new mmTextCtrl(slider_, wxID_HIGHEST + i, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
        val->SetMinSize(wxSize(100,-1));
        val->SetName(wxString::Format("value_box%i", i));
        flexGridSizer_->Add(cb, g_flagsH);
        flexGridSizer_->Add(cbc, g_flagsExpand);
        flexGridSizer_->Add(val, g_flagsH);
        cbc->SetFocus();
        slider_->FitInside();
        slider_->ScrollLines(cbc->GetSize().GetY() * 2);
    }
}

void mmSplitTransactionDialog::OnTextEntered(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_HIGHEST;

    auto name = wxString::Format("value_box%i", i);
    auto val = static_cast<mmTextCtrl*>(FindWindowByName(name));

    double amount = 0;
    if (val && val->GetDouble(amount))
    {
        name = wxString::Format("category_box%i", i);
        auto cbc = static_cast<mmComboBoxCategory*>(FindWindowByName(name));
        if (cbc->mmIsValid()) {
            name = wxString::Format("check_box%i", i);
            auto cb = static_cast<wxCheckBox*>(FindWindowByName(name));
            if (cb) {
                cb->SetValue(true);
                cb->Enable();

                Split s;
                s.CATEGID = cbc->mmGetCategoryId();
                s.SUBCATEGID = cbc->mmGetSubcategoryId();
                s.SPLITTRANSAMOUNT = amount;
                if (i < static_cast<int>(m_local_splits.size()))
                    m_local_splits[i] = s;
                else {
                    m_local_splits.push_back(s);
                    mmDoEnableLineById(i + 1);
                }
                isItemsChanged_ = true;
            }
        }
        else
            return mmErrorDialogs::InvalidCategory(cbc, true);
    }
    UpdateSplitTotal();
}

void mmSplitTransactionDialog::OnCheckBox(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_HIGHEST;

    if (i >= static_cast<int>(m_local_splits.size()) && event.IsChecked()) {

        auto name = wxString::Format("category_box%i", i);
        auto cbc = static_cast<mmComboBoxCategory*>(FindWindowByName(name));
        name = wxString::Format("value_box%i", i);
        auto val = static_cast<mmTextCtrl*>(FindWindowByName(name));

        Split s;
        if (val && val->Calculate())
            val->GetDouble(s.SPLITTRANSAMOUNT);
        else
            s.SPLITTRANSAMOUNT = 0.0;
        s.CATEGID = cbc->mmIsValid() ? cbc->mmGetCategoryId() : -1;
        s.SUBCATEGID = cbc->mmIsValid() ? cbc->mmGetSubcategoryId() : -1;
        SplitDetailDialog dlg(this, s, transType_, accountID_);
        if (dlg.ShowModal() == wxID_OK) {
            s = dlg.getResult();

            auto categ = Model_Category::full_name(s.CATEGID, s.SUBCATEGID);
            if (cbc) cbc->SetValue(categ);
            if (val) val->SetValue(s.SPLITTRANSAMOUNT);

            m_local_splits.push_back(s);
            isItemsChanged_ = true;

            mmDoEnableLineById(i + 1);
        }
        else
            mmDoEnableLineById(i, false);
    }
    UpdateSplitTotal();
}

void mmSplitTransactionDialog::OnFocusChange(wxChildFocusEvent& event)
{
    auto name = wxString::Format("category_box%i", object_in_focus_ - wxID_HIGHEST);
    auto cbc = static_cast<mmComboBoxCategory*>(FindWindowByName(name));
    if (cbc)
        cbc->SetValue(cbc->GetValue());

    name = wxString::Format("value_box%i", object_in_focus_ - wxID_HIGHEST);
    auto val = static_cast<mmTextCtrl*>(FindWindowByName(name));
    if (val && val->Calculate()) {
        wxCommandEvent evt(wxID_ANY, object_in_focus_);
        OnTextEntered(evt);
    }

    wxWindow* w = event.GetWindow();
    if (w) {
        object_in_focus_ = w->GetId();
    }

    UpdateSplitTotal();
}