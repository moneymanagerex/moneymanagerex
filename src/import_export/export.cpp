/*******************************************************
Copyright (C) 2013 Nikolay

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

#include "export.h"
#include "constants.h"
#include "paths.h"
#include "util.h"
#include "model/Model_Account.h"
#include "model/Model_Attachment.h"
#include "model/Model_Category.h"
#include "model/Model_Checking.h"
#include "model/Model_Currency.h"
#include "model/Model_CustomField.h"
#include "model/Model_Payee.h"
#include "model/Model_CustomFieldData.h"
#include "model/Model_CustomField.h"


mmExportTransaction::mmExportTransaction()
{}

mmExportTransaction::~mmExportTransaction()
{}

const wxString mmExportTransaction::getTransactionCSV(const Model_Checking::Full_Data& full_tran
    , const wxString& dateMask, bool reverce)
{
    wxString buffer = "";
    bool is_transfer = Model_Checking::is_transfer(full_tran.TRANSCODE);
    const wxString delimiter = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);

    wxString categ = full_tran.m_splits.empty() ? Model_Category::full_name(full_tran.CATEGID, ":") : "";
    wxString transNum = full_tran.TRANSACTIONNUMBER;
    wxString notes = (full_tran.NOTES);
    wxString payee = full_tran.PAYEENAME;

    const auto acc_in = Model_Account::instance().get(full_tran.ACCOUNTID);
    const auto curr_in = Model_Currency::instance().get(acc_in->CURRENCYID);
    wxString account = acc_in->ACCOUNTNAME;
    wxString currency = curr_in->CURRENCY_SYMBOL;

    if (is_transfer)
    {
        const auto acc_to = Model_Account::instance().get(full_tran.TOACCOUNTID);
        const auto curr_to = Model_Currency::instance().get(acc_to->CURRENCYID);

        payee = reverce ? acc_to->ACCOUNTNAME : acc_in->ACCOUNTNAME;
        account = reverce ? acc_in->ACCOUNTNAME : acc_to->ACCOUNTNAME;
        currency = reverce ? curr_in->CURRENCY_SYMBOL : curr_to->CURRENCY_SYMBOL;

        //Transaction number used to make transaction unique
        // to proper merge transfer records
        if (transNum.IsEmpty() && notes.IsEmpty()) {
            transNum = wxString::Format("#%i", full_tran.id());
        }
    }

    if (full_tran.has_split())
    {
        for (const auto &split_entry : full_tran.m_splits)
        {
            double valueSplit = split_entry.SPLITTRANSAMOUNT;
            if (Model_Checking::type_id(full_tran) == Model_Checking::TYPE_ID_WITHDRAWAL)
                valueSplit = -valueSplit;
            const wxString split_amount = wxString::FromCDouble(valueSplit, 2);
            const wxString split_categ = Model_Category::full_name(split_entry.CATEGID, ":");

            buffer << inQuotes(wxString::Format("%i", full_tran.TRANSID), delimiter) << delimiter;
            buffer << inQuotes(mmGetDateForDisplay(full_tran.TRANSDATE, dateMask), delimiter) << delimiter;
            buffer << inQuotes(full_tran.STATUS, delimiter) << delimiter;
            buffer << inQuotes(full_tran.TRANSCODE, delimiter) << delimiter;

            buffer << inQuotes(acc_in->ACCOUNTNAME, delimiter) << delimiter;

            buffer << inQuotes(payee, delimiter) << delimiter;
            buffer << inQuotes(split_categ, delimiter) << delimiter;

            buffer << inQuotes(split_amount, delimiter) << delimiter;
            buffer << inQuotes(curr_in->CURRENCY_SYMBOL, delimiter) << delimiter;
            buffer << inQuotes(transNum, delimiter) << delimiter;
            buffer << inQuotes(notes, delimiter);

            buffer << "\n";
        }
    }
    else
    {
        buffer << inQuotes(wxString::Format("%i", full_tran.TRANSID), delimiter) << delimiter;
        buffer << inQuotes(mmGetDateForDisplay(full_tran.TRANSDATE, dateMask), delimiter) << delimiter;
        buffer << inQuotes(full_tran.STATUS, delimiter) << delimiter;
        buffer << inQuotes(full_tran.TRANSCODE, delimiter) << delimiter;

        buffer << inQuotes(account, delimiter) << delimiter;

        buffer << inQuotes(payee, delimiter) << delimiter;
        buffer << inQuotes(categ, delimiter) << delimiter;
        double value = Model_Checking::balance(full_tran
            , (reverce ? full_tran.ACCOUNTID : full_tran.TOACCOUNTID));
        const wxString& s = wxString::FromCDouble(value, 2);
        buffer << inQuotes(s, delimiter) << delimiter;
        buffer << inQuotes(currency, delimiter) << delimiter;
        buffer << inQuotes(transNum, delimiter) << delimiter;
        buffer << inQuotes(notes, delimiter);

        buffer << "\n";
    }

    return buffer;
}

const wxString mmExportTransaction::getTransactionQIF(const Model_Checking::Full_Data& full_tran
    , const wxString& dateMask, bool reverce)
{
    bool transfer = Model_Checking::is_transfer(full_tran.TRANSCODE);

    wxString buffer = "";
    wxString categ = full_tran.m_splits.empty() ? Model_Category::full_name(full_tran.CATEGID, ":") : "";
    // Replace square brackets which are used to denote transfers in QIF
    categ.Replace("[", "(");
    categ.Replace("]", ")");
    wxString transNum = full_tran.TRANSACTIONNUMBER;
    wxString notes = (full_tran.NOTES);
    wxString payee = full_tran.PAYEENAME;

    if (transfer)
    {
        const auto acc_in = Model_Account::instance().get(full_tran.ACCOUNTID);
        const auto acc_to = Model_Account::instance().get(full_tran.TOACCOUNTID);
        const auto curr_in = Model_Currency::instance().get(acc_in->CURRENCYID);
        const auto curr_to = Model_Currency::instance().get(acc_to->CURRENCYID);

        categ = "[" + (reverce ? full_tran.ACCOUNTNAME : full_tran.TOACCOUNTNAME) + "]";
        payee = wxString::Format("%s %s %s -> %s %s %s"
            , wxString::FromCDouble(full_tran.TRANSAMOUNT, 2), curr_in->CURRENCY_SYMBOL, acc_in->ACCOUNTNAME
            , wxString::FromCDouble(full_tran.TOTRANSAMOUNT, 2), curr_to->CURRENCY_SYMBOL, acc_to->ACCOUNTNAME);
        //Transaction number used to make transaction unique
        // to proper merge transfer records
        if (transNum.IsEmpty() && notes.IsEmpty())
            transNum = wxString::Format("#%i", full_tran.id());
    }

    // don't allow '/' in category name as it is reserved for the class/tag separator
    categ.Replace("/", "-");
    if (!full_tran.m_tags.empty())
    {
        categ.Append("/");
        auto numTags = full_tran.m_tags.size();
        for (decltype(numTags) i = 0; i < numTags; i++)
            categ.Append((i > 0 ? ":" : "") + Model_Tag::instance().get(full_tran.m_tags[i].TAGID)->TAGNAME);
    }

    buffer << "D" << mmGetDateForDisplay(full_tran.TRANSDATE, dateMask) << "\n";
    buffer << "C" << (full_tran.STATUS == Model_Checking::STATUS_KEY_RECONCILED ? "R" : "") << "\n";
    double value = Model_Checking::balance(full_tran
        , (reverce ? full_tran.TOACCOUNTID : full_tran.ACCOUNTID));
    const wxString& s = wxString::FromCDouble(value, 2);
    buffer << "T" << s << "\n";
    if (!payee.empty())
        buffer << "P" << payee << "\n";
    if (!transNum.IsEmpty())
        buffer << "N" << transNum << "\n";
    if (!categ.IsEmpty())
        buffer << "L" << categ << "\n";
    if (!notes.IsEmpty())
    {
        notes.Replace("''", "'");
        notes.Replace("\n", "\nM");
        buffer << "M" << notes << "\n";
    }

    wxString reftype = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTIONSPLIT);
    for (const auto &split_entry : full_tran.m_splits)
    {
        double valueSplit = split_entry.SPLITTRANSAMOUNT;
        if (Model_Checking::type_id(full_tran) == Model_Checking::TYPE_ID_WITHDRAWAL)
            valueSplit = -valueSplit;
        const wxString split_amount = wxString::FromCDouble(valueSplit, 2);
        wxString split_categ = Model_Category::full_name(split_entry.CATEGID, ":");
        split_categ.Replace("/", "-");
        Model_Taglink::Data_Set splitTags = Model_Taglink::instance().find(Model_Taglink::REFTYPE(reftype), Model_Taglink::REFID(split_entry.SPLITTRANSID));
        if (!splitTags.empty())
        {
            split_categ.Append("/");
            auto numTags = splitTags.size();
            for (decltype(numTags) i = 0; i < numTags; i++)
            {
                split_categ.Append((i > 0 ? ":" : "") + Model_Tag::instance().get(splitTags[i].TAGID)->TAGNAME);
            }
        }
        buffer << "S" << split_categ << "\n"
            << "$" << split_amount << "\n";
        if (!split_entry.NOTES.IsEmpty())
        {
            notes = split_entry.NOTES;
            notes.Replace("''", "'");
            notes.Replace("\n", "\nE");
            buffer << "E" << notes << "\n";
        }
    }

    buffer << "^" << "\n";
    return buffer;
}

const wxString mmExportTransaction::getAccountHeaderQIF(int accountID)
{
    wxString buffer = "";
    wxString currency_symbol = Model_Currency::GetBaseCurrency()->CURRENCY_SYMBOL;
    Model_Account::Data *account = Model_Account::instance().get(accountID);
    if (account)
    {
        double dInitBalance = account->INITIALBAL;
        Model_Currency::Data *currency = Model_Currency::instance().get(account->CURRENCYID);
        if (currency)
        {
            currency_symbol = currency->CURRENCY_SYMBOL;
        }

        const wxString currency_code = "[" + currency_symbol + "]";
        const wxString sInitBalance = Model_Currency::toString(dInitBalance, currency);

        buffer = wxString("!Account") << "\n"
            << "N" << account->ACCOUNTNAME << "\n"
            << "T" << qif_acc_type(account->ACCOUNTTYPE) << "\n"
            << "D" << currency_code << "\n"
            << (dInitBalance != 0 ? wxString::Format("$%s\n", sInitBalance) : "")
            << "^" << "\n"
            << "!Type:Cash" << "\n";
    }

    return buffer;
}

const wxString mmExportTransaction::getCategoriesQIF()
{
    wxString buffer_qif = "";

    buffer_qif << "!Type:Cat" << "\n";
    for (const auto& category : Model_Category::instance().all())
    {
        const wxString& categ_name = Model_Category::full_name(category.CATEGID, ":");
        bool bIncome = Model_Category::has_income(category.CATEGID);
        buffer_qif << "N" << categ_name << "\n"
            << (bIncome ? "I" : "E") << "\n"
            << "^" << "\n";
    }
    return buffer_qif;
}

//map Quicken !Account type strings to Model_Account::TYPE
// (not sure whether these need to be translated)
const std::unordered_map<wxString, int> mmExportTransaction::m_QIFaccountTypes =
{
    std::make_pair(wxString("Cash"), Model_Account::CASH), //Cash Flow: Cash Account
    std::make_pair(wxString("Bank"), Model_Account::CHECKING), //Cash Flow: Checking Account
    std::make_pair(wxString("CCard"), Model_Account::CREDIT_CARD), //Cash Flow: Credit Card Account
    std::make_pair(wxString("Invst"), Model_Account::INVESTMENT), //Investing: Investment Account
    std::make_pair(wxString("Oth A"), Model_Account::CHECKING), //Property & Debt: Asset
    std::make_pair(wxString("Oth L"), Model_Account::CHECKING), //Property & Debt: Liability
    std::make_pair(wxString("Invoice"), Model_Account::CHECKING), //Invoice (Quicken for Business only)
};

const wxString mmExportTransaction::qif_acc_type(const wxString& mmex_type)
{
    wxString qif_acc_type = m_QIFaccountTypes.begin()->first;
    for (const auto &item : m_QIFaccountTypes)
    {
        if (item.second == Model_Account::all_type().Index(mmex_type))
        {
            qif_acc_type = item.first;
            break;
        }
    }
    return qif_acc_type;
}

const wxString mmExportTransaction::mm_acc_type(const wxString& qif_type)
{
    wxString mm_acc_type = Model_Account::all_type()[Model_Account::CASH];
    for (const auto &item : m_QIFaccountTypes)
    {
        if (item.first == qif_type)
        {
            mm_acc_type = Model_Account::all_type()[(item.second)];
            break;
        }
    }
    return mm_acc_type;
}

// JSON Export ----------------------------------------------------------------------------

void mmExportTransaction::getAccountsJSON(PrettyWriter<StringBuffer>& json_writer
    , std::unordered_map <int /*account ID*/, wxString>& allAccounts4Export)
{
    json_writer.Key("ACCOUNTS");
    json_writer.StartArray();
    for (const auto &entry : allAccounts4Export)
    {
        Model_Account::Data* a = Model_Account::instance().get(entry.first);
        const auto c = Model_Currency::instance().get(a->CURRENCYID);
        json_writer.StartObject();
        json_writer.Key("ID");
        json_writer.Int(a->ACCOUNTID);
        json_writer.Key("NAME");
        json_writer.String(a->ACCOUNTNAME.utf8_str());
        json_writer.Key("INITIAL_BALANCE");
        json_writer.Double(a->INITIALBAL);
        json_writer.Key("TYPE");
        json_writer.String(a->ACCOUNTTYPE.utf8_str());
        json_writer.Key("CURRENCY_SYMBOL");
        json_writer.String(c->CURRENCY_SYMBOL.utf8_str());
        json_writer.EndObject();
    }
    json_writer.EndArray();
}

