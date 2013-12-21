/*******************************************************
 Copyright (C) 2013 Guan Lisheng (guanlisheng@gmail.com)

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

#include "Model_Asset.h"

Model_Asset::Model_Asset()
: Model<DB_Table_ASSETS_V1>()
{
}

Model_Asset::~Model_Asset()
{
}

/**
* Initialize the global Model_Asset table.
* Reset the Model_Asset table or create the table if it does not exist.
*/
Model_Asset& Model_Asset::instance(wxSQLite3Database* db)
{
    Model_Asset& ins = Singleton<Model_Asset>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_Asset table */
Model_Asset& Model_Asset::instance()
{
    return Singleton<Model_Asset>::instance();
}

wxString Model_Asset::version()
{
    return "$Rev$";
}

wxArrayString Model_Asset::all_rate()
{
    wxArrayString rates;
    //  keep the sequence with RATE
    rates.Add(wxTRANSLATE("None"));
    rates.Add(wxTRANSLATE("Appreciates"));
    rates.Add(wxTRANSLATE("Depreciates"));

    return rates;
}

wxArrayString Model_Asset::all_type()
{
    wxArrayString types;
    // keep the sequence with TYPE
    types.Add(wxTRANSLATE("Property"));
    types.Add(wxTRANSLATE("Automobile"));
    types.Add(wxTRANSLATE("Household Object"));
    types.Add(wxTRANSLATE("Art"));
    types.Add(wxTRANSLATE("Jewellery"));
    types.Add(wxTRANSLATE("Cash"));
    types.Add(wxTRANSLATE("Other"));

    return types;
}

double Model_Asset::balance()
{
    double balance = 0.0;
    for (const auto& r: this->all())
    {
        balance += value(r);
    }
    return balance;
}

DB_Table_ASSETS_V1::ASSETTYPE Model_Asset::ASSETTYPE(TYPE type, OP op)
{
    return DB_Table_ASSETS_V1::ASSETTYPE(all_type()[type], op);
}

DB_Table_ASSETS_V1::STARTDATE Model_Asset::STARTDATE(const wxDate& date, OP op)
{
    return DB_Table_ASSETS_V1::STARTDATE(date.FormatISODate(), op);
}

wxDate Model_Asset::STARTDATE(const Data* r)
{
    return Model::to_date(r->STARTDATE);
}

wxDate Model_Asset::STARTDATE(const Data& r)
{
    return Model::to_date(r.STARTDATE);
}

Model_Asset::TYPE Model_Asset::type(const Data* r)
{
    if (r->ASSETTYPE.CmpNoCase(all_type()[TYPE_PROPERTY]) == 0)
        return TYPE_PROPERTY;
    else if (r->ASSETTYPE.CmpNoCase(all_type()[TYPE_AUTO]) == 0)
        return TYPE_AUTO;
    else if (r->ASSETTYPE.CmpNoCase(all_type()[TYPE_HOUSE]) == 0)
        return TYPE_HOUSE;
    else if (r->ASSETTYPE.CmpNoCase(all_type()[TYPE_ART]) == 0)
        return TYPE_ART;
    else if (r->ASSETTYPE.CmpNoCase(all_type()[TYPE_JEWELLERY]) == 0)
        return TYPE_JEWELLERY;
    else if (r->ASSETTYPE.CmpNoCase(all_type()[TYPE_CASH]) == 0)
        return TYPE_CASH;
    else if (r->ASSETTYPE.CmpNoCase(all_type()[TYPE_OTHER]) == 0)
        return TYPE_OTHER;

    return TYPE(-1);
}

Model_Asset::TYPE Model_Asset::type(const Data& r)
{
    return type(&r);
}

Model_Asset::RATE Model_Asset::rate(const Data* r)
{
    if (r->VALUECHANGE.CmpNoCase(all_rate()[RATE_NONE]) == 0)
        return RATE_NONE;
    else if (r->VALUECHANGE.CmpNoCase(all_rate()[RATE_APPRECIATE]) == 0)
        return RATE_APPRECIATE;
    else if (r->VALUECHANGE.CmpNoCase(all_rate()[RATE_DEPRECIATE]) == 0)
        return RATE_DEPRECIATE;

    return RATE(-1);
}

Model_Asset::RATE Model_Asset::rate(const Data& r)
{
    return rate(&r);
}

Model_Currency::Data* Model_Asset::currency(const Data* /* r */)
{
    return Model_Currency::instance().GetBaseCurrency();
}

double Model_Asset::value(const Data* r)
{
    double sum = r->VALUE;
    wxDate start_date = STARTDATE(r);
    int diff_days = abs(start_date.Subtract(wxDateTime::Now()).GetDays());
    switch (rate(r))
    {
    case RATE_NONE:
        break;
    case RATE_APPRECIATE:
        sum += ((r->VALUE * (r->VALUECHANGERATE/100))/365.25) * diff_days;
        break;
    case RATE_DEPRECIATE:
        sum -= ((r->VALUE * (r->VALUECHANGERATE/100))/365.25) * diff_days;
        break;
    default:
        break;
    }

    return sum;
}

double Model_Asset::value(const Data& r)
{
    return value(&r);
}
