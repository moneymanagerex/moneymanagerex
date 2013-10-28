#!/usr/bin/env python
# vi:tabstop=4:expandtab:shiftwidth=4:softtabstop=4:autoindent:smarttab
'''
Usage: python sqlite2cpp.py path_to_sql_file
'''

import sys
import os
import datetime
import sqlite3

# https://github.com/django/django/blob/master/django/db/backends/sqlite3/introspection.py
def get_table_list(cursor):
    "Returns a list of table names in the current database."
    # Skip the sqlite_sequence system table used for autoincrement key
    # generation.
    cursor.execute("""
        SELECT name, sql FROM sqlite_master
        WHERE type='table' AND NOT name='sqlite_sequence'
        ORDER BY name""")
    return [(row[0], row[1]) for row in cursor.fetchall()]


def _table_info(cursor, name):
    cursor.execute('PRAGMA table_info(%s)' % name)
    # cid, name, type, notnull, dflt_value, pk
    return [{'name': field[1],
        'type': field[2].upper(),
        'null_ok': not field[3],
        'pk': field[5]     # undocumented
        } for field in cursor.fetchall()]

base_data_types_reverse = {
    'TEXT': 'wxString',
    'NUMERIC': 'double',
    'INTEGER': 'int',
    'REAL': 'double',
    'BLOB': 'wxString',
    'DATE': 'wxDateTime',
}

base_data_types_function = {
    'TEXT': 'GetString',
    'NUMERIC': 'GetDouble',
    'INTEGER': 'GetInt',
    'REAL': 'GetDouble',
}

