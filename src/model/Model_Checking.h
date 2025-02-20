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

#include "choices.h"
#include "db/DB_Table_Checkingaccount_V1.h"
#include "Model.h"
#include "Model_Splittransaction.h"
#include "Model_CustomField.h"
#include "Model_Taglink.h"
// cannot include "util.h"
const wxString mmGetTimeForDisplay(const wxString& datetime_iso);

class Model_Checking : public Model<DB_Table_CHECKINGACCOUNT_V1>
{
public:
    using Model<DB_Table_CHECKINGACCOUNT_V1>::remove;
    using Model<DB_Table_CHECKINGACCOUNT_V1>::save;
    typedef Model_Splittransaction::Data_Set Split_Data_Set;
    typedef Model_Taglink::Data_Set Taglink_Data_Set;

public:
    enum TYPE_ID
    {
        TYPE_ID_WITHDRAWAL = 0,
        TYPE_ID_DEPOSIT,
        TYPE_ID_TRANSFER,
        TYPE_ID_size
    };
    enum STATUS_ID
    {
        STATUS_ID_NONE = 0,
        STATUS_ID_RECONCILED,
        STATUS_ID_VOID,
        STATUS_ID_FOLLOWUP,
        STATUS_ID_DUPLICATE,
        STATUS_ID_size
    };
    static const wxString TYPE_NAME_WITHDRAWAL;
    static const wxString TYPE_NAME_DEPOSIT;
    static const wxString TYPE_NAME_TRANSFER;
    static const wxString STATUS_KEY_NONE;
    static const wxString STATUS_KEY_RECONCILED;
    static const wxString STATUS_KEY_VOID;
    static const wxString STATUS_KEY_FOLLOWUP;
    static const wxString STATUS_KEY_DUPLICATE;
    static const wxString STATUS_NAME_NONE;
    static const wxString STATUS_NAME_RECONCILED;
    static const wxString STATUS_NAME_VOID;
    static const wxString STATUS_NAME_FOLLOWUP;
    static const wxString STATUS_NAME_DUPLICATE;

private:
    static ChoicesName TYPE_CHOICES;
    static ChoicesKeyName STATUS_CHOICES;

public:
    struct Full_Data: public Data
    {
        Full_Data();
        explicit Full_Data(const Data& r);
        Full_Data(const Data& r,
            const std::map<int64 /* TRANSID */, Model_Splittransaction::Data_Set> & splits,
            const std::map<int64 /* TRANSID */, Model_Taglink::Data_Set> & tags
        );
        ~Full_Data();
        void fill_data();
        wxString real_payee_name(int64 account_id) const;
        const wxString get_currency_code(int64 account_id) const;
        const wxString get_account_name(int64 account_id) const;
        bool has_split() const;
        bool has_tags() const;
        bool has_attachment() const;
        bool is_foreign() const;
        bool is_foreign_transfer() const;
        wxString info() const;
        const wxString to_json();

        // filled-in by constructor
        wxString displayID;
        wxString ACCOUNTNAME, TOACCOUNTNAME;
        wxString PAYEENAME;
        wxString CATEGNAME;
        Split_Data_Set m_splits;
        Taglink_Data_Set m_tags;
        wxString TAGNAMES;

        // filled-in by constructor; overwritten by mmCheckingPanel::filterTable()
        int64 ACCOUNTID_W, ACCOUNTID_D;
        double TRANSAMOUNT_W, TRANSAMOUNT_D;

        // filled-in by mmCheckingPanel::filterTable()
        long SN;
        wxString displaySN;
        double ACCOUNT_FLOW;
        double ACCOUNT_BALANCE;
        wxArrayString ATTACHMENT_DESCRIPTION;
        Model_CustomField::TYPE_ID UDFC_type[5] = {
            Model_CustomField::TYPE_ID_UNKNOWN,
            Model_CustomField::TYPE_ID_UNKNOWN,
            Model_CustomField::TYPE_ID_UNKNOWN,
            Model_CustomField::TYPE_ID_UNKNOWN,
            Model_CustomField::TYPE_ID_UNKNOWN
        };
        wxString UDFC_content[5];
        double UDFC_value[5] = {0, 0, 0, 0, 0};
    };

    typedef std::vector<Full_Data> Full_Data_Set;

