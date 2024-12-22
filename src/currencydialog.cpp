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

#include <fmt/core.h>
#include <fmt/format.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmCurrencyDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmCurrencyDialog, wxDialog)
EVT_BUTTON(wxID_OK, mmCurrencyDialog::OnOk)
EVT_BUTTON(wxID_CANCEL, mmCurrencyDialog::OnCancel)
EVT_TEXT(ID_DIALOG_CURRENCY, mmCurrencyDialog::OnDataChanged)
EVT_CHOICE(ID_DIALOG_CURRENCY, mmCurrencyDialog::OnDataChanged)
EVT_RADIOBUTTON(ID_DIALOG_CURRENCY, mmCurrencyDialog::OnDataChanged)
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
        m_currency->CURRENCY_TYPE = Model_Currency::TYPE_STR_FIAT;
    }

    // Check if locale will be used in preference
    const wxString locale = Model_Infotable::instance().GetStringInfo("LOCALE", "");
    m_locale_used = false;
    if (!locale.empty())
    {
        try {
            fmt::format(std::locale(locale.c_str()), "{:L}", 123);
            m_locale_used = true;
        }
        catch (...) {
            // nothing
        }
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
    return true;
}

void mmCurrencyDialog::fillControls()
{
    if (m_currency)
    {
        mctrl_name->ChangeValue(m_currency->CURRENCYNAME);
        mctrl_code->ChangeValue(m_currency->CURRENCY_SYMBOL);

        if (m_currency->PFX_SYMBOL.IsEmpty())
        {
            mctrl_suffix->SetValue(true);
            mctrl_symbol->ChangeValue(m_currency->SFX_SYMBOL);
        } else
        {
            mctrl_prefix->SetValue(true);
            mctrl_symbol->ChangeValue(m_currency->PFX_SYMBOL);
        }
        unsigned int i;
        for (i = 0; i<mctrl_decimalSep->GetCount(); i++)
            if (static_cast<wxStringClientData *>(mctrl_decimalSep->GetClientObject(i))->GetData()
                                 == m_currency->DECIMAL_POINT)
                break;
        mctrl_decimalSep->SetSelection(i);
        for (i = 0; i<mctrl_groupSep->GetCount(); i++)
            if (static_cast<wxStringClientData *>(mctrl_groupSep->GetClientObject(i))->GetData()
                                == m_currency->GROUP_SEPARATOR)
                break;
        mctrl_groupSep->SetSelection(i);
        m_scale = log10(m_currency->SCALE.GetValue());
        mctrl_decimalSep->Enable(!m_locale_used && m_scale > 0); 
        mctrl_groupSep->Enable(!m_locale_used); 
        const wxString& scale_value = wxString::Format("%i", m_scale);
        mctrl_scale->ChangeValue(scale_value);
        mctrl_code->ChangeValue(m_currency->CURRENCY_SYMBOL);

        bool baseCurrency = (Option::instance().getBaseCurrencyID() == m_currency->CURRENCYID);
        mctrl_baseConvRate->SetValue((baseCurrency ? 1.00 : m_currency->BASECONVRATE), SCALE);
        mctrl_baseConvRate->Enable(!baseCurrency);
    }
    else
    {
        mctrl_baseConvRate->SetValue(1.00, SCALE);
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
    mctrl_name = new wxTextCtrl(this, ID_DIALOG_CURRENCY);
    mctrl_name->SetMinSize(wxSize(220, -1));
    itemFlexGridSizer3->Add(mctrl_name, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Currency Code")), g_flagsH);
    mctrl_code = new wxTextCtrl(this, ID_DIALOG_CURRENCY);
    mctrl_code->SetMaxLength(12);
    itemFlexGridSizer3->Add(mctrl_code, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Currency Symbol")), g_flagsH);
    mctrl_symbol = new wxTextCtrl(this, ID_DIALOG_CURRENCY, "");
    itemFlexGridSizer3->Add(mctrl_symbol, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Symbol Location")), g_flagsH);
    mctrl_prefix = new wxRadioButton(this, ID_DIALOG_CURRENCY, _("Prefix"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    mctrl_suffix = new wxRadioButton(this, ID_DIALOG_CURRENCY, _("Suffix"), wxDefaultPosition, wxDefaultSize);
    wxBoxSizer* radSizer = new wxBoxSizer(wxHORIZONTAL);
    radSizer->Add(mctrl_prefix);
    radSizer->Add(mctrl_suffix);
    itemFlexGridSizer3->Add(radSizer, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Decimal Char")), g_flagsH);
    mctrl_decimalSep = new wxChoice(this, ID_DIALOG_CURRENCY);
    itemFlexGridSizer3->Add(mctrl_decimalSep, g_flagsExpand);
    mctrl_decimalSep->Append(_("Dot"), new wxStringClientData("."));
    mctrl_decimalSep->Append(_("Comma"), new wxStringClientData(","));

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Grouping Char")), g_flagsH);
    mctrl_groupSep = new wxChoice(this, ID_DIALOG_CURRENCY);
    itemFlexGridSizer3->Add(mctrl_groupSep, g_flagsExpand);
    mctrl_groupSep->Append(_("None"), new wxStringClientData(""));    
    mctrl_groupSep->Append(_("Dot"), new wxStringClientData("."));
    mctrl_groupSep->Append(_("Comma"), new wxStringClientData(","));
    mctrl_groupSep->Append(_("Space"), new wxStringClientData(" "));

    wxIntegerValidator<int> valInt(&m_scale, wxNUM_VAL_THOUSANDS_SEPARATOR);
    valInt.SetMin(0); // Only allow positive numbers
    valInt.SetMax(SCALE);
    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Scale")), g_flagsH);
    mctrl_scale = new wxTextCtrl(this, ID_DIALOG_CURRENCY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, valInt);
    itemFlexGridSizer3->Add(mctrl_scale, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Conversion to Base Rate")), g_flagsH);
    mctrl_baseConvRate = new mmTextCtrl(this, ID_DIALOG_CURRENCY_RATE, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    mctrl_baseConvRate->Connect(ID_DIALOG_CURRENCY_RATE, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmCurrencyDialog::OnTextEntered), nullptr, this);
    mctrl_baseConvRate->SetAltPrecision(SCALE);
    wxString ConvRateTooltip = wxEmptyString;
    if (Option::instance().getCurrencyHistoryEnabled())
        ConvRateTooltip = _("Conversion rate will be used in case no currency history has been found for the currency");
    else
        ConvRateTooltip = _("Fixed conversion rate");
    mmToolTip(mctrl_baseConvRate, ConvRateTooltip);
    itemFlexGridSizer3->Add(mctrl_baseConvRate, g_flagsExpand);

    //--------------------------
    wxStaticBox* itemStaticBox_01 = new wxStaticBox(this, wxID_STATIC, _("Currency Format Sample:"));
    wxStaticBoxSizer* itemStaticBoxSizer_01 = new wxStaticBoxSizer(itemStaticBox_01, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer_01, wxSizerFlags(g_flagsExpand).Proportion(0));

    mctrl_sampleText = new wxStaticText(this, wxID_STATIC, "");
    itemStaticBoxSizer_01->Add(mctrl_sampleText, g_flagsExpand);
    mctrl_sampleText->SetMinSize(wxSize(220, -1));

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
    const wxString name = mctrl_name->GetValue().Trim();
    if (name.empty())
        return mmErrorDialogs::InvalidName(mctrl_name);

    const wxString code = mctrl_code->GetValue().Trim();
    if (code.empty())
        return mmErrorDialogs::InvalidName(mctrl_code);

    const auto currency_code = Model_Currency::instance().find(Model_Currency::CURRENCY_SYMBOL(code));
    if (!currency_code.empty() && m_currency->CURRENCYID == -1)
        return mmErrorDialogs::InvalidSymbol(mctrl_code, true);

    if (m_currency->SCALE > 1)
        if (m_currency->GROUP_SEPARATOR == m_currency->DECIMAL_POINT) {
            return mmErrorDialogs::ToolTip4Object(mctrl_groupSep, _("Invalid Entry")
                        , _("Grouping character is unable to be the same as the decimal character"));
        }

    if (mctrl_baseConvRate->Calculate(SCALE))
        mctrl_baseConvRate->GetDouble(m_currency->BASECONVRATE);
    if (!mctrl_baseConvRate->checkValue(m_currency->BASECONVRATE))
        return mmErrorDialogs::ToolTip4Object(mctrl_baseConvRate, _("Invalid Entry"), _("Conversion to Base Rate"));;

    Model_Currency::instance().save(m_currency);
    EndModal(wxID_OK);
}

void mmCurrencyDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmCurrencyDialog::OnDataChanged(wxCommandEvent& WXUNUSED(event))
{
    const wxString decimal = static_cast<wxStringClientData *>(mctrl_decimalSep->GetClientObject(mctrl_decimalSep->GetSelection()))->GetData();
    const wxString grouping = static_cast<wxStringClientData *>(mctrl_groupSep->GetClientObject(mctrl_groupSep->GetSelection()))->GetData();
    int scale = wxAtoi(mctrl_scale->GetValue());

    if ((scale > 0) && (grouping == decimal))
        mmErrorDialogs::ToolTip4Object(mctrl_groupSep, _("Invalid Entry")
                , _("Grouping character is unable to be the same as the decimal character"));

    mctrl_decimalSep->Enable(!m_locale_used && scale > 0); 

    if (mctrl_prefix->GetValue())
    {
        m_currency->PFX_SYMBOL = mctrl_symbol->GetValue();
        m_currency->SFX_SYMBOL = "";
    } else
    {
        m_currency->PFX_SYMBOL = "";
        m_currency->SFX_SYMBOL = mctrl_symbol->GetValue();  
    } 
    m_currency->DECIMAL_POINT = decimal;
    m_currency->GROUP_SEPARATOR = grouping;
    m_currency->SCALE = pow10(scale);
    m_currency->CURRENCY_SYMBOL = mctrl_code->GetValue().Trim();
    m_currency->CURRENCYNAME = mctrl_name->GetValue();

    wxString dispAmount = "";
    double base_amount = 1234567.89;

    dispAmount = wxString::Format(_("%.2f Shown As: %s"), base_amount, Model_Currency::toCurrency(base_amount, m_currency, scale));
    if (m_locale_used)
        dispAmount = dispAmount + "  " + _("(Using Locale)");
    mctrl_sampleText->SetLabelText(dispAmount);
}

void mmCurrencyDialog::OnTextEntered(wxCommandEvent& WXUNUSED(event))
{
    if (mctrl_baseConvRate->Calculate(SCALE))
        mctrl_baseConvRate->GetDouble(m_currency->BASECONVRATE);
}
