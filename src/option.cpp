/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016 - 2017 Stefano Giorgio [stef145g]
 Copyright (C) 2017 James Higley

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


#include "option.h"
#include "constants.h"
#include "images_list.h"
#include "singleton.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include "model/Model_Account.h"
#include "maincurrencydialog.h"
#include "model/Model_Currency.h"
#include "model/Model_CurrencyHistory.h"
#include "reports/allreport.h"

struct ReportInfo
{
    enum Reports {
        MyUsage = 0,
        MonthlySummaryofAccounts,
        YearlySummaryofAccounts,
        WheretheMoneyGoes,
        WheretheMoneyComesFrom,
        CategoriesSummary,
        CategoriesMonthly,
        Payees,
        IncomevsExpensesSummary,
        IncomevsExpensesMonthly,
        BudgetPerformance,
        BudgetCategorySummary,
        MonthlyCashFlow,
        DailyCashFlow,
        StocksReportPerformance,
        StocksReportSummary,
        ForecastReport,
    };
    ReportInfo(wxString g, wxString n, bool t, Reports r) { group = g; name = n; type = t; id = r; }
    wxString group;
    wxString name;
    bool type;
    Reports id;
};

int ReportCompare(_wxArraywxArrayPtrVoid *first, _wxArraywxArrayPtrVoid *second)
{
    ReportInfo *f = reinterpret_cast<ReportInfo*>(*first);
    ReportInfo *s = reinterpret_cast<ReportInfo*>(*second);
    if (f->group.IsEmpty())
    {
        if (s->group.IsEmpty())
            return f->name.Cmp(s->name);
        else
            return f->name.Cmp(s->group);
    }
    else
    {
        if (s->group.IsEmpty())
            return f->group.Cmp(s->name);
        else
        {
            int r = f->group.Cmp(s->group);
            if (r == 0)
                r = f->name.Cmp(s->name);
            return r;
        }
    }
}

//----------------------------------------------------------------------------
Option::Option()
:   m_dateFormat(mmex::DEFDATEFORMAT)
    , m_language(wxLANGUAGE_UNKNOWN)
    , m_databaseUpdated(false)
    , m_budgetFinancialYears(false)
    , m_budgetIncludeTransfers(false)
    , m_budgetSetupWithoutSummaries(false)
    , m_budgetReportWithSummaries(true)
    , m_ignoreFutureTransactions(false)
    , m_transPayeeSelection(Option::NONE)
    , m_transCategorySelection(Option::NONE)
    , m_transStatusReconciled(Option::NONE)
    , m_usageStatistics(true)
    , m_transDateDefault(0)
    , m_sharePrecision(4)
    , m_html_font_size(100)
    , m_ico_size(16)
    , m_hideReport(0)
{
    m_reports.Add(new ReportInfo("", wxTRANSLATE("My Usage"), false, ReportInfo::MyUsage));
    m_reports.Add(new ReportInfo(wxTRANSLATE("Summary of Accounts"), wxTRANSLATE("Monthly"), false, ReportInfo::MonthlySummaryofAccounts));
    m_reports.Add(new ReportInfo(wxTRANSLATE("Summary of Accounts"), wxTRANSLATE("Yearly"), false, ReportInfo::YearlySummaryofAccounts));
    m_reports.Add(new ReportInfo(wxTRANSLATE("Categories"), wxTRANSLATE("Where the Money Goes"), false, ReportInfo::WheretheMoneyGoes));
    m_reports.Add(new ReportInfo(wxTRANSLATE("Categories"), wxTRANSLATE("Where the Money Comes From"), false, ReportInfo::WheretheMoneyComesFrom));
    m_reports.Add(new ReportInfo(wxTRANSLATE("Categories"), wxTRANSLATE("Summary"), false, ReportInfo::CategoriesSummary));
    m_reports.Add(new ReportInfo(wxTRANSLATE("Categories"), wxTRANSLATE("Monthly"), false, ReportInfo::CategoriesMonthly));
    m_reports.Add(new ReportInfo("", wxTRANSLATE("Payees"), false, ReportInfo::Payees));
    m_reports.Add(new ReportInfo(wxTRANSLATE("Income vs Expenses"), wxTRANSLATE("Summary"), false, ReportInfo::IncomevsExpensesSummary));
    m_reports.Add(new ReportInfo(wxTRANSLATE("Income vs Expenses"), wxTRANSLATE("Monthly"), false, ReportInfo::IncomevsExpensesMonthly));
    m_reports.Add(new ReportInfo(wxTRANSLATE("Budget"), wxTRANSLATE("Performance"), true, ReportInfo::BudgetPerformance));
    m_reports.Add(new ReportInfo(wxTRANSLATE("Budget"), wxTRANSLATE("Category Summary"), true, ReportInfo::BudgetCategorySummary));
    m_reports.Add(new ReportInfo(wxTRANSLATE("Cash Flow"), wxTRANSLATE("Monthly"), false, ReportInfo::MonthlyCashFlow));
    m_reports.Add(new ReportInfo(wxTRANSLATE("Cash Flow"), wxTRANSLATE("Daily"), false, ReportInfo::DailyCashFlow));
    m_reports.Add(new ReportInfo(wxTRANSLATE("Stocks Report"), wxTRANSLATE("Performance"), false, ReportInfo::StocksReportPerformance));
    m_reports.Add(new ReportInfo(wxTRANSLATE("Stocks Report"), wxTRANSLATE("Summary"), false, ReportInfo::StocksReportSummary));
    m_reports.Add(new ReportInfo("", wxTRANSLATE("Forecast Report"), false, ReportInfo::ForecastReport));
    //Sort by group name and report name
    m_reports.Sort(ReportCompare);
}

