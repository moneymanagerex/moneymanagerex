// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013 - 2022 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017 - 2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022 Mark Whalley (mark@ipx.co.uk)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2022-09-28 23:10:47.317664.
 *          DO NOT EDIT!
 */
//=============================================================================
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
                db->ExecuteUpdate("CREATE TABLE CURRENCYFORMATS_V1(CURRENCYID integer primary key, CURRENCYNAME TEXT COLLATE NOCASE NOT NULL UNIQUE, PFX_SYMBOL TEXT, SFX_SYMBOL TEXT, DECIMAL_POINT TEXT, GROUP_SEPARATOR TEXT, UNIT_NAME TEXT COLLATE NOCASE, CENT_NAME TEXT COLLATE NOCASE, SCALE integer, BASECONVRATE numeric, CURRENCY_SYMBOL TEXT COLLATE NOCASE NOT NULL UNIQUE, CURRENCY_TYPE TEXT NOT NULL /* Fiat, Crypto */)");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("CURRENCYFORMATS_V1: Exception %s", e.GetMessage().utf8_str());
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
            wxLogError("CURRENCYFORMATS_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('1', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("United States dollar"), L"$", L"", L".", L" ", L"", L"", L"USD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('2', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("European euro"), L"€", L"", L".", L" ", L"", L"", L"EUR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('3', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("UK Pound"), L"£", L"", L".", L" ", L"Pound", L"Pence", L"GBP", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('4', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Russian Ruble"), L"", L"р", L",", L" ", L"руб.", L"коп.", L"RUB", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('5', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Ukrainian hryvnia"), L"₴", L"", L",", L" ", L"", L"", L"UAH", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('6', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Afghan afghani"), L"؋", L"", L".", L" ", L"", L"pul", L"AFN", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('7', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Albanian lek"), L"", L"L", L".", L" ", L"", L"", L"ALL", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('8', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Algerian dinar"), L"دج", L"", L".", L" ", L"", L"", L"DZD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('9', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Angolan kwanza"), L"", L"Kz", L".", L" ", L"", L"Céntimo", L"AOA", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('10', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("East Caribbean dollar"), L"EC$", L"", L".", L" ", L"", L"", L"XCD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('11', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Argentine peso"), L"AR$", L"", L",", L".", L"", L"centavo", L"ARS", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('12', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Armenian dram"), L"", L"", L".", L" ", L"", L"", L"AMD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('13', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Aruban florin"), L"ƒ", L"", L".", L" ", L"", L"", L"AWG", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('14', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Australian dollar"), L"$", L"", L".", L",", L"", L"", L"AUD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('15', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Azerbaijani manat"), L"", L"", L".", L" ", L"", L"", L"AZN", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('16', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Bahamian dollar"), L"B$", L"", L".", L" ", L"", L"", L"BSD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('17', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Bahraini dinar"), L"", L"", L".", L" ", L"", L"", L"BHD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('18', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Bangladeshi taka"), L"", L"", L".", L" ", L"", L"", L"BDT", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('19', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Barbadian dollar"), L"Bds$", L"", L".", L" ", L"", L"", L"BBD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('20', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Belarusian ruble"), L"Br", L"", L",", L" ", L"", L"", L"BYR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('21', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Belize dollar"), L"BZ$", L"", L".", L" ", L"", L"", L"BZD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('22', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("West African CFA franc"), L"CFA", L"", L".", L" ", L"", L"", L"XOF", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('23', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Bermudian dollar"), L"BD$", L"", L".", L" ", L"", L"", L"BMD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('24', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Bhutanese ngultrum"), L"Nu.", L"", L".", L" ", L"", L"", L"BTN", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('25', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Bolivian boliviano"), L"Bs.", L"", L".", L" ", L"", L"", L"BOB", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('26', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Bosnia and Herzegovina konvertibilna marka"), L"KM", L"", L",", L".", L"", L"", L"BAM", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('27', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Botswana pula"), L"P", L"", L".", L" ", L"", L"", L"BWP", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('28', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Brazilian real"), L"R$", L"", L".", L" ", L"", L"", L"BRL", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('29', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Brunei dollar"), L"B$", L"", L".", L" ", L"", L"", L"BND", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('30', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Bulgarian lev"), L"", L"", L".", L" ", L"", L"", L"BGN", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('31', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Burundi franc"), L"FBu", L"", L".", L" ", L"", L"", L"BIF", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('32', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Cambodian riel"), L"", L"", L".", L" ", L"", L"", L"KHR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('33', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Central African CFA franc"), L"CFA", L"", L".", L" ", L"", L"", L"XAF", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('34', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Canadian dollar"), L"$", L"", L".", L" ", L"", L"", L"CAD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('35', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Cape Verdean escudo"), L"Esc", L"", L".", L" ", L"", L"", L"CVE", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('36', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Cayman Islands dollar"), L"KY$", L"", L".", L" ", L"", L"", L"KYD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('37', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Chilean peso"), L"$", L"", L".", L" ", L"", L"", L"CLP", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('38', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Chinese renminbi"), L"¥", L"", L".", L" ", L"", L"", L"CNY", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('39', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Colombian peso"), L"Col$", L"", L".", L" ", L"", L"", L"COP", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('40', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Comorian franc"), L"", L"", L".", L" ", L"", L"", L"KMF", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('41', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Congolese franc"), L"F", L"", L".", L" ", L"", L"", L"CDF", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('42', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Costa Rican colon"), L"₡", L"", L".", L" ", L"", L"", L"CRC", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('43', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Croatian kuna"), L"kn", L"", L".", L" ", L"", L"", L"HRK", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('44', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Czech koruna"), L"Kč", L"", L".", L" ", L"", L"", L"CZK", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('45', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Danish krone"), L"Kr", L"", L".", L" ", L"", L"", L"DKK", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('46', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Djiboutian franc"), L"Fdj", L"", L".", L" ", L"", L"", L"DJF", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('47', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Dominican peso"), L"RD$", L"", L".", L" ", L"", L"", L"DOP", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('48', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Egyptian pound"), L"£", L"", L".", L" ", L"", L"", L"EGP", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('49', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Eritrean nakfa"), L"Nfa", L"", L".", L" ", L"", L"", L"ERN", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('50', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Ethiopian birr"), L"Br", L"", L".", L" ", L"", L"", L"ETB", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('51', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Falkland Islands pound"), L"£", L"", L".", L" ", L"", L"", L"FKP", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('52', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Fijian dollar"), L"FJ$", L"", L".", L" ", L"", L"", L"FJD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('53', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("CFP franc"), L"F", L"", L".", L" ", L"", L"", L"XPF", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('54', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Gambian dalasi"), L"D", L"", L".", L" ", L"", L"", L"GMD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('55', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Georgian lari"), L"", L"", L".", L" ", L"", L"", L"GEL", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('56', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Ghanaian cedi"), L"", L"", L".", L" ", L"", L"", L"GHS", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('57', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Gibraltar pound"), L"£", L"", L".", L" ", L"", L"", L"GIP", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('58', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Guatemalan quetzal"), L"Q", L"", L".", L" ", L"", L"", L"GTQ", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('59', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Guinean franc"), L"FG", L"", L".", L" ", L"", L"", L"GNF", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('60', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Guyanese dollar"), L"GY$", L"", L".", L" ", L"", L"", L"GYD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('61', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Haitian gourde"), L"G", L"", L".", L" ", L"", L"", L"HTG", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('62', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Honduran lempira"), L"L", L"", L".", L" ", L"", L"", L"HNL", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('63', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Hong Kong dollar"), L"HK$", L"", L".", L" ", L"", L"", L"HKD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('64', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Hungarian forint"), L"Ft", L"", L".", L" ", L"", L"", L"HUF", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('65', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", L"Icelandic króna", L"kr", L"", L".", L" ", L"", L"", L"ISK", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('66', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Indian rupee"), L"₹", L"", L".", L" ", L"", L"", L"INR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('67', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Indonesian rupiah"), L"Rp", L"", L".", L" ", L"", L"", L"IDR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('68', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Special Drawing Rights"), L"SDR", L"", L".", L" ", L"", L"", L"XDR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('69', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Iranian rial"), L"", L"", L".", L" ", L"", L"", L"IRR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('70', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Iraqi dinar"), L"", L"", L".", L" ", L"", L"", L"IQD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('71', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Israeli new shekel"), L"₪", L"", L".", L" ", L"", L"", L"ILS", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('72', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Jamaican dollar"), L"J$", L"", L".", L" ", L"", L"", L"JMD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('73', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Japanese yen"), L"¥", L"", L".", L" ", L"", L"", L"JPY", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('74', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Jordanian dinar"), L"", L"", L".", L" ", L"", L"", L"JOD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('75', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Kazakhstani tenge"), L"T", L"", L".", L" ", L"", L"", L"KZT", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('76', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Kenyan shilling"), L"KSh", L"", L".", L" ", L"", L"", L"KES", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('77', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("North Korean won"), L"W", L"", L".", L" ", L"", L"", L"KPW", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('78', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("South Korean won"), L"W", L"", L".", L" ", L"", L"", L"KRW", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('79', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Kuwaiti dinar"), L"", L"", L".", L" ", L"", L"", L"KWD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('80', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Kyrgyzstani som"), L"", L"", L".", L" ", L"", L"", L"KGS", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('81', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Lao kip"), L"KN", L"", L".", L" ", L"", L"", L"LAK", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('82', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Latvian lats"), L"Ls", L"", L".", L" ", L"", L"", L"LVL", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('83', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Lebanese lira"), L"", L"", L".", L" ", L"", L"", L"LBP", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('84', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Lesotho loti"), L"M", L"", L".", L" ", L"", L"", L"LSL", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('85', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Liberian dollar"), L"L$", L"", L".", L" ", L"", L"", L"LRD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('86', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Libyan dinar"), L"LD", L"", L".", L" ", L"", L"", L"LYD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('87', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Lithuanian litas"), L"Lt", L"", L".", L" ", L"", L"", L"LTL", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('88', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Macanese pataca"), L"P", L"", L".", L" ", L"", L"", L"MOP", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('89', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Macedonian denar"), L"", L"", L".", L" ", L"", L"", L"MKD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('90', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Malagasy ariary"), L"FMG", L"", L".", L" ", L"", L"", L"MGA", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('91', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Malawian kwacha"), L"MK", L"", L".", L" ", L"", L"", L"MWK", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('92', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Malaysian ringgit"), L"RM", L"", L".", L" ", L"", L"", L"MYR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('93', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Maldivian rufiyaa"), L"Rf", L"", L".", L" ", L"", L"", L"MVR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('94', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Mauritanian ouguiya"), L"UM", L"", L".", L" ", L"", L"", L"MRO", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('95', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Mauritian rupee"), L"Rs", L"", L".", L" ", L"", L"", L"MUR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('96', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Mexican peso"), L"$", L"", L".", L" ", L"", L"", L"MXN", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('97', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Moldovan leu"), L"", L"", L".", L" ", L"", L"", L"MDL", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('98', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Mongolian tugrik"), L"₮", L"", L".", L" ", L"", L"", L"MNT", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('99', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Moroccan dirham"), L"", L"", L".", L" ", L"", L"", L"MAD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('100', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Myanma kyat"), L"K", L"", L".", L" ", L"", L"", L"MMK", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('101', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Namibian dollar"), L"N$", L"", L".", L" ", L"", L"", L"NAD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('102', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Nepalese rupee"), L"NRs", L"", L".", L" ", L"", L"", L"NPR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('103', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Netherlands Antillean gulden"), L"NAƒ", L"", L".", L" ", L"", L"", L"ANG", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('104', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("New Zealand dollar"), L"NZ$", L"", L".", L" ", L"", L"", L"NZD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('105', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", L"Nicaraguan córdoba", L"C$", L"", L".", L" ", L"", L"", L"NIO", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('106', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Nigerian naira"), L"₦", L"", L".", L" ", L"", L"", L"NGN", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('107', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Norwegian krone"), L"kr", L"", L".", L" ", L"", L"", L"NOK", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('108', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Omani rial"), L"", L"", L".", L" ", L"", L"", L"OMR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('109', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Pakistani rupee"), L"Rs.", L"", L".", L" ", L"", L"", L"PKR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('110', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Panamanian balboa"), L"B./", L"", L".", L" ", L"", L"", L"PAB", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('111', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Papua New Guinean kina"), L"K", L"", L".", L" ", L"", L"", L"PGK", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('112', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Paraguayan guarani"), L"", L"", L".", L" ", L"", L"", L"PYG", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('113', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Peruvian nuevo sol"), L"S/.", L"", L".", L" ", L"", L"", L"PEN", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('114', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Philippine peso"), L"₱", L"", L".", L" ", L"", L"", L"PHP", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('115', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Polish zloty"), L"", L"zł", L",", L".", L"złoty", L"grosz", L"PLN", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('116', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Qatari riyal"), L"QR", L"", L".", L" ", L"", L"", L"QAR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('117', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Romanian leu"), L"L", L"", L".", L" ", L"", L"", L"RON", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('118', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Rwandan franc"), L"RF", L"", L".", L" ", L"", L"", L"RWF", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('119', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", L"São Tomé and Príncipe dobra", L"Db", L"", L".", L" ", L"", L"", L"STD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('120', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Saudi riyal"), L"SR", L"", L".", L" ", L"", L"", L"SAR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('121', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Serbian dinar"), L"din.", L"", L".", L" ", L"", L"", L"RSD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('122', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Seychellois rupee"), L"SR", L"", L".", L" ", L"", L"", L"SCR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('123', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Sierra Leonean leone"), L"Le", L"", L".", L" ", L"", L"", L"SLL", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('124', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Singapore dollar"), L"S$", L"", L".", L" ", L"", L"", L"SGD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('125', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Solomon Islands dollar"), L"SI$", L"", L".", L" ", L"", L"", L"SBD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('126', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Somali shilling"), L"Sh.", L"", L".", L" ", L"", L"", L"SOS", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('127', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("South African rand"), L"R", L"", L".", L" ", L"", L"", L"ZAR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('128', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Sri Lankan rupee"), L"Rs", L"", L".", L" ", L"", L"", L"LKR", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('129', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Saint Helena pound"), L"£", L"", L".", L" ", L"", L"", L"SHP", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('130', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Sudanese pound"), L"", L"", L".", L" ", L"", L"", L"SDG", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('131', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Surinamese dollar"), L"$", L"", L".", L" ", L"", L"", L"SRD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('132', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Swazi lilangeni"), L"E", L"", L".", L" ", L"", L"", L"SZL", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('133', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Swedish krona"), L"kr", L"", L".", L" ", L"", L"", L"SEK", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('134', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Swiss franc"), L"Fr.", L"", L".", L" ", L"", L"", L"CHF", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('135', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Syrian pound"), L"", L"", L".", L" ", L"", L"", L"SYP", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('136', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("New Taiwan dollar"), L"NT$", L"", L".", L" ", L"", L"", L"TWD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('137', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Tajikistani somoni"), L"", L"", L".", L" ", L"", L"", L"TJS", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('138', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Tanzanian shilling"), L"", L"", L".", L" ", L"", L"", L"TZS", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('139', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Thai baht"), L"฿", L"", L".", L" ", L"", L"", L"THB", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('140', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Trinidad and Tobago dollar"), L"TT$", L"", L".", L" ", L"", L"", L"TTD", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('141', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Tunisian dinar"), L"DT", L"", L".", L" ", L"", L"", L"TND", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('142', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Turkish lira"), L"₺", L"", L".", L" ", L"", L"", L"TRY", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('143', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Turkmen manat"), L"m", L"", L".", L" ", L"", L"", L"TMT", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('144', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Ugandan shilling"), L"USh", L"", L".", L" ", L"", L"", L"UGX", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('145', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("UAE dirham"), L"", L"", L".", L" ", L"", L"", L"AED", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('146', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Uruguayan peso"), L"$U", L"", L".", L" ", L"", L"", L"UYU", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('147', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Uzbekistani som"), L"", L"", L".", L" ", L"", L"", L"UZS", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('148', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Vanuatu vatu"), L"VT", L"", L".", L" ", L"", L"", L"VUV", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('149', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Vietnamese dong"), L"₫", L"", L".", L" ", L"", L"", L"VND", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('150', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", _("Samoan tala"), L"WS$", L"", L".", L" ", L"", L"", L"WST", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('151', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '1', '1', '%s', '%s')", _("Yemeni rial"), L"", L"", L".", L" ", L"", L"", L"YER", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('152', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100', '1', '%s', '%s')", L"Venezuelan Bolívar", L"Bs.", L"", L".", L",", L"bolívar", L"céntimos", L"VEF", L"Fiat"));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CURRENCYFORMATS_V1 VALUES ('153', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '100000000', '1', '%s', '%s')", _("Bitcoin"), L"Ƀ", L"", L".", L",", L"", L"", L"BTC", L"Crypto"));
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
            json_writer.String(this->CURRENCYNAME.utf8_str());
            json_writer.Key("PFX_SYMBOL");
            json_writer.String(this->PFX_SYMBOL.utf8_str());
            json_writer.Key("SFX_SYMBOL");
            json_writer.String(this->SFX_SYMBOL.utf8_str());
            json_writer.Key("DECIMAL_POINT");
            json_writer.String(this->DECIMAL_POINT.utf8_str());
            json_writer.Key("GROUP_SEPARATOR");
            json_writer.String(this->GROUP_SEPARATOR.utf8_str());
            json_writer.Key("UNIT_NAME");
            json_writer.String(this->UNIT_NAME.utf8_str());
            json_writer.Key("CENT_NAME");
            json_writer.String(this->CENT_NAME.utf8_str());
            json_writer.Key("SCALE");
            json_writer.Int(this->SCALE);
            json_writer.Key("BASECONVRATE");
            json_writer.Double(this->BASECONVRATE);
            json_writer.Key("CURRENCY_SYMBOL");
            json_writer.String(this->CURRENCY_SYMBOL.utf8_str());
            json_writer.Key("CURRENCY_TYPE");
            json_writer.String(this->CURRENCY_TYPE.utf8_str());
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
            wxLogError("CURRENCYFORMATS_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
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
            wxLogError("CURRENCYFORMATS_V1: Exception %s", e.GetMessage().utf8_str());
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
        wxString where = wxString::Format(" WHERE %s = ?", PRIMARY::name().utf8_str());
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
            wxLogError("%s: Exception %s", this->name().utf8_str(), e.GetMessage().utf8_str());
        }
        
        if (!entity) 
        {
            entity = this->fake_;
            // wxLogError("%s: %d not found", this->name().utf8_str(), id);
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
            wxLogError("%s: Exception %s", this->name().utf8_str(), e.GetMessage().utf8_str());
        }

        return result;
    }
};

