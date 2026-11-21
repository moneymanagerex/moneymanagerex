#include "PocketBaseSync.h"
#include "base/_defs.h"
#include "model/CurrencyModel.h"
#include "model/AccountModel.h"
#include "model/CategoryModel.h"
#include "data/CategoryData.h"
#include "data/AccountData.h"
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <map>
#include <set>
#include <string>

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
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, PocketBaseWrite);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
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
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, PocketBaseWrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
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
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, PocketBaseWrite);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
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
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, PocketBaseWrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
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
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, PocketBaseWrite);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
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
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, PocketBaseWrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
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
