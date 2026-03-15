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

#include "base/constants.h"
#include "base/paths.h"
#include "util/_util.h"
#include "export.h"

#include "model/AccountModel.h"
#include "model/AttachmentModel.h"
#include "model/CategoryModel.h"
#include "model/TrxModel.h"
#include "model/CurrencyModel.h"
#include "model/FieldModel.h"
#include "model/PayeeModel.h"
#include "model/FieldValueModel.h"
#include "model/FieldModel.h"
#include "model/TagModel.h"
#include "uicontrols/navigatortypes.h"

mmExportTransaction::mmExportTransaction()
{}

mmExportTransaction::~mmExportTransaction()
{}

const wxString mmExportTransaction::getTransactionCSV(
    const TrxModel::Full_Data& trx_xd,
    const wxString& dateMask,
    bool reverce
) {
    auto account_id = trx_xd.m_account_id;
    wxString buffer = "";
    bool is_transfer = trx_xd.is_transfer();
    const wxString delimiter = InfoModel::instance().getString("DELIMITER", mmex::DEFDELIMTER);

    wxString categ = trx_xd.m_splits.empty()
        ? CategoryModel::instance().get_id_fullname(trx_xd.m_category_id_n, ":")
        : "";
    wxString transNum = trx_xd.m_number;
    wxString notes = trx_xd.m_notes;
    wxString payee = trx_xd.PAYEENAME;

    const auto acc_in = AccountModel::instance().get_id_data_n(trx_xd.m_account_id);
    const auto curr_in = CurrencyModel::instance().get_id_data_n(acc_in->m_currency_id);
    wxString account = acc_in->m_name;
    wxString currency = curr_in->m_symbol;

    if (is_transfer) {
        account_id = reverce ? trx_xd.m_account_id : trx_xd.m_to_account_id_n;
        const auto acc_to = AccountModel::instance().get_id_data_n(trx_xd.m_to_account_id_n);
        const auto curr_to = CurrencyModel::instance().get_id_data_n(acc_to->m_currency_id);

        payee = reverce ? acc_to->m_name : acc_in->m_name;
        account = reverce ? acc_in->m_name : acc_to->m_name;
        currency = reverce ? curr_in->m_symbol : curr_to->m_symbol;

        //Transaction number used to make transaction unique
        // to proper merge transfer records
        if (transNum.IsEmpty() && notes.IsEmpty()) {
            transNum = wxString::Format("#%lld", trx_xd.m_id);
        }
    }

    if (trx_xd.has_split()) {
        for (const auto& tp_d : trx_xd.m_splits) {
            double valueSplit = tp_d.m_amount;
            if (trx_xd.is_withdrawal())
                valueSplit = -valueSplit;
            const wxString split_amount = wxString::FromCDouble(valueSplit, 2);
            const wxString split_categ = CategoryModel::instance().get_id_fullname(tp_d.m_category_id, ":");

            buffer << inQuotes(wxString::Format("%lld", trx_xd.m_id), delimiter) << delimiter;
            buffer << inQuotes(mmGetDateTimeForDisplay(trx_xd.m_date_time.isoDateTime(), dateMask), delimiter) << delimiter;
            buffer << inQuotes(trx_xd.m_status.key(), delimiter) << delimiter;
            buffer << inQuotes(trx_xd.m_type.name(), delimiter) << delimiter;

            buffer << inQuotes(acc_in->m_name, delimiter) << delimiter;

            buffer << inQuotes(payee, delimiter) << delimiter;
            buffer << inQuotes(split_categ, delimiter) << delimiter;

            buffer << inQuotes(split_amount, delimiter) << delimiter;
            buffer << inQuotes(curr_in->m_symbol, delimiter) << delimiter;
            buffer << inQuotes(transNum, delimiter) << delimiter;
            buffer << inQuotes(notes, delimiter);

            buffer << "\n";
        }
    }
    else {
        buffer << inQuotes(wxString::Format("%lld", trx_xd.m_id), delimiter) << delimiter;
        buffer << inQuotes(mmGetDateTimeForDisplay(trx_xd.m_date_time.isoDateTime(), dateMask), delimiter) << delimiter;
        buffer << inQuotes(trx_xd.m_status.key(), delimiter) << delimiter;
        buffer << inQuotes(trx_xd.m_type.name(), delimiter) << delimiter;

        buffer << inQuotes(account, delimiter) << delimiter;

        buffer << inQuotes(payee, delimiter) << delimiter;
        buffer << inQuotes(categ, delimiter) << delimiter;
        double value = trx_xd.account_flow(account_id);
        const wxString& s = wxString::FromCDouble(value, 2);
        buffer << inQuotes(s, delimiter) << delimiter;
        buffer << inQuotes(currency, delimiter) << delimiter;
        buffer << inQuotes(transNum, delimiter) << delimiter;
        buffer << inQuotes(notes, delimiter);

        buffer << "\n";
    }

    return buffer;
}

