/*******************************************************
 Copyright (C) 2013,2014 James Higley
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "ScheduledSplitModel.h"
#include "AttachmentModel.h"
#include "TagLinkModel.h"

ScheduledSplitModel::ScheduledSplitModel()
: Model<ScheduledSplitTable>()
{
}

ScheduledSplitModel::~ScheduledSplitModel()
{
}

/**
* Initialize the global ScheduledSplitModel table.
* Reset the ScheduledSplitModel table or create the table if it does not exist.
*/
ScheduledSplitModel& ScheduledSplitModel::instance(wxSQLite3Database* db)
{
    ScheduledSplitModel& ins = Singleton<ScheduledSplitModel>::instance();
    ins.m_db = db;
    ins.destroy_cache();
    ins.ensure_table();

    return ins;
}

/** Return the static instance of ScheduledSplitModel table */
ScheduledSplitModel& ScheduledSplitModel::instance()
{
    return Singleton<ScheduledSplitModel>::instance();
}

double ScheduledSplitModel::get_total(const Data_Set& rows)
{
    double total = 0.0;
    for (auto& r : rows) total += r.SPLITTRANSAMOUNT;

    return total;
}

bool ScheduledSplitModel::remove(const int64 id)
{
    // Delete all tags for the split before removing it
    TagLinkModel::instance().DeleteAllTags(ScheduledSplitModel::refTypeName, id);
    return Model<ScheduledSplitTable>::remove(id);
}

std::map<int64, ScheduledSplitModel::Data_Set> ScheduledSplitModel::get_all_id()
{
    std::map<int64, ScheduledSplitModel::Data_Set> data;
    for (const auto & split : instance().get_all())
    {
        data[split.TRANSID].push_back(split);
    }

    return data;
}

int ScheduledSplitModel::update(Data_Set& rows, int64 transactionID)
{

    Data_Set split = instance().find(TRANSID(transactionID));
    for (const auto& split_item : split)
    {
        instance().remove(split_item.SPLITTRANSID);
    }

    if (!rows.empty())
    {
        Data_Set split_items;
        for (const auto &item : rows)
        {
            Data *split_item = instance().create();
            split_item->TRANSID = transactionID;
            split_item->SPLITTRANSAMOUNT = item.SPLITTRANSAMOUNT;
            split_item->CATEGID = item.CATEGID;
            split_item->NOTES = item.NOTES;
            split_items.push_back(*split_item);
        }
        instance().save(split_items);

        // Send back the new SPLITTRANSID which is needed to update taglinks
        for (int i = 0; i < static_cast<int>(rows.size()); i++)
            rows.at(i).SPLITTRANSID = split_items.at(i).SPLITTRANSID;
    }
    return rows.size();
}
