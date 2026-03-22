/*******************************************************
Copyright (C) 2014 Gabriele-V
Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)
Copyright (C) 2026 George Ef (george.a.ef@gmail.com)

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

#include "base/defs.h"
#include "base/paths.h"
#include "util/_util.h"

#include "model/AccountModel.h"
#include "model/AttachmentModel.h"
#include "model/CategoryModel.h"
#include "model/InfoModel.h"

#include "dialog/AttachmentDialog.h"
#include "dialog/TrxDialog.h"
#include "uicontrols/navigatortypes.h"
#include "webapp.h"

//Expected WebAppVersion
const wxString WebAppParam::ApiExpectedVersion = "1.0.1";

// Internal constants
const wxString mmWebApp::url()
{
    return InfoModel::instance().getString("WEBAPPURL", "");
}

const wxString mmWebApp::guid()
{
    return InfoModel::instance().getString("WEBAPPGUID", "");
}

// Parameters used in services.php
const wxString WebAppParam::CheckApiVersion        = "check_api_version";
const wxString WebAppParam::CheckGuid              = "check_guid";
const wxString WebAppParam::DeleteAccount          = "delete_bankaccount";
const wxString WebAppParam::DeleteAttachment       = "delete_attachment";
const wxString WebAppParam::DeleteCategory         = "delete_category";
const wxString WebAppParam::DeletePayee            = "delete_payee";
const wxString WebAppParam::DeleteTrx              = "delete_group";
const wxString WebAppParam::DownloadAttachment     = "download_attachment";
const wxString WebAppParam::DownloadNewTransaction = "download_transaction";
const wxString WebAppParam::MessageSuccedeed       = "Operation has succeeded";
const wxString WebAppParam::MessageWrongGuid       = "Wrong GUID";
const wxString WebAppParam::ServicesPage           = "services.php";
const wxString WebAppParam::UploadAccount          = "import_bankaccount";
const wxString WebAppParam::UploadCategory         = "import_category";
const wxString WebAppParam::UploadPayee            = "import_payee";

// Return services page URL with GUID inserted
const wxString mmWebApp::services()
{
    return mmWebApp::url() + "/" +
        WebAppParam::ServicesPage + "?" +
        "guid=" + mmWebApp::guid();
}

// Get WebApp Api version
const wxString mmWebApp::apiVersion()
{
    wxString output_message;
    http_get_data(
        mmWebApp::services() + "&" + WebAppParam::CheckApiVersion,
        output_message
    );
    // TODO: check for errors?
    return output_message;
}


/***************
** Functions  **
***************/
// Return function result
bool mmWebApp::result(int& error_code, wxString& output_message)
{
    return (error_code == 0 && output_message == WebAppParam::MessageSuccedeed);
}

// Check if WebApp is enabled
bool mmWebApp::isEnabled()
{
    return (
        InfoModel::instance().getString("WEBAPPURL", "") != wxEmptyString &&
        InfoModel::instance().getString("WEBAPPGUID", "") != wxEmptyString
    );
}

// Check if WebApp Guid is correct
bool mmWebApp::checkGuid()
{
    wxString output_message;
    http_get_data(
        mmWebApp::services() + "&" + WebAppParam::CheckGuid,
        output_message
    );

    if (output_message == WebAppParam::MessageSuccedeed) {
        return true;
    }
    else if (output_message == WebAppParam::MessageWrongGuid) {
        wxString msgStr = wxString() << _t("Wrong WebApp GUID:") << "\n"
            << _t("please check it in network options.") << "\n";
        wxMessageBox(msgStr, _t("Wrong WebApp settings"), wxICON_ERROR);
        return false;
    }
    else {
        wxString msgStr = wxString() << _t("Unable to connect to WebApp:") << "\n"
            << _t("Please check settings and/or Internet connection.") << "\n\n"
            << wxString::Format(_t("Error: %s"), "\n" + output_message + "\n");
        wxMessageBox(msgStr, _t("WebApp connection error"), wxICON_ERROR);
        return false;
    }
}

