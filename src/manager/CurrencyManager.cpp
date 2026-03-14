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

#include "base/defs.h"
#include <fmt/core.h>
#include <fmt/format.h>
#include <wx/combobox.h>
#include <wx/valnum.h>

#include "base/constants.h"
#include "base/paths.h"
#include "util/_util.h"
#include "util/_simple.h"
#include "util/mmTextCtrl.h"
#include "util/mmCalcValidator.h"

#include "model/CurrencyModel.h"
#include "model/CurrencyHistoryModel.h"
#include "model/PrefModel.h"

#include "CurrencyManager.h"

wxIMPLEMENT_DYNAMIC_CLASS(CurrencyManager, wxDialog);

wxBEGIN_EVENT_TABLE(CurrencyManager, wxDialog)
    EVT_BUTTON(wxID_OK,                 CurrencyManager::OnOk)
    EVT_BUTTON(wxID_CANCEL,             CurrencyManager::OnCancel)
    EVT_TEXT(ID_DIALOG_CURRENCY,        CurrencyManager::OnDataChanged)
    EVT_CHOICE(ID_DIALOG_CURRENCY,      CurrencyManager::OnDataChanged)
    EVT_RADIOBUTTON(ID_DIALOG_CURRENCY, CurrencyManager::OnDataChanged)
wxEND_EVENT_TABLE()

static const int SCALE = 9;

CurrencyManager::CurrencyManager()
{
}

CurrencyManager::~CurrencyManager()
{
}

