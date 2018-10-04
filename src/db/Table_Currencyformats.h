// -*- C++ -*-
/** @file
 * @brief     CRUD implementation for CURRENCYFORMATS SQLite table
 * @warning   Auto generated with sqlite2cpp.py script. DO NOT EDIT!
 * @copyright © 2013-2018 Guan Lisheng
 * @copyright © 2017-2018 Stefano Giorgio
 * @author    Guan Lisheng (guanlisheng@gmail.com)
 * @author    Stefano Giorgio (stef145g)
 * @author    Tomasz Słodkowicz
 * @date      2018-10-07 02:45:31.001407
 */
#pragma execution_character_set("UTF-8")
#pragma once

#include "Table.h"

struct DB_Table_CURRENCYFORMATS : public DB_Table
{
    struct Data;
    typedef DB_Table_CURRENCYFORMATS Self;

    /** A container to hold list of Data records for the table*/
    struct Data_Set : public std::vector<Self::Data>
    {
        /** Return the data records as a json array string */
        wxString to_json() const
        {
            StringBuffer json_buffer;
            PrettyWriter<StringBuffer> json_writer(json_buffer);

            json_writer.StartArray();
            for (const auto & item: *this)
            {
                json_writer.StartObject();
                item.as_json(json_writer);
                json_writer.EndObject();
            }
            json_writer.EndArray();

            return json_buffer.GetString();
        }
    };

    /** A container to hold a list of Data record pointers for the table in memory*/
    typedef std::vector<Self::Data*> Cache;
    typedef std::map<int, Self::Data*> Index_By_Id;
    Cache cache_;
    Index_By_Id index_by_id_;
    Data* fake_; // in case the entity not found