// Check if WebApp API Version is correct
bool mmWebApp::checkApiVersion()
{
    if (mmWebApp::apiVersion() == WebAppParam::ApiExpectedVersion)
        return true;

    wxString msgStr = _t("Wrong WebApp API version:") + "\n" +
        wxString::Format(_t("WebApp   API version: %s"), mmWebApp::apiVersion()) + "\n" +
        wxString::Format(_t("Expected API version: %s"), WebAppParam::ApiExpectedVersion) + "\n";
    wxMessageBox(msgStr, _t("Wrong WebApp API version"), wxICON_ERROR);
    return false;
}

// POST data as JSON
int mmWebApp::postData(
    const wxString& website,
    const wxString& data_json,
    wxString& output
) {
    const auto content =
        R"(--Custom_Boundary_MMEX_WebApp
Content-Disposition: form-data; name="MMEX_Post"

%s

--Custom_Boundary_MMEX_WebApp--
)";

    return http_post_data(
        website,
        wxString::Format(content, data_json),
        "Content-Type: multipart/form-data; boundary=Custom_Boundary_MMEX_WebApp",
        output
    );
}

// Delete all accounts on WebApp
bool mmWebApp::deleteAccount()
{
    wxString output_message;
    int error_code = http_get_data(
        mmWebApp::services() + "&" + WebAppParam::DeleteAccount,
        output_message
    );
    return mmWebApp::result(error_code, output_message);
}

// Delete all payees on WebApp
bool mmWebApp::deletePayee()
{
    wxString output_message;
    int error_code = http_get_data(
        mmWebApp::services() + "&" + WebAppParam::DeletePayee,
        output_message
    );
    return mmWebApp::result(error_code, output_message);
}

// Delete all categories on WebApp
bool mmWebApp::deleteCategory()
{
    wxString output_message;
    int error_code = http_get_data(
        mmWebApp::services() + "&" + WebAppParam::DeleteCategory,
        output_message
    );
    return mmWebApp::result(error_code, output_message);
}

// Delete one transaction from WebApp
bool mmWebApp::deleteTrxWebId(int64 trx_w_id)
{
    wxString url = mmWebApp::services() + "&" + WebAppParam::DeleteTrx + "="
        << trx_w_id.GetValue();
    wxString output_message;
    int error_code = http_get_data(
        url,
        output_message
    );
    return mmWebApp::result(error_code, output_message);
}

// Upload all accounts from MMEX to WebApp
bool mmWebApp::uploadAccount()
{
    if (!mmWebApp::isEnabled() || !mmWebApp::checkGuid() || !mmWebApp::checkApiVersion())
        return false;

    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("Accounts");

    json_writer.StartArray();

    for (const auto& account_d : AccountModel::instance().find_all(
        AccountCol::COL_ID_ACCOUNTNAME
    )) {
        if (AccountModel::type_id(account_d) == NavigatorTypes::TYPE_ID_INVESTMENT ||
            account_d.is_closed()
        )
            continue;

        json_writer.StartObject();
        json_writer.Key("AccountName");
        json_writer.String(account_d.m_name.utf8_str());
        json_writer.EndObject();
    }

    json_writer.EndArray();
    json_writer.EndObject();

    mmWebApp::deleteAccount();
    wxString account_a_json = /*wxString::FromUTF8*/(json_buffer.GetString());

    wxString output_message;
    int error_code = mmWebApp::postData(
        mmWebApp::services() + "&" + WebAppParam::UploadAccount + "=true",
        account_a_json,
        output_message
    );
    return mmWebApp::result(error_code, output_message);
}

