#!/usr/bin/env python
# vim: tabstop=4 shiftwidth=4 softtabstop=4 expandtab autoindent smarttab foldmethod=marker
'''
Usage: python sqlite2cpp.py path_to_sql_file
'''

# {{{ import

import sys
import os
import datetime
import sqlite3
import codecs

# }}}
# {{{ const

# existing files
base_class_name = 'TableBase'
base_file_basename = '_TableBase'

# generated files
tables_sql_filename = 'tables_en.sql'
patch_currency_filename = 'patch_currency.sql'
patch_currency_utf8_filename = 'patch_currency_utf8.sql'

# convert DB table name to C++ class basename
table_class_basename = {
    'ACCOUNTLIST_V1'             : 'Account',
    'ASSETS_V1'                  : 'Asset',
    'ATTACHMENT_V1'              : 'Attachment',
    'BILLSDEPOSITS_V1'           : 'Scheduled',
    'BUDGETSPLITTRANSACTIONS_V1' : 'ScheduledSplit',
    'BUDGETTABLE_V1'             : 'Budget',
    'BUDGETYEAR_V1'              : 'BudgetPeriod',
    'CATEGORY_V1'                : 'Category',
    'CHECKINGACCOUNT_V1'         : 'Transaction',
    'CURRENCYFORMATS_V1'         : 'Currency',
    'CURRENCYHISTORY_V1'         : 'CurrencyHistory',
    'CUSTOMFIELDDATA_V1'         : 'FieldValue',
    'CUSTOMFIELD_V1'             : 'Field',
    'INFOTABLE_V1'               : 'Info',
    'PAYEE_V1'                   : 'Payee',
    'REPORT_V1'                  : 'Report',
    'SETTING_V1'                 : 'Setting',
    'SHAREINFO_V1'               : 'TransactionShare',
    'SPLITTRANSACTIONS_V1'       : 'TransactionSplit',
    'STOCKHISTORY_V1'            : 'StockHistory',
    'STOCK_V1'                   : 'Stock',
    'TAGLINK_V1'                 : 'TagLink',
    'TAG_V1'                     : 'Tag',
    'TRANSLINK_V1'               : 'TransactionLink',
    'USAGE_V1'                   : 'Usage',
}

# convert DB type to C++ type
dbtype_ctype = {
    'TEXT'    : 'wxString',
    'NUMERIC' : 'double',
    'INTEGER' : 'int64',
    'REAL'    : 'double',
    'BLOB'    : 'wxString',
    'DATE'    : 'wxDateTime',
}

# convert DB type to C++ function
dbtype_function = {
    'TEXT'    : 'GetString',
    'NUMERIC' : 'GetDouble',
    'INTEGER' : 'GetInt64',
    'REAL'    : 'GetDouble',
}

# }}}

# {{{ def is_ascii(s)

# http://stackoverflow.com/questions/196345/how-to-check-if-a-string-in-python-is-in-ascii
def is_ascii(s):
    """Class: Check for Ascii String"""
    if isinstance(s, str):
        return all(ord(c) < 128 for c in s)
    return False

# }}}
# {{{ def is_trans(s)

def is_trans(s):
    """Check translation requirements for cpp"""
    if isinstance(s, int):
        return False

    if not is_ascii(s): # it is unicode string
        return True

    if len(s) > 4 and s[0:4] == "_tr_": # requires wxTRANSLATE for cpp
        return True

    return False

# }}}
# {{{ def adjust_translate(s)

def adjust_translate(s):
    """Return the correct translated syntax for c++"""
    trans_str = s.replace("_tr_", "").replace('"','')
    trans_str = '_("' + trans_str + '")'

    return trans_str

# }}}
# {{{ def translation_for(s)

def translation_for(s):
    """Return the correct translated syntax for c++"""
    trans_str = ''
    if not is_ascii(s):  # it is unicode string
        if len(s) > 4 and s[0:4] == "_tr_": # requires wxTRANSLATE for cpp
            trans_str = adjust_translate(s)
        else:
            trans_str = 'L"%s"' % s
    else:
        trans_str = adjust_translate(s)

    return trans_str

# }}}

# {{{ def get_table_a(cursor)

