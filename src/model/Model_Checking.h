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

#ifndef MODEL_CHECKING_H
#define MODEL_CHECKING_H

#include "Model.h"
#include "db/DB_Table_Checkingaccount_V1.h"
#include "Model_Splittransaction.h"
#include "Model_CustomField.h"
#include "Model_Taglink.h"

class Model_Checking : public Model<DB_Table_CHECKINGACCOUNT_V1>
{
public:
    using Model<DB_Table_CHECKINGACCOUNT_V1>::remove;
    using Model<DB_Table_CHECKINGACCOUNT_V1>::save;
    typedef Model_Splittransaction::Data_Set Split_Data_Set;

public:
    enum TYPE_ID
    {
        TYPE_ID_WITHDRAWAL = 0,
        TYPE_ID_DEPOSIT,
        TYPE_ID_TRANSFER
    };
    enum STATUS_ID
    {
        STATUS_ID_NONE = 0,
        STATUS_ID_RECONCILED,
        STATUS_ID_VOID,
        STATUS_ID_FOLLOWUP,
        STATUS_ID_DUPLICATE
    };
    static wxArrayString TYPE_STR;
    static const wxString TYPE_STR_WITHDRAWAL;
    static const wxString TYPE_STR_DEPOSIT;
    static const wxString TYPE_STR_TRANSFER;
    static wxArrayString STATUS_KEY;
    static const wxString STATUS_KEY_NONE;
    static const wxString STATUS_KEY_RECONCILED;
    static const wxString STATUS_KEY_VOID;
    static const wxString STATUS_KEY_FOLLOWUP;
    static const wxString STATUS_KEY_DUPLICATE;
    static wxArrayString STATUS_STR;
    static const wxString STATUS_STR_NONE;
    static const wxString STATUS_STR_RECONCILED;
    static const wxString STATUS_STR_VOID;
    static const wxString STATUS_STR_FOLLOWUP;
    static const wxString STATUS_STR_DUPLICATE;

private:
    static const std::vector<std::pair<TYPE_ID, wxString> > TYPE_CHOICES;
    static const std::vector<std::tuple<STATUS_ID, wxString, wxString> > STATUS_CHOICES;
    static wxArrayString type_str_all();
    static wxArrayString status_key_all();
    static wxArrayString status_str_all();

public:
    struct Full_Data: public Data
    {
        Full_Data();
        explicit Full_Data(const Data& r);
        Full_Data(const Data& r
            , const std::map<int /*trans id*/, Model_Splittransaction::Data_Set /*split trans*/ > & splits
            , const std::map<int /*trans id*/, Model_Taglink::Data_Set /*split trans*/ >& tags);

        ~Full_Data();
        wxString ACCOUNTNAME, TOACCOUNTNAME;
        wxString PAYEENAME;
        wxString CATEGNAME;
        wxString TAGNAMES;
        wxString displayID;
        double AMOUNT;
        double BALANCE;
        wxArrayString ATTACHMENT_DESCRIPTION;
        Model_Splittransaction::Data_Set m_splits;
        Model_Taglink::Data_Set m_tags;
        wxString real_payee_name(int account_id) const;
        const wxString get_currency_code(int account_id) const;
        const wxString get_account_name(int account_id) const;
        bool has_split() const;
        bool has_tags() const;
        bool has_attachment() const;
        bool is_foreign() const;
        bool is_foreign_transfer() const;

        wxString info() const;
        const wxString to_json();