Option::~Option()
{
    for (int i = 0; i < ReportCount(); i++)
    {
        ReportInfo* r = reinterpret_cast<ReportInfo*>(m_reports[i]);
        delete r;
    }
}

//----------------------------------------------------------------------------
Option& Option::instance()
{
    return Singleton<Option>::instance();
}

//----------------------------------------------------------------------------
void Option::LoadOptions(bool include_infotable)
{
    if (include_infotable)
    {
        m_dateFormat = Model_Infotable::instance().GetStringInfo("DATEFORMAT", mmex::DEFDATEFORMAT);
        m_userNameString = Model_Infotable::instance().GetStringInfo("USERNAME", "");
        m_financialYearStartDayString = Model_Infotable::instance().GetStringInfo("FINANCIAL_YEAR_START_DAY", "1");
        m_financialYearStartMonthString = Model_Infotable::instance().GetStringInfo("FINANCIAL_YEAR_START_MONTH", "7");
        m_budget_days_offset = Model_Infotable::instance().GetIntInfo("BUDGET_DAYS_OFFSET", 0);
        m_sharePrecision = Model_Infotable::instance().GetIntInfo("SHARE_PRECISION", 4);
        m_baseCurrency = Model_Infotable::instance().GetIntInfo("BASECURRENCYID", -1);
        // Ensure that base currency is set for the database.
        while (m_baseCurrency < 1)
        {
            if (mmMainCurrencyDialog::Execute(m_baseCurrency))
            {
                BaseCurrency(m_baseCurrency);
                Model_CurrencyHistory::ResetCurrencyHistory();
                Model_Currency::ResetBaseConversionRates();
            }
        }
    }

    m_language = static_cast<wxLanguage>(Model_Setting::instance().GetIntSetting(LANGUAGE_PARAMETER, wxLANGUAGE_UNKNOWN));

    m_budgetFinancialYears = Model_Setting::instance().GetBoolSetting(INIDB_BUDGET_FINANCIAL_YEARS, false);
    m_budgetIncludeTransfers = Model_Setting::instance().GetBoolSetting(INIDB_BUDGET_INCLUDE_TRANSFERS, false);
    m_budgetSetupWithoutSummaries = Model_Setting::instance().GetBoolSetting(INIDB_BUDGET_SETUP_WITHOUT_SUMMARY, false);
    m_budgetReportWithSummaries = Model_Setting::instance().GetBoolSetting(INIDB_BUDGET_SUMMARY_WITHOUT_CATEG, true);
    m_ignoreFutureTransactions = Model_Setting::instance().GetBoolSetting(INIDB_IGNORE_FUTURE_TRANSACTIONS, false);

    // Read the preference as a string and convert to int
    m_transPayeeSelection = Model_Setting::instance().GetIntSetting("TRANSACTION_PAYEE_NONE", Option::NONE);

    // For the category selection, default behavior should remain that the last category used for the payee is selected.
    //  This is item 1 (0-indexed) in the list.
    m_transCategorySelection = Model_Setting::instance().GetIntSetting("TRANSACTION_CATEGORY_NONE", Option::LASTUSED);
    m_transStatusReconciled = Model_Setting::instance().GetIntSetting("TRANSACTION_STATUS_RECONCILED", Option::NONE);
    m_transDateDefault = Model_Setting::instance().GetIntSetting("TRANSACTION_DATE_DEFAULT", 0);
    m_usageStatistics = Model_Setting::instance().GetBoolSetting(INIDB_SEND_USAGE_STATS, true);

    // Windows problem on high res screens Ref Issue #478
    int default_font_size = 100;
#ifdef _WINDOWS
    default_font_size = 116;
#endif

    m_html_font_size = Model_Setting::instance().GetIntSetting("HTMLSCALE", default_font_size);
    m_ico_size = 16;
    if (m_html_font_size >= 300)
    {
        m_ico_size = 48;
    }
    else if (m_html_font_size >= 200)
    {
        m_ico_size = 32;
    }
    else if (m_html_font_size >= 150)
    {
        m_ico_size = 24;
    }

    m_hideReport = Model_Setting::instance().GetIntSetting("HIDE_REPORT", 0);
}