// Upload all payees from MMEX to WebApp
bool mmWebApp::uploadPayee()
{
    if (!mmWebApp::isEnabled() || !mmWebApp::checkGuid() || !mmWebApp::checkApiVersion())
        return false;

    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("Payees");

    json_writer.StartArray();

    wxString cat_name, subcat_name;
    for (const auto& payee_d : PayeeModel::instance().find_all(
        PayeeCol::COL_ID_PAYEENAME
    )) {
        const CategoryData* cat_n = CategoryModel::instance().get_id_data_n(
            payee_d.m_category_id_n
        );
        if (!cat_n) {
            cat_name = "None";
            subcat_name = "None";
        }
        else if (cat_n->m_parent_id_n == -1) {
            cat_name = cat_n->m_name;
            subcat_name = "None";
        }
        else {
            const CategoryData* parent_cat_n = CategoryModel::instance().get_id_data_n(
                cat_n->m_parent_id_n
            );
            if (parent_cat_n && parent_cat_n->m_parent_id_n == -1) {
                cat_name = parent_cat_n->m_name;
                subcat_name = cat_n->m_name;
            }
            else {
                cat_name = "None";
                subcat_name = "None";
            }
        }

        json_writer.StartObject();
        json_writer.Key("PayeeName");
        json_writer.String(payee_d.m_name.utf8_str());
        json_writer.Key("DefCateg");
        json_writer.String(cat_name.utf8_str());
        json_writer.Key("DefSubCateg");
        json_writer.String(subcat_name.utf8_str());
        json_writer.EndObject();
    }

    json_writer.EndArray();
    json_writer.EndObject();

    mmWebApp::deletePayee();
    wxString payee_a_json = /*wxString::FromUTF8*/(json_buffer.GetString());

    wxString output_message;
    int error_code = mmWebApp::postData(
        mmWebApp::services() + "&" + WebAppParam::UploadPayee + "=true",
        payee_a_json,
        output_message
    );
    return mmWebApp::result(error_code, output_message);
}

// Upload all categories from MMEX to WebApp
bool mmWebApp::uploadCategory()
{
    if (!mmWebApp::isEnabled() || !mmWebApp::checkGuid() || !mmWebApp::checkApiVersion())
        return false;

    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("Categories");

    json_writer.StartArray();
    for (const CategoryData& cat_d : CategoryModel::instance().find(
        CategoryCol::PARENTID(-1)
    )) {
        bool first_category_run = true;
        bool sub_category_found = false;

        json_writer.StartObject();
        json_writer.Key("CategoryName");
        json_writer.String(cat_d.m_name.utf8_str());

        for (const auto& subcat_d : CategoryModel::instance().find_data_sub_a(cat_d)) {
            sub_category_found = true;
            if (first_category_run == true) {
                json_writer.Key("SubCategoryName");
                json_writer.String(subcat_d.m_name.utf8_str());
                json_writer.EndObject();

                first_category_run = false;
            }
            else {
                json_writer.StartObject();
                json_writer.Key("CategoryName");
                json_writer.String(cat_d.m_name.utf8_str());

                json_writer.Key("SubCategoryName");
                json_writer.String(subcat_d.m_name.utf8_str());
                json_writer.EndObject();

                first_category_run = false;
            }
        }

        if (sub_category_found == false) {
            json_writer.Key("SubCategoryName");
            json_writer.String("None");
            json_writer.EndObject();
        }
    }

    json_writer.EndArray();
    json_writer.EndObject();

    mmWebApp::deleteCategory();
    wxString cat_a_json = /*wxString::FromUTF8*/(json_buffer.GetString());

    wxString output_message;
    int error_code = mmWebApp::postData(
        mmWebApp::services() + "&" + WebAppParam::UploadCategory + "=true",
        cat_a_json,
        output_message
    );
    return mmWebApp::result(error_code, output_message);
}

