#include "PocketBaseSync.h"
#include "base/_defs.h"
#include "model/CurrencyModel.h"
#include "model/AccountModel.h"
#include "model/CategoryModel.h"
#include "data/CategoryData.h"
#include "model/PayeeModel.h"
#include "model/AssetModel.h"
#include "data/AssetData.h"
#include "model/StockModel.h"
#include "model/StockHistoryModel.h"
#include "data/StockData.h"
#include "data/StockHistoryData.h"
#include "data/PayeeData.h"
#include "data/AccountData.h"
#include "model/TrxModel.h"
#include "model/TrxSplitModel.h"
#include "model/TrxLinkModel.h"
#include "model/TrxShareModel.h"
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <functional>

namespace {
bool PocketBaseValuesEqual(const rapidjson::Value& lhs, const rapidjson::Value& rhs) {
    if (lhs.IsString() && rhs.IsString()) return std::string(lhs.GetString(), lhs.GetStringLength()) == std::string(rhs.GetString(), rhs.GetStringLength());
    if (lhs.IsNumber() && rhs.IsNumber()) return lhs.GetDouble() == rhs.GetDouble();
    if (lhs.IsBool() && rhs.IsBool()) return lhs.GetBool() == rhs.GetBool();
    return lhs.IsNull() && rhs.IsNull();
}
bool PocketBaseRecordEqual(const rapidjson::Value& lhs, const rapidjson::Value& rhs) {
    if (!lhs.IsObject() || !rhs.IsObject()) return false;
    for (auto field = lhs.MemberBegin(); field != lhs.MemberEnd(); ++field) {
        const auto name = field->name.GetString();
        if (!rhs.HasMember(name) || !PocketBaseValuesEqual(field->value, rhs[name])) return false;
    }
    return true;
}
size_t PocketBaseWrite(char* data, size_t size, size_t count, void* context) {
    static_cast<std::string*>(context)->append(data, size * count);
    return size * count;
}

void configurePocketBaseTransfer(CURL* curl, curl_slist* headers, std::string& response, long timeoutSeconds)
{
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, PocketBaseWrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSeconds);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
}

bool fetchPocketBaseCollection(const wxString& base, const wxString& token,
    const char* collection, std::vector<std::string>& records, wxString& error)
{
    const auto auth = std::string("Authorization: ") + token.utf8_string().data();
    for (int page = 1; ; ++page) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            error = "Could not initialize HTTP client.";
            return false;
        }
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, auth.c_str());
        std::string response;
        long status = 0;
        const auto url = (base + wxString::Format("/api/collections/%s/records?perPage=500&page=%d", collection, page)).utf8_string();
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        configurePocketBaseTransfer(curl, headers, response, 60L);
        const CURLcode rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        if (rc != CURLE_OK || status < 200 || status >= 300) {
            error = rc != CURLE_OK
                ? wxString::Format("%s list request failed: %s", collection, curl_easy_strerror(rc))
                : wxString::Format("%s list request failed (%ld): %s", collection, status, response);
            return false;
        }
        rapidjson::Document list;
        list.Parse(response.c_str());
        if (list.HasParseError() || !list.IsObject() || !list.HasMember("items") || !list["items"].IsArray()
            || !list.HasMember("totalPages") || !list["totalPages"].IsInt()) {
            error = wxString::Format("PocketBase returned an invalid %s list.", collection);
            return false;
        }
        for (const auto& item : list["items"].GetArray()) {
            if (!item.IsObject()) continue;
            rapidjson::StringBuffer copy;
            rapidjson::Writer<rapidjson::StringBuffer> writer(copy);
            item.Accept(writer);
            records.emplace_back(copy.GetString(), copy.GetSize());
        }
        if (page >= list["totalPages"].GetInt()) break;
    }
    return true;
}

int64 pocketBaseInteger(const rapidjson::Value& record, const char* field, int64 fallback = -1)
{
    if (!record.IsObject() || !record.HasMember(field) || !record[field].IsNumber()) return fallback;
    const auto& value = record[field];
    return value.IsInt64() ? value.GetInt64() : static_cast<int64>(value.GetDouble());
}

wxString pocketBaseString(const rapidjson::Value& record, const char* field)
{
    return record.IsObject() && record.HasMember(field) && record[field].IsString()
        ? wxString::FromUTF8(record[field].GetString()) : wxString();
}

void normalizeTransactionRecord(rapidjson::Value& record, rapidjson::Document::AllocatorType& allocator,
    bool compareDateOnly = true)
{
    const auto setNull = [&record](const char* field) {
        if (record.HasMember(field)) record[field].SetNull();
    };
    const auto nullWhenUnset = [&record, &setNull](const char* field) {
        if (!record.HasMember(field) || record[field].IsNull()
            || (record[field].IsNumber() && record[field].GetDouble() <= 0)) setNull(field);
    };
    nullWhenUnset("TOACCOUNTID");
    nullWhenUnset("CATEGID");
    nullWhenUnset("FOLLOWUPID");
    if (record.HasMember("COLOR") && record["COLOR"].IsNumber() && record["COLOR"].GetDouble() == -1)
        setNull("COLOR");
    if (record.HasMember("TRANSCODE") && record["TRANSCODE"].IsString()
        && std::string(record["TRANSCODE"].GetString()) != "Transfer") setNull("TOTRANSAMOUNT");

    for (const char* field : {"STATUS", "TRANSACTIONNUMBER", "NOTES", "LASTUPDATEDTIME", "DELETEDTIME"}) {
        if (record.HasMember(field) && record[field].IsNull())
            record[field].SetString("", allocator);
    }
    for (const char* field : {"LASTUPDATEDTIME", "DELETEDTIME"}) {
        if (!record.HasMember(field) || !record[field].IsString()) continue;
        std::string timestamp(record[field].GetString());
        if (timestamp.size() > 10 && timestamp[10] == ' ') {
            timestamp[10] = 'T';
            record[field].SetString(timestamp.c_str(), static_cast<rapidjson::SizeType>(timestamp.size()), allocator);
        }
    }
    if (compareDateOnly && record.HasMember("TRANSDATE") && record["TRANSDATE"].IsString()) {
        std::string date(record["TRANSDATE"].GetString());
        if (date.size() > 10) {
            date.resize(10);
            record["TRANSDATE"].SetString(date.c_str(), static_cast<rapidjson::SizeType>(date.size()), allocator);
        }
    }
}

bool supportedTransaction(const TrxData& transaction)
{
    return (transaction.m_type == TrxType(TrxType::e_withdrawal)
        || transaction.m_type == TrxType(TrxType::e_deposit)
        || transaction.m_type == TrxType(TrxType::e_transfer))
        && transaction.m_status.id() >= TrxStatus::e_unreconciled
        && transaction.m_status.id() < TrxStatus::size;
}

bool supportedTransaction(const rapidjson::Value& record)
{
    const auto type = pocketBaseString(record, "TRANSCODE");
    const auto status = pocketBaseString(record, "STATUS");
    return (type == "Withdrawal" || type == "Deposit" || type == "Transfer")
        && (status.empty() || status == "R" || status == "V" || status == "F" || status == "D");
}

bool postPocketBaseRecord(const wxString& base, const wxString& token,
    const char* collection, const wxString& payload, wxString& error)
{
    CURL* curl = curl_easy_init();
    if (!curl) {
        error = "Could not initialize HTTP client.";
        return false;
    }
    const auto auth = std::string("Authorization: ") + token.utf8_string().data();
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth.c_str());
    std::string response;
    long status = 0;
    const wxString url = base + wxString::Format("/api/collections/%s/records", collection);
    const auto urlUtf8 = url.utf8_string();
    const auto body = payload.utf8_string();
    configurePocketBaseTransfer(curl, headers, response, 30L);
    curl_easy_setopt(curl, CURLOPT_URL, urlUtf8.data());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.length()));
    const CURLcode rc = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (rc == CURLE_OK && status >= 200 && status < 300)
        return true;
    error = rc != CURLE_OK
        ? wxString::Format("%s upload transport error: %s", collection, curl_easy_strerror(rc))
        : wxString::Format("%s upload failed (%ld): %s", collection, status, response);
    return false;
}

