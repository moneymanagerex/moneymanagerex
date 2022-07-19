/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)

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

//----------------------------------------------------------------------------
#ifndef MM_EX_REPORTBASE_H_
#define MM_EX_REPORTBASE_H_
//----------------------------------------------------------------------------
#include "filtertrans.h"
#include "mmDateRange.h"
#include "option.h"
#include "model/Model_Report.h"
class wxString;
class wxArrayString;
//----------------------------------------------------------------------------

class mmPrintableBase
{
public:
    mmPrintableBase(const wxString& title);
    virtual ~mmPrintableBase();
    virtual wxString getHTMLText() = 0;
    virtual void RefreshData() {}
    virtual const wxString getReportTitle(bool translate = true) const;
    virtual int report_parameters();
    int getReportId() { return m_id; }
    void date_range(const mmDateRange* date_range, int selection);
    void initial_report(bool initial);

    int getDateSelection() const;
    int getAccountSelection() const;
    int getChartSelection() const;
    int getForwardMonths() const;   
    const wxString getAccountNames() const;
    void chart(int selection);
    void setAccounts(int selection, const wxString& name);
    void setSelection(int sel);
    void setForwardMonths(int sel);
    void setReportSettings();
    void setReportParameters(int id);
    const wxString getReportSettings() const;
    void restoreReportSettings();
    void initReportSettings(const wxString& settings);

public:
    mmFilterTransactions m_filter;
    static const char * m_template;
    enum RepParams
    {
        NONE = 0
        , SINGLE_DATE = 1
        , DATE_RANGE = 2
        , MONTHES = 4
        , BUDGET_DATES = 8
        , ONLY_YEARS = 16
        , ACCOUNTS_LIST = 32
        , CHART = 64
        , FORWARD_MONTHS = 128
    };

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
        TransactionsCashFlow,
        StocksReportPerformance,
        StocksReportSummary,
        ForecastReport,
        BugReport,
        CategoryOverTimePerformance,
        UNUSED = -1
    };

protected:
    int m_chart_selection;
    int m_date_selection;
    int m_forward_months;
    wxString m_title;
    const mmDateRange* m_date_range;
    wxSharedPtr<wxArrayString> accountArray_;
    wxSharedPtr<wxArrayString> selectedAccountArray_;
    bool m_only_active;

private:
    bool m_initial;
    int m_account_selection;
    int m_id;
    int m_parameters;
    wxString m_settings;
};

inline void mmPrintableBase::setSelection(int sel) { m_date_selection = sel; }
inline int mmPrintableBase::getDateSelection() const { return this->m_date_selection; }
inline void mmPrintableBase::setForwardMonths(int sel) { m_forward_months = sel; }
inline int mmPrintableBase::getForwardMonths() const { return this->m_forward_months; }
inline int mmPrintableBase::getAccountSelection() const { return this->m_account_selection; }
inline int mmPrintableBase::getChartSelection() const { return this->m_chart_selection; }
inline void mmPrintableBase::chart(int selection) { m_chart_selection = selection; }
inline void mmPrintableBase::initial_report(bool initial) { m_initial = initial; }
inline  int mmPrintableBase::report_parameters() { return m_parameters; }
inline  const wxString mmPrintableBase::getReportSettings() const { return m_settings; }
inline void mmPrintableBase::initReportSettings(const wxString & settings) { m_settings = settings; }

class mmGeneralReport : public mmPrintableBase
{
public:
    explicit mmGeneralReport(const Model_Report::Data* report);

public:
    wxString getHTMLText();
    virtual int report_parameters();

private:
    const Model_Report::Data* m_report;
};

#include "html_template.h"
class mm_html_template: public html_template
{
public:
    explicit mm_html_template(const wxString & arg_template);

private:
    void load_context();
};

//----------------------------------------------------------------------------
#endif // MM_EX_REPORTBASE_H_