// Download new transactions
bool mmWebApp::downloadNewTrx(
    TrxWebDataA& new_trx_wa,
    const bool check_only,
    wxString& error
) {
    wxString new_trx_json;
    CURLcode error_code = http_get_data(
        mmWebApp::services() + "&" + WebAppParam::DownloadNewTransaction,
        new_trx_json
    );

    if (new_trx_json == "null" || new_trx_json.IsEmpty() || error_code != CURLE_OK) {
        error = curl_easy_strerror(error_code);
        return error_code == CURLE_OK;
    }
    if (check_only)
        return true;

    Document j_doc;
    if (j_doc.Parse(new_trx_json.utf8_str()).HasParseError()) {
        error = curl_easy_strerror(CURLE_BAD_CONTENT_ENCODING);
        return false;
    }

    // CHECK: new_trx_w is not cleared (all fields keep theirs previous value)
    TrxWebData new_trx_w;
    new_trx_wa.clear();

    for (auto& j_obj : j_doc.GetObject()) {
        Value j_value = j_obj.value.GetObject();

        if (j_value.HasMember("ID") && j_value["ID"].IsString()) {
            new_trx_w.ID = std::stoll(j_value["ID"].GetString());
        }

        if (j_value.HasMember("Date") && j_value["Date"].IsString()) {
            mmParseISODate(wxString::FromUTF8(j_value["Date"].GetString()), new_trx_w.Date);
        }

        if (j_value.HasMember("Amount") && j_value["Amount"].IsString()) {
            wxString(j_value["Amount"].GetString()).ToCDouble(&new_trx_w.Amount);
        }

        if (j_value.HasMember("Account") && j_value["Account"].IsString()) {
            new_trx_w.Account = wxString::FromUTF8(j_value["Account"].GetString());
        }

        if (j_value.HasMember("ToAccount") && j_value["ToAccount"].IsString()) {
            new_trx_w.ToAccount = wxString::FromUTF8(j_value["ToAccount"].GetString());
        }

        if (j_value.HasMember("Status") && j_value["Status"].IsString()) {
            new_trx_w.Status = wxString::FromUTF8(j_value["Status"].GetString());
        }

        if (j_value.HasMember("Type") && j_value["Type"].IsString()) {
            new_trx_w.Type = wxString::FromUTF8(j_value["Type"].GetString());
        }

        if (j_value.HasMember("Payee") && j_value["Payee"].IsString()) {
            wxString payee_name = wxString::FromUTF8(j_value["Payee"].GetString());
            if (payee_name == "None" || payee_name.IsEmpty()) {
                payee_name = _t("Unknown");
            }
            new_trx_w.Payee = payee_name;
        }

        if (j_value.HasMember("Category") && j_value["Category"].IsString()) {
            wxString cat_name = wxString::FromUTF8(j_value["Category"].GetString());
            cat_name.Replace(":", "|");
            if (cat_name == "None" || cat_name.IsEmpty()) {
                cat_name = _t("Unknown");
            }
            new_trx_w.Category = cat_name;
        }

        if (j_value.HasMember("SubCategory") && j_value["SubCategory"].IsString()) {
            wxString subcat_name = wxString::FromUTF8(j_value["SubCategory"].GetString());
            subcat_name.Replace(":", "|");
            if (subcat_name == "None" || subcat_name.IsEmpty()) {
                // Empty and not "Unknown" because it could be a category
                // without any subcategory: if categories are not used at all,
                // Unknown as category is enough
                subcat_name = wxEmptyString;
            }
            new_trx_w.SubCategory = subcat_name;
        }

        if (j_value.HasMember("Notes") && j_value["Notes"].IsString()) {
            new_trx_w.Notes = wxString::FromUTF8(j_value["Notes"].GetString());
        }

        if (j_value.HasMember("Attachments") && j_value["Attachments"].IsString()) {
            new_trx_w.Attachments = wxString::FromUTF8(j_value["Attachments"].GetString());
        }

        new_trx_wa.push_back(new_trx_w);
    }
    return true;
}

