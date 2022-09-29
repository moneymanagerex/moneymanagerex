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


#include "webapp.h"
#include "attachmentdialog.h"
#include "paths.h"
#include "transdialog.h"
#include "util.h"
#include "model/Model_Account.h"
#include "model/Model_Attachment.h"
#include "model/Model_Category.h"
#include "model/Model_Subcategory.h"
#include "model/Model_Infotable.h"

//Expected WebAppVersion
const wxString WebAppParam::ApiExpectedVersion = "1.0.1";

//Internal constants
const wxString mmWebApp::getUrl()
{
    wxString Url = Model_Infotable::instance().GetStringInfo("WEBAPPURL", "");
    return Url;
}

const wxString mmWebApp::getGuid()
{
    return Model_Infotable::instance().GetStringInfo("WEBAPPGUID", "");
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
    if (Model_Infotable::instance().GetStringInfo("WEBAPPURL", "") != wxEmptyString
        && Model_Infotable::instance().GetStringInfo("WEBAPPGUID", "") != wxEmptyString)
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
        wxString msgStr = wxString() << _("Wrong WebApp GUID:") << "\n"
            << _("please check it in network options.") << "\n";
        wxMessageBox(msgStr, _("Wrong WebApp settings"), wxICON_ERROR);
        return false;
    }
    else
    {
        wxString msgStr = wxString() << _("Unable to connect to WebApp:") << "\n"
            << _("please check settings and / or internet connection.") << "\n\n"
            << wxString::Format(_("Error: %s"), "\n" + outputMessage + "\n");
        wxMessageBox(msgStr, _("WebApp connection error"), wxICON_ERROR);
        return false;
    }
}

