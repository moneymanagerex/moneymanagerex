/*******************************************************
Copyright (C) 2014 Gabriele-V
Copyright (C) 2022  Mark Whalley (mark@ipx.co.uk)

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

//Internal constants
const wxString mmWebApp::getUrl()
{
    wxString Url = InfoModel::instance().getString("WEBAPPURL", "");
    return Url;
}

const wxString mmWebApp::getGuid()
{
    return InfoModel::instance().getString("WEBAPPGUID", "");
}

//Parameters used in services.php
const wxString WebAppParam::ServicesPage           = "services.php";
const wxString WebAppParam::CheckGuid              = "check_guid";
const wxString WebAppParam::CheckApiVersion        = "check_api_version";
const wxString WebAppParam::DeleteAccount          = "delete_bankaccount";
const wxString WebAppParam::ImportAccount          = "import_bankaccount";
const wxString WebAppParam::DeletePayee            = "delete_payee";
const wxString WebAppParam::ImportPayee            = "import_payee";
const wxString WebAppParam::DeleteCategory         = "delete_category";
const wxString WebAppParam::ImportCategory         = "import_category";
const wxString WebAppParam::DeleteOneTransaction   = "delete_group";
const wxString WebAppParam::DownloadNewTransaction = "download_transaction";
const wxString WebAppParam::DownloadAttachments    = "download_attachment";
const wxString WebAppParam::DeleteAttachment       = "delete_attachment";
const wxString WebAppParam::MessageSuccedeed       = "Operation has succeeded";
const wxString WebAppParam::MessageWrongGuid       = "Wrong GUID";

// Return services page URL with GUID inserted
const wxString mmWebApp::getServicesPageURL()
{
    return mmWebApp::getUrl() + "/" + WebAppParam::ServicesPage + "?" + "guid=" + mmWebApp::getGuid();
}

//Get WebApp Api version
const wxString mmWebApp::WebApp_getApiVersion()
{
    wxString outputMessage;
    http_get_data(mmWebApp::getServicesPageURL() + "&" + WebAppParam::CheckApiVersion, outputMessage);
    // TODO: check for errors?
    return outputMessage;
}


/***************
** Functions  **
***************/
//Return function result
bool mmWebApp::returnResult(int& ErrorCode, wxString& outputMessage)
{
    if (ErrorCode == 0 && outputMessage == WebAppParam::MessageSuccedeed)
        return true;
    else
        return false;
}

//Check if WebApp is enabled
bool mmWebApp::WebApp_CheckEnabled()
{
    if (InfoModel::instance().getString("WEBAPPURL", "") != wxEmptyString
        && InfoModel::instance().getString("WEBAPPGUID", "") != wxEmptyString)
        return true;
    else
        return false;
}

//Check guid
bool mmWebApp::WebApp_CheckGuid()
{
    wxString outputMessage;
    http_get_data(mmWebApp::getServicesPageURL() + "&" + WebAppParam::CheckGuid, outputMessage);

    if (outputMessage == WebAppParam::MessageSuccedeed)
        return true;
    else if (outputMessage == WebAppParam::MessageWrongGuid)
    {
        wxString msgStr = wxString() << _t("Wrong WebApp GUID:") << "\n"
            << _t("please check it in network options.") << "\n";
        wxMessageBox(msgStr, _t("Wrong WebApp settings"), wxICON_ERROR);
        return false;
    }
    else
    {
        wxString msgStr = wxString() << _t("Unable to connect to WebApp:") << "\n"
            << _t("Please check settings and/or Internet connection.") << "\n\n"
            << wxString::Format(_t("Error: %s"), "\n" + outputMessage + "\n");
        wxMessageBox(msgStr, _t("WebApp connection error"), wxICON_ERROR);
        return false;
    }
}

//Check WebApp Api version
bool mmWebApp::WebApp_CheckApiVersion()
{
    if (mmWebApp::WebApp_getApiVersion() != WebAppParam::ApiExpectedVersion)
    {
        wxString msgStr = _t("Wrong WebApp API version:") + "\n"
            + wxString::Format(_t("WebApp   API version: %s"), mmWebApp::WebApp_getApiVersion()) + "\n"
            + wxString::Format(_t("Expected API version: %s"), WebAppParam::ApiExpectedVersion) + "\n";
        wxMessageBox(msgStr, _t("Wrong WebApp API version"), wxICON_ERROR);
        return false;
    }
    else
        return true;
}

