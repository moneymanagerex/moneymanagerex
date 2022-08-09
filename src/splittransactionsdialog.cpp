/*******************************************************
Copyright (C) 2006-2012 Madhan Kanagavel
Copyright (C) 2013 - 2016, 2020 - 2022 Nikolay Akimov
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

#include "splittransactionsdialog.h"
#include "categdialog.h"
#include "constants.h"
#include "images_list.h"
#include "mmSimpleDialogs.h"
#include "util.h"
#include "paths.h"
#include "splitdetailsdialog.h"
#include "validators.h"

#include "model/Model_Account.h"
#include "model/Model_Category.h"

#include <wx/statline.h>

#define STATIC_SPLIT_NUM 2

 wxBEGIN_EVENT_TABLE(mmSplitTransactionDialog, wxDialog)
     EVT_CHILD_FOCUS(mmSplitTransactionDialog::OnFocusChange)
     EVT_BUTTON(wxID_OK, mmSplitTransactionDialog::OnOk)
     EVT_BUTTON(mmID_SPLIT, mmSplitTransactionDialog::OnAddRow)
     EVT_BUTTON(mmID_REMOVE, mmSplitTransactionDialog::OnRemoveRow)
     EVT_TEXT_ENTER(wxID_ANY, mmSplitTransactionDialog::OnTextEntered)
 wxEND_EVENT_TABLE()

mmSplitTransactionDialog::mmSplitTransactionDialog( )
{
}

mmSplitTransactionDialog::~mmSplitTransactionDialog()
{
     Model_Infotable::instance().Set("SPLITTRANSACTION_DIALOG_SIZE", GetSize());
}

mmSplitTransactionDialog::mmSplitTransactionDialog(wxWindow* parent
    , std::vector<Split>& split
    , int accountID
    , int transType
    , double totalAmount
    , bool is_view_only
)
    : m_splits(split)
    , totalAmount_(totalAmount)
    , transType_(transType)
    , row_num_(static_cast<int>(split.size()))
    , is_view_only_(is_view_only)
{
    Model_Account::Data* account = Model_Account::instance().get(accountID);
    m_currency = account ? Model_Account::currency(account) : Model_Currency::GetBaseCurrency();

    this->SetFont(parent->GetFont());
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
    Centre();
    SetIcon(mmex::getProgramIcon());

    return TRUE;
}

void mmSplitTransactionDialog::CreateControls()
{
    SetEvtHandlerEnabled(false);
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainSizer);

    slider_ = new wxScrolledWindow(this, wxNewId(), wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    mainSizer->Add(slider_, wxSizerFlags().Align(wxALIGN_LEFT | wxEXPAND).Border(wxALL, 1).Proportion(0));

    wxBoxSizer* dialogMainSizerV = new wxBoxSizer(wxVERTICAL);
    slider_->SetSizer(dialogMainSizerV);

    flexGridSizer_ = new wxFlexGridSizer(0, 3, 0, 0);
    flexGridSizer_->AddGrowableCol(1, 0);
    dialogMainSizerV->Add(flexGridSizer_, g_flagsExpand);

    wxStaticText* categoryText = new wxStaticText(slider_, wxID_STATIC, _("Category"));
    wxStaticText* amountText = new wxStaticText(slider_, wxID_STATIC, _("Amount"));
    flexGridSizer_->AddSpacer(1);
    flexGridSizer_->Add(categoryText, g_flagsH);
    flexGridSizer_->Add(amountText, g_flagsH);

    int size = static_cast<int>(m_splits.size()) + 1;
    if (size < STATIC_SPLIT_NUM) size = STATIC_SPLIT_NUM;
    for (int i = 0; i < size; i++)
    {
        wxCheckBox* cb = new wxCheckBox(slider_, wxID_HIGHEST + i, ""
            , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator
            , wxString::Format("check_box%i", i));
        mmComboBoxCategory* cbc = new mmComboBoxCategory(slider_, wxID_HIGHEST + i);
        cbc->SetName(wxString::Format("category_box%i", i));
        cbc->Bind(wxEVT_CHAR_HOOK, &mmSplitTransactionDialog::OnComboKey, this);

        mmTextCtrl* val = new mmTextCtrl(slider_, wxID_HIGHEST + i, ""
            , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
        val->SetMinSize(wxSize(100,-1));
        val->SetName(wxString::Format("value_box%i", i));
        flexGridSizer_->Add(cb, g_flagsH);
        flexGridSizer_->Add(cbc, g_flagsExpand);
        flexGridSizer_->Add(val, g_flagsH);

        if (i < m_splits.size())
        {
            cb->SetValue(true);
            const auto categ = Model_Category::full_name(m_splits.at(i).CATEGID
                , m_splits.at(i).SUBCATEGID);
            cbc->ChangeValue(categ);
            val->SetValue(m_splits.at(i).SPLITTRANSAMOUNT, m_currency);
        }

        cb->Enable(false);
        cb->Hide();
        cbc->Enable(i <= m_splits.size() && !is_view_only_);
        val->Enable(i <= m_splits.size() && !is_view_only_);
        if (i == static_cast<int>(m_splits.size())) {
            cbc->SetFocusFromKbd();
        }
    }

    slider_->Fit();
    slider_->SetMinSize(slider_->GetBestVirtualSize());
    slider_->SetScrollRate(1, 1);
    SetMinSize(wxSize(384, 432));

    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* plusAmountSizer = new wxBoxSizer(wxHORIZONTAL);
    bottomSizer->Add(plusAmountSizer, wxSizerFlags().Align(wxALIGN_LEFT).Border(wxALL, 5).Proportion(1));

    wxButton* bAdd = new wxButton(this, mmID_SPLIT, _("Add Split"));
    bAdd->SetName("");
    bAdd->Enable(!is_view_only_);
    plusAmountSizer->AddSpacer(mmBitmapButtonSize + 10);
    plusAmountSizer->Add(bAdd);

    wxButton* bRemove = new wxButton(this, mmID_REMOVE, _("Remove Split"));
    bRemove->SetName("");
    bRemove->Enable(!is_view_only_);
    plusAmountSizer->AddSpacer(5);
    plusAmountSizer->Add(bRemove);

    wxBoxSizer* totalAmountSizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* transAmountText = new wxStaticText(this, wxID_STATIC, _("Total:"));
    transAmount_ = new wxStaticText(this, wxID_STATIC, wxEmptyString);
    totalAmountSizer->Add(transAmountText, wxSizerFlags());
    totalAmountSizer->Add(transAmount_, wxSizerFlags().Border(wxLEFT, 5));
    bottomSizer->Add(totalAmountSizer, wxSizerFlags().Border(wxALL, 5));
    mainSizer->Add(bottomSizer, g_flagsExpand);

    // OK Cancel buttons
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    mainSizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 5));
    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

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
    itemButtonOK_->Enable(!is_view_only_);

    SetEvtHandlerEnabled(true);

    Fit();
}

void mmSplitTransactionDialog::OnOk( wxCommandEvent& /*event*/ )
{
    int i = 0;
    std::vector<Split> split;
    while (true)
    {
        auto name = wxString::Format("check_box%i", i);
        auto cb = static_cast<wxCheckBox*>(FindWindowByName(name));
        if (cb)
        {
            if (!mmDoCheckRow(i))
                return;

            if (cb->IsChecked())
            {
                Split s;
                name = wxString::Format("category_box%i", i);
                auto cbc = static_cast<mmComboBoxCategory*>(FindWindowByName(name));
                s.CATEGID = cbc->mmGetCategoryId();
                s.SUBCATEGID = cbc->mmGetSubcategoryId();

                name = wxString::Format("value_box%i", i);
                auto val = static_cast<mmTextCtrl*>(FindWindowByName(name));
                val->GetDouble(s.SPLITTRANSAMOUNT);
                split.push_back(s);
            }
            i++;
        }
        else
            break;
    }

    //Check total amount - should be positive
    totalAmount_ = 0;
    for (const auto& entry : split)
        totalAmount_ += entry.SPLITTRANSAMOUNT;
    if (totalAmount_ < 0) {
        return mmErrorDialogs::MessageError(this, _("Invalid Total Amount"), _("Error"));
    }

    m_splits = split;
    EndModal(wxID_OK);
}