void Option::DateFormat(const wxString& dateformat)
{
    m_dateFormat = dateformat;
    Model_Infotable::instance().Set("DATEFORMAT", dateformat);
}

wxString Option::DateFormat()
{
    return m_dateFormat;
}

void Option::Language(wxLanguage& language)
{
    m_language = language;
    Model_Setting::instance().Set(LANGUAGE_PARAMETER, language);
}

wxLanguage Option::Language(bool get_db)
{
    if (get_db)
    {
        m_language = static_cast<wxLanguage>(Model_Setting::instance().GetIntSetting(LANGUAGE_PARAMETER, wxLANGUAGE_UNKNOWN));
    }

    return m_language;
}

wxString Option::LanguageISO6391(bool get_db)
{
    Option::Language(get_db);
    if (m_language==wxLANGUAGE_UNKNOWN)
        return wxEmptyString;
    if (m_language==wxLANGUAGE_DEFAULT)
        return wxTranslations::Get()->GetBestTranslation("mmex", wxLANGUAGE_ENGLISH_US).Left(2);
    return wxLocale::GetLanguageCanonicalName(m_language).Left(2);
}

void Option::UserName(const wxString& username)
{
    m_userNameString = username;
    Model_Infotable::instance().Set("USERNAME", username);
}

wxString Option::UserName()
{
    return m_userNameString;
}

wxString Option::FinancialYearStartDay()
{
    return m_financialYearStartDayString;
}

void Option::FinancialYearStartDay(const wxString& setting)
{
    m_financialYearStartDayString = setting;
    Model_Infotable::instance().Set("FINANCIAL_YEAR_START_DAY", setting);
}

wxString Option::FinancialYearStartMonth()
{
    return m_financialYearStartMonthString;
}

void Option::FinancialYearStartMonth(const wxString& setting)
{
    m_financialYearStartMonthString = setting;
    Model_Infotable::instance().Set("FINANCIAL_YEAR_START_MONTH", setting);
}

void Option::BaseCurrency(int base_currency_id)
{
    m_baseCurrency = base_currency_id;
    Model_Infotable::instance().Set("BASECURRENCYID", base_currency_id);
}

int Option::BaseCurrency()
{
    return m_baseCurrency;
}

void Option::DatabaseUpdated(bool value)
{
    m_databaseUpdated = value;
}

bool Option::DatabaseUpdated()
{
    return m_databaseUpdated;
}

void Option::BudgetFinancialYears(bool value)
{
    Model_Setting::instance().Set(INIDB_BUDGET_FINANCIAL_YEARS, value);
    m_budgetFinancialYears = value;
}