void mmExportTransaction::getPayeesJSON(PrettyWriter<StringBuffer>& json_writer, wxArrayInt& allPayeess4Export)
{
    if (!allPayeess4Export.empty())
    {
        json_writer.Key("PAYEES");
        json_writer.StartArray();
        for (const auto& entry : allPayeess4Export) {
            Model_Payee::Data* p = Model_Payee::instance().get(entry);
            if (p) {
                json_writer.StartObject();
                json_writer.Key("ID");
                json_writer.Int(p->PAYEEID);
                json_writer.Key("NAME");
                json_writer.String(p->PAYEENAME.utf8_str());
                json_writer.Key("CATEGORY_ID");
                json_writer.Int(p->CATEGID);
                json_writer.EndObject();
            }
        }
        json_writer.EndArray();
    }
}

void mmExportTransaction::getCategoriesJSON(PrettyWriter<StringBuffer>& json_writer)
{
    json_writer.Key("CATEGORIES");
    json_writer.StartArray();
    for (const auto& category : Model_Category::instance().all())
    {
        json_writer.StartObject();
        json_writer.Key("ID");
        json_writer.Int(category.CATEGID);
        json_writer.Key("NAME");
        json_writer.String(Model_Category::full_name(category.CATEGID, ":").utf8_str());
        json_writer.EndObject();
    }
    json_writer.EndArray();
}

