
#include "cashflow.h"
#include "../mmex.h"
#include "../constants.h"
#include "htmlbuilder.h"
#include "mmRepeat.h"
#include "../mmCurrencyFormatter.h"
#include "../db/transactionbill.h"

mmReportCashFlow::mmReportCashFlow(mmCoreDB* core, int cashflowreporttype)
: mmPrintableBase(core)
, accountArray_(0)
, activeTermAccounts_(false)
, activeBankAccounts_(false)
, cashflowreporttype_(cashflowreporttype)
{}

void mmReportCashFlow::activateTermAccounts() 
{
    activeTermAccounts_ = true;
}

void mmReportCashFlow::activateBankAccounts() 
{
    activeBankAccounts_ = true;
}

void mmReportCashFlow::getSpecificAccounts()
{
    wxArrayString accountArray;
    wxArrayString* selections = new wxArrayString();

    for (const auto& account: core_->accountList_.accounts_) accountArray.Add(account->name_);

    wxMultiChoiceDialog mcd(0, _("Choose Accounts"), _("Cash Flow"), accountArray);
    if (mcd.ShowModal() == wxID_OK)
    {
        wxArrayInt arraySel = mcd.GetSelections();

        for (size_t i = 0; i < arraySel.size(); ++i)
        {
            selections->Add(accountArray.Item(arraySel[i]));
        }
    }

    this->accountArray_ = selections;
}

void mmReportCashFlow::SetRepeatForecast(forecastVec& fvec
, TTransactionBillEntry* repeat_entry, double& amount)
{
    mmRepeatForecast rf;
    rf.date = repeat_entry->NextOccurDate();
    rf.amount = amount;

    fvec.push_back(rf);
    repeat_entry->AdjustNextOccuranceDate();
}

void mmReportCashFlow::SetYearsRepeatForecast(forecastVec& fvec
, TTransactionBillEntry* repeat_entry, double& amount, const wxDateTime& future_year)
{
    while((repeat_entry->NextOccurDate() < future_year))
    {
        SetRepeatForecast(fvec, repeat_entry, amount);
    }
}

wxString mmReportCashFlow::getHTMLText()
{
    return this->getHTMLText_i();
}

