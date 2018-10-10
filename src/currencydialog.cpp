/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 Gabriele-V
 Copyright (C) 2013-2017 Nikolay Akimov

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
#include "Model_Currency.h"
#include "defs.h"
#include "paths.h"
#include "validators.h"
#include "mmTextCtrl.h"
#include "mmSimpleDialogs.h"
#include "option.h"

#include <wx/combobox.h>
#include <wx/valnum.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmCurrencyDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmCurrencyDialog, wxDialog)
EVT_BUTTON(wxID_OK, mmCurrencyDialog::OnOk)
EVT_BUTTON(wxID_CANCEL, mmCurrencyDialog::OnCancel)
EVT_TEXT(ID_DIALOG_CURRENCY, mmCurrencyDialog::OnTextChanged)
EVT_CHECKBOX(ID_DIALOG_CURRENCY, mmCurrencyDialog::OnTextChanged)
wxEND_EVENT_TABLE()

static const int SCALE = 9;

mmCurrencyDialog::mmCurrencyDialog()
{
}

mmCurrencyDialog::~mmCurrencyDialog()
{
}

mmCurrencyDialog::mmCurrencyDialog(wxWindow* parent
    , const Model_Currency::Data * currency)
        : m_scale(SCALE)
        , m_currencyName(nullptr)
        , m_sample_text(nullptr)
        , m_currencySymbol(nullptr)
        , pfxTx_(nullptr)
        , sfxTx_(nullptr)
        , decTx_(nullptr)
        , grpTx_(nullptr)
        , scaleTx_(nullptr)
        , m_historic(nullptr)

{

    if (currency)
    {
        m_currency = Model_Currency::instance().clone(currency);
        m_currency->CURRENCYID = currency->CURRENCYID;
    }
    else
    {
        m_currency = Model_Currency::instance().create();
        m_currency->SCALE = 2;
    }

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
        mmSingleChoiceDialog select_currency_name(this, _("Currency Name"), _("Select Currency"), Model_Currency::instance().all_currency_names());
        if (select_currency_name.ShowModal() == wxID_OK)
        {
            const wxString currencyname = select_currency_name.GetStringSelection();
            m_currency = Model_Currency::instance().get_one(Model_Currency::CURRENCYNAME(currencyname));
        }
    }

    SetEvtHandlerEnabled(false);
    fillControls();
    SetEvtHandlerEnabled(true);

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmex::getProgramIcon());

    Centre();
    return TRUE;
}

void mmCurrencyDialog::fillControls()
{
    if (m_currency)
    {
        m_currencyType->SetStringSelection(wxGetTranslation(m_currency->CURRENCY_TYPE));

        m_currencyName->ChangeValue(m_currency->CURRENCYNAME);
        m_currencySymbol->ChangeValue(m_currency->CURRENCY_SYMBOL);

        pfxTx_->ChangeValue(m_currency->PFX_SYMBOL);
        sfxTx_->ChangeValue(m_currency->SFX_SYMBOL);

        const wxString& decimal_separator = wxString::FromAscii(wxNumberFormatter::GetDecimalSeparator());
        const wxString& ds = m_currency->DECIMAL_POINT.empty() ? decimal_separator : m_currency->DECIMAL_POINT;
        decTx_->ChangeValue(ds);
        grpTx_->ChangeValue(m_currency->GROUP_SEPARATOR);

        m_scale = log10(m_currency->SCALE);
        const wxString& scale_value = wxString::Format("%i", m_scale);
        scaleTx_->ChangeValue(scale_value);
        m_currencySymbol->ChangeValue(m_currency->CURRENCY_SYMBOL);
        m_historic->SetValue(Model_Currency::BoolOf(m_currency->HISTORIC));
    }
}

void mmCurrencyDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer3->AddGrowableCol(1);
    itemBoxSizer2->Add(itemFlexGridSizer3, g_flagsV);

    //--------------------------
    wxStaticText* type_label = new wxStaticText(this, wxID_STATIC, _("Currency Type"));
    itemFlexGridSizer3->Add(type_label, g_flagsH);
    type_label->SetFont(this->GetFont().Bold());
    m_currencyType = new wxChoice(this, ID_DIALOG_CURRENCY, wxDefaultPosition, wxSize(220, -1));
    for (const auto& type : Model_Currency::instance().all_currency_types())
        m_currencyType->Append(wxGetTranslation(type), new wxStringClientData(type));
    itemFlexGridSizer3->Add(m_currencyType, g_flagsH);

    wxStaticText* name_label = new wxStaticText(this, wxID_STATIC, _("Currency Name"));
    itemFlexGridSizer3->Add(name_label, g_flagsH);
    name_label->SetFont(this->GetFont().Bold());
    m_currencyName = new mmTextCtrl(this, ID_DIALOG_CURRENCY, "", wxDefaultPosition, wxSize(220, -1));
    itemFlexGridSizer3->Add(m_currencyName, g_flagsH);

    wxStaticText* symbol_label = new wxStaticText(this, wxID_STATIC, _("Currency Symbol"));
    itemFlexGridSizer3->Add(symbol_label, g_flagsH);
    symbol_label->SetFont(this->GetFont().Bold());
    m_currencySymbol = new mmTextCtrl(this, ID_DIALOG_CURRENCY, "", wxDefaultPosition, wxSize(220, -1));
    m_currencySymbol->SetMaxLength(3);
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

    wxStaticText* decimal_label = new wxStaticText(this, wxID_STATIC, _("Decimal Char"));
    itemFlexGridSizer3->Add(decimal_label, g_flagsH);
    decimal_label->SetFont(this->GetFont().Bold());
    decTx_ = new wxTextCtrl(this, ID_DIALOG_CURRENCY, "", wxDefaultPosition, wxDefaultSize, 0L, valid);
    decTx_->SetMaxLength(1);
    itemFlexGridSizer3->Add(decTx_, g_flagsExpand);

    chars.Add(" ");
    valid.SetIncludes(chars);
    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Grouping Char")), g_flagsH);
    grpTx_ = new wxTextCtrl(this, ID_DIALOG_CURRENCY, "", wxDefaultPosition, wxDefaultSize, 0L, valid);
    grpTx_->SetMaxLength(1);
    itemFlexGridSizer3->Add(grpTx_, g_flagsExpand);

    wxIntegerValidator<int> valInt(&m_scale, wxNUM_VAL_THOUSANDS_SEPARATOR | wxNUM_VAL_ZERO_AS_BLANK);
    valInt.SetMin(0); // Only allow positive numbers
    valInt.SetMax(SCALE);
    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Scale")), g_flagsH);
    scaleTx_ = new wxTextCtrl(this, ID_DIALOG_CURRENCY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, valInt);
    itemFlexGridSizer3->Add(scaleTx_, g_flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText(this, wxID_STATIC, _("Historic currency")), g_flagsH);
    m_historic = new wxCheckBox(this, ID_DIALOG_CURRENCY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxCHK_2STATE);
    itemFlexGridSizer3->Add(m_historic, g_flagsExpand);

    //--------------------------
    wxStaticBox* sb01 = new wxStaticBox(this, wxID_STATIC, _("Value Display Sample:"));
    wxStaticBoxSizer* sbs01 = new wxStaticBoxSizer(sb01, wxHORIZONTAL);
    itemBoxSizer2->Add(sbs01, g_flagsExpand);

    m_sample_text = new wxStaticText(this, wxID_STATIC, "");
    sbs01->Add(m_sample_text, g_flagsH);

    //--------------------------
    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22, wxSizerFlags(g_flagsV).Centre());

    wxButton* itemButton24 = new wxButton(this, wxID_OK, _("&OK "));
    itemBoxSizer22->Add(itemButton24, g_flagsH);
    itemButton24->SetToolTip(_("Save any changes made"));

    wxButton* itemButton25 = new wxButton(this, wxID_CANCEL, wxGetTranslation(g_CloseLabel));
    itemBoxSizer22->Add(itemButton25, g_flagsH);
    itemButton25->SetToolTip(_("Any changes will be lost without update"));
}

void mmCurrencyDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    const int type = m_currencyType->GetSelection();
    if (type == -1)
        return mmErrorDialogs::InvalidChoice(m_currencyType);

    const wxString name = m_currencyName->GetValue().Trim();
    if (name.empty())
        return mmErrorDialogs::InvalidName(m_currencyName);

    const wxString symbol = m_currencySymbol->GetValue().Trim();
    if (name.empty() || symbol.empty())
        return mmErrorDialogs::InvalidName(m_currencySymbol);

    const auto currency_symb = Model_Currency::instance().find(Model_Currency::CURRENCY_SYMBOL(symbol));
    if (!currency_symb.empty() && m_currency->CURRENCYID == -1)
        return mmErrorDialogs::InvalidSymbol(m_currencySymbol, true);
    
    if (m_currency->DECIMAL_POINT.empty() || !wxString(".,").Contains(m_currency->DECIMAL_POINT))
        return mmErrorDialogs::ToolTip4Object(decTx_, _("Invalid Entry"), _("Decimal Char"));

    if ((m_currency->GROUP_SEPARATOR == m_currency->DECIMAL_POINT) || (!wxString(" .,").Contains(m_currency->GROUP_SEPARATOR)))
        return mmErrorDialogs::ToolTip4Object(grpTx_, _("Invalid Entry"), _("Grouping Char"));

    Model_Currency::instance().save(m_currency);
    EndModal(wxID_OK);
}

 void mmCurrencyDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void mmCurrencyDialog::OnTextChanged(wxCommandEvent& WXUNUSED(event))
{  
    int scale = wxAtoi(scaleTx_->GetValue());
    m_currency->PFX_SYMBOL = pfxTx_->GetValue();
    m_currency->SFX_SYMBOL = sfxTx_->GetValue();
    m_currency->DECIMAL_POINT = decTx_->GetValue();
    m_currency->GROUP_SEPARATOR = grpTx_->GetValue();
    m_currency->SCALE = static_cast<int>(pow(10, scale));
    m_currency->CURRENCY_SYMBOL = m_currencySymbol->GetValue().Trim().Upper();
    m_currency->CURRENCYNAME = m_currencyName->GetValue();
    const int type_selection = m_currencyType->GetSelection();
    m_currency->CURRENCY_TYPE = type_selection != -1 ? Model_Currency::currtype_desc(type_selection) : "";
    m_currency->HISTORIC = m_historic->GetValue() ? 1 : 0;

    double base_amount = 123456.78;
    const wxString& dispAmount = wxString::Format(_("%s Shown As: %s"), wxString::FromCDouble(base_amount, 2)
        , Model_Currency::toCurrency(base_amount, m_currency));
    m_sample_text->SetLabelText(dispAmount);
}