//POST data as JSON
int mmWebApp::WebApp_SendJson(wxString& Website, const wxString& JsonData, wxString& Output)
{
    const auto temp =
        R"(--Custom_Boundary_MMEX_WebApp
Content-Disposition: form-data; name="MMEX_Post"

%s

--Custom_Boundary_MMEX_WebApp--
)";

    const auto ContentType = "Content-Type: multipart/form-data; boundary=Custom_Boundary_MMEX_WebApp";
    const auto Text = wxString::Format(temp, JsonData);
    return http_post_data(Website, Text, ContentType, Output);
}


//Delete all account on WebApp
bool mmWebApp::WebApp_DeleteAllAccount()
{
    wxString outputMessage;
    int ErrorCode = http_get_data(mmWebApp::getServicesPageURL() + "&" + WebAppParam::DeleteAccount, outputMessage);

    return mmWebApp::returnResult(ErrorCode, outputMessage);
}

// Update Account on WebApp
bool mmWebApp::WebApp_UpdateAccount()
{
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

    mmWebApp::WebApp_DeleteAllAccount();
    wxString update_account_url = mmWebApp::getServicesPageURL() + "&" + WebAppParam::ImportAccount + "=true";
    wxString json_account_list = /*wxString::FromUTF8*/(json_buffer.GetString());
    wxString output_message;

    int error_code = mmWebApp::WebApp_SendJson(update_account_url, json_account_list, output_message);

    return mmWebApp::returnResult(error_code, output_message);
}

//Delete all payee on WebApp
bool mmWebApp::WebApp_DeleteAllPayee()
{
    wxString outputMessage;
    int ErrorCode = http_get_data(mmWebApp::getServicesPageURL() + "&" + WebAppParam::DeletePayee, outputMessage);

    return mmWebApp::returnResult(ErrorCode, outputMessage);
}

//Update payee on WebApp
bool mmWebApp::WebApp_UpdatePayee()
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("Payees");

    json_writer.StartArray();

    wxString def_category_name, def_subcategory_name;
    for (const auto& payee_d : PayeeModel::instance().find_all(PayeeCol::COL_ID_PAYEENAME)) {
        const CategoryData* def_category = CategoryModel::instance().get_id_data_n(payee_d.m_category_id_n);
        if (def_category) {
            if (def_category->m_parent_id_n == -1) {
                def_category_name = def_category->m_name;
                def_subcategory_name = "None";
            }
            else {
                const CategoryData* parent_category = CategoryModel::instance().get_id_data_n(def_category->m_parent_id_n);
                if (parent_category != nullptr && parent_category->m_parent_id_n == -1) {
                    def_category_name = parent_category->m_name;
                    def_subcategory_name = def_category->m_name;
                }
                else {
                    def_category_name = "None";
                    def_subcategory_name = "None";
                }
            }
        }
        else {
            def_category_name = "None";
            def_subcategory_name = "None";
        }

        json_writer.StartObject();
        json_writer.Key("PayeeName");
        json_writer.String(payee_d.m_name.utf8_str());
        json_writer.Key("DefCateg");
        json_writer.String(def_category_name.utf8_str());
        json_writer.Key("DefSubCateg");
        json_writer.String(def_subcategory_name.utf8_str());
        json_writer.EndObject();
    }

    json_writer.EndArray();
    json_writer.EndObject();

    mmWebApp::WebApp_DeleteAllPayee();
    wxString update_payee_url = mmWebApp::getServicesPageURL() + "&" + WebAppParam::ImportPayee + "=true";
    wxString json_payee_list = /*wxString::FromUTF8*/(json_buffer.GetString());
    wxString output_message;
    int error_code = mmWebApp::WebApp_SendJson(update_payee_url, json_payee_list, output_message);

    return mmWebApp::returnResult(error_code, output_message);
}

//Delete all category on WebApp
bool mmWebApp::WebApp_DeleteAllCategory()
{
    wxString outputMessage;
    int ErrorCode = http_get_data(mmWebApp::getServicesPageURL() + "&" + WebAppParam::DeleteCategory, outputMessage);

    return mmWebApp::returnResult(ErrorCode, outputMessage);
}

