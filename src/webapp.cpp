/*******************************************************
Copyright (C) 2014 Gabriele-V

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


#include "attachmentdialog.h"
#include "webapp.h"
#include "model/Model_Setting.h"
#include "model/Model_Account.h"
#include "model/Model_Attachment.h"
#include "model/Model_Category.h"
#include "model/Model_Payee.h"
#include "model/Model_Subcategory.h"
#include "paths.h"
#include "transdialog.h"
#include "util.h"
#include <wx/sstream.h>
#include <wx/protocol/http.h>

//Expected WebAppVersion
const wxString WebAppParam::ApiExpectedVersion = "1.0.1";

//Internal constants
const wxString mmWebApp::getUrl()
{
    wxString Url = Model_Infotable::instance().GetStringInfo("WEBAPPURL", "");
    Url.Replace("https://", "http://");
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
    site_content(mmWebApp::getServicesPageURL() + "&" + WebAppParam::CheckApiVersion, outputMessage);

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
    site_content(mmWebApp::getServicesPageURL() + "&" + WebAppParam::CheckGuid, outputMessage);

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
    wxHTTP http;
    int ErrorCode = 0;

    //Build string connection
    Website.Replace("http://", "");
    wxString BaseServerAddress = Website.SubString(0, Website.Find("/")-1);
    wxString PagePath = Website.SubString(Website.Find("/"), Website.Length());

    //Create multipart form
    wxString Boundary = "Custom_Boundary_MMEX_WebApp";
    wxString Text = wxEmptyString;
    Text.Append(wxString::Format("--%s\r\n", Boundary));
    Text.Append(wxString::Format("Content-Disposition: form-data; name=\"%s\"\r\n\r\n", "MMEX_Post"));
    Text.Append(wxString::Format("%s\r\n", JsonData));
    Text.Append(wxString::Format("\r\n--%s--\r\n", Boundary));

    http.SetPostText("multipart/form-data; boundary=" + Boundary, Text);

    if (http.Connect(BaseServerAddress))
    {
        ErrorCode = http.GetError();
        if (ErrorCode == wxPROTO_NOERR)
        {
            wxInputStream *httpStream = http.GetInputStream(PagePath);
            if (httpStream)
            {
                wxStringOutputStream out_stream(&Output);
                httpStream->Read(out_stream);
            }
            else
                ErrorCode = -1;
            wxDELETE(httpStream);
        }
        http.Close();
    }
    else
        ErrorCode = http.GetError();

    if (ErrorCode != wxPROTO_NOERR)
    {
        if (ErrorCode == wxPROTO_NETERR) Output = _("A generic network error occurred");
        else if (ErrorCode == wxPROTO_PROTERR) Output = _("An error occurred during negotiation");
        else if (ErrorCode == wxPROTO_CONNERR) Output = _("The client failed to connect the server");
        else if (ErrorCode == wxPROTO_INVVAL) Output = _("Invalid value");
        else if (ErrorCode == wxPROTO_NOFILE) Output = _("The remote file doesn't exist");
        else if (ErrorCode == wxPROTO_RCNCT) Output = _("An error occurred during reconnection");
        else if (ErrorCode == wxPROTO_STREAMING) Output = _("Someone tried to send a command during a transfer");
        else if (ErrorCode == -1) Output = _("Cannot get data from website");
        else Output = _("Unknown error");
    }
    return ErrorCode;
}


//Delete all account on WebApp
bool mmWebApp::WebApp_DeleteAllAccount()
{
    wxString outputMessage;
    int ErrorCode = site_content(mmWebApp::getServicesPageURL() + "&" + WebAppParam::DeleteAccount, outputMessage);

    return mmWebApp::returnResult(ErrorCode, outputMessage);
}

// Update Account on WebApp
bool mmWebApp::WebApp_UpdateAccount()
{
    int i = 0;
    json::Object jsonAccountList;
    std::wstringstream jsonAccountStream;
    wxString outputMessage;

    wxString UpdateAccountUrl = mmWebApp::getServicesPageURL() + "&" + WebAppParam::ImportAccount + "=true";

    mmWebApp::WebApp_DeleteAllAccount();

    for (const auto &Account : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
    {
        if (Model_Account::type(Account) != Model_Account::INVESTMENT)
            jsonAccountList[L"Accounts"][i][L"AccountName"] = json::String(Account.ACCOUNTNAME.ToStdWstring());
        i++;
    }

    json::Writer::Write(jsonAccountList, jsonAccountStream);
    wxString AccountList = jsonAccountStream.str();

    int ErrorCode = mmWebApp::WebApp_SendJson(UpdateAccountUrl, AccountList, outputMessage);

    return mmWebApp::returnResult(ErrorCode, outputMessage);
}

//Delete all payee on WebApp
bool mmWebApp::WebApp_DeleteAllPayee()
{
    wxString outputMessage;
    int ErrorCode = site_content(mmWebApp::getServicesPageURL() + "&" + WebAppParam::DeletePayee, outputMessage);

    return mmWebApp::returnResult(ErrorCode, outputMessage);
}

//Update payee on WebApp
bool mmWebApp::WebApp_UpdatePayee()
{
    int i = 0;
    json::Object jsonPayeeList;
    std::wstringstream jsonPayeeStream;
    wxString outputMessage, DefCategoryName, DefSubCategoryName;

    wxString UpdatePayeeUrl = mmWebApp::getServicesPageURL() + "&" + WebAppParam::ImportPayee + "=true";

    mmWebApp::WebApp_DeleteAllPayee();

    for (const auto &Payee : Model_Payee::instance().all(Model_Payee::COL_PAYEENAME))
    {
        const Model_Category::Data* DefCategory = Model_Category::instance().get(Payee.CATEGID);
        if (DefCategory != nullptr)
            DefCategoryName = DefCategory->CATEGNAME;
        else
            DefCategoryName = "None";

        const Model_Subcategory::Data* DefSubCategory = Model_Subcategory::instance().get(Payee.SUBCATEGID);
        if (DefSubCategory != nullptr)
            DefSubCategoryName = DefSubCategory->SUBCATEGNAME;
        else
            DefSubCategoryName = "None";
        
        jsonPayeeList[L"Payees"][i][L"PayeeName"] = json::String(Payee.PAYEENAME.ToStdWstring());
        jsonPayeeList[L"Payees"][i][L"DefCateg"] = json::String(DefCategoryName.ToStdWstring());
        jsonPayeeList[L"Payees"][i][L"DefSubCateg"] = json::String(DefSubCategoryName.ToStdWstring());

        i++;
    }

    json::Writer::Write(jsonPayeeList, jsonPayeeStream);
    wxString PayeesList = jsonPayeeStream.str();

    int ErrorCode = mmWebApp::WebApp_SendJson(UpdatePayeeUrl, PayeesList, outputMessage);

    return mmWebApp::returnResult(ErrorCode, outputMessage);
}

//Delete all category on WebApp
bool mmWebApp::WebApp_DeleteAllCategory()
{
    wxString outputMessage;
    int ErrorCode = site_content(mmWebApp::getServicesPageURL() + "&" + WebAppParam::DeleteCategory, outputMessage);

    return mmWebApp::returnResult(ErrorCode, outputMessage);
}

//Update category on WebApp
bool mmWebApp::WebApp_UpdateCategory()
{
    int i = 0;
    json::Object jsonCategoryList;
    std::wstringstream jsonCategoryStream;
    wxString outputMessage, SubCategoryName;

    wxString UpdateCategoryUrl = mmWebApp::getServicesPageURL() + "&" + WebAppParam::ImportCategory + "=true";

    mmWebApp::WebApp_DeleteAllCategory();

    const auto &categories = Model_Category::instance().all();
    for (const Model_Category::Data& category : categories)
    {
        bool FirstCategoryRun = true;
        bool SubCategoryFound = false;
        jsonCategoryList[L"Categories"][i][L"CategoryName"] = json::String(category.CATEGNAME.ToStdWstring());
        for (const auto &sub_category : Model_Category::sub_category(category))
        {
            SubCategoryFound = true;
            if (FirstCategoryRun == true)
            {
                jsonCategoryList[L"Categories"][i][L"SubCategoryName"] = json::String(sub_category.SUBCATEGNAME.ToStdWstring());
                i++;
                FirstCategoryRun = false;
            }
            else
            {
                jsonCategoryList[L"Categories"][i][L"CategoryName"] = json::String(category.CATEGNAME.ToStdWstring());
                jsonCategoryList[L"Categories"][i][L"SubCategoryName"] = json::String(sub_category.SUBCATEGNAME.ToStdWstring());
                i++;
                FirstCategoryRun = false;
            }
        }

        if (SubCategoryFound == false)
            jsonCategoryList[L"Categories"][i][L"SubCategoryName"] = json::String(L"None");
        else
            i--;

        i++;
    }

    json::Writer::Write(jsonCategoryList, jsonCategoryStream);
    wxString CategoryList = jsonCategoryStream.str();

    int ErrorCode = mmWebApp::WebApp_SendJson(UpdateCategoryUrl, CategoryList, outputMessage);

    return mmWebApp::returnResult(ErrorCode, outputMessage);
}

//Download new transactions
bool mmWebApp::WebApp_DownloadNewTransaction(WebTranVector& WebAppTransactions_, const bool CheckOnly)
{
    wxString NewTransactionJSON;
    int ErrorCode = site_content(mmWebApp::getServicesPageURL() + "&" + WebAppParam::DownloadNewTransaction, NewTransactionJSON);

    if (NewTransactionJSON == "null" || NewTransactionJSON.IsEmpty() || ErrorCode != 0)
        return false;
    else if (CheckOnly)
        return true;
    else
    {
        json::Object jsonTransaction;
        std::wstringstream jsonTransactionStream;

        if (!(NewTransactionJSON.StartsWith("{") && NewTransactionJSON.EndsWith("}"))) return true;
        jsonTransactionStream << NewTransactionJSON.ToStdWstring();
        json::Reader::Read(jsonTransaction, jsonTransactionStream);

        //Define variables
        webtran_holder WebTran;
        WebAppTransactions_.clear();
        std::wstring TrProgrStr;
        wxDateTime dt;
        wxString dtStr,Payee,Category,SubCategory;

        for (int i = 0; i < static_cast<int>(jsonTransaction.Size()); i++)
        {
            TrProgrStr = std::to_wstring(i);
            
            WebTran.ID = wxAtoi(wxString(json::String(jsonTransaction[TrProgrStr][L"ID"])));
            
            dtStr = wxString(json::String(jsonTransaction[TrProgrStr][L"Date"]));
            mmParseDisplayStringToDate(dt, dtStr, "%Y-%m-%d");
            WebTran.Date = dt;
      
            WebTran.Account = wxString(json::String(jsonTransaction[TrProgrStr][L"Account"]));
            WebTran.ToAccount = wxString(json::String(jsonTransaction[TrProgrStr][L"ToAccount"]));
            WebTran.Status = wxString(json::String(jsonTransaction[TrProgrStr][L"Status"]));
            WebTran.Type = wxString(json::String(jsonTransaction[TrProgrStr][L"Type"]));

            Payee = wxString(json::String(jsonTransaction[TrProgrStr][L"Payee"]));
            if (Payee == "None" || Payee.IsEmpty()) Payee = _("Unknown");
            WebTran.Payee = Payee;

            Category = wxString(json::String(jsonTransaction[TrProgrStr][L"Category"]));
            if (Category == "None" || Category.IsEmpty()) Category = _("Unknown");
            WebTran.Category = Category;

            SubCategory = wxString(json::String(jsonTransaction[TrProgrStr][L"SubCategory"]));
            if (SubCategory == "None" || SubCategory.IsEmpty()) SubCategory = wxEmptyString;
            WebTran.SubCategory = SubCategory;

            //Amount -> TODO: Test with json::Number
            wxString jsonAmount = wxString(json::String(jsonTransaction[TrProgrStr][L"Amount"]));
            double TransactionAmount; jsonAmount.ToDouble(&TransactionAmount);
            WebTran.Amount = TransactionAmount;

            WebTran.Notes = wxString(json::String(jsonTransaction[TrProgrStr][L"Notes"]));
            WebTran.Attachments = wxString(json::String(jsonTransaction[TrProgrStr][L"Attachments"]));

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
    if (Account != nullptr)
    {
        AccountID = Account->ACCOUNTID;
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
                FistAccountName = FirstAccount.ACCOUNTNAME;
                AccountID = FirstAccount.ACCOUNTID;
                break;
            }
        }

        wxString msgStr = wxString::Format(_("Account '%s' not found!"), WebAppTrans.Account)
            << "\n\n"
            << wxString::Format(_("Transaction will be inserted with the first bank account:\n'%s' and marked as  'Follow Up'")
            , FistAccountName) << "\n";
        wxMessageBox(msgStr, _("Wrong WebApp account"), wxICON_ERROR);
    }

    //Search ToAccount
    if (WebAppTrans.ToAccount != "None")
    {
        const Model_Account::Data* ToAccount = Model_Account::instance().get(WebAppTrans.ToAccount);
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
        NewPayee->CATEGID = CategoryID;
        NewPayee->SUBCATEGID = SubCategoryID;
        int NewPayeeID = Model_Payee::instance().save(NewPayee);
        PayeeID = NewPayeeID;
    }  

    //Create New Transaction
    Model_Checking::Data * desktopNewTransaction;
    desktopNewTransaction = Model_Checking::instance().create();
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
            wxString AttachmentsFolder = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting());
            if (AttachmentsFolder == wxEmptyString || !wxDirExists(AttachmentsFolder))
            {
                DeskNewTrID = Model_Checking::instance().remove(DeskNewTrID);
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
                    DesktopAttachmentName = WebApp_DownloadOneAttachment(WebAppAttachmentName, DeskNewTrID, AttachmentNr);
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
                        DeskNewTrID = Model_Checking::instance().remove(DeskNewTrID);
                        DeskNewTrID = -1;

                        wxString msgStr = wxString() << _("Unable to download attachments from webapp.") << "\n"
                            << _("Communication error") << "\n" << "\n"
                            << _("Transaction not downloaded: please close and open MMEX to try re-download transaction") << "\n";
                        wxMessageBox(msgStr, _("Attachment download error"), wxICON_ERROR);
                        break;
                    }

                    WebAppAttachmentName = wxEmptyString;

                } //End loop throught attachments
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
    int ErrorCode = site_content(DeleteOneTransactionUrl, outputMessage);

    return mmWebApp::returnResult(ErrorCode, outputMessage);
}

//Download one attachment from WebApp
wxString mmWebApp::WebApp_DownloadOneAttachment(const wxString& AttachmentName, int DesktopTransactionID, int AttachmentNr)
{
    wxString FileExtension = wxFileName(AttachmentName).GetExt().MakeLower();
    wxString FileName = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION) + "_" + wxString::Format("%i", DesktopTransactionID)
        + "_Attach" + wxString::Format("%i", AttachmentNr) + "." + FileExtension;
    wxString FilePath = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting()) + wxFileName::GetPathSeparator()
        + Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION) + wxFileName::GetPathSeparator() + FileName;
    wxString URL = mmWebApp::getServicesPageURL() + "&" + WebAppParam::DownloadAttachments + "=" + AttachmentName;
    if (download_file(URL, FilePath))
        return FileName;
    else
        return wxEmptyString;
}

//Get one attachment from WebApp
wxString mmWebApp::WebApp_GetAttachment(const wxString& AttachmentFileName)
{
    wxString FileExtension = wxFileName(AttachmentFileName).GetExt().MakeLower();
    wxString FilePath = mmex::getTempFolder() + "WebAppAttach_" + wxDateTime::Now().Format("%Y%m%d%H%M%S") + "." + FileExtension;
    wxString URL = mmWebApp::getServicesPageURL() + "&" + WebAppParam::DownloadAttachments + "=" + AttachmentFileName;
    if (download_file(URL, FilePath))
        return FilePath;
    else
    return wxEmptyString;
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