        // Reserved string variables for custom data
        wxString UDFC01;
        double UDFC01_val;
        Model_CustomField::TYPE_ID UDFC01_Type;
        wxString UDFC02;
        double UDFC02_val;
        Model_CustomField::TYPE_ID UDFC02_Type;
        wxString UDFC03;
        double UDFC03_val;
        Model_CustomField::TYPE_ID UDFC03_Type;
        wxString UDFC04;
        double UDFC04_val;
        Model_CustomField::TYPE_ID UDFC04_Type;
        wxString UDFC05;
        double UDFC05_val;
        Model_CustomField::TYPE_ID UDFC05_Type;
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
    struct SorterByDEPOSIT
    {
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return x.AMOUNT < y.AMOUNT;
        }
    };
    struct SorterByWITHDRAWAL
    {
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return x.AMOUNT > y.AMOUNT;
        }
    };
    struct SorterByNUMBER
    {
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return x.TRANSACTIONNUMBER.IsNumber() && y.TRANSACTIONNUMBER.IsNumber()
                ? (wxAtoi(x.TRANSACTIONNUMBER) < wxAtoi(y.TRANSACTIONNUMBER))
                : x.TRANSACTIONNUMBER < y.TRANSACTIONNUMBER;
        }
    };
    struct SorterByTAGNAMES
    {
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return x.TAGNAMES < y.TAGNAMES;
        }
    };
    struct SorterByTRANSTIME
    {
        template <class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return x.TRANSDATE < y.TRANSDATE;
        }
    };

public:
    Model_Checking();
    ~Model_Checking();

public:
    /**
    Initialize the global Model_Checking table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Checking table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Checking& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Checking table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Checking& instance();

public:
    bool remove(int id);
    int save(Data* r);
    int save(std::vector<Data>& rows);
    int save(std::vector<Data*>& rows);
    void updateTimestamp(int id);
public:
    static const Model_Splittransaction::Data_Set splittransaction(const Data* r);
    static const Model_Splittransaction::Data_Set splittransaction(const Data& r);

public:
    static DB_Table_CHECKINGACCOUNT_V1::TRANSDATE TRANSDATE(const wxDateTime& date, OP op = EQUAL);
    static DB_Table_CHECKINGACCOUNT_V1::DELETEDTIME DELETEDTIME(const wxString& date, OP op = EQUAL);
    static DB_Table_CHECKINGACCOUNT_V1::TRANSDATE TRANSDATE(const wxString& date_iso_str, OP op = EQUAL);
    static DB_Table_CHECKINGACCOUNT_V1::STATUS STATUS(STATUS_ID status, OP op = EQUAL);
    static DB_Table_CHECKINGACCOUNT_V1::TRANSCODE TRANSCODE(TYPE_ID type, OP op = EQUAL);
    static DB_Table_CHECKINGACCOUNT_V1::TRANSACTIONNUMBER TRANSACTIONNUMBER(const wxString& num, OP op = EQUAL);

public:
    static wxDate TRANSDATE(const Data* r);
    static wxDate TRANSDATE(const Data& r);
    static TYPE_ID type_id(const wxString& r);
    static TYPE_ID type_id(const Data* r);
    static TYPE_ID type_id(const Data& r);
    static wxString status_key(const wxString& r);
    static STATUS_ID status_id(const wxString& r);
    static STATUS_ID status_id(const Data* r);
    static STATUS_ID status_id(const Data& r);
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
    static bool is_locked(const Data* r);
    static bool is_transfer(const wxString& r);
    static bool is_transfer(const Data* r);
    static bool is_deposit(const wxString& r);
    static bool is_deposit(const Data* r);
    static void getFrequentUsedNotes(std::vector<wxString> &frequentNotes, int accountID = -1);
    static void getEmptyTransaction(Data &data, int accountID);
    static bool getTransactionData(Data &data, const Data* r);
    static void putDataToTransaction(Data *r, const Data &data);
    static bool foreignTransaction(const Data& data);
    static bool foreignTransactionAsTransfer(const Data& data);
};

inline bool Model_Checking::Full_Data::has_split() const { return !this->m_splits.empty(); }
inline bool Model_Checking::Full_Data::has_tags() const { return !this->m_tags.empty(); }
inline bool Model_Checking::Full_Data::has_attachment() const { return !ATTACHMENT_DESCRIPTION.empty(); }

#endif // 