    /** Destructor: clears any data records stored in memory */
    ~DB_Table_CURRENCYFORMATS() 
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
                db->ExecuteUpdate("CREATE TABLE CURRENCYFORMATS(CURRENCYID integer primary key, CURRENCYNAME TEXT COLLATE NOCASE NOT NULL, PFX_SYMBOL TEXT, SFX_SYMBOL TEXT, DECIMAL_POINT TEXT, GROUP_SEPARATOR TEXT, SCALE integer, CURRENCY_SYMBOL TEXT COLLATE NOCASE NOT NULL UNIQUE, CURRENCY_TYPE TEXT /* Fiat, Crypto */, HISTORIC integer DEFAULT 0 /* 1 if no longer official */)");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("CURRENCYFORMATS: Exception %s", e.GetMessage().c_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CURRENCYFORMATS_SYMBOL ON CURRENCYFORMATS(CURRENCY_SYMBOL)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("CURRENCYFORMATS: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('1', '%s', '$', '', '.', ' ', '100', 'USD', 'Fiat', '0')", wxTRANSLATE("US Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('2', '%s', '%s', '', '.', ' ', '100', 'EUR', 'Fiat', '0')", wxTRANSLATE("Euro"), L"€"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('3', '%s', '%s', '', '.', ' ', '100', 'GBP', 'Fiat', '0')", wxTRANSLATE("Pound Sterling"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('4', '%s', '', '%s', ',', ' ', '100', 'RUB', 'Fiat', '0')", wxTRANSLATE("Russian Ruble"), L"р"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('5', '%s', '%s', '', ',', ' ', '100', 'UAH', 'Fiat', '0')", wxTRANSLATE("Hryvnia"), L"₴"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('6', '%s', '%s', '', '.', ' ', '100', 'AFN', 'Fiat', '0')", wxTRANSLATE("Afghani"), L"؋"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('7', '%s', '', 'L', '.', ' ', '100', 'ALL', 'Fiat', '0')", wxTRANSLATE("Lek")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('8', '%s', '%s', '', '.', ' ', '100', 'DZD', 'Fiat', '0')", wxTRANSLATE("Algerian Dinar"), L"دج"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('9', '%s', '', 'Kz', '.', ' ', '100', 'AOA', 'Fiat', '0')", wxTRANSLATE("Kwanza")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('10', '%s', 'EC$', '', '.', ' ', '100', 'XCD', 'Fiat', '0')", wxTRANSLATE("East Caribbean Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('11', '%s', 'AR$', '', ',', '.', '100', 'ARS', 'Fiat', '0')", wxTRANSLATE("Argentine Peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('12', '%s', '', '', '.', ' ', '100', 'AMD', 'Fiat', '0')", wxTRANSLATE("Armenian Dram")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('13', '%s', '%s', '', '.', ' ', '100', 'AWG', 'Fiat', '0')", wxTRANSLATE("Aruban Florin"), L"ƒ"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('14', '%s', '$', '', '.', ',', '100', 'AUD', 'Fiat', '0')", wxTRANSLATE("Australian Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('15', '%s', '', '', '.', ' ', '100', 'AZN', 'Fiat', '0')", wxTRANSLATE("Azerbaijan Manat")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('16', '%s', 'B$', '', '.', ' ', '100', 'BSD', 'Fiat', '0')", wxTRANSLATE("Bahamian Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('17', '%s', '', '', '.', ' ', '1000', 'BHD', 'Fiat', '0')", wxTRANSLATE("Bahraini Dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('18', '%s', '', '', '.', ' ', '100', 'BDT', 'Fiat', '0')", wxTRANSLATE("Taka")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('19', '%s', 'Bds$', '', '.', ' ', '100', 'BBD', 'Fiat', '0')", wxTRANSLATE("Barbados Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('20', '%s', 'Br', '', ',', ' ', '1', 'BYR', 'Fiat', '1')", wxTRANSLATE("Belarusian Ruble (before 2017-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('21', '%s', 'BZ$', '', '.', ' ', '100', 'BZD', 'Fiat', '0')", wxTRANSLATE("Belize Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('22', '%s', 'CFA', '', '.', ' ', '1', 'XOF', 'Fiat', '0')", wxTRANSLATE("CFA Franc BCEAO")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('23', '%s', 'BD$', '', '.', ' ', '100', 'BMD', 'Fiat', '0')", wxTRANSLATE("Bermudian Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('24', '%s', 'Nu.', '', '.', ' ', '100', 'BTN', 'Fiat', '0')", wxTRANSLATE("Ngultrum")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('25', '%s', 'Bs.', '', '.', ' ', '100', 'BOB', 'Fiat', '0')", wxTRANSLATE("Boliviano")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('26', '%s', 'KM', '', ',', '.', '100', 'BAM', 'Fiat', '0')", wxTRANSLATE("Convertible Mark")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('27', '%s', 'P', '', '.', ' ', '100', 'BWP', 'Fiat', '0')", wxTRANSLATE("Pula")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('28', '%s', 'R$', '', '.', ' ', '100', 'BRL', 'Fiat', '0')", wxTRANSLATE("Brazilian Real")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('29', '%s', 'B$', '', '.', ' ', '100', 'BND', 'Fiat', '0')", wxTRANSLATE("Brunei Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('30', '%s', '', '', '.', ' ', '100', 'BGN', 'Fiat', '0')", wxTRANSLATE("Bulgarian Lev")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('31', '%s', 'FBu', '', '.', ' ', '1', 'BIF', 'Fiat', '0')", wxTRANSLATE("Burundi Franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('32', '%s', '', '', '.', ' ', '100', 'KHR', 'Fiat', '0')", wxTRANSLATE("Riel")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('33', '%s', 'CFA', '', '.', ' ', '1', 'XAF', 'Fiat', '0')", wxTRANSLATE("CFA Franc BEAC")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('34', '%s', '$', '', '.', ' ', '100', 'CAD', 'Fiat', '0')", wxTRANSLATE("Canadian Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('35', '%s', 'Esc', '', '.', ' ', '100', 'CVE', 'Fiat', '0')", wxTRANSLATE("Cabo Verde Escudo")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('36', '%s', 'KY$', '', '.', ' ', '100', 'KYD', 'Fiat', '0')", wxTRANSLATE("Cayman Islands Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('37', '%s', '$', '', '.', ' ', '1', 'CLP', 'Fiat', '0')", wxTRANSLATE("Chilean Peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('38', '%s', '%s', '', '.', ' ', '100', 'CNY', 'Fiat', '0')", wxTRANSLATE("Yuan Renminbi"), L"¥"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('39', '%s', 'Col$', '', '.', ' ', '100', 'COP', 'Fiat', '0')", wxTRANSLATE("Colombian Peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('40', '%s', '', '', '.', ' ', '1', 'KMF', 'Fiat', '0')", wxTRANSLATE("Comorian Franc ")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('41', '%s', 'F', '', '.', ' ', '100', 'CDF', 'Fiat', '0')", wxTRANSLATE("Congolese Franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('42', '%s', '%s', '', '.', ' ', '100', 'CRC', 'Fiat', '0')", wxTRANSLATE("Costa Rican Colon"), L"₡"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('43', '%s', 'kn', '', '.', ' ', '100', 'HRK', 'Fiat', '0')", wxTRANSLATE("Kuna")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('44', '%s', '%s', '', '.', ' ', '100', 'CZK', 'Fiat', '0')", wxTRANSLATE("Czech Koruna"), L"Kč"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('45', '%s', 'Kr', '', '.', ' ', '100', 'DKK', 'Fiat', '0')", wxTRANSLATE("Danish Krone")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('46', '%s', 'Fdj', '', '.', ' ', '1', 'DJF', 'Fiat', '0')", wxTRANSLATE("Djibouti Franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('47', '%s', 'RD$', '', '.', ' ', '100', 'DOP', 'Fiat', '0')", wxTRANSLATE("Dominican Peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('48', '%s', '%s', '', '.', ' ', '100', 'EGP', 'Fiat', '0')", wxTRANSLATE("Egyptian Pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('49', '%s', 'Nfa', '', '.', ' ', '100', 'ERN', 'Fiat', '0')", wxTRANSLATE("Nakfa")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('50', '%s', 'Br', '', '.', ' ', '100', 'ETB', 'Fiat', '0')", wxTRANSLATE("Ethiopian Birr")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('51', '%s', '%s', '', '.', ' ', '100', 'FKP', 'Fiat', '0')", wxTRANSLATE("Falkland Islands Pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('52', '%s', 'FJ$', '', '.', ' ', '100', 'FJD', 'Fiat', '0')", wxTRANSLATE("Fiji Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('53', '%s', 'F', '', '.', ' ', '1', 'XPF', 'Fiat', '0')", wxTRANSLATE("CFP Franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('54', '%s', 'D', '', '.', ' ', '100', 'GMD', 'Fiat', '0')", wxTRANSLATE("Dalasi")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('55', '%s', '', '', '.', ' ', '100', 'GEL', 'Fiat', '0')", wxTRANSLATE("Lari")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('56', '%s', '', '', '.', ' ', '100', 'GHS', 'Fiat', '0')", wxTRANSLATE("Ghana Cedi")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('57', '%s', '%s', '', '.', ' ', '100', 'GIP', 'Fiat', '0')", wxTRANSLATE("Gibraltar Pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('58', '%s', 'Q', '', '.', ' ', '100', 'GTQ', 'Fiat', '0')", wxTRANSLATE("Quetzal")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('59', '%s', 'FG', '', '.', ' ', '1', 'GNF', 'Fiat', '0')", wxTRANSLATE("Guinean Franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('60', '%s', 'GY$', '', '.', ' ', '100', 'GYD', 'Fiat', '0')", wxTRANSLATE("Guyana Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('61', '%s', 'G', '', '.', ' ', '100', 'HTG', 'Fiat', '0')", wxTRANSLATE("Gourde")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('62', '%s', 'L', '', '.', ' ', '100', 'HNL', 'Fiat', '0')", wxTRANSLATE("Lempira")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('63', '%s', 'HK$', '', '.', ' ', '100', 'HKD', 'Fiat', '0')", wxTRANSLATE("Hong Kong Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('64', '%s', 'Ft', '', '.', ' ', '100', 'HUF', 'Fiat', '0')", wxTRANSLATE("Forint")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('65', '%s', 'kr', '', '.', ' ', '1', 'ISK', 'Fiat', '0')", wxTRANSLATE("Iceland Krona")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('66', '%s', '%s', '', '.', ' ', '100', 'INR', 'Fiat', '0')", wxTRANSLATE("Indian Rupee"), L"₹"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('67', '%s', 'Rp', '', '.', ' ', '100', 'IDR', 'Fiat', '0')", wxTRANSLATE("Rupiah")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('68', '%s', 'SDR', '', '.', ' ', '100', 'XDR', 'Fiat', '0')", wxTRANSLATE("Special Drawing Rights")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('69', '%s', '', '', '.', ' ', '100', 'IRR', 'Fiat', '0')", wxTRANSLATE("Iranian Rial")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('70', '%s', '', '', '.', ' ', '1000', 'IQD', 'Fiat', '0')", wxTRANSLATE("Iraqi Dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('71', '%s', '%s', '', '.', ' ', '100', 'ILS', 'Fiat', '0')", wxTRANSLATE("New Israeli Sheqel"), L"₪"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('72', '%s', 'J$', '', '.', ' ', '100', 'JMD', 'Fiat', '0')", wxTRANSLATE("Jamaican Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('73', '%s', '%s', '', '.', ' ', '1', 'JPY', 'Fiat', '0')", wxTRANSLATE("Yen"), L"¥"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('74', '%s', '', '', '.', ' ', '1000', 'JOD', 'Fiat', '0')", wxTRANSLATE("Jordanian Dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('75', '%s', 'T', '', '.', ' ', '100', 'KZT', 'Fiat', '0')", wxTRANSLATE("Tenge")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('76', '%s', 'KSh', '', '.', ' ', '100', 'KES', 'Fiat', '0')", wxTRANSLATE("Kenyan Shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('77', '%s', 'W', '', '.', ' ', '100', 'KPW', 'Fiat', '0')", wxTRANSLATE("North Korean Won")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('78', '%s', 'W', '', '.', ' ', '1', 'KRW', 'Fiat', '0')", wxTRANSLATE("Won")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('79', '%s', '', '', '.', ' ', '1000', 'KWD', 'Fiat', '0')", wxTRANSLATE("Kuwaiti Dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('80', '%s', '', '', '.', ' ', '100', 'KGS', 'Fiat', '0')", wxTRANSLATE("Som")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('81', '%s', 'KN', '', '.', ' ', '100', 'LAK', 'Fiat', '0')", wxTRANSLATE("Lao Kip")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('82', '%s', 'Ls', '', '.', ' ', '100', 'LVL', 'Fiat', '1')", wxTRANSLATE("Latvian Lats (before 2014-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('83', '%s', '', '', '.', ' ', '100', 'LBP', 'Fiat', '0')", wxTRANSLATE("Lebanese Pound")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('84', '%s', 'M', '', '.', ' ', '100', 'LSL', 'Fiat', '0')", wxTRANSLATE("Loti")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('85', '%s', 'L$', '', '.', ' ', '100', 'LRD', 'Fiat', '0')", wxTRANSLATE("Liberian Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('86', '%s', 'LD', '', '.', ' ', '1000', 'LYD', 'Fiat', '0')", wxTRANSLATE("Libyan Dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('87', '%s', 'Lt', '', '.', ' ', '100', 'LTL', 'Fiat', '1')", wxTRANSLATE("Lithuanian Litas (before 2014-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('88', '%s', 'P', '', '.', ' ', '100', 'MOP', 'Fiat', '0')", wxTRANSLATE("Pataca")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('89', '%s', '', '', '.', ' ', '100', 'MKD', 'Fiat', '0')", wxTRANSLATE("Denar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('90', '%s', 'FMG', '', '.', ' ', '100', 'MGA', 'Fiat', '0')", wxTRANSLATE("Malagasy Ariary")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('91', '%s', 'MK', '', '.', ' ', '100', 'MWK', 'Fiat', '0')", wxTRANSLATE("Malawi Kwacha")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('92', '%s', 'RM', '', '.', ' ', '100', 'MYR', 'Fiat', '0')", wxTRANSLATE("Malaysian Ringgit")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('93', '%s', 'Rf', '', '.', ' ', '100', 'MVR', 'Fiat', '0')", wxTRANSLATE("Rufiyaa")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('94', '%s', 'UM', '', '.', ' ', '100', 'MRO', 'Fiat', '1')", wxTRANSLATE("Ouguiya (before 2017-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('95', '%s', 'Rs', '', '.', ' ', '100', 'MUR', 'Fiat', '0')", wxTRANSLATE("Mauritius Rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('96', '%s', '$', '', '.', ' ', '100', 'MXN', 'Fiat', '0')", wxTRANSLATE("Mexican Peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('97', '%s', '', '', '.', ' ', '100', 'MDL', 'Fiat', '0')", wxTRANSLATE("Moldovan Leu")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('98', '%s', '%s', '', '.', ' ', '100', 'MNT', 'Fiat', '0')", wxTRANSLATE("Tugrik"), L"₮"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('99', '%s', '', '', '.', ' ', '100', 'MAD', 'Fiat', '0')", wxTRANSLATE("Moroccan Dirham")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('100', '%s', 'K', '', '.', ' ', '100', 'MMK', 'Fiat', '0')", wxTRANSLATE("Kyat")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('101', '%s', 'N$', '', '.', ' ', '100', 'NAD', 'Fiat', '0')", wxTRANSLATE("Namibia Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('102', '%s', 'NRs', '', '.', ' ', '100', 'NPR', 'Fiat', '0')", wxTRANSLATE("Nepalese Rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('103', '%s', '%s', '', '.', ' ', '100', 'ANG', 'Fiat', '0')", wxTRANSLATE("Netherlands Antillean Guilder"), L"NAƒ"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('104', '%s', 'NZ$', '', '.', ' ', '100', 'NZD', 'Fiat', '0')", wxTRANSLATE("New Zealand Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('105', '%s', 'C$', '', '.', ' ', '100', 'NIO', 'Fiat', '0')", wxTRANSLATE("Cordoba Oro")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('106', '%s', '%s', '', '.', ' ', '100', 'NGN', 'Fiat', '0')", wxTRANSLATE("Naira"), L"₦"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('107', '%s', 'kr', '', '.', ' ', '100', 'NOK', 'Fiat', '0')", wxTRANSLATE("Norwegian Krone")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('108', '%s', '', '', '.', ' ', '1000', 'OMR', 'Fiat', '0')", wxTRANSLATE("Rial Omani")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('109', '%s', 'Rs.', '', '.', ' ', '100', 'PKR', 'Fiat', '0')", wxTRANSLATE("Pakistan Rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('110', '%s', 'B./', '', '.', ' ', '100', 'PAB', 'Fiat', '0')", wxTRANSLATE("Balboa")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('111', '%s', 'K', '', '.', ' ', '100', 'PGK', 'Fiat', '0')", wxTRANSLATE("Kina")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('112', '%s', '', '', '.', ' ', '1', 'PYG', 'Fiat', '0')", wxTRANSLATE("Guarani")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('113', '%s', 'S/.', '', '.', ' ', '100', 'PEN', 'Fiat', '0')", wxTRANSLATE("Sol")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('114', '%s', '%s', '', '.', ' ', '100', 'PHP', 'Fiat', '0')", wxTRANSLATE("Philippine Piso"), L"₱"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('115', '%s', '', '%s', ',', '.', '100', 'PLN', 'Fiat', '0')", wxTRANSLATE("Zloty"), L"zł"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('116', '%s', 'QR', '', '.', ' ', '100', 'QAR', 'Fiat', '0')", wxTRANSLATE("Qatari Rial")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('117', '%s', 'L', '', '.', ' ', '100', 'RON', 'Fiat', '0')", wxTRANSLATE("Romanian Leu")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('118', '%s', 'RF', '', '.', ' ', '1', 'RWF', 'Fiat', '0')", wxTRANSLATE("Rwanda Franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('119', '%s', 'Db', '', '.', ' ', '0', 'STD', 'Fiat', '1')", wxTRANSLATE("Dobra (before 2017-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('120', '%s', 'SR', '', '.', ' ', '100', 'SAR', 'Fiat', '0')", wxTRANSLATE("Saudi Riyal")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('121', '%s', 'din.', '', '.', ' ', '100', 'RSD', 'Fiat', '0')", wxTRANSLATE("Serbian Dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('122', '%s', 'SR', '', '.', ' ', '100', 'SCR', 'Fiat', '0')", wxTRANSLATE("Seychelles Rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('123', '%s', 'Le', '', '.', ' ', '100', 'SLL', 'Fiat', '0')", wxTRANSLATE("Leone")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('124', '%s', 'S$', '', '.', ' ', '100', 'SGD', 'Fiat', '0')", wxTRANSLATE("Singapore Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('125', '%s', 'SI$', '', '.', ' ', '100', 'SBD', 'Fiat', '0')", wxTRANSLATE("Solomon Islands Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('126', '%s', 'Sh.', '', '.', ' ', '100', 'SOS', 'Fiat', '0')", wxTRANSLATE("Somali Shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('127', '%s', 'R', '', '.', ' ', '100', 'ZAR', 'Fiat', '0')", wxTRANSLATE("Rand")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('128', '%s', 'Rs', '', '.', ' ', '100', 'LKR', 'Fiat', '0')", wxTRANSLATE("Sri Lanka Rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('129', '%s', '%s', '', '.', ' ', '100', 'SHP', 'Fiat', '0')", wxTRANSLATE("Saint Helena Pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('130', '%s', '', '', '.', ' ', '100', 'SDG', 'Fiat', '0')", wxTRANSLATE("Sudanese Pound")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('131', '%s', '$', '', '.', ' ', '100', 'SRD', 'Fiat', '0')", wxTRANSLATE("Surinam Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('132', '%s', 'E', '', '.', ' ', '100', 'SZL', 'Fiat', '0')", wxTRANSLATE("Lilangeni")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('133', '%s', 'kr', '', '.', ' ', '100', 'SEK', 'Fiat', '0')", wxTRANSLATE("Swedish Krona")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('134', '%s', 'Fr.', '', '.', ' ', '100', 'CHF', 'Fiat', '0')", wxTRANSLATE("Swiss Franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('135', '%s', '', '', '.', ' ', '100', 'SYP', 'Fiat', '0')", wxTRANSLATE("Syrian Pound")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('136', '%s', 'NT$', '', '.', ' ', '100', 'TWD', 'Fiat', '0')", wxTRANSLATE("New Taiwan Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('137', '%s', '', '', '.', ' ', '100', 'TJS', 'Fiat', '0')", wxTRANSLATE("Somoni")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('138', '%s', '', '', '.', ' ', '100', 'TZS', 'Fiat', '0')", wxTRANSLATE("Tanzanian Shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('139', '%s', '%s', '', '.', ' ', '100', 'THB', 'Fiat', '0')", wxTRANSLATE("Baht"), L"฿"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('140', '%s', 'TT$', '', '.', ' ', '100', 'TTD', 'Fiat', '0')", wxTRANSLATE("Trinidad and Tobago Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('141', '%s', 'DT', '', '.', ' ', '1000', 'TND', 'Fiat', '0')", wxTRANSLATE("Tunisian Dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('142', '%s', '%s', '', '.', ' ', '100', 'TRY', 'Fiat', '0')", wxTRANSLATE("Turkish Lira"), L"₺"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('143', '%s', 'm', '', '.', ' ', '100', 'TMT', 'Fiat', '0')", wxTRANSLATE("Turkmenistan New Manat")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('144', '%s', 'USh', '', '.', ' ', '1', 'UGX', 'Fiat', '0')", wxTRANSLATE("Uganda Shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('145', '%s', '', '', '.', ' ', '100', 'AED', 'Fiat', '0')", wxTRANSLATE("UAE Dirham")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('146', '%s', '$U', '', '.', ' ', '100', 'UYU', 'Fiat', '0')", wxTRANSLATE("Peso Uruguayo")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('147', '%s', '', '', '.', ' ', '100', 'UZS', 'Fiat', '0')", wxTRANSLATE("Uzbekistan Sum")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('148', '%s', 'VT', '', '.', ' ', '1', 'VUV', 'Fiat', '0')", wxTRANSLATE("Vatu")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('149', '%s', '%s', '', '.', ' ', '1', 'VND', 'Fiat', '0')", wxTRANSLATE("Dong"), L"₫"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('150', '%s', 'WS$', '', '.', ' ', '100', 'WST', 'Fiat', '0')", wxTRANSLATE("Tala")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('151', '%s', '', '', '.', ' ', '100', 'YER', 'Fiat', '0')", wxTRANSLATE("Yemeni Rial")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('152', '%s', 'Bs.', '', '.', ',', '100', 'VEF', 'Fiat', '0')", wxTRANSLATE("Bolívar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('153', '%s', '%s', '', '.', ',', '100000000', 'BTC', 'Crypto', '0')", wxTRANSLATE("Bitcoin"), L"Ƀ"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('154', '%s', '', '', '.', ',', '100', 'ADP', 'Fiat', '1')", wxTRANSLATE("Andorran Peseta (before 2003-07)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('155', '%s', '', '', '.', ',', '100', 'AFA', 'Fiat', '1')", wxTRANSLATE("Afghani (before 2003-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('156', '%s', '', '', '.', ',', '100', 'ALK', 'Fiat', '1')", wxTRANSLATE("Old Lek (before 1989-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('157', '%s', '', '', '.', ',', '100', 'AOK', 'Fiat', '1')", wxTRANSLATE("Kwanza (before 1991-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('158', '%s', '', '', '.', ',', '100', 'AON', 'Fiat', '1')", wxTRANSLATE("New Kwanza (before 2000-02)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('159', '%s', '', '', '.', ',', '100', 'AOR', 'Fiat', '1')", wxTRANSLATE("Kwanza Reajustado (before 2000-02)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('160', '%s', '', '', '.', ',', '100', 'ARA', 'Fiat', '1')", wxTRANSLATE("Austral (before 1992-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('161', '%s', '', '', '.', ',', '100', 'ARP', 'Fiat', '1')", wxTRANSLATE("Peso Argentino (before 1985-07)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('162', '%s', '', '', '.', ',', '100', 'ARY', 'Fiat', '1')", wxTRANSLATE("Peso (1989 to 1990)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('163', '%s', '', '', '.', ',', '100', 'ATS', 'Fiat', '1')", wxTRANSLATE("Schilling (before 2002-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('164', '%s', '', '', '.', ',', '100', 'AYM', 'Fiat', '1')", wxTRANSLATE("Azerbaijan Manat (before 2005-10)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('165', '%s', '', '', '.', ',', '100', 'AZM', 'Fiat', '1')", wxTRANSLATE("Azerbaijanian Manat (before 2005-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('166', '%s', '', '', '.', ',', '100', 'BAD', 'Fiat', '1')", wxTRANSLATE("Dinar (before 1998-07)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('167', '%s', '', '', '.', ',', '100', 'BEC', 'Fiat', '1')", wxTRANSLATE("Convertible Franc (before 1990-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('168', '%s', '', '', '.', ',', '100', 'BEF', 'Fiat', '1')", wxTRANSLATE("Belgian Franc (before 2002-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('169', '%s', '', '', '.', ',', '100', 'BEL', 'Fiat', '1')", wxTRANSLATE("Financial Franc (before 1990-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('170', '%s', '', '', '.', ',', '100', 'BGJ', 'Fiat', '1')", wxTRANSLATE("Lev A/52 (1989 to 1990)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('171', '%s', '', '', '.', ',', '100', 'BGK', 'Fiat', '1')", wxTRANSLATE("Lev A/62 (1989 to 1990)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('172', '%s', '', '', '.', ',', '100', 'BGL', 'Fiat', '1')", wxTRANSLATE("Lev (before 2003-11)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('173', '%s', '', '', '.', ',', '100', 'BOP', 'Fiat', '1')", wxTRANSLATE("Peso boliviano (before 1987-02)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('174', '%s', '', '', '.', ',', '100', 'BRB', 'Fiat', '1')", wxTRANSLATE("Cruzeiro (before 1986-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('175', '%s', '', '', '.', ',', '100', 'BRC', 'Fiat', '1')", wxTRANSLATE("Cruzado (before 1989-02)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('176', '%s', '', '', '.', ',', '100', 'BRE', 'Fiat', '1')", wxTRANSLATE("Cruzeiro (before 1993-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('177', '%s', '', '', '.', ',', '100', 'BRN', 'Fiat', '1')", wxTRANSLATE("New Cruzado (before 1990-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('178', '%s', '', '', '.', ',', '100', 'BRR', 'Fiat', '1')", wxTRANSLATE("Cruzeiro Real (before 1994-07)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('179', '%s', '', '', '.', ',', '100', 'BUK', 'Fiat', '1')", wxTRANSLATE("Kyat (before 1990-02)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('180', '%s', '', '', '.', ',', '100', 'BYB', 'Fiat', '1')", wxTRANSLATE("Belarusian Ruble (before 2001-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('181', '%s', '', '', '.', ',', '100', 'BYN', 'Fiat', '0')", wxTRANSLATE("Belarusian Ruble")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('182', '%s', '', '', '.', ',', '100', 'CHC', 'Fiat', '1')", wxTRANSLATE("WIR Franc (for electronic) (before 2004-11)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('183', '%s', '', '', '.', ',', '100', 'CSD', 'Fiat', '1')", wxTRANSLATE("Serbian Dinar (before 2006-10)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('184', '%s', '', '', '.', ',', '100', 'CSJ', 'Fiat', '1')", wxTRANSLATE("Krona A/53 (1989 to 1990)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('185', '%s', '', '', '.', ',', '100', 'CSK', 'Fiat', '1')", wxTRANSLATE("Koruna (before 1993-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('186', '%s', '', '', '.', ',', '100', 'CUC', 'Fiat', '0')", wxTRANSLATE("Peso Convertible")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('187', '%s', '', '', '.', ',', '100', 'CUP', 'Fiat', '0')", wxTRANSLATE("Cuban Peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('188', '%s', '', '', '.', ',', '100', 'CYP', 'Fiat', '1')", wxTRANSLATE("Cyprus Pound (before 2008-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('189', '%s', '', '', '.', ',', '100', 'DDM', 'Fiat', '1')", wxTRANSLATE("Mark der DDR (1990-07 to 1990-09)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('190', '%s', '', '', '.', ',', '100', 'DEM', 'Fiat', '1')", wxTRANSLATE("Deutsche Mark (before 2002-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('191', '%s', '', '', '.', ',', '100', 'ECS', 'Fiat', '1')", wxTRANSLATE("Sucre (before 2000-09)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('192', '%s', '', '', '.', ',', '100', 'ECV', 'Fiat', '1')", wxTRANSLATE("Unidad de Valor Constante (UVC) (before 2000-09)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('193', '%s', '', '', '.', ',', '100', 'EEK', 'Fiat', '1')", wxTRANSLATE("Kroon (before 2011-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('194', '%s', '', '', '.', ',', '100', 'ESA', 'Fiat', '1')", wxTRANSLATE("Spanish Peseta (1978 to 1981)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('195', '%s', '', '', '.', ',', '100', 'ESB', 'Fiat', '1')", wxTRANSLATE("A Account (convertible Peseta Account) (before 1994-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('196', '%s', '', '', '.', ',', '100', 'ESP', 'Fiat', '1')", wxTRANSLATE("Spanish Peseta (before 2002-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('197', '%s', '', '', '.', ',', '100', 'FIM', 'Fiat', '1')", wxTRANSLATE("Markka (before 2002-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('198', '%s', '', '', '.', ',', '100', 'FRF', 'Fiat', '1')", wxTRANSLATE("French Franc (before 1999-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('199', '%s', '', '', '.', ',', '100', 'GEK', 'Fiat', '1')", wxTRANSLATE("Georgian Coupon (before 1995-10)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('200', '%s', '', '', '.', ',', '100', 'GHC', 'Fiat', '1')", wxTRANSLATE("Cedi (before 2008-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('201', '%s', '', '', '.', ',', '100', 'GHP', 'Fiat', '1')", wxTRANSLATE("Ghana Cedi (before 2007-06)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('202', '%s', '', '', '.', ',', '100', 'GNE', 'Fiat', '1')", wxTRANSLATE("Syli (before 1989-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('203', '%s', '', '', '.', ',', '100', 'GNS', 'Fiat', '1')", wxTRANSLATE("Syli (before 1986-02)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('204', '%s', '', '', '.', ',', '100', 'GQE', 'Fiat', '1')", wxTRANSLATE("Ekwele (before 1986-06)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('205', '%s', '', '', '.', ',', '100', 'GRD', 'Fiat', '1')", wxTRANSLATE("Drachma (before 2002-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('206', '%s', '', '', '.', ',', '100', 'GWE', 'Fiat', '1')", wxTRANSLATE("Guinea Escudo (1978 to 1981)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('207', '%s', '', '', '.', ',', '100', 'GWP', 'Fiat', '1')", wxTRANSLATE("Guinea-Bissau Peso (before 1997-05)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('208', '%s', '', '', '.', ',', '100', 'HRD', 'Fiat', '1')", wxTRANSLATE("Croatian Dinar (before 1995-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('209', '%s', '', '', '.', ',', '100', 'IEP', 'Fiat', '1')", wxTRANSLATE("Irish Pound (before 2002-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('210', '%s', '', '', '.', ',', '100', 'ILP', 'Fiat', '1')", wxTRANSLATE("Pound (1978 to 1981)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('211', '%s', '', '', '.', ',', '100', 'ILR', 'Fiat', '1')", wxTRANSLATE("Old Shekel (1989 to 1990)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('212', '%s', '', '', '.', ',', '100', 'ISJ', 'Fiat', '1')", wxTRANSLATE("Old Krona (1989 to 1990)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('213', '%s', '', '', '.', ',', '100', 'ITL', 'Fiat', '1')", wxTRANSLATE("Italian Lira (before 2002-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('214', '%s', '', '', '.', ',', '100', 'LAJ', 'Fiat', '1')", wxTRANSLATE("Pathet Lao Kip (before 1979-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('215', '%s', '', '', '.', ',', '100', 'LSM', 'Fiat', '1')", wxTRANSLATE("Loti (before 1985-05)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('216', '%s', '', '', '.', ',', '100', 'LTT', 'Fiat', '1')", wxTRANSLATE("Talonas (before 1993-07)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('217', '%s', '', '', '.', ',', '100', 'LUC', 'Fiat', '1')", wxTRANSLATE("Luxembourg Convertible Franc (before 1990-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('218', '%s', '', '', '.', ',', '100', 'LUF', 'Fiat', '1')", wxTRANSLATE("Luxembourg Franc (before 2002-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('219', '%s', '', '', '.', ',', '100', 'LUL', 'Fiat', '1')", wxTRANSLATE("Luxembourg Financial Franc (before 1990-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('220', '%s', '', '', '.', ',', '100', 'LVR', 'Fiat', '1')", wxTRANSLATE("Latvian Ruble (before 1994-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('221', '%s', '', '', '.', ',', '100', 'MGF', 'Fiat', '1')", wxTRANSLATE("Malagasy Franc (before 2004-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('222', '%s', '', '', '.', ',', '100', 'MLF', 'Fiat', '1')", wxTRANSLATE("Mali Franc (before 1984-11)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('223', '%s', '', '', '.', ',', '100', 'MRU', 'Fiat', '0')", wxTRANSLATE("Ouguiya")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('224', '%s', '', '', '.', ',', '100', 'MTL', 'Fiat', '1')", wxTRANSLATE("Maltese Lira (before 2008-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('225', '%s', '', '', '.', ',', '100', 'MTP', 'Fiat', '1')", wxTRANSLATE("Maltese Pound (before 1983-06)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('226', '%s', '', '', '.', ',', '100', 'MVQ', 'Fiat', '1')", wxTRANSLATE("Maldive Rupee (before 1989-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('227', '%s', '', '', '.', ',', '100', 'MXP', 'Fiat', '1')", wxTRANSLATE("Mexican Peso (before 1993-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('228', '%s', '', '', '.', ',', '100', 'MZE', 'Fiat', '1')", wxTRANSLATE("Mozambique Escudo (1978 to 1981)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('229', '%s', '', '', '.', ',', '100', 'MZM', 'Fiat', '1')", wxTRANSLATE("Mozambique Metical (before 2006-06)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('230', '%s', '', '', '.', ',', '100', 'MZN', 'Fiat', '0')", wxTRANSLATE("Mozambique Metical")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('231', '%s', '', '', '.', ',', '100', 'NIC', 'Fiat', '1')", wxTRANSLATE("Cordoba (before 1990-10)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('232', '%s', '', '', '.', ',', '100', 'NLG', 'Fiat', '1')", wxTRANSLATE("Netherlands Guilder (before 2002-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('233', '%s', '', '', '.', ',', '100', 'PEH', 'Fiat', '1')", wxTRANSLATE("Sol (1989 to 1990)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('234', '%s', '', '', '.', ',', '100', 'PEI', 'Fiat', '1')", wxTRANSLATE("Inti (before 1991-07)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('235', '%s', '', '', '.', ',', '100', 'PES', 'Fiat', '1')", wxTRANSLATE("Sol (before 1986-02)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('236', '%s', '', '', '.', ',', '100', 'PLZ', 'Fiat', '1')", wxTRANSLATE("Zloty (before 1997-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('237', '%s', '', '', '.', ',', '100', 'PTE', 'Fiat', '1')", wxTRANSLATE("Portuguese Escudo (before 2002-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('238', '%s', '', '', '.', ',', '100', 'RHD', 'Fiat', '1')", wxTRANSLATE("Rhodesian Dollar (1978 to 1981)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('239', '%s', '', '', '.', ',', '100', 'ROK', 'Fiat', '1')", wxTRANSLATE("Leu A/52 (1989 to 1990)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('240', '%s', '', '', '.', ',', '100', 'ROL', 'Fiat', '1')", wxTRANSLATE("Old Leu (before 2005-06)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('241', '%s', '', '', '.', ',', '100', 'RUR', 'Fiat', '1')", wxTRANSLATE("Russian Ruble (before 1994-07)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('242', '%s', '', '', '.', ',', '100', 'SDD', 'Fiat', '1')", wxTRANSLATE("Sudanese Dinar (before 2007-07)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('243', '%s', '', '', '.', ',', '100', 'SDP', 'Fiat', '1')", wxTRANSLATE("Sudanese Pound (before 1998-06)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('244', '%s', '', '', '.', ',', '100', 'SIT', 'Fiat', '1')", wxTRANSLATE("Tolar (before 2007-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('245', '%s', '', '', '.', ',', '100', 'SKK', 'Fiat', '1')", wxTRANSLATE("Slovak Koruna (before 2009-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('246', '%s', '', '', '.', ',', '100', 'SRG', 'Fiat', '1')", wxTRANSLATE("Surinam Guilder (before 2003-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('247', '%s', '', '', '.', ',', '100', 'SSP', 'Fiat', '0')", wxTRANSLATE("South Sudanese Pound")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('248', '%s', '', '', '.', ',', '100', 'STN', 'Fiat', '0')", wxTRANSLATE("Dobra")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('249', '%s', '', '', '.', ',', '100', 'SUR', 'Fiat', '1')", wxTRANSLATE("Rouble (before 1990-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('250', '%s', '', '', '.', ',', '100', 'SVC', 'Fiat', '0')", wxTRANSLATE("El Salvador Colon")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('251', '%s', '', '', '.', ',', '100', 'TJR', 'Fiat', '1')", wxTRANSLATE("Tajik Ruble (before 2001-04)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('252', '%s', '', '', '.', ',', '100', 'TMM', 'Fiat', '1')", wxTRANSLATE("Turkmenistan Manat (before 2009-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('253', '%s', '', '', '.', ',', '100', 'TOP', 'Fiat', '0')", wxTRANSLATE("Pa’anga")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('254', '%s', '', '', '.', ',', '100', 'TPE', 'Fiat', '1')", wxTRANSLATE("Timor Escudo (before 2002-11)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('255', '%s', '', '', '.', ',', '100', 'TRL', 'Fiat', '1')", wxTRANSLATE("Old Turkish Lira (before 2005-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('256', '%s', '', '', '.', ',', '100', 'UAK', 'Fiat', '1')", wxTRANSLATE("Karbovanet (before 1996-09)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('257', '%s', '', '', '.', ',', '100', 'UGS', 'Fiat', '1')", wxTRANSLATE("Uganda Shilling (before 1987-05)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('258', '%s', '', '', '.', ',', '100', 'UGW', 'Fiat', '1')", wxTRANSLATE("Old Shilling (1989 to 1990)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('259', '%s', '', '', '.', ',', '100', 'USS', 'Fiat', '1')", wxTRANSLATE("US Dollar (Same day) (before 2014-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('260', '%s', '', '', '.', ',', '100', 'UYN', 'Fiat', '1')", wxTRANSLATE("Old Uruguay Peso (before 1989-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('261', '%s', '', '', '.', ',', '100', 'UYP', 'Fiat', '1')", wxTRANSLATE("Uruguayan Peso (before 1993-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('262', '%s', '', '', '.', ',', '100', 'VEB', 'Fiat', '1')", wxTRANSLATE("Bolivar (before 2008-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('263', '%s', '', '', '.', ',', '100', 'VNC', 'Fiat', '1')", wxTRANSLATE("Old Dong (1989-1990)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('264', '%s', '', '', '.', ',', '100', 'XEU', 'Fiat', '1')", wxTRANSLATE("European Currency Unit (E.C.U) (before 1999-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('265', '%s', '', '', '.', ',', '100', 'XFO', 'Fiat', '1')", wxTRANSLATE("Gold-Franc (before 2006-10)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('266', '%s', '', '', '.', ',', '100', 'YDD', 'Fiat', '1')", wxTRANSLATE("Yemeni Dinar (before 1991-09)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('267', '%s', '', '', '.', ',', '100', 'YUD', 'Fiat', '1')", wxTRANSLATE("New Yugoslavian Dinar (before 1990-01)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('268', '%s', '', '', '.', ',', '100', 'YUM', 'Fiat', '1')", wxTRANSLATE("New Dinar (before 2003-07)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('269', '%s', '', '', '.', ',', '100', 'YUN', 'Fiat', '1')", wxTRANSLATE("Yugoslavian Dinar (before 1995-11)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('270', '%s', '', '', '.', ',', '100', 'ZAL', 'Fiat', '1')", wxTRANSLATE("Financial Rand (before 1995-03)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('271', '%s', '', '', '.', ',', '100', 'ZMK', 'Fiat', '1')", wxTRANSLATE("Zambian Kwacha (before 2012-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('272', '%s', '', '', '.', ',', '100', 'ZMW', 'Fiat', '0')", wxTRANSLATE("Zambian Kwacha")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('273', '%s', '', '', '.', ',', '100', 'ZRN', 'Fiat', '1')", wxTRANSLATE("New Zaire (before 1999-06)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('274', '%s', '', '', '.', ',', '100', 'ZRZ', 'Fiat', '1')", wxTRANSLATE("Zaire (before 1994-02)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('275', '%s', '', '', '.', ',', '100', 'ZWC', 'Fiat', '1')", wxTRANSLATE("Rhodesian Dollar (before 1989-12)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('276', '%s', '', '', '.', ',', '100', 'ZWD', 'Fiat', '1')", wxTRANSLATE("Zimbabwe Dollar (before 2008-08)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('277', '%s', '', '', '.', ',', '100', 'ZWL', 'Fiat', '0')", wxTRANSLATE("Zimbabwe Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('278', '%s', '', '', '.', ',', '100', 'ZWN', 'Fiat', '1')", wxTRANSLATE("Zimbabwe Dollar (new) (before 2006-09)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS VALUES ('279', '%s', '', '', '.', ',', '100', 'ZWR', 'Fiat', '1')", wxTRANSLATE("Zimbabwe Dollar (before 2009-06)")));
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
    
    struct SCALE : public DB_Column<int>
    { 
        static wxString name() { return "SCALE"; } 
        explicit SCALE(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
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
    
    struct HISTORIC : public DB_Column<int>
    { 
        static wxString name() { return "HISTORIC"; } 
        explicit HISTORIC(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
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
        , COL_SCALE = 6
        , COL_CURRENCY_SYMBOL = 7
        , COL_CURRENCY_TYPE = 8
        , COL_HISTORIC = 9
        , COL_UNKNOWN = -1
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
            case COL_SCALE: return "SCALE";
            case COL_CURRENCY_SYMBOL: return "CURRENCY_SYMBOL";
            case COL_CURRENCY_TYPE: return "CURRENCY_TYPE";
            case COL_HISTORIC: return "HISTORIC";
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
        else if ("SCALE" == name) return COL_SCALE;
        else if ("CURRENCY_SYMBOL" == name) return COL_CURRENCY_SYMBOL;
        else if ("CURRENCY_TYPE" == name) return COL_CURRENCY_TYPE;
        else if ("HISTORIC" == name) return COL_HISTORIC;

        return COL_UNKNOWN;
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_CURRENCYFORMATS;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int CURRENCYID; // primary key
        wxString CURRENCYNAME;
        wxString PFX_SYMBOL;
        wxString SFX_SYMBOL;
        wxString DECIMAL_POINT;
        wxString GROUP_SEPARATOR;
        int SCALE;
        wxString CURRENCY_SYMBOL;
        wxString CURRENCY_TYPE;
        int HISTORIC;

        int id() const
        {
            return CURRENCYID;
        }

        void id(int id)
        {
            CURRENCYID = id;
        }

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
            HISTORIC = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = 0)
        {
            table_ = table;
        
            CURRENCYID = q.GetInt(0);
            CURRENCYNAME = q.GetString(1);
            PFX_SYMBOL = q.GetString(2);
            SFX_SYMBOL = q.GetString(3);
            DECIMAL_POINT = q.GetString(4);
            GROUP_SEPARATOR = q.GetString(5);
            SCALE = q.GetInt(6);
            CURRENCY_SYMBOL = q.GetString(7);
            CURRENCY_TYPE = q.GetString(8);
            HISTORIC = q.GetInt(9);
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
            SCALE = other.SCALE;
            CURRENCY_SYMBOL = other.CURRENCY_SYMBOL;
            CURRENCY_TYPE = other.CURRENCY_TYPE;
            HISTORIC = other.HISTORIC;
            return *this;
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

        bool match(const Self::SCALE &in) const
        {
            return this->SCALE == in.v_;
        }

        bool match(const Self::CURRENCY_SYMBOL &in) const
        {
            return this->CURRENCY_SYMBOL.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::CURRENCY_TYPE &in) const
        {
            return this->CURRENCY_TYPE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::HISTORIC &in) const
        {
            return this->HISTORIC == in.v_;
        }

        /** Return the data record as a json string */
        wxString to_json() const
        {
            StringBuffer json_buffer;
            PrettyWriter<StringBuffer> json_writer(json_buffer);

            json_writer.StartObject();
            this->as_json(json_writer);
            json_writer.EndObject();

            return json_buffer.GetString();
        }

        /** Add the field data as json key:value pairs */
        void as_json(PrettyWriter<StringBuffer>& json_writer) const
        {
            json_writer.Key("CURRENCYID");
            json_writer.Int(this->CURRENCYID);
            json_writer.Key("CURRENCYNAME");
            json_writer.String(this->CURRENCYNAME.c_str());
            json_writer.Key("PFX_SYMBOL");
            json_writer.String(this->PFX_SYMBOL.c_str());
            json_writer.Key("SFX_SYMBOL");
            json_writer.String(this->SFX_SYMBOL.c_str());
            json_writer.Key("DECIMAL_POINT");
            json_writer.String(this->DECIMAL_POINT.c_str());
            json_writer.Key("GROUP_SEPARATOR");
            json_writer.String(this->GROUP_SEPARATOR.c_str());
            json_writer.Key("SCALE");
            json_writer.Int(this->SCALE);
            json_writer.Key("CURRENCY_SYMBOL");
            json_writer.String(this->CURRENCY_SYMBOL.c_str());
            json_writer.Key("CURRENCY_TYPE");
            json_writer.String(this->CURRENCY_TYPE.c_str());
            json_writer.Key("HISTORIC");
            json_writer.Int(this->HISTORIC);
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
            row(L"SCALE") = SCALE;
            row(L"CURRENCY_SYMBOL") = CURRENCY_SYMBOL;
            row(L"CURRENCY_TYPE") = CURRENCY_TYPE;
            row(L"HISTORIC") = HISTORIC;
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
            t(L"SCALE") = SCALE;
            t(L"CURRENCY_SYMBOL") = CURRENCY_SYMBOL;
            t(L"CURRENCY_TYPE") = CURRENCY_TYPE;
            t(L"HISTORIC") = HISTORIC;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save CURRENCYFORMATS");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove CURRENCYFORMATS");
                return false;
            }
            
            return table_->remove(this, db);
        }

        void destroy()
        {
            delete this;
        }
    };

    enum
    {
        NUM_COLUMNS = 10
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table */
    wxString name() const { return "CURRENCYFORMATS"; }

    DB_Table_CURRENCYFORMATS() : fake_(new Data())
    {
        query_ = "SELECT CURRENCYID, CURRENCYNAME, PFX_SYMBOL, SFX_SYMBOL, DECIMAL_POINT, GROUP_SEPARATOR, SCALE, CURRENCY_SYMBOL, CURRENCY_TYPE, HISTORIC FROM CURRENCYFORMATS ";
    }

    /** Create a new Data record and add to memory table (cache) */
    Self::Data* create()
    {
        Self::Data* entity = new Self::Data(this);
        cache_.push_back(entity);
        return entity;
    }
    
    /** Create a copy of the Data record and add to memory table (cache) */
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
            sql = "INSERT INTO CURRENCYFORMATS(CURRENCYNAME, PFX_SYMBOL, SFX_SYMBOL, DECIMAL_POINT, GROUP_SEPARATOR, SCALE, CURRENCY_SYMBOL, CURRENCY_TYPE, HISTORIC) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE CURRENCYFORMATS SET CURRENCYNAME = ?, PFX_SYMBOL = ?, SFX_SYMBOL = ?, DECIMAL_POINT = ?, GROUP_SEPARATOR = ?, SCALE = ?, CURRENCY_SYMBOL = ?, CURRENCY_TYPE = ?, HISTORIC = ? WHERE CURRENCYID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->CURRENCYNAME);
            stmt.Bind(2, entity->PFX_SYMBOL);
            stmt.Bind(3, entity->SFX_SYMBOL);
            stmt.Bind(4, entity->DECIMAL_POINT);
            stmt.Bind(5, entity->GROUP_SEPARATOR);
            stmt.Bind(6, entity->SCALE);
            stmt.Bind(7, entity->CURRENCY_SYMBOL);
            stmt.Bind(8, entity->CURRENCY_TYPE);
            stmt.Bind(9, entity->HISTORIC);
            if (entity->id() > 0)
                stmt.Bind(10, entity->CURRENCYID);

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
            wxLogError("CURRENCYFORMATS: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
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
            wxString sql = "DELETE FROM CURRENCYFORMATS WHERE CURRENCYID = ?";
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
            wxLogError("CURRENCYFORMATS: Exception %s", e.GetMessage().c_str());
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

