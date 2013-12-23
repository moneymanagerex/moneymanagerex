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

#ifndef MODEL_CHECKING_H
#define MODEL_CHECKING_H

#include "Model.h"
#include "db/DB_Table_Checkingaccount_V1.h"
#include "Model_Splittransaction.h"

class Model_Checking : public Model<DB_Table_CHECKINGACCOUNT_V1>
{
public:
    using Model<DB_Table_CHECKINGACCOUNT_V1>::remove;

public:
    enum TYPE { WITHDRAWAL = 0, DEPOSIT, TRANSFER };
    enum STATUS_ENUM { NONE = 0, RECONCILED, VOID_, FOLLOWUP, DUPLICATE_ };

public:
    static const std::vector<std::pair<TYPE, wxString> > TYPE_CHOICES;

public:
    struct Full_Data: public Data
    {
        Full_Data();
        Full_Data(const Data& r);

        ~Full_Data();
        wxString ACCOUNTNAME, TOACCOUNTNAME;
        wxString PAYEENAME;
        wxString CATEGNAME;
        wxString DEPOSIT;
        wxString WITHDRAWAL;
        double BALANCE;
        Model_Splittransaction::Data_Set m_splits;
        
        wxString real_payee_name(int account_id) const;
        bool has_split() const;
    };
    typedef std::vector<Full_Data> Full_Data_Set;

    struct SorterByBALANCE
    { 
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return x.BALANCE < y.BALANCE;
        }
    };

public:
    Model_Checking();
    ~Model_Checking();

public:
    static wxArrayString all_type();
    static wxArrayString all_status();

public:
    /**
    * Initialize the global Model_Checking table.
    * Reset the Model_Checking table or create the table if it does not exist.
    */
    static Model_Checking& instance(wxSQLite3Database* db);

    /** Return the static instance of Model_Checking table */
    static Model_Checking& instance();

public:
    bool remove(int id);

public:
    static Model_Splittransaction::Data_Set splittransaction(const Data* r);
    static Model_Splittransaction::Data_Set splittransaction(const Data& r);

public:
    static DB_Table_CHECKINGACCOUNT_V1::TRANSDATE TRANSDATE(const wxDate& date, OP op = EQUAL);
    static DB_Table_CHECKINGACCOUNT_V1::STATUS STATUS(STATUS_ENUM status, OP op = EQUAL);
    static DB_Table_CHECKINGACCOUNT_V1::TRANSCODE TRANSCODE(TYPE type, OP op = EQUAL);

public:
    static wxDate TRANSDATE(const Data* r);
    static wxDate TRANSDATE(const Data& r);
    static TYPE type(const Data* r);
    static TYPE type(const Data& r);
    static STATUS_ENUM status(const Data* r);
    static STATUS_ENUM status(const Data& r);
    static double amount(const Data* r, int account_id = -1);
    static double amount(const Data&r, int account_id = -1);
    static double balance(const Data* r, int account_id = -1);
    static double balance(const Data& r, int account_id = -1);
    static double withdrawal(const Data* r, int account_id = -1);
    static double withdrawal(const Data& r, int account_id);
    static double deposit(const Data* r, int account_id);
    static double deposit(const Data& r, int account_id);
    static double reconciled(const Data* r, int account_id);
    static double reconciled(const Data& r, int account_id);
    static wxString toShortStatus(const wxString& fullStatus);
};

#endif // 