CurrencyManager::CurrencyManager(wxWindow* parent, const CurrencyData* currency) :
    m_scale(SCALE)
{
    if (currency) {
        m_currency_d.clone_from(*currency);
        m_currency_n = &m_currency_d;
        m_currency_n->m_id             = currency->m_id;
        m_currency_n->m_base_conv_rate = CurrencyHistoryModel::instance().get_id_last_rate(m_currency_n->m_id);
    }
    else {
        m_currency_d = CurrencyData();
        m_currency_n = &m_currency_d;
        m_currency_n->m_base_conv_rate  = 1;
        m_currency_n->m_scale           = 100;
        m_currency_n->m_decimal_point   = ".";
        m_currency_n->m_group_separator = ",";
    }

    // Check if locale will be used in preference
    const wxString locale = InfoModel::instance().getString("LOCALE", "");
    m_locale_used = false;
    if (!locale.empty()) {
        try { fmt::format(std::locale(locale.c_str()), "{:L}", 123);
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
    mmThemeAutoColour(this);
}

bool CurrencyManager::Create(wxWindow* parent, wxWindowID id
    , const wxString& caption, const wxString& name
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    SetEvtHandlerEnabled(false);

    CreateControls();

    if (!m_currency_n) {
        mmSingleChoiceDialog select_currency_name(this,
            _t("Currency name"),
            _t("Select Currency"),
            CurrencyModel::instance().find_all_name_a()
        );
        if (select_currency_name.ShowModal() == wxID_OK) {
            const wxString currencyname = select_currency_name.GetStringSelection();
            m_currency_n = CurrencyModel::instance().unsafe_search_cache_n(
                CurrencyCol::CURRENCYNAME(currencyname)
            );
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

void CurrencyManager::fillControls()
{
    if (!m_currency_n) {
        w_baseConvRate->SetValue(1.00, SCALE);
        return;
    }

    w_name->ChangeValue(m_currency_n->m_name);
    w_code->ChangeValue(m_currency_n->m_symbol);

    if (m_currency_n->m_prefix_symbol.IsEmpty()) {
        w_suffix->SetValue(true);
        w_symbol->ChangeValue(m_currency_n->m_suffix_symbol);
    }
    else {
        w_prefix->SetValue(true);
        w_symbol->ChangeValue(m_currency_n->m_prefix_symbol);
    }
    unsigned int i;
    for (i = 0; i<w_decimalSep->GetCount(); i++)
        if (static_cast<wxStringClientData *>(w_decimalSep->GetClientObject(i))->GetData()
            == m_currency_n->m_decimal_point
        )
            break;
    w_decimalSep->SetSelection(i);
    for (i = 0; i<w_groupSep->GetCount(); i++)
        if (static_cast<wxStringClientData *>(w_groupSep->GetClientObject(i))->GetData()
            == m_currency_n->m_group_separator
        )
            break;
    w_groupSep->SetSelection(i);
    m_scale = log10(m_currency_n->m_scale.GetValue());
    w_decimalSep->Enable(!m_locale_used && m_scale > 0); 
    w_groupSep->Enable(!m_locale_used); 
    const wxString& scale_value = wxString::Format("%i", m_scale);
    w_scale->ChangeValue(scale_value);
    w_code->ChangeValue(m_currency_n->m_symbol);

    bool baseCurrency = (PrefModel::instance().getBaseCurrencyID() == m_currency_n->m_id);
    w_baseConvRate->SetValue((baseCurrency ? 1.00 : m_currency_n->m_base_conv_rate), SCALE);
    w_baseConvRate->Enable(!baseCurrency);
}

void CurrencyManager::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer3->AddGrowableCol(1, 1);
    itemBoxSizer2->Add(itemFlexGridSizer3, g_flagsExpand);

    //--------------------------
    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _t("Currency Name")), g_flagsH);
    w_name = new wxTextCtrl(this, ID_DIALOG_CURRENCY);
    w_name->SetMinSize(wxSize(220, -1));
    itemFlexGridSizer3->Add(w_name, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _t("Currency Code")), g_flagsH);
    w_code = new wxTextCtrl(this, ID_DIALOG_CURRENCY);
    w_code->SetMaxLength(12);
    itemFlexGridSizer3->Add(w_code, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _t("Currency Symbol")), g_flagsH);
    w_symbol = new wxTextCtrl(this, ID_DIALOG_CURRENCY, "");
    itemFlexGridSizer3->Add(w_symbol, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _t("Symbol Location")), g_flagsH);
    w_prefix = new wxRadioButton(this, ID_DIALOG_CURRENCY, _t("Prefix"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    w_suffix = new wxRadioButton(this, ID_DIALOG_CURRENCY, _t("Suffix"), wxDefaultPosition, wxDefaultSize);
    wxBoxSizer* radSizer = new wxBoxSizer(wxHORIZONTAL);
    radSizer->Add(w_prefix);
    radSizer->Add(w_suffix);
    itemFlexGridSizer3->Add(radSizer, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _t("Decimal Char:")), g_flagsH);
    w_decimalSep = new wxChoice(this, ID_DIALOG_CURRENCY);
    itemFlexGridSizer3->Add(w_decimalSep, g_flagsExpand);
    w_decimalSep->Append(_t("Dot"), new wxStringClientData("."));
    w_decimalSep->Append(_t("Comma"), new wxStringClientData(","));

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _t("Grouping Char")), g_flagsH);
    w_groupSep = new wxChoice(this, ID_DIALOG_CURRENCY);
    itemFlexGridSizer3->Add(w_groupSep, g_flagsExpand);
    w_groupSep->Append(_t("None"), new wxStringClientData(""));    
    w_groupSep->Append(_t("Dot"), new wxStringClientData("."));
    w_groupSep->Append(_t("Comma"), new wxStringClientData(","));
    w_groupSep->Append(_t("Space"), new wxStringClientData(" "));

    wxIntegerValidator<int> valInt(&m_scale, wxNUM_VAL_THOUSANDS_SEPARATOR);
    valInt.SetMin(0); // Only allow positive numbers
    valInt.SetMax(SCALE);
    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _t("Scale")), g_flagsH);
    w_scale = new wxTextCtrl(this, ID_DIALOG_CURRENCY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, valInt);
    itemFlexGridSizer3->Add(w_scale, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _t("Conversion to Base Rate")), g_flagsH);
    w_baseConvRate = new mmTextCtrl(this, ID_DIALOG_CURRENCY_RATE, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    w_baseConvRate->Connect(ID_DIALOG_CURRENCY_RATE, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(CurrencyManager::OnTextEntered), nullptr, this);
    w_baseConvRate->SetAltPrecision(SCALE);
    wxString ConvRateTooltip = wxEmptyString;
    if (PrefModel::instance().getUseCurrencyHistory())
        ConvRateTooltip = _t("Conversion rate will be used in case no currency history has been found for the currency");
    else
        ConvRateTooltip = _t("Fixed conversion rate");
    mmToolTip(w_baseConvRate, ConvRateTooltip);
    itemFlexGridSizer3->Add(w_baseConvRate, g_flagsExpand);

    //--------------------------
    wxStaticBox* itemStaticBox_01 = new wxStaticBox(this, wxID_STATIC, _t("Currency Format Sample:"));
    wxStaticBoxSizer* itemStaticBoxSizer_01 = new wxStaticBoxSizer(itemStaticBox_01, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer_01, wxSizerFlags(g_flagsExpand).Proportion(0));

    w_sampleText = new wxStaticText(this, wxID_STATIC, "");
    itemStaticBoxSizer_01->Add(w_sampleText, g_flagsExpand);
    w_sampleText->SetMinSize(wxSize(220, -1));

    //--------------------------
    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22, wxSizerFlags(g_flagsV).Centre());

    wxButton* itemButton24 = new wxButton(this, wxID_OK, _t("&OK "));
    itemBoxSizer22->Add(itemButton24, g_flagsH);
    mmToolTip(itemButton24, _t("Save any changes made"));

    wxButton* itemButton25 = new wxButton(this, wxID_CANCEL, wxGetTranslation(g_CloseLabel));
    itemBoxSizer22->Add(itemButton25, g_flagsH);
    mmToolTip(itemButton25, _t("Any changes will be lost without update"));
}

