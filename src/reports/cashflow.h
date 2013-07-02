#ifndef _MM_EX_REPORTCASHFLOW_H_
#define _MM_EX_REPORTCASHFLOW_H_

#include "reportbase.h"

class mmGUIFrame;
class wxArrayString;
class TTransactionBillEntry;

class mmReportCashFlow : public mmPrintableBase
{
public:
    mmReportCashFlow(mmCoreDB* core, mmGUIFrame* frame, int cashflowreporttype);

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

    void SetRepeatForecast(forecastVec& fvec
                           , TTransactionBillEntry* repeat_entry, double& amount);
    void SetYearsRepeatForecast(forecastVec& fvec
                                , TTransactionBillEntry* repeat_entry, double& amount, const wxDateTime& future_year);

    mmGUIFrame* frame_;
    const wxArrayString* accountArray_;
    bool activeTermAccounts_;
    bool activeBankAccounts_;
    int cashflowreporttype_;
};

class mmReportCashFlowAllAccounts : public mmReportCashFlow
{
public:
    mmReportCashFlowAllAccounts(mmCoreDB* core, mmGUIFrame* frame);
};

class mmReportCashFlowBankAccounts : public mmReportCashFlow
{
public:
    mmReportCashFlowBankAccounts(mmCoreDB* core, mmGUIFrame* frame);
};

class mmReportCashFlowTermAccounts: public mmReportCashFlow
{
public:
    mmReportCashFlowTermAccounts(mmCoreDB* core, mmGUIFrame* frame);
};

class mmReportCashFlowSpecificAccounts: public mmReportCashFlow
{
public:
    mmReportCashFlowSpecificAccounts(mmCoreDB* core, mmGUIFrame* frame);
    virtual wxString getHTMLText();
};

class mmReportDailyCashFlowSpecificAccounts: public mmReportCashFlowSpecificAccounts
{
public:
    mmReportDailyCashFlowSpecificAccounts(mmCoreDB* core, mmGUIFrame* frame);
};

#endif // _MM_EX_REPORTCASHFLOW_H_