void mmExportTransaction::getTagsJSON(PrettyWriter<StringBuffer>& json_writer, wxArrayInt& allTags4Export)
{
    json_writer.Key("TAGS");
    json_writer.StartArray();
    for (const auto& tagID : allTags4Export)
    {
        Model_Tag::Data* tag = Model_Tag::instance().get(tagID);
        if (tag)
        {
            json_writer.StartObject();
            json_writer.Key("ID");
            json_writer.Int(tag->TAGID);
            json_writer.Key("NAME");
            json_writer.String(tag->TAGNAME.utf8_str());
            json_writer.EndObject();
        }
    }
    json_writer.EndArray();
}

void mmExportTransaction::getUsedCategoriesJSON(PrettyWriter<StringBuffer>& json_writer)
{
    json_writer.Key("CATEGORIES");
    json_writer.StartArray();
    for (const auto& category : Model_Category::instance().all())
    {
        if (!Model_Category::instance().is_used(category.CATEGID))
            continue;
        json_writer.StartObject();
        json_writer.Key("ID");
        json_writer.Int(category.CATEGID);
        json_writer.Key("NAME");
        json_writer.String(Model_Category::full_name(category.CATEGID, ":").utf8_str());
        json_writer.EndObject();
    }
    json_writer.EndArray();
}

