/*******************************************************
 Copyright (C) 2013 Guan Lisheng

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

#ifndef MODEL_ACCOUNT_H
#define MODEL_ACCOUNT_H

#include "Model.h"
#include "db/DB_Table_Accountlist_V1.h"
#include "Model_Currency.h" // detect base currency
#include "Model_Checking.h"
#include "Model_Billsdeposits.h"
#include "Model_Stock.h"

class Model_Account : public Model, public DB_Table_ACCOUNTLIST_V1
{
    using DB_Table_ACCOUNTLIST_V1::all;
    using DB_Table_ACCOUNTLIST_V1::get;
    using DB_Table_ACCOUNTLIST_V1::save;
    using DB_Table_ACCOUNTLIST_V1::remove;
public:
    enum STATUS { OPEN = 0, CLOSED };
    enum TYPE  { CHECKING = 0, TERM, INVESTMENT };
public:
    Model_Account(): Model(), DB_Table_ACCOUNTLIST_V1() 
    {
    };
    ~Model_Account() {};

public:
    wxArrayString all_account_names()
    {
        wxSortedArrayString accounts;
        for (const auto &account : this->all(COL_ACCOUNTNAME))
        {
            accounts.Add(account.ACCOUNTNAME);
        }
        return accounts;
    }
    static wxArrayString all_status()
    {
        wxArrayString status;
        status.Add("Open");
        status.Add("Closed");

        return status;
    }
    static wxArrayString all_type()
    {
        wxArrayString type;
        type.Add("Checking");
        type.Add("Term");
        type.Add("Investment");

        return type;
    }

public:
    static Model_Account& instance()
    {
        return Singleton<Model_Account>::instance();
    }
    static Model_Account& instance(wxSQLite3Database* db)
    {
        Model_Account& ins = Singleton<Model_Account>::instance();
        ins.db_ = db;
        ins.destroy_cache();
        ins.all();
        return ins;
    }
public:
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        this->ensure(this->db_);
        return all(db_, col, asc);
    }
    template<typename... Args>
    Data_Set find(const Args&... args)
    {
        return find_by(this, db_, true, args...);
    }
    Data* get(int id)
    {
        return this->get(id, this->db_);
    }
    Data* get(const wxString& name)
    {
        Data* account = 0;
        Data_Set items = this->find(ACCOUNTNAME(name));
        if (!items.empty()) account = this->get(items[0].ACCOUNTID, this->db_);
        return account;
    }
    int save(Data* r)
    {
        r->save(this->db_);
        return r->id();
    }
    bool remove(int id)
    {
        for (const auto& r: Model_Checking::instance().find_or(Model_Checking::ACCOUNTID(id), Model_Checking::TOACCOUNTID(id)))
            Model_Checking::instance().remove(r.TRANSID);
        for (const auto& r: Model_Billsdeposits::instance().find_or(Model_Billsdeposits::ACCOUNTID(id), Model_Billsdeposits::TOACCOUNTID(id)))
            Model_Billsdeposits::instance().remove(r.BDID);

        return this->remove(id, db_);
    }

    static Model_Currency::Data* currency(const Data* r)
    {
        return Model_Currency::instance().get(r->CURRENCYID);
    }
    static Model_Currency::Data* currency(const Data& r) { return currency(&r); }
public:
    static Model_Checking::Data_Set transaction(const Data*r )
    {
        Model_Checking::Data_Set trans = Model_Checking::instance().find_or(Model_Checking::ACCOUNTID(r->ACCOUNTID), Model_Checking::TOACCOUNTID(r->ACCOUNTID));
        std::sort(trans.begin(), trans.end());
        std::stable_sort(trans.begin(), trans.end(), SorterByTRANSDATE());

        return trans;
	}
    static Model_Checking::Data_Set transaction(const Data& r) { return transaction(&r); }
    static Model_Billsdeposits::Data_Set billsdeposits(const Data* r)
    {
		return Model_Billsdeposits::instance().find_or(Model_Billsdeposits::ACCOUNTID(r->ACCOUNTID), Model_Billsdeposits::TOACCOUNTID(r->ACCOUNTID));
	}
    static Model_Billsdeposits::Data_Set billsdeposits(const Data& r) { return billsdeposits(&r); }
    static wxDate last_date(const Data* r)
    {
        Model_Checking::Data_Set trans = Model_Account::transaction(r);
        if (!trans.empty()) return Model_Checking::TRANSDATE(trans.back());
        
        return wxDateTime::Now().GetDateOnly();
    }
    static wxDate last_date(const Data& r) { return last_date(&r); }
    static double balance(const Data* r)
    {
        double sum = r->INITIALBAL;
        for (const auto& tran: transaction(r))
        {
           sum += Model_Checking::balance(tran, r->ACCOUNTID); 
        }
        return sum;
    }
    static double balance(const Data& r) { return balance(&r); }
    static std::pair<double, double> investment_balance(const Data* r)
    {
        std::pair<double /*origianl input value*/, double /**/> sum;
        for (const auto& stock: Model_Stock::instance().find(Model_Stock::HELDAT(r->ACCOUNTID)))
        {
            sum.first += stock.VALUE;
            sum.second += Model_Stock::value(stock);
        }
        return sum;
    }
    static std::pair<double, double> investment_balance(const Data& r) { return investment_balance(&r); }
    static wxString toCurrency(double value, const Data* r)
    {
        return Model_Currency::toCurrency(value, currency(r));
    }    
    static wxString toString(double value, const Data* r)
    {
        return Model_Currency::toString(value, currency(r));
    }
    static wxString toString(double value, const Data& r) { return toString(value, &r); }
public:
    static STATUS status(const Data* account)
    {
        if (account->STATUS.CmpNoCase(all_status()[OPEN]) == 0)
            return OPEN;
        return CLOSED;
    }
    static STATUS status(const Data& account)
    {
        return status(&account);
    }
    static TYPE type(const Data* account)
    {
        if (account->ACCOUNTTYPE.CmpNoCase(all_type()[CHECKING]) == 0)
            return CHECKING;
        else if (account->ACCOUNTTYPE.CmpNoCase(all_type()[TERM]) == 0)
            return TERM;
        else
            return INVESTMENT;
    }
    static TYPE type(const Data& account)
    {
        return type(&account);
    }
    static bool FAVORITEACCT(const Data* r)
    {
        return r->FAVORITEACCT.CmpNoCase("TRUE") == 0;
    }
    static bool FAVORITEACCT(const Data& r) { return FAVORITEACCT(&r); }
    static bool is_used(const Model_Currency::Data* c)
    {
        Data_Set accounts = Model_Account::instance().find(CURRENCYID(c->CURRENCYID));
        return !accounts.empty();
       
    }
    static bool is_used(const Model_Currency::Data& c)
    {
        return is_used(&c);
    }
    static int checking_account_num()
    {
        return Model_Account::instance().find(ACCOUNTTYPE(all_type()[CHECKING])).size();
    }
    static int investment_account_num()
    {
        return Model_Account::instance().find(ACCOUNTTYPE(all_type()[INVESTMENT])).size();
    }
};

#endif // 
