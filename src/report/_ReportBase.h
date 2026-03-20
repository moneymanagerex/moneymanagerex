/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025, 2026 Klaus Wich

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

#pragma once

#include "base/defs.h"
#include "util/mmDateRange.h"
#include "util/mmDateRange2.h"

#include "model/PrefModel.h"
#include "model/ReportModel.h"
#include "model/TrxFilter.h"

class wxString;
class wxArrayString;

class ReportBase
{
public:
    enum REPORT_ID {
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
        NONE = -1
    };

    enum PARAM_MASK
    {
        M_NONE           = 0,
        M_SINGLE_DATE    = 1,
        M_DATE_RANGE     = 2,
        M_TIME           = 4,
        M_MONTHS         = 8,
        M_BUDGET         = 16,
        M_YEAR           = 32,
        M_ACCOUNT        = 64,
        M_CHART          = 128,
        M_FORWARD_MONTHS = 256,
        M_STOCK_NAMES    = 512,
        M_GENERIC_FILTER = 1024,
        M_GENERIC_SELECTION = 2048
    };

protected:
    REPORT_ID m_report_id = REPORT_ID::NONE;
    wxString m_title;
    int m_parameters = 0;
    wxString m_settings = "";
    mmDateRange* m_date_range = nullptr;
    mmDateRange2 m_date_range2;
    int64 m_date_selection = 0;
    int m_forward_months = 24;
    wxSharedPtr<wxArrayString> m_account_a;
    wxSharedPtr<wxArrayString> m_account_selected_a;
    int m_account_selection = 0;
    bool m_only_active = false;
    int m_chart_selection = 0;
    int m_stock_selection = 0;
    wxString m_stock_name = "";
    wxString m_generic_filter = "";
    std::map<wxString, wxString> m_filter_map;
    std::map<wxString, wxString> m_selection_map;
    wxArrayString m_selections;

public:
    TrxFilter m_filter;
    static const char* m_template;

public:
    ReportBase(const wxString& title);
    virtual ~ReportBase();

public:
    virtual const wxString getTitle(bool translate = true) const;
    virtual int getParameters();
    virtual int extractParameters();
    virtual void refreshData() {}
    virtual wxString getHTMLText() = 0;

public:
    void setReportParameters(REPORT_ID report_id);
    void setReportSettings(const wxString& settings);
    void setDateRange(const mmDateRange2& date_range2);
    void setDateSelection(int64 sel);
    void setForwardMonths(int sel);
    void setAccounts(int selection, const wxString& type_name);
    void setChartSelection(int selection);
    void setStockSelection(int selection);

    REPORT_ID getReportId() const;
    const wxString getReportSettings() const;
    int64 getDateSelection() const;
    int getForwardMonths() const;
    int getAccountSelection() const;
    const wxString getAccountNames() const;
    wxString getFilterValue() const;
    int getChartSelection() const;
    int getStockSelection() const;
    void setStockName(const wxString& name);

    void saveReportSettings();
    void restoreReportSettings();
    std::map<wxString, wxString> getFilterMap() const;
    std::map<wxString, wxString>getSelectionMap() const;
};

// virtual
inline int ReportBase::getParameters() { return m_parameters; }
inline int ReportBase::extractParameters() { return m_parameters; }

// set
inline void ReportBase::setReportSettings(const wxString & settings) { m_settings = settings; }
inline void ReportBase::setDateRange(const mmDateRange2& date_range2)
{
    m_date_range2 = date_range2;
    m_date_range = new mmDateRange();
    m_date_range->start_date(date_range2.rangeStartN().value().getDateTime()
        .ResetTime()
    );
    m_date_range->end_date(date_range2.rangeEndN().value().getDateTime()
        .ResetTime().Add(wxTimeSpan(23,59,59,999))
    );
}
inline void ReportBase::setDateSelection(int64 sel) { m_date_selection = sel; }
inline void ReportBase::setForwardMonths(int sel) { m_forward_months = sel; }
inline void ReportBase::setChartSelection(int selection) { m_chart_selection = selection; }
inline void ReportBase::setStockSelection(int selection) { m_stock_selection = selection; }
inline void ReportBase::setStockName(const wxString& name) { m_stock_name = name; }



// get
inline ReportBase::REPORT_ID ReportBase::getReportId() const { return m_report_id; }
inline const wxString ReportBase::getReportSettings() const { return m_settings; }
inline int64 ReportBase::getDateSelection() const { return this->m_date_selection; }
inline int ReportBase::getForwardMonths() const { return this->m_forward_months; }
inline int ReportBase::getAccountSelection() const { return this->m_account_selection; }
inline int ReportBase::getChartSelection() const { return this->m_chart_selection; }
inline int ReportBase::getStockSelection() const { return this->m_stock_selection; }
inline wxString ReportBase::getFilterValue() const { return this->m_generic_filter; }
inline std::map<wxString, wxString> ReportBase::getFilterMap() const { return this->m_filter_map; }
inline std::map<wxString, wxString> ReportBase::getSelectionMap() const { return this->m_selection_map; }


class mmGeneralReport : public ReportBase
{
public:
    explicit mmGeneralReport(const ReportData* report);

public:
    wxString getHTMLText();
    virtual int extractParameters();
    std::map<wxString, wxString> extractVarDetails(const wxString& input, const wxString& marker);

private:
    const ReportData* m_report;
};

#include <html_template.h>
class mm_html_template: public html_template
{
public:
    explicit mm_html_template(const wxString & arg_template);

private:
    void load_context();
};
