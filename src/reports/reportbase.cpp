/*******************************************************
 Copyright (C) 2013 James Higley
 Copyright (C) 2020 Nikolay Akimov

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
#include "util.h"

mmPrintableBase::mmPrintableBase(const wxString& title)
    : m_title(title)
    , m_date_range(nullptr)
    , m_initial(true)
    , m_date_selection(0)
    , m_account_selection(0)
    , m_chart_selection(0)
    , accountArray_(nullptr)
    , m_only_active(false)
    , m_id(-1)
    , m_parameters(0)
    , m_settings("")
{
}

mmPrintableBase::~mmPrintableBase()
{
    if (accountArray_)
        delete accountArray_;
}

void mmPrintableBase::setReportParameters(int id)
{
    m_id = id;

    switch (id) {
    case MyUsage:                     m_parameters = DATE_RANGE | CHART; break;
    case MonthlySummaryofAccounts:    m_parameters = NONE; break;
    case YearlySummaryofAccounts:     m_parameters = NONE; break;
    case WheretheMoneyGoes:           m_parameters = DATE_RANGE | CHART | ACCOUNTS_LIST; break;
    case WheretheMoneyComesFrom:      m_parameters = DATE_RANGE | CHART | ACCOUNTS_LIST; break;
    case CategoriesSummary:           m_parameters = DATE_RANGE | CHART | ACCOUNTS_LIST; break;
    case CategoriesMonthly:           m_parameters = DATE_RANGE | CHART | ACCOUNTS_LIST; break;
    case Payees:                      m_parameters = DATE_RANGE | CHART; break;
    case IncomevsExpensesSummary:     m_parameters = DATE_RANGE | ACCOUNTS_LIST; break;
    case IncomevsExpensesMonthly:     m_parameters = DATE_RANGE | ACCOUNTS_LIST | CHART; break;
    case BudgetPerformance:           m_parameters = ONLY_YEARS; break;
    case BudgetCategorySummary:       m_parameters = BUDGET_DATES | CHART; break;
    case MonthlyCashFlow:             m_parameters = ACCOUNTS_LIST | CHART; break;
    case DailyCashFlow:               m_parameters = ACCOUNTS_LIST | CHART; break;
    case StocksReportPerformance:     m_parameters = DATE_RANGE; break;
    case StocksReportSummary:         m_parameters = NONE; break;
    case ForecastReport:              m_parameters = SINGLE_DATE; break;
    case BugReport:                   m_parameters = NONE; break;
    case CategoryOverTimePerformance: m_parameters = MONTHES | CHART | ACCOUNTS_LIST; break;
    default:                          m_parameters = NONE; break;
    }
}

void mmPrintableBase::setReportSettings()
{
    int ID = getReportId();
    wxLogDebug("%d - %s", ID, m_title);

    Document j_doc;
    if (ID >= 0)
    {
        StringBuffer json_buffer;
        PrettyWriter<StringBuffer> json_writer(json_buffer);

        bool isActive = false;

        json_writer.StartObject();

        if (m_parameters & DATE_RANGE)
        {
            json_writer.Key("REPORTPERIOD");
            json_writer.Int(m_date_selection);
            isActive = true;
        }

        if (m_parameters & ACCOUNTS_LIST)
        {
            isActive = true;
            json_writer.Key("ACCOUNTSELECTION");
            json_writer.Int(m_account_selection);

            if (accountArray_ && !accountArray_->empty())
            {
                json_writer.Key("ACCOUNTS");

                json_writer.StartArray();
                for (const auto& entry : *accountArray_)
                {
                    json_writer.String(entry.utf8_str());
                }
                json_writer.EndArray();
            }
        }

        if (m_parameters & CHART)
        {
            isActive = true;
            json_writer.Key("CHART");
            json_writer.Int(m_chart_selection);
        }

        json_writer.EndObject();
        if (isActive)
        {
            const wxString& rj_key = wxString::Format("REPORT_%d", ID);
            const wxString& rj_value = wxString::FromUTF8(json_buffer.GetString());
            Model_Infotable::instance().Set(rj_key, rj_value);
            m_settings = rj_value;
        }
    }
}

void mmPrintableBase::restoreReportSettings()
{
    Document j_doc;
    if (j_doc.Parse(m_settings.c_str()).HasParseError())
        return;

    if (j_doc.HasMember("REPORTPERIOD") && j_doc["REPORTPERIOD"].IsInt()) {
        m_date_selection = j_doc["REPORTPERIOD"].GetInt();
    }

    if (j_doc.HasMember("ACCOUNTSELECTION") && j_doc["ACCOUNTSELECTION"].IsInt()) {
        m_account_selection = j_doc["ACCOUNTSELECTION"].GetInt();
    }

    if (accountArray_) {
        delete accountArray_;
    }

    if (j_doc.HasMember("ACCOUNTS") && j_doc["ACCOUNTS"].IsArray()) {
        wxArrayString* accountSelections = new wxArrayString();
        const Value& a = j_doc["ACCOUNTS"].GetArray();
        for (const auto& v : a.GetArray()) {
            accountSelections->Add(v.GetString());
        }
        accountArray_ = accountSelections;
    }

    if (j_doc.HasMember("CHART") && j_doc["CHART"].IsInt()) {
        m_chart_selection = j_doc["CHART"].GetInt();
    }
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
    wxString out;
    int error = Model_Report::instance().get_html(this->m_report, out);
    if (error != 0) {
        const char* error_template = R"(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <title>Error</title>
    <style type="text/css">
    h1.error {
        left: 0;
        line-height: 50px;
        margin-top: -15px;
        position: absolute;
        text-align: center;
        top: 15%;
        width: 90%;
        font-size: 2em;
        color: #dadada;
        -webkit-text-fill-color: #dadada;
        -webkit-text-stroke-width: 1px;
        -webkit-text-stroke-color: black;
    }
    </style>
</head>
<body>
    <h1 class="error"><TMPL_VAR ERROR></h1> </body>
</html>
)";
        wxString html = error_template;
        html.Replace("<TMPL_VAR ERROR>", out);
        out.swap(html);
    }

    return out;
}
 
int mmGeneralReport::report_parameters()
{
    int params = 0;
    const auto content = m_report->SQLCONTENT.Lower();
    if (content.Contains("&begin_date")
        || content.Contains("&end_date"))
        params |= DATE_RANGE;
    else if (content.Contains("&single_date"))
        params |= SINGLE_DATE;

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
    if (m_date_range) {
        title += " - " + m_date_range->local_title();
    }
    return title;
}
