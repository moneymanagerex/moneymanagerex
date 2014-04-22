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


#include "webapp.h"
#include "model/Model_Setting.h"
#include "model/Model_Account.h"
#include "model/Model_Category.h"
#include "model/Model_Payee.h"
#include "model/Model_Subcategory.h"
#include "paths.h"
#include "transdialog.h"
#include "util.h"
#include <wx/sstream.h>
#include <wx/protocol/http.h>

//Expected WebAppVersion
const wxString mmWebApp::getApiExpectedVersion()
{
	return "0.9.9";
}

//Internal constants
const wxString mmWebApp::getUrl()
{
    return Model_Infotable::instance().GetStringInfo("WEBAPPURL", "");
}

const wxString mmWebApp::getGuid()
{
    return Model_Infotable::instance().GetStringInfo("WEBAPPGUID", "");
}

const wxString mmWebApp::getServicesPage()
{
    return "services.php";
}

const wxString mmWebApp::getCheckGuidParameter()
{
	return "check_guid";
}

const wxString mmWebApp::getCheckApiVersionParameter()
{
	return "check_api_version";
}

const wxString mmWebApp::getDeleteAccountParameter()
{
	return "delete_bankaccount";
}

const wxString mmWebApp::getImportAccountParameter()
{
    return "import_bankaccount";
}

const wxString mmWebApp::getDeletePayeeParameter()
{
	return "delete_payee";
}

const wxString mmWebApp::getImportPayeeParameter()
{
    return "import_payee";
}

const wxString mmWebApp::getDeleteCategoryParameter()
{
	return "delete_category";
}

const wxString mmWebApp::getImportCategoryParameter()
{
	return "import_category";
}

const wxString mmWebApp::getDeleteOneTransactionParameter()
{
	return "delete_group";
}

const wxString mmWebApp::getDownloadNewTransactionParameter()
{
    return "download_transaction";
}

const wxString mmWebApp::getServicesPageURL()
{
	return mmWebApp::getUrl() + "/" + mmWebApp::getServicesPage() + "?" + "guid=" + mmWebApp::getGuid();
}

//Get WebApp Api version
const wxString mmWebApp::WebApp_getApiVersion()
{
	wxString outputMessage;
	site_content(mmWebApp::getServicesPageURL() + "&" + mmWebApp::getCheckApiVersionParameter(), outputMessage);

	return outputMessage;
}

const wxString mmWebApp::getMessageSucceeded()
{
    return "Operation has succeeded";
}

const wxString mmWebApp::getMessageWrongGuid()
{
    return "Wrong GUID";
}


/***************
** Functions  **
***************/
//Return function result
bool mmWebApp::returnResult(int& ErrorCode, wxString& outputMessage)
{
	if (ErrorCode == 0 && outputMessage == mmWebApp::getMessageSucceeded())
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
	site_content(mmWebApp::getServicesPageURL() + "&" + mmWebApp::getCheckGuidParameter(), outputMessage);

	if (outputMessage == mmWebApp::getMessageSucceeded())
		return true;
	else if (outputMessage == mmWebApp::getMessageWrongGuid())
	{
		wxString msgStr = wxString() << _("Wrong WebApp GUID:") << "\n"
			<< _("please check it in network options.") << "\n";
		wxMessageBox(msgStr, _("Wrong WebApp settings"), wxICON_ERROR);
		return false;
	}
	else
	{
		wxString msgStr = wxString() << _("Unable to connect to WebApp:") << "\n"
			<< _("please check settings and / or internet connection.") << "\n";
		wxMessageBox(msgStr, _("WebApp connection error"), wxICON_ERROR);
		return false;
	}
}

