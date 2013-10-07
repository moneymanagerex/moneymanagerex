
#ifndef MODEL_SUBCATEGORY_H
#define MODEL_SUBCATEGORY_H

#include "Model.h"
#include "db/DB_Table_Subcategory_V1.h"
#include "Model_Category.h"

class Model_Subcategory : public Model, public DB_Table_SUBCATEGORY_V1
{
    using DB_Table_SUBCATEGORY_V1::all;
    using DB_Table_SUBCATEGORY_V1::find;
    using DB_Table_SUBCATEGORY_V1::get;
    using DB_Table_SUBCATEGORY_V1::remove;
public:
    Model_Subcategory(): Model(), DB_Table_SUBCATEGORY_V1() 
    {
    };
    ~Model_Subcategory() {};

public:
    wxArrayString types_;

public:
    static Model_Subcategory& instance()
    {
        return Singleton<Model_Subcategory>::instance();
    }
    static Model_Subcategory& instance(wxSQLite3Database* db)
    {
        Model_Subcategory& ins = Singleton<Model_Subcategory>::instance();
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
    Data* get(int id)
    {
        return this->get(id, this->db_);
    }
    Data* get(const wxString& name, int category_id = -1)
    {
        Data* category = 0;
        Data_Set items = this->find(this->db_, COL_SUBCATEGNAME, name, COL_CATEGID, category_id);
        if (!items.empty()) category = this->get(items[0].SUBCATEGID, this->db_);
        return category;
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
    static bool is_used(int /* id */)
    {
        // TODO
        return true;
    }
};

#endif // 