class DB_Table:
    def __init__(self, table, fields):
        self._table = table
        self._fields = fields
        self._primay_key = [field['name'] for field in self._fields if field['pk']][0]

    def generate_class(self, header, sql):
        fp = open('DB_Table_' + self._table.title() + '.h', 'w')
        fp.write(header + self.to_string(sql))
        fp.close()
    
    def to_string(self, sql = None):
        
        s = '''
#ifndef DB_TABLE_%s_H
#define DB_TABLE_%s_H

#include "DB_Table.h"

struct DB_Table_%s : public DB_Table
{
    struct Data;
    typedef DB_Table_%s Self;
    /** A List container to hold Data records for the table*/
    typedef std::vector<Self::Data> Data_Set;
    /** A List container to hold Data record pointers for the table*/
    typedef std::vector<Self::Data*> Cache;
    Cache cache_;
    ~DB_Table_%s() 
    {
        destroy_cache();
    }
    
    /** Removes all table data stored in memory*/ 
    void destroy_cache()
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fun(&Data::destroy));
        cache_.clear();
    }
''' % (self._table.upper(), self._table.upper(), self._table, self._table, self._table)
        
        s += '''
    /** Creates the database table if the table does not exist*/
    bool ensure(wxSQLite3Database* db) const
    {
        if (exists(db)) return true;

        try
        {
            db->ExecuteUpdate("%s");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("%s: Exception %%s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }
''' % (sql.replace('\n', ''), self._table)

        for field in self._fields:
            s += '''
    struct %s : public DB_Column<%s>
    { 
        static wxString name() { return "%s"; } 
        %s(const %s &v, OP op = EQUAL): DB_Column<%s>(v, op) {}
    };''' % (field['name'], base_data_types_reverse[field['type']], field['name']
            , field['name'], base_data_types_reverse[field['type']], base_data_types_reverse[field['type']])

        s += '''
    typedef %s PRIMARY;''' % self._primay_key

        s += '''
    enum COLUMN
    {
        COL_%s = 0''' % self._primay_key.upper()
        
        for index, name in enumerate([field['name'] for field in self._fields if not field['pk']]):
            s += '''
        , COL_%s = %d''' % (name.upper(), index +1)

        s +='''
    };
'''
        s += '''
    /** Returns the column name as a string*/
    wxString column_to_name(COLUMN col) const
    {
        switch(col)
        {
            case COL_%s: return "%s";''' % (self._primay_key.upper(), self._primay_key)

        for index, name in enumerate([field['name'] for field in self._fields if not field['pk']]):
            s += '''
            case COL_%s: return "%s";''' %(name.upper(), name)
        s +='''
            default: break;
        }
        
        return "UNKNOWN";
    }
'''
        s +='''
    /** Returns the comumn number from the given column name*/
    COLUMN name_to_column(const wxString& name) const
    {
        if ("%s" == name) return COL_%s;''' % (self._primay_key, self._primay_key.upper())

        for index, name in enumerate([field['name'] for field in self._fields if not field['pk']]):
            s += '''
        else if ("%s" == name) return COL_%s;''' %(name, name.upper())

        s += '''

        return COLUMN(-1);
    }
    '''
        s += '''
    /** Contains the table record for the table*/
    struct Data
    {
        friend struct DB_Table_%s;
        Self* view_;
    ''' % self._table.upper()
        for field in self._fields:
            s += '''
        %s %s;%s''' % (base_data_types_reverse[field['type']], field['name'], field['pk'] and '//  primay key' or '')

        s +='''
        int id() const { return %s; }
        void id(int id) { %s = id; }
        bool operator < (const Data& r) const
        {
            return this->id() < r.id();
        }
        bool operator < (const Data* r) const
        {
            return this->id() < r->id();
        }
''' % (self._primay_key, self._primay_key)
        
        s += '''
        Data(Self* view = 0) 
        {
            view_ = view;
        '''

        for field in self._fields:
            type = base_data_types_reverse[field['type']]
            if type == 'wxString': 
                continue
            elif type == 'double':
                s += '''
            %s = 0.0;''' % field['name']
            elif type == 'int':
                s += '''
            %s = -1;''' % field['name']
                

        s += '''
        }

        Data(wxSQLite3ResultSet& q, Self* view = 0)
        {
            view_ = view;
        '''
        for field in self._fields:
            func = base_data_types_function[field['type']]
            s += '''
            %s = q.%s("%s");''' % (field['name'], func, field['name'])

        s += '''
        }
'''
        s += '''
        wxString to_json() const
        {
            json::Object o;
            this->to_json(o);
            std::stringstream ss;
            json::Writer::Write(o, ss);
            return ss.str();
        }
        
        int to_json(json::Object& o) const
        {'''

        for field in self._fields:
            type = base_data_types_reverse[field['type']]
            if type == 'wxString':
                s += '''
            o["%s"] = json::String(this->%s.ToStdString());''' % (field['name'], field['name'])
            else:
                s += '''
            o["%s"] = json::Number(this->%s);''' % (field['name'], field['name'])
            
        s +='''
            return 0;
        }'''

        s += '''

        bool save(wxSQLite3Database* db)
        {
            if (!view_ || !db) 
            {
                wxLogError("can not save");
                return false;
            }

            return view_->save(this, db);
        }

        bool remove(wxSQLite3Database* db)
        {
            if (!view_ || !db) 
            {
                wxLogError("can not remove");
                return false;
            }
            
            return view_->remove(this, db);
        }

        void destroy() { delete this; }
    };
'''
        s +='''
    enum
    {
        NUM_COLUMNS = %d
    };

    size_t num_columns() const { return NUM_COLUMNS; }
''' % len(self._fields)

        s += '''
    /** Name of the table*/    
    wxString name() const { return "%s"; }
''' % self._table
        
        s +='''
    DB_Table_%s() 
    {
        query_ = "SELECT %s FROM %s ";
    }
''' % (self._table, ', '.join([field['name'] for field in self._fields]), self._table)
        
        s +='''
    /** Create a new data record*/
    Self::Data* create()
    {
        Self::Data* entity = new Self::Data(this);
        cache_.push_back(entity);
        return entity;
    }
    
    /** Create a copy of the data record*/
    Self::Data* clone(const Data* e)
    {
        Self::Data* entity = create();
        *entity = *e;
        entity->id(-1);
        return entity;
    }
'''
        s +='''
    /** Save the data record to the database, either create new or update the existing record*/
    bool save(Self::Data* entity, wxSQLite3Database* db)
    {
        wxString sql = wxEmptyString;
        if (entity->id() < 0) //  new & insert
        {
            sql = "INSERT INTO %s(%s) VALUES(%s)";
        }''' % (self._table, ', '.join([field['name'] for field in self._fields if not field['pk']]), ', '.join(['?' for field in self._fields if not field['pk']]))
        
        s +='''
        else
        {
            sql = "UPDATE %s SET %s WHERE %s = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);
''' % (self._table, ', '.join([field['name'] + ' = ?' for field in self._fields if not field['pk']]), self._primay_key)
        
        for index, name in enumerate([field['name'] for field in self._fields if not field['pk']]):
            s +='''
            stmt.Bind(%d, entity->%s);'''% (index + 1, name)
            
        
        s +='''
            if (entity->id() > 0)
                stmt.Bind(%d, entity->%s);

            wxLogDebug(stmt.GetSQL());
            stmt.ExecuteUpdate();
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("%s: Exception %%s, %%s", e.GetMessage().c_str(), entity->to_json());
            return false;
        }

        if (entity->id() < 0) entity->id((db->GetLastRowId()).ToLong());
        return true;
    }
''' % (len(self._fields), self._primay_key, self._table)
        s +='''
    bool remove(int id, wxSQLite3Database* db)
    {
        try
        {
            wxString sql = "DELETE FROM %s WHERE %s = ?";
            wxSQLite3Statement stmt = db->PrepareStatement(sql);
            stmt.Bind(1, id);
            wxLogDebug(stmt.GetSQL());
            stmt.ExecuteUpdate();
            stmt.Finalize();

            Cache c;
            for(Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it)
            {
                Self::Data* entity = *it;
                if (entity->id() == id) 
                    delete entity;
                else 
                    c.push_back(entity);
            }
            cache_.clear();
            cache_.swap(c);
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("%s: Exception %%s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    bool remove(Self::Data* entity, wxSQLite3Database* db)
    {
        if (remove(entity->id(), db))
        {
            entity->id(-1);
            return true;
        }

        return false;
    }
''' % (self._table, self._primay_key, self._table)
        
        s +='''
    
    Self::Data* get(int id, wxSQLite3Database* db)
    {
        if (id < 0) 
        {
            ++ skip_;
            wxLogDebug("%s :%d SKIP (hit %ld, miss %ld, skip %ld)", this->name(), id, this->hit_, this->miss_, this->skip_);
            return 0;
        }
        for(Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it)
        {
            Self::Data* entity = *it;
            if (entity->id() == id) 
            {
                ++ hit_;
                wxLogDebug("%s :%d HIT (hit %ld, miss %ld, skip %ld)", this->name(), id, this->hit_, this->miss_, this->skip_);
                return entity;
            }
        }
        
        ++ miss_;
        wxLogDebug("%s :%d MISS (hit %ld, miss %ld, skip %ld)", this->name(), id, this->hit_, this->miss_, this->skip_);
        Self::Data* entity = 0;
        wxString where = wxString::Format(" WHERE %s = ?", PRIMARY::name().c_str());
        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(this->query() + where);
            stmt.Bind(1, id);

            wxLogDebug(stmt.GetSQL());
            wxSQLite3ResultSet q = stmt.ExecuteQuery();
            if(q.NextRow())
            {
                entity = new Self::Data(q, this);
                cache_.push_back(entity);
            }
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("%s: Exception %s", this->name().c_str(), e.GetMessage().c_str());
        }
        
        if (!entity) 
        {
            wxLogError("%s: %d not found", this->name().c_str(), id);
        }
 
        return entity;
    }
'''
        s +='''
    /** Return a list of all the records in the database*/
    Data_Set all(wxSQLite3Database* db, COLUMN col = COLUMN(0), bool asc = true)
    {
        Data_Set result;
        try
        {
            wxSQLite3ResultSet q = db->ExecuteQuery(this->query() + " ORDER BY " + column_to_name(col) + " COLLATE NOCASE " + (asc ? " ASC " : " DESC ")
                + "," + PRIMARY::name());

            wxLogDebug(q.GetSQL());
            while(q.NextRow())
            {
                Self::Data entity(q, this);
                result.push_back(entity);
            }

            q.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("%s: Exception %s", this->name().c_str(), e.GetMessage().c_str());
        }

        return result;
    }
'''

        s += '''
};
#endif //
''' 
        return s