# https://github.com/django/django/blob/master/django/db/backends/sqlite3/introspection.py
def get_table_a(cursor):
    "Returns a list of table (name, sql) in database."
    # Skip the sqlite_sequence system table used for autoincrement key
    # generation.
    cursor.execute("""
        SELECT name, sql FROM sqlite_master
        WHERE type='table' AND NOT name='sqlite_sequence'
        ORDER BY name""")
    return [(row[0], row[1]) for row in cursor.fetchall()]


# }}}
# {{{ def get_table_index_a(cursor, table_name)

def get_table_index_a(cursor, table_name):
    "Returns a list of index (name, sql) for table_name."
    # Skip the sqlite_sequence system table used for autoincrement key
    # generation.
    cursor.execute("""
        SELECT name, sql FROM sqlite_master
        WHERE type='index' AND name NOT LIKE 'sqlite_autoindex_%%' AND tbl_name = '%s'
        ORDER BY name""" % table_name)
    return [(row[0], row[1]) for row in cursor.fetchall()]

# }}}
# {{{ def get_table_field_a(cursor, table_name)

def get_table_field_a(cursor, table_name):
    "Returns a list of field (cid, name, type, null_ok, pk) for table_name."
    cursor.execute('PRAGMA table_info(%s)' % table_name)
    # cid, name, type, notnull, dflt_value, pk
    return [{
        'cid'     : field[0],
        'name'    : field[1],
        'type'    : field[2].upper(),
        'null_ok' : not field[3],
        'pk'      : field[5]     # undocumented
    } for field in cursor.fetchall()]

# }}}
# {{{ def get_table_data_a(cursor, table_name)

def get_table_data_a(cursor, table_name):
    "Returns a list of row data in table_name."
    cursor.execute("select * from %s" % table_name)
    return cursor.fetchall()

# }}}

# {{{ class Table