void mmExportTransaction::getTransactionJSON(PrettyWriter<StringBuffer>& json_writer, const Model_Checking::Full_Data& full_tran)
{
    json_writer.StartObject();
    full_tran.as_json(json_writer);

    json_writer.Key("TAGS");
    json_writer.StartArray();
    for (const auto& tag : full_tran.m_tags)
        json_writer.Int(tag.TAGID);
    json_writer.EndArray();

    if (!full_tran.m_splits.empty()) {
        json_writer.Key("DIVISION");
        json_writer.StartArray();
        for (const auto &split_entry : full_tran.m_splits)
        {
            double valueSplit = split_entry.SPLITTRANSAMOUNT;
            if (Model_Checking::type_id(full_tran) == Model_Checking::TYPE_ID_WITHDRAWAL) {
                valueSplit = -valueSplit;
            }

            json_writer.StartObject();
            json_writer.Key("CATEGORY_ID");
            json_writer.Int(split_entry.CATEGID);
            json_writer.Key("AMOUNT");
            json_writer.Double(valueSplit);
            json_writer.Key("TAGS");
            json_writer.StartArray();
            for (const auto& tag : Model_Taglink::instance().get(Model_Attachment::reftype_desc(Model_Attachment::TRANSACTIONSPLIT), split_entry.SPLITTRANSID))
                json_writer.Int(tag.second);
            json_writer.EndArray();
            json_writer.EndObject();

        }
        json_writer.EndArray();
    }

    const wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    Model_Attachment::Data_Set attachments = Model_Attachment::instance().FilterAttachments(RefType, full_tran.id());

    if (!attachments.empty())
    {
        const wxString folder = Model_Infotable::instance().GetStringInfo("ATTACHMENTSFOLDER:" + mmPlatformType(), "");
        json_writer.Key("ATTACHMENTS");
        json_writer.StartArray();
        for (const auto &entry : attachments) {
            json_writer.Int(entry.ATTACHMENTID);
        }
        json_writer.EndArray();
    }

    auto data = Model_CustomFieldData::instance().find(Model_CustomFieldData::REFID(full_tran.id()));
    auto f = Model_CustomField::instance().find(Model_CustomField::REFTYPE(RefType));
    if (!data.empty())
    {
        json_writer.Key("CUSTOM_FIELDS");
        json_writer.StartArray();
        for (const auto &entry : data)
        {

            auto customFields = Model_CustomField::instance().find(
                Model_CustomField::REFTYPE(RefType)
                , Model_CustomField::FIELDID(entry.FIELDID));

            for (const auto& i : customFields) {
                json_writer.Int(i.FIELDID);
            }
        }
        json_writer.EndArray();
    }

    json_writer.EndObject();
}