//Check WebApp Api version
bool mmWebApp::WebApp_CheckApiVersion()
{
	if (mmWebApp::WebApp_getApiVersion() != mmWebApp::getApiExpectedVersion())
	{
		wxString msgStr = wxString() << _("Wrong WebApp API version:") << "\n" <<
			_("WebApp   API version -> ") << mmWebApp::WebApp_getApiVersion() << "\n" <<
			_("Expected API version -> ") << mmWebApp::getApiExpectedVersion() << "\n";
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

	http.SetPostText("application/x-www-form-urlencoded","MMEX_Post=" + JsonData);

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
	int ErrorCode = site_content(mmWebApp::getServicesPageURL() + "&" + mmWebApp::getDeleteAccountParameter(), outputMessage);

	return mmWebApp::returnResult(ErrorCode, outputMessage);
}

// Update Account on WebApp
bool mmWebApp::WebApp_UpdateAccount()
{
	int i = 0;
	json::Object jsonAccountList;
	std::stringstream jsonAccountStream;
	wxString outputMessage;
	int ErrorCode = 0;

	wxString UpdateAccountUrl = mmWebApp::getServicesPageURL() + "&" + mmWebApp::getImportAccountParameter() + "=true";

	mmWebApp::WebApp_DeleteAllAccount();

	for (const auto &Account : Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME))
	{
		if (Model_Account::type(Account) != Model_Account::INVESTMENT)
			jsonAccountList["Accounts"][i]["AccountName"] = json::String(Account.ACCOUNTNAME.ToStdString());
		i++;
	}

	json::Writer::Write(jsonAccountList, jsonAccountStream);
	wxString AccountList = jsonAccountStream.str();

	ErrorCode = mmWebApp::WebApp_SendJson(UpdateAccountUrl, AccountList, outputMessage);

	return mmWebApp::returnResult(ErrorCode, outputMessage);
}

//Delete all payee on WebApp
bool mmWebApp::WebApp_DeleteAllPayee()
{
	wxString outputMessage;
	int ErrorCode = site_content(mmWebApp::getServicesPageURL() + "&" + mmWebApp::getDeletePayeeParameter(), outputMessage);

	return mmWebApp::returnResult(ErrorCode, outputMessage);
}

//Update payee on WebApp
bool mmWebApp::WebApp_UpdatePayee()
{
	int i = 0;
	json::Object jsonPayeeList;
	std::stringstream jsonPayeeStream;
	wxString outputMessage;
	wxString DefCategoryName;
	wxString DefSubCategoryName;
	int ErrorCode = 0;

	wxString UpdatePayeeUrl = mmWebApp::getServicesPageURL() + "&" + mmWebApp::getImportPayeeParameter() + "=true";

	mmWebApp::WebApp_DeleteAllPayee();

	for (const auto &Payee : Model_Payee::instance().all(Model_Payee::COL_PAYEENAME))
	{
		const Model_Category::Data* DefCategory = Model_Category::instance().get(Payee.CATEGID);
		if (DefCategory != NULL)
			DefCategoryName = DefCategory->CATEGNAME;
		else
			DefCategoryName = "None";

		const Model_Subcategory::Data* DefSubCategory = Model_Subcategory::instance().get(Payee.SUBCATEGID);
		if (DefSubCategory != NULL)
			DefSubCategoryName = DefSubCategory->SUBCATEGNAME;
		else
			DefSubCategoryName = "None";
		
		jsonPayeeList["Payees"][i]["PayeeName"] = json::String(Payee.PAYEENAME.ToStdString());
		jsonPayeeList["Payees"][i]["DefCateg"] = json::String(DefCategoryName.ToStdString());
		jsonPayeeList["Payees"][i]["DefSubCateg"] = json::String(DefSubCategoryName.ToStdString());

		i++;
	}

	json::Writer::Write(jsonPayeeList, jsonPayeeStream);
	wxString PayeesList = jsonPayeeStream.str();

	ErrorCode = mmWebApp::WebApp_SendJson(UpdatePayeeUrl, PayeesList, outputMessage);

	return mmWebApp::returnResult(ErrorCode, outputMessage);
}

//Delete all category on WebApp
bool mmWebApp::WebApp_DeleteAllCategory()
{
	wxString outputMessage;
	int ErrorCode = site_content(mmWebApp::getServicesPageURL() + "&" + mmWebApp::getDeleteCategoryParameter(), outputMessage);

	return mmWebApp::returnResult(ErrorCode, outputMessage);
}