template<typename Data, typename Model, typename Decode>
bool syncPortfolioCollection(const wxString& base, const wxString& token,
    const char* collection, const char* keyField, const std::vector<Data>& local,
    Model& model, Decode decode, PocketBaseSyncResult& result, wxString& error)
{
    std::vector<std::string> records;
    if (!fetchPocketBaseCollection(base, token, collection, records, error))
        return false;
    std::map<int64, std::string> remote;
    for (const auto& json : records) {
        rapidjson::Document record;
        record.Parse(json.c_str());
        const int64 id = pocketBaseInteger(record, keyField);
        if (record.HasParseError() || !record.IsObject() || id <= 0) {
            ++result.conflicts;
            continue;
        }
        remote[id] = json;
    }
    std::set<int64> localIds;
    for (const auto& row : local) localIds.insert(row.id());
    for (const auto& [id, json] : remote) {
        if (localIds.count(id)) continue;
        rapidjson::Document record;
        record.Parse(json.c_str());
        Data row;
        if (!decode(record, id, row)) {
            ++result.conflicts;
            continue;
        }
        if (model.import_data_n(row, id)) ++result.imported;
        else ++result.conflicts;
    }
    for (const auto& row : local) {
        const auto payload = row.to_json().utf8_string();
        rapidjson::Document localRecord;
        localRecord.Parse(payload.data());
        if (localRecord.HasParseError() || !localRecord.IsObject()) {
            ++result.conflicts;
            continue;
        }
        const auto found = remote.find(row.id());
        if (found != remote.end()) {
            rapidjson::Document remoteRecord;
            remoteRecord.Parse(found->second.c_str());
            if (!remoteRecord.HasParseError() && PocketBaseRecordEqual(localRecord, remoteRecord)) ++result.unchanged;
            else ++result.conflicts;
            continue;
        }
        if (!postPocketBaseRecord(base, token, collection, wxString::FromUTF8(payload.data()), error))
            return false;
        ++result.created;
    }
    return true;
}
}

PocketBaseSyncResult PocketBaseSyncService::syncCurrencies(const wxString& base, const wxString& token)
{
    PocketBaseSyncResult result;
    if (token.empty()) {
        result.message = "Sign in before uploading currencies.";
        return result;
    }
    auto currencies = CurrencyModel::instance().find_data_a();
    size_t created = 0, updated = 0, skipped = 0, conflicts = 0, imported = 0;
    bool failed = false;
    std::map<long long, std::pair<wxString, std::string>> remoteRecords;
    {
        CURL* curl = curl_easy_init();
        std::string response;
        long status = 0;
        struct curl_slist* headers = nullptr;
        const auto auth = std::string("Authorization: ") + token.utf8_string().data();
        headers = curl_slist_append(headers, auth.c_str());
        const auto url = (base + "/api/collections/CURRENCYFORMATS_V1/records?perPage=500&page=1").utf8_string();
        if (!curl) {
            result.message = "Could not initialize HTTP client.";
            failed = true;
        } else {
            curl_easy_setopt(curl, CURLOPT_URL, url.data());
            configurePocketBaseTransfer(curl, headers, response, 30L);
            const CURLcode rc = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
            if (rc != CURLE_OK || status < 200 || status >= 300) {
                result.message = rc != CURLE_OK
                    ? "Currency list request failed: " + wxString::FromUTF8(curl_easy_strerror(rc))
                    : wxString::Format("Currency list request failed (%ld): %s", status, response);
                failed = true;
            } else {
                rapidjson::Document list; list.Parse(response.c_str());
                if (list.HasParseError() || !list.IsObject() || !list.HasMember("items") || !list["items"].IsArray()) {
                    result.message = "PocketBase returned an invalid currency list.";
                    failed = true;
                } else {
                    for (const auto& item : list["items"].GetArray()) {
                        if (item.IsObject() && item.HasMember("CURRENCYID") && item["CURRENCYID"].IsInt64()
                            && item.HasMember("id") && item["id"].IsString()) {
                            rapidjson::StringBuffer copy;
                            rapidjson::Writer<rapidjson::StringBuffer> writer(copy);
                            item.Accept(writer);
                            remoteRecords[item["CURRENCYID"].GetInt64()] = {
                                wxString::FromUTF8(item["id"].GetString()),
                                std::string(copy.GetString(), copy.GetSize())
                            };
                        }
                    }
                }
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
    }
    std::set<long long> localIds;
    for (const auto& currency : currencies) localIds.insert(currency.id().GetValue());
    for (const auto& [businessId, remote] : remoteRecords) {
        if (localIds.count(businessId)) continue;
        rapidjson::Document record;
        record.Parse(remote.second.c_str());
        if (record.HasParseError() || !record.IsObject()) { ++conflicts; continue; }
        CurrencyData currency;
        currency.id(businessId);
        auto str = [&record](const char* key) { return record.HasMember(key) && record[key].IsString() ? wxString::FromUTF8(record[key].GetString()) : wxString(); };
        currency.m_name = str("CURRENCYNAME");
        currency.m_symbol = str("CURRENCY_SYMBOL");
        currency.m_type = CurrencyType(str("CURRENCY_TYPE"));
        currency.m_prefix_symbol = str("PFX_SYMBOL");
        currency.m_suffix_symbol = str("SFX_SYMBOL");
        currency.m_decimal_point = str("DECIMAL_POINT");
        currency.m_group_separator = str("GROUP_SEPARATOR");
        currency.m_unit_name = str("UNIT_NAME");
        currency.m_cent_name = str("CENT_NAME");
        currency.m_scale = record.HasMember("SCALE") && record["SCALE"].IsInt64() ? record["SCALE"].GetInt64() : 100;
        currency.m_base_conv_rate = record.HasMember("BASECONVRATE") && record["BASECONVRATE"].IsNumber() ? record["BASECONVRATE"].GetDouble() : 1.0;
        if (CurrencyModel::instance().import_data_n(currency, businessId)) ++imported;
        else ++conflicts;
    }
    for (const auto& currency : currencies) {
        if (failed) break;
        const wxString payload = currency.to_json();
        rapidjson::Document localRecord;
        const auto payloadUtf8 = payload.utf8_string();
        localRecord.Parse(payloadUtf8.data());
        const auto foundRecord = remoteRecords.find(currency.id().GetValue());
        const bool exists = foundRecord != remoteRecords.end();
        wxString recordId = exists ? foundRecord->second.first : "";
        if (exists) {
            rapidjson::Document remoteRecord;
            remoteRecord.Parse(foundRecord->second.second.c_str());
            if (!localRecord.HasParseError() && !remoteRecord.HasParseError()) {
                if (PocketBaseRecordEqual(localRecord, remoteRecord)) ++skipped;
                else ++conflicts;
                continue;
            }
        }

        CURL* curl = curl_easy_init();
        if (!curl) { result.message = "Could not initialize HTTP client."; failed = true; break; }
        const auto auth = std::string("Authorization: ") + token.utf8_string().data();
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, auth.c_str());
        std::string response;
        long status = 0;
        configurePocketBaseTransfer(curl, headers, response, 20L);
        const wxString method = recordId.empty() ? "POST" : "PATCH";
        wxString writeUrl = base + "/api/collections/CURRENCYFORMATS_V1/records";
        if (!recordId.empty()) writeUrl += "/" + recordId;
        const auto url = writeUrl.utf8_string();
        const auto body = payload.utf8_string();
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.utf8_string().data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.length()));
        CURLcode rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        if (rc != CURLE_OK || status < 200 || status >= 300) {
            result.message = rc != CURLE_OK
                ? "Currency upload transport error: " + wxString::FromUTF8(curl_easy_strerror(rc)) + " (" + wxString::FromUTF8(url.data()) + ")"
                : wxString::Format("Currency upload failed (%ld): %s", status, response);
            failed = true;
            curl_slist_free_all(headers); curl_easy_cleanup(curl); break;
        }
        if (recordId.empty()) ++created; else ++updated;
        curl_slist_free_all(headers); curl_easy_cleanup(curl);
    }
    if (!failed) result.message = wxString::Format("Currency sync complete: %zu downloaded, %zu created, %zu updated, %zu unchanged, %zu conflicts.", imported, created, updated, skipped, conflicts);
    result.ok = !failed;
    return result;
}