void mmExportTransaction::getAttachmentsJSON(PrettyWriter<StringBuffer>& json_writer, wxArrayInt& allAttachment4Export)
{

    if (!allAttachment4Export.empty())
    {
        const wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
        const wxString folder = Model_Infotable::instance().GetStringInfo("ATTACHMENTSFOLDER:" + mmPlatformType(), "");
        const wxString AttachmentsFolder = mmex::getPathAttachment(folder);

        json_writer.Key("ATTACHMENTS");
        json_writer.StartObject();

        json_writer.Key("FOLDER");
        json_writer.String(folder.utf8_str());
        json_writer.Key("FULL_PATH");
        json_writer.String(AttachmentsFolder.utf8_str());
        json_writer.Key("REFTYPE");
        json_writer.String(RefType.utf8_str());

        json_writer.Key("ATTACHMENTS_DATA");
        json_writer.StartArray();

        Model_Attachment::Data_Set attachments = Model_Attachment::instance().all();
        for (const auto& entry : attachments)
        {
            if (entry.REFTYPE != RefType) continue;
            if (allAttachment4Export.Index(entry.REFID) == wxNOT_FOUND) continue;

            json_writer.StartObject();
            entry.as_json(json_writer);
            json_writer.EndObject();
        }
        json_writer.EndArray();
        json_writer.EndObject();
    }
}