//Update category on WebApp
bool mmWebApp::WebApp_UpdateCategory()
{
	int i = 0;
	json::Object jsonCategoryList;
	std::stringstream jsonCategoryStream;
	wxString outputMessage;
	wxString SubCategoryName;
	int ErrorCode = 0;

	wxString UpdateCategoryUrl = mmWebApp::getServicesPageURL() + "&" + mmWebApp::getImportCategoryParameter() + "=true";

	mmWebApp::WebApp_DeleteAllCategory();

	const auto &categories = Model_Category::instance().all();
	for (const Model_Category::Data& category : categories)
	{
		bool FirstCategoryRun = true;
		bool SubCategoryFound = false;
		jsonCategoryList["Categories"][i]["CategoryName"] = json::String(category.CATEGNAME.ToStdString());
		for (const auto &sub_category : Model_Category::sub_category(category))
		{
			SubCategoryFound = true;
			if (FirstCategoryRun == true)
			{
				jsonCategoryList["Categories"][i]["SubCategoryName"] = json::String(sub_category.SUBCATEGNAME.ToStdString());
				i++;
				FirstCategoryRun = false;
			}
			else
			{
				jsonCategoryList["Categories"][i]["CategoryName"] = json::String(category.CATEGNAME.ToStdString());
				jsonCategoryList["Categories"][i]["SubCategoryName"] = json::String(sub_category.SUBCATEGNAME.ToStdString());
				i++;
				FirstCategoryRun = false;
			}
		}

		if (SubCategoryFound == false)
			jsonCategoryList["Categories"][i]["SubCategoryName"] = json::String("None");
		else
			i--;

		i++;
	}

	json::Writer::Write(jsonCategoryList, jsonCategoryStream);
	wxString CategoryList = jsonCategoryStream.str();

	ErrorCode = mmWebApp::WebApp_SendJson(UpdateCategoryUrl, CategoryList, outputMessage);

	return mmWebApp::returnResult(ErrorCode, outputMessage);
}

//Check New Transaction
bool mmWebApp::WebApp_CheckNewTransaction()
{
	wxString NewTransaction;

	return mmWebApp::WebApp_DownloadNewTransaction(NewTransaction);
}

//Download new transactions
bool mmWebApp::WebApp_DownloadNewTransaction(wxString& NewTransactionJSON)
{
	int ErrorCode = site_content(mmWebApp::getServicesPageURL() + "&" + mmWebApp::getDownloadNewTransactionParameter(), NewTransactionJSON);

	if (NewTransactionJSON != "null" && ErrorCode == 0)
		return true;
	else
		return false;
}