bool Option::BudgetFinancialYears()
{
    return m_budgetFinancialYears;
}

void Option::BudgetIncludeTransfers(bool value)
{
    Model_Setting::instance().Set(INIDB_BUDGET_INCLUDE_TRANSFERS, value);
    m_budgetIncludeTransfers = value;

}

bool Option::BudgetIncludeTransfers()
{
    return m_budgetIncludeTransfers;
}

void Option::BudgetSetupWithoutSummaries(bool value)
{
    Model_Setting::instance().Set(INIDB_BUDGET_SETUP_WITHOUT_SUMMARY, value);
    m_budgetSetupWithoutSummaries = value;
}

bool Option::BudgetSetupWithoutSummaries()
{
    return m_budgetSetupWithoutSummaries;
}

void Option::BudgetReportWithSummaries(bool value)
{
    Model_Setting::instance().Set(INIDB_BUDGET_SUMMARY_WITHOUT_CATEG, value);
    m_budgetReportWithSummaries = value;

}
bool Option::BudgetReportWithSummaries()
{
    return m_budgetReportWithSummaries;
}

void Option::IgnoreFutureTransactions(bool value)
{
    Model_Setting::instance().Set(INIDB_IGNORE_FUTURE_TRANSACTIONS, value);
    m_ignoreFutureTransactions = value;
}

bool Option::IgnoreFutureTransactions()
{
    return m_ignoreFutureTransactions;
}

void Option::TransPayeeSelection(int value)
{
    Model_Setting::instance().Set("TRANSACTION_PAYEE_NONE", value);
    m_transPayeeSelection = value;
}

int Option::TransPayeeSelection()
{
    return m_transPayeeSelection;
}

void Option::TransCategorySelection(int value)
{
    Model_Setting::instance().Set("TRANSACTION_CATEGORY_NONE", value);
    m_transCategorySelection = value;
}

int Option::TransCategorySelection()
{
    return m_transCategorySelection;
}

void Option::TransStatusReconciled(int value)
{
    Model_Setting::instance().Set("TRANSACTION_STATUS_RECONCILED", value);
    m_transStatusReconciled = value;
}

int Option::TransStatusReconciled()
{
    return m_transStatusReconciled;
}

void Option::TransDateDefault(int value)
{
    Model_Setting::instance().Set("TRANSACTION_DATE_DEFAULT", value);
    m_transDateDefault = value;
}

int Option::TransDateDefault()
{
    return m_transDateDefault;
}

void Option::SharePrecision(int value)
{
    Model_Infotable::instance().Set("SHARE_PRECISION", value);
    m_sharePrecision = value;
}

int Option::SharePrecision()
{
    return m_sharePrecision;
}

void Option::SendUsageStatistics(bool value)
{
    m_usageStatistics = value;
    Model_Setting::instance().Set(INIDB_SEND_USAGE_STATS, value);
}

bool Option::SendUsageStatistics()
{
    return m_usageStatistics;
}

void Option::HtmlFontSize(int value)
{
    Model_Setting::instance().Set("HTMLSCALE", value);
    m_html_font_size = value;
}

int Option::HtmlFontSize()
{
    return m_html_font_size;
}

void Option::BudgetDaysOffset(int value)
{
    Model_Infotable::instance().Set("BUDGET_DAYS_OFFSET", value);
    m_budget_days_offset = value;
}

int Option::BudgetDaysOffset()
{
    return m_budget_days_offset;
}

void Option::BudgetDateOffset(wxDateTime& date)
{
    if (m_budget_days_offset != 0)
        date.Add(wxDateSpan::Days(m_budget_days_offset));
}

void Option::IconSize(int value)
{
    m_ico_size = value;
}

int Option::IconSize()
{
    return m_ico_size;
}

