#include "billsdepositspanel.h"
#include "htmlbuilder.h"
#include "util.h"
#include "mmCurrencyFormatter.h"
#include "html_widget_bills_and_deposits.h"
#include "model/Model_Billsdeposits.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"

#include <algorithm>

htmlWidgetBillsAndDeposits::htmlWidgetBillsAndDeposits(const wxString& title, mmDateRange* date_range)
    : title_(title)
    , date_range_(date_range)
{}

htmlWidgetBillsAndDeposits::~htmlWidgetBillsAndDeposits()
{
    if (date_range_) delete date_range_;
}

wxString htmlWidgetBillsAndDeposits::getHTMLText()
{
    mmHTMLBuilder hb;
    std::vector<mmBDTransactionHolder> trans_;

    const wxDateTime &today = date_range_->today();
    bool visibleEntries = false;
    for (const auto& q1 : Model_Billsdeposits::instance().all())
    {
        mmBDTransactionHolder th;

        th.id_             = q1.BDID;
        th.nextOccurDate_  = Model_Billsdeposits::NEXTOCCURRENCEDATE(q1);
        th.nextOccurStr_   = mmGetDateForDisplay(th.nextOccurDate_);
        int numRepeats     = q1.NUMOCCURRENCES;

        int repeats        = q1.REPEATS;
        // DeMultiplex the Auto Executable fields.
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
            repeats -= BD_REPEATS_MULTIPLEX_BASE;

        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
            repeats -= BD_REPEATS_MULTIPLEX_BASE;

        wxTimeSpan ts = th.nextOccurDate_.Subtract(today);
        th.daysRemaining_ = ts.GetDays();
        int minutesRemaining_ = ts.GetMinutes();

        if (minutesRemaining_ > 0)
            th.daysRemaining_ += 1;

        th.daysRemainingStr_ = wxString::Format("%d", th.daysRemaining_) + _(" days remaining");

        if (th.daysRemaining_ == 0)
        {
            if ( ((repeats > 10) && (repeats < 15)) && (numRepeats < 0) )
                th.daysRemainingStr_ = _("Inactive");
        }

        if (th.daysRemaining_ < 0)
        {
            th.daysRemainingStr_ = wxString::Format("%d", abs(th.daysRemaining_)) + _(" days overdue!");
            if ( ((repeats > 10) && (repeats < 15)) && (numRepeats < 0) )
                th.daysRemainingStr_ = _("Inactive");
        }

        th.payeeID_        = q1.PAYEEID;
        th.transType_      = q1.TRANSCODE;
        th.accountID_      = q1.ACCOUNTID;
        th.toAccountID_    = q1.TOACCOUNTID;

        th.amt_            = q1.TRANSAMOUNT;
        th.toAmt_          = q1.TOTRANSAMOUNT;

        th.transAmtString_ = CurrencyFormatter::float2String(th.amt_);
        //for Withdrawal amount should be negative
        if (th.transType_== TRANS_TYPE_WITHDRAWAL_STR)
        {
            th.transAmtString_= "-" + th.transAmtString_;
            th.amt_ = -th.amt_;
        }

        th.transToAmtString_ = CurrencyFormatter::float2String(th.toAmt_);

        if (th.transType_ == TRANS_TYPE_TRANSFER_STR)
        {
            Model_Account::Data *account = Model_Account::instance().get(th.toAccountID_);
            if (account)
                th.payeeStr_ = account->ACCOUNTNAME;
        }
        else
        {
            Model_Payee::Data* payee = Model_Payee::instance().get(th.payeeID_);
            if (payee)
                th.payeeStr_ = payee->PAYEENAME;
        }

        if (th.daysRemaining_ <= 14)
            visibleEntries = true;

        trans_.push_back(th);
    }
    std::sort(trans_.begin(), trans_.end(),
        [](const mmBDTransactionHolder& x, const mmBDTransactionHolder& y){ return x.daysRemaining_ < y.daysRemaining_; });

    ////////////////////////////////////
    if ( visibleEntries )
    {
        wxString colorStr;

        hb.startTable("100%");
        hb.addTableHeaderRowLink("billsdeposits", title_, 3);

        std::vector<wxString> data4;
        for (size_t bdidx = 0; bdidx < trans_.size(); ++bdidx)
        {
            data4.clear();
            wxTimeSpan ts = trans_[bdidx].nextOccurDate_.Subtract(today);
            //int hoursRemaining_ = ts.GetHours();

            if (trans_[bdidx].daysRemaining_ <= 14)
            {
                wxString daysRemainingStr;
                colorStr = "#9999FF";

                daysRemainingStr = trans_[bdidx].daysRemainingStr_;
                if (trans_[bdidx].daysRemaining_ < 0)
                    colorStr = "#FF6600";

                // TODO Load the currency for this BD

                hb.startTableRow();
                hb.addTableCell(trans_[bdidx].payeeStr_, false, true);
                hb.addTableCell(CurrencyFormatter::float2Money(trans_[bdidx].amt_), true);
                //Draw it as numeric that mean align right
                hb.addTableCell(daysRemainingStr, true, false, false, colorStr);
                hb.endTableRow();
            }
        }
        hb.endTable();
    }
    return hb.getHTMLinTableWraper(true);
}