const wxString mmExportTransaction::getTransactionQIF(
    const TrxModel::Full_Data& trx_xd,
    const wxString& dateMask,
    bool reverse
) {
    wxString buffer = "";
    wxString categ = trx_xd.m_splits.empty()
        ? CategoryModel::instance().get_id_fullname(trx_xd.m_category_id_n, ":")
        : "";
    // Replace square brackets which are used to denote transfers in QIF
    categ.Replace("[", "(");
    categ.Replace("]", ")");
    wxString transNum = trx_xd.m_number;
    wxString notes = trx_xd.m_notes;
    wxString payee = trx_xd.PAYEENAME;

    if (trx_xd.is_transfer()) {
        const auto acc_in = AccountModel::instance().get_id_data_n(trx_xd.m_account_id);
        const auto acc_to = AccountModel::instance().get_id_data_n(trx_xd.m_to_account_id_n);
        const auto curr_in = CurrencyModel::instance().get_id_data_n(acc_in->m_currency_id);
        const auto curr_to = CurrencyModel::instance().get_id_data_n(acc_to->m_currency_id);

        categ = "[" + (reverse ? trx_xd.ACCOUNTNAME : trx_xd.TOACCOUNTNAME) + "]";
        payee = wxString::Format("%s %s %s -> %s %s %s"
            , wxString::FromCDouble(trx_xd.m_amount, 2), curr_in->m_symbol, acc_in->m_name
            , wxString::FromCDouble(trx_xd.m_to_amount, 2), curr_to->m_symbol, acc_to->m_name);
        //Transaction number used to make transaction unique
        // to proper merge transfer records
        if (transNum.IsEmpty() && notes.IsEmpty())
            transNum = wxString::Format("#%lld", trx_xd.m_id);
    }

    // don't allow '/' in category name as it is reserved for the class/tag separator
    categ.Replace("/", "-");
    if (!trx_xd.m_tags.empty()) {
        categ.Append("/");
        auto numTags = trx_xd.m_tags.size();
        for (decltype(numTags) i = 0; i < numTags; i++) {
            const TagData* tag_n = TagModel::instance().get_id_data_n(trx_xd.m_tags[i].m_tag_id);
            categ.Append((i > 0 ? ":" : "") + tag_n->m_name);
        }
    }

    buffer << "D" << mmGetDateTimeForDisplay(trx_xd.m_date_time.isoDateTime(), dateMask) << "\n";
    buffer << "C" << (trx_xd.is_reconciled() ? "R" : "") << "\n";
    double value = trx_xd.account_flow(
        reverse ? trx_xd.m_to_account_id_n : trx_xd.m_account_id
    );
    const wxString& s = wxString::FromCDouble(value, 2);
    buffer << "T" << s << "\n";
    if (!payee.empty())
        buffer << "P" << payee << "\n";
    if (!transNum.IsEmpty())
        buffer << "N" << transNum << "\n";
    if (!categ.IsEmpty())
        buffer << "L" << categ << "\n";
    if (!notes.IsEmpty()) {
        notes.Replace("''", "'");
        notes.Replace("\n", "\nM");
        buffer << "M" << notes << "\n";
    }

    for (const auto& tp_d : trx_xd.m_splits) {
        double valueSplit = tp_d.m_amount;
        if (trx_xd.is_withdrawal())
            valueSplit = -valueSplit;
        const wxString split_amount = wxString::FromCDouble(valueSplit, 2);
        wxString split_categ = CategoryModel::instance().get_id_fullname(tp_d.m_category_id, ":");
        split_categ.Replace("/", "-");
        TagLinkModel::DataA splitTags = TagLinkModel::instance().find(
            TagLinkCol::REFTYPE(TrxSplitModel::s_ref_type.name_n()),
            TagLinkCol::REFID(tp_d.m_id)
        );
        if (!splitTags.empty()) {
            split_categ.Append("/");
            auto numTags = splitTags.size();
            for (decltype(numTags) i = 0; i < numTags; i++) {
                const TagData* tag_n = TagModel::instance().get_id_data_n(splitTags[i].m_tag_id);
                split_categ.Append((i > 0 ? ":" : "") + tag_n->m_name);
            }
        }
        buffer << "S" << split_categ << "\n"
            << "$" << split_amount << "\n";
        if (!tp_d.m_notes.IsEmpty()) {
            notes = tp_d.m_notes;
            notes.Replace("''", "'");
            notes.Replace("\n", "\nE");
            buffer << "E" << notes << "\n";
        }
    }

    buffer << "^" << "\n";
    return buffer;
}

