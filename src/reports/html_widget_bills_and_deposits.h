
#include "reportbase.h"
#include "mmDateRange.h"

#ifndef HTML_WIDGET_BILLS_AND_DEPOSITS_H
#define HTML_WIDGET_BILLS_AND_DEPOSITS_H

class htmlWidgetBillsAndDeposits : public mmPrintableBase
{
public:
    ~htmlWidgetBillsAndDeposits();
    htmlWidgetBillsAndDeposits(mmCoreDB* core
        , const wxString& title
        , mmDateRange* date_range = new mmAllTime());

    wxString getHTMLText();

protected:
    mmDateRange* date_range_;
    wxString title_;
};

#endif //
