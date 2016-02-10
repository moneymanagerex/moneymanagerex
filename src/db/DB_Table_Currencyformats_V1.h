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
 *          AUTO GENERATED at 2016-02-10 22:43:03.430373.
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
		}

        this->ensure_index(db);
        this->ensure_data(db);

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
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (1, 'United States dollar', '$', '', '.', ' ', '', '', 100, 1, 'USD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (2, 'European euro', '€', '', '.', ' ', '', '', 100, 1, 'EUR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (3, 'UK Pound', '£', '', '.', ' ', 'Pound', 'Pence', 100, 1, 'GBP')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (4, 'Russian Ruble', '', 'р', ',', ' ', 'руб.', 'коп.', 100, 1, 'RUB')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (5, 'Ukrainian hryvnia', '₴', '', ',', ' ', '', '', 100, 1, 'UAH')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (6, 'Afghan afghani', '؋', '', '.', ' ', '', 'pul', 100, 1, 'AFN')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (7, 'Albanian lek', '', 'L', '.', ' ', '', '', 1, 1, 'ALL')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (8, 'Algerian dinar', 'دج', '', '.', ' ', '', '', 100, 1, 'DZD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (9, 'Angolan kwanza', '', 'Kz', '.', ' ', '', 'Céntimo', 100, 1, 'AOA')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (10, 'East Caribbean dollar', 'EC$', '', '.', ' ', '', '', 100, 1, 'XCD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (11, 'Argentine peso', 'AR$', '', ',', '.', '', 'centavo', 100, 1, 'ARS')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (12, 'Armenian dram', '', '', '.', ' ', '', '', 1, 1, 'AMD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (13, 'Aruban florin', 'ƒ', '', '.', ' ', '', '', 100, 1, 'AWG')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (14, 'Australian dollar', '$', '', '.', ',', '', '', 100, 1, 'AUD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (15, 'Azerbaijani manat', '', '', '.', ' ', '', '', 100, 1, 'AZN')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (16, 'Bahamian dollar', 'B$', '', '.', ' ', '', '', 100, 1, 'BSD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (17, 'Bahraini dinar', '', '', '.', ' ', '', '', 100, 1, 'BHD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (18, 'Bangladeshi taka', '', '', '.', ' ', '', '', 100, 1, 'BDT')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (19, 'Barbadian dollar', 'Bds$', '', '.', ' ', '', '', 100, 1, 'BBD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (20, 'Belarusian ruble', 'Br', '', ',', ' ', '', '', 1, 1, 'BYR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (21, 'Belize dollar', 'BZ$', '', '.', ' ', '', '', 100, 1, 'BZD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (22, 'West African CFA franc', 'CFA', '', '.', ' ', '', '', 100, 1, 'XOF')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (23, 'Bermudian dollar', 'BD$', '', '.', ' ', '', '', 100, 1, 'BMD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (24, 'Bhutanese ngultrum', 'Nu.', '', '.', ' ', '', '', 100, 1, 'BTN')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (25, 'Bolivian boliviano', 'Bs.', '', '.', ' ', '', '', 100, 1, 'BOB')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (26, 'Bosnia and Herzegovina konvertibilna marka', 'KM', '', ',', '.', '', '', 100, 1, 'BAM')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (27, 'Botswana pula', 'P', '', '.', ' ', '', '', 100, 1, 'BWP')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (28, 'Brazilian real', 'R$', '', '.', ' ', '', '', 100, 1, 'BRL')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (29, 'Brunei dollar', 'B$', '', '.', ' ', '', '', 100, 1, 'BND')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (30, 'Bulgarian lev', '', '', '.', ' ', '', '', 100, 1, 'BGN')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (31, 'Burundi franc', 'FBu', '', '.', ' ', '', '', 1, 1, 'BIF')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (32, 'Cambodian riel', '', '', '.', ' ', '', '', 100, 1, 'KHR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (33, 'Central African CFA franc', 'CFA', '', '.', ' ', '', '', 1, 1, 'XAF')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (34, 'Canadian dollar', '$', '', '.', ' ', '', '', 100, 1, 'CAD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (35, 'Cape Verdean escudo', 'Esc', '', '.', ' ', '', '', 100, 1, 'CVE')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (36, 'Cayman Islands dollar', 'KY$', '', '.', ' ', '', '', 100, 1, 'KYD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (37, 'Chilean peso', '$', '', '.', ' ', '', '', 1, 1, 'CLP')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (38, 'Chinese renminbi', '¥', '', '.', ' ', '', '', 100, 1, 'CNY')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (39, 'Colombian peso', 'Col$', '', '.', ' ', '', '', 100, 1, 'COP')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (40, 'Comorian franc', '', '', '.', ' ', '', '', 1, 1, 'KMF')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (41, 'Congolese franc', 'F', '', '.', ' ', '', '', 100, 1, 'CDF')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (42, 'Costa Rican colon', '₡', '', '.', ' ', '', '', 1, 1, 'CRC')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (43, 'Croatian kuna', 'kn', '', '.', ' ', '', '', 100, 1, 'HRK')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (44, 'Czech koruna', 'Kč', '', '.', ' ', '', '', 100, 1, 'CZK')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (45, 'Danish krone', 'Kr', '', '.', ' ', '', '', 100, 1, 'DKK')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (46, 'Djiboutian franc', 'Fdj', '', '.', ' ', '', '', 1, 1, 'DJF')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (47, 'Dominican peso', 'RD$', '', '.', ' ', '', '', 100, 1, 'DOP')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (48, 'Egyptian pound', '£', '', '.', ' ', '', '', 100, 1, 'EGP')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (49, 'Eritrean nakfa', 'Nfa', '', '.', ' ', '', '', 100, 1, 'ERN')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (50, 'Ethiopian birr', 'Br', '', '.', ' ', '', '', 100, 1, 'ETB')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (51, 'Falkland Islands pound', '£', '', '.', ' ', '', '', 100, 1, 'FKP')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (52, 'Fijian dollar', 'FJ$', '', '.', ' ', '', '', 100, 1, 'FJD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (53, 'CFP franc', 'F', '', '.', ' ', '', '', 100, 1, 'XPF')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (54, 'Gambian dalasi', 'D', '', '.', ' ', '', '', 100, 1, 'GMD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (55, 'Georgian lari', '', '', '.', ' ', '', '', 100, 1, 'GEL')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (56, 'Ghanaian cedi', '', '', '.', ' ', '', '', 100, 1, 'GHS')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (57, 'Gibraltar pound', '£', '', '.', ' ', '', '', 100, 1, 'GIP')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (58, 'Guatemalan quetzal', 'Q', '', '.', ' ', '', '', 100, 1, 'GTQ')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (59, 'Guinean franc', 'FG', '', '.', ' ', '', '', 1, 1, 'GNF')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (60, 'Guyanese dollar', 'GY$', '', '.', ' ', '', '', 100, 1, 'GYD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (61, 'Haitian gourde', 'G', '', '.', ' ', '', '', 100, 1, 'HTG')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (62, 'Honduran lempira', 'L', '', '.', ' ', '', '', 100, 1, 'HNL')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (63, 'Hong Kong dollar', 'HK$', '', '.', ' ', '', '', 100, 1, 'HKD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (64, 'Hungarian forint', 'Ft', '', '.', ' ', '', '', 1, 1, 'HUF')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (65, 'Icelandic króna', 'kr', '', '.', ' ', '', '', 1, 1, 'ISK')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (66, 'Indian rupee', '₹', '', '.', ' ', '', '', 100, 1, 'INR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (67, 'Indonesian rupiah', 'Rp', '', '.', ' ', '', '', 1, 1, 'IDR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (68, 'Special Drawing Rights', 'SDR', '', '.', ' ', '', '', 100, 1, 'XDR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (69, 'Iranian rial', '', '', '.', ' ', '', '', 1, 1, 'IRR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (70, 'Iraqi dinar', '', '', '.', ' ', '', '', 1, 1, 'IQD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (71, 'Israeli new sheqel', '', '', '.', ' ', '', '', 100, 1, 'ILS')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (72, 'Jamaican dollar', 'J$', '', '.', ' ', '', '', 100, 1, 'JMD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (73, 'Japanese yen', '¥', '', '.', ' ', '', '', 1, 1, 'JPY')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (74, 'Jordanian dinar', '', '', '.', ' ', '', '', 100, 1, 'JOD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (75, 'Kazakhstani tenge', 'T', '', '.', ' ', '', '', 100, 1, 'KZT')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (76, 'Kenyan shilling', 'KSh', '', '.', ' ', '', '', 100, 1, 'KES')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (77, 'North Korean won', 'W', '', '.', ' ', '', '', 100, 1, 'KPW')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (78, 'South Korean won', 'W', '', '.', ' ', '', '', 1, 1, 'KRW')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (79, 'Kuwaiti dinar', '', '', '.', ' ', '', '', 100, 1, 'KWD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (80, 'Kyrgyzstani som', '', '', '.', ' ', '', '', 100, 1, 'KGS')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (81, 'Lao kip', 'KN', '', '.', ' ', '', '', 100, 1, 'LAK')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (82, 'Latvian lats', 'Ls', '', '.', ' ', '', '', 100, 1, 'LVL')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (83, 'Lebanese lira', '', '', '.', ' ', '', '', 1, 1, 'LBP')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (84, 'Lesotho loti', 'M', '', '.', ' ', '', '', 100, 1, 'LSL')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (85, 'Liberian dollar', 'L$', '', '.', ' ', '', '', 100, 1, 'LRD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (86, 'Libyan dinar', 'LD', '', '.', ' ', '', '', 100, 1, 'LYD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (87, 'Lithuanian litas', 'Lt', '', '.', ' ', '', '', 100, 1, 'LTL')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (88, 'Macanese pataca', 'P', '', '.', ' ', '', '', 100, 1, 'MOP')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (89, 'Macedonian denar', '', '', '.', ' ', '', '', 100, 1, 'MKD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (90, 'Malagasy ariary', 'FMG', '', '.', ' ', '', '', 100, 1, 'MGA')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (91, 'Malawian kwacha', 'MK', '', '.', ' ', '', '', 1, 1, 'MWK')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (92, 'Malaysian ringgit', 'RM', '', '.', ' ', '', '', 100, 1, 'MYR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (93, 'Maldivian rufiyaa', 'Rf', '', '.', ' ', '', '', 100, 1, 'MVR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (94, 'Mauritanian ouguiya', 'UM', '', '.', ' ', '', '', 100, 1, 'MRO')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (95, 'Mauritian rupee', 'Rs', '', '.', ' ', '', '', 1, 1, 'MUR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (96, 'Mexican peso', '$', '', '.', ' ', '', '', 100, 1, 'MXN')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (97, 'Moldovan leu', '', '', '.', ' ', '', '', 100, 1, 'MDL')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (98, 'Mongolian tugrik', '₮', '', '.', ' ', '', '', 100, 1, 'MNT')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (99, 'Moroccan dirham', '', '', '.', ' ', '', '', 100, 1, 'MAD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (100, 'Myanma kyat', 'K', '', '.', ' ', '', '', 1, 1, 'MMK')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (101, 'Namibian dollar', 'N$', '', '.', ' ', '', '', 100, 1, 'NAD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (102, 'Nepalese rupee', 'NRs', '', '.', ' ', '', '', 100, 1, 'NPR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (103, 'Netherlands Antillean gulden', 'NAƒ', '', '.', ' ', '', '', 100, 1, 'ANG')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (104, 'New Zealand dollar', 'NZ$', '', '.', ' ', '', '', 100, 1, 'NZD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (105, 'Nicaraguan córdoba', 'C$', '', '.', ' ', '', '', 100, 1, 'NIO')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (106, 'Nigerian naira', '₦', '', '.', ' ', '', '', 100, 1, 'NGN')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (107, 'Norwegian krone', 'kr', '', '.', ' ', '', '', 100, 1, 'NOK')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (108, 'Omani rial', '', '', '.', ' ', '', '', 100, 1, 'OMR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (109, 'Pakistani rupee', 'Rs.', '', '.', ' ', '', '', 1, 1, 'PKR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (110, 'Panamanian balboa', 'B./', '', '.', ' ', '', '', 100, 1, 'PAB')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (111, 'Papua New Guinean kina', 'K', '', '.', ' ', '', '', 100, 1, 'PGK')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (112, 'Paraguayan guarani', '', '', '.', ' ', '', '', 1, 1, 'PYG')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (113, 'Peruvian nuevo sol', 'S/.', '', '.', ' ', '', '', 100, 1, 'PEN')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (114, 'Philippine peso', '₱', '', '.', ' ', '', '', 100, 1, 'PHP')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (115, 'Polish zloty', '', '', '.', ' ', '', '', 100, 1, 'PLN')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (116, 'Qatari riyal', 'QR', '', '.', ' ', '', '', 100, 1, 'QAR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (117, 'Romanian leu', 'L', '', '.', ' ', '', '', 100, 1, 'RON')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (118, 'Rwandan franc', 'RF', '', '.', ' ', '', '', 1, 1, 'RWF')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (119, 'São Tomé and Príncipe dobra', 'Db', '', '.', ' ', '', '', 100, 1, 'STD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (120, 'Saudi riyal', 'SR', '', '.', ' ', '', '', 100, 1, 'SAR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (121, 'Serbian dinar', 'din.', '', '.', ' ', '', '', 1, 1, 'RSD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (122, 'Seychellois rupee', 'SR', '', '.', ' ', '', '', 100, 1, 'SCR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (123, 'Sierra Leonean leone', 'Le', '', '.', ' ', '', '', 100, 1, 'SLL')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (124, 'Singapore dollar', 'S$', '', '.', ' ', '', '', 100, 1, 'SGD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (125, 'Solomon Islands dollar', 'SI$', '', '.', ' ', '', '', 100, 1, 'SBD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (126, 'Somali shilling', 'Sh.', '', '.', ' ', '', '', 1, 1, 'SOS')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (127, 'South African rand', 'R', '', '.', ' ', '', '', 100, 1, 'ZAR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (128, 'Sri Lankan rupee', 'Rs', '', '.', ' ', '', '', 100, 1, 'LKR')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (129, 'Saint Helena pound', '£', '', '.', ' ', '', '', 100, 1, 'SHP')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (130, 'Sudanese pound', '', '', '.', ' ', '', '', 100, 1, 'SDG')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (131, 'Surinamese dollar', '$', '', '.', ' ', '', '', 100, 1, 'SRD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (132, 'Swazi lilangeni', 'E', '', '.', ' ', '', '', 100, 1, 'SZL')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (133, 'Swedish krona', 'kr', '', '.', ' ', '', '', 100, 1, 'SEK')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (134, 'Swiss franc', 'Fr.', '', '.', ' ', '', '', 100, 1, 'CHF')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (135, 'Syrian pound', '', '', '.', ' ', '', '', 1, 1, 'SYP')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (136, 'New Taiwan dollar', 'NT$', '', '.', ' ', '', '', 100, 1, 'TWD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (137, 'Tajikistani somoni', '', '', '.', ' ', '', '', 100, 1, 'TJS')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (138, 'Tanzanian shilling', '', '', '.', ' ', '', '', 1, 1, 'TZS')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (139, 'Thai baht', '฿', '', '.', ' ', '', '', 100, 1, 'THB')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (140, 'Trinidad and Tobago dollar', 'TT$', '', '.', ' ', '', '', 100, 1, 'TTD')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (141, 'Tunisian dinar', 'DT', '', '.', ' ', '', '', 100, 1, 'TND')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (142, 'Turkish lira', '₺', '', '.', ' ', '', '', 100, 1, 'TRY')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (143, 'Turkmen manat', 'm', '', '.', ' ', '', '', 100, 1, 'TMT')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (144, 'Ugandan shilling', 'USh', '', '.', ' ', '', '', 1, 1, 'UGX')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (145, 'UAE dirham', '', '', '.', ' ', '', '', 100, 1, 'AED')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (146, 'Uruguayan peso', '$U', '', '.', ' ', '', '', 100, 1, 'UYU')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (147, 'Uzbekistani som', '', '', '.', ' ', '', '', 1, 1, 'UZS')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (148, 'Vanuatu vatu', 'VT', '', '.', ' ', '', '', 100, 1, 'VUV')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (149, 'Vietnamese dong', '₫', '', '.', ' ', '', '', 1, 1, 'VND')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (150, 'Samoan tala', 'WS$', '', '.', ' ', '', '', 100, 1, 'WST')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (151, 'Yemeni rial', '', '', '.', ' ', '', '', 1, 1, 'YER')");
            db->ExecuteUpdate("REPLACE INTO CURRENCYFORMATS_V1 VALUES (152, 'Venezuelan Bolívar', 'Bs.', '', '.', ',', 'bolívar', 'céntimos', 100, 1, 'VEF')");
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
