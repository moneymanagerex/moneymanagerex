﻿// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013 - 2018 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017 - 2018 Stefano Giorgio (stef145g)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2018-04-03 22:36:58.027000.
 *          DO NOT EDIT!
 */
//=============================================================================
#pragma execution_character_set("UTF-8")
#pragma once

#include "DB_Table.h"

struct DB_Table_CURRENCYFORMATS_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_CURRENCYFORMATS_V1 Self;

    /** A container to hold list of Data records for the table*/
    struct Data_Set : public std::vector<Self::Data>
    {
        /**Return the data records as a json array string */
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
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('1', '%s', '$', '', '.', ' ', '', '', '100', '1', 'USD')", wxTRANSLATE("US Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('2', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'EUR')", wxTRANSLATE("Euro"), L"€"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('3', '%s', '%s', '', '.', ' ', 'Pound', 'Pence', '100', '1', 'GBP')", wxTRANSLATE("Pound Sterling"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('4', '%s', '', '%s', ',', ' ', '%s', '%s', '100', '1', 'RUB')", wxTRANSLATE("Russian Ruble"), L"р", L"руб.", L"коп."));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('5', '%s', '%s', '', ',', ' ', '', '', '100', '1', 'UAH')", wxTRANSLATE("Hryvnia"), L"₴"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('6', '%s', '%s', '', '.', ' ', '', 'pul', '100', '1', 'AFN')", wxTRANSLATE("Afghani"), L"؋"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('7', '%s', '', 'L', '.', ' ', '', '', '100', '1', 'ALL')", wxTRANSLATE("Lek")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('8', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'DZD')", wxTRANSLATE("Algerian Dinar"), L"دج"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('9', '%s', '', 'Kz', '.', ' ', '', '%s', '100', '1', 'AOA')", wxTRANSLATE("Kwanza"), L"Céntimo"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('10', '%s', 'EC$', '', '.', ' ', '', '', '100', '1', 'XCD')", wxTRANSLATE("East Caribbean Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('11', '%s', 'AR$', '', ',', '.', '', 'centavo', '100', '1', 'ARS')", wxTRANSLATE("Argentine Peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('12', '%s', '', '', '.', ' ', '', '', '100', '1', 'AMD')", wxTRANSLATE("Armenian Dram")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('13', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'AWG')", wxTRANSLATE("Aruban Florin"), L"ƒ"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('14', '%s', '$', '', '.', ',', '', '', '100', '1', 'AUD')", wxTRANSLATE("Australian Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('15', '%s', '', '', '.', ' ', '', '', '100', '1', 'AZN')", wxTRANSLATE("Azerbaijan Manat")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('16', '%s', 'B$', '', '.', ' ', '', '', '100', '1', 'BSD')", wxTRANSLATE("Bahamian Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('17', '%s', '', '', '.', ' ', '', '', '1000', '1', 'BHD')", wxTRANSLATE("Bahraini Dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('18', '%s', '', '', '.', ' ', '', '', '100', '1', 'BDT')", wxTRANSLATE("Taka")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('19', '%s', 'Bds$', '', '.', ' ', '', '', '100', '1', 'BBD')", wxTRANSLATE("Barbados Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('20', '%s', 'Br', '', ',', ' ', '', '', '1', '1', 'BYR')", wxTRANSLATE("Belarusian ruble")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('21', '%s', 'BZ$', '', '.', ' ', '', '', '100', '1', 'BZD')", wxTRANSLATE("Belize Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('22', '%s', 'CFA', '', '.', ' ', '', '', '1', '1', 'XOF')", wxTRANSLATE("CFA Franc BCEAO")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('23', '%s', 'BD$', '', '.', ' ', '', '', '100', '1', 'BMD')", wxTRANSLATE("Bermudian Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('24', '%s', 'Nu.', '', '.', ' ', '', '', '100', '1', 'BTN')", wxTRANSLATE("Ngultrum")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('25', '%s', 'Bs.', '', '.', ' ', '', '', '100', '1', 'BOB')", wxTRANSLATE("Boliviano")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('26', '%s', 'KM', '', ',', '.', '', '', '100', '1', 'BAM')", wxTRANSLATE("Convertible Mark")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('27', '%s', 'P', '', '.', ' ', '', '', '100', '1', 'BWP')", wxTRANSLATE("Pula")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('28', '%s', 'R$', '', '.', ' ', '', '', '100', '1', 'BRL')", wxTRANSLATE("Brazilian Real")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('29', '%s', 'B$', '', '.', ' ', '', '', '100', '1', 'BND')", wxTRANSLATE("Brunei Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('30', '%s', '', '', '.', ' ', '', '', '100', '1', 'BGN')", wxTRANSLATE("Bulgarian Lev")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('31', '%s', 'FBu', '', '.', ' ', '', '', '1', '1', 'BIF')", wxTRANSLATE("Burundi Franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('32', '%s', '', '', '.', ' ', '', '', '100', '1', 'KHR')", wxTRANSLATE("Riel")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('33', '%s', 'CFA', '', '.', ' ', '', '', '1', '1', 'XAF')", wxTRANSLATE("CFA Franc BEAC")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('34', '%s', '$', '', '.', ' ', '', '', '100', '1', 'CAD')", wxTRANSLATE("Canadian Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('35', '%s', 'Esc', '', '.', ' ', '', '', '100', '1', 'CVE')", wxTRANSLATE("Cabo Verde Escudo")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('36', '%s', 'KY$', '', '.', ' ', '', '', '100', '1', 'KYD')", wxTRANSLATE("Cayman Islands Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('37', '%s', '$', '', '.', ' ', '', '', '1', '1', 'CLP')", wxTRANSLATE("Chilean Peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('38', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'CNY')", wxTRANSLATE("Yuan Renminbi"), L"¥"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('39', '%s', 'Col$', '', '.', ' ', '', '', '100', '1', 'COP')", wxTRANSLATE("Colombian Peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('40', '%s', '', '', '.', ' ', '', '', '1', '1', 'KMF')", wxTRANSLATE("Comorian Franc ")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('41', '%s', 'F', '', '.', ' ', '', '', '100', '1', 'CDF')", wxTRANSLATE("Congolese Franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('42', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'CRC')", wxTRANSLATE("Costa Rican Colon"), L"₡"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('43', '%s', 'kn', '', '.', ' ', '', '', '100', '1', 'HRK')", wxTRANSLATE("Kuna")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('44', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'CZK')", wxTRANSLATE("Czech Koruna"), L"Kč"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('45', '%s', 'Kr', '', '.', ' ', '', '', '100', '1', 'DKK')", wxTRANSLATE("Danish Krone")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('46', '%s', 'Fdj', '', '.', ' ', '', '', '1', '1', 'DJF')", wxTRANSLATE("Djibouti Franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('47', '%s', 'RD$', '', '.', ' ', '', '', '100', '1', 'DOP')", wxTRANSLATE("Dominican Peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('48', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'EGP')", wxTRANSLATE("Egyptian Pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('49', '%s', 'Nfa', '', '.', ' ', '', '', '100', '1', 'ERN')", wxTRANSLATE("Nakfa")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('50', '%s', 'Br', '', '.', ' ', '', '', '100', '1', 'ETB')", wxTRANSLATE("Ethiopian Birr")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('51', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'FKP')", wxTRANSLATE("Falkland Islands Pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('52', '%s', 'FJ$', '', '.', ' ', '', '', '100', '1', 'FJD')", wxTRANSLATE("Fiji Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('53', '%s', 'F', '', '.', ' ', '', '', '1', '1', 'XPF')", wxTRANSLATE("CFP Franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('54', '%s', 'D', '', '.', ' ', '', '', '100', '1', 'GMD')", wxTRANSLATE("Dalasi")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('55', '%s', '', '', '.', ' ', '', '', '100', '1', 'GEL')", wxTRANSLATE("Lari")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('56', '%s', '', '', '.', ' ', '', '', '100', '1', 'GHS')", wxTRANSLATE("Ghana Cedi")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('57', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'GIP')", wxTRANSLATE("Gibraltar Pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('58', '%s', 'Q', '', '.', ' ', '', '', '100', '1', 'GTQ')", wxTRANSLATE("Quetzal")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('59', '%s', 'FG', '', '.', ' ', '', '', '1', '1', 'GNF')", wxTRANSLATE("Guinean Franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('60', '%s', 'GY$', '', '.', ' ', '', '', '100', '1', 'GYD')", wxTRANSLATE("Guyana Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('61', '%s', 'G', '', '.', ' ', '', '', '100', '1', 'HTG')", wxTRANSLATE("Gourde")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('62', '%s', 'L', '', '.', ' ', '', '', '100', '1', 'HNL')", wxTRANSLATE("Lempira")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('63', '%s', 'HK$', '', '.', ' ', '', '', '100', '1', 'HKD')", wxTRANSLATE("Hong Kong Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('64', '%s', 'Ft', '', '.', ' ', '', '', '100', '1', 'HUF')", wxTRANSLATE("Forint")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('65', '%s', 'kr', '', '.', ' ', '', '', '1', '1', 'ISK')", wxTRANSLATE("Iceland Krona")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('66', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'INR')", wxTRANSLATE("Indian Rupee"), L"₹"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('67', '%s', 'Rp', '', '.', ' ', '', '', '100', '1', 'IDR')", wxTRANSLATE("Rupiah")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('68', '%s', 'SDR', '', '.', ' ', '', '', '100', '1', 'XDR')", wxTRANSLATE("Special Drawing Rights")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('69', '%s', '', '', '.', ' ', '', '', '100', '1', 'IRR')", wxTRANSLATE("Iranian Rial")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('70', '%s', '', '', '.', ' ', '', '', '1000', '1', 'IQD')", wxTRANSLATE("Iraqi Dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('71', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'ILS')", wxTRANSLATE("New Israeli Sheqel"), L"₪"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('72', '%s', 'J$', '', '.', ' ', '', '', '100', '1', 'JMD')", wxTRANSLATE("Jamaican Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('73', '%s', '%s', '', '.', ' ', '', '', '1', '1', 'JPY')", wxTRANSLATE("Yen"), L"¥"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('74', '%s', '', '', '.', ' ', '', '', '1000', '1', 'JOD')", wxTRANSLATE("Jordanian Dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('75', '%s', 'T', '', '.', ' ', '', '', '100', '1', 'KZT')", wxTRANSLATE("Tenge")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('76', '%s', 'KSh', '', '.', ' ', '', '', '100', '1', 'KES')", wxTRANSLATE("Kenyan Shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('77', '%s', 'W', '', '.', ' ', '', '', '100', '1', 'KPW')", wxTRANSLATE("North Korean Won")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('78', '%s', 'W', '', '.', ' ', '', '', '1', '1', 'KRW')", wxTRANSLATE("Won")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('79', '%s', '', '', '.', ' ', '', '', '1000', '1', 'KWD')", wxTRANSLATE("Kuwaiti Dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('80', '%s', '', '', '.', ' ', '', '', '100', '1', 'KGS')", wxTRANSLATE("Som")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('81', '%s', 'KN', '', '.', ' ', '', '', '100', '1', 'LAK')", wxTRANSLATE("Lao Kip")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('82', '%s', 'Ls', '', '.', ' ', '', '', '100', '1', 'LVL')", wxTRANSLATE("Latvian lats")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('83', '%s', '', '', '.', ' ', '', '', '100', '1', 'LBP')", wxTRANSLATE("Lebanese Pound")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('84', '%s', 'M', '', '.', ' ', '', '', '100', '1', 'LSL')", wxTRANSLATE("Loti")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('85', '%s', 'L$', '', '.', ' ', '', '', '100', '1', 'LRD')", wxTRANSLATE("Liberian Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('86', '%s', 'LD', '', '.', ' ', '', '', '1000', '1', 'LYD')", wxTRANSLATE("Libyan Dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('87', '%s', 'Lt', '', '.', ' ', '', '', '100', '1', 'LTL')", wxTRANSLATE("Lithuanian litas")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('88', '%s', 'P', '', '.', ' ', '', '', '100', '1', 'MOP')", wxTRANSLATE("Pataca")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('89', '%s', '', '', '.', ' ', '', '', '100', '1', 'MKD')", wxTRANSLATE("Denar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('90', '%s', 'FMG', '', '.', ' ', '', '', '100', '1', 'MGA')", wxTRANSLATE("Malagasy Ariary")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('91', '%s', 'MK', '', '.', ' ', '', '', '100', '1', 'MWK')", wxTRANSLATE("Malawi Kwacha")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('92', '%s', 'RM', '', '.', ' ', '', '', '100', '1', 'MYR')", wxTRANSLATE("Malaysian Ringgit")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('93', '%s', 'Rf', '', '.', ' ', '', '', '100', '1', 'MVR')", wxTRANSLATE("Rufiyaa")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('94', '%s', 'UM', '', '.', ' ', '', '', '100', '1', 'MRO')", wxTRANSLATE("Mauritanian ouguiya")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('95', '%s', 'Rs', '', '.', ' ', '', '', '100', '1', 'MUR')", wxTRANSLATE("Mauritius Rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('96', '%s', '$', '', '.', ' ', '', '', '100', '1', 'MXN')", wxTRANSLATE("Mexican Peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('97', '%s', '', '', '.', ' ', '', '', '100', '1', 'MDL')", wxTRANSLATE("Moldovan Leu")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('98', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'MNT')", wxTRANSLATE("Tugrik"), L"₮"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('99', '%s', '', '', '.', ' ', '', '', '100', '1', 'MAD')", wxTRANSLATE("Moroccan Dirham")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('100', '%s', 'K', '', '.', ' ', '', '', '100', '1', 'MMK')", wxTRANSLATE("Kyat")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('101', '%s', 'N$', '', '.', ' ', '', '', '100', '1', 'NAD')", wxTRANSLATE("Namibia Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('102', '%s', 'NRs', '', '.', ' ', '', '', '100', '1', 'NPR')", wxTRANSLATE("Nepalese Rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('103', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'ANG')", wxTRANSLATE("Netherlands Antillean Guilder"), L"NAƒ"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('104', '%s', 'NZ$', '', '.', ' ', '', '', '100', '1', 'NZD')", wxTRANSLATE("New Zealand Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('105', '%s', 'C$', '', '.', ' ', '', '', '100', '1', 'NIO')", wxTRANSLATE("Cordoba Oro")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('106', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'NGN')", wxTRANSLATE("Naira"), L"₦"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('107', '%s', 'kr', '', '.', ' ', '', '', '100', '1', 'NOK')", wxTRANSLATE("Norwegian Krone")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('108', '%s', '', '', '.', ' ', '', '', '1000', '1', 'OMR')", wxTRANSLATE("Rial Omani")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('109', '%s', 'Rs.', '', '.', ' ', '', '', '100', '1', 'PKR')", wxTRANSLATE("Pakistan Rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('110', '%s', 'B./', '', '.', ' ', '', '', '100', '1', 'PAB')", wxTRANSLATE("Balboa")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('111', '%s', 'K', '', '.', ' ', '', '', '100', '1', 'PGK')", wxTRANSLATE("Kina")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('112', '%s', '', '', '.', ' ', '', '', '1', '1', 'PYG')", wxTRANSLATE("Guarani")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('113', '%s', 'S/.', '', '.', ' ', '', '', '100', '1', 'PEN')", wxTRANSLATE("Sol")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('114', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'PHP')", wxTRANSLATE("Philippine Piso"), L"₱"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('115', '%s', '', '%s', ',', '.', '%s', 'grosz', '100', '1', 'PLN')", wxTRANSLATE("Zloty"), L"zł", L"złoty"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('116', '%s', 'QR', '', '.', ' ', '', '', '100', '1', 'QAR')", wxTRANSLATE("Qatari Rial")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('117', '%s', 'L', '', '.', ' ', '', '', '100', '1', 'RON')", wxTRANSLATE("Romanian Leu")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('118', '%s', 'RF', '', '.', ' ', '', '', '1', '1', 'RWF')", wxTRANSLATE("Rwanda Franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('119', '%s', 'Db', '', '.', ' ', '', '', '100', '1', 'STD')", L"São Tomé and Príncipe dobra"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('120', '%s', 'SR', '', '.', ' ', '', '', '100', '1', 'SAR')", wxTRANSLATE("Saudi Riyal")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('121', '%s', 'din.', '', '.', ' ', '', '', '100', '1', 'RSD')", wxTRANSLATE("Serbian Dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('122', '%s', 'SR', '', '.', ' ', '', '', '100', '1', 'SCR')", wxTRANSLATE("Seychelles Rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('123', '%s', 'Le', '', '.', ' ', '', '', '100', '1', 'SLL')", wxTRANSLATE("Leone")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('124', '%s', 'S$', '', '.', ' ', '', '', '100', '1', 'SGD')", wxTRANSLATE("Singapore Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('125', '%s', 'SI$', '', '.', ' ', '', '', '100', '1', 'SBD')", wxTRANSLATE("Solomon Islands Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('126', '%s', 'Sh.', '', '.', ' ', '', '', '100', '1', 'SOS')", wxTRANSLATE("Somali Shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('127', '%s', 'R', '', '.', ' ', '', '', '100', '1', 'ZAR')", wxTRANSLATE("Rand")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('128', '%s', 'Rs', '', '.', ' ', '', '', '100', '1', 'LKR')", wxTRANSLATE("Sri Lanka Rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('129', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'SHP')", wxTRANSLATE("Saint Helena Pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('130', '%s', '', '', '.', ' ', '', '', '100', '1', 'SDG')", wxTRANSLATE("Sudanese Pound")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('131', '%s', '$', '', '.', ' ', '', '', '100', '1', 'SRD')", wxTRANSLATE("Surinam Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('132', '%s', 'E', '', '.', ' ', '', '', '100', '1', 'SZL')", wxTRANSLATE("Lilangeni")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('133', '%s', 'kr', '', '.', ' ', '', '', '100', '1', 'SEK')", wxTRANSLATE("Swedish Krona")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('134', '%s', 'Fr.', '', '.', ' ', '', '', '100', '1', 'CHF')", wxTRANSLATE("Swiss Franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('135', '%s', '', '', '.', ' ', '', '', '100', '1', 'SYP')", wxTRANSLATE("Syrian Pound")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('136', '%s', 'NT$', '', '.', ' ', '', '', '100', '1', 'TWD')", wxTRANSLATE("New Taiwan Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('137', '%s', '', '', '.', ' ', '', '', '100', '1', 'TJS')", wxTRANSLATE("Somoni")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('138', '%s', '', '', '.', ' ', '', '', '100', '1', 'TZS')", wxTRANSLATE("Tanzanian Shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('139', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'THB')", wxTRANSLATE("Baht"), L"฿"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('140', '%s', 'TT$', '', '.', ' ', '', '', '100', '1', 'TTD')", wxTRANSLATE("Trinidad and Tobago Dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('141', '%s', 'DT', '', '.', ' ', '', '', '1000', '1', 'TND')", wxTRANSLATE("Tunisian Dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('142', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'TRY')", wxTRANSLATE("Turkish Lira"), L"₺"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('143', '%s', 'm', '', '.', ' ', '', '', '100', '1', 'TMT')", wxTRANSLATE("Turkmenistan New Manat")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('144', '%s', 'USh', '', '.', ' ', '', '', '1', '1', 'UGX')", wxTRANSLATE("Uganda Shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('145', '%s', '', '', '.', ' ', '', '', '100', '1', 'AED')", wxTRANSLATE("UAE Dirham")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('146', '%s', '$U', '', '.', ' ', '', '', '100', '1', 'UYU')", wxTRANSLATE("Peso Uruguayo")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('147', '%s', '', '', '.', ' ', '', '', '100', '1', 'UZS')", wxTRANSLATE("Uzbekistan Sum")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('148', '%s', 'VT', '', '.', ' ', '', '', '1', '1', 'VUV')", wxTRANSLATE("Vatu")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('149', '%s', '%s', '', '.', ' ', '', '', '1', '1', 'VND')", wxTRANSLATE("Dong"), L"₫"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('150', '%s', 'WS$', '', '.', ' ', '', '', '100', '1', 'WST')", wxTRANSLATE("Tala")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('151', '%s', '', '', '.', ' ', '', '', '100', '1', 'YER')", wxTRANSLATE("Yemeni Rial")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('152', '%s', 'Bs.', '', '.', ',', '%s', '%s', '100', '1', 'VEF')", wxTRANSLATE("Bolívar"), L"bolívar", L"céntimos"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('153', '%s', '%s', '', '.', ',', '', '', '100000000', '1', 'BTC')", wxTRANSLATE("Bitcoin"), L"Ƀ"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('154', '%s', '', '', '.', ',', '', '', '100', '1', 'BOV')", wxTRANSLATE("Mvdol")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('156', '%s', '', '', '.', ',', '', '', '100', '1', 'CHE')", wxTRANSLATE("WIR Euro")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('157', '%s', '', '', '.', ',', '', '', '100', '1', 'CHW')", wxTRANSLATE("WIR Franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('158', '%s', '', '', '.', ',', '', '', '10000', '1', 'CLF')", wxTRANSLATE("Unidad de Fomento")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('159', '%s', '', '', '.', ',', '', '', '100', '1', 'COU')", wxTRANSLATE("Unidad de Valor Real")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('160', '%s', '', '', '.', ',', '', '', '100', '1', 'CUC')", wxTRANSLATE("Peso Convertible")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('161', '%s', '', '', '.', ',', '', '', '100', '1', 'CUP')", wxTRANSLATE("Cuban Peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('162', '%s', '', '', '.', ',', '', '', '100', '1', 'MRU')", wxTRANSLATE("Ouguiya")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('163', '%s', '', '', '.', ',', '', '', '100', '1', 'MXV')", wxTRANSLATE("Mexican Unidad de Inversion (UDI)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('164', '%s', '', '', '.', ',', '', '', '100', '1', 'MZN')", wxTRANSLATE("Mozambique Metical")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('165', '%s', '', '', '.', ',', '', '', '100', '1', 'SSP')", wxTRANSLATE("South Sudanese Pound")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('166', '%s', '', '', '.', ',', '', '', '100', '1', 'STN')", wxTRANSLATE("Dobra")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('167', '%s', '', '', '.', ',', '', '', '100', '1', 'SVC')", wxTRANSLATE("El Salvador Colon")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('168', '%s', '', '', '.', ',', '', '', '100', '1', 'TOP')", wxTRANSLATE("Pa’anga")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('169', '%s', '', '', '.', ',', '', '', '100', '1', 'USN')", wxTRANSLATE("US Dollar (Next day)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('170', '%s', '', '', '.', ',', '', '', '1', '1', 'UYI')", wxTRANSLATE("Uruguay Peso en Unidades Indexadas (URUIURUI)")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('171', '%s', '', '', '.', ',', '', '', '100', '1', 'ZMW')", wxTRANSLATE("Zambian Kwacha")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('172', '%s', '', '', '.', ',', '', '', '100', '1', 'ZWL')", wxTRANSLATE("Zimbabwe Dollar")));
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

        // Return the data record as a json string
        wxString to_json() const
        {
            StringBuffer json_buffer;
            PrettyWriter<StringBuffer> json_writer(json_buffer);

			json_writer.StartObject();			
			this->as_json(json_writer);
            json_writer.EndObject();

            return json_buffer.GetString();
        }

        // Add the field data as json key:value pairs
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
            json_writer.Key("UNIT_NAME");
            json_writer.String(this->UNIT_NAME.c_str());
            json_writer.Key("CENT_NAME");
            json_writer.String(this->CENT_NAME.c_str());
            json_writer.Key("SCALE");
            json_writer.Int(this->SCALE);
            json_writer.Key("BASECONVRATE");
            json_writer.Double(this->BASECONVRATE);
            json_writer.Key("CURRENCY_SYMBOL");
            json_writer.String(this->CURRENCY_SYMBOL.c_str());
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
        query_ = "SELECT CURRENCYID, CURRENCYNAME, PFX_SYMBOL, SFX_SYMBOL, DECIMAL_POINT, GROUP_SEPARATOR, UNIT_NAME, CENT_NAME, SCALE, BASECONVRATE, CURRENCY_SYMBOL FROM CURRENCYFORMATS_V1 ";
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