void mmExportTransaction::getCustomFieldsJSON(PrettyWriter<StringBuffer>& json_writer, wxArrayInt& allCustomFields4Export)
{

    if (!allCustomFields4Export.empty())
    {
        const wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);

        json_writer.Key("CUSTOM_FIELDS");
        json_writer.StartObject();

        // Data
        wxArrayInt cd;
        Model_CustomFieldData::Data_Set cds = Model_CustomFieldData::instance().all();

        if (!cds.empty()) {
            json_writer.Key("CUSTOM_FIELDS_DATA");
            json_writer.StartArray();

            for (const auto& entry : cds)
            {
                if (allCustomFields4Export.Index(entry.FIELDATADID) != wxNOT_FOUND)
                {
                    if (cd.Index(entry.FIELDID) == wxNOT_FOUND) {
                        cd.Add(entry.FIELDID);
                    }
                    json_writer.StartObject();
                    entry.as_json(json_writer);
                    json_writer.EndObject();
                }
            }
            json_writer.EndArray();
        }

        //Settings
        Model_CustomField::Data_Set custom_fields = Model_CustomField::instance().find(
            Model_CustomField::DB_Table_CUSTOMFIELD_V1::REFTYPE(RefType)
        );

        if (!custom_fields.empty()) {
            json_writer.Key("CUSTOM_FIELDS_SETTINGS");
            json_writer.StartArray();

            for (const auto& entry : custom_fields)
            {
                if (cd.Index(entry.FIELDID) == wxNOT_FOUND)
                    continue;

                json_writer.StartObject();
                json_writer.Key("ID");
                json_writer.Int(entry.FIELDID);
                json_writer.Key("REFTYPE");
                json_writer.String(entry.REFTYPE.utf8_str());
                json_writer.Key("DESCRIPTION");
                json_writer.String(entry.DESCRIPTION.utf8_str());
                json_writer.Key("TYPE");
                json_writer.String(entry.TYPE.utf8_str());
                json_writer.Key("PROPERTIES");
                json_writer.RawValue(entry.PROPERTIES.utf8_str(), entry.PROPERTIES.utf8_str().length(), rapidjson::Type::kObjectType);
                json_writer.EndObject();
            }
            json_writer.EndArray();
            json_writer.EndObject();
        }
    }
}