// Insert transaction in MMEX desktop, returns MMEX transaction ID
int64 mmWebApp::insertNewTrx(TrxWebData& trx_w)
{
    int64 trx_d_id = 0;
    int64 account_id = -1;
    int64 to_account_id = -1;
    int64 payee_id = -1;
    int64 cat_id = -1;
    TrxStatus trx_status = TrxStatus();

    //Search Account
    const AccountData* account_n = AccountModel::instance().get_name_data_n(trx_w.Account);
    wxString account_name;
    mmDateN account_open_date_n = mmDateN();
    if (account_n) {
        account_id = account_n->m_id;
        account_name = account_n->m_name;
        account_open_date_n = account_n->m_open_date;
        trx_status = TrxStatus(trx_w.Status);
    }
    else {
        trx_status = TrxStatus(TrxStatus::e_followup);

        // Search first bank account
        for (const auto& first_account_d : AccountModel::instance().find_all(
            AccountCol::COL_ID_ACCOUNTNAME
        )) {
            if (AccountModel::type_id(first_account_d) != NavigatorTypes::TYPE_ID_INVESTMENT &&
                AccountModel::type_id(first_account_d) != NavigatorTypes::TYPE_ID_TERM
            ) {
                account_name = first_account_d.m_name;
                account_id = first_account_d.m_id;
                account_open_date_n = first_account_d.m_open_date;
                break;
            }
        }

        wxString msgStr =
            wxString::Format(_t("Account %s not found."), trx_w.Account) << "\n\n" <<
            wxString::Format(_t("Transaction will be inserted with the first bank account:\n"
                "'%s' and marked as  'Follow Up'"
            ), account_name) << "\n";
        wxMessageBox(msgStr, _t("Wrong WebApp account"), wxICON_ERROR);
    }

    // Search ToAccount
    const AccountData* to_account_n = nullptr;
    if (trx_w.ToAccount != "None") {
        to_account_n = AccountModel::instance().get_name_data_n(trx_w.ToAccount);
        if (to_account_n)
            to_account_id = to_account_n->m_id;
    }

    //Search or insert Category
    const CategoryData* cat_n = CategoryModel::instance().get_key_data_n(
        trx_w.Category, int64(-1)
    );
    if (cat_n)
        cat_id = cat_n->m_id;
    else {
        CategoryData new_cat_d = CategoryData();
        new_cat_d.m_name = trx_w.Category;
        CategoryModel::instance().add_data_n(new_cat_d);
        cat_id = new_cat_d.m_id;
    }

    // Search or insert SubCategory
    if (!trx_w.SubCategory.IsEmpty()) {
        const CategoryData* subcat_n = CategoryModel::instance().get_key_data_n(
            trx_w.SubCategory, cat_id
        );
        if (subcat_n) {
            cat_id = subcat_n->m_id;
        }
        else if (cat_id != -1) {
            CategoryData new_subcat_d = CategoryData();
            new_subcat_d.m_name        = trx_w.SubCategory;
            new_subcat_d.m_parent_id_n = cat_id;
            CategoryModel::instance().add_data_n(new_subcat_d);
            cat_id = new_subcat_d.m_id;
        }
    }

    // Search or insert Payee
    const PayeeData* payee_n = PayeeModel::instance().get_name_data_n(trx_w.Payee);
    if (payee_n) {
        payee_id = payee_n->m_id;
    }
    else {
        PayeeData new_payee_d = PayeeData();
        new_payee_d.m_name          = trx_w.Payee;
        new_payee_d.m_category_id_n = cat_id;
        PayeeModel::instance().add_data_n(new_payee_d);
        payee_id = new_payee_d.m_id;
    }

    // Create New Transaction
    TrxData new_trx_d = TrxData();
    mmDateTime trx_datetime = mmDateTime(trx_w.Date);
    if ((mmDate(trx_datetime) < account_open_date_n.value()) ||
        (to_account_n && mmDate(trx_datetime) < to_account_n->m_open_date)
    ) {
        wxString msgStr = wxString::Format("%s: %s / %s: %s\n\n%s\n%s",
            _t("Account"), account_name,
            _t("Date"), mmDate(trx_datetime).isoDate(),
            _t("The opening date for the account is later than the date of this transaction"),
            _t("Today will be used as the transaction date")
        );
        wxMessageBox(msgStr, _t("Invalid Date"), wxICON_ERROR);
        trx_datetime = mmDateTime::now();
    }

    new_trx_d.m_date_time       = trx_datetime;
    new_trx_d.m_type            = TrxType(trx_w.Type);
    new_trx_d.m_status          = trx_status;
    new_trx_d.m_account_id      = account_id;
    new_trx_d.m_to_account_id_n = to_account_id;
    new_trx_d.m_payee_id_n      = payee_id;
    new_trx_d.m_category_id_n   = cat_id;
    new_trx_d.m_amount          = trx_w.Amount;
    new_trx_d.m_to_amount       = trx_w.Amount;
    new_trx_d.m_number          = "";
    new_trx_d.m_notes           = trx_w.Notes;
    new_trx_d.m_followup_id     = -1;
    new_trx_d.m_color           = -1;
    TrxModel::instance().add_data_n(new_trx_d);
    trx_d_id = new_trx_d.m_id;

    if (trx_d_id <= 0)
        return trx_d_id;

    if (!trx_w.Attachments.IsEmpty()) {
        const wxString attachment_folder = mmex::getPathAttachment(
            mmAttachmentManage::InfotablePathSetting()
        );
        if (attachment_folder == wxEmptyString || !wxDirExists(attachment_folder)) {
            TrxModel::instance().purge_id(trx_d_id);

            wxString msgStr = wxString()
                << _t("Unable to download attachments from the WebApp.") << "\n"
                << _t("Attachments folder not set or unavailable.") << "\n" << "\n"
                << _t("Transaction not downloaded:") << "\n"
                << _t("Please fix the attachments folder or delete the attachments from the WebApp.") << "\n";
            wxMessageBox(msgStr, _t("Attachment folder error"), wxICON_ERROR);
            return -1;
        }

        int attachment_number = 0;
        wxArrayString attachment_w_name_a;
        wxStringTokenizer tkz1(trx_w.Attachments, (';'), wxTOKEN_RET_EMPTY_ALL);
        while (tkz1.HasMoreTokens()) {
            attachment_w_name_a.Add(tkz1.GetNextToken());
        }
        for (size_t i = 0; i < attachment_w_name_a.GetCount(); ++i) {
            attachment_number += 1;
            const wxString attachment_w_name = attachment_w_name_a.Item(i);
            wxString error = "";
            const wxString attachment_filename = downloadAttachment(
                attachment_w_name,
                trx_d_id,
                attachment_number,
                error
            );
            if (attachment_filename == wxEmptyString) {
                TrxModel::instance().purge_id(trx_d_id);

                wxString msgStr = wxString() <<
                    _t("Unable to download attachments from the WebApp.") << "\n"
                    << error << "\n" << "\n"
                    << _t("Transaction not downloaded: please retry to download transactions") << "\n";
                wxMessageBox(msgStr, _t("Attachment download error"), wxICON_ERROR);
                return -1;
            }

            AttachmentData new_att_d = AttachmentData();
            new_att_d.m_ref_type_n  = RefTypeN(RefTypeN::e_trx);
            new_att_d.m_ref_id      = trx_d_id;
            new_att_d.m_description = _t("Attachment") + "_" << attachment_number;
            new_att_d.m_filename    = attachment_filename;
            AttachmentModel::instance().add_data_n(new_att_d);
        }
    }

    deleteTrxWebId(trx_w.ID);
    return trx_d_id;
}