class Table:
    """ Class: Defines the database table in SQLite3"""
    # {{{ def __init__(self, cursor, table_name_, table_sql_)

    def __init__(self, cursor, table_name_, table_sql_):
        self.table_name = table_name_
        self.table_sql = table_sql_

        #self.class_name = 'DB_Table_' + self.table_name
        #self.file_basename = 'DB_Table_' + self.table_name.title()
        self.class_basename = table_class_basename.get(self.table_name, self.table_name + '_')
        self.class_name = self.class_basename + 'Table'
        self.file_basename = self.class_name

        self.index_a = get_table_index_a(cursor, self.table_name)
        self.field_a = get_table_field_a(cursor, self.table_name)
        self.data_a = get_table_data_a(cursor, self.table_name)

        self.field_name_a = [field['name'] for field in self.field_a]
        self.primay_key = [field['name'] for field in self.field_a if field['pk']][0]

    # }}}
    # {{{ def generate_table_h(self, header)

    def generate_table_h(self, header):
        """ Generate .h file for the table class"""

        # {{{ include

        code = '''
#pragma once

#include "%s.h"
''' % base_file_basename

        # }}}
        # {{{ struct (self.class_name) ...

        code += '''
struct %s : public %s
{
    struct Data;
''' % (self.class_name, base_class_name)

        # }}}
        # {{{ enum COLUMN

        code += '''
    enum COLUMN
    {
        COL_%s = 0''' % self.primay_key.upper()

        for field in self.field_a:
            if field['pk']:
                continue
            code += ''',
        COL_%s''' % field['name'].upper()

        code += ''',
        COL_size
    };
'''

        # }}}
        for field in self.field_a:
            # {{{ struct (field)

            code += '''
    struct %s : public TableOpV<%s>
    {
        static wxString name() { return "%s"; }
        explicit %s(const %s &v): TableOpV<%s>(OP_EQ, v) {}
        explicit %s(OP op, const %s &v): TableOpV<%s>(op, v) {}
    };
''' % (field['name'], dbtype_ctype[field['type']], field['name'],
            field['name'], dbtype_ctype[field['type']], dbtype_ctype[field['type']],
            field['name'], dbtype_ctype[field['type']], dbtype_ctype[field['type']])

            # }}}
        # {{{ typedef PRIMARY

        code += '''
    typedef %s PRIMARY;
''' % self.primay_key

        # }}}
        # {{{ struct Data

        code += '''
    // Data is a single record in the database table
    struct Data
    {'''

        for field in self.field_a:
            code += '''
        %s %s;%s''' % (
            dbtype_ctype[field['type']],
            field['name'], field['pk'] and ' // primary key' or '')
        code += '''
'''

        code += '''
        explicit Data();
        explicit Data(wxSQLite3ResultSet& q);
        Data(const Data& other) = default;
'''

        code += '''
        int64 id() const { return %s; }
        void id(const int64 id) { %s = id; }
        bool equals(const Data* r) const;
        wxString to_json() const;
        void as_json(PrettyWriter<StringBuffer>& json_writer) const;
        row_t to_row_t() const;
        void to_template(html_template& t) const;
        void destroy();
''' % (self.primay_key, self.primay_key)

        code += '''
        Data& operator=(const Data& other);

        auto operator < (const Data& other) const
        {
            return id() < other.id();
        }

        auto operator < (const Data* other) const
        {
            return id() < other->id();
        }
    };
'''

        # }}}
        # {{{ struct Data_Set

        code += '''
    // A container to hold list of Data records for the table
    struct Data_Set : public std::vector<Data>
    {
        wxString to_json() const;
    };
'''

        # }}}
        # {{{ static methods

        code += '''
    static wxString column_to_name(const COLUMN col);
    static COLUMN name_to_column(const wxString& name);
'''
        code += '''
    template<typename C>
    static bool match(const Data* r, const C&)
    {
        return false;
    }
'''

        for field in self.field_a:
            ftype = dbtype_ctype[field['type']]
            if ftype == 'wxString':
                code += '''
    static bool match(const Data* data, const %s& op)
    {
        return data->%s.CmpNoCase(op.m_value) == 0;
    }
''' % (field['name'], field['name'])
            else:
                code += '''
    static bool match(const Data* data, const %s& op)
    {
        return data->%s == op.m_value;
    }
''' % (field['name'], field['name'])

        code += '''
    template<typename Arg1, typename... Args>
    static bool match(const Data* data, const Arg1& arg1, const Args&... args)
    {
        return (match(data, arg1) && ... && match(data, args));
    }

    // TODO: in the above match() functions, check if op.m_operator == OP_EQ
'''

        # }}}
        # {{{ member variables

        code += '''
    // A container to hold a list of Data record pointers for the table in memory
    typedef std::vector<Data*> Cache;
    typedef std::map<int64, Data*> CacheIndex;
    Cache m_cache;
    CacheIndex m_cache_index;
    Data* fake_; // in case the entity not found
'''

        # }}}
        # {{{ instance methods

        code += '''
    %s();
    ~%s();
''' % (self.class_name, self.class_name)

        code += '''
    size_t num_columns() const { return COL_size; }
    void destroy_cache();
    bool ensure_table();
    bool ensure_index();
    void ensure_data();
    Data* create();
    Data* clone(const Data* e);
    bool save(Data* entity);
    bool remove(const int64 id);
    bool remove(Data* entity);
'''

        # }}}
        # {{{ search_cache, cache_id, get_id, get_all

        code += '''
    template<typename... Args>
    Data* search_cache(const Args& ... args)
    {
        for (auto& [_, item] : m_cache_index) {
            if (item->id() > 0 && %s::match(item, args...)) {
                ++m_hit;
                return item;
            }
        }
        ++m_miss;
        return 0;
    }

    Data* cache_id(const int64 id);
    Data* get_id(const int64 id);
    const Data_Set get_all(const COLUMN col = COLUMN(0), const bool asc = true);
''' % self.class_name

        # }}}
        # {{{ SorterBy

        for field_name in sorted(self.field_name_a):
            code += '''
    struct SorterBy%s
    {
        bool operator()(const Data& x, const Data& y)
        {''' % field_name

            if field_name in ['ACCOUNTNAME', 'CATEGNAME', 'PAYEENAME', 'SUBCATEGNAME']:
                code += '''
            // Locale case-insensitive
            return std::wcscoll(x.%s.Lower().wc_str(), y.%s.Lower().wc_str()) < 0;
''' % (field_name, field_name)

            elif field_name in ['CURRENCYNAME']:
                code += '''
            return wxGetTranslation(x.%s) < wxGetTranslation(y.%s);
''' % (field_name, field_name)

            else:
                code += '''
            return x.%s < y.%s;
''' % (field_name, field_name)

            code += '''        }
    };
'''

        # }}}
        # {{{ ... struct (self.class_name)

        code += '''};
'''
        # }}}

        file_name = self.file_basename + '.h'
        print ('Generate %s (source code for %s)' % (file_name, self.table_name))
        rfp = codecs.open(file_name, 'w', 'utf-8-sig')
        rfp.write(header)
        rfp.write(code)
        rfp.close()

    # }}}
    # {{{ def generate_table_cpp(self, header)

    def generate_table_cpp(self, header):
        """ Generate .cpp file for the table class"""

        # {{{ include

        code = '''
#include "%s.h"
''' % self.file_basename

        # }}}
        # {{{ Data::Data

        code += '''
%s::Data::Data()
{''' % self.class_name

        for field in self.field_a:
            ftype = dbtype_ctype[field['type']]
            if ftype == 'wxString':
                continue
            elif ftype == 'double':
                code += '''
    %s = 0.0;''' % field['name']
            elif ftype == 'int64':
                code += '''
    %s = -1;''' % field['name']

        code += '''
}

%s::Data::Data(wxSQLite3ResultSet& q)
{''' % self.class_name

        for field in self.field_a:
            func = dbtype_function[field['type']]
            code += '''
    %s = q.%s(%d);''' % (field['name'], func, field['cid'])

        code += '''
}
'''

        # }}}
        # {{{ Data::equals

        code += '''
bool %s::Data::equals(const %s::Data* r) const
{''' % (self.class_name, self.class_name)

        for field in self.field_a:
            ftype = dbtype_ctype[field['type']]
            if ftype == 'int64' or ftype == 'double':
                code += '''
    if (%s != r->%s) return false;''' % (field['name'], field['name'])
            elif ftype == 'wxString':
                code += '''
    if (!%s.IsSameAs(r->%s)) return false;''' % (field['name'], field['name'])

        code += '''

    return true;
}
'''

        # }}}
        # {{{ Data::operator=

        code += '''
%s::Data& %s::Data::operator=(const %s::Data& other)
{
    if (this == &other) return *this;
''' % (self.class_name, self.class_name, self.class_name)

        for field in self.field_a:
            code += '''
    %s = other.%s;''' % (field['name'], field['name'])

        code += '''

    return *this;
}
'''

        # }}}
        # {{{ Data::to_json, Data::as_json

        code += '''
// Return the data record as a json string
wxString %s::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}
''' % self.class_name

        code += '''
// Add the field data as json key:value pairs
void %s::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{''' % self.class_name

        for field in self.field_a:
            type = dbtype_ctype[field['type']]
            if type == 'int64':
                code += '''
    json_writer.Key("%s");
    json_writer.Int64(this->%s.GetValue());''' % (field['name'], field['name'])
            elif type == 'double':
                code += '''
    json_writer.Key("%s");
    json_writer.Double(this->%s);''' % (field['name'], field['name'])
            elif type == 'wxString':
                code += '''
    json_writer.Key("%s");
    json_writer.String(this->%s.utf8_str());''' % (field['name'], field['name'])
            else:
                assert "Field type Error"

        code += '''
}
'''

        # }}}
        # {{{ Data::to_row_t, Data::to_template

        code += '''
row_t %s::Data::to_row_t() const
{
    row_t row;
''' % self.class_name

        for field in self.field_a:
            code += '''
    row(L"%s") = %s;''' % (field['name'], field['name'] + '.GetValue()' if field['type'] == 'INTEGER' else field['name'])

        code += '''

    return row;
}
'''

        code += '''
void %s::Data::to_template(html_template& t) const
{''' % self.class_name

        for field in self.field_a:
            code += '''
    t(L"%s") = %s;''' % (field['name'], field['name'] + '.GetValue()' if field['type'] == 'INTEGER' else field['name'])

        code += '''
}
'''

        # }}}
        # {{{ Data::destroy

        code += '''
void %s::Data::destroy()
{
    delete this;
}
''' % self.class_name

        # }}}
        # {{{ Data_Set::to_json

        code += '''
// Return the data records as a json array string
wxString %s::Data_Set::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartArray();
    for (const auto & item: *this) {
        json_writer.StartObject();
        item.as_json(json_writer);
        json_writer.EndObject();
    }
    json_writer.EndArray();

    return json_buffer.GetString();
}
''' % self.class_name

    # }}}

        # {{{ (self.class_name)

        code += '''
%s::%s() :
    fake_(new Data())
{
    m_table_name = "%s";
    m_query_select = "SELECT %s FROM %s ";
}
''' % (self.class_name, self.class_name, self.table_name, ', '.join([field['name'] for field in self.field_a]), self.table_name)

        code += '''
// Destructor: clears any data records stored in memory
%s::~%s()
{
    delete this->fake_;
    destroy_cache();
}
''' % (self.class_name, self.class_name)

        # }}}
        # {{{ destroy_cache

        code += '''
// Remove all records stored in memory (cache) for the table
void %s::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}
''' % self.class_name

        # }}}
        # {{{ ensure, ensure_index, ensure_data

        code += '''
// Creates the database table if the table does not exist
bool %s::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("%s");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("%s: Exception %%s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}
''' % (self.class_name, self.table_sql.replace('\n', ''), self.table_name)

        code += '''
bool %s::ensure_index()
{
    try {''' % self.class_name

        for index_name, index_sql in self.index_a:
            index_sql_tokens = index_sql.split()
            index_sql_tokens.insert(2, 'IF')
            index_sql_tokens.insert(3, 'NOT')
            index_sql_tokens.insert(4, 'EXISTS')
            index_ensure_sql = ' '.join(index_sql_tokens)
            code += '''
        m_db->ExecuteUpdate("%s");''' % (index_ensure_sql.replace('\n', ''))

        code += '''
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("%s: Exception %%s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}
''' % self.table_name

        code += '''
void %s::ensure_data()
{
    m_db->Begin();''' % self.class_name

        rf1, rf2, rf3 = '', '', ''
        for r in self.data_a:
            rf2 = ', '.join(["'%s'" if is_trans(i) else "'%s'" % i for i in r])
            rf3 = ', '.join([translation_for(i) for i in r if is_trans(i)])
            if rf2.find('%s') >= 0:
                rf3 = ', ' + rf3
            rf1 = '"INSERT INTO %s VALUES (%s)"%s' % (self.table_name, rf2, rf3)
            if rf2.find('%s') >= 0:
                rf1 = 'wxString::Format(' + rf1 + ')'
            code += '''
    m_db->ExecuteUpdate(%s);''' % (rf1)

        code += '''
    m_db->Commit();
}
'''

        # }}}
        # {{{ column_to_name, name_to_column

        code += '''
// Returns the column name as a string
wxString %s::column_to_name(const %s::COLUMN col)
{
    switch(col) {
        case COL_%s: return "%s";''' % (self.class_name, self.class_name, self.primay_key.upper(), self.primay_key)

        for col_i0, col_name in enumerate([field['name'] for field in self.field_a if not field['pk']]):
            code += '''
        case COL_%s: return "%s";''' % (col_name.upper(), col_name)

        code += '''
        default: break;
    }

    return "UNKNOWN";
}
'''

        code += '''
// Returns the column number from the given column name
%s::COLUMN %s::name_to_column(const wxString& name)
{
    if (name == "%s") return COL_%s;''' % (self.class_name, self.class_name, self.primay_key, self.primay_key.upper())

        for col_i0, col_name in enumerate([field['name'] for field in self.field_a if not field['pk']]):
            code += '''
    else if (name == "%s") return COL_%s;''' %(col_name, col_name.upper())

        code += '''

    return COLUMN(-1);
}
'''

        # }}}
        # {{{ create, clone

        code += '''
// Create a new Data record and add to memory table (cache)
%s::Data* %s::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
%s::Data* %s::clone(const %s::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}
''' % (self.class_name, self.class_name, self.class_name, self.class_name, self.class_name)

        # }}}
        # {{{ save

        code += '''
// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool %s::save(%s::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO %s(%s, %s) VALUES(%s)";
    }''' % (self.class_name, self.class_name, self.table_name, ', '.join([field['name']\
            for field in self.field_a if not field['pk']]), self.primay_key,
            ', '.join(['?' for field in self.field_a]))

        code += '''
    else {
        sql = "UPDATE %s SET %s WHERE %s = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);
''' % (self.table_name, ', '.join([field['name'] + ' = ?'\
    for field in self.field_a if not field['pk']]), self.primay_key)

        for col_i0, col_name in enumerate([field['name'] for field in self.field_a if not field['pk']]):
            code += '''
        stmt.Bind(%d, entity->%s);'''% (col_i0+1, col_name)

        code += '''
        stmt.Bind(%d, entity->id() > 0 ? entity->%s : newId());

        stmt.ExecuteUpdate();
        stmt.Finalize();

        if (entity->id() > 0) {
            // existent
            for (Cache::iterator it = m_cache.begin(); it != m_cache.end(); ++ it) {
                Data* e = *it;
                if (e->id() == entity->id())
                    // update in place
                    *e = *entity;
            }
        }
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("%s: Exception %%s, %%s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}
''' % (len(self.field_a), self.primay_key, self.table_name)

        # }}}
        # {{{ remove

        code += '''
// Remove the Data record from the database and the memory table (cache)
bool %s::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM %s WHERE %s = ?";
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);
        stmt.Bind(1, id);
        stmt.ExecuteUpdate();
        stmt.Finalize();

        Cache c;
        for (Cache::iterator it = m_cache.begin(); it != m_cache.end(); ++ it) {
            Data* entity = *it;
            if (entity->id() == id) {
                m_cache_index.erase(entity->id());
                delete entity;
            }
            else {
                c.push_back(entity);
            }
        }
        m_cache.clear();
        m_cache.swap(c);
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("%s: Exception %%s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}
''' % (self.class_name, self.table_name, self.primay_key, self.table_name)

        code += '''
// Remove the Data record from the database and the memory table (cache)
bool %s::remove(%s::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}
''' % (self.class_name, self.class_name)

        # }}}
        # {{{ cache_id, get_id

        code += '''
// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
%s::Data* %s::cache_id(const int64 id)
{
    if (id <= 0) {
        ++m_skip;
        return nullptr;
    }

    if (auto it = m_cache_index.find(id); it != m_cache_index.end()) {
        ++m_hit;
        return it->second;
    }

    ++m_miss;
    Data* entity = nullptr;
    wxString where = wxString::Format(" WHERE %%s = ?", PRIMARY::name().utf8_str());
    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(this->m_query_select + where);
        stmt.Bind(1, id);

        wxSQLite3ResultSet q = stmt.ExecuteQuery();
        if(q.NextRow()) {
            entity = new Data(q);
            m_cache.push_back(entity);
            m_cache_index.insert(std::make_pair(id, entity));
        }
        stmt.Finalize();
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("%%s: Exception %%s", m_table_name.utf8_str(), e.GetMessage().utf8_str());
    }

    if (!entity) {
        entity = fake_;
        // wxLogError("%%s: %%d not found", m_table_name.utf8_str(), id);
    }

    return entity;
}
''' % (self.class_name, self.class_name)

        code += '''
// Search the database for the data record, bypassing the cache.
%s::Data* %s::get_id(const int64 id)
{
    if (id <= 0) {
        ++m_skip;
        return nullptr;
    }

    Data* entity = nullptr;
    wxString where = wxString::Format(" WHERE %%s = ?", PRIMARY::name().utf8_str());
    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(this->m_query_select + where);
        stmt.Bind(1, id);

        wxSQLite3ResultSet q = stmt.ExecuteQuery();
        if (q.NextRow()) {
            entity = new Data(q);
        }
        stmt.Finalize();
    }
    catch (const wxSQLite3Exception &e) {
        wxLogError("%%s: Exception %%s", m_table_name.utf8_str(), e.GetMessage().utf8_str());
    }

    if (!entity) {
        entity = fake_;
        // wxLogError("%%s: %%d not found", m_table_name.utf8_str(), id);
    }

    return entity;
}
''' % (self.class_name, self.class_name)

        # }}}
        # {{{ get_all

        code += '''
// Return a list of Data records (Data_Set) derived directly from the database.
// The Data_Set is sorted based on the column number.
const %s::Data_Set %s::get_all(const COLUMN col, const bool asc)
{
    Data_Set result;
    try {
        wxSQLite3ResultSet q = m_db->ExecuteQuery(col == COLUMN(0) ? this->m_query_select
            : this->m_query_select + " ORDER BY " + column_to_name(col) + " COLLATE NOCASE " + (asc ? " ASC " : " DESC ")
        );

        while(q.NextRow()) {
            Data entity(q);
            result.push_back(std::move(entity));
        }

        q.Finalize();
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("%%s: Exception %%s", m_table_name.utf8_str(), e.GetMessage().utf8_str());
    }

    return result;
}
''' % (self.class_name, self.class_name)

        # }}}

        file_name = self.file_basename + '.cpp'
        print ('Generate %s (source code for %s)' % (file_name, self.table_name))
        rfp = codecs.open(file_name, 'w', 'utf-8-sig')
        rfp.write(header)
        rfp.write(code)
        rfp.close()

    # }}}
    # {{{ def generate_patch_currency(self)

    def generate_patch_currency(self, patch_filename, only_unicode=False):
        """Generate patch for CURRENCYFORMATS_V1"""
        code = '''-- MMEX Debug SQL - Update --
-- Required MMEX db version: 10
-- This script adds missing currencies and overwrites currency parameterss.'''
        if only_unicode:
            code += '''
-- Only currencies with unicode text are affected.'''

        for row in self.data_a:
            values = ', '.join(["%s='%s'" % (k, row[k]) for k in row.keys() if k.upper() != 'CURRENCYID' and k.upper() != 'CURRENCY_SYMBOL'])
            values = values.replace('_tr_', '')

            if not only_unicode or not is_ascii(values):
                code += '''
INSERT OR IGNORE INTO %s (CURRENCYNAME, CURRENCY_SYMBOL) VALUES ('%s', '%s');
UPDATE OR IGNORE %s SET %s WHERE CURRENCY_SYMBOL='%s';''' % (self.table_name, row['CURRENCYNAME'].replace('_tr_', ''), row['CURRENCY_SYMBOL'], self.table_name, values, row['CURRENCY_SYMBOL'])

        code += '''
'''

        print ('Generate %s (patch for CURRENCYFORMATS_V1)' % patch_filename)
        rfp = codecs.open(patch_filename, 'w', 'utf-8')
        rfp.write(code)
        rfp.close()

    # }}}
