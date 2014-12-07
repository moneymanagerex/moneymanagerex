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

wxString mmPrintableBase::local_title() const
{
    if (!m_date_range) 
        return m_local_title; 
    else 
        return m_local_title + _(" - ") + m_date_range->local_title();
}

mmGeneralReport::mmGeneralReport(const Model_Report::Data* report)
: mmPrintableBase(report->REPORTNAME, wxGetTranslation(report->REPORTNAME))
, m_report(report)
{
}

wxString mmGeneralReport::getHTMLText()
{
    return Model_Report::instance().get_html(this->m_report);
}

mmPrintableBaseSpecificAccounts::mmPrintableBaseSpecificAccounts(const wxString& report_name, int sort_column)
: accountArray_(0)
, reportName_(report_name)
{
}

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

    wxMultiChoiceDialog mcd(0, _("Choose Accounts"), reportName_, accounts);
    wxButton* ok = (wxButton*) mcd.FindWindow(wxID_OK);
    if (ok) ok->SetLabel(_("&OK "));
    wxButton* ca = (wxButton*) mcd.FindWindow(wxID_CANCEL);
    if (ca) ca->SetLabel(_("&Cancel "));

    if (mcd.ShowModal() == wxID_OK)
    {
        wxArrayInt arraySel = mcd.GetSelections();

        for (size_t i = 0; i < arraySel.size(); ++i)
        {
            selections->Add(accounts.Item(arraySel[i]));
        }
    }

    accountArray_ = selections;
}