wxString mmReportCashFlow::getHTMLText_i()
{
    core_->currencyList_.LoadBaseCurrencySettings();

    mmHTMLBuilder hb;
    hb.init();
    int years = cashflowreporttype_ == 0 ? 10: 1;// Monthly for 10 years or Daily for 1 year

    
    wxString headerMsg = wxString::Format (_("Cash Flow Forecast for %d Years Ahead"), years);
    hb.addHeader(2, headerMsg );
    headerMsg = _("Accounts: ");
    if (accountArray_ == NULL) 
    {
        if (activeBankAccounts_ && activeTermAccounts_)
            headerMsg << _("All Accounts");
        else if (activeBankAccounts_)
            headerMsg << _("All Bank Accounts");
        else if (activeTermAccounts_)
            headerMsg << _("All Term Accounts");

    } 
    else 
    {
        int arrIdx = 0;
 
        if ( (int)accountArray_->size() == 0 )
            headerMsg << "?";
        //else if ( (int)accountArray_->size() > 1 )
        //    msgString = "s" + msgString; //<-- we can't translate it to other languages

        if ( (int)accountArray_->size() > 0 )
        {
            headerMsg << accountArray_->Item(arrIdx);
            arrIdx ++;
        }
        while ( arrIdx < (int)accountArray_->size() )
        {
            headerMsg << ", " << accountArray_->Item(arrIdx);
            arrIdx ++;
        }

    }

    hb.addHeader(2, headerMsg);
    hb.addDateNow();
    hb.addLineBreak();

    hb.startCenter();

    hb.startTable("65%");
    hb.startTableRow();
    hb.addTableHeaderCell(_("Date"));
    hb.addTableHeaderCell(_("Total"), true);
    hb.addTableHeaderCell(_("Difference"), true);
    hb.endTableRow();

    double tInitialBalance = 0.0;
    std::map<wxDateTime, double> daily_balance;
          
    for (const auto& account: core_->accountList_.accounts_)
    {
        if (account->status_ == mmAccount::MMEX_Closed || account->acctType_ == ACCOUNT_TYPE_STOCK) continue;

        if (accountArray_) 
        {
            if (wxNOT_FOUND == accountArray_->Index(account->name_)) continue;
        }
        else
        {
            if (! activeTermAccounts_ && account->acctType_ == ACCOUNT_TYPE_TERM) continue;
            if (! activeBankAccounts_ && account->acctType_ == ACCOUNT_TYPE_BANK) continue;
        }


        core_->bTransactionList_.getDailyBalance(core_, account->id_, daily_balance);

        mmCurrency* pCurrencyPtr = core_->accountList_.getCurrencySharedPtr(account->id_);
        wxASSERT(pCurrencyPtr);
        CurrencyFormatter::instance().loadSettings(*pCurrencyPtr);
        double rate = pCurrencyPtr->baseConv_;
        tInitialBalance += account->initialBalance_ * rate;
   }

    // We now know the total balance on the account
    // Start by walking through the repeating transaction list

    wxDateTime yearFromNow = wxDateTime::Now().Add(wxDateSpan::Years(years));
    forecastVec fvec;

/*  TODO: Activate code using TTransactionBillList
    when other sections are completed and tested.
*/
#define USING_NEW_DB_CLASSES_  // Activation switch
#ifdef USING_NEW_DB_CLASSES
    // load a fresh list of repeating transactions.
    TTransactionBillList repeat_trans_list(core_->db_.get());

    for (const auto& repeat_entry:repeat_trans_list.entrylist_)
    {
        bool from_account_found = true, to_account_found = true;
        if (accountArray_ != NULL)
        {
            if (wxNOT_FOUND == accountArray_->Index(core_->accountList_.GetAccountName(repeat_entry->id_from_account_))) //linear search
                from_account_found = false;

            if (wxNOT_FOUND == accountArray_->Index(core_->accountList_.GetAccountName(repeat_entry->id_to_account_))) //linear search
                to_account_found = false;
        }
        if (!from_account_found && !to_account_found) continue; // skip account

        double convRate = core_->accountList_.getAccountBaseCurrencyConvRate(repeat_entry->id_from_account_);
        double amount = repeat_entry->AdjustedValue(repeat_entry->id_from_account_) * convRate;
        if (to_account_found && repeat_entry->IsTransferTo(repeat_entry->id_to_account_))
        {
            double toConvRate = core_->accountList_.getAccountBaseCurrencyConvRate(repeat_entry->id_to_account_);
            amount += repeat_entry->amount_to_ * toConvRate;
        }

        if (repeat_entry->UsingRepeatProcessing())
        {
            if (repeat_entry->UsingIn_X_Processing())
            {
                SetRepeatForecast(fvec, repeat_entry, amount);
            }
            else if (repeat_entry->UsingEvery_X_Processing())
            {
                SetYearsRepeatForecast(fvec, repeat_entry, amount, yearFromNow);
            }
            else
            {
                while (repeat_entry->num_repeats_ > 0)
                {
                    SetRepeatForecast(fvec, repeat_entry, amount);
                }
            }
        }
        else
        {
            SetYearsRepeatForecast(fvec, repeat_entry, amount, yearFromNow);
        }
    }

#else
    wxSQLite3ResultSet q1 = core_->db_.get()->ExecuteQuery(SELECT_ALL_FROM_BILLSDEPOSITS_V1);

    while (q1.NextRow())
    {
        wxDateTime nextOccurDate = q1.GetDate("NEXTOCCURRENCEDATE");
           
        int repeats             = q1.GetInt("REPEATS");
        int numRepeats          = q1.GetInt("NUMOCCURRENCES");
        wxString transType      = q1.GetString("TRANSCODE");
        double amt              = q1.GetDouble("TRANSAMOUNT");
        double toAmt            = q1.GetDouble("TOTRANSAMOUNT");

        // DeMultiplex the Auto Executable fields from the db entry: REPEATS
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
            repeats -= BD_REPEATS_MULTIPLEX_BASE;
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
            repeats -= BD_REPEATS_MULTIPLEX_BASE;

        bool processNumRepeats = false;
        if (numRepeats != -1)
            processNumRepeats = true;

        if (repeats == 0)
        {
            numRepeats = 1;
            processNumRepeats = true;
        }

        if (nextOccurDate > yearFromNow)
            continue;

        int accountID = q1.GetInt("ACCOUNTID");
        int toAccountID = q1.GetInt("TOACCOUNTID");

        bool isAccountFound = true, isToAccountFound = true;
        if (accountArray_ != NULL)
        {
            if (wxNOT_FOUND == accountArray_->Index(core_->accountList_.GetAccountName(accountID))) //linear search
                isAccountFound = false;

            if (wxNOT_FOUND == accountArray_->Index(core_->accountList_.GetAccountName(toAccountID))) //linear search
                isToAccountFound = false;
        }

        if (!isAccountFound && !isToAccountFound) continue; // skip account

        double convRate = core_->accountList_.getAccountBaseCurrencyConvRate(accountID);
        double toConvRate = core_->accountList_.getAccountBaseCurrencyConvRate(toAccountID);

        // Process all possible repeating transactions for this BD
        while(1)
        {
            if (nextOccurDate > yearFromNow)
                break;

            if (processNumRepeats)
                numRepeats--;

            mmRepeatForecast rf;
            rf.date = nextOccurDate;
            rf.amount = 0.0;

            if (transType == TRANS_TYPE_WITHDRAWAL_STR)
            {
                rf.amount = -amt * convRate;
            }
            else if (transType == TRANS_TYPE_DEPOSIT_STR)
            {
                rf.amount = +amt * convRate;
            }
            else //if (transType == TRANS_TYPE_TRANSFER_STR)
            {
                if (isAccountFound)
                    rf.amount -= amt * convRate;
                if (isToAccountFound)
                    rf.amount += toAmt * toConvRate;
            }

            fvec.push_back(rf);   

            if (processNumRepeats && (numRepeats <=0))
                break;

//            nextOccurDate = nextOccurDate.Add(mmRepeat(repeats));

            if (repeats == 1)
            {
                nextOccurDate = nextOccurDate.Add(wxTimeSpan::Week());
            }
            else if (repeats == 2)
            {
                nextOccurDate = nextOccurDate.Add(wxTimeSpan::Weeks(2));
            }
            else if (repeats == 3)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Month());
            }
            else if (repeats == 4)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Months(2));
            }
            else if (repeats == 5)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Months(3));
            }
            else if (repeats == 6)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Months(6));
            }
            else if (repeats == 7)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Year());
            }
            else if (repeats == 8)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Months(4));
            }
            else if (repeats == 9)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Weeks(4));
            }
            else if (repeats == 10)
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Days(1));
            }
            else if (repeats == 11) // repeat in numRepeats Days (Once only)
            {
                if (numRepeats > 0)
                {
                    nextOccurDate = nextOccurDate.Add(wxDateSpan::Days(numRepeats));
                    numRepeats = -1;
                }
                else break;
            }
            else if (repeats == 12) // repeat in numRepeats Months (Once only)
            {
                if (numRepeats > 0)
                {
                    nextOccurDate = nextOccurDate.Add(wxDateSpan::Months(numRepeats));
                    numRepeats = -1;
                }
                else break;
            }
            else if (repeats == 13) // repeat every numRepeats Days
            {
                if (numRepeats > 0)
                {
                    nextOccurDate = nextOccurDate.Add(wxDateSpan::Days(numRepeats));
                }
                else break;
            }
            else if (repeats == 14) // repeat every numRepeats Months
            {
                if (numRepeats > 0)
                {
                    nextOccurDate = nextOccurDate.Add(wxDateSpan::Months(numRepeats));
                }
                else break;
            }
            else if ((repeats == 15) || (repeats == 16))
            {
                nextOccurDate = nextOccurDate.Add(wxDateSpan::Month());
                nextOccurDate = nextOccurDate.SetToLastMonthDay(nextOccurDate.GetMonth(),nextOccurDate.GetYear());
                if (repeats == 16) // last weekday of month
                {
                    if (nextOccurDate.GetWeekDay() == wxDateTime::Sun || nextOccurDate.GetWeekDay() == wxDateTime::Sat)
                        nextOccurDate.SetToPrevWeekDay(wxDateTime::Fri);
                }
            }
            else break;
        } // end while
    } //end query
    q1.Finalize();
