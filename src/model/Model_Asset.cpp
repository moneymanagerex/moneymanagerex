/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
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

#include "Model_Asset.h"
#include "Model_Translink.h"
#include "Model_CurrencyHistory.h"

const std::vector<std::pair<Model_Asset::TYPE_ID, wxString> > Model_Asset::TYPE_CHOICES = 
{
    { Model_Asset::TYPE_ID_PROPERTY,  wxString(wxTRANSLATE("Property")) },
    { Model_Asset::TYPE_ID_AUTO,      wxString(wxTRANSLATE("Automobile")) },
    { Model_Asset::TYPE_ID_HOUSE,     wxString(wxTRANSLATE("Household Object")) },
    { Model_Asset::TYPE_ID_ART,       wxString(wxTRANSLATE("Art")) },
    { Model_Asset::TYPE_ID_JEWELLERY, wxString(wxTRANSLATE("Jewellery")) },
    { Model_Asset::TYPE_ID_CASH,      wxString(wxTRANSLATE("Cash")) },
    { Model_Asset::TYPE_ID_OTHER,     wxString(wxTRANSLATE("Other")) }
};

const std::vector<std::pair<Model_Asset::STATUS_ID, wxString> > Model_Asset::STATUS_CHOICES = 
{
    { Model_Asset::STATUS_ID_CLOSED, wxString(wxTRANSLATE("Closed")) },
    { Model_Asset::STATUS_ID_OPEN,   wxString(wxTRANSLATE("Open")) }
};

const std::vector<std::pair<Model_Asset::CHANGE_ID, wxString> > Model_Asset::CHANGE_CHOICES = 
{
    { Model_Asset::CHANGE_ID_NONE,       wxString(wxTRANSLATE("None")) },
    { Model_Asset::CHANGE_ID_APPRECIATE, wxString(wxTRANSLATE("Appreciates")) },
    { Model_Asset::CHANGE_ID_DEPRECIATE, wxString(wxTRANSLATE("Depreciates")) }
};

const std::vector<std::pair<Model_Asset::CHANGEMODE_ID, wxString> > Model_Asset::CHANGEMODE_CHOICES = 
{
    { Model_Asset::CHANGEMODE_ID_PERCENTAGE, wxString(wxTRANSLATE("Percentage")) },
    { Model_Asset::CHANGEMODE_ID_LINEAR,     wxString(wxTRANSLATE("Linear")) }
};

wxArrayString Model_Asset::TYPE_STR = type_str_all();
wxArrayString Model_Asset::STATUS_STR = status_str_all();
wxArrayString Model_Asset::CHANGE_STR = change_str_all();
wxArrayString Model_Asset::CHANGEMODE_STR = changemode_str_all();

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

wxString Model_Asset::get_asset_name(int asset_id)
{
    Data* asset = instance().get(asset_id);
    if (asset)
        return asset->ASSETNAME;
    else
        return _("Asset Error");
}

wxArrayString Model_Asset::type_str_all()
{
    wxArrayString types;
    int i = 0;
    for (const auto& item: TYPE_CHOICES)
    {
        wxASSERT_MSG(item.first == i++, "Wrong order in Model_Asset::TYPE_CHOICES");
        types.Add(item.second);
    }
    return types;
}

wxArrayString Model_Asset::status_str_all()
{
    wxArrayString statusList;
    int i = 0;
    for (const auto& item: STATUS_CHOICES)
    {
        wxASSERT_MSG(item.first == i++, "Wrong order in Model_Asset::STATUS_CHOICES");
        statusList.Add(item.second);
    }
    return statusList;
}

wxArrayString Model_Asset::change_str_all()
{
    wxArrayString rates;
    int i = 0;
    for (const auto& item: CHANGE_CHOICES)
    {
        wxASSERT_MSG(item.first == i++, "Wrong order in Model_Asset::CHANGE_CHOICES");
        rates.Add(item.second);
    }
    return rates;
}