PocketBaseSyncResult PocketBaseSyncService::syncAccounts(const wxString& base, const wxString& token)
{
    PocketBaseSyncResult result;
    if (token.empty()) {
        result.message = "Sign in before uploading accounts.";
        return result;
    }
    auto accounts = AccountModel::instance().find_data_a();
    size_t created = 0, updated = 0, skipped = 0, conflicts = 0, imported = 0;
    bool failed = false;
    std::map<long long, std::pair<wxString, std::string>> remoteRecords;
    {
        CURL* curl = curl_easy_init();
        std::string response;
        long status = 0;
        struct curl_slist* headers = nullptr;
        const auto auth = std::string("Authorization: ") + token.utf8_string().data();
        headers = curl_slist_append(headers, auth.c_str());
        const auto url = (base + "/api/collections/ACCOUNTLIST_V1/records?perPage=500&page=1").utf8_string();
        if (!curl) {
            result.message = "Could not initialize HTTP client.";
            failed = true;
        } else {
            curl_easy_setopt(curl, CURLOPT_URL, url.data());
            configurePocketBaseTransfer(curl, headers, response, 30L);
            const CURLcode rc = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
            if (rc != CURLE_OK || status < 200 || status >= 300) {
                result.message = rc != CURLE_OK
                    ? "Account list request failed: " + wxString::FromUTF8(curl_easy_strerror(rc))
                    : wxString::Format("Account list request failed (%ld): %s", status, response);
                failed = true;
            } else {
                rapidjson::Document list; list.Parse(response.c_str());
                if (list.HasParseError() || !list.IsObject() || !list.HasMember("items") || !list["items"].IsArray()) {
                    result.message = "PocketBase returned an invalid account list.";
                    failed = true;
                } else {
                    for (const auto& item : list["items"].GetArray()) {
                        if (item.IsObject() && item.HasMember("ACCOUNTID") && item["ACCOUNTID"].IsInt64()
                            && item.HasMember("id") && item["id"].IsString()) {
                            rapidjson::StringBuffer copy;
                            rapidjson::Writer<rapidjson::StringBuffer> writer(copy);
                            item.Accept(writer);
                            remoteRecords[item["ACCOUNTID"].GetInt64()] = {
                                wxString::FromUTF8(item["id"].GetString()),
                                std::string(copy.GetString(), copy.GetSize())
                            };
                        }
                    }
                }
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
    }
    std::set<long long> localAccountIds;
    for (const auto& account : accounts) localAccountIds.insert(account.id().GetValue());
    for (const auto& [businessId, remote] : remoteRecords) {
        if (localAccountIds.count(businessId)) continue;
        rapidjson::Document record;
        record.Parse(remote.second.c_str());
        if (record.HasParseError() || !record.IsObject()) { ++conflicts; continue; }
        const auto integer = [&record](const char* key, int64 fallback = 0) -> int64 {
            return record.HasMember(key) && record[key].IsInt64() ? int64(record[key].GetInt64()) : fallback;
        };
        const auto number = [&record](const char* key) -> double {
            return record.HasMember(key) && record[key].IsNumber() ? record[key].GetDouble() : 0.0;
        };
        const auto string = [&record](const char* key) -> wxString {
            return record.HasMember(key) && record[key].IsString() ? wxString::FromUTF8(record[key].GetString()) : wxString();
        };
        const int64 currencyId = integer("CURRENCYID", -1);
        if (currencyId <= 0 || !CurrencyModel::instance().get_idN_data_n(currencyId)) { ++conflicts; continue; }
        AccountData account;
        account.id(businessId);
        account.m_name = string("ACCOUNTNAME");
        account.m_type_ = string("ACCOUNTTYPE");
        account.m_currency_id = currencyId;
        account.m_status = AccountStatus(string("STATUS"));
        account.m_favorite = AccountFavorite(string("FAVORITEACCT"));
        account.m_num = string("ACCOUNTNUM");
        account.m_notes = string("NOTES");
        account.m_held_at = string("HELDAT");
        account.m_website = string("WEBSITE");
        account.m_contact_info = string("CONTACTINFO");
        account.m_access_info = string("ACCESSINFO");
        account.m_open_date = mmDate(string("INITIALDATE"));
        account.m_open_balance = number("INITIALBAL");
        account.m_stmt_locked = integer("STATEMENTLOCKED") != 0;
        account.m_stmt_date_n = mmDateN(string("STATEMENTDATE"));
        account.m_min_balance = number("MINIMUMBALANCE");
        account.m_credit_limit = number("CREDITLIMIT");
        account.m_interest_rate = number("INTERESTRATE");
        account.m_payment_due_date_n = mmDateN(string("PAYMENTDUEDATE"));
        account.m_min_payment = number("MINIMUMPAYMENT");
        if (AccountModel::instance().import_data_n(account, businessId)) ++imported;
        else ++conflicts;
    }
    for (const auto& account : accounts) {
        if (failed) break;
        const wxString payload = account.to_json();
        rapidjson::Document localRecord;
        const auto payloadUtf8 = payload.utf8_string();
        localRecord.Parse(payloadUtf8.data());
        const auto foundRecord = remoteRecords.find(account.id().GetValue());
        const bool exists = foundRecord != remoteRecords.end();
        wxString recordId = exists ? foundRecord->second.first : "";
        if (exists) {
            rapidjson::Document remoteRecord;
            remoteRecord.Parse(foundRecord->second.second.c_str());
            if (!localRecord.HasParseError() && !remoteRecord.HasParseError()) {
                if (PocketBaseRecordEqual(localRecord, remoteRecord)) ++skipped;
                else ++conflicts;
                continue;
            }
        }

        CURL* curl = curl_easy_init();
        if (!curl) { result.message = "Could not initialize HTTP client."; failed = true; break; }
        const auto auth = std::string("Authorization: ") + token.utf8_string().data();
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, auth.c_str());
        std::string response;
        long status = 0;
        configurePocketBaseTransfer(curl, headers, response, 20L);
        const wxString method = recordId.empty() ? "POST" : "PATCH";
        wxString writeUrl = base + "/api/collections/ACCOUNTLIST_V1/records";
        if (!recordId.empty()) writeUrl += "/" + recordId;
        const auto url = writeUrl.utf8_string();
        const auto body = payload.utf8_string();
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.utf8_string().data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.length()));
        CURLcode rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        if (rc != CURLE_OK || status < 200 || status >= 300) {
            result.message = rc != CURLE_OK
                ? "Account upload transport error: " + wxString::FromUTF8(curl_easy_strerror(rc)) + " (" + wxString::FromUTF8(url.data()) + ")"
                : wxString::Format("Account upload failed (%ld): %s", status, response);
            failed = true;
            curl_slist_free_all(headers); curl_easy_cleanup(curl); break;
        }
        if (recordId.empty()) ++created; else ++updated;
        curl_slist_free_all(headers); curl_easy_cleanup(curl);
    }
    if (!failed) result.message = wxString::Format("Account sync complete: %zu downloaded, %zu created, %zu updated, %zu unchanged, %zu conflicts.", imported, created, updated, skipped, conflicts);
    result.ok = !failed;
    return result;
}


