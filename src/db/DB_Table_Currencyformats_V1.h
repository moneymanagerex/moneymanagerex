// -*- C++ -*-
//=============================================================================
/**
 *      Copyright (c) 2013 - 2017 Guan Lisheng (guanlisheng@gmail.com)
 *      Modifications: (c) 2017 Stefano Giorgio
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2017-08-15 22:46:16.970000.
 *          DO NOT EDIT!
 */
//=============================================================================

#pragma execution_character_set("UTF-8")

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
        std::wstring to_json(json::Array& a) const
        {
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
                db->ExecuteUpdate("CREATE TABLE CURRENCYFORMATS_V1(CURRENCYID integer primary key, CURRENCYNAME TEXT COLLATE NOCASE NOT NULL UNIQUE, PFX_SYMBOL TEXT, SFX_SYMBOL TEXT, DECIMAL_POINT TEXT, GROUP_SEPARATOR TEXT, UNIT_NAME TEXT COLLATE NOCASE, CENT_NAME TEXT COLLATE NOCASE, SCALE integer, BASECONVRATE numeric, CURRENCY_SYMBOL TEXT COLLATE NOCASE NOT NULL UNIQUE, CURRENCY_TYPE TEXT /* Traditional, Crypto */)");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("CURRENCYFORMATS_V1: Exception %s", e.GetMessage().c_str());
                return false;
            }
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

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('1', '%s', '$', '', '.', ' ', '', '', '100', '1', 'USD', 'Traditional')", wxTRANSLATE("United States dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('2', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'EUR', 'Traditional')", wxTRANSLATE("European euro"), L"€"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('3', '%s', '%s', '', '.', ' ', 'Pound', 'Pence', '100', '1', 'GBP', 'Traditional')", wxTRANSLATE("UK Pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('4', '%s', '', '%s', ',', ' ', '%s', '%s', '100', '1', 'RUB', 'Traditional')", wxTRANSLATE("Russian Ruble"), L"р", L"руб.", L"коп."));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('5', '%s', '%s', '', ',', ' ', '', '', '100', '1', 'UAH', 'Traditional')", wxTRANSLATE("Ukrainian hryvnia"), L"₴"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('6', '%s', '%s', '', '.', ' ', '', 'pul', '100', '1', 'AFN', 'Traditional')", wxTRANSLATE("Afghan afghani"), L"؋"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('7', '%s', '', 'L', '.', ' ', '', '', '1', '1', 'ALL', 'Traditional')", wxTRANSLATE("Albanian lek")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('8', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'DZD', 'Traditional')", wxTRANSLATE("Algerian dinar"), L"دج"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('9', '%s', '', 'Kz', '.', ' ', '', '%s', '100', '1', 'AOA', 'Traditional')", wxTRANSLATE("Angolan kwanza"), L"Céntimo"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('10', '%s', 'EC$', '', '.', ' ', '', '', '100', '1', 'XCD', 'Traditional')", wxTRANSLATE("East Caribbean dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('11', '%s', 'AR$', '', ',', '.', '', 'centavo', '100', '1', 'ARS', 'Traditional')", wxTRANSLATE("Argentine peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('12', '%s', '', '', '.', ' ', '', '', '1', '1', 'AMD', 'Traditional')", wxTRANSLATE("Armenian dram")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('13', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'AWG', 'Traditional')", wxTRANSLATE("Aruban florin"), L"ƒ"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('14', '%s', '$', '', '.', ',', '', '', '100', '1', 'AUD', 'Traditional')", wxTRANSLATE("Australian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('15', '%s', '', '', '.', ' ', '', '', '100', '1', 'AZN', 'Traditional')", wxTRANSLATE("Azerbaijani manat")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('16', '%s', 'B$', '', '.', ' ', '', '', '100', '1', 'BSD', 'Traditional')", wxTRANSLATE("Bahamian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('17', '%s', '', '', '.', ' ', '', '', '100', '1', 'BHD', 'Traditional')", wxTRANSLATE("Bahraini dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('18', '%s', '', '', '.', ' ', '', '', '100', '1', 'BDT', 'Traditional')", wxTRANSLATE("Bangladeshi taka")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('19', '%s', 'Bds$', '', '.', ' ', '', '', '100', '1', 'BBD', 'Traditional')", wxTRANSLATE("Barbadian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('20', '%s', 'Br', '', ',', ' ', '', '', '1', '1', 'BYR', 'Traditional')", wxTRANSLATE("Belarusian ruble")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('21', '%s', 'BZ$', '', '.', ' ', '', '', '100', '1', 'BZD', 'Traditional')", wxTRANSLATE("Belize dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('22', '%s', 'CFA', '', '.', ' ', '', '', '100', '1', 'XOF', 'Traditional')", wxTRANSLATE("West African CFA franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('23', '%s', 'BD$', '', '.', ' ', '', '', '100', '1', 'BMD', 'Traditional')", wxTRANSLATE("Bermudian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('24', '%s', 'Nu.', '', '.', ' ', '', '', '100', '1', 'BTN', 'Traditional')", wxTRANSLATE("Bhutanese ngultrum")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('25', '%s', 'Bs.', '', '.', ' ', '', '', '100', '1', 'BOB', 'Traditional')", wxTRANSLATE("Bolivian boliviano")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('26', '%s', 'KM', '', ',', '.', '', '', '100', '1', 'BAM', 'Traditional')", wxTRANSLATE("Bosnia and Herzegovina konvertibilna marka")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('27', '%s', 'P', '', '.', ' ', '', '', '100', '1', 'BWP', 'Traditional')", wxTRANSLATE("Botswana pula")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('28', '%s', 'R$', '', '.', ' ', '', '', '100', '1', 'BRL', 'Traditional')", wxTRANSLATE("Brazilian real")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('29', '%s', 'B$', '', '.', ' ', '', '', '100', '1', 'BND', 'Traditional')", wxTRANSLATE("Brunei dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('30', '%s', '', '', '.', ' ', '', '', '100', '1', 'BGN', 'Traditional')", wxTRANSLATE("Bulgarian lev")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('31', '%s', 'FBu', '', '.', ' ', '', '', '1', '1', 'BIF', 'Traditional')", wxTRANSLATE("Burundi franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('32', '%s', '', '', '.', ' ', '', '', '100', '1', 'KHR', 'Traditional')", wxTRANSLATE("Cambodian riel")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('33', '%s', 'CFA', '', '.', ' ', '', '', '1', '1', 'XAF', 'Traditional')", wxTRANSLATE("Central African CFA franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('34', '%s', '$', '', '.', ' ', '', '', '100', '1', 'CAD', 'Traditional')", wxTRANSLATE("Canadian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('35', '%s', 'Esc', '', '.', ' ', '', '', '100', '1', 'CVE', 'Traditional')", wxTRANSLATE("Cape Verdean escudo")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('36', '%s', 'KY$', '', '.', ' ', '', '', '100', '1', 'KYD', 'Traditional')", wxTRANSLATE("Cayman Islands dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('37', '%s', '$', '', '.', ' ', '', '', '1', '1', 'CLP', 'Traditional')", wxTRANSLATE("Chilean peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('38', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'CNY', 'Traditional')", wxTRANSLATE("Chinese renminbi"), L"¥"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('39', '%s', 'Col$', '', '.', ' ', '', '', '100', '1', 'COP', 'Traditional')", wxTRANSLATE("Colombian peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('40', '%s', '', '', '.', ' ', '', '', '1', '1', 'KMF', 'Traditional')", wxTRANSLATE("Comorian franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('41', '%s', 'F', '', '.', ' ', '', '', '100', '1', 'CDF', 'Traditional')", wxTRANSLATE("Congolese franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('42', '%s', '%s', '', '.', ' ', '', '', '1', '1', 'CRC', 'Traditional')", wxTRANSLATE("Costa Rican colon"), L"₡"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('43', '%s', 'kn', '', '.', ' ', '', '', '100', '1', 'HRK', 'Traditional')", wxTRANSLATE("Croatian kuna")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('44', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'CZK', 'Traditional')", wxTRANSLATE("Czech koruna"), L"Kč"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('45', '%s', 'Kr', '', '.', ' ', '', '', '100', '1', 'DKK', 'Traditional')", wxTRANSLATE("Danish krone")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('46', '%s', 'Fdj', '', '.', ' ', '', '', '1', '1', 'DJF', 'Traditional')", wxTRANSLATE("Djiboutian franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('47', '%s', 'RD$', '', '.', ' ', '', '', '100', '1', 'DOP', 'Traditional')", wxTRANSLATE("Dominican peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('48', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'EGP', 'Traditional')", wxTRANSLATE("Egyptian pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('49', '%s', 'Nfa', '', '.', ' ', '', '', '100', '1', 'ERN', 'Traditional')", wxTRANSLATE("Eritrean nakfa")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('50', '%s', 'Br', '', '.', ' ', '', '', '100', '1', 'ETB', 'Traditional')", wxTRANSLATE("Ethiopian birr")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('51', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'FKP', 'Traditional')", wxTRANSLATE("Falkland Islands pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('52', '%s', 'FJ$', '', '.', ' ', '', '', '100', '1', 'FJD', 'Traditional')", wxTRANSLATE("Fijian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('53', '%s', 'F', '', '.', ' ', '', '', '100', '1', 'XPF', 'Traditional')", wxTRANSLATE("CFP franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('54', '%s', 'D', '', '.', ' ', '', '', '100', '1', 'GMD', 'Traditional')", wxTRANSLATE("Gambian dalasi")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('55', '%s', '', '', '.', ' ', '', '', '100', '1', 'GEL', 'Traditional')", wxTRANSLATE("Georgian lari")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('56', '%s', '', '', '.', ' ', '', '', '100', '1', 'GHS', 'Traditional')", wxTRANSLATE("Ghanaian cedi")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('57', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'GIP', 'Traditional')", wxTRANSLATE("Gibraltar pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('58', '%s', 'Q', '', '.', ' ', '', '', '100', '1', 'GTQ', 'Traditional')", wxTRANSLATE("Guatemalan quetzal")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('59', '%s', 'FG', '', '.', ' ', '', '', '1', '1', 'GNF', 'Traditional')", wxTRANSLATE("Guinean franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('60', '%s', 'GY$', '', '.', ' ', '', '', '100', '1', 'GYD', 'Traditional')", wxTRANSLATE("Guyanese dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('61', '%s', 'G', '', '.', ' ', '', '', '100', '1', 'HTG', 'Traditional')", wxTRANSLATE("Haitian gourde")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('62', '%s', 'L', '', '.', ' ', '', '', '100', '1', 'HNL', 'Traditional')", wxTRANSLATE("Honduran lempira")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('63', '%s', 'HK$', '', '.', ' ', '', '', '100', '1', 'HKD', 'Traditional')", wxTRANSLATE("Hong Kong dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('64', '%s', 'Ft', '', '.', ' ', '', '', '1', '1', 'HUF', 'Traditional')", wxTRANSLATE("Hungarian forint")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('65', '%s', 'kr', '', '.', ' ', '', '', '1', '1', 'ISK', 'Traditional')", L"Icelandic króna"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('66', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'INR', 'Traditional')", wxTRANSLATE("Indian rupee"), L"₹"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('67', '%s', 'Rp', '', '.', ' ', '', '', '1', '1', 'IDR', 'Traditional')", wxTRANSLATE("Indonesian rupiah")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('68', '%s', 'SDR', '', '.', ' ', '', '', '100', '1', 'XDR', 'Traditional')", wxTRANSLATE("Special Drawing Rights")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('69', '%s', '', '', '.', ' ', '', '', '1', '1', 'IRR', 'Traditional')", wxTRANSLATE("Iranian rial")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('70', '%s', '', '', '.', ' ', '', '', '1', '1', 'IQD', 'Traditional')", wxTRANSLATE("Iraqi dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('71', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'ILS', 'Traditional')", wxTRANSLATE("Israeli new shekel"), L"₪"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('72', '%s', 'J$', '', '.', ' ', '', '', '100', '1', 'JMD', 'Traditional')", wxTRANSLATE("Jamaican dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('73', '%s', '%s', '', '.', ' ', '', '', '1', '1', 'JPY', 'Traditional')", wxTRANSLATE("Japanese yen"), L"¥"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('74', '%s', '', '', '.', ' ', '', '', '100', '1', 'JOD', 'Traditional')", wxTRANSLATE("Jordanian dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('75', '%s', 'T', '', '.', ' ', '', '', '100', '1', 'KZT', 'Traditional')", wxTRANSLATE("Kazakhstani tenge")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('76', '%s', 'KSh', '', '.', ' ', '', '', '100', '1', 'KES', 'Traditional')", wxTRANSLATE("Kenyan shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('77', '%s', 'W', '', '.', ' ', '', '', '100', '1', 'KPW', 'Traditional')", wxTRANSLATE("North Korean won")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('78', '%s', 'W', '', '.', ' ', '', '', '1', '1', 'KRW', 'Traditional')", wxTRANSLATE("South Korean won")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('79', '%s', '', '', '.', ' ', '', '', '100', '1', 'KWD', 'Traditional')", wxTRANSLATE("Kuwaiti dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('80', '%s', '', '', '.', ' ', '', '', '100', '1', 'KGS', 'Traditional')", wxTRANSLATE("Kyrgyzstani som")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('81', '%s', 'KN', '', '.', ' ', '', '', '100', '1', 'LAK', 'Traditional')", wxTRANSLATE("Lao kip")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('82', '%s', 'Ls', '', '.', ' ', '', '', '100', '1', 'LVL', 'Traditional')", wxTRANSLATE("Latvian lats")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('83', '%s', '', '', '.', ' ', '', '', '1', '1', 'LBP', 'Traditional')", wxTRANSLATE("Lebanese lira")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('84', '%s', 'M', '', '.', ' ', '', '', '100', '1', 'LSL', 'Traditional')", wxTRANSLATE("Lesotho loti")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('85', '%s', 'L$', '', '.', ' ', '', '', '100', '1', 'LRD', 'Traditional')", wxTRANSLATE("Liberian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('86', '%s', 'LD', '', '.', ' ', '', '', '100', '1', 'LYD', 'Traditional')", wxTRANSLATE("Libyan dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('87', '%s', 'Lt', '', '.', ' ', '', '', '100', '1', 'LTL', 'Traditional')", wxTRANSLATE("Lithuanian litas")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('88', '%s', 'P', '', '.', ' ', '', '', '100', '1', 'MOP', 'Traditional')", wxTRANSLATE("Macanese pataca")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('89', '%s', '', '', '.', ' ', '', '', '100', '1', 'MKD', 'Traditional')", wxTRANSLATE("Macedonian denar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('90', '%s', 'FMG', '', '.', ' ', '', '', '100', '1', 'MGA', 'Traditional')", wxTRANSLATE("Malagasy ariary")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('91', '%s', 'MK', '', '.', ' ', '', '', '1', '1', 'MWK', 'Traditional')", wxTRANSLATE("Malawian kwacha")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('92', '%s', 'RM', '', '.', ' ', '', '', '100', '1', 'MYR', 'Traditional')", wxTRANSLATE("Malaysian ringgit")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('93', '%s', 'Rf', '', '.', ' ', '', '', '100', '1', 'MVR', 'Traditional')", wxTRANSLATE("Maldivian rufiyaa")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('94', '%s', 'UM', '', '.', ' ', '', '', '100', '1', 'MRO', 'Traditional')", wxTRANSLATE("Mauritanian ouguiya")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('95', '%s', 'Rs', '', '.', ' ', '', '', '1', '1', 'MUR', 'Traditional')", wxTRANSLATE("Mauritian rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('96', '%s', '$', '', '.', ' ', '', '', '100', '1', 'MXN', 'Traditional')", wxTRANSLATE("Mexican peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('97', '%s', '', '', '.', ' ', '', '', '100', '1', 'MDL', 'Traditional')", wxTRANSLATE("Moldovan leu")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('98', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'MNT', 'Traditional')", wxTRANSLATE("Mongolian tugrik"), L"₮"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('99', '%s', '', '', '.', ' ', '', '', '100', '1', 'MAD', 'Traditional')", wxTRANSLATE("Moroccan dirham")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('100', '%s', 'K', '', '.', ' ', '', '', '1', '1', 'MMK', 'Traditional')", wxTRANSLATE("Myanma kyat")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('101', '%s', 'N$', '', '.', ' ', '', '', '100', '1', 'NAD', 'Traditional')", wxTRANSLATE("Namibian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('102', '%s', 'NRs', '', '.', ' ', '', '', '100', '1', 'NPR', 'Traditional')", wxTRANSLATE("Nepalese rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('103', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'ANG', 'Traditional')", wxTRANSLATE("Netherlands Antillean gulden"), L"NAƒ"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('104', '%s', 'NZ$', '', '.', ' ', '', '', '100', '1', 'NZD', 'Traditional')", wxTRANSLATE("New Zealand dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('105', '%s', 'C$', '', '.', ' ', '', '', '100', '1', 'NIO', 'Traditional')", L"Nicaraguan córdoba"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('106', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'NGN', 'Traditional')", wxTRANSLATE("Nigerian naira"), L"₦"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('107', '%s', 'kr', '', '.', ' ', '', '', '100', '1', 'NOK', 'Traditional')", wxTRANSLATE("Norwegian krone")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('108', '%s', '', '', '.', ' ', '', '', '100', '1', 'OMR', 'Traditional')", wxTRANSLATE("Omani rial")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('109', '%s', 'Rs.', '', '.', ' ', '', '', '1', '1', 'PKR', 'Traditional')", wxTRANSLATE("Pakistani rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('110', '%s', 'B./', '', '.', ' ', '', '', '100', '1', 'PAB', 'Traditional')", wxTRANSLATE("Panamanian balboa")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('111', '%s', 'K', '', '.', ' ', '', '', '100', '1', 'PGK', 'Traditional')", wxTRANSLATE("Papua New Guinean kina")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('112', '%s', '', '', '.', ' ', '', '', '1', '1', 'PYG', 'Traditional')", wxTRANSLATE("Paraguayan guarani")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('113', '%s', 'S/.', '', '.', ' ', '', '', '100', '1', 'PEN', 'Traditional')", wxTRANSLATE("Peruvian nuevo sol")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('114', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'PHP', 'Traditional')", wxTRANSLATE("Philippine peso"), L"₱"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('115', '%s', '', '%s', ',', '.', '%s', 'grosz', '100', '1', 'PLN', 'Traditional')", wxTRANSLATE("Polish zloty"), L"zł", L"złoty"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('116', '%s', 'QR', '', '.', ' ', '', '', '100', '1', 'QAR', 'Traditional')", wxTRANSLATE("Qatari riyal")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('117', '%s', 'L', '', '.', ' ', '', '', '100', '1', 'RON', 'Traditional')", wxTRANSLATE("Romanian leu")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('118', '%s', 'RF', '', '.', ' ', '', '', '1', '1', 'RWF', 'Traditional')", wxTRANSLATE("Rwandan franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('119', '%s', 'Db', '', '.', ' ', '', '', '100', '1', 'STD', 'Traditional')", L"São Tomé and Príncipe dobra"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('120', '%s', 'SR', '', '.', ' ', '', '', '100', '1', 'SAR', 'Traditional')", wxTRANSLATE("Saudi riyal")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('121', '%s', 'din.', '', '.', ' ', '', '', '1', '1', 'RSD', 'Traditional')", wxTRANSLATE("Serbian dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('122', '%s', 'SR', '', '.', ' ', '', '', '100', '1', 'SCR', 'Traditional')", wxTRANSLATE("Seychellois rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('123', '%s', 'Le', '', '.', ' ', '', '', '100', '1', 'SLL', 'Traditional')", wxTRANSLATE("Sierra Leonean leone")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('124', '%s', 'S$', '', '.', ' ', '', '', '100', '1', 'SGD', 'Traditional')", wxTRANSLATE("Singapore dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('125', '%s', 'SI$', '', '.', ' ', '', '', '100', '1', 'SBD', 'Traditional')", wxTRANSLATE("Solomon Islands dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('126', '%s', 'Sh.', '', '.', ' ', '', '', '1', '1', 'SOS', 'Traditional')", wxTRANSLATE("Somali shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('127', '%s', 'R', '', '.', ' ', '', '', '100', '1', 'ZAR', 'Traditional')", wxTRANSLATE("South African rand")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('128', '%s', 'Rs', '', '.', ' ', '', '', '100', '1', 'LKR', 'Traditional')", wxTRANSLATE("Sri Lankan rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('129', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'SHP', 'Traditional')", wxTRANSLATE("Saint Helena pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('130', '%s', '', '', '.', ' ', '', '', '100', '1', 'SDG', 'Traditional')", wxTRANSLATE("Sudanese pound")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('131', '%s', '$', '', '.', ' ', '', '', '100', '1', 'SRD', 'Traditional')", wxTRANSLATE("Surinamese dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('132', '%s', 'E', '', '.', ' ', '', '', '100', '1', 'SZL', 'Traditional')", wxTRANSLATE("Swazi lilangeni")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('133', '%s', 'kr', '', '.', ' ', '', '', '100', '1', 'SEK', 'Traditional')", wxTRANSLATE("Swedish krona")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('134', '%s', 'Fr.', '', '.', ' ', '', '', '100', '1', 'CHF', 'Traditional')", wxTRANSLATE("Swiss franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('135', '%s', '', '', '.', ' ', '', '', '1', '1', 'SYP', 'Traditional')", wxTRANSLATE("Syrian pound")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('136', '%s', 'NT$', '', '.', ' ', '', '', '100', '1', 'TWD', 'Traditional')", wxTRANSLATE("New Taiwan dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('137', '%s', '', '', '.', ' ', '', '', '100', '1', 'TJS', 'Traditional')", wxTRANSLATE("Tajikistani somoni")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('138', '%s', '', '', '.', ' ', '', '', '1', '1', 'TZS', 'Traditional')", wxTRANSLATE("Tanzanian shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('139', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'THB', 'Traditional')", wxTRANSLATE("Thai baht"), L"฿"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('140', '%s', 'TT$', '', '.', ' ', '', '', '100', '1', 'TTD', 'Traditional')", wxTRANSLATE("Trinidad and Tobago dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('141', '%s', 'DT', '', '.', ' ', '', '', '100', '1', 'TND', 'Traditional')", wxTRANSLATE("Tunisian dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('142', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'TRY', 'Traditional')", wxTRANSLATE("Turkish lira"), L"₺"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('143', '%s', 'm', '', '.', ' ', '', '', '100', '1', 'TMT', 'Traditional')", wxTRANSLATE("Turkmen manat")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('144', '%s', 'USh', '', '.', ' ', '', '', '1', '1', 'UGX', 'Traditional')", wxTRANSLATE("Ugandan shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('145', '%s', '', '', '.', ' ', '', '', '100', '1', 'AED', 'Traditional')", wxTRANSLATE("UAE dirham")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('146', '%s', '$U', '', '.', ' ', '', '', '100', '1', 'UYU', 'Traditional')", wxTRANSLATE("Uruguayan peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('147', '%s', '', '', '.', ' ', '', '', '1', '1', 'UZS', 'Traditional')", wxTRANSLATE("Uzbekistani som")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('148', '%s', 'VT', '', '.', ' ', '', '', '100', '1', 'VUV', 'Traditional')", wxTRANSLATE("Vanuatu vatu")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('149', '%s', '%s', '', '.', ' ', '', '', '1', '1', 'VND', 'Traditional')", wxTRANSLATE("Vietnamese dong"), L"₫"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('150', '%s', 'WS$', '', '.', ' ', '', '', '100', '1', 'WST', 'Traditional')", wxTRANSLATE("Samoan tala")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('151', '%s', '', '', '.', ' ', '', '', '1', '1', 'YER', 'Traditional')", wxTRANSLATE("Yemeni rial")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('152', '%s', 'Bs.', '', '.', ',', '%s', '%s', '100', '1', 'VEF', 'Traditional')", L"Venezuelan Bolívar", L"bolívar", L"céntimos"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('153', '%s', '%s', '', '.', ',', '', '', '100000000', '1', 'BTC', 'Crypto')", wxTRANSLATE("Bitcoin"), L"Ƀ"));
        db->Commit();
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
    struct CURRENCY_TYPE : public DB_Column<wxString>
    { 
        static wxString name() { return "CURRENCY_TYPE"; } 
        explicit CURRENCY_TYPE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
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
        , COL_CURRENCY_TYPE = 11
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
            case COL_CURRENCY_TYPE: return "CURRENCY_TYPE";
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
        else if ("CURRENCY_TYPE" == name) return COL_CURRENCY_TYPE;

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
        wxString CURRENCY_TYPE;
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
            CURRENCY_TYPE = q.GetString(11); // CURRENCY_TYPE
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
            CURRENCY_TYPE = other.CURRENCY_TYPE;
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
        bool match(const Self::CURRENCY_TYPE &in) const
        {
            return this->CURRENCY_TYPE.CmpNoCase(in.v_) == 0;
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
            o[L"CURRENCY_TYPE"] = json::String(this->CURRENCY_TYPE.ToStdWstring());
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
            row(L"CURRENCY_TYPE") = CURRENCY_TYPE;
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
            t(L"CURRENCY_TYPE") = CURRENCY_TYPE;
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
        NUM_COLUMNS = 12
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "CURRENCYFORMATS_V1"; }

    DB_Table_CURRENCYFORMATS_V1() : fake_(new Data())
    {
        query_ = "SELECT CURRENCYID, CURRENCYNAME, PFX_SYMBOL, SFX_SYMBOL, DECIMAL_POINT, GROUP_SEPARATOR, UNIT_NAME, CENT_NAME, SCALE, BASECONVRATE, CURRENCY_SYMBOL, CURRENCY_TYPE FROM CURRENCYFORMATS_V1 ";
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
            sql = "INSERT INTO CURRENCYFORMATS_V1(CURRENCYNAME, PFX_SYMBOL, SFX_SYMBOL, DECIMAL_POINT, GROUP_SEPARATOR, UNIT_NAME, CENT_NAME, SCALE, BASECONVRATE, CURRENCY_SYMBOL, CURRENCY_TYPE) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE CURRENCYFORMATS_V1 SET CURRENCYNAME = ?, PFX_SYMBOL = ?, SFX_SYMBOL = ?, DECIMAL_POINT = ?, GROUP_SEPARATOR = ?, UNIT_NAME = ?, CENT_NAME = ?, SCALE = ?, BASECONVRATE = ?, CURRENCY_SYMBOL = ?, CURRENCY_TYPE = ? WHERE CURRENCYID = ?";
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
            stmt.Bind(11, entity->CURRENCY_TYPE);
            if (entity->id() > 0)
                stmt.Bind(12, entity->CURRENCYID);

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
