
#ifndef MODEL_SPLITTRANSACTIONS_H
#define MODEL_SPLITTRANSACTIONS_H

#include "Model.h"
#include "db/DB_Table_Splittransactions_V1.h"

class Model_Splittransaction : public Model, public DB_Table_SPLITTRANSACTIONS_V1
{
    using DB_Table_SPLITTRANSACTIONS_V1::all;
    using DB_Table_SPLITTRANSACTIONS_V1::find;
    using DB_Table_SPLITTRANSACTIONS_V1::get;
public:
    Model_Splittransaction(): Model(), DB_Table_SPLITTRANSACTIONS_V1() 
    {
    };
    ~Model_Splittransaction() {};

public:
    wxArrayString types_;

public:
    static Model_Splittransaction& instance()
    {
        return Singleton<Model_Splittransaction>::instance();
    }
    static Model_Splittransaction& instance(wxSQLite3Database* db)
    {
        Model_Splittransaction& ins = Singleton<Model_Splittransaction>::instance();
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
    Data_Set find(COLUMN col, const V& v)
    {
        return find(db_, col, v);
    }
    template<class V1, class V2>
    Data_Set find(COLUMN col1, const V1& v1, COLUMN col2, const V2& v2)
    {
        return find(db_, col1, v1, col2, v2);
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
    int save(Data_Set& rows)
    {
        this->Begin();
        for (auto& r : rows) this->save(&r);
        this->Commit();

        return rows.size();
    }
};

#endif // 