#endif

    // Now we have a vector of dates and amounts over next year
    int fcstsz = cashflowreporttype_ == 0 ? 12 * years : 366 * years;
    std::vector<double> forecastOver12Months(fcstsz, 0.0);

    for (int idx = 0; idx < (int)forecastOver12Months.size(); idx++)
    {
        wxDateTime dtBegin = wxDateTime::Now();
        wxDateTime dtEnd = cashflowreporttype_ == 0 ? wxDateTime::Now().Add(wxDateSpan::Months(idx)): wxDateTime::Now().Add(wxDateSpan::Days(idx));

		for (const auto& balance: fvec)
		{
			if (balance.date.IsBetween(dtBegin, dtEnd)) forecastOver12Months[idx] += balance.amount;
		}
		
		for (const auto& d_balance: daily_balance)
		{
			if (! d_balance.first.IsLaterThan(dtEnd)) forecastOver12Months[idx] += d_balance.second;
		}
    }

    core_->currencyList_.LoadBaseCurrencySettings();

    bool initialMonths = true;
    int displayYear    = wxDateTime::Now().GetYear();

    for (int idx = 0; idx < (int)forecastOver12Months.size(); idx++)
    {
		wxDateTime dtEnd = cashflowreporttype_ == 0 ? wxDateTime::Now().Add(wxDateSpan::Months(idx)): wxDateTime::Now().Add(wxDateSpan::Days(idx));
           
        double balance = forecastOver12Months[idx] + tInitialBalance;
        double diff;
        diff = (idx==0 ? 0 : forecastOver12Months[idx] - forecastOver12Months[idx-1]) ;

        bool addSeparator = false;
        bool addSeparatorAfter = false;

        if (wxGetApp().m_frame->budgetFinancialYears())
        {
            if (initialMonths && (dtEnd.GetMonth() == getUserDefinedFinancialYear().GetMonth()))
            {
                addSeparator  = true;
                initialMonths = false;
            }
            else if ((dtEnd.GetMonth() == getUserDefinedFinancialYear().GetMonth()) && (displayYear != dtEnd.GetYear()))
            {
                addSeparator = true;
                displayYear  = dtEnd.GetYear();
            }
        }
        else if (displayYear != dtEnd.GetYear())
        {
            addSeparator = true;
            displayYear  = dtEnd.GetYear();
        }

        if (addSeparator) hb.addRowSeparator(3);

        wxString dtStr ; 
        //dtStr << mmGetNiceShortMonthName(dtEnd.GetMonth()) << " " << dtEnd.GetYear();
        dtStr << mmGetDateForDisplay(dtEnd); 

        hb.startTableRow();
        hb.addTableCell(dtStr, false, true);
        hb.addMoneyCell(balance);
        hb.addMoneyCell(diff);
        hb.endTableRow();
        
        // Add a separator for each month in daily cash flow report
        if (cashflowreporttype_ == 1)
        {
            wxDateTime dtLMD;// Last day of the month
            dtLMD = dtLMD.SetToLastMonthDay(dtEnd.GetMonth(),dtEnd.GetYear());
            
            if (dtEnd.IsSameDate(dtLMD))
                addSeparatorAfter = true;
        }
        if (addSeparatorAfter) hb.addRowSeparator(3);
    }

    hb.addRowSeparator(3);
    hb.endTable();
    hb.endCenter();
    hb.end();

    return hb.getHTMLText();
}