//Update category on WebApp
bool mmWebApp::WebApp_UpdateCategory()
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("Categories");

    json_writer.StartArray();
    for (const CategoryData& category_d : CategoryModel::instance().find(
        CategoryCol::PARENTID(-1)
    )) {
        bool first_category_run = true;
        bool sub_category_found = false;

        json_writer.StartObject();
        json_writer.Key("CategoryName");
        json_writer.String(category_d.m_name.utf8_str());

        for (const auto& subcategory_d : CategoryModel::instance().find_data_sub_a(category_d)) {
            sub_category_found = true;
            if (first_category_run == true) {
                json_writer.Key("SubCategoryName");
                json_writer.String(subcategory_d.m_name.utf8_str());
                json_writer.EndObject();

                first_category_run = false;
            }
            else {
                json_writer.StartObject();
                json_writer.Key("CategoryName");
                json_writer.String(category_d.m_name.utf8_str());

                json_writer.Key("SubCategoryName");
                json_writer.String(subcategory_d.m_name.utf8_str());
                json_writer.EndObject();

                first_category_run = false;
            }
        }

        if (sub_category_found == false)
        {
            json_writer.Key("SubCategoryName");
            json_writer.String("None");
            json_writer.EndObject();
        }
    }

    json_writer.EndArray();
    json_writer.EndObject();

    mmWebApp::WebApp_DeleteAllCategory();
    wxString update_category_url = mmWebApp::getServicesPageURL() + "&" + WebAppParam::ImportCategory + "=true";
    wxString category_list = /*wxString::FromUTF8*/(json_buffer.GetString());
    wxString output_message;
    int error_code = mmWebApp::WebApp_SendJson(update_category_url, category_list, output_message);

    return mmWebApp::returnResult(error_code, output_message);
}

//Download new transactions
bool mmWebApp::WebApp_DownloadNewTransaction(WebTranVector& WebAppTransactions_, const bool CheckOnly, wxString& Error)
{
    wxString NewTransactionJSON;
    CURLcode ErrorCode = http_get_data(mmWebApp::getServicesPageURL() + "&" + WebAppParam::DownloadNewTransaction, NewTransactionJSON);

    if (NewTransactionJSON == "null" || NewTransactionJSON.IsEmpty() || ErrorCode != CURLE_OK)
    {
        Error = curl_easy_strerror(ErrorCode);
        return ErrorCode == CURLE_OK;
    }
    else if (CheckOnly)
        return true;
    else
    {
        Document j_doc;
        if (j_doc.Parse(NewTransactionJSON.utf8_str()).HasParseError())
        {
            Error = curl_easy_strerror(CURLE_BAD_CONTENT_ENCODING);
            return false;
        }

        //Define variables
        webtran_holder WebTran;
        WebAppTransactions_.clear();

        for (auto& m : j_doc.GetObject())
        {
            Value trx = m.value.GetObject();

            if (trx.HasMember("ID") && trx["ID"].IsString()) {
                WebTran.ID = std::stoll(trx["ID"].GetString());
            }

            if (trx.HasMember("Date") && trx["Date"].IsString()) {
                wxDateTime d;
                mmParseISODate(wxString::FromUTF8(trx["Date"].GetString()), d);
                WebTran.Date = d;
            }

            if (trx.HasMember("Amount") && trx["Amount"].IsString()) {
                wxString sAmount = trx["Amount"].GetString();
                double dAmount;
                sAmount.ToCDouble(&dAmount);
                WebTran.Amount = dAmount;
            }

            if (trx.HasMember("Account") && trx["Account"].IsString()) {
                WebTran.Account = wxString::FromUTF8(trx["Account"].GetString());
            }

            if (trx.HasMember("ToAccount") && trx["ToAccount"].IsString()) {
                WebTran.ToAccount = wxString::FromUTF8(trx["ToAccount"].GetString());
            }

            if (trx.HasMember("Status") && trx["Status"].IsString()) {
                WebTran.Status = wxString::FromUTF8(trx["Status"].GetString());
            }

            if (trx.HasMember("Type") && trx["Type"].IsString()) {
                WebTran.Type = wxString::FromUTF8(trx["Type"].GetString());
            }


            if (trx.HasMember("Payee") && trx["Payee"].IsString()) {
                wxString Payee = wxString::FromUTF8(trx["Payee"].GetString());
                if (Payee == "None" || Payee.IsEmpty()) {
                    Payee = _t("Unknown");
                }
                WebTran.Payee = Payee;
            }

            if (trx.HasMember("Category") && trx["Category"].IsString()) {
                wxString Category = wxString::FromUTF8(trx["Category"].GetString());
                Category.Replace(":", "|");
                if (Category == "None" || Category.IsEmpty()) {
                    Category = _t("Unknown");
                }
                WebTran.Category = Category;
            }

            if (trx.HasMember("SubCategory") && trx["SubCategory"].IsString()) {
                wxString SubCategory = wxString::FromUTF8(trx["SubCategory"].GetString());
                SubCategory.Replace(":", "|");
                if (SubCategory == "None" || SubCategory.IsEmpty()) {
                    //Empty and not "Unknown" because it could be a category without any subcategory: if categories are not used at all, Unknown as category is enough
                    SubCategory = wxEmptyString;
                }
                WebTran.SubCategory = SubCategory;
            }

            if (trx.HasMember("Notes") && trx["Notes"].IsString()) {
                WebTran.Notes = wxString::FromUTF8(trx["Notes"].GetString());
            }

            if (trx.HasMember("Attachments") && trx["Attachments"].IsString()) {
                WebTran.Attachments = wxString::FromUTF8(trx["Attachments"].GetString());
            }

            WebAppTransactions_.push_back(WebTran);
        }
        return true;
    }
}

