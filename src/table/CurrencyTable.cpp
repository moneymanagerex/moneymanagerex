// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-2026 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
 *
 *      CurrencyTable.cpp
 *
 *      Implementation of the interface to database table CURRENCYFORMATS_V1
 *
 *      @author [sqlite2cpp.py]
 *
 *      Revision History:
 *          AUTO GENERATED at 2026-04-11 13:27:18.900578.
 *          DO NOT EDIT!
 */
//=============================================================================

#include "_TableFactory.tpp"
#include "CurrencyTable.h"
#include "data/CurrencyData.h"

template class TableFactory<CurrencyTable, CurrencyData>;
template class mmCache<int64, CurrencyData>;

// List of column names in database table CURRENCYFORMATS_V1,
// in the order of CurrencyCol::COL_ID.
const wxArrayString CurrencyCol::s_col_name_a = {
    "CURRENCYID",
    "CURRENCYNAME",
    "PFX_SYMBOL",
    "SFX_SYMBOL",
    "DECIMAL_POINT",
    "GROUP_SEPARATOR",
    "UNIT_NAME",
    "CENT_NAME",
    "SCALE",
    "BASECONVRATE",
    "CURRENCY_SYMBOL",
    "CURRENCY_TYPE"
};

const CurrencyCol::COL_ID CurrencyCol::s_primary_id = COL_ID_CURRENCYID;
const wxString CurrencyCol::s_primary_name = s_col_name_a[COL_ID_CURRENCYID];

// convenience variables
const wxString CurrencyCol::NAME_CURRENCYID = s_col_name_a[COL_ID_CURRENCYID];
const wxString CurrencyCol::NAME_CURRENCYNAME = s_col_name_a[COL_ID_CURRENCYNAME];
const wxString CurrencyCol::NAME_PFX_SYMBOL = s_col_name_a[COL_ID_PFX_SYMBOL];
const wxString CurrencyCol::NAME_SFX_SYMBOL = s_col_name_a[COL_ID_SFX_SYMBOL];
const wxString CurrencyCol::NAME_DECIMAL_POINT = s_col_name_a[COL_ID_DECIMAL_POINT];
const wxString CurrencyCol::NAME_GROUP_SEPARATOR = s_col_name_a[COL_ID_GROUP_SEPARATOR];
const wxString CurrencyCol::NAME_UNIT_NAME = s_col_name_a[COL_ID_UNIT_NAME];
const wxString CurrencyCol::NAME_CENT_NAME = s_col_name_a[COL_ID_CENT_NAME];
const wxString CurrencyCol::NAME_SCALE = s_col_name_a[COL_ID_SCALE];
const wxString CurrencyCol::NAME_BASECONVRATE = s_col_name_a[COL_ID_BASECONVRATE];
const wxString CurrencyCol::NAME_CURRENCY_SYMBOL = s_col_name_a[COL_ID_CURRENCY_SYMBOL];
const wxString CurrencyCol::NAME_CURRENCY_TYPE = s_col_name_a[COL_ID_CURRENCY_TYPE];

CurrencyRow::CurrencyRow()
{
    CURRENCYID = -1;
    SCALE = -1;
    BASECONVRATE = 0.0;
}

// Bind a Row record to database insert statement.
void CurrencyRow::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    stmt.Bind(1, CURRENCYNAME);
    stmt.Bind(2, PFX_SYMBOL);
    stmt.Bind(3, SFX_SYMBOL);
    stmt.Bind(4, DECIMAL_POINT);
    stmt.Bind(5, GROUP_SEPARATOR);
    stmt.Bind(6, UNIT_NAME);
    stmt.Bind(7, CENT_NAME);
    stmt.Bind(8, SCALE);
    stmt.Bind(9, BASECONVRATE);
    stmt.Bind(10, CURRENCY_SYMBOL);
    stmt.Bind(11, CURRENCY_TYPE);
    stmt.Bind(12, id);
}

CurrencyRow& CurrencyRow::from_select_result(wxSQLite3ResultSet& q)
{
    CURRENCYID = q.GetInt64(0);
    CURRENCYNAME = q.GetString(1);
    PFX_SYMBOL = q.GetString(2);
    SFX_SYMBOL = q.GetString(3);
    DECIMAL_POINT = q.GetString(4);
    GROUP_SEPARATOR = q.GetString(5);
    UNIT_NAME = q.GetString(6);
    CENT_NAME = q.GetString(7);
    SCALE = q.GetInt64(8);
    BASECONVRATE = q.GetDouble(9);
    CURRENCY_SYMBOL = q.GetString(10);
    CURRENCY_TYPE = q.GetString(11);

    return *this;
}