//Insert new transaction
int mmWebApp::MMEX_InsertNewTransaction(wxString& NewTransactionJSON)
{
	int desktopNewTransactionId = 0;
	json::Object jsonTransaction;
	std::stringstream jsonTransactionStream;

	if (!(NewTransactionJSON.StartsWith("{") && NewTransactionJSON.EndsWith("}"))) NewTransactionJSON = "{}";
	jsonTransactionStream << NewTransactionJSON.ToStdString();
	json::Reader::Read(jsonTransaction, jsonTransactionStream);

	int AccountID = -1;
	int ToAccountID = -1;
	int PayeeID = -1;
	int CategoryID = -1;
	int SubCategoryID = -1;
	wxString TrStatus;

	//Search Account
	wxString AccountName = wxString(json::String(jsonTransaction["Account"]));
	const Model_Account::Data* Account = Model_Account::instance().get(AccountName);
	if (Account != NULL)
	{
		AccountID = Account->ACCOUNTID;
		TrStatus = wxString(json::String(jsonTransaction["Status"]));
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

		wxString msgStr = wxString() << _("Account '") << AccountName << _("' not found!") << "\n"
			<< "\n"
			<< _("Transaction will be inserted with the first bank account:") << "\n"
			<< "'" << FistAccountName <<_("' and marked as  Follow Up") << "\n";
		wxMessageBox(msgStr, _("Wrong WebApp account"), wxICON_ERROR);
	}

	//Search ToAccount
	wxString ToAccountName = wxString(json::String(jsonTransaction["ToAccount"]));
	if (ToAccountName != "None")
	{
		const Model_Account::Data* ToAccount = Model_Account::instance().get(ToAccountName);
		if (ToAccount != NULL)
			ToAccountID = ToAccount->ACCOUNTID;
	}

	//Search or insert Category
	wxString CategoryName = wxString(json::String(jsonTransaction["Category"]));
	if (CategoryName != "None")
	{
		const Model_Category::Data* Category = Model_Category::instance().get(CategoryName);
		if (Category != NULL)
			CategoryID = Category->CATEGID;
		else
		{
			Model_Category::Data* NewCategory = Model_Category::instance().create();
			NewCategory->CATEGNAME = CategoryName;
			int NewCategoryID = Model_Category::instance().save(NewCategory);
			CategoryID = NewCategoryID;
		}
	}

	//Search or insert SubCategory
	wxString SubCategoryName = wxString(json::String(jsonTransaction["SubCategory"]));
	if (SubCategoryName != "None")
	{
		const Model_Subcategory::Data* SubCategory = Model_Subcategory::instance().get(SubCategoryName,CategoryID);
		if (SubCategory != NULL)
			SubCategoryID = SubCategory->SUBCATEGID;
		else if (CategoryID != -1)
		{
			Model_Subcategory::Data* NewSubCategory = Model_Subcategory::instance().create();
			NewSubCategory->CATEGID = CategoryID;
			NewSubCategory->SUBCATEGNAME = SubCategoryName;
			int NewSubCategoryID = Model_Subcategory::instance().save(NewSubCategory);
			SubCategoryID = NewSubCategoryID;
		}
	}

	//Search or insert Payee
	wxString PayeeName = wxString(json::String(jsonTransaction["Payee"]));
	if (PayeeName != "None")
	{
		const Model_Payee::Data* Payee = Model_Payee::instance().get(PayeeName);
		if (Payee != NULL)
			PayeeID = Payee->PAYEEID;
		else
		{
			Model_Payee::Data* NewPayee = Model_Payee::instance().create();
			NewPayee->PAYEENAME = PayeeName;
			NewPayee->CATEGID = CategoryID;
			NewPayee->SUBCATEGID = SubCategoryID;
			int NewPayeeID = Model_Payee::instance().save(NewPayee);
			PayeeID = NewPayeeID;
		}
	}

	//Fix wrong number conversion from JSON
	wxString jsonAmount = wxString(json::String(jsonTransaction["Amount"]));
		double TransactionAmount;
		jsonAmount.ToDouble(&TransactionAmount);
	int WebAppTrID = wxAtoi(wxString(json::String(jsonTransaction["ID"])));
	

	//Create New Transaction
    Model_Checking::Data * desktopNewTransaction;
    desktopNewTransaction = Model_Checking::instance().create();
	desktopNewTransaction->TRANSDATE = wxString(json::String(jsonTransaction["Date"]));
	desktopNewTransaction->STATUS = TrStatus;
	desktopNewTransaction->TRANSCODE = wxString(json::String(jsonTransaction["Type"]));
	desktopNewTransaction->TRANSAMOUNT = TransactionAmount;
	desktopNewTransaction->ACCOUNTID = AccountID;
	desktopNewTransaction->TOACCOUNTID = ToAccountID;
	desktopNewTransaction->PAYEEID = PayeeID;
	desktopNewTransaction->CATEGID = CategoryID;
	desktopNewTransaction->SUBCATEGID = SubCategoryID;
	desktopNewTransaction->TRANSACTIONNUMBER = "";
	desktopNewTransaction->NOTES = wxString(json::String(jsonTransaction["Notes"]));
	desktopNewTransaction->FOLLOWUPID = -1;
	desktopNewTransaction->TOTRANSAMOUNT = TransactionAmount;

	desktopNewTransactionId = Model_Checking::instance().save(desktopNewTransaction);

	if (desktopNewTransactionId > 0)
		mmWebApp::WebApp_DeleteOneTransaction(WebAppTrID);
	return desktopNewTransactionId;
}

//Delete one transaction from WebApp
bool mmWebApp::WebApp_DeleteOneTransaction(int& WebAppNewTransactionId)
{
	wxString DeleteOneTransactionUrl = mmWebApp::getServicesPageURL() + "&" + mmWebApp::getDeleteOneTransactionParameter() + "=" << WebAppNewTransactionId;

	wxString outputMessage;
	int ErrorCode = site_content(DeleteOneTransactionUrl, outputMessage);

	return mmWebApp::returnResult(ErrorCode, outputMessage);
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