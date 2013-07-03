
#include "reportbase.h"
#include "mmDateRange.h"

class htmlWidgetTop7Categories : public mmPrintableBase
{
public:
    ~htmlWidgetTop7Categories();
    htmlWidgetTop7Categories(mmCoreDB* core
        , const wxString& title = wxString::Format(_("Top Withdrawals: %s"), _("Last 30 Days"))
        , mmDateRange* date_range = new mmLast30Days());

    wxString getHTMLText();

protected:
    mmDateRange* date_range_;
    wxString title_;
};