// Return the data record as a json string
wxString CurrencyRow::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void CurrencyRow::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("CURRENCYID");
    json_writer.Int64(CURRENCYID.GetValue());

    json_writer.Key("CURRENCYNAME");
    json_writer.String(CURRENCYNAME.utf8_str());

    json_writer.Key("PFX_SYMBOL");
    json_writer.String(PFX_SYMBOL.utf8_str());

    json_writer.Key("SFX_SYMBOL");
    json_writer.String(SFX_SYMBOL.utf8_str());

    json_writer.Key("DECIMAL_POINT");
    json_writer.String(DECIMAL_POINT.utf8_str());

    json_writer.Key("GROUP_SEPARATOR");
    json_writer.String(GROUP_SEPARATOR.utf8_str());

    json_writer.Key("UNIT_NAME");
    json_writer.String(UNIT_NAME.utf8_str());

    json_writer.Key("CENT_NAME");
    json_writer.String(CENT_NAME.utf8_str());

    json_writer.Key("SCALE");
    json_writer.Int64(SCALE.GetValue());

    json_writer.Key("BASECONVRATE");
    json_writer.Double(BASECONVRATE);

    json_writer.Key("CURRENCY_SYMBOL");
    json_writer.String(CURRENCY_SYMBOL.utf8_str());

    json_writer.Key("CURRENCY_TYPE");
    json_writer.String(CURRENCY_TYPE.utf8_str());
}

row_t CurrencyRow::to_html_row() const
{
    row_t row;

    row(L"CURRENCYID") = CURRENCYID.GetValue();
    row(L"CURRENCYNAME") = CURRENCYNAME;
    row(L"PFX_SYMBOL") = PFX_SYMBOL;
    row(L"SFX_SYMBOL") = SFX_SYMBOL;
    row(L"DECIMAL_POINT") = DECIMAL_POINT;
    row(L"GROUP_SEPARATOR") = GROUP_SEPARATOR;
    row(L"UNIT_NAME") = UNIT_NAME;
    row(L"CENT_NAME") = CENT_NAME;
    row(L"SCALE") = SCALE.GetValue();
    row(L"BASECONVRATE") = BASECONVRATE;
    row(L"CURRENCY_SYMBOL") = CURRENCY_SYMBOL;
    row(L"CURRENCY_TYPE") = CURRENCY_TYPE;

    return row;
}

void CurrencyRow::to_html_template(html_template& t) const
{
    t(L"CURRENCYID") = CURRENCYID.GetValue();
    t(L"CURRENCYNAME") = CURRENCYNAME;
    t(L"PFX_SYMBOL") = PFX_SYMBOL;
    t(L"SFX_SYMBOL") = SFX_SYMBOL;
    t(L"DECIMAL_POINT") = DECIMAL_POINT;
    t(L"GROUP_SEPARATOR") = GROUP_SEPARATOR;
    t(L"UNIT_NAME") = UNIT_NAME;
    t(L"CENT_NAME") = CENT_NAME;
    t(L"SCALE") = SCALE.GetValue();
    t(L"BASECONVRATE") = BASECONVRATE;
    t(L"CURRENCY_SYMBOL") = CURRENCY_SYMBOL;
    t(L"CURRENCY_TYPE") = CURRENCY_TYPE;
}

bool CurrencyRow::equals(const CurrencyRow* other) const
{
    if ( CURRENCYID != other->CURRENCYID) return false;
    if (!CURRENCYNAME.IsSameAs(other->CURRENCYNAME)) return false;
    if (!PFX_SYMBOL.IsSameAs(other->PFX_SYMBOL)) return false;
    if (!SFX_SYMBOL.IsSameAs(other->SFX_SYMBOL)) return false;
    if (!DECIMAL_POINT.IsSameAs(other->DECIMAL_POINT)) return false;
    if (!GROUP_SEPARATOR.IsSameAs(other->GROUP_SEPARATOR)) return false;
    if (!UNIT_NAME.IsSameAs(other->UNIT_NAME)) return false;
    if (!CENT_NAME.IsSameAs(other->CENT_NAME)) return false;
    if ( SCALE != other->SCALE) return false;
    if ( BASECONVRATE != other->BASECONVRATE) return false;
    if (!CURRENCY_SYMBOL.IsSameAs(other->CURRENCY_SYMBOL)) return false;
    if (!CURRENCY_TYPE.IsSameAs(other->CURRENCY_TYPE)) return false;

    return true;
}

