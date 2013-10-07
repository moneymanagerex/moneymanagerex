
#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <map>
#include <algorithm>
#include <wx/datetime.h>
#include "singleton.h"
#include <wx/sharedptr.h>
#include <wx/log.h>
#include <wx/string.h>

class wxString;
class wxSQLite3Statement;
class wxSQLite3Database;
class wxSQLite3ResultSet;

typedef wxDateTime wxDate;

class Model
{
public:
    Model():db_(0) {};
    virtual ~Model() {};

public:
    void Begin()
    {
        this->db_->Begin();
    }
    void Commit()
    {
        this->db_->Commit();
    }
public:
    static wxDate to_date(const wxString& str_date)
    {
        wxDate date = wxDateTime::Today();
        date.ParseISODate(str_date); // the date in ISO 8601 format "YYYY-MM-DD".
        return date;
    }
protected:
    wxSQLite3Database* db_;
};
#endif // 
