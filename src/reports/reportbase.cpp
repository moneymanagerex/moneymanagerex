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
#include "mmSimpleDialogs.h"
#include "mmDateRange.h"
#include "model/Model_Account.h"

mmPrintableBase::mmPrintableBase(const wxString& title)
    : m_title(title)
    , m_date_range(nullptr)
    , m_initial(true)
    , m_date_selection(0)
    , m_account_selection(0)
    , m_chart_selection(0)
    , accountArray_(nullptr)
    , m_only_active(false)
    , m_settings("")
    , m_begin_date(wxDateTime::Today())
    , m_end_date(wxDateTime::Today())
{
}

mmPrintableBase::~mmPrintableBase()
{
    Document j_doc;
    if (!j_doc.Parse(m_settings.c_str()).HasParseError()) {

        if (j_doc.HasMember("ID") && j_doc["ID"].IsInt()) {
            int id = j_doc["ID"].GetInt();

            StringBuffer json_buffer;
            PrettyWriter<StringBuffer> json_writer(json_buffer);

            json_writer.StartObject();
            json_writer.Key("ID");
            json_writer.Int(id);

            if (m_date_selection)
            {
                json_writer.Key("REPORTPERIOD");
                json_writer.Int(m_date_selection);
            }

            json_writer.Key("DATE1");
            json_writer.String(m_begin_date.FormatISODate().c_str());

            json_writer.Key("DATE2");
            json_writer.String(m_end_date.FormatISODate().c_str());

            if (m_account_selection)
            {
                json_writer.Key("ACCOUNTSELECTION");
                json_writer.Int(m_account_selection);

                if (accountArray_ && !accountArray_->empty())
                {
                    json_writer.Key("ACCOUNTS");

                    json_writer.StartArray();
                    for (const auto& entry : *accountArray_)
                    {
                        json_writer.String(entry.c_str());
                    }
                    json_writer.EndArray();
                }
            }

            if (m_chart_selection)
            {
                json_writer.Key("CHART");
                json_writer.Int(m_chart_selection);
            }

            json_writer.EndObject();

            const wxString& rj_key = wxString::Format("REPORT_%d", id);
            const wxString& rj_value = json_buffer.GetString();
            Model_Infotable::instance().Set(rj_key, rj_value);
        }
    }

    if (accountArray_)
        delete accountArray_;
}

void mmPrintableBase::date_range(const mmDateRange* date_range, int selection)
{ 
    this->m_date_range = date_range; 
    this->m_date_selection = selection;
}

const wxString mmPrintableBase::getAccountNames() const
{
    wxString accountsMsg;
    if (accountArray_)
    {
        for (const auto& entry : *accountArray_) {
            accountsMsg.Append((accountsMsg.empty() ? "" : ", ") + entry);
        }
    }
    else
    {
        accountsMsg << _("All Accounts");
    }

    if (accountsMsg.empty()) {
        accountsMsg = _("None");
    }
    accountsMsg.Prepend(_("Accounts: "));
    return accountsMsg;
}

void mmPrintableBase::setAccounts(int selection, const wxString& name)
{
    if ((selection == 1) || (m_account_selection != selection))
    {
        m_account_selection = selection;
        if (accountArray_)
        {
            delete accountArray_;
            accountArray_ = nullptr;
        }

        switch (selection)
        {
        case 0: // All Accounts
            break;
        case 1: // Select Accounts
        {
            wxArrayString* accountSelections = new wxArrayString();
            Model_Account::Data_Set accounts =
                (m_only_active ? Model_Account::instance().find(Model_Account::ACCOUNTTYPE(Model_Account::all_type()[Model_Account::INVESTMENT], NOT_EQUAL)
                    , Model_Account::STATUS(Model_Account::OPEN))
                    : Model_Account::instance().find(Model_Account::ACCOUNTTYPE(Model_Account::all_type()[Model_Account::INVESTMENT], NOT_EQUAL)));
            std::stable_sort(accounts.begin(), accounts.end(), SorterByACCOUNTNAME());

            mmMultiChoiceDialog mcd(0, _("Choose Accounts"), m_title, accounts);

            if (mcd.ShowModal() == wxID_OK)
            {
                for (const auto &i : mcd.GetSelections()) {
                    accountSelections->Add(accounts.at(i).ACCOUNTNAME);
                }
            }

            accountArray_ = accountSelections;
        }
        break;
        default: // All of Account type
        {
            wxArrayString* accountSelections = new wxArrayString();
            auto accounts = Model_Account::instance().find(Model_Account::ACCOUNTTYPE(name)
                , Model_Account::STATUS(Model_Account::CLOSED, NOT_EQUAL));
            for (const auto &i : accounts) {
                accountSelections->Add(i.ACCOUNTNAME);
            }
            accountArray_ = accountSelections;
        }
        }
    }
}

//----------------------------------------------------------------------

mmGeneralReport::mmGeneralReport(const Model_Report::Data* report)
: mmPrintableBase(report->REPORTNAME)
, m_report(report)
{
}

wxString mmGeneralReport::getHTMLText()
{
    return Model_Report::instance().get_html(this->m_report);
}

int mmGeneralReport::report_parameters()
{
    int params = 0;
    const auto content = m_report->SQLCONTENT.Lower();
    if (content.Contains("&begin_date")
        || content.Contains("&end_date"))
        params |= RepParams::DATE_RANGE;
    else if (content.Contains("&single_date"))
        params |= RepParams::SINGLE_DATE;

    return params;
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
    wxButton* ok = static_cast<wxButton*>(mcd.FindWindow(wxID_OK));
    if (ok) ok->SetLabel(_("&OK "));
    wxButton* ca = static_cast<wxButton*>(mcd.FindWindow(wxID_CANCEL));
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

const wxString mmPrintableBase::getReportTitle() const
{
	wxString title = m_title;
	if (m_date_range)
	{
		if (m_date_range->title().IsEmpty())
			title += " - " + _("Custom");
		else
			title += " - " + wxGetTranslation(m_date_range->title());
	}
	return title;
}

const wxString mmPrintableBase::getFileName() const
{
	wxString file_name = getReportTitle();
	file_name.Replace(" - ", "-");
	file_name.Replace(" ", "_");
	file_name.Replace("/", "-");
	return file_name;
}
