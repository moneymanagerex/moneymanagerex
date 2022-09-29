/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 Gabriele-V
 Copyright (C) 2022  Mark Whalley (mark@ipx.co.uk)

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
#include "Model_CurrencyHistory.h"
#include "defs.h"
#include "option.h"
#include "paths.h"
#include "validators.h"
#include "mmTextCtrl.h"
#include "mmSimpleDialogs.h"

#include <wx/combobox.h>
#include <wx/valnum.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmCurrencyDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmCurrencyDialog, wxDialog)
EVT_BUTTON(wxID_OK, mmCurrencyDialog::OnOk)
EVT_BUTTON(wxID_CANCEL, mmCurrencyDialog::OnCancel)
EVT_TEXT(ID_DIALOG_CURRENCY, mmCurrencyDialog::OnTextChanged)
wxEND_EVENT_TABLE()

static const int SCALE = 9;

mmCurrencyDialog::mmCurrencyDialog()
{
}

mmCurrencyDialog::~mmCurrencyDialog()
{
}

mmCurrencyDialog::mmCurrencyDialog(wxWindow* parent, const Model_Currency::Data * currency)
    : m_scale(SCALE)
    , m_currencyName(nullptr)
    , sampleText_(nullptr)
    , m_currencySymbol(nullptr)
    , pfxTx_(nullptr)
    , sfxTx_(nullptr)
    , decTx_(nullptr)
    , grpTx_(nullptr)

    , scaleTx_(nullptr)
    , baseConvRate_(nullptr)
{
    if (currency)
    {
        m_currency = Model_Currency::instance().clone(currency);
        m_currency->CURRENCYID = currency->CURRENCYID;
        m_currency->BASECONVRATE = Model_CurrencyHistory::getLastRate(m_currency->CURRENCYID);
    }
    else
    {
        m_currency = Model_Currency::instance().create();
        m_currency->BASECONVRATE = 1;
        m_currency->SCALE = 100;
        m_currency->DECIMAL_POINT = ".";
        m_currency->GROUP_SEPARATOR = ",";
        m_currency->CURRENCY_TYPE = Model_Currency::FIAT_STR;
    }

    this->SetFont(parent->GetFont());
    Create(parent);
    SetMinSize(wxSize(300, 420));
    Fit();
}

bool mmCurrencyDialog::Create(wxWindow* parent, wxWindowID id
    , const wxString& caption, const wxString& name
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    SetEvtHandlerEnabled(false);

    CreateControls();

    if (!m_currency)
    {
        mmSingleChoiceDialog select_currency_name(this, _("Currency name"), _("Select Currency")
            , Model_Currency::instance().all_currency_names());
        if (select_currency_name.ShowModal() == wxID_OK)
        {
            const wxString currencyname = select_currency_name.GetStringSelection();
            m_currency = Model_Currency::instance().get_one(Model_Currency::CURRENCYNAME(currencyname));
        }
    }

    fillControls();
    SetEvtHandlerEnabled(true);

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmex::getProgramIcon());

    Fit();
    Centre();
    return TRUE;
}

void mmCurrencyDialog::fillControls()
{
    if (m_currency)
    {
        m_currencyName->ChangeValue(m_currency->CURRENCYNAME);
        m_currencySymbol->ChangeValue(m_currency->CURRENCY_SYMBOL);

        pfxTx_->ChangeValue(m_currency->PFX_SYMBOL);
        sfxTx_->ChangeValue(m_currency->SFX_SYMBOL);
        decTx_->ChangeValue(m_currency->DECIMAL_POINT);
        grpTx_->ChangeValue(m_currency->GROUP_SEPARATOR);
        m_scale = log10(m_currency->SCALE);
        const wxString& scale_value = wxString::Format("%i", m_scale);
        scaleTx_->ChangeValue(scale_value);

        /*const wxString locale = Model_Infotable::instance().GetStringInfo("LOCALE", "");
        if (!locale.empty())
        {
            try {
                fmt::format(std::locale(locale.c_str()), "{:L}", 123);
                decTx_->Disable();
                grpTx_->Disable();
            }
            catch (...) {
                //Do nothing
            }
        }*/

        m_currencySymbol->ChangeValue(m_currency->CURRENCY_SYMBOL);

        bool baseCurrency = (Option::instance().getBaseCurrencyID() == m_currency->CURRENCYID);
        baseConvRate_->SetValue((baseCurrency ? 1.00 : m_currency->BASECONVRATE), SCALE);
        baseConvRate_->Enable(!baseCurrency);
    }
    else
    {
        baseConvRate_->SetValue(1.00, SCALE);
    }
}

void mmCurrencyDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer3->AddGrowableCol(1, 1);
    itemBoxSizer2->Add(itemFlexGridSizer3, g_flagsExpand);

    //--------------------------
    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Currency Name")), g_flagsH);
    m_currencyName = new wxTextCtrl(this, ID_DIALOG_CURRENCY);
    m_currencyName->SetMinSize(wxSize(220, -1));
    itemFlexGridSizer3->Add(m_currencyName, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Currency Symbol")), g_flagsH);
    m_currencySymbol = new wxTextCtrl(this, ID_DIALOG_CURRENCY);
    m_currencySymbol->SetMaxLength(12);
    itemFlexGridSizer3->Add(m_currencySymbol, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Prefix Symbol")), g_flagsH);
    pfxTx_ = new wxTextCtrl(this, ID_DIALOG_CURRENCY, "");
    itemFlexGridSizer3->Add(pfxTx_, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Suffix Symbol")), g_flagsH);
    sfxTx_ = new wxTextCtrl(this, ID_DIALOG_CURRENCY, "");
    itemFlexGridSizer3->Add(sfxTx_, g_flagsExpand);

    wxTextValidator valid(wxFILTER_INCLUDE_CHAR_LIST);

    wxArrayString chars;
    chars.Add(".");
    chars.Add(",");
    valid.SetIncludes(chars);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Decimal Char")), g_flagsH);
    decTx_ = new wxTextCtrl(this, ID_DIALOG_CURRENCY, "", wxDefaultPosition, wxDefaultSize, 0L, valid);
    decTx_->SetMaxLength(1);
    itemFlexGridSizer3->Add(decTx_, g_flagsExpand);

    chars.Add(" ");
    valid.SetIncludes(chars);
    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Grouping Char")), g_flagsH);
    grpTx_ = new wxTextCtrl(this, ID_DIALOG_CURRENCY, "", wxDefaultPosition, wxDefaultSize, 0L, valid);
    grpTx_->SetMaxLength(1);
    itemFlexGridSizer3->Add(grpTx_, g_flagsExpand);

    wxIntegerValidator<int> valInt(&m_scale, wxNUM_VAL_THOUSANDS_SEPARATOR);
    valInt.SetMin(0); // Only allow positive numbers
    valInt.SetMax(SCALE);
    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Scale")), g_flagsH);
    scaleTx_ = new wxTextCtrl(this, ID_DIALOG_CURRENCY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, valInt);
    itemFlexGridSizer3->Add(scaleTx_, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Conversion to Base Rate")), g_flagsH);
    baseConvRate_ = new mmTextCtrl(this, ID_DIALOG_CURRENCY_RATE, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    baseConvRate_->Connect(ID_DIALOG_CURRENCY_RATE, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmCurrencyDialog::OnTextEntered), nullptr, this);
    baseConvRate_->SetAltPrecision(SCALE);
    wxString ConvRateTooltip = wxEmptyString;
    if (Option::instance().getCurrencyHistoryEnabled())
        ConvRateTooltip = _("Conversion rate will be used in case no currency history has been found for the currency");
    else
        ConvRateTooltip = _("Fixed conversion rate");
    mmToolTip(baseConvRate_, ConvRateTooltip);
    itemFlexGridSizer3->Add(baseConvRate_, g_flagsExpand);

    //--------------------------
    wxStaticBox* itemStaticBox_01 = new wxStaticBox(this, wxID_STATIC, _("Currency Format Sample:"));
    wxStaticBoxSizer* itemStaticBoxSizer_01 = new wxStaticBoxSizer(itemStaticBox_01, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer_01, wxSizerFlags(g_flagsExpand).Proportion(0));

    sampleText_ = new wxStaticText(this, wxID_STATIC, "");
    itemStaticBoxSizer_01->Add(sampleText_, g_flagsExpand);
    sampleText_->SetMinSize(wxSize(220, -1));

    //--------------------------
    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22, wxSizerFlags(g_flagsV).Centre());

    wxButton* itemButton24 = new wxButton(this, wxID_OK, _("&OK "));
    itemBoxSizer22->Add(itemButton24, g_flagsH);
    mmToolTip(itemButton24, _("Save any changes made"));

    wxButton* itemButton25 = new wxButton(this, wxID_CANCEL, wxGetTranslation(g_CloseLabel));
    itemBoxSizer22->Add(itemButton25, g_flagsH);
    mmToolTip(itemButton25, _("Any changes will be lost without update"));
}

void mmCurrencyDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    const wxString name = m_currencyName->GetValue().Trim();
    if (name.empty())
        return mmErrorDialogs::InvalidName(m_currencyName);

    const wxString symbol = m_currencySymbol->GetValue().Trim();
    if (name.empty() || symbol.empty())
        return mmErrorDialogs::InvalidName(m_currencySymbol);

    const auto currency_symb = Model_Currency::instance().find(Model_Currency::CURRENCY_SYMBOL(symbol));
    if (!currency_symb.empty() && m_currency->CURRENCYID == -1)
        return mmErrorDialogs::InvalidSymbol(m_currencySymbol, true);

    if (m_currency->SCALE > 1)
    {
        if (m_currency->DECIMAL_POINT.empty()) {
            return mmErrorDialogs::ToolTip4Object(decTx_, _("Invalid Entry"), _("Decimal Char"));
        }

        if (m_currency->GROUP_SEPARATOR == m_currency->DECIMAL_POINT) {
            return mmErrorDialogs::ToolTip4Object(grpTx_, _("Invalid Entry"), _("Grouping Char"));
        }
    }

    if (baseConvRate_->Calculate(SCALE))
        baseConvRate_->GetDouble(m_currency->BASECONVRATE);
    if (!baseConvRate_->checkValue(m_currency->BASECONVRATE))
        return mmErrorDialogs::ToolTip4Object(baseConvRate_, _("Invalid Entry"), _("Conversion to Base Rate"));;

    Model_Currency::instance().save(m_currency);
    EndModal(wxID_OK);
}

void mmCurrencyDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmCurrencyDialog::OnTextChanged(wxCommandEvent& WXUNUSED(event))
{
    if (m_currency->SCALE > 1)
    {
        if (!Model_Infotable::instance().GetStringInfo("LOCALE", "").empty())
        {
            if (decTx_->GetValue().empty()) {
                return mmErrorDialogs::ToolTip4Object(decTx_, _("Invalid Entry"), _("Decimal Char"));
            }

            if (grpTx_->GetValue() == m_currency->DECIMAL_POINT) {
                return mmErrorDialogs::ToolTip4Object(grpTx_, _("Invalid Entry"), _("Grouping Char"));
            }
        }
    }

    int scale = wxAtoi(scaleTx_->GetValue());
    m_currency->PFX_SYMBOL = pfxTx_->GetValue();
    m_currency->SFX_SYMBOL = sfxTx_->GetValue();
    m_currency->DECIMAL_POINT = decTx_->GetValue();
    m_currency->GROUP_SEPARATOR = grpTx_->GetValue();
    m_currency->SCALE = pow10(scale);
    m_currency->CURRENCY_SYMBOL = m_currencySymbol->GetValue().Trim();
    m_currency->CURRENCYNAME = m_currencyName->GetValue();

    wxString dispAmount = "";
    double base_amount = 1234567.89;

    dispAmount = wxString::Format(_("%.2f Shown As: %s"), base_amount, Model_Currency::toCurrency(base_amount, m_currency, scale));
    if (!Model_Infotable::instance().GetStringInfo("LOCALE","").empty())
        dispAmount = dispAmount + "  " + _("(Using Locale)");
    sampleText_->SetLabelText(dispAmount);
}

void mmCurrencyDialog::OnTextEntered(wxCommandEvent& WXUNUSED(event))
{
    if (baseConvRate_->Calculate(SCALE))
        baseConvRate_->GetDouble(m_currency->BASECONVRATE);
}