// Download attachment from WebApp
wxString mmWebApp::downloadAttachment(
    const wxString& attachment_w_name,
    int64 trx_id,
    int attachment_number,
    wxString& error
) {
    const wxString file_ext = wxFileName(attachment_w_name).GetExt().MakeLower();
    const wxString file_name =
        TrxModel::s_ref_type.name_n() + "_" +
        wxString::Format("%lld", trx_id) +
        "_Attach" + wxString::Format("%i", attachment_number) +
        "." + file_ext;
    const wxString file_path =
        mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting()) +
        TrxModel::s_ref_type.name_n() + wxFileName::GetPathSeparator() +
        file_name;

    CURLcode curlStatus = http_download_file(
        mmWebApp::services() + "&" + WebAppParam::DownloadAttachment + "=" + attachment_w_name,
        file_path
    );
    if (curlStatus == CURLE_OK)
        return file_name;
    else {
        error = curl_easy_strerror(curlStatus);
        return wxEmptyString;
    }
}

// Get one attachment from WebApp
bool mmWebApp::downloadAttachmentFile(wxString& file_name, wxString& error)
{
    const wxString file_ext = wxFileName(file_name).GetExt().MakeLower();
    const wxString orig_file_name = file_name;
    file_name = wxFileName::CreateTempFileName(file_name);

    CURLcode curlStatus = http_download_file(
        mmWebApp::services() + "&" + WebAppParam::DownloadAttachment + "=" + orig_file_name,
        file_name
    );
    if (curlStatus == CURLE_OK) {
        wxString file_name_ext = file_name + "." + file_ext;
        if (wxRenameFile(file_name, file_name_ext)) {
            file_name = file_name_ext;
            return true;
        }
    }
    else {
        error = curl_easy_strerror(curlStatus);
    }
    return false;
}