PocketBaseSyncResult PocketBaseSyncService::syncCategories(const wxString& base, const wxString& token)
{
    PocketBaseSyncResult result;
    if (token.empty()) {
        result.message = "Sign in before uploading categories.";
        return result;
    }
    auto categories = CategoryModel::instance().find_data_a();
    size_t created = 0, updated = 0, skipped = 0, conflicts = 0, imported = 0;
    bool failed = false;
    std::map<long long, std::pair<wxString, std::string>> remoteRecords;
    {
        CURL* curl = curl_easy_init();
        std::string response;
        long status = 0;
        struct curl_slist* headers = nullptr;
        const auto auth = std::string("Authorization: ") + token.utf8_string().data();
        headers = curl_slist_append(headers, auth.c_str());
        const auto url = (base + "/api/collections/CATEGORY_V1/records?perPage=500&page=1").utf8_string();
        if (!curl) {
            result.message = "Could not initialize HTTP client.";
            failed = true;
        } else {
            curl_easy_setopt(curl, CURLOPT_URL, url.data());
            configurePocketBaseTransfer(curl, headers, response, 30L);
            const CURLcode rc = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
            if (rc != CURLE_OK || status < 200 || status >= 300) {
                result.message = rc != CURLE_OK
                    ? "Category list request failed: " + wxString::FromUTF8(curl_easy_strerror(rc))
                    : wxString::Format("Category list request failed (%ld): %s", status, response);
                failed = true;
            } else {
                rapidjson::Document list; list.Parse(response.c_str());
                if (list.HasParseError() || !list.IsObject() || !list.HasMember("items") || !list["items"].IsArray()) {
                    result.message = "PocketBase returned an invalid category list.";
                    failed = true;
                } else {
                    for (const auto& item : list["items"].GetArray()) {
                        if (item.IsObject() && item.HasMember("CATEGID") && item["CATEGID"].IsInt64()
                            && item.HasMember("id") && item["id"].IsString()) {
                            rapidjson::StringBuffer copy;
                            rapidjson::Writer<rapidjson::StringBuffer> writer(copy);
                            item.Accept(writer);
                            remoteRecords[item["CATEGID"].GetInt64()] = {
                                wxString::FromUTF8(item["id"].GetString()),
                                std::string(copy.GetString(), copy.GetSize())
                            };
                        }
                    }
                }
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
    }
    std::set<long long> localIds;
    for (const auto& category : categories) localIds.insert(category.id().GetValue());
    for (const auto& [businessId, remote] : remoteRecords) {
        if (localIds.count(businessId)) continue;
        rapidjson::Document record;
        record.Parse(remote.second.c_str());
        if (record.HasParseError() || !record.IsObject()) { ++conflicts; continue; }
        CategoryData category;
        category.id(businessId);
        category.m_name = record.HasMember("CATEGNAME") && record["CATEGNAME"].IsString()
            ? wxString::FromUTF8(record["CATEGNAME"].GetString()) : wxString();
        category.m_active = !record.HasMember("ACTIVE") || !record["ACTIVE"].IsNumber() || record["ACTIVE"].GetInt() != 0;
        category.m_parent_id_n = record.HasMember("PARENTID") && record["PARENTID"].IsInt64() ? int64(record["PARENTID"].GetInt64()) : int64(-1);
        if (category.m_parent_id_n > 0 && !CategoryModel::instance().get_idN_data_n(category.m_parent_id_n)) { ++conflicts; continue; }
        if (CategoryModel::instance().import_data_n(category, businessId)) ++imported;
        else ++conflicts;
    }
    for (const auto& category : categories) {
        if (failed) break;
        const wxString payload = category.to_json();
        rapidjson::Document localRecord;
        const auto payloadUtf8 = payload.utf8_string();
        localRecord.Parse(payloadUtf8.data());
        const auto foundRecord = remoteRecords.find(category.id().GetValue());
        const bool exists = foundRecord != remoteRecords.end();
        wxString recordId = exists ? foundRecord->second.first : "";
        if (exists) {
            rapidjson::Document remoteRecord;
            remoteRecord.Parse(foundRecord->second.second.c_str());
            if (!localRecord.HasParseError() && !remoteRecord.HasParseError()) {
                if (PocketBaseRecordEqual(localRecord, remoteRecord)) ++skipped;
                else ++conflicts;
                continue;
            }
        }

        CURL* curl = curl_easy_init();
        if (!curl) { result.message = "Could not initialize HTTP client."; failed = true; break; }
        const auto auth = std::string("Authorization: ") + token.utf8_string().data();
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, auth.c_str());
        std::string response;
        long status = 0;
        configurePocketBaseTransfer(curl, headers, response, 20L);
        const wxString method = recordId.empty() ? "POST" : "PATCH";
        wxString writeUrl = base + "/api/collections/CATEGORY_V1/records";
        if (!recordId.empty()) writeUrl += "/" + recordId;
        const auto url = writeUrl.utf8_string();
        const auto body = payload.utf8_string();
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.utf8_string().data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.length()));
        CURLcode rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        if (rc != CURLE_OK || status < 200 || status >= 300) {
            result.message = rc != CURLE_OK
                ? "Category upload transport error: " + wxString::FromUTF8(curl_easy_strerror(rc)) + " (" + wxString::FromUTF8(url.data()) + ")"
                : wxString::Format("Category upload failed (%ld): %s", status, response);
            failed = true;
            curl_slist_free_all(headers); curl_easy_cleanup(curl); break;
        }
        if (recordId.empty()) ++created; else ++updated;
        curl_slist_free_all(headers); curl_easy_cleanup(curl);
    }
    if (!failed) result.message = wxString::Format("Category sync complete: %zu downloaded, %zu created, %zu updated, %zu unchanged, %zu conflicts.", imported, created, updated, skipped, conflicts);
    result.ok = !failed;
    return result;
}