def generate_base_class(header, fields=set):
    fp = open('DB_Table.h', 'w')
    code = header + '''
#ifndef DB_TABLE_H
#define DB_TABLE_H

#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <wx/wxsqlite3.h>

#include "cajun/json/elements.h"
#include "cajun/json/reader.h"
#include "cajun/json/writer.h"

class wxString;
enum OP { EQUAL = 0, GREATER, LESS, GREATER_OR_EQUAL, LESS_OR_EQUAL, NOT_EQUAL };
template<class V>
struct DB_Column
{
    V v_;
    OP op_;
    DB_Column(const V& v, OP op = EQUAL): v_(v), op_(op)
    {}
};

struct DB_Table
{
    DB_Table(): hit_(0), miss_(0), skip_(0) {};
    virtual ~DB_Table() {};
    wxString query_;
    size_t hit_, miss_, skip_;
    virtual wxString query() const { return this->query_; }
    virtual size_t num_columns() const = 0;
    virtual wxString name() const = 0;

    bool exists(wxSQLite3Database* db) const
    {
       return db->TableExists(this->name()); 
    }
};

template<typename Arg1>
void condition(wxString& out, bool op_and, const Arg1& arg1)
{
    out += Arg1::name();
    switch (arg1.op_)
    {
    case GREATER:           out += " > ? ";     break;
    case GREATER_OR_EQUAL:  out += " >= ? ";    break;
    case LESS:              out += " < ? ";     break;
    case LESS_OR_EQUAL:     out += " <= ? ";    break;
    case NOT_EQUAL:         out += " != ? ";    break;
    default:
        out += " = ? "; break;
    }
}

template<typename Arg1, typename... Args>
void condition(wxString& out, bool op_and, const Arg1& arg1, const Args&... args) 
{
    out += Arg1::name();
    switch (arg1.op_)
    {
    case GREATER:           out += " > ? ";     break;
    case GREATER_OR_EQUAL:  out += " >= ? ";    break;
    case LESS:              out += " < ? ";     break;
    case LESS_OR_EQUAL:     out += " <= ? ";    break;
    case NOT_EQUAL:         out += " != ? ";    break;
    default:
        out += " = ? "; break;
    }
    out += op_and? " AND " : " OR ";
    condition(out, op_and, args...);
}

template<typename Arg1>
void bind(wxSQLite3Statement& stmt, int index, const Arg1& arg1)
{
    stmt.Bind(index, arg1.v_);
}

template<typename Arg1, typename... Args>
void bind(wxSQLite3Statement& stmt, int index, const Arg1& arg1, const Args&... args)
{
    stmt.Bind(index, arg1.v_); 
    bind(stmt, index+1, args...);
}

template<typename TABLE, typename... Args>
typename TABLE::Data_Set find_by(TABLE* table, wxSQLite3Database* db, bool op_and, const Args&... args)
{
    typename TABLE::Data_Set result;
    try
    {
        wxString query = table->query() + " WHERE ";
        condition(query, op_and, args...);
        wxSQLite3Statement stmt = db->PrepareStatement(query);
        bind(stmt, 1, args...);

        wxLogDebug(stmt.GetSQL());
        wxSQLite3ResultSet q = stmt.ExecuteQuery();

        while(q.NextRow())
        {
            typename TABLE::Data entity(q, table);
            result.push_back(entity);
        }

        q.Finalize();
    }
    catch(const wxSQLite3Exception &e) 
    { 
        wxLogError("%s: Exception %s", table->name().c_str(), e.GetMessage().c_str());
    }
 
    return result;
}
'''
    for field in fields:
        code += '''
struct SorterBy%s
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return x.%s < y.%s;
    }
};
''' % (field, field, field)

    code += '''
#endif // 
'''
    fp = open('db_table.h', 'w')
    fp.write(code)
    fp.close

if __name__ == '__main__':
    header =  '''// -*- C++ -*-
//=============================================================================
/**
 *      Copyright (c) 2013 Guan Lisheng (guanlisheng@gmail.com)
 *
 *      @file
 *
 *      @author [%s]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at %s.
 *          DO NOT EDIT!
 */
//=============================================================================
'''% (os.path.basename(__file__), str(datetime.datetime.now()))

    conn, cur, sql_file = None, None, None
    try:
        sql_file = sys.argv[1]
        conn = sqlite3.connect(":memory:")
        conn.row_factory = sqlite3.Row 
        cur = conn.cursor()
    except:
        print __doc__
        sys.exit(1)

    sql = ""
    for line in open(sql_file, 'rb'):
        sql = sql + line;

    cur.executescript(sql)
  
    all_fields = set()
    for table, sql in get_table_list(cur):
        fields = _table_info(cur, table)
        view = DB_Table(table, fields)
        view.generate_class(header, sql)
        for field in fields:
            all_fields.add(field['name'])

    generate_base_class(header, all_fields)

    conn.close()