void mmSplitTransactionDialog::OnAddRow(wxCommandEvent& event)
{
    int i = 0;
    bool is_last_row_complited = false;
    while (true)
    {
        auto name = wxString::Format("check_box%i", i);
        auto cb = static_cast<wxCheckBox*>(FindWindowByName(name));
        if (cb) {
            i ++;
            is_last_row_complited = cb->IsChecked();
        }
        else
            break;
    }

    if (!is_last_row_complited)
        i--;

    if (mmDoCheckRow(i, false)) {
        mmDoEnableLineById(i);
    }
    event.Skip();
}

void mmSplitTransactionDialog::OnRemoveRow(wxCommandEvent& event)
{
    auto name = wxString::Format("category_box%i", row_num_);
    auto cbc = static_cast<mmComboBoxCategory*>(FindWindowByName(name));
    name = wxString::Format("value_box%i", row_num_);
    auto val = static_cast<mmTextCtrl*>(FindWindowByName(name));
    name = wxString::Format("check_box%i", row_num_);
    auto cb = static_cast<wxCheckBox*>(FindWindowByName(name));

    if (cb && cbc && val)
    {
        cb->SetValue(false);
        cbc->Disable();
        cbc->SetValue("");
        val->Disable();
        val->Clear();
        UpdateSplitTotal();
    }
}

void mmSplitTransactionDialog::UpdateSplitTotal()
{
    double total = 0;
    int i = 0;
    while (true)
    {
        auto name = wxString::Format("check_box%i", i);
        auto cb = static_cast<wxCheckBox*>(FindWindowByName(name));
        if (cb)
        {
            if (cb->IsChecked())
            {
                name = wxString::Format("value_box%i", i);
                auto val = static_cast<mmTextCtrl*>(FindWindowByName(name));
                double amount = 0.0;
                if (val && val->GetDouble(amount)) {
                    total += amount;
                }
            }
            i++;
        }
        else
            break;
    }

    totalAmount_ = total;

    wxString total_text = Model_Currency::toCurrency(total, m_currency);
    transAmount_->SetLabelText(total_text);
    Layout();
}