//-----------------------------------------------------------------------------
mmReportCashFlowAllAccounts::mmReportCashFlowAllAccounts(mmCoreDB* core)
: mmReportCashFlow(core, 0)
{
    this->activateBankAccounts();
    this->activateTermAccounts();
}

mmReportCashFlowBankAccounts::mmReportCashFlowBankAccounts(mmCoreDB* core)
: mmReportCashFlow(core, 0)
{
    this->activateBankAccounts();
}

mmReportCashFlowTermAccounts::mmReportCashFlowTermAccounts(mmCoreDB* core)
: mmReportCashFlow(core, 0)
{
    this->activateTermAccounts();
}

//-----------------------------------------------------------------------------
mmReportCashFlowSpecificAccounts::mmReportCashFlowSpecificAccounts(mmCoreDB* core)
: mmReportCashFlow(core, 0)
{
    this->cashflowreporttype_ = 0;
}

wxString mmReportCashFlowSpecificAccounts::getHTMLText()
{
    this->getSpecificAccounts();
    return this->getHTMLText_i();
}

//-----------------------------------------------------------------------------
mmReportDailyCashFlowSpecificAccounts::mmReportDailyCashFlowSpecificAccounts(mmCoreDB* core)
: mmReportCashFlowSpecificAccounts(core)
{
    this->cashflowreporttype_ = 1;
}
