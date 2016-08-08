/*******************************************************
 Copyright (C) 2013 James Higley

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

#include "reportbase.h"
#include "constants.h"
#include "mmex.h"
#include "mmDateRange.h"
#include "model/Model_Account.h"

wxString mmPrintableBase::title() const
{
    if (!m_date_range) 
        return m_title; 
    else 
        return m_title + " - " + m_date_range->title();
}

mmGeneralReport::mmGeneralReport(const Model_Report::Data* report)
: mmPrintableBase(report->REPORTNAME)
, m_report(report)
{
}

wxString mmGeneralReport::getHTMLText()
{
    return Model_Report::instance().get_html(this->m_report);
}

mmPrintableBaseSpecificAccounts::mmPrintableBaseSpecificAccounts(const wxString& report_name, int sort_column)
: mmPrintableBase(report_name)
, accountArray_(0)
{
}

const char * mmPrintableBase::m_template = "";

mmPrintableBaseSpecificAccounts::~mmPrintableBaseSpecificAccounts()
{
    if (accountArray_)
        delete accountArray_;
}

void mmPrintableBaseSpecificAccounts::getSpecificAccounts()
{
    wxArrayString* selections = new wxArrayString();
    wxArrayString accounts;
    for (const auto &account : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        if (Model_Account::type(account) == Model_Account::INVESTMENT) continue;
        accounts.Add(account.ACCOUNTNAME);
    }

    wxMultiChoiceDialog mcd(0, _("Choose Accounts"), m_title, accounts);
    wxButton* ok = (wxButton*) mcd.FindWindow(wxID_OK);
    if (ok) ok->SetLabel(_("&OK "));
    wxButton* ca = (wxButton*) mcd.FindWindow(wxID_CANCEL);
    if (ca) ca->SetLabel(wxGetTranslation(g_CancelLabel));

    if (mcd.ShowModal() == wxID_OK)
    {
        wxArrayInt arraySel = mcd.GetSelections();

        for (size_t i = 0; i < arraySel.size(); ++i)
        {
            selections->Add(accounts.Item(arraySel[i]));
        }
    }

    if (accountArray_)
        delete accountArray_;
    accountArray_ = selections;
}

mm_html_template::mm_html_template(const wxString& arg_template): html_template(arg_template.ToStdWstring())
{
    this->load_context();
}

void mm_html_template::load_context()
{
    (*this)(L"TODAY") = wxDate::Now().FormatISODate();
    for (const auto &r: Model_Infotable::instance().all())
        (*this)(r.INFONAME.ToStdWstring()) = r.INFOVALUE;
    (*this)(L"INFOTABLE") = Model_Infotable::to_loop_t();

    const Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
    if (currency) currency->to_template(*this);
}
