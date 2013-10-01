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
#include "mmex.h"
#include <algorithm>

#define ASSETS_SORT_BY_DATE     1
#define ASSETS_SORT_BY_NAME     2
#define ASSETS_SORT_BY_TYPE     3
#define ASSETS_SORT_BY_VALUE    4
#define ASSETS_SORT_BY_NOTES    5

mmReportSummaryAssets::mmReportSummaryAssets(mmCoreDB* core)
: mmPrintableBase(core, ASSETS_SORT_BY_NAME)
{
}

wxString mmReportSummaryAssets::getHTMLText()
{
    // structure for sorting of data
    struct data_holder {wxDateTime date; wxString name; wxString type; double value; wxString notes;} line;
    std::vector<data_holder> data;

    core_->currencyList_.LoadBaseCurrencySettings();

    double balance = 0.0;
    for (const auto& pEntry: Model_Asset::instance().all())
    {
        balance += pEntry.VALUE;

        line.date = Model_Asset::STARTDATE(pEntry);
        line.name = pEntry.ASSETNAME;
        line.type = wxGetTranslation(pEntry.ASSETTYPE);
        line.value = Model_Asset::value(pEntry);
        line.notes = pEntry.NOTES;
        data.push_back(line);
    }

    switch (sortColumn_)
    {
    case ASSETS_SORT_BY_DATE:
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.date != y.date)
                    return x.date < y.date;
                else return x.name < y.name;
            }
        );
        break;
    case ASSETS_SORT_BY_TYPE:
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.type != y.type) return x.type < y.type;
                else return x.name < y.name;
            }
        );
        break;
    case ASSETS_SORT_BY_VALUE:
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.value != y.value) return x.value < y.value;
                else return x.name < y.name;
            }
        );
        break;
    case ASSETS_SORT_BY_NOTES:
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                if (x.notes != y.notes) return x.notes < y.notes;
                else return x.name < y.name;
            }
        );
        break;
    default:
        sortColumn_ = ASSETS_SORT_BY_NAME;
        std::stable_sort(data.begin(), data.end()
            , [] (const data_holder& x, const data_holder& y)
            {
                return x.name < y.name;
            }
        );
    }

    wxGetApp().m_frame->SetStatusText(this->version());
    mmHTMLBuilder hb;
    hb.init();
    hb.addHeader(2, _("Summary of Assets"));
    hb.addDateNow();

    hb.startCenter();

    hb.startTable("95%");
    hb.startTableRow();
    if(ASSETS_SORT_BY_DATE == sortColumn_)
        hb.addTableHeaderCell(_("Date"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", ASSETS_SORT_BY_DATE), _("Date"));
    if(ASSETS_SORT_BY_NAME == sortColumn_)
        hb.addTableHeaderCell(_("Name"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", ASSETS_SORT_BY_NAME), _("Name"));
    if(ASSETS_SORT_BY_TYPE == sortColumn_)
        hb.addTableHeaderCell(_("Type"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", ASSETS_SORT_BY_TYPE), _("Type"));
    if(ASSETS_SORT_BY_VALUE == sortColumn_)
        hb.addTableHeaderCell(_("Current Value"), true);
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", ASSETS_SORT_BY_VALUE), _("Current Value"), true);
    if(ASSETS_SORT_BY_NOTES == sortColumn_)
        hb.addTableHeaderCell(_("Notes"));
    else
        hb.addTableHeaderCellLink(wxString::Format("SORT:%d", ASSETS_SORT_BY_NOTES), _("Notes"));
    hb.endTableRow();

    core_->currencyList_.LoadBaseCurrencySettings();

    for (const auto& entry : data)
    {
        hb.startTableRow();
        hb.addTableCell(mmGetDateForDisplay(entry.date), false, true);
        hb.addTableCell(entry.name, false, true);
        hb.addTableCell(entry.type);
        hb.addMoneyCell(entry.value);
        hb.addTableCell(entry.notes);
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
