/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#include "summaryassets.h"
#include "htmlbuilder.h"
#include "model/Model_Asset.h"
#include "util.h"
#include "model/Model_Infotable.h"
#include <algorithm>
#include "model/Model_Stock.h"

mmReportSummaryAssets::mmReportSummaryAssets()
: mmPrintableBase(Model_Asset::COL_ASSETNAME)
{
}

wxString mmReportSummaryAssets::getHTMLText()
{
    mmHTMLBuilder hb;
    hb.init();
    hb.addHeader(2, _("Summary of Assets"));
    hb.addDateNow();

    hb.startCenter();

    hb.startTable("95%");
    hb.startTableRow();
    if(Model_Asset::COL_STARTDATE == sortColumn_)
        hb.addTableHeaderCell(_("Date"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", Model_Asset::COL_STARTDATE), _("Date"));
    if(Model_Asset::COL_ASSETNAME == sortColumn_)
        hb.addTableHeaderCell(_("Name"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", Model_Asset::COL_ASSETNAME), _("Name"));
    if(Model_Asset::COL_ASSETTYPE == sortColumn_)
        hb.addTableHeaderCell(_("Type"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", Model_Asset::COL_ASSETTYPE), _("Type"));
    if(Model_Asset::COL_VALUE == sortColumn_)
        hb.addTableHeaderCell(_("Current Value"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", Model_Asset::COL_VALUE), _("Current Value"), true);
    if(Model_Asset::COL_NOTES == sortColumn_)
        hb.addTableHeaderCell(_("Notes"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", Model_Asset::COL_NOTES), _("Notes"));
    hb.endTableRow();

    double balance = 0.0;
    for (const auto& pEntry: Model_Asset::instance().all((Model_Asset::COLUMN)sortColumn_))
    {
        balance += pEntry.VALUE;

        hb.startTableRow();
        hb.addTableCell(mmGetDateForDisplay(Model_Asset::STARTDATE(pEntry)), false, true);
        hb.addTableCell(pEntry.ASSETNAME, false, true);
        hb.addTableCell(wxGetTranslation(pEntry.ASSETTYPE));
        hb.addMoneyCell(Model_Asset::value(pEntry));
        hb.addTableCell(pEntry.NOTES);
        hb.endTableRow();
    }

    /* Assets */
    hb.addRowSeparator(5);
    hb.addTotalRow(_("Total Assets: "), 4, balance);
    hb.addTableCell("");
    hb.endTableRow();
    hb.endTable();

    hb.endCenter();
    hb.end();

    return hb.getHTMLText();
}

mmReportSummaryAssetsNew::mmReportSummaryAssetsNew()
: mmPrintableBase(Model_Asset::COL_ASSETNAME)
{
}

wxString mmReportSummaryAssetsNew::getHTMLText()
{
    mm_html_template summaryasset("summaryasset.html");
    summaryasset("TITLE") = _("Summary of Assets");
    
    loop_t assets;
    double balance = 0.0;
    for (const auto& pEntry: Model_Asset::instance().all((Model_Asset::COLUMN)sortColumn_))
    {
        balance += pEntry.VALUE;
        row_t row = pEntry.to_row_t();
        row("STARTDATE") = mmGetDateForDisplay(Model_Asset::STARTDATE(pEntry));
        row("ASSETTYPE") = wxGetTranslation(pEntry.ASSETTYPE);
        row("VALUE") = Model_Currency::toString(pEntry.VALUE);
        row("CURRENT_VALUE") = Model_Currency::toString(Model_Asset::value(pEntry));

        assets += row;
    }

    summaryasset("ASSETS") = assets;
    summaryasset("ASSET_BALANCE") = Model_Currency::toCurrency(balance);
    summaryasset("INFOTABLE") = Model_Infotable::to_loop_t();

    loop_t stocks;
    for (const auto& r: Model_Stock::instance().all())
    {
        stocks += r.to_row_t();
    }
    summaryasset("stocks") = stocks;

    return summaryasset.Process();
}
