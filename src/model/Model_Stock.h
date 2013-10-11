
#ifndef MODEL_STOCK_H
#define MODEL_STOCK_H

#include "Model.h"
#include "db/DB_Table_Stock_V1.h"
#include "Model_Currency.h" // detect base currency

class Model_Stock : public Model, public DB_Table_STOCK_V1
{
    using DB_Table_STOCK_V1::all;
    using DB_Table_STOCK_V1::get;
    using DB_Table_STOCK_V1::save;
    using DB_Table_STOCK_V1::find;
    using DB_Table_STOCK_V1::remove;
public:
    Model_Stock(): Model(), DB_Table_STOCK_V1() {};
    ~Model_Stock() {};

public:
    static Model_Stock& instance()
    {
        return Singleton<Model_Stock>::instance();
    }
    static Model_Stock& instance(wxSQLite3Database* db)
    {
        Model_Stock& ins = Singleton<Model_Stock>::instance();
        ins.db_ = db;
        return ins;
    }
public:
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        this->ensure(this->db_);
        return all(db_, col, asc);
    }
    template<class V>
    Data_Set find(const V& v)
    {
        return find(db_, v);
    }
    Data* get(int id)
    {
        return this->get(id, this->db_);
    }
    int save(Data* r)
    {
        r->save(this->db_);
        return r->id();
    }
    bool remove(int id)
    {
        return this->remove(id, db_);
    }

public:
    static wxDate PURCHASEDATE(const Data* stock) { return Model::to_date(stock->PURCHASEDATE); }
    static wxDate PURCHASEDATE(const Data& stock) { return Model::to_date(stock.PURCHASEDATE); }
public:
    static double value(const Data* r)
    {
        return r->NUMSHARES * r->PURCHASEPRICE + r->COMMISSION;
    }
    static double value(const Data& r) { return value(&r); }
};

#endif // 