const int Option::AccountImageId(int account_id, bool def)
{
    int max = acc_img::MAX_XPM - img::LAST_NAVTREE_PNG;
    int min = 1;
    int custom_img_id = Model_Infotable::instance().GetIntInfo(wxString::Format("ACC_IMAGE_ID_%i", account_id), 0);
    if (custom_img_id > max) custom_img_id = custom_img_id - 20; //Bug #963 fix 
    if (!def && (custom_img_id >= min && custom_img_id <= max))
        return custom_img_id + img::LAST_NAVTREE_PNG - 1;

    int selectedImage = img::SAVINGS_ACC_NORMAL_PNG; //Default value
    wxString acctStatus = VIEW_ACCOUNTS_OPEN_STR;
    Model_Account::TYPE acctType = Model_Account::CHECKING;
    bool favorite = true;

    Model_Account::Data* account = Model_Account::instance().get(account_id);
    if (account)
    {
        acctType = Model_Account::type(account);
        acctStatus = account->STATUS;
        favorite = Model_Account::FAVORITEACCT(account);
    }
    bool closed = acctStatus == "Closed";

    switch (acctType)
    {
    case (Model_Account::CHECKING) :
        if (closed) selectedImage = img::SAVINGS_ACC_CLOSED_PNG;
        else if (favorite) selectedImage = img::SAVINGS_ACC_FAVORITE_PNG;
        else selectedImage = img::SAVINGS_ACC_NORMAL_PNG;
        break;
    case (Model_Account::TERM) :
        if (closed) selectedImage = img::TERM_ACC_CLOSED_PNG;
        else if (favorite) selectedImage = img::TERM_ACC_FAVORITE_PNG;
        else  selectedImage = img::TERMACCOUNT_PNG;
        break;
    case (Model_Account::INVESTMENT) :
        if (closed) selectedImage = img::STOCK_ACC_CLOSED_PNG;
        else if (favorite) selectedImage = img::STOCK_ACC_FAVORITE_PNG;
        else  selectedImage = img::STOCK_ACC_PNG;
        break;
    case (Model_Account::CREDIT_CARD) :
        if (closed) selectedImage = img::CARD_ACC_CLOSED_PNG;
        else if (favorite)   selectedImage = img::CARD_ACC_FAVORITE_PNG;
        else   selectedImage = img::CARD_ACC_PNG;
        break;
    case (Model_Account::CASH) :
        if (closed) selectedImage = img::CASH_ACC_CLOSED_PNG;
        else if (favorite) selectedImage = img::CASH_ACC_FAVORITE_PNG;
        else selectedImage = img::CASH_ACC_NORMAL_PNG;
        break;
    case (Model_Account::LOAN) :
        if (closed) selectedImage = img::LOAN_ACC_CLOSED_PNG;
        else if (favorite) selectedImage = img::LOAN_ACC_FAVORITE_PNG;
        else selectedImage = img::LOAN_ACC_NORMAL_PNG;
        break;
    case (Model_Account::ASSET) :
        if (closed) selectedImage = img::CASH_ACC_CLOSED_PNG;
        else if (favorite) selectedImage = img::CASH_ACC_FAVORITE_PNG;
        else selectedImage = img::CASH_ACC_NORMAL_PNG;
        break;
    case (Model_Account::SHARES) :
        if (closed) selectedImage = img::LOAN_ACC_CLOSED_PNG;
        else if (favorite) selectedImage = img::LOAN_ACC_FAVORITE_PNG;
        else selectedImage = img::LOAN_ACC_NORMAL_PNG;
        break;
    default:
        wxASSERT(false);
    }
    return selectedImage;
}

void Option::HideReport(int report, bool value)
{
    if ((report >= 0) && (report < ReportCount()))
    {
        ReportInfo* r = reinterpret_cast<ReportInfo*>(m_reports[report]);
        int bitField = 1 << static_cast<int>(r->id);
        if (value)
            m_hideReport |= bitField;
        else
            m_hideReport &= ~bitField;

        Model_Setting::instance().Set("HIDE_REPORT", m_hideReport);
    }
}

bool Option::HideReport(int report)
{
    bool hideReport = false;
    if ((report >= 0) && (report < ReportCount()))
    {
        ReportInfo* r = reinterpret_cast<ReportInfo*>(m_reports[report]);
        int bitField = 1 << static_cast<int>(r->id);
        hideReport = ((m_hideReport & bitField) != 0);
    }
    return hideReport;
}

int Option::ReportCount()
{
    return static_cast<int>(m_reports.size());
}

