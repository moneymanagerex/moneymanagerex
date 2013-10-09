#include "html_widget_top_categories.h"

#include "htmlbuilder.h"
#include "util.h"

#include <algorithm>

htmlWidgetTop7Categories::htmlWidgetTop7Categories(mmCoreDB* core, const wxString& title, mmDateRange* date_range)
    : core_(core)
    , title_(title)
    , date_range_(date_range)
{}

htmlWidgetTop7Categories::~htmlWidgetTop7Categories()
{
    if (date_range_) delete date_range_;
}

wxString htmlWidgetTop7Categories::getHTMLText()
{
    mmHTMLBuilder hb;

    wxString headerMsg = wxString::Format(_("Top Withdrawals: %s"), date_range_->title());

    hb.startTable("100%");
    hb.addTableHeaderRow(headerMsg, 2);
    hb.startTableRow();
    hb.addTableCell(_("Category"), false, false, true);
    //hb.addTableCell(_("QTY"), true, false, true);
    hb.addTableCell(_("Summary"), true, false, true);
    hb.endTableRow();

    //Get statistic for las 30 days
    mmDateRange* date_range = new mmLast30Days;
    std::vector<std::pair<wxString, double> > topCategoryStats;
    core_->bTransactionList_.getTopCategoryStats(
        topCategoryStats
        , date_range
    );
    delete date_range;

    for (const auto& i : topCategoryStats)
    {
        hb.startTableRow();
        hb.addTableCell((i.first.IsEmpty() ? "..." : i.first), false, true);
        hb.addMoneyCell(i.second);
        hb.endTableRow();
    }

    return hb.getHTMLinTableWraper(true);
}