void CurrencyManager::OnOk(wxCommandEvent& WXUNUSED(event))
{
    const wxString name = w_name->GetValue().Trim();
    if (name.empty())
        return mmErrorDialogs::InvalidName(w_name);

    const wxString code = w_code->GetValue().Trim();
    if (code.empty())
        return mmErrorDialogs::InvalidName(w_code);

    const auto currency_code = CurrencyModel::instance().find(
        CurrencyCol::CURRENCY_SYMBOL(code)
    );
    if (!currency_code.empty() && m_currency_n->m_id == -1)
        return mmErrorDialogs::InvalidSymbol(w_code, true);

    if (m_currency_n->m_scale > 1)
        if (m_currency_n->m_group_separator == m_currency_n->m_decimal_point) {
            return mmErrorDialogs::ToolTip4Object(w_groupSep, _t("Invalid Entry")
                        , _t("Grouping character is unable to be the same as the decimal character"));
        }

    if (w_baseConvRate->Calculate(SCALE))
        w_baseConvRate->GetDouble(m_currency_n->m_base_conv_rate);
    if (!w_baseConvRate->checkValue(m_currency_n->m_base_conv_rate))
        return mmErrorDialogs::ToolTip4Object(w_baseConvRate, _t("Invalid Entry"), _t("Conversion to Base Rate"));;

    CurrencyModel::instance().unsafe_save_data_n(m_currency_n);
    EndModal(wxID_OK);
}

void CurrencyManager::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void CurrencyManager::OnDataChanged(wxCommandEvent& WXUNUSED(event))
{
    const wxString decimal = static_cast<wxStringClientData *>(w_decimalSep->GetClientObject(w_decimalSep->GetSelection()))->GetData();
    const wxString grouping = static_cast<wxStringClientData *>(w_groupSep->GetClientObject(w_groupSep->GetSelection()))->GetData();
    int scale = wxAtoi(w_scale->GetValue());

    if ((scale > 0) && (grouping == decimal))
        mmErrorDialogs::ToolTip4Object(w_groupSep, _t("Invalid Entry")
                , _t("Grouping character is unable to be the same as the decimal character"));

    w_decimalSep->Enable(!m_locale_used && scale > 0); 

    if (w_prefix->GetValue())
    {
        m_currency_n->m_prefix_symbol = w_symbol->GetValue();
        m_currency_n->m_suffix_symbol = "";
    } else
    {
        m_currency_n->m_prefix_symbol = "";
        m_currency_n->m_suffix_symbol = w_symbol->GetValue();  
    } 
    m_currency_n->m_decimal_point = decimal;
    m_currency_n->m_group_separator = grouping;
    m_currency_n->m_scale = pow10(scale);
    m_currency_n->m_symbol = w_code->GetValue().Trim();
    m_currency_n->m_name = w_name->GetValue();

    wxString dispAmount = "";
    double base_amount = 1234567.89;

    dispAmount = wxString::Format(_t("%.2f Shown As: %s"),
        base_amount,
        CurrencyModel::instance().toCurrency(base_amount, m_currency_n, scale)
    );
    if (m_locale_used)
        dispAmount = dispAmount + "  " + _t("(Using Locale)");
    w_sampleText->SetLabelText(dispAmount);
}

void CurrencyManager::OnTextEntered(wxCommandEvent& WXUNUSED(event))
{
    if (w_baseConvRate->Calculate(SCALE))
        w_baseConvRate->GetDouble(m_currency_n->m_base_conv_rate);
}