//Insert new transaction
int64 mmWebApp::MMEX_InsertNewTransaction(webtran_holder& WebAppTrans)
{
    int64 DeskNewTrID = 0;
    bool bDeleteTrWebApp = false;

    int64 AccountID = -1;
    int64 ToAccountID = -1;
    int64 payeeID = -1;
    int64 category_id = -1;
    TrxStatus trx_status = TrxStatus();

    //Search Account
    const AccountData* account_n = AccountModel::instance().get_name_data_n(WebAppTrans.Account);
    wxString accountName;
    mmDateN accountInitialDate = mmDateN();
    if (account_n != nullptr) {
        AccountID = account_n->m_id;
        accountName = account_n->m_name;
        accountInitialDate = account_n->m_open_date;
        trx_status = TrxStatus(WebAppTrans.Status);
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
                accountName = first_account_d.m_name;
                AccountID = first_account_d.m_id;
                accountInitialDate = first_account_d.m_open_date;
                break;
            }
        }

        wxString msgStr = wxString::Format(_t("Account %s not found."), WebAppTrans.Account)
            << "\n\n"
            << wxString::Format(_t("Transaction will be inserted with the first bank account:\n"
            "'%s' and marked as  'Follow Up'")
            , accountName) << "\n";
        wxMessageBox(msgStr, _t("Wrong WebApp account"), wxICON_ERROR);
    }

    // Search ToAccount
    const AccountData* ToAccount = nullptr;
    if (WebAppTrans.ToAccount != "None") {
        ToAccount = AccountModel::instance().get_name_data_n(WebAppTrans.ToAccount);
        if (ToAccount)
            ToAccountID = ToAccount->m_id;
    }

    //Search or insert Category
    const CategoryData* category_n = CategoryModel::instance().get_key_data_n(
        WebAppTrans.Category, int64(-1)
    );
    if (category_n != nullptr)
        category_id = category_n->m_id;
    else {
        CategoryData new_category_d = CategoryData();
        new_category_d.m_name = WebAppTrans.Category;
        CategoryModel::instance().add_data_n(new_category_d);
        category_id = new_category_d.id();
    }

    // Search or insert SubCategory
    if (!WebAppTrans.SubCategory.IsEmpty()) {
        const CategoryData* subcategory_n = CategoryModel::instance().get_key_data_n(
            WebAppTrans.SubCategory, category_id
        );
        if (subcategory_n) {
            category_id = subcategory_n->m_id;
        }
        else if (category_id != -1) {
            CategoryData new_subcategory_d = CategoryData();
            new_subcategory_d.m_name        = WebAppTrans.SubCategory;
            new_subcategory_d.m_parent_id_n = category_id;
            CategoryModel::instance().add_data_n(new_subcategory_d);
            category_id = new_subcategory_d.id();
        }
    }

    // Search or insert Payee
    const PayeeData* payee_n = PayeeModel::instance().get_name_data_n(WebAppTrans.Payee);
    if (payee_n) {
        payeeID = payee_n->m_id;
    }
    else {
        PayeeData new_payee_d = PayeeData();
        new_payee_d.m_name          = WebAppTrans.Payee;
        new_payee_d.m_category_id_n = category_id;
        PayeeModel::instance().add_data_n(new_payee_d);
        payeeID = new_payee_d.id();
    }

    // Create New Transaction
    TrxData new_trx_d = TrxData();
    wxDateTime trx_datetime = WebAppTrans.Date;
    if ((mmDate(trx_datetime) < accountInitialDate.value()) ||
        (ToAccount && mmDate(trx_datetime) < ToAccount->m_open_date)
    ) {
        wxString msgStr = wxString::Format("%s: %s / %s: %s\n\n%s\n%s",
            _t("Account"), accountName,
            _t("Date"), mmDate(trx_datetime).isoDate(),
            _t("The opening date for the account is later than the date of this transaction"),
            _t("Today will be used as the transaction date")
        );
        wxMessageBox(msgStr, _t("Invalid Date"), wxICON_ERROR);
        trx_datetime = wxDate::Today();
    }

    new_trx_d.TRANSDATE         = trx_datetime.FormatISOCombined();
    new_trx_d.m_type            = TrxType(WebAppTrans.Type);
    new_trx_d.m_status          = trx_status;
    new_trx_d.m_account_id      = AccountID;
    new_trx_d.m_to_account_id_n = ToAccountID;
    new_trx_d.m_payee_id_n      = payeeID;
    new_trx_d.m_category_id_n   = category_id;
    new_trx_d.m_amount          = WebAppTrans.Amount;
    new_trx_d.m_to_amount       = WebAppTrans.Amount;
    new_trx_d.m_number          = "";
    new_trx_d.m_notes           = WebAppTrans.Notes;
    new_trx_d.m_followup_id     = -1;
    new_trx_d.m_color           = -1;
    TrxModel::instance().add_data_n(new_trx_d);
    DeskNewTrID = new_trx_d.id();

    if (DeskNewTrID > 0) {
        if (!WebAppTrans.Attachments.IsEmpty()) {
            const wxString AttachmentsFolder = mmex::getPathAttachment(
                mmAttachmentManage::InfotablePathSetting()
            );
            if (AttachmentsFolder == wxEmptyString || !wxDirExists(AttachmentsFolder)) {
                TrxModel::instance().purge_id(DeskNewTrID);
                DeskNewTrID = -1;

                wxString msgStr = wxString()
                    << _t("Unable to download attachments from the WebApp.") << "\n"
                    << _t("Attachments folder not set or unavailable.") << "\n" << "\n"
                    << _t("Transaction not downloaded:") << "\n"
                    << _t("Please fix the attachments folder or delete the attachments from the WebApp.") << "\n";
                wxMessageBox(msgStr, _t("Attachment folder error"), wxICON_ERROR);
            }
            else {
                int AttachmentNr = 0;
                wxString WebAppAttachmentName, DesktopAttachmentName;
                wxArrayString AttachmentsArray;
                wxStringTokenizer tkz1(WebAppTrans.Attachments, (';'), wxTOKEN_RET_EMPTY_ALL);
                while (tkz1.HasMoreTokens())
                    {AttachmentsArray.Add(tkz1.GetNextToken());}
                for (size_t i = 0; i < AttachmentsArray.GetCount(); i++) {
                    AttachmentNr++;
                    WebAppAttachmentName = AttachmentsArray.Item(i);
                    wxString CurlError = "";
                    DesktopAttachmentName = WebApp_DownloadOneAttachment(WebAppAttachmentName, DeskNewTrID, AttachmentNr, CurlError);
                    if (DesktopAttachmentName != wxEmptyString) {
                        AttachmentData new_att_d = AttachmentData();
                        new_att_d.m_ref_type_n  = RefTypeN(RefTypeN::e_trx);
                        new_att_d.m_ref_id      = DeskNewTrID;
                        new_att_d.m_description = _t("Attachment") + "_" << AttachmentNr;
                        new_att_d.m_filename    = DesktopAttachmentName;
                        AttachmentModel::instance().add_data_n(new_att_d);
                    }
                    else {
                        TrxModel::instance().purge_id(DeskNewTrID);
                        DeskNewTrID = -1;

                        wxString msgStr = wxString() << _t("Unable to download attachments from the WebApp.") << "\n"
                            << CurlError << "\n" << "\n"
                            << _t("Transaction not downloaded: please retry to download transactions") << "\n";
                        wxMessageBox(msgStr, _t("Attachment download error"), wxICON_ERROR);
                        break;
                    }

                    WebAppAttachmentName = wxEmptyString;

                } //End loop thought attachments
                bDeleteTrWebApp = true;
            }
        }
        //Transaction without attachments
        else {
            bDeleteTrWebApp = true;
        }
    }

    if (bDeleteTrWebApp)
        WebApp_DeleteOneTransaction(WebAppTrans.ID);
    return DeskNewTrID;
}

