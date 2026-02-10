/*******************************************************
 Copyright (C) 2013 James Higley
 Copyright (C) 2020 Nikolay Akimov
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 Klaus Wich

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
#include "model/AccountModel.h"
#include "util/util.h"

ReportBase::ReportBase(const wxString& title)
    : m_title(title)
{
}

ReportBase::~ReportBase()
{
}

const wxString ReportBase::getTitle(bool translate) const
{
    wxString title = getTranslation(translate, m_title);
    if (!m_date_range2.rangeName().empty()) {
        // range name is already translated
        title += " - " + m_date_range2.rangeName();
    }
    return title;
}

void ReportBase::setReportParameters(ReportBase::REPORT_ID report_id)
{
    m_report_id = report_id;

    switch (report_id) {
    case MyUsage:                     m_parameters = M_DATE_RANGE | M_CHART; break;
    case MonthlySummaryofAccounts:    m_parameters = M_CHART; break;
    case YearlySummaryofAccounts:     m_parameters = M_CHART; break;
    case WheretheMoneyGoes:           m_parameters = M_DATE_RANGE | M_ACCOUNT | M_CHART; break;
    case WheretheMoneyComesFrom:      m_parameters = M_DATE_RANGE | M_ACCOUNT | M_CHART; break;
    case CategoriesSummary:           m_parameters = M_DATE_RANGE | M_ACCOUNT | M_CHART; break;
    case CategoriesMonthly:           m_parameters = M_DATE_RANGE | M_ACCOUNT | M_CHART; break;
    case Payees:                      m_parameters = M_DATE_RANGE | M_CHART; break;
    case IncomevsExpensesSummary:     m_parameters = M_DATE_RANGE | M_ACCOUNT; break;
    case IncomevsExpensesMonthly:     m_parameters = M_DATE_RANGE | M_ACCOUNT | M_CHART; break;
    case BudgetPerformance:           m_parameters = M_BUDGET | M_ACCOUNT; break;
    case BudgetCategorySummary:       m_parameters = M_BUDGET | M_CHART; break;
    case MonthlyCashFlow:             m_parameters = M_FORWARD_MONTHS | M_ACCOUNT | M_CHART; break;
    case DailyCashFlow:               m_parameters = M_FORWARD_MONTHS | M_ACCOUNT | M_CHART; break;
    case TransactionsCashFlow:        m_parameters = M_FORWARD_MONTHS | M_ACCOUNT | M_CHART; break;
    case StocksReportPerformance:     m_parameters = M_DATE_RANGE; break;
    case StocksReportSummary:         m_parameters = M_NONE; break;
    case ForecastReport:              m_parameters = M_DATE_RANGE; break;
    case BugReport:                   m_parameters = M_NONE; break;
    case CategoryOverTimePerformance: m_parameters = M_MONTHS | M_ACCOUNT | M_CHART; break;
    default:                          m_parameters = M_NONE; break;
    }
}

void ReportBase::setAccounts(int selection, const wxString& type_name)
{
    if (selection != 1 && m_account_selection == selection)
        return;

    m_account_selection = selection;

    switch (selection)
    {
    case 0: // All Accounts
        m_account_a = nullptr;
        break;
    case 1: // Select Accounts
    {
        wxArrayString accounts;
        auto a = AccountModel::instance().all();
        std::stable_sort(a.begin(), a.end(), SorterByACCOUNTNAME());
        for (const auto& item : a) {
            if (m_only_active && item.STATUS != AccountModel::STATUS_NAME_OPEN)
                continue;
            accounts.Add(item.ACCOUNTNAME);
        }

        auto parent = wxWindow::FindWindowById(mmID_REPORTS);
        mmMultiChoiceDialog mcd(parent ? parent : 0,
            _t("Choose Accounts"), wxGetTranslation(m_title), accounts
        );

        if (m_account_selected_a && !m_account_selected_a->IsEmpty()) {
            wxArrayInt selected;
            int i = 0;
            for (const auto &account : accounts) {
                if (wxNOT_FOUND != m_account_selected_a->Index(account))
                    selected.Add(i);
                ++i;
            }
            mcd.SetSelections(selected);
        }

        if (mcd.ShowModal() == wxID_OK) {
            wxArrayString* accountSelections = new wxArrayString();
            for (const auto &i : mcd.GetSelections()) {
                accountSelections->Add(accounts[i]);
            }
            m_account_selected_a = m_account_a = accountSelections;
        }
    }
    break;
    default: // All of Account type
    {
        wxArrayString* accountSelections = new wxArrayString();
        auto accounts = AccountModel::instance().find(
            AccountModel::ACCOUNTTYPE(type_name),
            AccountModel::STATUS(AccountModel::STATUS_ID_CLOSED, NOT_EQUAL)
        );
        for (const auto &i : accounts) {
            accountSelections->Add(i.ACCOUNTNAME);
        }
        m_account_a = accountSelections;
    } }
}

const wxString ReportBase::getAccountNames() const
{
    wxString accountsMsg;
    if (m_account_a) {
        for (const auto& entry : *m_account_a) {
            accountsMsg.Append((accountsMsg.empty() ? "" : ", ") + entry);
        }
    }
    else {
        accountsMsg << _t("All Accounts");
    }

    if (accountsMsg.empty()) {
        accountsMsg = _t("None");
    }
    accountsMsg.Prepend(_t("Accounts: "));
    return accountsMsg;
}

void ReportBase::saveReportSettings()
{
    REPORT_ID report_id = getReportId();
    wxLogDebug("%d - %s", report_id, m_title);
    if (report_id < 0)
        return;

    Document j_doc;
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    bool isActive = false;

    json_writer.StartObject();

    if ((m_parameters & M_DATE_RANGE) || (m_parameters & M_BUDGET)) {
        isActive = true;
        json_writer.Key("REPORTPERIOD");
        json_writer.Int64(m_date_selection.GetValue());
    }

    if (m_parameters & M_ACCOUNT) {
        isActive = true;
        json_writer.Key("ACCOUNTSELECTION");
        json_writer.Int(m_account_selection);

        if (m_account_selected_a && !m_account_selected_a->IsEmpty()) {
            json_writer.Key("ACCOUNTS");
            json_writer.StartArray();
            for (const auto& entry : *m_account_selected_a) {
                json_writer.String(entry.utf8_str());
            }
            json_writer.EndArray();
        }
    }

    if (m_parameters & M_CHART) {
        isActive = true;
        json_writer.Key("CHART");
        json_writer.Int(m_chart_selection);
    }

    if (m_parameters & M_FORWARD_MONTHS) {
        isActive = true;
        json_writer.Key("FORWARDMONTHS");
        json_writer.Int(m_forward_months);
    }

    json_writer.EndObject();

    if (isActive) {
        const wxString& rj_key = wxString::Format("REPORT_%d", report_id);
        const wxString& rj_value = wxString::FromUTF8(json_buffer.GetString());
        InfotableModel::instance().setString(rj_key, rj_value);
        m_settings = rj_value;
    }
}

void ReportBase::restoreReportSettings()
{
    Document j_doc;
    if (j_doc.Parse(m_settings.c_str()).HasParseError())
        return;

    if (j_doc.HasMember("REPORTPERIOD") && j_doc["REPORTPERIOD"].IsInt()) {
        m_date_selection = j_doc["REPORTPERIOD"].GetInt();
    }

    if (j_doc.HasMember("CHART") && j_doc["CHART"].IsInt()) {
        m_chart_selection = j_doc["CHART"].GetInt();
    }

    if (j_doc.HasMember("FORWARDMONTHS") && j_doc["FORWARDMONTHS"].IsInt()) {
        m_forward_months = j_doc["FORWARDMONTHS"].GetInt();
    }


    m_account_selection = -1;
    int selection = 0;
    int acc_size = NavigatorTypes::instance().getNumberOfAccountTypes();
    if (j_doc.HasMember("ACCOUNTSELECTION") && j_doc["ACCOUNTSELECTION"].IsInt()) {
        selection = j_doc["ACCOUNTSELECTION"].GetInt();
        if (selection > (acc_size + 2)) selection = 0;
    }
    if (selection > acc_size)
        selection = 0;

    m_account_a = m_account_selected_a = nullptr;
    if (selection == 1) {
        wxArrayString* accountSelections = new wxArrayString();
        if (j_doc.HasMember("ACCOUNTS") && j_doc["ACCOUNTS"].IsArray()) {

            const Value& a = j_doc["ACCOUNTS"].GetArray();
            for (const auto& v : a.GetArray()) {
                accountSelections->Add(wxString::FromUTF8(v.GetString()));
            }
        }
        m_account_a = m_account_selected_a = accountSelections;
    }
    else if (selection > 1) {
        setAccounts(selection, NavigatorTypes::instance().type_name(selection - 2));
    }

    m_account_selection = selection;
}

//----------------------------------------------------------------------

mmGeneralReport::mmGeneralReport(const ReportModel::Data* report) :
    ReportBase(report->REPORTNAME),
    m_report(report)
{
    // Store reportid if no id is provided
    if (m_report_id == -1 && report->REPORTID >= LONG_MIN && report->REPORTID <= LONG_MAX) {
        m_report_id = static_cast<ReportBase::REPORT_ID>(int(report->REPORTID.ToLong()));
    }
}

wxString mmGeneralReport::getHTMLText()
{
    wxString out;
    int error = ReportModel::instance().get_html(this->m_report, out);
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

int mmGeneralReport::getParameters()
{
    int params = 0;
    const auto content = m_report->SQLCONTENT.Lower();
    if (content.Contains("&begin_date") || content.Contains("&end_date"))
        params |= M_DATE_RANGE;
    else if (content.Contains("&single_date"))
        params |= M_SINGLE_DATE;
    else if (content.Contains("&only_years"))
        params |= M_YEAR;

    if (content.Contains("&single_time"))
        params |= M_TIME;

    return params;
}

mm_html_template::mm_html_template(const wxString& arg_template): html_template(arg_template.ToStdWstring())
{
    this->load_context();
}

void mm_html_template::load_context()
{
    (*this)(L"TODAY") = wxDate::Now().FormatISODate();
    for (const auto &r: InfotableModel::instance().all())
        (*this)(r.INFONAME.ToStdWstring()) = r.INFOVALUE;
    (*this)(L"INFOTABLE") = InfotableModel::to_loop_t();

    const CurrencyModel::Data* currency = CurrencyModel::GetBaseCurrency();
    if (currency) currency->to_template(*this);
}
