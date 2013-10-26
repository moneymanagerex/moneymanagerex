/*******************************************************
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
#include "mmCurrencyFormatter.h"
#include "mmCalculator.h"
#include "defs.h"
#include "paths.h"
#include "validators.h"
#include "mmtextctrl.h"

#include <wx/combobox.h>
#include <wx/valnum.h>

IMPLEMENT_DYNAMIC_CLASS( mmCurrencyDialog, wxDialog )

BEGIN_EVENT_TABLE( mmCurrencyDialog, wxDialog )
    EVT_BUTTON(wxID_REFRESH, mmCurrencyDialog::OnUpdate)
END_EVENT_TABLE()

mmCurrencyDialog::mmCurrencyDialog()
{
}

mmCurrencyDialog::~mmCurrencyDialog()
{
}

mmCurrencyDialog::mmCurrencyDialog(Model_Currency::Data* currency, wxWindow* parent)
    : m_currency(currency)
{
    long style = wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX;
    Create(parent, wxID_STATIC, _("Currency Manager"), wxDefaultPosition, wxSize(500, 300), style);
}

bool mmCurrencyDialog::Create( wxWindow* parent, wxWindowID id,
                              const wxString& caption, const wxPoint& pos,
                              const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    scale_ = Model_Currency::precision(m_currency);
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    fillControls();

    baseConvRate_->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(mmCurrencyDialog::onTextEntered), NULL, this);

    Centre();
    return TRUE;
}

void mmCurrencyDialog::fillControls()
{
    wxSortedArrayString currency_names, currency_symbols;
    for(const auto& currency : mmCurrency::currency_map())
    {
        currency_symbols.Add(currency.currencySymbol_);
        currencySymbolCombo_->Append(currency.currencySymbol_);

        currency_names.Add(currency.currencyName_);
        currencyNameCombo_->Append(currency.currencyName_);
    }
    
    currencyNameCombo_->AutoComplete(currency_names);
    currencySymbolCombo_->AutoComplete(currency_symbols);

    if (this->m_currency)
    {
        currencyNameCombo_->SetValue(this->m_currency->CURRENCYNAME);
        currencySymbolCombo_->SetValue(this->m_currency->CURRENCY_SYMBOL);
    }

    currencyNameCombo_->Connect(ID_DIALOG_CURRENCY_CHOICE, wxEVT_COMMAND_COMBOBOX_SELECTED,
        wxCommandEventHandler(mmCurrencyDialog::OnCurrencyNameSelected), NULL, this);
    currencyNameCombo_->Connect(ID_DIALOG_CURRENCY_CHOICE, wxEVT_COMMAND_TEXT_UPDATED,
        wxCommandEventHandler(mmCurrencyDialog::OnCurrencyNameSelected), NULL, this);

    wxString dispAmount = "";
    double amount = 1000;

    if (this->m_currency)
    {
        dispAmount = wxString::Format(_("%.0f Converted to: %s"), amount
            , Model_Currency::toCurrency(amount, m_currency));
        baseRateSample_->SetLabel(dispAmount);
        amount = 123456.78;
        dispAmount = wxString::Format(_("%.2f Shown As: %s"), amount
            , Model_Currency::toCurrency(amount, m_currency));
        sampleText_->SetLabel(dispAmount);

        pfxTx_->SetValue(m_currency->PFX_SYMBOL);
        sfxTx_->SetValue(m_currency->SFX_SYMBOL);
        decTx_->SetValue(m_currency->DECIMAL_POINT);
        grpTx_->SetValue(m_currency->GROUP_SEPARATOR);
        unitTx_->SetValue(m_currency->UNIT_NAME);
        centTx_->SetValue(m_currency->CENT_NAME);
        scale_ = Model_Currency::precision(m_currency);
        wxString scale_value = wxString::Format("%i", scale_);
        scaleTx_->SetValue(scale_value);
        convRate_ = m_currency->BASECONVRATE;
        baseConvRate_->SetValue(wxString::Format("%.4f", m_currency->BASECONVRATE));
        currencySymbolCombo_->SetValue(m_currency->CURRENCY_SYMBOL);
        if (m_currency->BASECONVRATE != 0.0 )
            amount = amount / m_currency->BASECONVRATE;
        else
            amount = 0.0;
    }
    else
    {
        convRate_ = 1;
        baseConvRate_->SetValue("1");
    }

    // resize the dialog window
    Fit();
}

void mmCurrencyDialog::CreateControls()
{
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Expand();
    const wxSize size = wxSize(220, -1);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer3->AddGrowableCol(1);
    itemBoxSizer2->Add(itemFlexGridSizer3, flags);

    //--------------------------
    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Currency Name")), flags);

    currencyNameCombo_ = new wxComboBox( this, ID_DIALOG_CURRENCY_CHOICE
        , "", wxDefaultPosition, wxSize(220, -1));
    itemFlexGridSizer3->Add(currencyNameCombo_, flags);
    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Currency Symbol")), flags);

    currencySymbolCombo_ = new wxComboBox( this, wxID_ANY);
    itemFlexGridSizer3->Add(currencySymbolCombo_, flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Unit Name")), flags);
    unitTx_ = new wxTextCtrl( this, ID_DIALOG_CURRENCY_TEXT_UNIT, "");
    itemFlexGridSizer3->Add(unitTx_, flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Cents Name")), flags);
    centTx_ = new wxTextCtrl( this, ID_DIALOG_CURRENCY_TEXT_CENTS, "");
    itemFlexGridSizer3->Add(centTx_, flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Prefix Symbol")), flags);
    pfxTx_ = new wxTextCtrl( this, ID_DIALOG_CURRENCY_TEXT_PFX, "");
    itemFlexGridSizer3->Add(pfxTx_, flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Suffix Symbol")), flags);
    sfxTx_ = new wxTextCtrl( this, ID_DIALOG_CURRENCY_TEXT_SFX, "");
    itemFlexGridSizer3->Add(sfxTx_, flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Decimal Char")), flags);
    decTx_ = new wxTextCtrl( this, ID_DIALOG_CURRENCY_TEXT_DECIMAL, "");
    itemFlexGridSizer3->Add(decTx_, flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Grouping Char")), flags);
    grpTx_ = new wxTextCtrl( this, ID_DIALOG_CURRENCY_TEXT_GROUP, "");
    itemFlexGridSizer3->Add(grpTx_, flagsExpand);

    wxIntegerValidator<int> valInt(&scale_,
        wxNUM_VAL_THOUSANDS_SEPARATOR | wxNUM_VAL_ZERO_AS_BLANK);
    valInt.SetMin(0); // Only allow positive numbers
    valInt.SetMax(6);
    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Scale")), flags);
    scaleTx_ = new wxTextCtrl( this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize
        , wxALIGN_RIGHT|wxTE_PROCESS_ENTER , valInt );
    itemFlexGridSizer3->Add(scaleTx_, flagsExpand);

    itemFlexGridSizer3->Add(new wxStaticText( this, wxID_STATIC, _("Conversion to Base Rate")), flags);
    baseConvRate_ = new mmTextCtrl( this, ID_DIALOG_CURRENCY_TEXT_BASECONVRATE, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmCalcValidator() );
    itemFlexGridSizer3->Add(baseConvRate_, flagsExpand);
    baseConvRate_ ->SetToolTip(_("Other currency conversion rate. Set Base Currency to 1."));

    //--------------------------
    wxStaticBox* itemStaticBox_02 = new wxStaticBox(this, wxID_ANY, _("Base Rate Conversion Sample:"));
    wxStaticBoxSizer* itemStaticBoxSizer_02 = new wxStaticBoxSizer(itemStaticBox_02, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer_02, flagsExpand);

    baseRateSample_ = new wxStaticText( this, wxID_STATIC, "");
    itemStaticBoxSizer_02->Add(baseRateSample_, flags);

    //--------------------------
    wxStaticBox* itemStaticBox_01 = new wxStaticBox(this, wxID_STATIC, _("Value Display Sample:"));
    wxStaticBoxSizer* itemStaticBoxSizer_01 = new wxStaticBoxSizer(itemStaticBox_01, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer_01, flagsExpand);

    sampleText_ = new wxStaticText( this, wxID_STATIC, "");
    itemStaticBoxSizer_01->Add(sampleText_, flags);

    //--------------------------
    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22, flags.Centre());

    wxButton* itemButton24 = new wxButton( this, wxID_REFRESH, _("&Update"));
    itemBoxSizer22->Add(itemButton24, flags);
    itemButton24->SetToolTip(_("Save any changes made"));

    wxButton* itemButton25 = new wxButton( this, wxID_CANCEL, _("&Close "));
    itemBoxSizer22->Add(itemButton25, flags);
    itemButton25->SetToolTip(_("Any changes will be lost without update"));
}

void mmCurrencyDialog::OnUpdate(wxCommandEvent& /*event*/)
{
    if (!this->m_currency) this->m_currency = Model_Currency::instance().create();

    int scal = wxAtoi(scaleTx_->GetValue());
    baseConvRate_->GetValue().ToDouble(&convRate_);

    m_currency->PFX_SYMBOL = pfxTx_->GetValue();
    m_currency->SFX_SYMBOL = sfxTx_->GetValue();
    m_currency->DECIMAL_POINT = decTx_->GetValue();
    m_currency->GROUP_SEPARATOR =  grpTx_->GetValue();
    m_currency->UNIT_NAME = unitTx_->GetValue();
    m_currency->CENT_NAME = centTx_->GetValue();
    m_currency->SCALE = static_cast<int>(pow(10,scal));
    m_currency->BASECONVRATE = convRate_;
    m_currency->CURRENCY_SYMBOL = currencySymbolCombo_->GetValue();
    m_currency->CURRENCYNAME = currencyNameCombo_->GetValue();

    Model_Currency::instance().save(m_currency);

    fillControls();
}

