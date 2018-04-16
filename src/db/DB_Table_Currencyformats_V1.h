// -*- C++ -*-
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
 *          AUTO GENERATED at 2018-04-16 14:47:03.861665.
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
                db->ExecuteUpdate("CREATE TABLE CURRENCYFORMATS_V1(CURRENCYID integer primary key, CURRENCYNAME TEXT COLLATE NOCASE NOT NULL, PFX_SYMBOL TEXT, SFX_SYMBOL TEXT, DECIMAL_POINT TEXT, GROUP_SEPARATOR TEXT, SCALE integer, BASECONVRATE numeric, CURRENCY_SYMBOL TEXT COLLATE NOCASE NOT NULL UNIQUE, CURRENCY_TYPE TEXT /* Traditional, Crypto */, HISTORIC integer DEFAULT 0 /* 1 if no longer official */)");
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
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('1', '%s', '$', '', '.', ' ', '100', '1', 'USD', 'Traditional', '0')", wxTRANSLATE("United States dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('2', '%s', '%s', '', '.', ' ', '100', '1', 'EUR', 'Traditional', '0')", wxTRANSLATE("European euro"), L"€"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('3', '%s', '%s', '', '.', ' ', '100', '1', 'GBP', 'Traditional', '0')", wxTRANSLATE("UK Pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('4', '%s', '', '%s', ',', ' ', '100', '1', 'RUB', 'Traditional', '0')", wxTRANSLATE("Russian Ruble"), L"р"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('5', '%s', '%s', '', ',', ' ', '100', '1', 'UAH', 'Traditional', '0')", wxTRANSLATE("Ukrainian hryvnia"), L"₴"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('6', '%s', '%s', '', '.', ' ', '100', '1', 'AFN', 'Traditional', '0')", wxTRANSLATE("Afghan afghani"), L"؋"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('7', '%s', '', 'L', '.', ' ', '1', '1', 'ALL', 'Traditional', '0')", wxTRANSLATE("Albanian lek")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('8', '%s', '%s', '', '.', ' ', '100', '1', 'DZD', 'Traditional', '0')", wxTRANSLATE("Algerian dinar"), L"دج"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('9', '%s', '', 'Kz', '.', ' ', '100', '1', 'AOA', 'Traditional', '0')", wxTRANSLATE("Angolan kwanza")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('10', '%s', 'EC$', '', '.', ' ', '100', '1', 'XCD', 'Traditional', '0')", wxTRANSLATE("East Caribbean dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('11', '%s', 'AR$', '', ',', '.', '100', '1', 'ARS', 'Traditional', '0')", wxTRANSLATE("Argentine peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('12', '%s', '', '', '.', ' ', '1', '1', 'AMD', 'Traditional', '0')", wxTRANSLATE("Armenian dram")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('13', '%s', '%s', '', '.', ' ', '100', '1', 'AWG', 'Traditional', '0')", wxTRANSLATE("Aruban florin"), L"ƒ"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('14', '%s', '$', '', '.', ',', '100', '1', 'AUD', 'Traditional', '0')", wxTRANSLATE("Australian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('15', '%s', '', '', '.', ' ', '100', '1', 'AZN', 'Traditional', '0')", wxTRANSLATE("Azerbaijani manat")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('16', '%s', 'B$', '', '.', ' ', '100', '1', 'BSD', 'Traditional', '0')", wxTRANSLATE("Bahamian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('17', '%s', '', '', '.', ' ', '100', '1', 'BHD', 'Traditional', '0')", wxTRANSLATE("Bahraini dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('18', '%s', '', '', '.', ' ', '100', '1', 'BDT', 'Traditional', '0')", wxTRANSLATE("Bangladeshi taka")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('19', '%s', 'Bds$', '', '.', ' ', '100', '1', 'BBD', 'Traditional', '0')", wxTRANSLATE("Barbadian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('20', '%s', 'Br', '', ',', ' ', '1', '1', 'BYR', 'Traditional', '0')", wxTRANSLATE("Belarusian ruble")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('21', '%s', 'BZ$', '', '.', ' ', '100', '1', 'BZD', 'Traditional', '0')", wxTRANSLATE("Belize dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('22', '%s', 'CFA', '', '.', ' ', '100', '1', 'XOF', 'Traditional', '0')", wxTRANSLATE("West African CFA franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('23', '%s', 'BD$', '', '.', ' ', '100', '1', 'BMD', 'Traditional', '0')", wxTRANSLATE("Bermudian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('24', '%s', 'Nu.', '', '.', ' ', '100', '1', 'BTN', 'Traditional', '0')", wxTRANSLATE("Bhutanese ngultrum")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('25', '%s', 'Bs.', '', '.', ' ', '100', '1', 'BOB', 'Traditional', '0')", wxTRANSLATE("Bolivian boliviano")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('26', '%s', 'KM', '', ',', '.', '100', '1', 'BAM', 'Traditional', '0')", wxTRANSLATE("Bosnia and Herzegovina konvertibilna marka")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('27', '%s', 'P', '', '.', ' ', '100', '1', 'BWP', 'Traditional', '0')", wxTRANSLATE("Botswana pula")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('28', '%s', 'R$', '', '.', ' ', '100', '1', 'BRL', 'Traditional', '0')", wxTRANSLATE("Brazilian real")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('29', '%s', 'B$', '', '.', ' ', '100', '1', 'BND', 'Traditional', '0')", wxTRANSLATE("Brunei dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('30', '%s', '', '', '.', ' ', '100', '1', 'BGN', 'Traditional', '0')", wxTRANSLATE("Bulgarian lev")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('31', '%s', 'FBu', '', '.', ' ', '1', '1', 'BIF', 'Traditional', '0')", wxTRANSLATE("Burundi franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('32', '%s', '', '', '.', ' ', '100', '1', 'KHR', 'Traditional', '0')", wxTRANSLATE("Cambodian riel")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('33', '%s', 'CFA', '', '.', ' ', '1', '1', 'XAF', 'Traditional', '0')", wxTRANSLATE("Central African CFA franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('34', '%s', '$', '', '.', ' ', '100', '1', 'CAD', 'Traditional', '0')", wxTRANSLATE("Canadian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('35', '%s', 'Esc', '', '.', ' ', '100', '1', 'CVE', 'Traditional', '0')", wxTRANSLATE("Cape Verdean escudo")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('36', '%s', 'KY$', '', '.', ' ', '100', '1', 'KYD', 'Traditional', '0')", wxTRANSLATE("Cayman Islands dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('37', '%s', '$', '', '.', ' ', '1', '1', 'CLP', 'Traditional', '0')", wxTRANSLATE("Chilean peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('38', '%s', '%s', '', '.', ' ', '100', '1', 'CNY', 'Traditional', '0')", wxTRANSLATE("Chinese renminbi"), L"¥"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('39', '%s', 'Col$', '', '.', ' ', '100', '1', 'COP', 'Traditional', '0')", wxTRANSLATE("Colombian peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('40', '%s', '', '', '.', ' ', '1', '1', 'KMF', 'Traditional', '0')", wxTRANSLATE("Comorian franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('41', '%s', 'F', '', '.', ' ', '100', '1', 'CDF', 'Traditional', '0')", wxTRANSLATE("Congolese franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('42', '%s', '%s', '', '.', ' ', '1', '1', 'CRC', 'Traditional', '0')", wxTRANSLATE("Costa Rican colon"), L"₡"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('43', '%s', 'kn', '', '.', ' ', '100', '1', 'HRK', 'Traditional', '0')", wxTRANSLATE("Croatian kuna")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('44', '%s', '%s', '', '.', ' ', '100', '1', 'CZK', 'Traditional', '0')", wxTRANSLATE("Czech koruna"), L"Kč"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('45', '%s', 'Kr', '', '.', ' ', '100', '1', 'DKK', 'Traditional', '0')", wxTRANSLATE("Danish krone")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('46', '%s', 'Fdj', '', '.', ' ', '1', '1', 'DJF', 'Traditional', '0')", wxTRANSLATE("Djiboutian franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('47', '%s', 'RD$', '', '.', ' ', '100', '1', 'DOP', 'Traditional', '0')", wxTRANSLATE("Dominican peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('48', '%s', '%s', '', '.', ' ', '100', '1', 'EGP', 'Traditional', '0')", wxTRANSLATE("Egyptian pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('49', '%s', 'Nfa', '', '.', ' ', '100', '1', 'ERN', 'Traditional', '0')", wxTRANSLATE("Eritrean nakfa")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('50', '%s', 'Br', '', '.', ' ', '100', '1', 'ETB', 'Traditional', '0')", wxTRANSLATE("Ethiopian birr")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('51', '%s', '%s', '', '.', ' ', '100', '1', 'FKP', 'Traditional', '0')", wxTRANSLATE("Falkland Islands pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('52', '%s', 'FJ$', '', '.', ' ', '100', '1', 'FJD', 'Traditional', '0')", wxTRANSLATE("Fijian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('53', '%s', 'F', '', '.', ' ', '100', '1', 'XPF', 'Traditional', '0')", wxTRANSLATE("CFP franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('54', '%s', 'D', '', '.', ' ', '100', '1', 'GMD', 'Traditional', '0')", wxTRANSLATE("Gambian dalasi")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('55', '%s', '', '', '.', ' ', '100', '1', 'GEL', 'Traditional', '0')", wxTRANSLATE("Georgian lari")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('56', '%s', '', '', '.', ' ', '100', '1', 'GHS', 'Traditional', '0')", wxTRANSLATE("Ghanaian cedi")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('57', '%s', '%s', '', '.', ' ', '100', '1', 'GIP', 'Traditional', '0')", wxTRANSLATE("Gibraltar pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('58', '%s', 'Q', '', '.', ' ', '100', '1', 'GTQ', 'Traditional', '0')", wxTRANSLATE("Guatemalan quetzal")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('59', '%s', 'FG', '', '.', ' ', '1', '1', 'GNF', 'Traditional', '0')", wxTRANSLATE("Guinean franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('60', '%s', 'GY$', '', '.', ' ', '100', '1', 'GYD', 'Traditional', '0')", wxTRANSLATE("Guyanese dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('61', '%s', 'G', '', '.', ' ', '100', '1', 'HTG', 'Traditional', '0')", wxTRANSLATE("Haitian gourde")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('62', '%s', 'L', '', '.', ' ', '100', '1', 'HNL', 'Traditional', '0')", wxTRANSLATE("Honduran lempira")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('63', '%s', 'HK$', '', '.', ' ', '100', '1', 'HKD', 'Traditional', '0')", wxTRANSLATE("Hong Kong dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('64', '%s', 'Ft', '', '.', ' ', '1', '1', 'HUF', 'Traditional', '0')", wxTRANSLATE("Hungarian forint")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('65', '%s', 'kr', '', '.', ' ', '1', '1', 'ISK', 'Traditional', '0')", L"Icelandic króna"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('66', '%s', '%s', '', '.', ' ', '100', '1', 'INR', 'Traditional', '0')", wxTRANSLATE("Indian rupee"), L"₹"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('67', '%s', 'Rp', '', '.', ' ', '1', '1', 'IDR', 'Traditional', '0')", wxTRANSLATE("Indonesian rupiah")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('68', '%s', 'SDR', '', '.', ' ', '100', '1', 'XDR', 'Traditional', '0')", wxTRANSLATE("Special Drawing Rights")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('69', '%s', '', '', '.', ' ', '1', '1', 'IRR', 'Traditional', '0')", wxTRANSLATE("Iranian rial")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('70', '%s', '', '', '.', ' ', '1', '1', 'IQD', 'Traditional', '0')", wxTRANSLATE("Iraqi dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('71', '%s', '%s', '', '.', ' ', '100', '1', 'ILS', 'Traditional', '0')", wxTRANSLATE("Israeli new shekel"), L"₪"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('72', '%s', 'J$', '', '.', ' ', '100', '1', 'JMD', 'Traditional', '0')", wxTRANSLATE("Jamaican dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('73', '%s', '%s', '', '.', ' ', '1', '1', 'JPY', 'Traditional', '0')", wxTRANSLATE("Japanese yen"), L"¥"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('74', '%s', '', '', '.', ' ', '100', '1', 'JOD', 'Traditional', '0')", wxTRANSLATE("Jordanian dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('75', '%s', 'T', '', '.', ' ', '100', '1', 'KZT', 'Traditional', '0')", wxTRANSLATE("Kazakhstani tenge")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('76', '%s', 'KSh', '', '.', ' ', '100', '1', 'KES', 'Traditional', '0')", wxTRANSLATE("Kenyan shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('77', '%s', 'W', '', '.', ' ', '100', '1', 'KPW', 'Traditional', '0')", wxTRANSLATE("North Korean won")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('78', '%s', 'W', '', '.', ' ', '1', '1', 'KRW', 'Traditional', '0')", wxTRANSLATE("South Korean won")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('79', '%s', '', '', '.', ' ', '100', '1', 'KWD', 'Traditional', '0')", wxTRANSLATE("Kuwaiti dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('80', '%s', '', '', '.', ' ', '100', '1', 'KGS', 'Traditional', '0')", wxTRANSLATE("Kyrgyzstani som")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('81', '%s', 'KN', '', '.', ' ', '100', '1', 'LAK', 'Traditional', '0')", wxTRANSLATE("Lao kip")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('82', '%s', 'Ls', '', '.', ' ', '100', '1', 'LVL', 'Traditional', '0')", wxTRANSLATE("Latvian lats")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('83', '%s', '', '', '.', ' ', '1', '1', 'LBP', 'Traditional', '0')", wxTRANSLATE("Lebanese lira")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('84', '%s', 'M', '', '.', ' ', '100', '1', 'LSL', 'Traditional', '0')", wxTRANSLATE("Lesotho loti")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('85', '%s', 'L$', '', '.', ' ', '100', '1', 'LRD', 'Traditional', '0')", wxTRANSLATE("Liberian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('86', '%s', 'LD', '', '.', ' ', '100', '1', 'LYD', 'Traditional', '0')", wxTRANSLATE("Libyan dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('87', '%s', 'Lt', '', '.', ' ', '100', '1', 'LTL', 'Traditional', '0')", wxTRANSLATE("Lithuanian litas")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('88', '%s', 'P', '', '.', ' ', '100', '1', 'MOP', 'Traditional', '0')", wxTRANSLATE("Macanese pataca")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('89', '%s', '', '', '.', ' ', '100', '1', 'MKD', 'Traditional', '0')", wxTRANSLATE("Macedonian denar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('90', '%s', 'FMG', '', '.', ' ', '100', '1', 'MGA', 'Traditional', '0')", wxTRANSLATE("Malagasy ariary")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('91', '%s', 'MK', '', '.', ' ', '1', '1', 'MWK', 'Traditional', '0')", wxTRANSLATE("Malawian kwacha")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('92', '%s', 'RM', '', '.', ' ', '100', '1', 'MYR', 'Traditional', '0')", wxTRANSLATE("Malaysian ringgit")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('93', '%s', 'Rf', '', '.', ' ', '100', '1', 'MVR', 'Traditional', '0')", wxTRANSLATE("Maldivian rufiyaa")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('94', '%s', 'UM', '', '.', ' ', '100', '1', 'MRO', 'Traditional', '0')", wxTRANSLATE("Mauritanian ouguiya")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('95', '%s', 'Rs', '', '.', ' ', '1', '1', 'MUR', 'Traditional', '0')", wxTRANSLATE("Mauritian rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('96', '%s', '$', '', '.', ' ', '100', '1', 'MXN', 'Traditional', '0')", wxTRANSLATE("Mexican peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('97', '%s', '', '', '.', ' ', '100', '1', 'MDL', 'Traditional', '0')", wxTRANSLATE("Moldovan leu")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('98', '%s', '%s', '', '.', ' ', '100', '1', 'MNT', 'Traditional', '0')", wxTRANSLATE("Mongolian tugrik"), L"₮"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('99', '%s', '', '', '.', ' ', '100', '1', 'MAD', 'Traditional', '0')", wxTRANSLATE("Moroccan dirham")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('100', '%s', 'K', '', '.', ' ', '1', '1', 'MMK', 'Traditional', '0')", wxTRANSLATE("Myanma kyat")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('101', '%s', 'N$', '', '.', ' ', '100', '1', 'NAD', 'Traditional', '0')", wxTRANSLATE("Namibian dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('102', '%s', 'NRs', '', '.', ' ', '100', '1', 'NPR', 'Traditional', '0')", wxTRANSLATE("Nepalese rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('103', '%s', '%s', '', '.', ' ', '100', '1', 'ANG', 'Traditional', '0')", wxTRANSLATE("Netherlands Antillean gulden"), L"NAƒ"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('104', '%s', 'NZ$', '', '.', ' ', '100', '1', 'NZD', 'Traditional', '0')", wxTRANSLATE("New Zealand dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('105', '%s', 'C$', '', '.', ' ', '100', '1', 'NIO', 'Traditional', '0')", L"Nicaraguan córdoba"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('106', '%s', '%s', '', '.', ' ', '100', '1', 'NGN', 'Traditional', '0')", wxTRANSLATE("Nigerian naira"), L"₦"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('107', '%s', 'kr', '', '.', ' ', '100', '1', 'NOK', 'Traditional', '0')", wxTRANSLATE("Norwegian krone")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('108', '%s', '', '', '.', ' ', '100', '1', 'OMR', 'Traditional', '0')", wxTRANSLATE("Omani rial")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('109', '%s', 'Rs.', '', '.', ' ', '1', '1', 'PKR', 'Traditional', '0')", wxTRANSLATE("Pakistani rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('110', '%s', 'B./', '', '.', ' ', '100', '1', 'PAB', 'Traditional', '0')", wxTRANSLATE("Panamanian balboa")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('111', '%s', 'K', '', '.', ' ', '100', '1', 'PGK', 'Traditional', '0')", wxTRANSLATE("Papua New Guinean kina")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('112', '%s', '', '', '.', ' ', '1', '1', 'PYG', 'Traditional', '0')", wxTRANSLATE("Paraguayan guarani")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('113', '%s', 'S/.', '', '.', ' ', '100', '1', 'PEN', 'Traditional', '0')", wxTRANSLATE("Peruvian nuevo sol")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('114', '%s', '%s', '', '.', ' ', '100', '1', 'PHP', 'Traditional', '0')", wxTRANSLATE("Philippine peso"), L"₱"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('115', '%s', '', '%s', ',', '.', '100', '1', 'PLN', 'Traditional', '0')", wxTRANSLATE("Polish zloty"), L"zł"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('116', '%s', 'QR', '', '.', ' ', '100', '1', 'QAR', 'Traditional', '0')", wxTRANSLATE("Qatari riyal")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('117', '%s', 'L', '', '.', ' ', '100', '1', 'RON', 'Traditional', '0')", wxTRANSLATE("Romanian leu")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('118', '%s', 'RF', '', '.', ' ', '1', '1', 'RWF', 'Traditional', '0')", wxTRANSLATE("Rwandan franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('119', '%s', 'Db', '', '.', ' ', '0', '1', 'STD', 'Traditional', '0')", L"São Tomé and Príncipe dobra"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('120', '%s', 'SR', '', '.', ' ', '100', '1', 'SAR', 'Traditional', '0')", wxTRANSLATE("Saudi riyal")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('121', '%s', 'din.', '', '.', ' ', '1', '1', 'RSD', 'Traditional', '0')", wxTRANSLATE("Serbian dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('122', '%s', 'SR', '', '.', ' ', '100', '1', 'SCR', 'Traditional', '0')", wxTRANSLATE("Seychellois rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('123', '%s', 'Le', '', '.', ' ', '100', '1', 'SLL', 'Traditional', '0')", wxTRANSLATE("Sierra Leonean leone")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('124', '%s', 'S$', '', '.', ' ', '100', '1', 'SGD', 'Traditional', '0')", wxTRANSLATE("Singapore dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('125', '%s', 'SI$', '', '.', ' ', '100', '1', 'SBD', 'Traditional', '0')", wxTRANSLATE("Solomon Islands dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('126', '%s', 'Sh.', '', '.', ' ', '1', '1', 'SOS', 'Traditional', '0')", wxTRANSLATE("Somali shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('127', '%s', 'R', '', '.', ' ', '100', '1', 'ZAR', 'Traditional', '0')", wxTRANSLATE("South African rand")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('128', '%s', 'Rs', '', '.', ' ', '100', '1', 'LKR', 'Traditional', '0')", wxTRANSLATE("Sri Lankan rupee")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('129', '%s', '%s', '', '.', ' ', '100', '1', 'SHP', 'Traditional', '0')", wxTRANSLATE("Saint Helena pound"), L"£"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('130', '%s', '', '', '.', ' ', '100', '1', 'SDG', 'Traditional', '0')", wxTRANSLATE("Sudanese pound")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('131', '%s', '$', '', '.', ' ', '100', '1', 'SRD', 'Traditional', '0')", wxTRANSLATE("Surinamese dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('132', '%s', 'E', '', '.', ' ', '100', '1', 'SZL', 'Traditional', '0')", wxTRANSLATE("Swazi lilangeni")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('133', '%s', 'kr', '', '.', ' ', '100', '1', 'SEK', 'Traditional', '0')", wxTRANSLATE("Swedish krona")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('134', '%s', 'Fr.', '', '.', ' ', '100', '1', 'CHF', 'Traditional', '0')", wxTRANSLATE("Swiss franc")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('135', '%s', '', '', '.', ' ', '1', '1', 'SYP', 'Traditional', '0')", wxTRANSLATE("Syrian pound")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('136', '%s', 'NT$', '', '.', ' ', '100', '1', 'TWD', 'Traditional', '0')", wxTRANSLATE("New Taiwan dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('137', '%s', '', '', '.', ' ', '100', '1', 'TJS', 'Traditional', '0')", wxTRANSLATE("Tajikistani somoni")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('138', '%s', '', '', '.', ' ', '1', '1', 'TZS', 'Traditional', '0')", wxTRANSLATE("Tanzanian shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('139', '%s', '%s', '', '.', ' ', '100', '1', 'THB', 'Traditional', '0')", wxTRANSLATE("Thai baht"), L"฿"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('140', '%s', 'TT$', '', '.', ' ', '100', '1', 'TTD', 'Traditional', '0')", wxTRANSLATE("Trinidad and Tobago dollar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('141', '%s', 'DT', '', '.', ' ', '100', '1', 'TND', 'Traditional', '0')", wxTRANSLATE("Tunisian dinar")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('142', '%s', '%s', '', '.', ' ', '100', '1', 'TRY', 'Traditional', '0')", wxTRANSLATE("Turkish lira"), L"₺"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('143', '%s', 'm', '', '.', ' ', '100', '1', 'TMT', 'Traditional', '0')", wxTRANSLATE("Turkmen manat")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('144', '%s', 'USh', '', '.', ' ', '1', '1', 'UGX', 'Traditional', '0')", wxTRANSLATE("Ugandan shilling")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('145', '%s', '', '', '.', ' ', '100', '1', 'AED', 'Traditional', '0')", wxTRANSLATE("UAE dirham")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('146', '%s', '$U', '', '.', ' ', '100', '1', 'UYU', 'Traditional', '0')", wxTRANSLATE("Uruguayan peso")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('147', '%s', '', '', '.', ' ', '1', '1', 'UZS', 'Traditional', '0')", wxTRANSLATE("Uzbekistani som")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('148', '%s', 'VT', '', '.', ' ', '100', '1', 'VUV', 'Traditional', '0')", wxTRANSLATE("Vanuatu vatu")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('149', '%s', '%s', '', '.', ' ', '1', '1', 'VND', 'Traditional', '0')", wxTRANSLATE("Vietnamese dong"), L"₫"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('150', '%s', 'WS$', '', '.', ' ', '100', '1', 'WST', 'Traditional', '0')", wxTRANSLATE("Samoan tala")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('151', '%s', '', '', '.', ' ', '1', '1', 'YER', 'Traditional', '0')", wxTRANSLATE("Yemeni rial")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('152', '%s', 'Bs.', '', '.', ',', '100', '1', 'VEF', 'Traditional', '0')", L"Venezuelan Bolívar"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('153', '%s', '%s', '', '.', ',', '100000000', '1', 'BTC', 'Crypto', '0')", wxTRANSLATE("Bitcoin"), L"Ƀ"));
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
        , COL_BASECONVRATE = 7
        , COL_CURRENCY_SYMBOL = 8
        , COL_CURRENCY_TYPE = 9
        , COL_HISTORIC = 10
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
            case COL_BASECONVRATE: return "BASECONVRATE";
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
        else if ("BASECONVRATE" == name) return COL_BASECONVRATE;
        else if ("CURRENCY_SYMBOL" == name) return COL_CURRENCY_SYMBOL;
        else if ("CURRENCY_TYPE" == name) return COL_CURRENCY_TYPE;
        else if ("HISTORIC" == name) return COL_HISTORIC;

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
        int SCALE;
        double BASECONVRATE;
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
            BASECONVRATE = 0.0;
            HISTORIC = -1;
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
            SCALE = q.GetInt(6); // SCALE
            BASECONVRATE = q.GetDouble(7); // BASECONVRATE
            CURRENCY_SYMBOL = q.GetString(8); // CURRENCY_SYMBOL
            CURRENCY_TYPE = q.GetString(9); // CURRENCY_TYPE
            HISTORIC = q.GetInt(10); // HISTORIC
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
            BASECONVRATE = other.BASECONVRATE;
            CURRENCY_SYMBOL = other.CURRENCY_SYMBOL;
            CURRENCY_TYPE = other.CURRENCY_TYPE;
            HISTORIC = other.HISTORIC;
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

        bool match(const Self::HISTORIC &in) const
        {
            return this->HISTORIC == in.v_;
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
            json_writer.Key("SCALE");
            json_writer.Int(this->SCALE);
            json_writer.Key("BASECONVRATE");
            json_writer.Double(this->BASECONVRATE);
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
            row(L"BASECONVRATE") = BASECONVRATE;
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
            t(L"BASECONVRATE") = BASECONVRATE;
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
        query_ = "SELECT CURRENCYID, CURRENCYNAME, PFX_SYMBOL, SFX_SYMBOL, DECIMAL_POINT, GROUP_SEPARATOR, SCALE, BASECONVRATE, CURRENCY_SYMBOL, CURRENCY_TYPE, HISTORIC FROM CURRENCYFORMATS_V1 ";
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
            sql = "INSERT INTO CURRENCYFORMATS_V1(CURRENCYNAME, PFX_SYMBOL, SFX_SYMBOL, DECIMAL_POINT, GROUP_SEPARATOR, SCALE, BASECONVRATE, CURRENCY_SYMBOL, CURRENCY_TYPE, HISTORIC) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE CURRENCYFORMATS_V1 SET CURRENCYNAME = ?, PFX_SYMBOL = ?, SFX_SYMBOL = ?, DECIMAL_POINT = ?, GROUP_SEPARATOR = ?, SCALE = ?, BASECONVRATE = ?, CURRENCY_SYMBOL = ?, CURRENCY_TYPE = ?, HISTORIC = ? WHERE CURRENCYID = ?";
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
            stmt.Bind(7, entity->BASECONVRATE);
            stmt.Bind(8, entity->CURRENCY_SYMBOL);
            stmt.Bind(9, entity->CURRENCY_TYPE);
            stmt.Bind(10, entity->HISTORIC);
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

