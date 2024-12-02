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

#include "Model_Budgetsplittransaction.h"
#include "Model_Attachment.h"
#include "Model_Taglink.h"

Model_Budgetsplittransaction::Model_Budgetsplittransaction()
: Model<DB_Table_BUDGETSPLITTRANSACTIONS_V1>()
{
}

Model_Budgetsplittransaction::~Model_Budgetsplittransaction()
{
}

/**
* Initialize the global Model_Budgetsplittransaction table.
* Reset the Model_Budgetsplittransaction table or create the table if it does not exist.
*/
Model_Budgetsplittransaction& Model_Budgetsplittransaction::instance(wxSQLite3Database* db)
{
    Model_Budgetsplittransaction& ins = Singleton<Model_Budgetsplittransaction>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_Budgetsplittransaction table */
Model_Budgetsplittransaction& Model_Budgetsplittransaction::instance()
{
    return Singleton<Model_Budgetsplittransaction>::instance();
}

double Model_Budgetsplittransaction::get_total(const Data_Set& rows)
{
    double total = 0.0;
    for (auto& r : rows) total += r.SPLITTRANSAMOUNT;

    return total;
}

bool Model_Budgetsplittransaction::remove(int64 id)
{
    // Delete all tags for the split before removing it
    Model_Taglink::instance().DeleteAllTags(Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSITSPLIT), id);
    return this->remove(id, db_);
}

std::map<int64, Model_Budgetsplittransaction::Data_Set> Model_Budgetsplittransaction::get_all()
{
    std::map<int64, Model_Budgetsplittransaction::Data_Set> data;
    for (const auto & split : instance().all())
    {
        data[split.TRANSID].push_back(split);
    }

    return data;
}

int Model_Budgetsplittransaction::update(Data_Set& rows, int64 transactionID)
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
        for (int i = 0; i < rows.size(); i++)
            rows.at(i).SPLITTRANSID = split_items.at(i).SPLITTRANSID;
    }
    return rows.size();
}