void mmCurrencyDialog::OnCurrencyNameSelected(wxCommandEvent& /*event*/)
{
    for (const auto& i : mmCurrency::currency_map())
    {
        if (i.currencyName_ == currencyNameCombo_->GetValue())
        {
            currencySymbolCombo_->SetValue(i.currencySymbol_);
            pfxTx_->SetValue(i.pfxSymbol_);
            sfxTx_->SetValue(i.sfxSymbol_);
            decTx_->SetValue(i.dec_);
            grpTx_->SetValue(i.grp_);
            unitTx_->SetValue(i.unit_);
            centTx_->SetValue(i.cent_);
            scaleTx_->SetValue(wxString::Format("%i", static_cast<int>(log10((double)i.scaleDl_))));
            convRate_ = i.baseConv_;
            baseConvRate_->SetValue(wxString::Format("%.4f",convRate_));
            currencySymbolCombo_->SetValue(i.currencySymbol_);
            break;
        }
    }
}

void mmCurrencyDialog::onTextEntered(wxCommandEvent& event)
{
    if (event.GetId() == baseConvRate_->GetId())
    {
        mmCalculator calc;
        if (calc.is_ok(baseConvRate_->GetValue()))
            baseConvRate_->SetValue(wxString::Format("%.4f", calc.get_result()));
        baseConvRate_->SetInsertionPoint(baseConvRate_->GetValue().Len());
    }
}
