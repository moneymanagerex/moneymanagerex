
#include "reportbase.h"
#include "mmDateRange.h"

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