//Check WebApp Api version
bool mmWebApp::WebApp_CheckApiVersion()
{
    if (mmWebApp::WebApp_getApiVersion() != WebAppParam::ApiExpectedVersion)
    {
        wxString msgStr = _("Wrong WebApp API version:") + "\n"
            + wxString::Format(_("WebApp   API version -> %s"), mmWebApp::WebApp_getApiVersion()) + "\n"
            + wxString::Format(_("Expected API version -> %s"), WebAppParam::ApiExpectedVersion) + "\n";
        wxMessageBox(msgStr, _("Wrong WebApp API version"), wxICON_ERROR);
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

    const auto ContentType = "Content-Type: multipart/form-data; charset=utf-8; boundary=Custom_Boundary_MMEX_WebApp";
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

    for (const auto &account : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        if (Model_Account::type(account) != Model_Account::INVESTMENT && Model_Account::status(account) != Model_Account::CLOSED)
        {
            json_writer.StartObject();
            json_writer.Key("AccountName");
            json_writer.String(account.ACCOUNTNAME.utf8_str());
            json_writer.EndObject();
        }
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
    for (const auto &payee : Model_Payee::instance().all(Model_Payee::COL_PAYEENAME))
    {
        const Model_Category::Data* def_category = Model_Category::instance().get(payee.CATEGID);
        if (def_category != nullptr)
            def_category_name = def_category->CATEGNAME;
        else
            def_category_name = "None";

        const Model_Subcategory::Data* def_subcategory = Model_Subcategory::instance().get(payee.SUBCATEGID);
        if (def_subcategory != nullptr)
            def_subcategory_name = def_subcategory->SUBCATEGNAME;
        else
            def_subcategory_name = "None";

        json_writer.StartObject();
        json_writer.Key("PayeeName");
        json_writer.String(payee.PAYEENAME.utf8_str());
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
    const auto &categories = Model_Category::instance().all();
    for (const Model_Category::Data& category : categories)
    {
        bool first_category_run = true;
        bool sub_category_found = false;

        json_writer.StartObject();
        json_writer.Key("CategoryName");
        json_writer.String(category.CATEGNAME.utf8_str());

        for (const auto &sub_category : Model_Category::sub_category(category))
        {
            sub_category_found = true;
            if (first_category_run == true)
            {
                json_writer.Key("SubCategoryName");
                json_writer.String(sub_category.SUBCATEGNAME.utf8_str());
                json_writer.EndObject();

                first_category_run = false;
            }
            else
            {
                json_writer.StartObject();
                json_writer.Key("CategoryName");
                json_writer.String(category.CATEGNAME.utf8_str());

                json_writer.Key("SubCategoryName");
                json_writer.String(sub_category.SUBCATEGNAME.utf8_str());
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
                WebTran.ID = wxAtoi(trx["ID"].GetString());
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
                    Payee = _("Unknown");
                }
                WebTran.Payee = Payee;
            }

            if (trx.HasMember("Category") && trx["Category"].IsString()) {
                wxString Category = wxString::FromUTF8(trx["Category"].GetString());
                Category.Replace(":", "|");
                if (Category == "None" || Category.IsEmpty()) {
                    Category = _("Unknown");
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
int mmWebApp::MMEX_InsertNewTransaction(webtran_holder& WebAppTrans)
{
    int DeskNewTrID = 0;
    bool bDeleteTrWebApp = false;

    int AccountID = -1;
    int ToAccountID = -1;
    int PayeeID = -1;
    int CategoryID = -1;
    int SubCategoryID = -1;
    wxString TrStatus;

    //Search Account
    const Model_Account::Data* Account = Model_Account::instance().get(WebAppTrans.Account);
    wxString accountName, accountInitialDate;
    if (Account != nullptr)
    {
        AccountID = Account->ACCOUNTID;
        accountName = Account->ACCOUNTNAME;
        accountInitialDate = Account->INITIALDATE;
        TrStatus = WebAppTrans.Status;
    }
    else
    {
        TrStatus = "F";
        wxString FistAccountName;

        //Search first bank account
        for (const auto &FirstAccount : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
        {
            if (Model_Account::type(FirstAccount) != Model_Account::INVESTMENT && Model_Account::type(FirstAccount) != Model_Account::TERM)
            {
                accountName = FirstAccount.ACCOUNTNAME;
                AccountID = FirstAccount.ACCOUNTID;
                accountInitialDate = FirstAccount.INITIALDATE;
                break;
            }
        }

        wxString msgStr = wxString::Format(_("Account '%s' not found!"), WebAppTrans.Account)
            << "\n\n"
            << wxString::Format(_("Transaction will be inserted with the first bank account:\n"
            "'%s' and marked as  'Follow Up'")
            , accountName) << "\n";
        wxMessageBox(msgStr, _("Wrong WebApp account"), wxICON_ERROR);
    }

    //Search ToAccount
    Model_Account::Data* ToAccount = NULL;
    if (WebAppTrans.ToAccount != "None")
    {
        ToAccount = Model_Account::instance().get(WebAppTrans.ToAccount);
        if (ToAccount != nullptr)
            ToAccountID = ToAccount->ACCOUNTID;
    }

    //Search or insert Category
    const Model_Category::Data* Category = Model_Category::instance().get(WebAppTrans.Category);
    if (Category != nullptr)
        CategoryID = Category->CATEGID;
    else
    {
        Model_Category::Data* NewCategory = Model_Category::instance().create();
        NewCategory->CATEGNAME = WebAppTrans.Category;
        NewCategory->ACTIVE = 1;
        int NewCategoryID = Model_Category::instance().save(NewCategory);
        CategoryID = NewCategoryID;
    }

    //Search or insert SubCategory
    if (!WebAppTrans.SubCategory.IsEmpty())
    {
        const Model_Subcategory::Data* SubCategory = Model_Subcategory::instance().get(WebAppTrans.SubCategory, CategoryID);
        if (SubCategory != nullptr)
            SubCategoryID = SubCategory->SUBCATEGID;
        else if (CategoryID != -1)
        {
            Model_Subcategory::Data* NewSubCategory = Model_Subcategory::instance().create();
            NewSubCategory->CATEGID = CategoryID;
            NewSubCategory->SUBCATEGNAME = WebAppTrans.SubCategory;
            NewSubCategory->ACTIVE = 1;
            int NewSubCategoryID = Model_Subcategory::instance().save(NewSubCategory);
            SubCategoryID = NewSubCategoryID;
        }
    }

    //Search or insert Payee
    const Model_Payee::Data* Payee = Model_Payee::instance().get(WebAppTrans.Payee);
    if (Payee != nullptr)
        PayeeID = Payee->PAYEEID;
    else
    {
        Model_Payee::Data* NewPayee = Model_Payee::instance().create();
        NewPayee->PAYEENAME = WebAppTrans.Payee;
        NewPayee->ACTIVE = 1;
        NewPayee->CATEGID = CategoryID;
        NewPayee->SUBCATEGID = SubCategoryID;
        int NewPayeeID = Model_Payee::instance().save(NewPayee);
        PayeeID = NewPayeeID;
    }

    //Create New Transaction
    Model_Checking::Data * desktopNewTransaction;
    desktopNewTransaction = Model_Checking::instance().create();
    wxString trxDate = WebAppTrans.Date.FormatISODate();
    if ((trxDate < accountInitialDate) ||
            (ToAccount && trxDate < ToAccount->INITIALDATE))
    {
        wxString msgStr = wxString::Format("%s: %s / %s: %s\n\n%s\n%s"
                            , _("Account"), accountName
                            , _("Date"), trxDate
                            , _("The opening date for the account is later than the date of this transaction")
                            , _("Today will be used as the transaction date"));
        wxMessageBox(msgStr, _("Invalid Date"), wxICON_ERROR);
        trxDate = wxDate::Today().FormatISODate();
    }
    desktopNewTransaction->TRANSDATE = WebAppTrans.Date.FormatISODate();
    desktopNewTransaction->STATUS = TrStatus;
    desktopNewTransaction->TRANSCODE = WebAppTrans.Type;
    desktopNewTransaction->TRANSAMOUNT = WebAppTrans.Amount;
    desktopNewTransaction->ACCOUNTID = AccountID;
    desktopNewTransaction->TOACCOUNTID = ToAccountID;
    desktopNewTransaction->PAYEEID = PayeeID;
    desktopNewTransaction->CATEGID = CategoryID;
    desktopNewTransaction->SUBCATEGID = SubCategoryID;
    desktopNewTransaction->TRANSACTIONNUMBER = "";
    desktopNewTransaction->NOTES = WebAppTrans.Notes;
    desktopNewTransaction->FOLLOWUPID = -1;
    desktopNewTransaction->TOTRANSAMOUNT = WebAppTrans.Amount;

    DeskNewTrID = Model_Checking::instance().save(desktopNewTransaction);

    if (DeskNewTrID > 0)
    {
        if (!WebAppTrans.Attachments.IsEmpty())
        {
            const wxString AttachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting());
            if (AttachmentsFolder == wxEmptyString || !wxDirExists(AttachmentsFolder))
            {
                Model_Checking::instance().remove(DeskNewTrID);
                DeskNewTrID = -1;

                wxString msgStr = wxString() << _("Unable to download attachments from webapp.") << "\n"
                    << _("Attachments folder not set or unavailable") << "\n" << "\n"
                    << _("Transaction not downloaded:") << "\n"
                    << _("please fix attachments folder or delete attachments from WebApp") << "\n";
                wxMessageBox(msgStr, _("Attachment folder error"), wxICON_ERROR);
            }
            else
            {
                int AttachmentNr = 0;
                wxString WebAppAttachmentName, DesktopAttachmentName;
                wxArrayString AttachmentsArray;
                wxStringTokenizer tkz1(WebAppTrans.Attachments, (';'), wxTOKEN_RET_EMPTY_ALL);
                while (tkz1.HasMoreTokens())
                    {AttachmentsArray.Add(tkz1.GetNextToken());}
                for (size_t i = 0; i < AttachmentsArray.GetCount(); i++)
                {
                    AttachmentNr++;
                    WebAppAttachmentName = AttachmentsArray.Item(i);
                    wxString CurlError = "";
                    DesktopAttachmentName = WebApp_DownloadOneAttachment(WebAppAttachmentName, DeskNewTrID, AttachmentNr, CurlError);
                    if (DesktopAttachmentName != wxEmptyString)
                    {
                        Model_Attachment::Data* NewAttachment = Model_Attachment::instance().create();
                            NewAttachment->REFTYPE = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
                            NewAttachment->REFID = DeskNewTrID;
                            NewAttachment->DESCRIPTION = _("Attachment") + "_" << AttachmentNr;
                            NewAttachment->FILENAME = DesktopAttachmentName;
                        Model_Attachment::instance().save(NewAttachment);
                    }
                    else
                    {
                        Model_Checking::instance().remove(DeskNewTrID);
                        DeskNewTrID = -1;

                        wxString msgStr = wxString() << _("Unable to download attachments from webapp.") << "\n"
                            << CurlError << "\n" << "\n"
                            << _("Transaction not downloaded: please retry to download transactions") << "\n";
                        wxMessageBox(msgStr, _("Attachment download error"), wxICON_ERROR);
                        break;
                    }

                    WebAppAttachmentName = wxEmptyString;

                } //End loop thought attachments
                bDeleteTrWebApp = true;
            }
        }
        else //Transaction without attachments
        {
            bDeleteTrWebApp = true;
        }
    }

    if (bDeleteTrWebApp)
        WebApp_DeleteOneTransaction(WebAppTrans.ID);
    return DeskNewTrID;
}

//Delete one transaction from WebApp
bool mmWebApp::WebApp_DeleteOneTransaction(int WebAppTransactionId)
{
    wxString DeleteOneTransactionUrl = mmWebApp::getServicesPageURL() + "&" + WebAppParam::DeleteOneTransaction + "=" << WebAppTransactionId;

    wxString outputMessage;
    int ErrorCode = http_get_data(DeleteOneTransactionUrl, outputMessage);

    return mmWebApp::returnResult(ErrorCode, outputMessage);
}

//Download one attachment from WebApp
wxString mmWebApp::WebApp_DownloadOneAttachment(const wxString& AttachmentName, int DesktopTransactionID, int AttachmentNr, wxString& Error)
{
    wxString FileExtension = wxFileName(AttachmentName).GetExt().MakeLower();
    wxString FileName = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION) + "_" + wxString::Format("%i", DesktopTransactionID)
        + "_Attach" + wxString::Format("%i", AttachmentNr) + "." + FileExtension;
    const wxString FilePath = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting())
        + Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION) + wxFileName::GetPathSeparator() + FileName;
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