PocketBaseSyncResult PocketBaseSyncService::syncPayees(const wxString& base, const wxString& token)
{
    PocketBaseSyncResult result;
    if (token.empty()) {
        result.message = "Sign in before uploading payees.";
        return result;
    }
    auto payees = PayeeModel::instance().find_data_a();
    size_t created = 0, updated = 0, skipped = 0, conflicts = 0, imported = 0;
    bool failed = false;
    std::map<long long, std::pair<wxString, std::string>> remoteRecords;
    {
        CURL* curl = curl_easy_init();
        std::string response;
        long status = 0;
        struct curl_slist* headers = nullptr;
        const auto auth = std::string("Authorization: ") + token.utf8_string().data();
        headers = curl_slist_append(headers, auth.c_str());
        const auto url = (base + "/api/collections/PAYEE_V1/records?perPage=500&page=1").utf8_string();
        if (!curl) {
            result.message = "Could not initialize HTTP client.";
            failed = true;
        } else {
            curl_easy_setopt(curl, CURLOPT_URL, url.data());
            configurePocketBaseTransfer(curl, headers, response, 30L);
            const CURLcode rc = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
            if (rc != CURLE_OK || status < 200 || status >= 300) {
                result.message = rc != CURLE_OK
                    ? "Payee list request failed: " + wxString::FromUTF8(curl_easy_strerror(rc))
                    : wxString::Format("Payee list request failed (%ld): %s", status, response);
                failed = true;
            } else {
                rapidjson::Document list; list.Parse(response.c_str());
                if (list.HasParseError() || !list.IsObject() || !list.HasMember("items") || !list["items"].IsArray()) {
                    result.message = "PocketBase returned an invalid payee list.";
                    failed = true;
                } else {
                    for (const auto& item : list["items"].GetArray()) {
                        if (item.IsObject() && item.HasMember("PAYEEID") && item["PAYEEID"].IsInt64()
                            && item.HasMember("id") && item["id"].IsString()) {
                            rapidjson::StringBuffer copy;
                            rapidjson::Writer<rapidjson::StringBuffer> writer(copy);
                            item.Accept(writer);
                            remoteRecords[item["PAYEEID"].GetInt64()] = {
                                wxString::FromUTF8(item["id"].GetString()),
                                std::string(copy.GetString(), copy.GetSize())
                            };
                        }
                    }
                }
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
    }
    std::set<long long> localIds;
    for (const auto& payee : payees) localIds.insert(payee.id().GetValue());
    for (const auto& [businessId, remote] : remoteRecords) {
        if (localIds.count(businessId)) continue;
        rapidjson::Document record;
        record.Parse(remote.second.c_str());
        if (record.HasParseError() || !record.IsObject()) { ++conflicts; continue; }
        PayeeData payee;
        payee.id(businessId);
        payee.m_name = record.HasMember("PAYEENAME") && record["PAYEENAME"].IsString() ? wxString::FromUTF8(record["PAYEENAME"].GetString()) : wxString();
        payee.m_number = record.HasMember("NUMBER") && record["NUMBER"].IsString() ? wxString::FromUTF8(record["NUMBER"].GetString()) : wxString();
        payee.m_website = record.HasMember("WEBSITE") && record["WEBSITE"].IsString() ? wxString::FromUTF8(record["WEBSITE"].GetString()) : wxString();
        payee.m_notes = record.HasMember("NOTES") && record["NOTES"].IsString() ? wxString::FromUTF8(record["NOTES"].GetString()) : wxString();
        payee.m_pattern = record.HasMember("PATTERN") && record["PATTERN"].IsString() ? wxString::FromUTF8(record["PATTERN"].GetString()) : wxString();
        payee.m_active = !record.HasMember("ACTIVE") || !record["ACTIVE"].IsNumber() || record["ACTIVE"].GetInt() != 0;
        payee.m_category_id_n = record.HasMember("CATEGID") && record["CATEGID"].IsInt64() ? int64(record["CATEGID"].GetInt64()) : int64(-1);
        if (payee.m_category_id_n > 0 && !CategoryModel::instance().get_idN_data_n(payee.m_category_id_n)) { ++conflicts; continue; }
        if (PayeeModel::instance().import_data_n(payee, businessId)) ++imported;
        else ++conflicts;
    }
    for (const auto& payee : payees) {
        if (failed) break;
        const wxString payload = payee.to_json();
        rapidjson::Document localRecord;
        const auto payloadUtf8 = payload.utf8_string();
        localRecord.Parse(payloadUtf8.data());
        const auto foundRecord = remoteRecords.find(payee.id().GetValue());
        const bool exists = foundRecord != remoteRecords.end();
        wxString recordId = exists ? foundRecord->second.first : "";
        if (exists) {
            rapidjson::Document remoteRecord;
            remoteRecord.Parse(foundRecord->second.second.c_str());
            if (!localRecord.HasParseError() && !remoteRecord.HasParseError()) {
                if (PocketBaseRecordEqual(localRecord, remoteRecord)) ++skipped;
                else ++conflicts;
                continue;
            }
        }

        CURL* curl = curl_easy_init();
        if (!curl) { result.message = "Could not initialize HTTP client."; failed = true; break; }
        const auto auth = std::string("Authorization: ") + token.utf8_string().data();
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, auth.c_str());
        std::string response;
        long status = 0;
        configurePocketBaseTransfer(curl, headers, response, 20L);
        const wxString method = recordId.empty() ? "POST" : "PATCH";
        wxString writeUrl = base + "/api/collections/PAYEE_V1/records";
        if (!recordId.empty()) writeUrl += "/" + recordId;
        const auto url = writeUrl.utf8_string();
        const auto body = payload.utf8_string();
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.utf8_string().data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.length()));
        CURLcode rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        if (rc != CURLE_OK || status < 200 || status >= 300) {
            result.message = rc != CURLE_OK
                ? "Payee upload transport error: " + wxString::FromUTF8(curl_easy_strerror(rc)) + " (" + wxString::FromUTF8(url.data()) + ")"
                : wxString::Format("Payee upload failed (%ld): %s", status, response);
            failed = true;
            curl_slist_free_all(headers); curl_easy_cleanup(curl); break;
        }
        if (recordId.empty()) ++created; else ++updated;
        curl_slist_free_all(headers); curl_easy_cleanup(curl);
    }
    if (!failed) result.message = wxString::Format("Payee sync complete: %zu downloaded, %zu created, %zu updated, %zu unchanged, %zu conflicts.", imported, created, updated, skipped, conflicts);
    result.ok = !failed;
    return result;
}

PocketBaseSyncResult PocketBaseSyncService::syncAssets(const wxString& base, const wxString& token)
{
    PocketBaseSyncResult result;
    if (token.empty()) {
        result.message = "Sign in before syncing assets.";
        return result;
    }
    const auto local = AssetModel::instance().find_data_a();
    std::map<int64, std::pair<wxString, std::string>> remote;
    std::vector<std::string> records;
    wxString error;
    if (!fetchPocketBaseCollection(base, token, "ASSETS_V1", records, error)) {
        result.message = error;
        return result;
    }
    for (const auto& json : records) {
        rapidjson::Document record;
        record.Parse(json.c_str());
        if (record.HasParseError() || !record.IsObject() || !record.HasMember("id")
            || !record["id"].IsString() || pocketBaseInteger(record, "ASSETID") <= 0) {
            ++result.conflicts;
            continue;
        }
        rapidjson::StringBuffer copy;
        rapidjson::Writer<rapidjson::StringBuffer> writer(copy);
        record.Accept(writer);
        remote[pocketBaseInteger(record, "ASSETID")] = {
            wxString::FromUTF8(record["id"].GetString()), std::string(copy.GetString(), copy.GetSize())};
    }
    std::set<int64> localIds, currencies;
    for (const auto& item : local) localIds.insert(item.m_id);
    for (const auto& currency : CurrencyModel::instance().find_data_a()) currencies.insert(currency.m_id);
    for (const auto& [id, value] : remote) {
        if (localIds.count(id)) continue;
        rapidjson::Document record;
        record.Parse(value.second.c_str());
        AssetData asset;
        asset.m_id = id;
        asset.m_name = pocketBaseString(record, "ASSETNAME");
        asset.m_type = AssetType(pocketBaseString(record, "ASSETTYPE"));
        asset.m_status = AssetStatus(pocketBaseString(record, "ASSETSTATUS"));
        asset.m_change = AssetChange(pocketBaseString(record, "VALUECHANGE"));
        asset.m_change_mode = AssetChangeMode(pocketBaseString(record, "VALUECHANGEMODE"));
        asset.m_notes = pocketBaseString(record, "NOTES");
        const auto startDate = pocketBaseString(record, "STARTDATE");
        wxDateTime parsedDate;
        if (asset.m_name.empty() || !parsedDate.ParseISODate(startDate.BeforeFirst('T'))) {
            ++result.conflicts;
            continue;
        }
        asset.m_start_date = mmDate(startDate);
        const int64 currencyId = pocketBaseInteger(record, "CURRENCYID", -1);
        asset.m_currency_id_n = currencyId > 0 ? currencyId : -1;
        if (asset.m_currency_id_n > 0 && !currencies.count(asset.m_currency_id_n)) {
            ++result.conflicts;
            continue;
        }
        asset.m_value = record.HasMember("VALUE") && record["VALUE"].IsNumber() ? record["VALUE"].GetDouble() : 0.0;
        asset.m_change_rate = record.HasMember("VALUECHANGERATE") && record["VALUECHANGERATE"].IsNumber() ? record["VALUECHANGERATE"].GetDouble() : 0.0;
        if (AssetModel::instance().import_data_n(asset, id)) ++result.imported;
        else ++result.conflicts;
    }
    bool failed = false;
    for (const auto& asset : local) {
        const auto found = remote.find(asset.m_id);
        const wxString payload = asset.to_json();
        const auto body = payload.utf8_string();
        rapidjson::Document localRecord;
        localRecord.Parse(body.data());
        if (localRecord.HasParseError() || !localRecord.IsObject()) { ++result.conflicts; continue; }
        if (found != remote.end()) {
            rapidjson::Document remoteRecord;
            remoteRecord.Parse(found->second.second.c_str());
            if (!remoteRecord.HasParseError() && PocketBaseRecordEqual(localRecord, remoteRecord)) ++result.unchanged;
            else ++result.conflicts;
            continue;
        }
        CURL* curl = curl_easy_init();
        if (!curl) { result.message = "Could not initialize HTTP client."; failed = true; break; }
        const auto auth = std::string("Authorization: ") + token.utf8_string().data();
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, auth.c_str());
        std::string response;
        long status = 0;
        const wxString url = base + "/api/collections/ASSETS_V1/records";
        const auto urlUtf8 = url.utf8_string();
        configurePocketBaseTransfer(curl, headers, response, 30L);
        curl_easy_setopt(curl, CURLOPT_URL, urlUtf8.data());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.length()));
        const CURLcode rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        if (rc != CURLE_OK || status < 200 || status >= 300) {
            result.message = rc != CURLE_OK
                ? "Asset upload transport error: " + wxString::FromUTF8(curl_easy_strerror(rc))
                : wxString::Format("Asset upload failed (%ld): %s", status, response);
            failed = true;
            break;
        }
        ++result.created;
    }
    if (!failed) result.message = wxString::Format("Asset sync complete: %zu downloaded, %zu created, %zu unchanged, %zu conflicts.",
        result.imported, result.created, result.unchanged, result.conflicts);
    result.ok = !failed;
    return result;
}

