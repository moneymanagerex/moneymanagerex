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

#include "Model_TransferTrans.h"

const std::vector<std::pair<Model_TransferTrans::TABLE_TYPE, wxString> > Model_TransferTrans::TABLE_TYPE_CHOICES = 
{
    std::make_pair(Model_TransferTrans::ASSETS, wxTRANSLATE("Assets"))
    , std::make_pair(Model_TransferTrans::STOCKS, wxTRANSLATE("Stocks"))
};

Model_TransferTrans::Model_TransferTrans()
: Model<DB_Table_TRANSFERTRANS_V1>()
{
}

Model_TransferTrans::~Model_TransferTrans()
{
}

/**
* Initialize the global Model_TransferTrans table.
* Reset the Model_TransferTrans table or create the table if it does not exist.
*/
Model_TransferTrans& Model_TransferTrans::instance(wxSQLite3Database* db)
{
    Model_TransferTrans& ins = Singleton<Model_TransferTrans>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_TransferTrans table */
Model_TransferTrans& Model_TransferTrans::instance()
{
    return Singleton<Model_TransferTrans>::instance();
}

wxArrayString Model_TransferTrans::all_table_type()
{
    wxArrayString table_type;
    for (const auto& item : TABLE_TYPE_CHOICES)
    {
        table_type.Add(item.second);
    }
    return table_type;
}

Model_TransferTrans::TABLE_TYPE Model_TransferTrans::type(const Data* r)
{
    for (const auto & item : TABLE_TYPE_CHOICES)
    {
        if (item.second.CmpNoCase(r->TABLE_TYPE) == 0)
            return item.first;
    }
    return TABLE_TYPE(-1);
}

Model_TransferTrans::TABLE_TYPE Model_TransferTrans::type(const Data& r)
{
    return type(&r);
}

Model_Currency::Data* Model_TransferTrans::currency(const Data* r)
{
    Model_Currency::Data * currency = Model_Currency::instance().get(r->ID_CURRENCY);
    if (!currency)
    {
        return Model_Currency::GetBaseCurrency();
    }
    return currency;
}