const wxString mmExportTransaction::getAccountHeaderQIF(int64 accountID)
{
    wxString buffer = "";
    wxString currency_symbol = CurrencyModel::instance().get_base_data_n()->m_symbol;
    const AccountData *account_n = AccountModel::instance().get_id_data_n(accountID);
    if (account_n) {
        double dInitBalance = account_n->m_open_balance;
        const CurrencyData *currency = CurrencyModel::instance().get_id_data_n(account_n->m_currency_id);
        if (currency) {
            currency_symbol = currency->m_symbol;
        }

        const wxString currency_code = "[" + currency_symbol + "]";
        const wxString sInitBalance = CurrencyModel::instance().toString(dInitBalance, currency);

        buffer = wxString("!Account") << "\n"
            << "N" << account_n->m_name << "\n"
            << "T" << qif_acc_type(account_n->m_type_) << "\n"
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
    for (const auto& cat_d : CategoryModel::instance().find_all()) {
        const wxString& full_name = CategoryModel::instance().get_id_fullname(cat_d.m_id, ":");
        double cat_income = CategoryModel::instance().get_id_income(cat_d.m_id);
        buffer_qif << "N" << full_name << "\n"
            << (cat_income > 0.0 ? "I" : "E") << "\n"
            << "^" << "\n";
    }
    return buffer_qif;
}

//map Quicken !Account type strings to NavigatorTypes::TYPE_ID
// (not sure whether these need to be translated)
const std::unordered_map<wxString, int> mmExportTransaction::m_QIFaccountTypes =
{
    std::make_pair(wxString("Cash"), NavigatorTypes::TYPE_ID_CASH), //Cash Flow: Cash Account
    std::make_pair(wxString("Bank"), NavigatorTypes::TYPE_ID_CHECKING), //Cash Flow: Checking Account
    std::make_pair(wxString("CCard"), NavigatorTypes::TYPE_ID_CREDIT_CARD), //Cash Flow: Credit Card Account
    std::make_pair(wxString("Invst"), NavigatorTypes::TYPE_ID_INVESTMENT), //Investing: Investment Account
    std::make_pair(wxString("Oth A"), NavigatorTypes::TYPE_ID_CHECKING), //Property & Debt: Asset
    std::make_pair(wxString("Oth L"), NavigatorTypes::TYPE_ID_CHECKING), //Property & Debt: Liability
    std::make_pair(wxString("Invoice"), NavigatorTypes::TYPE_ID_CHECKING), //Invoice (Quicken for Business only)
};

const wxString mmExportTransaction::qif_acc_type(const wxString& mmex_type)
{
    int mmex_typeId = NavigatorTypes::instance().getTypeIdFromDBName(mmex_type, -1);
    wxString qif_acc_type = m_QIFaccountTypes.begin()->first;
    for (const auto &item : m_QIFaccountTypes) {
        if (item.second == mmex_typeId) {
            qif_acc_type = item.first;
            break;
        }
    }
    return qif_acc_type;
}

const wxString mmExportTransaction::mm_acc_type(const wxString& qif_type)
{
    wxString mm_acc_type = NavigatorTypes::instance().getCashAccountStr();
    for (const auto &item : m_QIFaccountTypes)
    {
        if (item.first == qif_type)
        {
            mm_acc_type = NavigatorTypes::instance().type_name(item.second);
            break;
        }
    }
    return mm_acc_type;
}

// JSON Export ----------------------------------------------------------------------------

void mmExportTransaction::getAccountsJSON(PrettyWriter<StringBuffer>& json_writer
    , std::map <int64 /*account ID*/, wxString>& allAccounts4Export)
{
    json_writer.Key("ACCOUNTS");
    json_writer.StartArray();
    for (const auto &entry : allAccounts4Export)
    {
        const AccountData* a = AccountModel::instance().get_id_data_n(entry.first);
        const CurrencyData* c = CurrencyModel::instance().get_id_data_n(a->m_currency_id);
        json_writer.StartObject();
        json_writer.Key("ID");
        json_writer.Int64(a->m_id.GetValue());
        json_writer.Key("NAME");
        json_writer.String(a->m_name.utf8_str());
        json_writer.Key("INITIAL_BALANCE");
        json_writer.Double(a->m_open_balance);
        json_writer.Key("TYPE");
        json_writer.String(a->m_type_.utf8_str());
        json_writer.Key("CURRENCY_SYMBOL");
        json_writer.String(c->m_symbol.utf8_str());
        json_writer.EndObject();
    }
    json_writer.EndArray();
}

void mmExportTransaction::getPayeesJSON(PrettyWriter<StringBuffer>& json_writer, wxArrayInt64& allPayeess4Export)
{
    if (!allPayeess4Export.empty())
    {
        json_writer.Key("PAYEES");
        json_writer.StartArray();
        for (const auto& entry : allPayeess4Export) {
            const PayeeData* payee_n = PayeeModel::instance().get_id_data_n(entry);
            if (payee_n) {
                json_writer.StartObject();
                json_writer.Key("ID");
                json_writer.Int64(payee_n->m_id.GetValue());
                json_writer.Key("NAME");
                json_writer.String(payee_n->m_name.utf8_str());
                json_writer.Key("CATEGORY_ID");
                json_writer.Int64(payee_n->m_category_id_n.GetValue());
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
    for (const auto& category : CategoryModel::instance().find_all())
    {
        json_writer.StartObject();
        json_writer.Key("ID");
        json_writer.Int64(category.m_id.GetValue());
        json_writer.Key("NAME");
        json_writer.String(CategoryModel::instance().get_id_fullname(category.m_id, ":").utf8_str());
        json_writer.EndObject();
    }
    json_writer.EndArray();
}

void mmExportTransaction::getTagsJSON(PrettyWriter<StringBuffer>& json_writer, wxArrayInt64& allTags4Export)
{
    json_writer.Key("TAGS");
    json_writer.StartArray();
    for (const auto& tagID : allTags4Export)
    {
        const TagData* tag_n = TagModel::instance().get_id_data_n(tagID);
        if (tag_n) {
            json_writer.StartObject();
            json_writer.Key("ID");
            json_writer.Int64(tag_n->m_id.GetValue());
            json_writer.Key("NAME");
            json_writer.String(tag_n->m_name.utf8_str());
            json_writer.EndObject();
        }
    }
    json_writer.EndArray();
}

void mmExportTransaction::getUsedCategoriesJSON(PrettyWriter<StringBuffer>& json_writer)
{
    json_writer.Key("CATEGORIES");
    json_writer.StartArray();
    for (const auto& category : CategoryModel::instance().find_all())
    {
        if (!CategoryModel::instance().is_used(category.m_id))
            continue;
        json_writer.StartObject();
        json_writer.Key("ID");
        json_writer.Int64(category.m_id.GetValue());
        json_writer.Key("NAME");
        json_writer.String(CategoryModel::instance().get_id_fullname(category.m_id, ":").utf8_str());
        json_writer.EndObject();
    }
    json_writer.EndArray();
}

void mmExportTransaction::getTransactionJSON(
    PrettyWriter<StringBuffer>& json_writer,
    const TrxModel::Full_Data& trx_xd
) {
    json_writer.StartObject();
    trx_xd.as_json(json_writer);

    json_writer.Key("TAGS");
    json_writer.StartArray();
    for (const auto& gl_d : trx_xd.m_tags)
        json_writer.Int64(gl_d.m_tag_id.GetValue());
    json_writer.EndArray();

    if (!trx_xd.m_splits.empty()) {
        json_writer.Key("DIVISION");
        json_writer.StartArray();
        for (const auto& tp_d : trx_xd.m_splits) {
            double valueSplit = tp_d.m_amount;
            if (trx_xd.is_withdrawal()) {
                valueSplit = -valueSplit;
            }

            json_writer.StartObject();
            json_writer.Key("CATEGORY_ID");
            json_writer.Int64(tp_d.m_category_id.GetValue());
            json_writer.Key("AMOUNT");
            json_writer.Double(valueSplit);
            json_writer.Key("TAGS");
            json_writer.StartArray();
            for (const auto& tag : TagLinkModel::instance().find_ref_tag_m(
                TrxSplitModel::s_ref_type, tp_d.m_id)
            )
                json_writer.Int64(tag.second.GetValue());
            json_writer.EndArray();
            json_writer.EndObject();

        }
        json_writer.EndArray();
    }

    AttachmentModel::DataA att_a = AttachmentModel::instance().find_ref_data_a(
        TrxModel::s_ref_type, trx_xd.m_id
    );

    if (!att_a.empty()) {
        //const wxString folder = InfoModel::instance().getString("ATTACHMENTSFOLDER:" + mmPlatformType(), "");
        json_writer.Key("ATTACHMENTS");
        json_writer.StartArray();
        for (const auto& att_d : att_a) {
            json_writer.Int64(att_d.m_id.GetValue());
        }
        json_writer.EndArray();
    }

    auto fv_a = FieldValueModel::instance().find(
        FieldValueModel::REFTYPEID(TrxModel::s_ref_type, trx_xd.m_id)
    );
    auto f = FieldModel::instance().find(
        FieldCol::REFTYPE(TrxModel::s_ref_type.name_n())
    );
    if (!fv_a.empty()) {
        json_writer.Key("CUSTOM_FIELDS");
        json_writer.StartArray();
        for (const auto& fv_d : fv_a) {
            // TODO: field_d.m_id is equal to fv_d.m_field_id
            auto field_a = FieldModel::instance().find(
                FieldCol::REFTYPE(TrxModel::s_ref_type.name_n()),
                FieldCol::FIELDID(fv_d.m_field_id)
            );
            for (const auto& field_d : field_a) {
                json_writer.Int64(field_d.m_id.GetValue());
            }
        }
        json_writer.EndArray();
    }

    json_writer.EndObject();
}

void mmExportTransaction::getAttachmentsJSON(
    PrettyWriter<StringBuffer>& json_writer,
    wxArrayInt64& ref_id_a
) {
    if (ref_id_a.empty())
        return;

    RefTypeN ref_type = RefTypeN(RefTypeN::e_trx);
    const wxString folder = InfoModel::instance().getString("ATTACHMENTSFOLDER:" + mmPlatformType(), "");
    const wxString AttachmentsFolder = mmex::getPathAttachment(folder);

    json_writer.Key("ATTACHMENTS");
    json_writer.StartObject();

    json_writer.Key("FOLDER");
    json_writer.String(folder.utf8_str());
    json_writer.Key("FULL_PATH");
    json_writer.String(AttachmentsFolder.utf8_str());
    json_writer.Key("REFTYPE");
    json_writer.String(ref_type.name_n().utf8_str());

    json_writer.Key("ATTACHMENTS_DATA");
    json_writer.StartArray();

    AttachmentModel::DataA att_a = AttachmentModel::instance().find_all();
    for (const auto& att_d : att_a) {
        if (att_d.m_ref_type_n.id_n() != ref_type.id_n())
            continue;
        if (std::find(ref_id_a.begin(), ref_id_a.end(), att_d.m_ref_id) == ref_id_a.end())
            continue;
        json_writer.StartObject();
        att_d.as_json(json_writer);
        json_writer.EndObject();
    }
    json_writer.EndArray();
    json_writer.EndObject();
}

void mmExportTransaction::getCustomFieldsJSON(
    PrettyWriter<StringBuffer>& json_writer,
    wxArrayInt64& fv_id_a
) {
    if (fv_id_a.empty())
        return;

    json_writer.Key("CUSTOM_FIELDS");
    json_writer.StartObject();

    // Data
    wxArrayInt64 field_id_a;
    FieldValueModel::DataA fv_a = FieldValueModel::instance().find_all();
    if (!fv_a.empty()) {
        json_writer.Key("CUSTOM_FIELDS_DATA");
        json_writer.StartArray();
        for (const auto& fv_d : fv_a) {
            if (std::find(fv_id_a.begin(), fv_id_a.end(), fv_d.m_id) == fv_id_a.end())
                continue;

            if (std::find(
                field_id_a.begin(), field_id_a.end(), fv_d.m_field_id
            ) == field_id_a.end()) {
                field_id_a.push_back(fv_d.m_field_id);
            }
            json_writer.StartObject();
            fv_d.as_json(json_writer);
            json_writer.EndObject();
        }
        json_writer.EndArray();
    }

    //Settings
    FieldModel::DataA field_a = FieldModel::instance().find(
        FieldCol::REFTYPE(TrxModel::s_ref_type.name_n())
    );

    if (!field_a.empty()) {
        json_writer.Key("CUSTOM_FIELDS_SETTINGS");
        json_writer.StartArray();

        for (const auto& field_d : field_a) {
            if (std::find(
                field_id_a.begin(), field_id_a.end(), field_d.m_id
            ) == field_id_a.end()) {
                continue;
            }

            json_writer.StartObject();
            json_writer.Key("ID");
            json_writer.Int64(field_d.m_id.GetValue());
            json_writer.Key("REFTYPE");
            json_writer.String(field_d.m_ref_type.name_n().utf8_str());
            json_writer.Key("DESCRIPTION");
            json_writer.String(field_d.m_description.utf8_str());
            json_writer.Key("TYPE");
            json_writer.String(field_d.m_type_n.name_n().utf8_str());
            json_writer.Key("PROPERTIES");
            json_writer.RawValue(field_d.m_properties.utf8_str(), field_d.m_properties.utf8_str().length(), rapidjson::Type::kObjectType);
            json_writer.EndObject();
        }
        json_writer.EndArray();
        json_writer.EndObject();
    }
}