PocketBaseSyncResult PocketBaseSyncService::syncStocks(const wxString& base, const wxString& token)
{
    PocketBaseSyncResult result;
    if (token.empty()) { result.message = "Sign in before syncing stocks."; return result; }
    wxString error;
    std::set<int64> accountIds;
    for (const auto& account : AccountModel::instance().find_data_a()) accountIds.insert(account.m_id);
    auto local = StockModel::instance().find_data_a();
    const bool ok = syncPortfolioCollection(base, token, "STOCK_V1", "STOCKID", local,
        StockModel::instance(), [&accountIds](const rapidjson::Value& record, int64 id, StockData& stock) {
            stock.m_id = id;
            stock.m_account_id_n = pocketBaseInteger(record, "HELDAT", -1);
            if (stock.m_account_id_n > 0 && !accountIds.count(stock.m_account_id_n)) return false;
            stock.m_name = pocketBaseString(record, "STOCKNAME");
            stock.m_symbol = pocketBaseString(record, "SYMBOL");
            stock.m_notes = pocketBaseString(record, "NOTES");
            const auto date = pocketBaseString(record, "PURCHASEDATE");
            wxDateTime parsed;
            if (stock.m_name.empty() || !parsed.ParseISODate(date.BeforeFirst('T'))) return false;
            stock.m_purchase_date = mmDate(date);
            stock.m_num_shares = record.HasMember("NUMSHARES") && record["NUMSHARES"].IsNumber() ? record["NUMSHARES"].GetDouble() : 0.0;
            stock.m_purchase_price = record.HasMember("PURCHASEPRICE") && record["PURCHASEPRICE"].IsNumber() ? record["PURCHASEPRICE"].GetDouble() : 0.0;
            stock.m_current_price = record.HasMember("CURRENTPRICE") && record["CURRENTPRICE"].IsNumber() ? record["CURRENTPRICE"].GetDouble() : 0.0;
            stock.m_purchase_value = record.HasMember("VALUE") && record["VALUE"].IsNumber() ? record["VALUE"].GetDouble() : 0.0;
            stock.m_commission = record.HasMember("COMMISSION") && record["COMMISSION"].IsNumber() ? record["COMMISSION"].GetDouble() : 0.0;
            return true;
        }, result, error);
    result.ok = ok;
    if (ok) result.message = wxString::Format("Stock sync complete: %zu downloaded, %zu created, %zu unchanged, %zu conflicts.", result.imported, result.created, result.unchanged, result.conflicts);
    else result.message = error;
    return result;
}

PocketBaseSyncResult PocketBaseSyncService::syncStockHistory(const wxString& base, const wxString& token)
{
    PocketBaseSyncResult result;
    if (token.empty()) { result.message = "Sign in before syncing stock price history."; return result; }
    wxString error;
    auto local = StockHistoryModel::instance().find_data_a();
    const bool ok = syncPortfolioCollection(base, token, "STOCKHISTORY_V1", "HISTID", local,
        StockHistoryModel::instance(), [](const rapidjson::Value& record, int64 id, StockHistoryData& history) {
            history.m_id = id;
            history.m_symbol = pocketBaseString(record, "SYMBOL");
            const auto date = pocketBaseString(record, "DATE");
            wxDateTime parsed;
            if (history.m_symbol.empty() || !parsed.ParseISODate(date.BeforeFirst('T'))
                || !record.HasMember("VALUE") || !record["VALUE"].IsNumber()) return false;
            history.m_date = mmDate(date);
            history.m_price = record["VALUE"].GetDouble();
            history.m_update_type = UpdateType::from_code(
                static_cast<int>(pocketBaseInteger(record, "UPDTYPE", 0).GetValue()));
            return true;
        }, result, error);
    result.ok = ok;
    if (ok) result.message = wxString::Format("Stock price history sync complete: %zu downloaded, %zu created, %zu unchanged, %zu conflicts.", result.imported, result.created, result.unchanged, result.conflicts);
    else result.message = error;
    return result;
}

namespace {
PocketBaseSyncResult syncTransactionsImpl(const wxString& base, const wxString& token,
    bool includeInvestmentTransactions);

PocketBaseSyncResult syncInvestmentTransactionLinks(const wxString& base, const wxString& token)
{
    PocketBaseSyncResult result;
    wxString error;
    std::set<int64> transactionIds, assetIds, stockIds;
    for (const auto& row : TrxModel::instance().find_data_a()) transactionIds.insert(row.m_id);
    for (const auto& row : AssetModel::instance().find_data_a()) assetIds.insert(row.m_id);
    for (const auto& row : StockModel::instance().find_data_a()) stockIds.insert(row.m_id);
    const auto local = TrxLinkModel::instance().find_data_a();
    const bool ok = syncPortfolioCollection(base, token, "TRANSLINK_V1", "TRANSLINKID", local,
        TrxLinkModel::instance(), [&transactionIds, &assetIds, &stockIds](const rapidjson::Value& record, int64 id, TrxLinkData& link) {
            link.m_id = id;
            link.m_trx_id = pocketBaseInteger(record, "CHECKINGACCOUNTID");
            link.m_ref_type = RefTypeN(pocketBaseString(record, "LINKTYPE"));
            link.m_ref_id = pocketBaseInteger(record, "LINKRECORDID");
            const bool validRef = (link.m_ref_type.id_n() == RefTypeN::e_asset && assetIds.count(link.m_ref_id))
                || (link.m_ref_type.id_n() == RefTypeN::e_stock && stockIds.count(link.m_ref_id));
            return transactionIds.count(link.m_trx_id) && validRef;
        }, result, error);
    result.ok = ok;
    if (ok) result.message = wxString::Format("Investment transaction links synced: %zu downloaded, %zu created, %zu unchanged, %zu conflicts.", result.imported, result.created, result.unchanged, result.conflicts);
    else result.message = error;
    return result;
}

PocketBaseSyncResult syncInvestmentShareDetails(const wxString& base, const wxString& token)
{
    PocketBaseSyncResult result;
    wxString error;
    std::set<int64> transactionIds;
    for (const auto& row : TrxModel::instance().find_data_a()) transactionIds.insert(row.m_id);
    const auto local = TrxShareModel::instance().find_data_a();
    const bool ok = syncPortfolioCollection(base, token, "SHAREINFO_V1", "SHAREINFOID", local,
        TrxShareModel::instance(), [&transactionIds](const rapidjson::Value& record, int64 id, TrxShareData& share) {
            share.m_id = id;
            share.m_trx_id = pocketBaseInteger(record, "CHECKINGACCOUNTID");
            if (!transactionIds.count(share.m_trx_id)) return false;
            share.m_number = record.HasMember("SHARENUMBER") && record["SHARENUMBER"].IsNumber() ? record["SHARENUMBER"].GetDouble() : 0.0;
            share.m_price = record.HasMember("SHAREPRICE") && record["SHAREPRICE"].IsNumber() ? record["SHAREPRICE"].GetDouble() : 0.0;
            share.m_commission = record.HasMember("SHARECOMMISSION") && record["SHARECOMMISSION"].IsNumber() ? record["SHARECOMMISSION"].GetDouble() : 0.0;
            share.m_lot = pocketBaseString(record, "SHARELOT");
            return true;
        }, result, error);
    result.ok = ok;
    if (ok) result.message = wxString::Format("Investment share details synced: %zu downloaded, %zu created, %zu unchanged, %zu conflicts.", result.imported, result.created, result.unchanged, result.conflicts);
    else result.message = error;
    return result;
}
}