CurrencyTable::CurrencyTable()
{
    m_table_name = "CURRENCYFORMATS_V1";

    m_create_query = "CREATE TABLE CURRENCYFORMATS_V1(CURRENCYID integer primary key, CURRENCYNAME TEXT COLLATE NOCASE NOT NULL UNIQUE, PFX_SYMBOL TEXT, SFX_SYMBOL TEXT, DECIMAL_POINT TEXT, GROUP_SEPARATOR TEXT, UNIT_NAME TEXT COLLATE NOCASE, CENT_NAME TEXT COLLATE NOCASE, SCALE integer, BASECONVRATE numeric, CURRENCY_SYMBOL TEXT COLLATE NOCASE NOT NULL UNIQUE, CURRENCY_TYPE TEXT NOT NULL /* Fiat, Crypto */)";

    m_drop_query = "DROP TABLE IF EXISTS CURRENCYFORMATS_V1";

    m_index_query_a = {
        "CREATE INDEX IF NOT EXISTS IDX_CURRENCYFORMATS_SYMBOL ON CURRENCYFORMATS_V1(CURRENCY_SYMBOL)"
    };

    m_insert_query = "INSERT INTO CURRENCYFORMATS_V1(CURRENCYNAME, PFX_SYMBOL, SFX_SYMBOL, DECIMAL_POINT, GROUP_SEPARATOR, UNIT_NAME, CENT_NAME, SCALE, BASECONVRATE, CURRENCY_SYMBOL, CURRENCY_TYPE, CURRENCYID) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    m_update_query = "UPDATE CURRENCYFORMATS_V1 SET CURRENCYNAME = ?, PFX_SYMBOL = ?, SFX_SYMBOL = ?, DECIMAL_POINT = ?, GROUP_SEPARATOR = ?, UNIT_NAME = ?, CENT_NAME = ?, SCALE = ?, BASECONVRATE = ?, CURRENCY_SYMBOL = ?, CURRENCY_TYPE = ? WHERE CURRENCYID = ?";

    m_delete_query = "DELETE FROM CURRENCYFORMATS_V1 WHERE CURRENCYID = ?";

    m_select_query = "SELECT CURRENCYID, CURRENCYNAME, PFX_SYMBOL, SFX_SYMBOL, DECIMAL_POINT, GROUP_SEPARATOR, UNIT_NAME, CENT_NAME, SCALE, BASECONVRATE, CURRENCY_SYMBOL, CURRENCY_TYPE FROM CURRENCYFORMATS_V1";
}