wxArrayString Model_Asset::changemode_str_all()
{
    wxArrayString changemodes;
    int i = 0;
    for (const auto& item: CHANGEMODE_CHOICES)
    {
        wxASSERT_MSG(item.first == i++, "Wrong order in Model_Asset::CHANGEMODE_CHOICES");
        changemodes.Add(item.second);
    }
    return changemodes;
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

DB_Table_ASSETS_V1::ASSETTYPE Model_Asset::ASSETTYPE(TYPE_ID type, OP op)
{
    return DB_Table_ASSETS_V1::ASSETTYPE(TYPE_STR[type], op);
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

Model_Asset::TYPE_ID Model_Asset::type_id(const Data* r)
{
    for (const auto& item : TYPE_CHOICES)
        if (item.second.CmpNoCase(r->ASSETTYPE) == 0) return item.first;
    return TYPE_ID(-1);
}
Model_Asset::TYPE_ID Model_Asset::type_id(const Data& r)
{
    return type_id(&r);
}

Model_Asset::STATUS_ID Model_Asset::status_id(const Data* r)
{
    for (const auto & item : STATUS_CHOICES)
        if (item.second.CmpNoCase(r->ASSETSTATUS) == 0) return item.first;
    return STATUS_ID(-1);
}
Model_Asset::STATUS_ID Model_Asset::status_id(const Data& r)
{
    return status_id(&r);
}

Model_Asset::CHANGE_ID Model_Asset::change_id(const Data* r)
{
    for (const auto & item : CHANGE_CHOICES)
        if (item.second.CmpNoCase(r->VALUECHANGE) == 0) return item.first;
    return CHANGE_ID(-1);
}
Model_Asset::CHANGE_ID Model_Asset::change_id(const Data& r)
{
    return change_id(&r);
}

Model_Asset::CHANGEMODE_ID Model_Asset::changemode_id(const Data* r)
{
    for (const auto & item : CHANGEMODE_CHOICES)
        if (item.second.CmpNoCase(r->VALUECHANGEMODE) == 0) return item.first;
    return CHANGEMODE_ID(-1);
}
Model_Asset::CHANGEMODE_ID Model_Asset::changemode_id(const Data& r)
{
    return changemode_id(&r);
}

Model_Currency::Data* Model_Asset::currency(const Data* /* r */)
{
    return Model_Currency::instance().GetBaseCurrency();
}

double Model_Asset::value(const Data* r)
{
    return instance().valueAtDate(r, wxDate::Today());
}

double Model_Asset::value(const Data& r)
{
    return instance().valueAtDate(&r, wxDate::Today());
}

double Model_Asset::valueAtDate(const Data* r, const wxDate date)
{
    double balance = 0;
    if (date >= STARTDATE(r)) {
        Model_Translink::Data_Set translink_records = Model_Translink::instance().find(Model_Translink::LINKRECORDID(r->ASSETID), Model_Translink::LINKTYPE(Model_Attachment::reftype_desc(Model_Attachment::ASSET)));
        if (!translink_records.empty())
        {
            for (const auto& link : translink_records)
            {
                const Model_Checking::Data* tran = Model_Checking::instance().get(link.CHECKINGACCOUNTID);
                const wxDate tranDate = Model_Checking::TRANSDATE(tran);
                if (tranDate <= date)
                {
                    double amount = -1 * Model_Checking::balance(tran, tran->ACCOUNTID) *
                        Model_CurrencyHistory::getDayRate(Model_Account::instance().get(tran->ACCOUNTID)->CURRENCYID, tranDate);
                    wxTimeSpan diff_time = date - tranDate;
                    double diff_time_in_days = static_cast<double>(diff_time.GetDays());

                    switch (change_id(r))
                    {
                    case CHANGE_ID_NONE:
                        break;
                    case CHANGE_ID_APPRECIATE:
                        amount *= pow(1.0 + (r->VALUECHANGERATE / 36500.0), diff_time_in_days);
                        break;
                    case CHANGE_ID_DEPRECIATE:
                        amount *= pow(1.0 - (r->VALUECHANGERATE / 36500.0), diff_time_in_days);
                        break;
                    default:
                        break;
                    }

                    balance += amount;
                }
            }
        }
        else {
            balance = r->VALUE;
            wxTimeSpan diff_time = date - STARTDATE(r);
            double diff_time_in_days = static_cast<double>(diff_time.GetDays());

            switch (change_id(r))
            {
            case CHANGE_ID_NONE:
                break;
            case CHANGE_ID_APPRECIATE:
                balance *= pow(1.0 + (r->VALUECHANGERATE / 36500.0), diff_time_in_days);
                break;
            case CHANGE_ID_DEPRECIATE:
                balance *= pow(1.0 - (r->VALUECHANGERATE / 36500.0), diff_time_in_days);
                break;
            default:
                break;
            }
        }
    }
    return balance;
}
