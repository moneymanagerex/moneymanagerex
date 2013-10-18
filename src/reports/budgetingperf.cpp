#include "budgetingperf.h"
#include "budgetingpanel.h"
#include "htmlbuilder.h"
#include "mmex.h"
#include "model/Model_Budgetyear.h"
#include "model/Model_Budget.h"
#include "model/Model_Category.h"
#include "model/Model_Subcategory.h"

mmReportBudgetingPerformance::mmReportBudgetingPerformance(int budgetYearID)
: budgetYearID_(budgetYearID)
{}

void mmReportBudgetingPerformance::DisplayEstimateMonths(mmHTMLBuilder& hb, mmBudgetEntryHolder& budgetEntry, int startMonth)
{
    int month;
    for (int yidx = 0; yidx < 12; yidx++)
    {
        month = yidx + startMonth;
        if (month > 11)
        {
            month = month - 12;
        }
        // Set the estimate for each month
        hb.addMoneyCell(budgetEntry.estimated_ / 12);
    }
}

void mmReportBudgetingPerformance::DisplayActualMonths(mmHTMLBuilder& hb, mmBudgetEntryHolder& budgetEntry, int startMonth, long startYear)
{
    bool evaluateTransfer = false;
    if (wxGetApp().m_frame->budgetTransferTotal())
    {
        evaluateTransfer = true;
    }
    int month;
    for (int yidx = 0; yidx < 12; yidx++)
    {
        double currentStartYear = startYear;
        double currentEndYear = startYear;
        month = yidx + startMonth;
        if (month > 11)
        {
            month = month - 12;
            currentStartYear ++;
            currentEndYear++;
        }
        wxDateTime dtBegin(1, (wxDateTime::Month)month, currentStartYear);
        wxDateTime dtEnd = dtBegin.GetLastMonthDay((wxDateTime::Month)month, currentEndYear);
        bool transferAsDeposit = true;
        if (budgetEntry.amt_ < 0)
        {
            transferAsDeposit = false;
        }
        double actualMonthVal = Model_Category::instance().getAmountForCategory(budgetEntry.categID_, budgetEntry.subcategID_,
            false, dtBegin, dtEnd, evaluateTransfer, transferAsDeposit, mmIniOptions::instance().ignoreFutureTransactions_
        );

        if(actualMonthVal < budgetEntry.estimated_)
        {
            hb.addMoneyCell(actualMonthVal, "RED");
        }
        else
        {
            hb.addMoneyCell(actualMonthVal);
        }
    }
}

