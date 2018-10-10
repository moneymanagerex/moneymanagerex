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

#include "Model_Asset.h"
#include <cmath>

const std::vector<std::pair<Model_Asset::RATE, wxString> > Model_Asset::RATE_CHOICES = 
{
    {Model_Asset::RATE_NONE, wxString(wxTRANSLATE("None"))}
    , {Model_Asset::RATE_APPRECIATE, wxString(wxTRANSLATE("Appreciates"))}
    , {Model_Asset::RATE_DEPRECIATE, wxString(wxTRANSLATE("Depreciates"))}
};

const std::vector<std::pair<Model_Asset::TYPE, wxString> > Model_Asset::TYPE_CHOICES = 
{
    {Model_Asset::TYPE_PROPERTY, wxString(wxTRANSLATE("Property"))}
    , {Model_Asset::TYPE_AUTO, wxString(wxTRANSLATE("Automobile"))}
    , {Model_Asset::TYPE_HOUSE, wxString(wxTRANSLATE("Household Object"))}
    , {Model_Asset::TYPE_ART, wxString(wxTRANSLATE("Art"))}
    , {Model_Asset::TYPE_JEWELLERY, wxString(wxTRANSLATE("Jewellery"))}
    , {Model_Asset::TYPE_CASH, wxString(wxTRANSLATE("Cash"))}
    , {Model_Asset::TYPE_OTHER, wxString(wxTRANSLATE("Other"))}
};

Model_Asset::Model_Asset()
: Model<DB_Table_ASSETS>()
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

wxArrayString Model_Asset::all_rate()
{
    static wxArrayString rates;
    if (rates.empty())
    {
        for (const auto& item : RATE_CHOICES)
            rates.Add(item.second);
    }
    return rates;
}

wxArrayString Model_Asset::all_type()
{
    static wxArrayString types;
    if (types.empty())
    {
        for (const auto& item : TYPE_CHOICES)
            types.Add(item.second);
    }
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

DB_Table_ASSETS::ASSETTYPE Model_Asset::ASSETTYPE(TYPE type, OP op)
{
    return DB_Table_ASSETS::ASSETTYPE(all_type()[type], op);
}

DB_Table_ASSETS::STARTDATE Model_Asset::STARTDATE(const wxDate& date, OP op)
{
    return DB_Table_ASSETS::STARTDATE(date.FormatISODate(), op);
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
    for (const auto& item : TYPE_CHOICES) if (item.second.CmpNoCase(r->ASSETTYPE) == 0) return item.first;

    return TYPE_UNKNOWN;
}

Model_Asset::TYPE Model_Asset::type(const Data& r)
{
    return type(&r);
}

Model_Asset::RATE Model_Asset::rate(const Data* r)
{
    for (const auto & item : RATE_CHOICES) if (item.second.CmpNoCase(r->VALUECHANGE) == 0) return item.first;
    return RATE_UNKNOWN;
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
    const wxDate today = wxDate::Today();
	wxTimeSpan diff_time = today - start_date;
	double diff_time_in_days = static_cast<double>(diff_time.GetDays());
    switch (rate(r))
    {
    case RATE_NONE:
        break;
    case RATE_APPRECIATE:
        sum *= pow(1.0 + (r->VALUECHANGERATE / 36500.0), diff_time_in_days);
        break;
    case RATE_DEPRECIATE:
        sum *= pow(1.0 - (r->VALUECHANGERATE / 36500.0), diff_time_in_days);
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
