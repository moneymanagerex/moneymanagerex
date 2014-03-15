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
const static wxString geturl();
const static wxString getguid();
const static wxString getdelimiter();
const static wxString getservicespage();
const static wxString getimportaccountparameter();
const static wxString getimportpayeeparameter();
const static wxString getdeletealltransactionparameter();
const static wxString getdownloadnewtransactionparameter();

public:
	const static wxString getmessagesucceeded();
	const static wxString getmessagewrongguid();
	bool UpdateAccount();
	bool UpdatePayee();
	bool DeleteAllTransaction();
	bool DownloadNewTransaction();
};

#endif _MM_EX_WEBAPP_H_