wxString mmReportBudgetingPerformance::getHTMLText()
{
    int startDay = 1;
    int startMonth = wxDateTime::Jan;
    int endDay   = 31;
    int endMonth = wxDateTime::Dec;

    long startYear;
    wxString startYearStr = Model_Budgetyear::instance().Get(budgetYearID_);
    startYearStr.ToLong(&startYear);

    wxString headingStr = AdjustYearValues(startDay, startMonth, startYear, startYearStr);
    wxDateTime yearBegin(startDay, (wxDateTime::Month)startMonth, startYear);
    wxDateTime yearEnd(endDay, (wxDateTime::Month)endMonth, startYear);

    AdjustDateForEndFinancialYear(yearEnd);

    bool evaluateTransfer = false;
    if (wxGetApp().m_frame->budgetTransferTotal())
    {
        evaluateTransfer = true;
    }
    mmHTMLBuilder hb;
    hb.init();
    hb.addHeader(2, _("Budget Performance for ") + headingStr );
    hb.DisplayDateHeading(yearBegin, yearEnd);

    hb.startCenter();

    hb.startTable();
    hb.startTableRow();
    hb.addTableHeaderCell(_("Category"));
    hb.addTableHeaderCell(_("Type"));

    for (wxDateTime::Month m = wxDateTime::Jan; m <= wxDateTime::Dec; m = wxDateTime::Month(m + 1))
        hb.addTableHeaderCell(wxGetTranslation(wxDateTime::GetEnglishMonthName(m, wxDateTime::Name_Abbr)));
    hb.addTableHeaderCell(_("Overall"));
    hb.addTableHeaderCell(_("%"));
    hb.endTableRow();

    for (const Model_Category::Data& category: Model_Category::instance().all(Model_Category::COL_CATEGNAME))
    {
        mmBudgetEntryHolder th;
        initBudgetEntryFields(th, budgetYearID_);
        th.categID_ = category.CATEGID;
        th.catStr_  = category.CATEGNAME;
        Model_Budget::instance().getBudgetEntry(budgetYearID_, th.categID_, th.subcategID_, th.period_, th.amt_);

        // Set the estimated amount for the year
        setBudgetYearlyEstimate(th);
        double totalEstimated_ = th.estimated_;

        // set the actual amount for the year
        bool transferAsDeposit = true;
        if (th.amt_ < 0)
        {
            transferAsDeposit = false;
        }
        th.actual_ = Model_Category::instance().getAmountForCategory(th.categID_, th.subcategID_, false,
            yearBegin, yearEnd, evaluateTransfer, transferAsDeposit, mmIniOptions::instance().ignoreFutureTransactions_
        );

        // estimated stuff
        if ((totalEstimated_ != 0.0) || (th.actual_ != 0.0))
        {
            hb.startTableRow();
            hb.addTableCell(th.catStr_, false, true);
            hb.addTableCell(_("Estimated"));

            DisplayEstimateMonths(hb, th, startMonth);

            hb.addMoneyCell(totalEstimated_);
            hb.addTableCell("-");
            hb.endTableRow();

            // actual stuff
            hb.startTableRow();
            hb.addTableCell(th.catStr_, false, true);
            hb.addTableCell(_("Actual"));

            DisplayActualMonths(hb, th, startMonth, startYear);

            // year end
            if(th.actual_ < totalEstimated_)
            {
                hb.addMoneyCell(th.actual_, "RED");
            }
            else
            {
                hb.addMoneyCell(th.actual_);
            }

            if (((totalEstimated_ < 0) && (th.actual_ < 0)) ||
                ((totalEstimated_ > 0) && (th.actual_ > 0)))
            {
                double percent = (fabs(th.actual_) / fabs(totalEstimated_)) * 100.0;
                hb.addTableCell(wxString::Format("%.0f", percent));
            }
            else
            {
                hb.addTableCell("-");
            }

            hb.endTableRow();

            hb.addRowSeparator(16);
        }

        for (const Model_Subcategory::Data& subcategory: Model_Subcategory::instance().find(Model_Subcategory::CATEGID(th.categID_)))
        {
            mmBudgetEntryHolder thsub;
            initBudgetEntryFields(thsub, budgetYearID_);
            thsub.categID_ = th.categID_;
            thsub.catStr_  = th.catStr_;
            thsub.subcategID_ = subcategory.SUBCATEGID;
            thsub.subCatStr_  = subcategory.SUBCATEGNAME;

            Model_Budget::instance().getBudgetEntry(budgetYearID_, thsub.categID_, thsub.subcategID_, thsub.period_, thsub.amt_);

            // Set the estimated amount for the year
            setBudgetYearlyEstimate(thsub);
            totalEstimated_ = thsub.estimated_;

            // set the actual abount for the year
            transferAsDeposit = true;
            if (thsub.amt_ < 0)
            {
                transferAsDeposit = false;
            }
            thsub.actual_ = Model_Category::instance().getAmountForCategory(thsub.categID_, thsub.subcategID_, false,
                yearBegin, yearEnd, evaluateTransfer, transferAsDeposit, mmIniOptions::instance().ignoreFutureTransactions_
            );

            if ((totalEstimated_ != 0.0) || (thsub.actual_ != 0.0))
            {
                hb.startTableRow();
                hb.addTableCell(thsub.catStr_+ ": " + thsub.subCatStr_, false, true);
                hb.addTableCell(_("Estimated"));

                DisplayEstimateMonths(hb, thsub, startMonth);

                hb.addMoneyCell(totalEstimated_);
                hb.addTableCell("-");
                hb.endTableRow();

                hb.startTableRow();
                hb.addTableCell(thsub.catStr_+ ": " + thsub.subCatStr_, false, true);
                hb.addTableCell(_("Actual"));

                DisplayActualMonths(hb, thsub, startMonth, startYear);

                // year end
                if(thsub.actual_ < totalEstimated_)
                {
                    hb.addMoneyCell(thsub.actual_, "RED");
                }
                else
                {
                    hb.addMoneyCell(thsub.actual_);
                }

                if (((totalEstimated_ < 0) && (thsub.actual_ < 0)) ||
                    ((totalEstimated_ > 0) && (thsub.actual_ > 0)))
                {
                    double percent = (fabs(thsub.actual_) / fabs(totalEstimated_)) * 100.0;
                    hb.addTableCell(wxString::Format("%.0f", percent));
                }
                else
                {
                    hb.addTableCell("-");
                }

                hb.endTableRow();
                hb.addRowSeparator(16);
            }
        }
    }

    hb.endTable();
    hb.endCenter();

    hb.end();
    return hb.getHTMLText();
}