# }}}
# {{{ __main__

if __name__ == '__main__':
    # {{{ header

    header = '''// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-%s Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
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
'''% (datetime.date.today().year, os.path.basename(__file__), str(datetime.datetime.now()))

    # }}}
    sql_file, conn, cursor = None, None, None
    try:
        sql_file = sys.argv[1]
        conn = sqlite3.connect(":memory:")
        conn.row_factory = sqlite3.Row
        cursor = conn.cursor()
    except:
        print (__doc__)
        sys.exit(1)

    sql = ""
    sql_txt = '''-- NOTE:
-- This file has been AUTO GENERATED from database/tables.sql
-- All translation markers "_tr_" have been removed.
-- This file can be used to manually generate a database.

'''

    for line_bytes in open(sql_file, 'rb'):
        line = line_bytes.decode('utf-8')
        sql = sql + line

        if line.find('_tr_') > 0: # Remove _tr_ identifyer for wxTRANSLATE
            line = line.replace('_tr_', '')

        sql_txt = sql_txt + line

    print ('Generate %s (clean database without translation)' % tables_sql_filename)
    file_data = codecs.open(tables_sql_filename, 'w', 'utf-8')
    file_data.write(sql_txt)
    file_data.close()

    cursor.executescript(sql)

    for table_name, table_sql in get_table_a(cursor):
        table = Table(cursor, table_name, table_sql)
        table.generate_table_h(header)
        table.generate_table_cpp(header)
        if table_name.upper() == 'CURRENCYFORMATS_V1':
            table.generate_patch_currency(patch_currency_filename, False)
            table.generate_patch_currency(patch_currency_utf8_filename, True)

    conn.close()
    print ('Done')

# }}}