void CurrencyTable::ensure_data()
{
    m_db->Begin();
    try {
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('1', '%s', '$', '', '.', ',', 'Dollar', 'Cent', '100', '1', 'USD', 'Fiat')", _("US dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('2', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'EUR', 'Fiat')", _("Euro"), L"€"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('3', '%s', '%s', '', '.', ' ', 'Pound', 'Pence', '100', '1', 'GBP', 'Fiat')", _("British pound"), L"£"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('4', '%s', '', '%s', ',', ' ', '%s', '%s', '100', '1', 'RUB', 'Fiat')", _("Russian ruble"), L"р", L"руб.", L"коп."));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('5', '%s', '%s', '', ',', ' ', '', '', '100', '1', 'UAH', 'Fiat')", _("Ukrainian hryvnia"), L"₴"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('6', '%s', '%s', '', '.', ' ', '', 'pul', '100', '1', 'AFN', 'Fiat')", _("Afghan afghani"), L"؋"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('7', '%s', '', 'L', '.', ' ', '', '', '1', '1', 'ALL', 'Fiat')", _("Albanian lek")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('8', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'DZD', 'Fiat')", _("Algerian dinar"), L"دج"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('9', '%s', '', 'Kz', '.', ' ', '', '%s', '100', '1', 'AOA', 'Fiat')", _("Angolan kwanza"), L"Céntimo"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('10', '%s', 'EC$', '', '.', ' ', '', '', '100', '1', 'XCD', 'Fiat')", _("East Caribbean dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('11', '%s', 'AR$', '', ',', '.', '', 'centavo', '100', '1', 'ARS', 'Fiat')", _("Argentine peso")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('12', '%s', '', '', '.', ' ', '', '', '1', '1', 'AMD', 'Fiat')", _("Armenian dram")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('13', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'AWG', 'Fiat')", _("Aruban florin"), L"ƒ"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('14', '%s', '$', '', '.', ',', '', '', '100', '1', 'AUD', 'Fiat')", _("Australian dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('15', '%s', '', '', '.', ' ', '', '', '100', '1', 'AZN', 'Fiat')", _("Azerbaijani manat")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('16', '%s', 'B$', '', '.', ' ', '', '', '100', '1', 'BSD', 'Fiat')", _("Bahamian dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('17', '%s', '', '', '.', ' ', '', '', '100', '1', 'BHD', 'Fiat')", _("Bahraini dinar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('18', '%s', '', '', '.', ' ', '', '', '100', '1', 'BDT', 'Fiat')", _("Bangladeshi taka")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('19', '%s', 'Bds$', '', '.', ' ', '', '', '100', '1', 'BBD', 'Fiat')", _("Barbadian dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('20', '%s', 'Br', '', ',', ' ', '', '', '1', '1', 'BYR', 'Fiat')", _("Belarusian ruble (2000-2016)")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('21', '%s', 'BZ$', '', '.', ' ', '', '', '100', '1', 'BZD', 'Fiat')", _("Belize dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('22', '%s', 'CFA', '', '.', ' ', '', '', '100', '1', 'XOF', 'Fiat')", _("West African CFA franc")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('23', '%s', 'BD$', '', '.', ' ', '', '', '100', '1', 'BMD', 'Fiat')", _("Bermudan dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('24', '%s', 'Nu.', '', '.', ' ', '', '', '100', '1', 'BTN', 'Fiat')", _("Bhutanese ngultrum")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('25', '%s', 'Bs.', '', '.', ' ', '', '', '100', '1', 'BOB', 'Fiat')", _("Bolivian boliviano")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('26', '%s', 'KM', '', ',', '.', '', '', '100', '1', 'BAM', 'Fiat')", _("Bosnia-Herzegovina convertible mark")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('27', '%s', 'P', '', '.', ' ', '', '', '100', '1', 'BWP', 'Fiat')", _("Botswanan pula")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('28', '%s', 'R$', '', '.', ' ', '', '', '100', '1', 'BRL', 'Fiat')", _("Brazilian real")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('29', '%s', 'B$', '', '.', ' ', '', '', '100', '1', 'BND', 'Fiat')", _("Brunei dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('30', '%s', '', '', '.', ' ', '', '', '100', '1', 'BGN', 'Fiat')", _("Bulgarian lev")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('31', '%s', 'FBu', '', '.', ' ', '', '', '1', '1', 'BIF', 'Fiat')", _("Burundian franc")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('32', '%s', '', '', '.', ' ', '', '', '100', '1', 'KHR', 'Fiat')", _("Cambodian riel")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('33', '%s', 'CFA', '', '.', ' ', '', '', '1', '1', 'XAF', 'Fiat')", _("Central African CFA franc")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('34', '%s', '$', '', '.', ' ', '', '', '100', '1', 'CAD', 'Fiat')", _("Canadian dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('35', '%s', 'Esc', '', '.', ' ', '', '', '100', '1', 'CVE', 'Fiat')", _("Cape Verdean escudo")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('36', '%s', 'KY$', '', '.', ' ', '', '', '100', '1', 'KYD', 'Fiat')", _("Cayman Islands dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('37', '%s', '$', '', '.', ' ', '', '', '1', '1', 'CLP', 'Fiat')", _("Chilean peso")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('38', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'CNY', 'Fiat')", _("Chinese yuan"), L"¥"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('39', '%s', 'Col$', '', '.', ' ', '', '', '100', '1', 'COP', 'Fiat')", _("Colombian peso")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('40', '%s', '', '', '.', ' ', '', '', '1', '1', 'KMF', 'Fiat')", _("Comorian franc")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('41', '%s', 'F', '', '.', ' ', '', '', '100', '1', 'CDF', 'Fiat')", _("Congolese franc")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('42', '%s', '%s', '', '.', ' ', '', '', '1', '1', 'CRC', 'Fiat')", L"Costa Rican colón", L"₡"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('43', '%s', 'kn', '', '.', ' ', '', '', '100', '1', 'HRK', 'Fiat')", _("Croatian kuna")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('44', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'CZK', 'Fiat')", _("Czech koruna"), L"Kč"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('45', '%s', 'Kr', '', '.', ' ', '', '', '100', '1', 'DKK', 'Fiat')", _("Danish krone")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('46', '%s', 'Fdj', '', '.', ' ', '', '', '1', '1', 'DJF', 'Fiat')", _("Djiboutian franc")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('47', '%s', 'RD$', '', '.', ' ', '', '', '100', '1', 'DOP', 'Fiat')", _("Dominican peso")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('48', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'EGP', 'Fiat')", _("Egyptian pound"), L"£"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('49', '%s', 'Nfa', '', '.', ' ', '', '', '100', '1', 'ERN', 'Fiat')", _("Eritrean nakfa")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('50', '%s', 'Br', '', '.', ' ', '', '', '100', '1', 'ETB', 'Fiat')", _("Ethiopian birr")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('51', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'FKP', 'Fiat')", _("Falkland Islands pound"), L"£"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('52', '%s', 'FJ$', '', '.', ' ', '', '', '100', '1', 'FJD', 'Fiat')", _("Fijian dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('53', '%s', 'F', '', '.', ' ', '', '', '100', '1', 'XPF', 'Fiat')", _("CFP franc")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('54', '%s', 'D', '', '.', ' ', '', '', '100', '1', 'GMD', 'Fiat')", _("Gambian dalasi")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('55', '%s', '', '', '.', ' ', '', '', '100', '1', 'GEL', 'Fiat')", _("Georgian lari")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('56', '%s', '', '', '.', ' ', '', '', '100', '1', 'GHS', 'Fiat')", _("Ghanaian cedi")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('57', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'GIP', 'Fiat')", _("Gibraltar pound"), L"£"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('58', '%s', 'Q', '', '.', ' ', '', '', '100', '1', 'GTQ', 'Fiat')", _("Guatemalan quetzal")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('59', '%s', 'FG', '', '.', ' ', '', '', '1', '1', 'GNF', 'Fiat')", _("Guinean franc")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('60', '%s', 'GY$', '', '.', ' ', '', '', '100', '1', 'GYD', 'Fiat')", _("Guyanaese dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('61', '%s', 'G', '', '.', ' ', '', '', '100', '1', 'HTG', 'Fiat')", _("Haitian gourde")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('62', '%s', 'L', '', '.', ' ', '', '', '100', '1', 'HNL', 'Fiat')", _("Honduran lempira")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('63', '%s', 'HK$', '', '.', ' ', '', '', '100', '1', 'HKD', 'Fiat')", _("Hong Kong dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('64', '%s', '', 'Ft', '.', ' ', '', '', '1', '1', 'HUF', 'Fiat')", _("Hungarian forint")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('65', '%s', 'kr', '', '.', ' ', '', '', '1', '1', 'ISK', 'Fiat')", L"Icelandic króna"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('66', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'INR', 'Fiat')", _("Indian rupee"), L"₹"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('67', '%s', 'Rp', '', '.', ' ', '', '', '1', '1', 'IDR', 'Fiat')", _("Indonesian rupiah")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('68', '%s', 'SDR', '', '.', ' ', '', '', '100', '1', 'XDR', 'Fiat')", _("Special drawing rights")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('69', '%s', '', '', '.', ' ', '', '', '1', '1', 'IRR', 'Fiat')", _("Iranian rial")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('70', '%s', '', '', '.', ' ', '', '', '1', '1', 'IQD', 'Fiat')", _("Iraqi dinar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('71', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'ILS', 'Fiat')", _("Israeli new shekel"), L"₪"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('72', '%s', 'J$', '', '.', ' ', '', '', '100', '1', 'JMD', 'Fiat')", _("Jamaican dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('73', '%s', '%s', '', '.', ' ', '', '', '1', '1', 'JPY', 'Fiat')", _("Japanese yen"), L"¥"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('74', '%s', '', '', '.', ' ', '', '', '100', '1', 'JOD', 'Fiat')", _("Jordanian dinar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('75', '%s', 'T', '', '.', ' ', '', '', '100', '1', 'KZT', 'Fiat')", _("Kazakhstani tenge")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('76', '%s', 'KSh', '', '.', ' ', '', '', '100', '1', 'KES', 'Fiat')", _("Kenyan shilling")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('77', '%s', 'W', '', '.', ' ', '', '', '100', '1', 'KPW', 'Fiat')", _("North Korean won")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('78', '%s', 'W', '', '.', ' ', '', '', '1', '1', 'KRW', 'Fiat')", _("South Korean won")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('79', '%s', '', '', '.', ' ', '', '', '100', '1', 'KWD', 'Fiat')", _("Kuwaiti dinar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('80', '%s', '', '', '.', ' ', '', '', '100', '1', 'KGS', 'Fiat')", _("Kyrgystani som")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('81', '%s', 'KN', '', '.', ' ', '', '', '100', '1', 'LAK', 'Fiat')", _("Laotian kip")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('82', '%s', 'Ls', '', '.', ' ', '', '', '100', '1', 'LVL', 'Fiat')", _("Latvian lats")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('83', '%s', '', '', '.', ' ', '', '', '1', '1', 'LBP', 'Fiat')", _("Lebanese pound")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('84', '%s', 'M', '', '.', ' ', '', '', '100', '1', 'LSL', 'Fiat')", _("Lesotho loti")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('85', '%s', 'L$', '', '.', ' ', '', '', '100', '1', 'LRD', 'Fiat')", _("Liberian dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('86', '%s', 'LD', '', '.', ' ', '', '', '100', '1', 'LYD', 'Fiat')", _("Libyan dinar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('87', '%s', 'Lt', '', '.', ' ', '', '', '100', '1', 'LTL', 'Fiat')", _("Lithuanian litas")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('88', '%s', 'P', '', '.', ' ', '', '', '100', '1', 'MOP', 'Fiat')", _("Macanese pataca")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('89', '%s', '', '', '.', ' ', '', '', '100', '1', 'MKD', 'Fiat')", _("Macedonian denar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('90', '%s', 'FMG', '', '.', ' ', '', '', '100', '1', 'MGA', 'Fiat')", _("Malagasy ariary")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('91', '%s', 'MK', '', '.', ' ', '', '', '1', '1', 'MWK', 'Fiat')", _("Malawian kwacha")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('92', '%s', 'RM', '', '.', ' ', '', '', '100', '1', 'MYR', 'Fiat')", _("Malaysian ringgit")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('93', '%s', 'Rf', '', '.', ' ', '', '', '100', '1', 'MVR', 'Fiat')", _("Maldivian rufiyaa")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('94', '%s', 'UM', '', '.', ' ', '', '', '100', '1', 'MRO', 'Fiat')", _("Mauritanian ouguiya (1973-2017)")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('95', '%s', 'Rs', '', '.', ' ', '', '', '1', '1', 'MUR', 'Fiat')", _("Mauritian rupee")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('96', '%s', '$', '', '.', ' ', '', '', '100', '1', 'MXN', 'Fiat')", _("Mexican peso")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('97', '%s', '', '', '.', ' ', '', '', '100', '1', 'MDL', 'Fiat')", _("Moldovan leu")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('98', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'MNT', 'Fiat')", _("Mongolian tugrik"), L"₮"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('99', '%s', '', '', '.', ' ', '', '', '100', '1', 'MAD', 'Fiat')", _("Moroccan dirham")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('100', '%s', 'K', '', '.', ' ', '', '', '1', '1', 'MMK', 'Fiat')", _("Myanmar kyat")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('101', '%s', 'N$', '', '.', ' ', '', '', '100', '1', 'NAD', 'Fiat')", _("Namibian dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('102', '%s', 'NRs', '', '.', ' ', '', '', '100', '1', 'NPR', 'Fiat')", _("Nepalese rupee")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('103', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'ANG', 'Fiat')", _("Netherlands Antillean guilder"), L"NAƒ"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('104', '%s', 'NZ$', '', '.', ' ', '', '', '100', '1', 'NZD', 'Fiat')", _("New Zealand dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('105', '%s', 'C$', '', '.', ' ', '', '', '100', '1', 'NIO', 'Fiat')", L"Nicaraguan córdoba"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('106', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'NGN', 'Fiat')", _("Nigerian naira"), L"₦"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('107', '%s', 'kr', '', '.', ' ', '', '', '100', '1', 'NOK', 'Fiat')", _("Norwegian krone")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('108', '%s', '', '', '.', ' ', '', '', '100', '1', 'OMR', 'Fiat')", _("Omani rial")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('109', '%s', 'Rs.', '', '.', ' ', '', '', '1', '1', 'PKR', 'Fiat')", _("Pakistani rupee")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('110', '%s', 'B./', '', '.', ' ', '', '', '100', '1', 'PAB', 'Fiat')", _("Panamanian balboa")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('111', '%s', 'K', '', '.', ' ', '', '', '100', '1', 'PGK', 'Fiat')", _("Papua New Guinean kina")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('112', '%s', '', '', '.', ' ', '', '', '1', '1', 'PYG', 'Fiat')", _("Paraguayan guarani")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('113', '%s', 'S/.', '', '.', ' ', '', '', '100', '1', 'PEN', 'Fiat')", _("Peruvian sol")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('114', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'PHP', 'Fiat')", _("Philippine peso"), L"₱"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('115', '%s', '', '%s', ',', '.', '%s', 'grosz', '100', '1', 'PLN', 'Fiat')", _("Polish zloty"), L"zł", L"złoty"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('116', '%s', 'QR', '', '.', ' ', '', '', '100', '1', 'QAR', 'Fiat')", _("Qatari riyal")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('117', '%s', 'L', '', '.', ' ', '', '', '100', '1', 'RON', 'Fiat')", _("Romanian leu")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('118', '%s', 'RF', '', '.', ' ', '', '', '1', '1', 'RWF', 'Fiat')", _("Rwandan franc")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('119', '%s', 'Db', '', '.', ' ', '', '', '100', '1', 'STD', 'Fiat')", L"São Tomé & Príncipe dobra (1977-2017)"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('120', '%s', 'SR', '', '.', ' ', '', '', '100', '1', 'SAR', 'Fiat')", _("Saudi riyal")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('121', '%s', 'din.', '', '.', ' ', '', '', '1', '1', 'RSD', 'Fiat')", _("Serbian dinar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('122', '%s', 'SR', '', '.', ' ', '', '', '100', '1', 'SCR', 'Fiat')", _("Seychellois rupee")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('123', '%s', 'Le', '', '.', ' ', '', '', '100', '1', 'SLL', 'Fiat')", _("Sierra Leonean leone (1964-2022)")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('124', '%s', 'S$', '', '.', ' ', '', '', '100', '1', 'SGD', 'Fiat')", _("Singapore dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('125', '%s', 'SI$', '', '.', ' ', '', '', '100', '1', 'SBD', 'Fiat')", _("Solomon Islands dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('126', '%s', 'Sh.', '', '.', ' ', '', '', '1', '1', 'SOS', 'Fiat')", _("Somali shilling")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('127', '%s', 'R', '', '.', ' ', '', '', '100', '1', 'ZAR', 'Fiat')", _("South African rand")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('128', '%s', 'Rs', '', '.', ' ', '', '', '100', '1', 'LKR', 'Fiat')", _("Sri Lankan rupee")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('129', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'SHP', 'Fiat')", _("St. Helena pound"), L"£"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('130', '%s', '', '', '.', ' ', '', '', '100', '1', 'SDG', 'Fiat')", _("Sudanese pound")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('131', '%s', '$', '', '.', ' ', '', '', '100', '1', 'SRD', 'Fiat')", _("Surinamese dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('132', '%s', 'E', '', '.', ' ', '', '', '100', '1', 'SZL', 'Fiat')", _("Swazi lilangeni")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('133', '%s', 'kr', '', '.', ' ', '', '', '100', '1', 'SEK', 'Fiat')", _("Swedish krona")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('134', '%s', 'Fr.', '', '.', ' ', '', '', '100', '1', 'CHF', 'Fiat')", _("Swiss franc")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('135', '%s', '', '', '.', ' ', '', '', '1', '1', 'SYP', 'Fiat')", _("Syrian pound")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('136', '%s', 'NT$', '', '.', ' ', '', '', '100', '1', 'TWD', 'Fiat')", _("New Taiwan dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('137', '%s', '', '', '.', ' ', '', '', '100', '1', 'TJS', 'Fiat')", _("Tajikistani somoni")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('138', '%s', '', '', '.', ' ', '', '', '1', '1', 'TZS', 'Fiat')", _("Tanzanian shilling")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('139', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'THB', 'Fiat')", _("Thai baht"), L"฿"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('140', '%s', 'TT$', '', '.', ' ', '', '', '100', '1', 'TTD', 'Fiat')", _("Trinidad & Tobago dollar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('141', '%s', 'DT', '', '.', ' ', '', '', '100', '1', 'TND', 'Fiat')", _("Tunisian dinar")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('142', '%s', '%s', '', '.', ' ', '', '', '100', '1', 'TRY', 'Fiat')", _("Turkish lira"), L"₺"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('143', '%s', 'm', '', '.', ' ', '', '', '100', '1', 'TMT', 'Fiat')", _("Turkmenistani manat")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('144', '%s', 'USh', '', '.', ' ', '', '', '1', '1', 'UGX', 'Fiat')", _("Ugandan shilling")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('145', '%s', '', '', '.', ' ', '', '', '100', '1', 'AED', 'Fiat')", _("UAE dirham")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('146', '%s', '$U', '', '.', ' ', '', '', '100', '1', 'UYU', 'Fiat')", _("Uruguayan peso")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('147', '%s', '', '', '.', ' ', '', '', '1', '1', 'UZS', 'Fiat')", _("Uzbekistani som")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('148', '%s', 'VT', '', '.', ' ', '', '', '100', '1', 'VUV', 'Fiat')", _("Vanuatu vatu")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('149', '%s', '%s', '', '.', ' ', '', '', '1', '1', 'VND', 'Fiat')", _("Vietnamese dong"), L"₫"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('150', '%s', 'WS$', '', '.', ' ', '', '', '100', '1', 'WST', 'Fiat')", _("Samoan tala")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('151', '%s', '', '', '.', ' ', '', '', '1', '1', 'YER', 'Fiat')", _("Yemeni rial")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('152', '%s', 'Bs.', '', '.', ',', '%s', '%s', '100', '1', 'VEF', 'Fiat')", L"Venezuelan bolívar (2008-2018)", L"bolívar", L"céntimos"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('153', '%s', '%s', '', '.', ',', '', '', '100000000', '1', 'BTC', 'Crypto')", _("Bitcoin"), L"Ƀ"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('154', '%s', 'BYN', '', '.', ',', '', '', '100', '1', 'BYN', 'Fiat')", _("Belarusian ruble")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('155', '%s', '$', '', '.', ',', '', '', '100', '1', 'CUC', 'Fiat')", _("Cuban convertible peso")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('156', '%s', '$', '', '.', ',', '', '', '100', '1', 'CUP', 'Fiat')", _("Cuban peso")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('157', '%s', 'MRU', '', '.', ',', '', '', '100', '1', 'MRU', 'Fiat')", _("Mauritanian ouguiya")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('158', '%s', 'MZN', '', '.', ',', '', '', '100', '1', 'MZN', 'Fiat')", _("Mozambican metical")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('159', '%s', 'SLE', '', '.', ',', '', '', '100', '1', 'SLE', 'Fiat')", _("Sierra Leonean leone")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('160', '%s', '%s', '', '.', ',', '', '', '100', '1', 'SSP', 'Fiat')", _("South Sudanese pound"), L"£"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('161', '%s', 'Db', '', '.', ',', '', '', '100', '1', 'STN', 'Fiat')", L"São Tomé & Príncipe dobra"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('162', '%s', 'SVC', '', '.', ',', '', '', '100', '1', 'SVC', 'Fiat')", L"Salvadoran colón"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('163', '%s', 'T$', '', '.', ',', '', '', '100', '1', 'TOP', 'Fiat')", L"Tongan paʻanga"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('164', '%s', 'UYW', '', '.', ',', '', '', '10000', '1', 'UYW', 'Fiat')", _("Uruguayan nominal wage index unit")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('165', '%s', 'VED', '', '.', ',', '', '', '100', '1', 'VED', 'Fiat')", L"Bolívar soberano"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('166', '%s', 'VES', '', '.', ',', '', '', '100', '1', 'VES', 'Fiat')", L"Venezuelan bolívar"));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('167', '%s', 'ZK', '', '.', ',', '', '', '100', '1', 'ZMW', 'Fiat')", _("Zambian kwacha")));
        m_db->ExecuteUpdate(wxString::Format("INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES ('168', '%s', 'ZWL', '', '.', ',', '', '', '100', '1', 'ZWL', 'Fiat')", _("Zimbabwean dollar (2009)")));
    }
    catch (wxSQLite3Exception& e) {
      wxLogError("SQL Exception during creation of currency table, currency list may not be complete\n\nError '%s'", e.GetMessage());
    }
    m_db->Commit();
}
