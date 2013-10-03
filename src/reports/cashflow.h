#ifndef _MM_EX_REPORTCASHFLOW_H_
#define _MM_EX_REPORTCASHFLOW_H_

#include "reportbase.h"

class wxArrayString;
class TTransactionBillEntry;

class mmReportCashFlow : public mmPrintableBase
{
public:
    mmReportCashFlow(mmCoreDB* core, int cashflowreporttype);
    ~mmReportCashFlow();

    virtual wxString getHTMLText();

protected:
    wxString getHTMLText_i();
    void activateTermAccounts();
    void activateBankAccounts();
    void getSpecificAccounts();

protected:
    struct mmRepeatForecast
    {
        wxDateTime date;
        double amount;
    };

    typedef std::vector<mmRepeatForecast> forecastVec;
    std::vector<forecastVec> bdForecastVec;

    const wxArrayString* accountArray_;
    bool activeTermAccounts_;
    bool activeBankAccounts_;
    int cashflowreporttype_;
};

class mmReportCashFlowAllAccounts : public mmReportCashFlow
{
public:
    mmReportCashFlowAllAccounts(mmCoreDB* core);
};

class mmReportCashFlowBankAccounts : public mmReportCashFlow
{
public:
    mmReportCashFlowBankAccounts(mmCoreDB* core);
};

class mmReportCashFlowTermAccounts: public mmReportCashFlow
{
public:
    mmReportCashFlowTermAccounts(mmCoreDB* core);
};

class mmReportCashFlowSpecificAccounts: public mmReportCashFlow
{
public:
    mmReportCashFlowSpecificAccounts(mmCoreDB* core);
    virtual wxString getHTMLText();
};

class mmReportDailyCashFlowSpecificAccounts: public mmReportCashFlowSpecificAccounts
{
public:
    mmReportDailyCashFlowSpecificAccounts(mmCoreDB* core);
};

#endif // _MM_EX_REPORTCASHFLOW_H_
