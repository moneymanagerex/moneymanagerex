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

#ifndef MM_EX_WEBAPP_H_
#define MM_EX_WEBAPP_H_

#include "defs.h"
#include <wx/spinctrl.h>

class mmWebApp
{
const static wxString getUrl();
const static wxString getGuid();
const static wxString getServicesPage();
const static wxString getApiExpectedVersion();

//Parameters used in services.php
const static wxString getCheckGuidParameter();
const static wxString getCheckApiVersionParameter();
const static wxString getDeleteAccountParameter();
const static wxString getImportAccountParameter();
const static wxString getDeletePayeeParameter();
const static wxString getImportPayeeParameter();
const static wxString getDeleteCategoryParameter();
const static wxString getImportCategoryParameter();
const static wxString getDeleteOneTransactionParameter();
const static wxString getDownloadNewTransactionParameter();

/** Return services page URL with GUID inserted */
const static wxString getServicesPageURL();

//Return messages from WebApp
const static wxString getMessageSucceeded();
const static wxString getMessageWrongGuid();


//Internal function
const static wxString WebApp_getApiVersion();
static int WebApp_SendJson(wxString& Website, const wxString& JsonData, wxString& Output);
static bool WebApp_DeleteAllAccount();
static bool WebApp_DeleteAllPayee();
static bool WebApp_DeleteAllCategory();
static bool WebApp_DeleteOneTransaction(int& WebAppNewTransactionId);


public:
	static bool returnResult(int& ErrorCode, wxString& outputMessage);

	/** Return true if WebApp is enabled */
	static bool WebApp_CheckEnabled();

	/** Return true if WebApp Guid is correct */
	static bool WebApp_CheckGuid();

	/** Return true if WebApp API Version is correct */
	static bool WebApp_CheckApiVersion();

	/** Update all accounts on WebApp */
	static bool WebApp_UpdateAccount();

	/** Update all payees on WebApp */
	static bool WebApp_UpdatePayee();

	/** Update all categories on WebApp */
	static bool WebApp_UpdateCategory();

	/** Return true if there are new transaction on WebApp */
	static bool WebApp_CheckNewTransaction();

	/** Download oldest new transaction JSON */
	static bool WebApp_DownloadNewTransaction(wxString& NewTransactionJSON);

	/** Insert transaction in MMEX desktop, returns transaction ID */
	static int MMEX_InsertNewTransaction(wxString& NewTransactionJSON);

	//FUNCTIONS CALLED IN MMEX TO UPDATE ON CHANGE
	/** Update all payees on WebApp if enabled */
	static bool MMEX_WebApp_UpdatePayee();

	/** Update all accounts on WebApp if enabled */
	static bool MMEX_WebApp_UpdateAccount();

	/** Update all categories on WebApp if enabled */
	static bool MMEX_WebApp_UpdateCategory();
};

#endif // MM_EX_WEBAPP_H_
