// -*- C++ -*-
//=============================================================================
/**
 *      Copyright (c) 2013 - 2016 Guan Lisheng (guanlisheng@gmail.com)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2016-02-13 12:50:39.241000.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_CURRENCYFORMATS_V1_H
#define DB_TABLE_CURRENCYFORMATS_V1_H

#include "DB_Table.h"

struct DB_Table_CURRENCYFORMATS_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_CURRENCYFORMATS_V1 Self;
    /** A container to hold list of Data records for the table*/
    struct Data_Set : public std::vector<Self::Data>
    {
        std::wstring to_json() const
        {
            json::Array a;
            for (const auto & item: *this)
            {
                json::Object o;
                item.to_json(o);
                a.Insert(o);
            }
            std::wstringstream ss;
            json::Writer::Write(a, ss);
            return ss.str();
        }
    };
    /** A container to hold a list of Data record pointers for the table in memory*/
    typedef std::vector<Self::Data*> Cache;
    typedef std::map<int, Self::Data*> Index_By_Id;
    Cache cache_;
    Index_By_Id index_by_id_;
    Data* fake_; // in case the entity not found

    /** Destructor: clears any data records stored in memory */
    ~DB_Table_CURRENCYFORMATS_V1() 
    {
        delete this->fake_;
        destroy_cache();
    }
     
    /** Removes all records stored in memory (cache) for the table*/ 
    void destroy_cache()
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fun(&Data::destroy));
        cache_.clear();
        index_by_id_.clear(); // no memory release since it just stores pointer and the according objects are in cache
    }

    /** Creates the database table if the table does not exist*/
    bool ensure(wxSQLite3Database* db)
    {
        if (!exists(db))
        {
            try
            {
                db->ExecuteUpdate("CREATE TABLE CURRENCYFORMATS_V1(CURRENCYID integer primary key, CURRENCYNAME TEXT COLLATE NOCASE NOT NULL UNIQUE, PFX_SYMBOL TEXT, SFX_SYMBOL TEXT, DECIMAL_POINT TEXT, GROUP_SEPARATOR TEXT, UNIT_NAME TEXT COLLATE NOCASE, CENT_NAME TEXT COLLATE NOCASE, SCALE integer, BASECONVRATE numeric, CURRENCY_SYMBOL TEXT COLLATE NOCASE NOT NULL UNIQUE)");
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("CURRENCYFORMATS_V1: Exception %s", e.GetMessage().c_str());
                return false;
            }
            this->ensure_data(db);
        }

        this->ensure_index(db);

        return true;
    }

    bool ensure_index(wxSQLite3Database* db)
    {
        try
        {
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CURRENCYFORMATS_SYMBOL ON CURRENCYFORMATS_V1(CURRENCY_SYMBOL)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("CURRENCYFORMATS_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    bool ensure_data(wxSQLite3Database* db)
    {
        try
        {
            db->ExecuteQuery("SELECT * FROM CURRENCYFORMATS_V1"); // dummy code
        
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (1, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("United States dollar"), _("$"), _(""), _("."), _(" "), _(""), _(""), _("USD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (2, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("European euro"), _("€"), _(""), _("."), _(" "), _(""), _(""), _("EUR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (3, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("UK Pound"), _("£"), _(""), _("."), _(" "), _("Pound"), _("Pence"), _("GBP")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (4, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Russian Ruble"), _(""), _("р"), _(","), _(" "), _("руб."), _("коп."), _("RUB")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (5, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Ukrainian hryvnia"), _("₴"), _(""), _(","), _(" "), _(""), _(""), _("UAH")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (6, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Afghan afghani"), _("؋"), _(""), _("."), _(" "), _(""), _("pul"), _("AFN")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (7, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Albanian lek"), _(""), _("L"), _("."), _(" "), _(""), _(""), _("ALL")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (8, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Algerian dinar"), _("دج"), _(""), _("."), _(" "), _(""), _(""), _("DZD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (9, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Angolan kwanza"), _(""), _("Kz"), _("."), _(" "), _(""), _("Céntimo"), _("AOA")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (10, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("East Caribbean dollar"), _("EC$"), _(""), _("."), _(" "), _(""), _(""), _("XCD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (11, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Argentine peso"), _("AR$"), _(""), _(","), _("."), _(""), _("centavo"), _("ARS")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (12, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Armenian dram"), _(""), _(""), _("."), _(" "), _(""), _(""), _("AMD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (13, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Aruban florin"), _("ƒ"), _(""), _("."), _(" "), _(""), _(""), _("AWG")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (14, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Australian dollar"), _("$"), _(""), _("."), _(","), _(""), _(""), _("AUD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (15, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Azerbaijani manat"), _(""), _(""), _("."), _(" "), _(""), _(""), _("AZN")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (16, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Bahamian dollar"), _("B$"), _(""), _("."), _(" "), _(""), _(""), _("BSD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (17, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Bahraini dinar"), _(""), _(""), _("."), _(" "), _(""), _(""), _("BHD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (18, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Bangladeshi taka"), _(""), _(""), _("."), _(" "), _(""), _(""), _("BDT")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (19, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Barbadian dollar"), _("Bds$"), _(""), _("."), _(" "), _(""), _(""), _("BBD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (20, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Belarusian ruble"), _("Br"), _(""), _(","), _(" "), _(""), _(""), _("BYR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (21, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Belize dollar"), _("BZ$"), _(""), _("."), _(" "), _(""), _(""), _("BZD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (22, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("West African CFA franc"), _("CFA"), _(""), _("."), _(" "), _(""), _(""), _("XOF")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (23, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Bermudian dollar"), _("BD$"), _(""), _("."), _(" "), _(""), _(""), _("BMD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (24, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Bhutanese ngultrum"), _("Nu."), _(""), _("."), _(" "), _(""), _(""), _("BTN")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (25, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Bolivian boliviano"), _("Bs."), _(""), _("."), _(" "), _(""), _(""), _("BOB")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (26, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Bosnia and Herzegovina konvertibilna marka"), _("KM"), _(""), _(","), _("."), _(""), _(""), _("BAM")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (27, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Botswana pula"), _("P"), _(""), _("."), _(" "), _(""), _(""), _("BWP")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (28, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Brazilian real"), _("R$"), _(""), _("."), _(" "), _(""), _(""), _("BRL")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (29, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Brunei dollar"), _("B$"), _(""), _("."), _(" "), _(""), _(""), _("BND")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (30, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Bulgarian lev"), _(""), _(""), _("."), _(" "), _(""), _(""), _("BGN")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (31, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Burundi franc"), _("FBu"), _(""), _("."), _(" "), _(""), _(""), _("BIF")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (32, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Cambodian riel"), _(""), _(""), _("."), _(" "), _(""), _(""), _("KHR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (33, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Central African CFA franc"), _("CFA"), _(""), _("."), _(" "), _(""), _(""), _("XAF")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (34, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Canadian dollar"), _("$"), _(""), _("."), _(" "), _(""), _(""), _("CAD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (35, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Cape Verdean escudo"), _("Esc"), _(""), _("."), _(" "), _(""), _(""), _("CVE")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (36, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Cayman Islands dollar"), _("KY$"), _(""), _("."), _(" "), _(""), _(""), _("KYD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (37, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Chilean peso"), _("$"), _(""), _("."), _(" "), _(""), _(""), _("CLP")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (38, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Chinese renminbi"), _("¥"), _(""), _("."), _(" "), _(""), _(""), _("CNY")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (39, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Colombian peso"), _("Col$"), _(""), _("."), _(" "), _(""), _(""), _("COP")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (40, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Comorian franc"), _(""), _(""), _("."), _(" "), _(""), _(""), _("KMF")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (41, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Congolese franc"), _("F"), _(""), _("."), _(" "), _(""), _(""), _("CDF")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (42, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Costa Rican colon"), _("₡"), _(""), _("."), _(" "), _(""), _(""), _("CRC")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (43, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Croatian kuna"), _("kn"), _(""), _("."), _(" "), _(""), _(""), _("HRK")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (44, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Czech koruna"), _("Kč"), _(""), _("."), _(" "), _(""), _(""), _("CZK")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (45, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Danish krone"), _("Kr"), _(""), _("."), _(" "), _(""), _(""), _("DKK")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (46, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Djiboutian franc"), _("Fdj"), _(""), _("."), _(" "), _(""), _(""), _("DJF")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (47, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Dominican peso"), _("RD$"), _(""), _("."), _(" "), _(""), _(""), _("DOP")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (48, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Egyptian pound"), _("£"), _(""), _("."), _(" "), _(""), _(""), _("EGP")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (49, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Eritrean nakfa"), _("Nfa"), _(""), _("."), _(" "), _(""), _(""), _("ERN")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (50, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Ethiopian birr"), _("Br"), _(""), _("."), _(" "), _(""), _(""), _("ETB")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (51, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Falkland Islands pound"), _("£"), _(""), _("."), _(" "), _(""), _(""), _("FKP")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (52, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Fijian dollar"), _("FJ$"), _(""), _("."), _(" "), _(""), _(""), _("FJD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (53, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("CFP franc"), _("F"), _(""), _("."), _(" "), _(""), _(""), _("XPF")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (54, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Gambian dalasi"), _("D"), _(""), _("."), _(" "), _(""), _(""), _("GMD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (55, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Georgian lari"), _(""), _(""), _("."), _(" "), _(""), _(""), _("GEL")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (56, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Ghanaian cedi"), _(""), _(""), _("."), _(" "), _(""), _(""), _("GHS")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (57, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Gibraltar pound"), _("£"), _(""), _("."), _(" "), _(""), _(""), _("GIP")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (58, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Guatemalan quetzal"), _("Q"), _(""), _("."), _(" "), _(""), _(""), _("GTQ")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (59, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Guinean franc"), _("FG"), _(""), _("."), _(" "), _(""), _(""), _("GNF")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (60, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Guyanese dollar"), _("GY$"), _(""), _("."), _(" "), _(""), _(""), _("GYD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (61, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Haitian gourde"), _("G"), _(""), _("."), _(" "), _(""), _(""), _("HTG")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (62, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Honduran lempira"), _("L"), _(""), _("."), _(" "), _(""), _(""), _("HNL")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (63, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Hong Kong dollar"), _("HK$"), _(""), _("."), _(" "), _(""), _(""), _("HKD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (64, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Hungarian forint"), _("Ft"), _(""), _("."), _(" "), _(""), _(""), _("HUF")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (65, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Icelandic króna"), _("kr"), _(""), _("."), _(" "), _(""), _(""), _("ISK")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (66, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Indian rupee"), _("₹"), _(""), _("."), _(" "), _(""), _(""), _("INR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (67, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Indonesian rupiah"), _("Rp"), _(""), _("."), _(" "), _(""), _(""), _("IDR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (68, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Special Drawing Rights"), _("SDR"), _(""), _("."), _(" "), _(""), _(""), _("XDR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (69, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Iranian rial"), _(""), _(""), _("."), _(" "), _(""), _(""), _("IRR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (70, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Iraqi dinar"), _(""), _(""), _("."), _(" "), _(""), _(""), _("IQD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (71, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Israeli new sheqel"), _(""), _(""), _("."), _(" "), _(""), _(""), _("ILS")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (72, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Jamaican dollar"), _("J$"), _(""), _("."), _(" "), _(""), _(""), _("JMD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (73, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Japanese yen"), _("¥"), _(""), _("."), _(" "), _(""), _(""), _("JPY")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (74, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Jordanian dinar"), _(""), _(""), _("."), _(" "), _(""), _(""), _("JOD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (75, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Kazakhstani tenge"), _("T"), _(""), _("."), _(" "), _(""), _(""), _("KZT")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (76, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Kenyan shilling"), _("KSh"), _(""), _("."), _(" "), _(""), _(""), _("KES")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (77, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("North Korean won"), _("W"), _(""), _("."), _(" "), _(""), _(""), _("KPW")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (78, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("South Korean won"), _("W"), _(""), _("."), _(" "), _(""), _(""), _("KRW")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (79, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Kuwaiti dinar"), _(""), _(""), _("."), _(" "), _(""), _(""), _("KWD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (80, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Kyrgyzstani som"), _(""), _(""), _("."), _(" "), _(""), _(""), _("KGS")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (81, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Lao kip"), _("KN"), _(""), _("."), _(" "), _(""), _(""), _("LAK")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (82, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Latvian lats"), _("Ls"), _(""), _("."), _(" "), _(""), _(""), _("LVL")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (83, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Lebanese lira"), _(""), _(""), _("."), _(" "), _(""), _(""), _("LBP")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (84, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Lesotho loti"), _("M"), _(""), _("."), _(" "), _(""), _(""), _("LSL")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (85, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Liberian dollar"), _("L$"), _(""), _("."), _(" "), _(""), _(""), _("LRD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (86, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Libyan dinar"), _("LD"), _(""), _("."), _(" "), _(""), _(""), _("LYD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (87, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Lithuanian litas"), _("Lt"), _(""), _("."), _(" "), _(""), _(""), _("LTL")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (88, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Macanese pataca"), _("P"), _(""), _("."), _(" "), _(""), _(""), _("MOP")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (89, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Macedonian denar"), _(""), _(""), _("."), _(" "), _(""), _(""), _("MKD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (90, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Malagasy ariary"), _("FMG"), _(""), _("."), _(" "), _(""), _(""), _("MGA")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (91, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Malawian kwacha"), _("MK"), _(""), _("."), _(" "), _(""), _(""), _("MWK")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (92, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Malaysian ringgit"), _("RM"), _(""), _("."), _(" "), _(""), _(""), _("MYR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (93, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Maldivian rufiyaa"), _("Rf"), _(""), _("."), _(" "), _(""), _(""), _("MVR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (94, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Mauritanian ouguiya"), _("UM"), _(""), _("."), _(" "), _(""), _(""), _("MRO")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (95, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Mauritian rupee"), _("Rs"), _(""), _("."), _(" "), _(""), _(""), _("MUR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (96, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Mexican peso"), _("$"), _(""), _("."), _(" "), _(""), _(""), _("MXN")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (97, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Moldovan leu"), _(""), _(""), _("."), _(" "), _(""), _(""), _("MDL")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (98, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Mongolian tugrik"), _("₮"), _(""), _("."), _(" "), _(""), _(""), _("MNT")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (99, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Moroccan dirham"), _(""), _(""), _("."), _(" "), _(""), _(""), _("MAD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (100, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Myanma kyat"), _("K"), _(""), _("."), _(" "), _(""), _(""), _("MMK")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (101, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Namibian dollar"), _("N$"), _(""), _("."), _(" "), _(""), _(""), _("NAD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (102, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Nepalese rupee"), _("NRs"), _(""), _("."), _(" "), _(""), _(""), _("NPR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (103, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Netherlands Antillean gulden"), _("NAƒ"), _(""), _("."), _(" "), _(""), _(""), _("ANG")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (104, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("New Zealand dollar"), _("NZ$"), _(""), _("."), _(" "), _(""), _(""), _("NZD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (105, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Nicaraguan córdoba"), _("C$"), _(""), _("."), _(" "), _(""), _(""), _("NIO")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (106, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Nigerian naira"), _("₦"), _(""), _("."), _(" "), _(""), _(""), _("NGN")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (107, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Norwegian krone"), _("kr"), _(""), _("."), _(" "), _(""), _(""), _("NOK")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (108, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Omani rial"), _(""), _(""), _("."), _(" "), _(""), _(""), _("OMR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (109, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Pakistani rupee"), _("Rs."), _(""), _("."), _(" "), _(""), _(""), _("PKR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (110, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Panamanian balboa"), _("B./"), _(""), _("."), _(" "), _(""), _(""), _("PAB")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (111, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Papua New Guinean kina"), _("K"), _(""), _("."), _(" "), _(""), _(""), _("PGK")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (112, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Paraguayan guarani"), _(""), _(""), _("."), _(" "), _(""), _(""), _("PYG")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (113, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Peruvian nuevo sol"), _("S/."), _(""), _("."), _(" "), _(""), _(""), _("PEN")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (114, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Philippine peso"), _("₱"), _(""), _("."), _(" "), _(""), _(""), _("PHP")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (115, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Polish zloty"), _(""), _(""), _("."), _(" "), _(""), _(""), _("PLN")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (116, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Qatari riyal"), _("QR"), _(""), _("."), _(" "), _(""), _(""), _("QAR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (117, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Romanian leu"), _("L"), _(""), _("."), _(" "), _(""), _(""), _("RON")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (118, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Rwandan franc"), _("RF"), _(""), _("."), _(" "), _(""), _(""), _("RWF")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (119, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("São Tomé and Príncipe dobra"), _("Db"), _(""), _("."), _(" "), _(""), _(""), _("STD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (120, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Saudi riyal"), _("SR"), _(""), _("."), _(" "), _(""), _(""), _("SAR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (121, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Serbian dinar"), _("din."), _(""), _("."), _(" "), _(""), _(""), _("RSD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (122, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Seychellois rupee"), _("SR"), _(""), _("."), _(" "), _(""), _(""), _("SCR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (123, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Sierra Leonean leone"), _("Le"), _(""), _("."), _(" "), _(""), _(""), _("SLL")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (124, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Singapore dollar"), _("S$"), _(""), _("."), _(" "), _(""), _(""), _("SGD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (125, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Solomon Islands dollar"), _("SI$"), _(""), _("."), _(" "), _(""), _(""), _("SBD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (126, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Somali shilling"), _("Sh."), _(""), _("."), _(" "), _(""), _(""), _("SOS")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (127, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("South African rand"), _("R"), _(""), _("."), _(" "), _(""), _(""), _("ZAR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (128, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Sri Lankan rupee"), _("Rs"), _(""), _("."), _(" "), _(""), _(""), _("LKR")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (129, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Saint Helena pound"), _("£"), _(""), _("."), _(" "), _(""), _(""), _("SHP")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (130, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Sudanese pound"), _(""), _(""), _("."), _(" "), _(""), _(""), _("SDG")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (131, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Surinamese dollar"), _("$"), _(""), _("."), _(" "), _(""), _(""), _("SRD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (132, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Swazi lilangeni"), _("E"), _(""), _("."), _(" "), _(""), _(""), _("SZL")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (133, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Swedish krona"), _("kr"), _(""), _("."), _(" "), _(""), _(""), _("SEK")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (134, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Swiss franc"), _("Fr."), _(""), _("."), _(" "), _(""), _(""), _("CHF")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (135, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Syrian pound"), _(""), _(""), _("."), _(" "), _(""), _(""), _("SYP")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (136, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("New Taiwan dollar"), _("NT$"), _(""), _("."), _(" "), _(""), _(""), _("TWD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (137, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Tajikistani somoni"), _(""), _(""), _("."), _(" "), _(""), _(""), _("TJS")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (138, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Tanzanian shilling"), _(""), _(""), _("."), _(" "), _(""), _(""), _("TZS")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (139, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Thai baht"), _("฿"), _(""), _("."), _(" "), _(""), _(""), _("THB")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (140, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Trinidad and Tobago dollar"), _("TT$"), _(""), _("."), _(" "), _(""), _(""), _("TTD")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (141, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Tunisian dinar"), _("DT"), _(""), _("."), _(" "), _(""), _(""), _("TND")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (142, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Turkish lira"), _("₺"), _(""), _("."), _(" "), _(""), _(""), _("TRY")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (143, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Turkmen manat"), _("m"), _(""), _("."), _(" "), _(""), _(""), _("TMT")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (144, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Ugandan shilling"), _("USh"), _(""), _("."), _(" "), _(""), _(""), _("UGX")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (145, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("UAE dirham"), _(""), _(""), _("."), _(" "), _(""), _(""), _("AED")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (146, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Uruguayan peso"), _("$U"), _(""), _("."), _(" "), _(""), _(""), _("UYU")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (147, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Uzbekistani som"), _(""), _(""), _("."), _(" "), _(""), _(""), _("UZS")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (148, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Vanuatu vatu"), _("VT"), _(""), _("."), _(" "), _(""), _(""), _("VUV")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (149, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Vietnamese dong"), _("₫"), _(""), _("."), _(" "), _(""), _(""), _("VND")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (150, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Samoan tala"), _("WS$"), _(""), _("."), _(" "), _(""), _(""), _("WST")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (151, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 1, 1, '%s')", _("Yemeni rial"), _(""), _(""), _("."), _(" "), _(""), _(""), _("YER")));
            db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES (152, '%s', '%s', '%s', '%s', '%s', '%s', '%s', 100, 1, '%s')", _("Venezuelan Bolívar"), _("Bs."), _(""), _("."), _(","), _("bolívar"), _("céntimos"), _("VEF")));
        }
        catch(const wxSQLite3Exception & e)
        {
            wxLogError("CURRENCYFORMATS_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }
    struct CURRENCYID : public DB_Column<int>
    { 
        static wxString name() { return "CURRENCYID"; } 
        explicit CURRENCYID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct CURRENCYNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "CURRENCYNAME"; } 
        explicit CURRENCYNAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct PFX_SYMBOL : public DB_Column<wxString>
    { 
        static wxString name() { return "PFX_SYMBOL"; } 
        explicit PFX_SYMBOL(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct SFX_SYMBOL : public DB_Column<wxString>
    { 
        static wxString name() { return "SFX_SYMBOL"; } 
        explicit SFX_SYMBOL(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct DECIMAL_POINT : public DB_Column<wxString>
    { 
        static wxString name() { return "DECIMAL_POINT"; } 
        explicit DECIMAL_POINT(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct GROUP_SEPARATOR : public DB_Column<wxString>
    { 
        static wxString name() { return "GROUP_SEPARATOR"; } 
        explicit GROUP_SEPARATOR(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct UNIT_NAME : public DB_Column<wxString>
    { 
        static wxString name() { return "UNIT_NAME"; } 
        explicit UNIT_NAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct CENT_NAME : public DB_Column<wxString>
    { 
        static wxString name() { return "CENT_NAME"; } 
        explicit CENT_NAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct SCALE : public DB_Column<int>
    { 
        static wxString name() { return "SCALE"; } 
        explicit SCALE(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct BASECONVRATE : public DB_Column<double>
    { 
        static wxString name() { return "BASECONVRATE"; } 
        explicit BASECONVRATE(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    struct CURRENCY_SYMBOL : public DB_Column<wxString>
    { 
        static wxString name() { return "CURRENCY_SYMBOL"; } 
        explicit CURRENCY_SYMBOL(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    typedef CURRENCYID PRIMARY;
    enum COLUMN
    {
        COL_CURRENCYID = 0
        , COL_CURRENCYNAME = 1
        , COL_PFX_SYMBOL = 2
        , COL_SFX_SYMBOL = 3
        , COL_DECIMAL_POINT = 4
        , COL_GROUP_SEPARATOR = 5
        , COL_UNIT_NAME = 6
        , COL_CENT_NAME = 7
        , COL_SCALE = 8
        , COL_BASECONVRATE = 9
        , COL_CURRENCY_SYMBOL = 10
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_CURRENCYID: return "CURRENCYID";
            case COL_CURRENCYNAME: return "CURRENCYNAME";
            case COL_PFX_SYMBOL: return "PFX_SYMBOL";
            case COL_SFX_SYMBOL: return "SFX_SYMBOL";
            case COL_DECIMAL_POINT: return "DECIMAL_POINT";
            case COL_GROUP_SEPARATOR: return "GROUP_SEPARATOR";
            case COL_UNIT_NAME: return "UNIT_NAME";
            case COL_CENT_NAME: return "CENT_NAME";
            case COL_SCALE: return "SCALE";
            case COL_BASECONVRATE: return "BASECONVRATE";
            case COL_CURRENCY_SYMBOL: return "CURRENCY_SYMBOL";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("CURRENCYID" == name) return COL_CURRENCYID;
        else if ("CURRENCYNAME" == name) return COL_CURRENCYNAME;
        else if ("PFX_SYMBOL" == name) return COL_PFX_SYMBOL;
        else if ("SFX_SYMBOL" == name) return COL_SFX_SYMBOL;
        else if ("DECIMAL_POINT" == name) return COL_DECIMAL_POINT;
        else if ("GROUP_SEPARATOR" == name) return COL_GROUP_SEPARATOR;
        else if ("UNIT_NAME" == name) return COL_UNIT_NAME;
        else if ("CENT_NAME" == name) return COL_CENT_NAME;
        else if ("SCALE" == name) return COL_SCALE;
        else if ("BASECONVRATE" == name) return COL_BASECONVRATE;
        else if ("CURRENCY_SYMBOL" == name) return COL_CURRENCY_SYMBOL;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_CURRENCYFORMATS_V1;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int CURRENCYID;//  primary key
        wxString CURRENCYNAME;
        wxString PFX_SYMBOL;
        wxString SFX_SYMBOL;
        wxString DECIMAL_POINT;
        wxString GROUP_SEPARATOR;
        wxString UNIT_NAME;
        wxString CENT_NAME;
        int SCALE;
        double BASECONVRATE;
        wxString CURRENCY_SYMBOL;
        int id() const { return CURRENCYID; }
        void id(int id) { CURRENCYID = id; }
        bool operator < (const Data& r) const
        {
            return this->id() < r.id();
        }
        bool operator < (const Data* r) const
        {
            return this->id() < r->id();
        }

        explicit Data(Self* table = 0) 
        {
            table_ = table;
        
            CURRENCYID = -1;
            SCALE = -1;
            BASECONVRATE = 0.0;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = 0)
        {
            table_ = table;
        
            CURRENCYID = q.GetInt(0); // CURRENCYID
            CURRENCYNAME = q.GetString(1); // CURRENCYNAME
            PFX_SYMBOL = q.GetString(2); // PFX_SYMBOL
            SFX_SYMBOL = q.GetString(3); // SFX_SYMBOL
            DECIMAL_POINT = q.GetString(4); // DECIMAL_POINT
            GROUP_SEPARATOR = q.GetString(5); // GROUP_SEPARATOR
            UNIT_NAME = q.GetString(6); // UNIT_NAME
            CENT_NAME = q.GetString(7); // CENT_NAME
            SCALE = q.GetInt(8); // SCALE
            BASECONVRATE = q.GetDouble(9); // BASECONVRATE
            CURRENCY_SYMBOL = q.GetString(10); // CURRENCY_SYMBOL
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            CURRENCYID = other.CURRENCYID;
            CURRENCYNAME = other.CURRENCYNAME;
            PFX_SYMBOL = other.PFX_SYMBOL;
            SFX_SYMBOL = other.SFX_SYMBOL;
            DECIMAL_POINT = other.DECIMAL_POINT;
            GROUP_SEPARATOR = other.GROUP_SEPARATOR;
            UNIT_NAME = other.UNIT_NAME;
            CENT_NAME = other.CENT_NAME;
            SCALE = other.SCALE;
            BASECONVRATE = other.BASECONVRATE;
            CURRENCY_SYMBOL = other.CURRENCY_SYMBOL;
            return *this;
        }

        template<typename C>
        bool match(const C &c) const
        {
            return false;
        }
        bool match(const Self::CURRENCYID &in) const
        {
            return this->CURRENCYID == in.v_;
        }
        bool match(const Self::CURRENCYNAME &in) const
        {
            return this->CURRENCYNAME.CmpNoCase(in.v_) == 0;
        }
        bool match(const Self::PFX_SYMBOL &in) const
        {
            return this->PFX_SYMBOL.CmpNoCase(in.v_) == 0;
        }
        bool match(const Self::SFX_SYMBOL &in) const
        {
            return this->SFX_SYMBOL.CmpNoCase(in.v_) == 0;
        }
        bool match(const Self::DECIMAL_POINT &in) const
        {
            return this->DECIMAL_POINT.CmpNoCase(in.v_) == 0;
        }
        bool match(const Self::GROUP_SEPARATOR &in) const
        {
            return this->GROUP_SEPARATOR.CmpNoCase(in.v_) == 0;
        }
        bool match(const Self::UNIT_NAME &in) const
        {
            return this->UNIT_NAME.CmpNoCase(in.v_) == 0;
        }
        bool match(const Self::CENT_NAME &in) const
        {
            return this->CENT_NAME.CmpNoCase(in.v_) == 0;
        }
        bool match(const Self::SCALE &in) const
        {
            return this->SCALE == in.v_;
        }
        bool match(const Self::BASECONVRATE &in) const
        {
            return this->BASECONVRATE == in.v_;
        }
        bool match(const Self::CURRENCY_SYMBOL &in) const
        {
            return this->CURRENCY_SYMBOL.CmpNoCase(in.v_) == 0;
        }
        wxString to_json() const
        {
            json::Object o;
            this->to_json(o);
            std::wstringstream ss;
            json::Writer::Write(o, ss);
            return ss.str();
        }
        
        int to_json(json::Object& o) const
        {
            o[L"CURRENCYID"] = json::Number(this->CURRENCYID);
            o[L"CURRENCYNAME"] = json::String(this->CURRENCYNAME.ToStdWstring());
            o[L"PFX_SYMBOL"] = json::String(this->PFX_SYMBOL.ToStdWstring());
            o[L"SFX_SYMBOL"] = json::String(this->SFX_SYMBOL.ToStdWstring());
            o[L"DECIMAL_POINT"] = json::String(this->DECIMAL_POINT.ToStdWstring());
            o[L"GROUP_SEPARATOR"] = json::String(this->GROUP_SEPARATOR.ToStdWstring());
            o[L"UNIT_NAME"] = json::String(this->UNIT_NAME.ToStdWstring());
            o[L"CENT_NAME"] = json::String(this->CENT_NAME.ToStdWstring());
            o[L"SCALE"] = json::Number(this->SCALE);
            o[L"BASECONVRATE"] = json::Number(this->BASECONVRATE);
            o[L"CURRENCY_SYMBOL"] = json::String(this->CURRENCY_SYMBOL.ToStdWstring());
            return 0;
        }
        row_t to_row_t() const
        {
            row_t row;
            row(L"CURRENCYID") = CURRENCYID;
            row(L"CURRENCYNAME") = CURRENCYNAME;
            row(L"PFX_SYMBOL") = PFX_SYMBOL;
            row(L"SFX_SYMBOL") = SFX_SYMBOL;
            row(L"DECIMAL_POINT") = DECIMAL_POINT;
            row(L"GROUP_SEPARATOR") = GROUP_SEPARATOR;
            row(L"UNIT_NAME") = UNIT_NAME;
            row(L"CENT_NAME") = CENT_NAME;
            row(L"SCALE") = SCALE;
            row(L"BASECONVRATE") = BASECONVRATE;
            row(L"CURRENCY_SYMBOL") = CURRENCY_SYMBOL;
            return row;
        }
        void to_template(html_template& t) const
        {
            t(L"CURRENCYID") = CURRENCYID;
            t(L"CURRENCYNAME") = CURRENCYNAME;
            t(L"PFX_SYMBOL") = PFX_SYMBOL;
            t(L"SFX_SYMBOL") = SFX_SYMBOL;
            t(L"DECIMAL_POINT") = DECIMAL_POINT;
            t(L"GROUP_SEPARATOR") = GROUP_SEPARATOR;
            t(L"UNIT_NAME") = UNIT_NAME;
            t(L"CENT_NAME") = CENT_NAME;
            t(L"SCALE") = SCALE;
            t(L"BASECONVRATE") = BASECONVRATE;
            t(L"CURRENCY_SYMBOL") = CURRENCY_SYMBOL;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save CURRENCYFORMATS_V1");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove CURRENCYFORMATS_V1");
                return false;
            }
            
            return table_->remove(this, db);
        }

        void destroy()
        {
            //if (this->id() < 0)
            //    wxSafeShowMessage("unsaved object", this->to_json());
            delete this;
        }
    };

    enum
    {
        NUM_COLUMNS = 11
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "CURRENCYFORMATS_V1"; }

    DB_Table_CURRENCYFORMATS_V1() : fake_(new Data())
    {
        query_ = "SELECT * FROM CURRENCYFORMATS_V1 ";
    }

    /** Create a new Data record and add to memory table (cache)*/
    Self::Data* create()
    {
        Self::Data* entity = new Self::Data(this);
        cache_.push_back(entity);
        return entity;
    }
    
    /** Create a copy of the Data record and add to memory table (cache)*/
    Self::Data* clone(const Data* e)
    {
        Self::Data* entity = create();
        *entity = *e;
        entity->id(-1);
        return entity;
    }

    /**
    * Saves the Data record to the database table.
    * Either create a new record or update the existing record.
    * Remove old record from the memory table (cache)
    */
    bool save(Self::Data* entity, wxSQLite3Database* db)
    {
        wxString sql = wxEmptyString;
        if (entity->id() <= 0) //  new & insert
        {
            sql = "INSERT INTO CURRENCYFORMATS_V1(CURRENCYNAME, PFX_SYMBOL, SFX_SYMBOL, DECIMAL_POINT, GROUP_SEPARATOR, UNIT_NAME, CENT_NAME, SCALE, BASECONVRATE, CURRENCY_SYMBOL) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE CURRENCYFORMATS_V1 SET CURRENCYNAME = ?, PFX_SYMBOL = ?, SFX_SYMBOL = ?, DECIMAL_POINT = ?, GROUP_SEPARATOR = ?, UNIT_NAME = ?, CENT_NAME = ?, SCALE = ?, BASECONVRATE = ?, CURRENCY_SYMBOL = ? WHERE CURRENCYID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->CURRENCYNAME);
            stmt.Bind(2, entity->PFX_SYMBOL);
            stmt.Bind(3, entity->SFX_SYMBOL);
            stmt.Bind(4, entity->DECIMAL_POINT);
            stmt.Bind(5, entity->GROUP_SEPARATOR);
            stmt.Bind(6, entity->UNIT_NAME);
            stmt.Bind(7, entity->CENT_NAME);
            stmt.Bind(8, entity->SCALE);
            stmt.Bind(9, entity->BASECONVRATE);
            stmt.Bind(10, entity->CURRENCY_SYMBOL);
            if (entity->id() > 0)
                stmt.Bind(11, entity->CURRENCYID);

            stmt.ExecuteUpdate();
            stmt.Finalize();

            if (entity->id() > 0) // existent
            {
                for(Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it)
                {
                    Self::Data* e = *it;
                    if (e->id() == entity->id()) 
                        *e = *entity;  // in-place update
                }
            }
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("CURRENCYFORMATS_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
            return false;
        }

        if (entity->id() <= 0)
        {
            entity->id((db->GetLastRowId()).ToLong());
            index_by_id_.insert(std::make_pair(entity->id(), entity));
        }
        return true;
    }

    /** Remove the Data record from the database and the memory table (cache) */
    bool remove(int id, wxSQLite3Database* db)
    {
        if (id <= 0) return false;
        try
        {
            wxString sql = "DELETE FROM CURRENCYFORMATS_V1 WHERE CURRENCYID = ?";
            wxSQLite3Statement stmt = db->PrepareStatement(sql);
            stmt.Bind(1, id);
            stmt.ExecuteUpdate();
            stmt.Finalize();

            Cache c;
            for(Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it)
            {
                Self::Data* entity = *it;
                if (entity->id() == id) 
                {
                    index_by_id_.erase(entity->id());
                    delete entity;
                }
                else 
                {
                    c.push_back(entity);
                }
            }
            cache_.clear();
            cache_.swap(c);
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("CURRENCYFORMATS_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    /** Remove the Data record from the database and the memory table (cache) */
    bool remove(Self::Data* entity, wxSQLite3Database* db)
    {
        if (remove(entity->id(), db))
        {
            entity->id(-1);
            return true;
        }

        return false;
    }

    template<typename... Args>
    Self::Data* get_one(const Args& ... args)
    {
        for (Index_By_Id::iterator it = index_by_id_.begin(); it != index_by_id_.end(); ++ it)
        {
            Self::Data* item = it->second;
            if (item->id() > 0 && match(item, args...)) 
            {
                ++ hit_;
                return item;
            }
        }

        ++ miss_;

        return 0;
    }
    
    /**
    * Search the memory table (Cache) for the data record.
    * If not found in memory, search the database and update the cache.
    */
    Self::Data* get(int id, wxSQLite3Database* db)
    {
        if (id <= 0) 
        {
            ++ skip_;
            return 0;
        }

        Index_By_Id::iterator it = index_by_id_.find(id);
        if (it != index_by_id_.end())
        {
            ++ hit_;
            return it->second;
        }
        
        ++ miss_;
        Self::Data* entity = 0;
        wxString where = wxString::Format(" WHERE %s = ?", PRIMARY::name().c_str());
        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(this->query() + where);
            stmt.Bind(1, id);

            wxSQLite3ResultSet q = stmt.ExecuteQuery();
            if(q.NextRow())
            {
                entity = new Self::Data(q, this);
                cache_.push_back(entity);
                index_by_id_.insert(std::make_pair(id, entity));
            }
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("%s: Exception %s", this->name().c_str(), e.GetMessage().c_str());
        }
        
        if (!entity) 
        {
            entity = this->fake_;
            // wxLogError("%s: %d not found", this->name().c_str(), id);
        }
 
        return entity;
    }

    /**
    * Return a list of Data records (Data_Set) derived directly from the database.
    * The Data_Set is sorted based on the column number.
    */
    const Data_Set all(wxSQLite3Database* db, COLUMN col = COLUMN(0), bool asc = true)
    {
        Data_Set result;
        try
        {
            wxSQLite3ResultSet q = db->ExecuteQuery(col == COLUMN(0) ? this->query() : this->query() + " ORDER BY " + column_to_name(col) + " COLLATE NOCASE " + (asc ? " ASC " : " DESC "));

            while(q.NextRow())
            {
                Self::Data entity(q, this);
                result.push_back(std::move(entity));
            }

            q.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("%s: Exception %s", this->name().c_str(), e.GetMessage().c_str());
        }

        return result;
    }
};
#endif //