PocketBaseSyncResult PocketBaseSyncService::syncInvestments(const wxString& base, const wxString& token)
{
    PocketBaseSyncResult total;
    if (token.empty()) { total.message = "Sign in before syncing investments."; return total; }
    const std::vector<std::pair<const char*, PocketBaseSyncResult(*)(const wxString&, const wxString&)>> steps{
        {"currencies", &PocketBaseSyncService::syncCurrencies},
        {"accounts", &PocketBaseSyncService::syncAccounts},
        {"categories", &PocketBaseSyncService::syncCategories},
        {"payees", &PocketBaseSyncService::syncPayees},
        {"assets", &PocketBaseSyncService::syncAssets},
        {"stocks", &PocketBaseSyncService::syncStocks},
        {"stock price history", &PocketBaseSyncService::syncStockHistory}
    };
    for (const auto& [name, sync] : steps) {
        auto result = sync(base, token);
        total.imported += result.imported;
        total.created += result.created;
        total.unchanged += result.unchanged;
        total.skipped += result.skipped;
        total.conflicts += result.conflicts;
        if (!result.ok) {
            total.message = wxString::Format("Investment sync stopped at %s: %s", name, result.message);
            return total;
        }
    }
    for (const auto& [name, sync] : std::vector<std::pair<const char*, PocketBaseSyncResult(*)(const wxString&, const wxString&)>>{
        {"investment transactions", [](const wxString& instance, const wxString& auth) { return syncTransactionsImpl(instance, auth, true); }},
        {"investment transaction links", &syncInvestmentTransactionLinks},
        {"investment share details", &syncInvestmentShareDetails}}) {
        auto result = sync(base, token);
        total.imported += result.imported;
        total.created += result.created;
        total.unchanged += result.unchanged;
        total.skipped += result.skipped;
        total.conflicts += result.conflicts;
        if (!result.ok) {
            total.message = wxString::Format("Investment sync stopped at %s: %s", name, result.message);
            return total;
        }
    }
    total.ok = true;
    total.message = wxString::Format("Investment sync complete: %zu downloaded, %zu created, %zu unchanged, %zu skipped, %zu conflicts.", total.imported, total.created, total.unchanged, total.skipped, total.conflicts);
    return total;
}

