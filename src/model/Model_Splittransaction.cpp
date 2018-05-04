/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)

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

#include "Model_Splittransaction.h"
#include "Model_Category.h"
#include "Model_Subcategory.h"

Model_Splittransaction::Model_Splittransaction()
: Model<DB_Table_SPLITTRANSACTIONS>()
{
}

Model_Splittransaction::~Model_Splittransaction()
{
};

/**
* Initialize the global Model_Splittransaction table.
* Reset the Model_Splittransaction table or create the table if it does not exist.
*/
Model_Splittransaction& Model_Splittransaction::instance(wxSQLite3Database* db)
{
    Model_Splittransaction& ins = Singleton<Model_Splittransaction>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_Splittransaction table */
Model_Splittransaction& Model_Splittransaction::instance()
{
    return Singleton<Model_Splittransaction>::instance();
}

double Model_Splittransaction::get_total(const Data_Set& rows)
{
    double total = 0.0;
    for (auto& r : rows) total += r.SPLITTRANSAMOUNT;
    return total;
}
double Model_Splittransaction::get_total(const std::vector<Split>& rows)
{
    double total = 0.0;
    for (auto& r : rows) total += r.SPLITTRANSAMOUNT;
    return total;
}

std::map<int, Model_Splittransaction::Data_Set> Model_Splittransaction::get_all()
{
    std::map<int, Model_Splittransaction::Data_Set> data;
    for (const auto &split : instance().all())
    {
        data[split.TRANSID].push_back(split);
    }
    return data;
}

int Model_Splittransaction::update(const Data_Set& rows, int transactionID)
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
            split_item->SUBCATEGID = item.SUBCATEGID;
            split_items.push_back(*split_item);
        }
        instance().save(split_items);
    }
    return rows.size();
}

const wxString Model_Splittransaction::get_tooltip(const std::vector<Split>& rows, const Model_Currency::Data* currency)
{
    wxString split_tooltip = "";
    for (const auto& entry : rows)
        split_tooltip += wxString::Format("%s = %s\n"
        , Model_Category::full_name(entry.CATEGID, entry.SUBCATEGID)
        , Model_Currency::toCurrency(entry.SPLITTRANSAMOUNT, currency));
    return split_tooltip;
}