//Delete one transaction from WebApp
bool mmWebApp::WebApp_DeleteOneTransaction(int64 WebAppTransactionId)
{
    wxString DeleteOneTransactionUrl = mmWebApp::getServicesPageURL() + "&" + WebAppParam::DeleteOneTransaction + "=" << WebAppTransactionId.GetValue();

    wxString outputMessage;
    int ErrorCode = http_get_data(DeleteOneTransactionUrl, outputMessage);

    return mmWebApp::returnResult(ErrorCode, outputMessage);
}

//Download one attachment from WebApp
wxString mmWebApp::WebApp_DownloadOneAttachment(const wxString& AttachmentName, int64 DesktopTransactionID, int AttachmentNr, wxString& Error)
{
    wxString FileExtension = wxFileName(AttachmentName).GetExt().MakeLower();
    wxString FileName = TrxModel::s_ref_type.name_n() + "_" + wxString::Format("%lld", DesktopTransactionID)
        + "_Attach" + wxString::Format("%i", AttachmentNr) + "." + FileExtension;
    const wxString FilePath = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting())
        + TrxModel::s_ref_type.name_n() + wxFileName::GetPathSeparator() + FileName;
    wxString URL = mmWebApp::getServicesPageURL() + "&" + WebAppParam::DownloadAttachments + "=" + AttachmentName;
    CURLcode CurlStatus = http_download_file(URL, FilePath);
    if (CurlStatus == CURLE_OK)
        return FileName;
    else
    {
        Error = curl_easy_strerror(CurlStatus);
        return wxEmptyString;
    }
}

