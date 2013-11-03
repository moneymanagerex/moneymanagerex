#ifndef _MM_EX_REPORTCASHFLOW_H_
#define _MM_EX_REPORTCASHFLOW_H_

#include "reportbase.h"
#include <vector>

class mmReportCashFlow : public mmPrintableBaseSpecificAccounts
{
public:
    mmReportCashFlow(int cashflowreporttype);
    ~mmReportCashFlow();

    virtual wxString getHTMLText();

protected:
    wxString getHTMLText_i();
    void activateTermAccounts();
    void activateBankAccounts();

protected:
    struct mmRepeatForecast
    {
        wxDateTime date;
        double amount;
    };

    typedef std::vector<mmRepeatForecast> forecastVec;
    std::vector<forecastVec> bdForecastVec;

    bool activeTermAccounts_;
    bool activeBankAccounts_;
    int cashflowreporttype_;
};

class mmReportCashFlowAllAccounts : public mmReportCashFlow
{
public:
    mmReportCashFlowAllAccounts();
};

class mmReportCashFlowBankAccounts : public mmReportCashFlow
{
public:
    mmReportCashFlowBankAccounts();
};

class mmReportCashFlowTermAccounts: public mmReportCashFlow
{
public:
    mmReportCashFlowTermAccounts();
};

class mmReportCashFlowSpecificAccounts: public mmReportCashFlow
{
public:
    mmReportCashFlowSpecificAccounts();
    virtual wxString getHTMLText();
};

class mmReportDailyCashFlowSpecificAccounts: public mmReportCashFlowSpecificAccounts
{
public:
    mmReportDailyCashFlowSpecificAccounts();
};

#endif // _MM_EX_REPORTCASHFLOW_H_