void mmSplitTransactionDialog::mmDoEnableLineById(int id)
{
    auto name = wxString::Format("check_box%i", id);
    auto cb = static_cast<wxCheckBox*>(FindWindowByName(name));
    if (cb) {
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
        int i = id;
        wxCheckBox* ncb = new wxCheckBox(slider_, wxID_HIGHEST + i, ""
            , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE, wxDefaultValidator
            , wxString::Format("check_box%i", i));
        mmComboBoxCategory* ncbc = new mmComboBoxCategory(slider_, wxID_HIGHEST + i);
        ncbc->Bind(wxEVT_CHAR_HOOK, &mmSplitTransactionDialog::OnComboKey, this);
        ncb->Disable();
        ncb->Hide();
        ncbc->SetName(wxString::Format("category_box%i", i));
        mmTextCtrl* nval = new mmTextCtrl(slider_, wxID_HIGHEST + i, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
        nval->SetMinSize(wxSize(100,-1));
        nval->SetName(wxString::Format("value_box%i", i));
        flexGridSizer_->Add(ncb, g_flagsH);
        flexGridSizer_->Add(ncbc, g_flagsExpand);
        flexGridSizer_->Add(nval, g_flagsH);
        ncbc->SetFocus();
        slider_->FitInside();
        slider_->ScrollLines(ncbc->GetSize().GetY() * 2);
    }
    slider_->SetMinSize(slider_->GetBestVirtualSize());
    Fit();
}

void mmSplitTransactionDialog::OnTextEntered(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_HIGHEST;

    mmDoCheckRow(i, true);

    auto name = wxString::Format("check_box%i", i);
    auto cb = static_cast<wxCheckBox*>(FindWindowByName(name));
    name = wxString::Format("value_box%i", i);
    auto val = static_cast<mmTextCtrl*>(FindWindowByName(name));
    name = wxString::Format("category_box%i", i);
    auto cbc = static_cast<mmComboBoxCategory*>(FindWindowByName(name));

    bool is_empty = (cbc && cbc->GetValue().empty() && val && val->GetValue().empty());
    if (is_empty) {
        return;
    }

    double amount = 0;
    if (val && val->checkValue(amount, false))
    {
        if (cb && cbc->mmIsValid()) {
            cb->SetValue(true);
            mmDoEnableLineById(i + 1);
        }
        else
            return mmErrorDialogs::InvalidCategory(cbc, true);
    }
    
    UpdateSplitTotal();
}

bool mmSplitTransactionDialog::mmDoCheckRow(int i, bool silent)
{
    auto name = wxString::Format("category_box%i", i);
    auto cbc = static_cast<mmComboBoxCategory*>(FindWindowByName(name));
    name = wxString::Format("value_box%i", i);
    auto val = static_cast<mmTextCtrl*>(FindWindowByName(name));
    name = wxString::Format("check_box%i", i);
    auto cb = static_cast<wxCheckBox*>(FindWindowByName(name));

    if (cbc && cbc->GetValue().empty() && val && val->GetValue().empty())
    {
        if (cb) {
            cb->SetValue(false);
        }
        return true;
    }

    double amount = 0.0;
    if (!silent)
    {
        if (cbc &&!cbc->mmIsValid()) {
            mmErrorDialogs::InvalidCategory(cbc, true);
            return false;
        }

        if (val && !val->checkValue(amount, false)) {
            return false;
        }
    }
    if (cb) {
        cb->SetValue(val->GetDouble(amount) && cbc->mmIsValid());
    }
    return true;
}

void mmSplitTransactionDialog::OnFocusChange(wxChildFocusEvent& event)
{
    auto name = wxString::Format("category_box%i", row_num_);
    auto cbc = static_cast<mmComboBoxCategory*>(FindWindowByName(name));
    if (cbc) {
        cbc->ChangeValue(cbc->GetValue());
    }

    name = wxString::Format("value_box%i", row_num_);
    auto val = static_cast<mmTextCtrl*>(FindWindowByName(name));
    if (val) {
        val->Calculate(Model_Currency::precision(m_currency));
        val->SelectAll();
    }

    mmDoCheckRow(row_num_, true);
    UpdateSplitTotal();

    wxWindow* w = event.GetWindow();
    ;
    if (w && !w->GetName().empty()) {
        row_num_ = w->GetId() - wxID_HIGHEST;
    }
}

void mmSplitTransactionDialog::OnComboKey(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_RETURN)
    {
        auto cbc = static_cast<mmComboBoxCategory*>(event.GetEventObject());
        if (cbc) {
            auto category = cbc->GetValue();
            if (category.empty())
            {
                mmCategDialog dlg(this, true, -1, -1);
                dlg.ShowModal();
                cbc->mmDoReInitialize();
                category = Model_Category::full_name(dlg.getCategId(), dlg.getSubCategId());
                cbc->ChangeValue(category);
            }
        }
    }
    event.Skip();
}