//Get one attachment from WebApp
bool mmWebApp::WebApp_DownloadAttachment(wxString& AttachmentFileName, wxString& Error)
{
    const wxString FileExtension = wxFileName(AttachmentFileName).GetExt().MakeLower();
    wxString orig_file_name = AttachmentFileName;
    AttachmentFileName = wxFileName::CreateTempFileName(AttachmentFileName);

    wxString URL = mmWebApp::getServicesPageURL() + "&" + WebAppParam::DownloadAttachments + "=" + orig_file_name;
    CURLcode CurlStatus = http_download_file(URL, AttachmentFileName);
    if (CurlStatus == CURLE_OK) {
        wxString temp_file = AttachmentFileName + "." + FileExtension;
        if (wxRenameFile(AttachmentFileName, temp_file)) {
            AttachmentFileName = temp_file;
            return true;
        }
    }
    else
    {
        Error = curl_easy_strerror(CurlStatus);
    }
    return false;
}


//Update account in MMEX
bool mmWebApp::MMEX_WebApp_UpdateAccount()
{
    if (mmWebApp::WebApp_CheckEnabled())
    {
        if (mmWebApp::WebApp_CheckGuid() && mmWebApp::WebApp_CheckApiVersion())
            return mmWebApp::WebApp_UpdateAccount();
        else
            return false;
    }
    else
        return false;
}

//Update payee in MMEX
bool mmWebApp::MMEX_WebApp_UpdatePayee()
{
    if (mmWebApp::WebApp_CheckEnabled())
    {
        if (mmWebApp::WebApp_CheckGuid() && mmWebApp::WebApp_CheckApiVersion())
            return mmWebApp::WebApp_UpdatePayee();
        else
            return false;
    }
    else
        return false;
}

//Update category in MMEX
bool mmWebApp::MMEX_WebApp_UpdateCategory()
{
    if (mmWebApp::WebApp_CheckEnabled())
    {
        if (mmWebApp::WebApp_CheckGuid() && mmWebApp::WebApp_CheckApiVersion())
            return mmWebApp::WebApp_UpdateCategory();
        else
            return false;
    }
    else
        return false;
}
