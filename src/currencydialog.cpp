﻿/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#include "currencydialog.h"
#include "constants.h"
#include "util.h"
#include "model/Model_Currency.h"
#include "defs.h"
#include "paths.h"
#include "validators.h"
#include "mmtextctrl.h"
#include "mmSimpleDialogs.h"

#include <wx/combobox.h>
#include <wx/valnum.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmCurrencyDialog, wxDialog);

wxBEGIN_EVENT_TABLE( mmCurrencyDialog, wxDialog )
    EVT_BUTTON(wxID_REFRESH, mmCurrencyDialog::OnUpdate)
wxEND_EVENT_TABLE()

mmCurrencyDialog::mmCurrencyDialog()
{
}

mmCurrencyDialog::~mmCurrencyDialog()
{
}

mmCurrencyDialog::mmCurrencyDialog(wxWindow* parent, Model_Currency::Data * currency)
    : m_currency(currency)
        , scale_(2)
        , baseConvRate_()
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_STATIC, _("Currency Manager"), wxDefaultPosition, wxSize(500, 300), style);
}

bool mmCurrencyDialog::Create(wxWindow* parent, wxWindowID id
    , const wxString& caption, const wxPoint& pos
    , const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();

    if (!m_currency)
    {
        wxArrayString c;
        auto cur = Model_Currency::all_currencies_template();
        for (const auto &i : cur)
            c.Add(std::get<Model_Currency::NAME>(i));
        mmSingleChoiceDialog select_currency_name(this, _("Currency name"), _("Select Currency"), c);
        if (select_currency_name.ShowModal() == wxID_OK)
        {
            wxString name = select_currency_name.GetStringSelection();
            // fill currency data from template
            for (const auto &data : Model_Currency::all_currencies_template())
            {
                if (std::get<Model_Currency::NAME>(data) != name) continue;
                // Sample : std::make_tuple("GBP", "UK Pound", L"£", "", "Pound", "Pence", 100, 1);
                m_currency = Model_Currency::instance().create();
                m_currency->CURRENCYNAME = name;
                m_currency->CURRENCY_SYMBOL = std::get<Model_Currency::SYMBOL>(data);
                m_currency->PFX_SYMBOL = std::get<Model_Currency::PREFIX>(data);
                m_currency->SFX_SYMBOL = std::get<Model_Currency::SUFFIX>(data);
                m_currency->UNIT_NAME = std::get<Model_Currency::CURR_NAME>(data);
                m_currency->CENT_NAME = std::get<Model_Currency::CENT_NAME>(data);
                m_currency->SCALE = std::get<Model_Currency::PRECISION>(data);
                m_currency->BASECONVRATE = std::get<Model_Currency::CONV_RATE>(data);
                m_currency->DECIMAL_POINT = wxNumberFormatter::GetDecimalSeparator();
                m_currency->GROUP_SEPARATOR = Model_Currency::os_group_separator();
            }
        }
    }

    fillControls();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmex::getProgramIcon());

    baseConvRate_->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(mmCurrencyDialog::onTextEntered), nullptr, this);

    Centre();
    return TRUE;
}

void mmCurrencyDialog::fillControls()
{
    if (m_currency)
    {
        m_currencyName->SetValue(m_currency->CURRENCYNAME);
        m_currencySymbol->SetValue(m_currency->CURRENCY_SYMBOL);

        wxString dispAmount = "";
        double base_amount = 1000;
        double amount = base_amount;

        if (m_currency->BASECONVRATE != 0.0)
            amount = base_amount / m_currency->BASECONVRATE;
        dispAmount = wxString::Format(_("%s Converted to: %s")
            , Model_Currency::toCurrency(base_amount)
            , Model_Currency::toCurrency(amount, m_currency));
        baseRateSample_->SetLabelText(dispAmount);
        amount = 123456.78;
        dispAmount = wxString::Format(_("%.2f Shown As: %s"), amount
            , Model_Currency::toCurrency(amount, m_currency));
        sampleText_->SetLabelText(dispAmount);

        pfxTx_->SetValue(m_currency->PFX_SYMBOL);
        sfxTx_->SetValue(m_currency->SFX_SYMBOL);
        decTx_->SetValue(m_currency->DECIMAL_POINT);
        grpTx_->SetValue(m_currency->GROUP_SEPARATOR);
        unitTx_->SetValue(m_currency->UNIT_NAME);
        centTx_->SetValue(m_currency->CENT_NAME);
        scale_ = log10(m_currency->SCALE);
        wxString scale_value = wxString::Format("%i", scale_);
        scaleTx_->SetValue(scale_value);
        baseConvRate_->SetValue(m_currency->BASECONVRATE, 4);
        m_currencySymbol->SetValue(m_currency->CURRENCY_SYMBOL);
    }
    else
    {
        convRate_ = 1;
        scale_ = 1;
        baseConvRate_->SetValue("1");
    }

    // resize the dialog window
    Fit();
}

void mmCurrencyDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer3->AddGrowableCol(1);
    itemBoxSizer2->Add(itemFlexGridSizer3, g_flags);

    //--------------------------
    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Currency Name")), g_flags);
    m_currencyName = new mmTextCtrl(this, ID_DIALOG_CURRENCY_CHOICE
        , "", wxDefaultPosition, wxSize(220, -1));
    itemFlexGridSizer3->Add(m_currencyName, g_flags);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Currency Symbol")), g_flags);
    m_currencySymbol = new mmTextCtrl(this, wxID_ANY
        , "", wxDefaultPosition, wxSize(220, -1));
    m_currencySymbol->SetMaxLength(3);
    itemFlexGridSizer3->Add(m_currencySymbol, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Unit Name")), g_flags);
    unitTx_ = new wxTextCtrl(this, ID_DIALOG_CURRENCY_TEXT_UNIT, "");
    itemFlexGridSizer3->Add(unitTx_, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Cents Name")), g_flags);
    centTx_ = new wxTextCtrl(this, ID_DIALOG_CURRENCY_TEXT_CENTS, "");
    itemFlexGridSizer3->Add(centTx_, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Prefix Symbol")), g_flags);
    pfxTx_ = new wxTextCtrl(this, ID_DIALOG_CURRENCY_TEXT_PFX, "");
    itemFlexGridSizer3->Add(pfxTx_, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Suffix Symbol")), g_flags);
    sfxTx_ = new wxTextCtrl(this, ID_DIALOG_CURRENCY_TEXT_SFX, "");
    itemFlexGridSizer3->Add(sfxTx_, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Decimal Char")), g_flags);
    decTx_ = new wxTextCtrl(this, ID_DIALOG_CURRENCY_TEXT_DECIMAL, "");
    itemFlexGridSizer3->Add(decTx_, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Grouping Char")), g_flags);
    grpTx_ = new wxTextCtrl(this, ID_DIALOG_CURRENCY_TEXT_GROUP, "");
    itemFlexGridSizer3->Add(grpTx_, g_flagsExpand);

    wxIntegerValidator<int> valInt(&scale_,
        wxNUM_VAL_THOUSANDS_SEPARATOR | wxNUM_VAL_ZERO_AS_BLANK);
    valInt.SetMin(0); // Only allow positive numbers
    valInt.SetMax(6);
    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Scale")), g_flags);
    scaleTx_ = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize
        , wxALIGN_RIGHT | wxTE_PROCESS_ENTER, valInt);
    itemFlexGridSizer3->Add(scaleTx_, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Conversion to Base Rate")), g_flags);
    baseConvRate_ = new mmTextCtrl(this, ID_DIALOG_CURRENCY_TEXT_BASECONVRATE, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    itemFlexGridSizer3->Add(baseConvRate_, g_flagsExpand);
    baseConvRate_->SetToolTip(_("Other currency conversion rate. Set Base Currency to 1."));

    //--------------------------
    wxStaticBox* itemStaticBox_02 = new wxStaticBox(this, wxID_ANY, _("Base Rate Conversion Sample:"));
    wxStaticBoxSizer* itemStaticBoxSizer_02 = new wxStaticBoxSizer(itemStaticBox_02, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer_02, g_flagsExpand);

    baseRateSample_ = new wxStaticText(this, wxID_STATIC, "");
    itemStaticBoxSizer_02->Add(baseRateSample_, g_flags);

    //--------------------------
    wxStaticBox* itemStaticBox_01 = new wxStaticBox(this, wxID_STATIC, _("Value Display Sample:"));
    wxStaticBoxSizer* itemStaticBoxSizer_01 = new wxStaticBoxSizer(itemStaticBox_01, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer_01, g_flagsExpand);

    sampleText_ = new wxStaticText(this, wxID_STATIC, "");
    itemStaticBoxSizer_01->Add(sampleText_, g_flags);

    //--------------------------
    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22, wxSizerFlags(g_flags).Centre());

    wxButton* itemButton24 = new wxButton( this, wxID_REFRESH, _("&Update"));
    itemBoxSizer22->Add(itemButton24, g_flags);
    itemButton24->SetToolTip(_("Save any changes made"));

    wxButton* itemButton25 = new wxButton( this, wxID_CANCEL, _("&Close "));
    itemBoxSizer22->Add(itemButton25, g_flags);
    itemButton25->SetToolTip(_("Any changes will be lost without update"));
}

void mmCurrencyDialog::OnUpdate(wxCommandEvent& /*event*/)
{
    double baseConvRate;
    if (!baseConvRate_->checkValue(baseConvRate))
        return;

    wxString name = m_currencyName->GetValue();
    if (name.empty()) return;

    if (!m_currency)
    {
        m_currency = Model_Currency::instance().create();
    }
    else
    {
        const auto &currencies = Model_Currency::instance().find(Model_Currency::CURRENCYNAME(name));
        if (!currencies.empty() && m_currency->CURRENCYID == -1)
        {
            wxMessageBox(_("Currency with same name exists"), _("Organize Currency: Add Currency"), wxOK | wxICON_ERROR);
            return;
        }
    }

    int scal = wxAtoi(scaleTx_->GetValue());

    m_currency->PFX_SYMBOL = pfxTx_->GetValue();
    m_currency->SFX_SYMBOL = sfxTx_->GetValue();
    m_currency->DECIMAL_POINT = decTx_->GetValue();
    m_currency->GROUP_SEPARATOR =  grpTx_->GetValue();
    m_currency->UNIT_NAME = unitTx_->GetValue();
    m_currency->CENT_NAME = centTx_->GetValue();
    m_currency->SCALE = static_cast<int>(pow(10,scal));
    m_currency->BASECONVRATE = baseConvRate;
    m_currency->CURRENCY_SYMBOL = m_currencySymbol->GetValue().Trim();
    m_currency->CURRENCYNAME = m_currencyName->GetValue();

    Model_Currency::instance().save(m_currency);
    fillControls();
}

void mmCurrencyDialog::onTextEntered(wxCommandEvent& event)
{
    if (event.GetId() == baseConvRate_->GetId())
    {
        baseConvRate_->Calculate(Model_Currency::GetBaseCurrency(), 4);
    }
}