wxString Option::ReportFullName(int report)
{
    wxString name = "";
    if ((report >= 0) && (report < ReportCount()))
    {
        ReportInfo* r = reinterpret_cast<ReportInfo*>(m_reports[report]);
        name = r->group;
        if (name.IsEmpty())
            name = r->name;
        else
            name += wxString(" (") + r->name + wxString(")");
    }
    return name;
}

wxString Option::ReportGroup(int report)
{
    wxString group = "";
    if ((report >= 0) && (report < ReportCount()))
    {
        ReportInfo* r = reinterpret_cast<ReportInfo*>(m_reports[report]);
        group = r->group;
    }
    return group;
}

wxString Option::ReportName(int report)
{
    wxString name = "";
    if ((report >= 0) && (report < ReportCount()))
    {
        ReportInfo* r = reinterpret_cast<ReportInfo*>(m_reports[report]);
        name = r->name;
    }
    return name;
}

bool Option::BudgetReport(int report)
{
    bool budget = false;
    if ((report >= 0) && (report < ReportCount()))
    {
        ReportInfo* r = reinterpret_cast<ReportInfo*>(m_reports[report]);
        budget = r->type;
    }
    return budget;
}

mmPrintableBase* Option::ReportFunction(int report)
{
    mmPrintableBase* function = nullptr;
    if ((report >= 0) && (report < ReportCount()))
    {
        ReportInfo* r = reinterpret_cast<ReportInfo*>(m_reports[report]);
        switch (r->id)
        {
        case ReportInfo::MyUsage:
            function = new mmReportMyUsage();
            break;
        case ReportInfo::MonthlySummaryofAccounts:
            function = new mmReportSummaryByDate(0);
            break;
        case ReportInfo::YearlySummaryofAccounts:
            function = new mmReportSummaryByDate(1);
            break;
        case ReportInfo::WheretheMoneyGoes:
            function = new mmReportCategoryExpensesGoes();
            break;
        case ReportInfo::WheretheMoneyComesFrom:
            function = new mmReportCategoryExpensesComes();
            break;
        case ReportInfo::CategoriesSummary:
            function = new mmReportCategoryExpensesCategories();
            break;
        case ReportInfo::CategoriesMonthly:
            function = new mmReportCategoryOverTimePerformance();
            break;
        case ReportInfo::Payees:
            function = new mmReportPayeeExpenses();
            break;
        case ReportInfo::IncomevsExpensesSummary:
            function = new mmReportIncomeExpenses();
            break;
        case ReportInfo::IncomevsExpensesMonthly:
            function = new mmReportIncomeExpensesMonthly();
            break;
        case ReportInfo::BudgetPerformance:
            function = new mmReportBudgetingPerformance();
            break;
        case ReportInfo::BudgetCategorySummary:
            function = new mmReportBudgetCategorySummary();
            break;
        case ReportInfo::MonthlyCashFlow:
            function = new mmReportCashFlow(mmReportCashFlow::MONTHLY);
            break;
        case ReportInfo::DailyCashFlow:
            function = new mmReportCashFlow(mmReportCashFlow::DAILY);
            break;
        case ReportInfo::StocksReportPerformance:
            function = new mmReportChartStocks();
            break;
        case ReportInfo::StocksReportSummary:
            function = new mmReportSummaryStocks();
            break;
        case ReportInfo::ForecastReport:
            function = new mmReportForecast();
            break;
        default:
            break;
        }
        if (function != nullptr)
            function->setSettings(ReportSettings(r->id));
    }
    return function;
}

wxString Option::ReportSettings(int id)
{
    wxString name = wxString::Format("REPORT_%d", id);
    wxString settings = Model_Infotable::instance().GetStringInfo(name, "");
    if (!(settings.StartsWith("{") && settings.EndsWith("}")))
        settings = "{}";

    json::Object o;
    o.Clear();
    if (!name.empty()) o[L"SETTINGSNAME"] = json::String(name.ToStdWstring());
    o[L"SETTINGSDATA"] = json::String(settings.ToStdWstring());

    std::wstringstream ss;
    json::Writer::Write(o, ss);
    return ss.str();
}
