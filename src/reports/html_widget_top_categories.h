
#include "mmcoredb.h"
#include "reportbase.h"
#include "mmDateRange.h"

#ifndef HTML_WIDGET_TOP_CATEGORIES_H
#define HTML_WIDGET_TOP_CATEGORIES_H

class htmlWidgetTop7Categories : public mmPrintableBase
{
public:
    ~htmlWidgetTop7Categories();
    htmlWidgetTop7Categories(
        mmDateRange* date_range);

    wxString getHTMLText();

protected:
    mmDateRange* date_range_;
    wxString title_;
    void getTopCategoryStats(
        std::vector<std::pair<wxString, double> > &categoryStats
        , mmDateRange* date_range) const;
};

#endif //
