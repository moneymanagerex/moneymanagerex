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

mmReportSummaryAssets::mmReportSummaryAssets(mmCoreDB* core)
: mmPrintableBase(core)
{}

wxString mmReportSummaryAssets::getHTMLText()
{
    mmHTMLBuilder hb;
    hb.init();
    hb.addHeader(2, _("Summary of Assets"));
    hb.addDateNow();

    hb.startCenter();

    hb.startTable("95%");
    hb.startTableRow();
    hb.addTableHeaderCell(_("Date"));
    hb.addTableHeaderCell(_("Name"));
    hb.addTableHeaderCell(_("Type"));
    hb.addTableHeaderCell(_("Current Value"), true);
    hb.addTableHeaderCell(_("Notes"));
    hb.endTableRow();

    core_->currencyList_.LoadBaseCurrencySettings();

    double balance = 0.0;
    Model_Asset::instance().db_ = core_->db_.get();
    for (const auto& pEntry: Model_Asset::instance().all())
    {
        hb.startTableRow();
        hb.addTableCell(pEntry.STARTDATE, false, true);
        hb.addTableCell(pEntry.ASSETNAME, false, true);
        hb.addTableCell(wxGetTranslation(pEntry.ASSETTYPE));
        hb.addMoneyCell(pEntry.VALUE);
        hb.addTableCell(pEntry.NOTES);
        hb.endTableRow();

        balance += pEntry.VALUE;
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