namespace {
PocketBaseSyncResult syncTransactionsImpl(const wxString& base, const wxString& token,
    bool includeInvestmentTransactions)
{
    PocketBaseSyncResult result;
    if (token.empty()) {
        result.message = "Sign in before syncing transactions.";
        return result;
    }

    auto transactions = TrxModel::instance().find_data_a();
    std::set<int64> localIds, localAccounts, localCategories, localPayees;
    std::set<int64> localUnsupported, localLinked, localEligibleInvestment, localInvalidLinks;
    std::set<int64> localStocksLinked, localShares;
    for (const auto& transaction : transactions) localIds.insert(transaction.m_id);
    for (const auto& account : AccountModel::instance().find_data_a()) localAccounts.insert(account.m_id);
    for (const auto& category : CategoryModel::instance().find_data_a()) localCategories.insert(category.m_id);
    for (const auto& payee : PayeeModel::instance().find_data_a()) localPayees.insert(payee.m_id);
    for (const auto& split : TrxSplitModel::instance().find_data_a()) localUnsupported.insert(split.m_trx_id);
    std::set<int64> localAssetIds, localStockIds;
    for (const auto& asset : AssetModel::instance().find_data_a()) localAssetIds.insert(asset.m_id);
    for (const auto& stock : StockModel::instance().find_data_a()) localStockIds.insert(stock.m_id);
    for (const auto& link : TrxLinkModel::instance().find_data_a()) {
        localLinked.insert(link.m_trx_id);
        if ((link.m_ref_type.id_n() == RefTypeN::e_asset && localAssetIds.count(link.m_ref_id))
            || (link.m_ref_type.id_n() == RefTypeN::e_stock && localStockIds.count(link.m_ref_id))) {
            localEligibleInvestment.insert(link.m_trx_id);
            if (link.m_ref_type.id_n() == RefTypeN::e_stock) localStocksLinked.insert(link.m_trx_id);
        } else localInvalidLinks.insert(link.m_trx_id);
    }
    for (const auto& share : TrxShareModel::instance().find_data_a()) localShares.insert(share.m_trx_id);
    for (const auto& id : localLinked) {
        if (!includeInvestmentTransactions || localInvalidLinks.count(id) || !localEligibleInvestment.count(id)
            || (localStocksLinked.count(id) && !localShares.count(id))) localUnsupported.insert(id);
    }
    for (const auto& id : localShares) {
        if (!includeInvestmentTransactions || !localEligibleInvestment.count(id)) localUnsupported.insert(id);
    }

    std::vector<std::string> transactionRecords, splitRecords, linkRecords, shareRecords;
    wxString error;
    for (const auto& [collection, records] : std::vector<std::pair<const char*, std::vector<std::string>*>>{
        {"CHECKINGACCOUNT_V1", &transactionRecords}, {"SPLITTRANSACTIONS_V1", &splitRecords},
        {"TRANSLINK_V1", &linkRecords}, {"SHAREINFO_V1", &shareRecords}}) {
        if (!fetchPocketBaseCollection(base, token, collection, *records, error)) {
            result.message = error;
            return result;
        }
    }

    std::set<int64> remoteUnsupported, remoteLinked, remoteEligibleInvestment, remoteInvalidLinks;
    std::set<int64> remoteStocksLinked, remoteShares, assetIds, stockIds;
    for (const auto& asset : AssetModel::instance().find_data_a()) assetIds.insert(asset.m_id);
    for (const auto& stock : StockModel::instance().find_data_a()) stockIds.insert(stock.m_id);
    for (const auto& json : splitRecords) {
        rapidjson::Document record; record.Parse(json.c_str());
        const int64 id = pocketBaseInteger(record, "TRANSID");
        if (id > 0) remoteUnsupported.insert(id);
    }
    for (const auto& json : linkRecords) {
        rapidjson::Document record; record.Parse(json.c_str());
        const int64 trxId = pocketBaseInteger(record, "CHECKINGACCOUNTID");
        const auto type = pocketBaseString(record, "LINKTYPE");
        const int64 refId = pocketBaseInteger(record, "LINKRECORDID");
        if (trxId <= 0) continue;
        remoteLinked.insert(trxId);
        if ((type == "Asset" && assetIds.count(refId)) || (type == "Stock" && stockIds.count(refId))) {
            remoteEligibleInvestment.insert(trxId);
            if (type == "Stock") remoteStocksLinked.insert(trxId);
        } else remoteInvalidLinks.insert(trxId);
    }
    for (const auto& json : shareRecords) {
        rapidjson::Document record; record.Parse(json.c_str());
        const int64 id = pocketBaseInteger(record, "CHECKINGACCOUNTID");
        if (id > 0) remoteShares.insert(id);
    }
    for (const auto& id : remoteLinked) {
        if (!includeInvestmentTransactions || remoteInvalidLinks.count(id) || !remoteEligibleInvestment.count(id)
            || (remoteStocksLinked.count(id) && !remoteShares.count(id))) remoteUnsupported.insert(id);
    }
    for (const auto& id : remoteShares) {
        if (!includeInvestmentTransactions || !remoteEligibleInvestment.count(id)) remoteUnsupported.insert(id);
    }

    std::map<int64, std::pair<wxString, std::string>> remoteByBusinessId;
    size_t conflicts = 0, skipped = 0, imported = 0, created = 0, unchanged = 0;
    for (const auto& json : transactionRecords) {
        rapidjson::Document record;
        record.Parse(json.c_str());
        if (record.HasParseError() || !record.IsObject()
            || !record.HasMember("id") || !record["id"].IsString()) {
            ++conflicts;
            continue;
        }
        const int64 businessId = pocketBaseInteger(record, "TRANSID");
        if (businessId <= 0) {
            ++conflicts;
            continue;
        }
        rapidjson::StringBuffer copy;
        rapidjson::Writer<rapidjson::StringBuffer> writer(copy);
        record.Accept(writer);
        remoteByBusinessId[businessId] = {wxString::FromUTF8(record["id"].GetString()), std::string(copy.GetString(), copy.GetSize())};
    }

    for (const auto& [businessId, remote] : remoteByBusinessId) {
        if (localIds.count(businessId)) continue;
        rapidjson::Document record;
        record.Parse(remote.second.c_str());
        if (remoteUnsupported.count(businessId) || !supportedTransaction(record)) {
            ++skipped;
            continue;
        }
        TrxData transaction;
        transaction.m_id = businessId;
        transaction.m_account_id = pocketBaseInteger(record, "ACCOUNTID");
        transaction.m_to_account_id_n = pocketBaseInteger(record, "TOACCOUNTID");
        transaction.m_payee_id_n = pocketBaseInteger(record, "PAYEEID");
        transaction.m_category_id_n = pocketBaseInteger(record, "CATEGID");
        transaction.m_type = TrxType(pocketBaseString(record, "TRANSCODE"));
        transaction.m_status = TrxStatus(pocketBaseString(record, "STATUS"));
        transaction.m_amount = record.HasMember("TRANSAMOUNT") && record["TRANSAMOUNT"].IsNumber()
            ? record["TRANSAMOUNT"].GetDouble() : 0.0;
        transaction.m_to_amount = record.HasMember("TOTRANSAMOUNT") && record["TOTRANSAMOUNT"].IsNumber()
            ? record["TOTRANSAMOUNT"].GetDouble() : 0.0;
        transaction.m_number = pocketBaseString(record, "TRANSACTIONNUMBER");
        transaction.m_notes = pocketBaseString(record, "NOTES");
        const wxString transactionDate = pocketBaseString(record, "TRANSDATE");
        wxDateTime parsedDate;
        const bool validDate = parsedDate.ParseISODate(transactionDate.BeforeFirst('T'));
        transaction.m_datetime = mmDateTime(transactionDate);
        wxString updatedTime = pocketBaseString(record, "LASTUPDATEDTIME");
        wxString deletedTime = pocketBaseString(record, "DELETEDTIME");
        if (updatedTime.length() > 10 && updatedTime[10] == ' ') updatedTime[10] = 'T';
        if (deletedTime.length() > 10 && deletedTime[10] == ' ') deletedTime[10] = 'T';
        transaction.m_updated_utc_n = mmDateTimeN(updatedTime);
        transaction.m_deleted_utc_n = mmDateTimeN(deletedTime);
        transaction.m_followup_id = pocketBaseInteger(record, "FOLLOWUPID");
        transaction.m_color = pocketBaseInteger(record, "COLOR");

        const bool validAccounts = transaction.m_account_id > 0 && localAccounts.count(transaction.m_account_id)
            && (transaction.m_type != TrxType(TrxType::e_transfer)
                || (transaction.m_to_account_id_n > 0 && transaction.m_to_account_id_n != transaction.m_account_id
                    && localAccounts.count(transaction.m_to_account_id_n)));
        const bool validPayee = transaction.m_payee_id_n <= 0 || localPayees.count(transaction.m_payee_id_n);
        const bool validCategory = transaction.m_category_id_n <= 0 || localCategories.count(transaction.m_category_id_n);
        if (!validAccounts || !validPayee || !validCategory || !validDate) {
            ++conflicts;
            continue;
        }
        if (TrxModel::instance().import_data_n(transaction, businessId)) ++imported;
        else ++conflicts;
    }

    bool failed = false;
    for (const auto& transaction : transactions) {
        if (localUnsupported.count(transaction.m_id) || !supportedTransaction(transaction)) {
            ++skipped;
            continue;
        }
        const bool validAccounts = transaction.m_account_id > 0 && localAccounts.count(transaction.m_account_id)
            && (transaction.m_type != TrxType(TrxType::e_transfer)
                || (transaction.m_to_account_id_n > 0 && transaction.m_to_account_id_n != transaction.m_account_id
                    && localAccounts.count(transaction.m_to_account_id_n)));
        const bool validPayee = transaction.m_payee_id_n <= 0 || localPayees.count(transaction.m_payee_id_n);
        const bool validCategory = transaction.m_category_id_n <= 0 || localCategories.count(transaction.m_category_id_n);
        if (!validAccounts || !validPayee || !validCategory) {
            ++conflicts;
            continue;
        }

        const auto found = remoteByBusinessId.find(transaction.m_id);
        const bool exists = found != remoteByBusinessId.end();
        if (exists && remoteUnsupported.count(transaction.m_id)) {
            ++skipped;
            continue;
        }
        wxString payload = transaction.to_json();
        rapidjson::Document localRecord;
        const auto payloadUtf8 = payload.utf8_string();
        localRecord.Parse(payloadUtf8.data());
        if (localRecord.HasParseError() || !localRecord.IsObject()) {
            ++conflicts;
            continue;
        }
        normalizeTransactionRecord(localRecord, localRecord.GetAllocator());
        if (exists) {
            rapidjson::Document remoteRecord;
            remoteRecord.Parse(found->second.second.c_str());
            if (remoteRecord.HasParseError() || !remoteRecord.IsObject()) {
                ++conflicts;
                continue;
            }
            normalizeTransactionRecord(remoteRecord, remoteRecord.GetAllocator());
            if (PocketBaseRecordEqual(localRecord, remoteRecord)) ++unchanged;
            else ++conflicts;
            continue;
        }

        rapidjson::Document uploadRecord;
        uploadRecord.Parse(payloadUtf8.data());
        normalizeTransactionRecord(uploadRecord, uploadRecord.GetAllocator(), false);
        rapidjson::StringBuffer bodyBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> bodyWriter(bodyBuffer);
        uploadRecord.Accept(bodyWriter);
        const wxString url = base + "/api/collections/CHECKINGACCOUNT_V1/records";
        const auto urlUtf8 = url.utf8_string();
        const auto auth = std::string("Authorization: ") + token.utf8_string().data();
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, auth.c_str());
        CURL* curl = curl_easy_init();
        if (!curl) {
            curl_slist_free_all(headers);
            result.message = "Could not initialize HTTP client.";
            failed = true;
            break;
        }
        std::string response;
        long status = 0;
        configurePocketBaseTransfer(curl, headers, response, 30L);
        curl_easy_setopt(curl, CURLOPT_URL, urlUtf8.data());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyBuffer.GetString());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(bodyBuffer.GetSize()));
        const CURLcode rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        if (rc != CURLE_OK || status < 200 || status >= 300) {
            result.message = rc != CURLE_OK
                ? "Transaction upload transport error: " + wxString::FromUTF8(curl_easy_strerror(rc)) + " (" + url + ")"
                : wxString::Format("Transaction upload failed (%ld): %s", status, response);
            if (rc == CURLE_OK && transaction.m_amount == 0.0
                && response.find("TRANSAMOUNT") != std::string::npos) {
                result.message = "PocketBase rejected a zero-value transaction: CHECKINGACCOUNT_V1.TRANSAMOUNT is marked Required, which rejects zero. Make this Number field optional in PocketBase and retry. The schema import files have been corrected.";
            }
            failed = true;
            break;
        }
        ++created;
    }

    if (!failed) {
        result.message = wxString::Format("Transaction sync complete: %zu downloaded, %zu created, %zu unchanged, %zu skipped (unsupported or investment-linked), %zu conflicts.",
            imported, created, unchanged, skipped, conflicts);
    }
    result.imported = imported;
    result.created = created;
    result.unchanged = unchanged;
    result.skipped = skipped;
    result.conflicts = conflicts;
    result.ok = !failed;
    return result;
}
}

PocketBaseSyncResult PocketBaseSyncService::syncTransactions(const wxString& base, const wxString& token)
{
    return syncTransactionsImpl(base, token, false);
}
