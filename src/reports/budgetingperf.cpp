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

void mmReportBudgetingPerformance::DisplayEstimateMonths(mmHTMLBuilder& hb, double estimated)
{
    for (int yidx = 0; yidx < 12; yidx++)
    {
        // Set the estimate for each month
        hb.addMoneyCell(estimated / 12);
    }
}

void mmReportBudgetingPerformance::DisplayActualMonths(mmHTMLBuilder& hb, double estimated, std::map<int, double>& actual)
{
    double est = estimated / 12;
    for (const auto &i : actual)
    {
        double actualMonthVal = i.second;

        if (actualMonthVal < est)
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
    mmSpecifiedRange date_range(yearBegin, yearEnd);

    bool evaluateTransfer = false;
    if (wxGetApp().m_frame->budgetTransferTotal())
    {
        evaluateTransfer = true;
    }
    //Get statistics
    std::map<int, std::map<int, wxString> > budgetPeriod;
    std::map<int, std::map<int, double> > budgetAmt;
    Model_Budget::instance().getBudgetEntry(budgetYearID_, budgetPeriod, budgetAmt);
    std::map<int, std::map<int, std::map<int, double> > > categoryStats;
    Model_Category::instance().getCategoryStats(categoryStats, &date_range, mmIniOptions::instance().ignoreFutureTransactions_,
        true, true, (evaluateTransfer ? &budgetAmt : 0));
    //Init totals
    const Model_Category::Data_Set allCategories = Model_Category::instance().all(Model_Category::COL_CATEGNAME);
    const Model_Subcategory::Data_Set allSubcategories = Model_Subcategory::instance().all(Model_Subcategory::COL_SUBCATEGNAME);
    std::map<int, std::map<int, double> > totals;
    for (const auto& category : allCategories)
    {
        totals[category.CATEGID][-1] = 0;
        for (const Model_Subcategory::Data& subcategory : allSubcategories)
        {
            if (subcategory.CATEGID == category.CATEGID)
                totals[category.CATEGID][subcategory.SUBCATEGID] = 0;
        }
    }
    for (const auto& category : allCategories)
    {
        for (const auto &i : categoryStats[category.CATEGID][-1])
        {
            totals[category.CATEGID][-1] += categoryStats[category.CATEGID][-1][i.first];
            for (const Model_Subcategory::Data& subcategory : allSubcategories)
            {
                if (subcategory.CATEGID == category.CATEGID)
                    totals[category.CATEGID][subcategory.SUBCATEGID] += categoryStats[category.CATEGID][subcategory.SUBCATEGID][i.first];
            }
        }
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

    for (const Model_Category::Data& category : allCategories)
    {
        mmBudgetEntryHolder th;
        initBudgetEntryFields(th, budgetYearID_);
        th.categID_ = category.CATEGID;
        th.catStr_  = category.CATEGNAME;
        th.period_ = budgetPeriod[th.categID_][th.subcategID_];
        th.amt_ = budgetAmt[th.categID_][th.subcategID_];

        // Set the estimated amount for the year
        setBudgetYearlyEstimate(th);
        double totalEstimated_ = th.estimated_;

        // set the actual amount for the year
        th.actual_ = totals[th.categID_][th.subcategID_];

        // estimated stuff
        if ((totalEstimated_ != 0.0) || (th.actual_ != 0.0))
        {
            hb.startTableRow();
            hb.addTableCell(th.catStr_, false, true);
            hb.addTableCell(_("Estimated"));

            DisplayEstimateMonths(hb, th.estimated_);

            hb.addMoneyCell(totalEstimated_);
            hb.addTableCell("-");
            hb.endTableRow();

            // actual stuff
            hb.startTableRow();
            hb.addTableCell(th.catStr_, false, true);
            hb.addTableCell(_("Actual"));

            DisplayActualMonths(hb, th.estimated_, categoryStats[th.categID_][th.subcategID_]);

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

        for (const Model_Subcategory::Data& subcategory : allSubcategories)
        {
            if (subcategory.CATEGID != category.CATEGID) continue;
            mmBudgetEntryHolder thsub;
            initBudgetEntryFields(thsub, budgetYearID_);
            thsub.categID_ = th.categID_;
            thsub.catStr_  = th.catStr_;
            thsub.subcategID_ = subcategory.SUBCATEGID;
            thsub.subCatStr_  = subcategory.SUBCATEGNAME;
            thsub.period_ = budgetPeriod[thsub.categID_][thsub.subcategID_];
            thsub.amt_ = budgetAmt[thsub.categID_][thsub.subcategID_];

            // Set the estimated amount for the year
            setBudgetYearlyEstimate(thsub);
            totalEstimated_ = thsub.estimated_;

            // set the actual abount for the year
            thsub.actual_ = totals[thsub.categID_][thsub.subcategID_];

            if ((totalEstimated_ != 0.0) || (thsub.actual_ != 0.0))
            {
                hb.startTableRow();
                hb.addTableCell(thsub.catStr_+ ": " + thsub.subCatStr_, false, true);
                hb.addTableCell(_("Estimated"));

                DisplayEstimateMonths(hb, thsub.estimated_);

                hb.addMoneyCell(totalEstimated_);
                hb.addTableCell("-");
                hb.endTableRow();

                hb.startTableRow();
                hb.addTableCell(thsub.catStr_+ ": " + thsub.subCatStr_, false, true);
                hb.addTableCell(_("Actual"));

                DisplayActualMonths(hb, thsub.estimated_, categoryStats[thsub.categID_][thsub.subcategID_]);

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