    struct SorterByBALANCE
    {
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return x.ACCOUNT_BALANCE < y.ACCOUNT_BALANCE;
        }
    };
    struct SorterByDEPOSIT
    {
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return x.ACCOUNTID_D != -1 && (y.ACCOUNTID_D == -1 || x.TRANSAMOUNT_D < y.TRANSAMOUNT_D);
        }
    };
    struct SorterByWITHDRAWAL
    {
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return x.ACCOUNTID_W != -1 && (y.ACCOUNTID_W == -1 || x.TRANSAMOUNT_W < y.TRANSAMOUNT_W);
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
    struct SorterByTRANSDATE_DATE
    {
        template <class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return x.TRANSDATE.Left(10) < y.TRANSDATE.Left(10);
        }
    };
    struct SorterByTRANSDATE_TIME
    {
        template <class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return mmGetTimeForDisplay(x.TRANSDATE) < mmGetTimeForDisplay(y.TRANSDATE);
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
    bool remove(int64 id);
    int64 save(Data* r);
    int save(std::vector<Data>& rows);
    int save(std::vector<Data*>& rows);
    void updateTimestamp(int64 id);
public:
    static const Model_Checking::Data_Set allByDateTimeId();
    static const Split_Data_Set split(const Data* r);
    static const Split_Data_Set split(const Data& r);

public:
    static DB_Table_CHECKINGACCOUNT_V1::TRANSDATE TRANSDATE(const wxDateTime& date, OP op = EQUAL);
    static DB_Table_CHECKINGACCOUNT_V1::DELETEDTIME DELETEDTIME(const wxString& date, OP op = EQUAL);
    static DB_Table_CHECKINGACCOUNT_V1::TRANSDATE TRANSDATE(const wxString& date_iso_str, OP op = EQUAL);
    static DB_Table_CHECKINGACCOUNT_V1::STATUS STATUS(STATUS_ID status, OP op = EQUAL);
    static DB_Table_CHECKINGACCOUNT_V1::TRANSCODE TRANSCODE(TYPE_ID type, OP op = EQUAL);
    static DB_Table_CHECKINGACCOUNT_V1::TRANSACTIONNUMBER TRANSACTIONNUMBER(const wxString& num, OP op = EQUAL);

public:
    static const wxString type_name(int id);
    static int type_id(const wxString& name, int default_id = TYPE_ID_WITHDRAWAL);
    static TYPE_ID type_id(const Data* r);
    static TYPE_ID type_id(const Data& r);

    static const wxString status_key(int id);
    static const wxString status_key(const wxString& keyOrName);
    static const wxString status_name(int id);
    static const wxString status_name(const wxString& keyOrName);
    static int status_id(const wxString& keyOrName, int default_id = STATUS_ID_NONE);
    static STATUS_ID status_id(const Data* r);
    static STATUS_ID status_id(const Data& r);

    static wxDate TRANSDATE(const Data* r);
    static wxDate TRANSDATE(const Data& r);

    static double account_flow(const Data* r, int64 account_id);
    static double account_flow(const Data& r, int64 account_id);
    static double account_outflow(const Data* r, int64 account_id);
    static double account_outflow(const Data& r, int64 account_id);
    static double account_inflow(const Data* r, int64 account_id);
    static double account_inflow(const Data& r, int64 account_id);
    static double account_recflow(const Data* r, int64 account_id);
    static double account_recflow(const Data& r, int64 account_id);
    static bool is_locked(const Data* r);
    static bool is_transfer(const wxString& r);
    static bool is_transfer(const Data* r);
    static bool is_deposit(const wxString& r);
    static bool is_deposit(const Data* r);
    static void getFrequentUsedNotes(std::vector<wxString> &frequentNotes, int64 accountID = -1);
    static void getEmptyData(Data &data, int64 accountID);

    static bool getTransactionData(Data &data, const Data* r);
    static void putDataToTransaction(Data *r, const Data &data);
    static bool foreignTransaction(const Data& data);
    static bool foreignTransactionAsTransfer(const Data& data);
};

//----------------------------------------------------------------------------

inline const wxString Model_Checking::type_name(int id)
{
    return TYPE_CHOICES.getName(id);
}

inline int Model_Checking::type_id(const wxString& name, int default_id)
{
    return TYPE_CHOICES.findName(name, default_id);
}

inline Model_Checking::TYPE_ID Model_Checking::type_id(const Data* r)
{
    return static_cast<TYPE_ID>(type_id(r->TRANSCODE));
}

inline Model_Checking::TYPE_ID Model_Checking::type_id(const Data& r)
{
    return type_id(&r);
}

inline const wxString Model_Checking::status_key(int id)
{
    return STATUS_CHOICES.getKey(id);
}

inline const wxString Model_Checking::status_key(const wxString& keyOrName)
{
    return status_key(status_id(keyOrName));
}

inline const wxString Model_Checking::status_name(int id)
{
    return STATUS_CHOICES.getName(id);
}

inline const wxString Model_Checking::status_name(const wxString& keyOrName)
{
    return status_name(status_id(keyOrName));
}

inline int Model_Checking::status_id(const wxString& keyOrName, int default_id)
{
    return STATUS_CHOICES.findKeyName(keyOrName, default_id);
}

inline Model_Checking::STATUS_ID Model_Checking::status_id(const Data* r)
{
    return static_cast<STATUS_ID>(status_id(r->STATUS));
}

inline Model_Checking::STATUS_ID Model_Checking::status_id(const Data& r)
{
    return status_id(&r);
}

inline bool Model_Checking::Full_Data::has_split() const
{
    return !this->m_splits.empty();
}

inline bool Model_Checking::Full_Data::has_tags() const

{
    return !this->m_tags.empty();
}

inline bool Model_Checking::Full_Data::has_attachment() const
{
    return !ATTACHMENT_DESCRIPTION.empty();
}

#endif // 
