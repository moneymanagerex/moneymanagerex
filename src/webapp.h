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

#ifndef _MM_EX_WEBAPP_H_
#define _MM_EX_WEBAPP_H_

#include "defs.h"
#include <wx/spinctrl.h>

class mmWebApp
{
const static wxString getUrl();
const static wxString getGuid();
const static wxString getDelimiter();
const static wxString getServicesPage();
const static wxString getCheckGuidParameter();
const static wxString getCheckApiVersionParameter();
const static wxString getCheckNewTransactionParameter();
const static wxString getImportAccountParameter();
const static wxString getImportPayeeParameter();
const static wxString getDeletePayeeParameter();
const static wxString getDeleteOneTransactionParameter();
const static wxString getWebApp_DeleteAllTransactionParameter();
const static wxString getDownloadNewTransactionParameter();
const static wxString getMessageSucceeded();
const static wxString getMessageWrongGuid();

public:
	const static wxString getDbFilePath();
	static wxString getApiExpectedVersion();
	static bool returnResult(const int& ErrorCode, wxString& outputMessage);
	static bool WebApp_CheckGuid();
	static wxString WebApp_getApiVersion();
	static bool WebApp_CheckNewTransaction();
	static bool WebApp_UpdateAccount();
	static bool WebApp_UpdatePayee();
	static bool WebApp_DeleteAllPayee();
	static bool WebApp_DeleteOneTransaction(const int& WebAppNewTransactionId);
	static bool WebApp_DeleteAllTransaction();
	static bool WebApp_DownloadNewTransaction();
	static int Local_getNextTransactionID();
	static int MMEX_InsertNewTransaction(const int& WebAppNewTransactionId);
	static bool Local_DeleteOneTransaction(const int& LocalTransactionId);
};

#endif // _MM_EX_WEBAPP_H_
