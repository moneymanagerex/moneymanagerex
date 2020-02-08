/*
    Copyright (c) 2016-2018 Xavier Leclercq

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

/// @file

#ifndef _WX_CHARTS_WXCHARTSCATEGORICALDATA_H_
#define _WX_CHARTS_WXCHARTSCATEGORICALDATA_H_

#include "wxchartscategoricaldataobserver.h"
#include "wxchartsdoubledataset.h"
#include <wx/vector.h>

/// Data organized in a finite number of categories. Each category has one or more values associated with it.

/// Charts that are able to display this type of data are wxBarChart, wxColumnChart, wxStackedBarChart and 
/// wxStackedColumnChart. The categories are displayed on one the axes and the associated value or values
/// on the other.
/// \ingroup dataclasses
class wxChartsCategoricalData
{
public:
    /// Smart pointer typedef.
    typedef wxSharedPtr<wxChartsCategoricalData> ptr;

    /// Constructs a wxChartsCategoricalData instance.
    /// @param categories The categories.
    wxChartsCategoricalData(const wxVector<wxString> &categories);
    static ptr make_shared(const wxVector<wxString> &categories);

    /// Adds a dataset.
    /// @param dataset The dataset to add.
    void AddDataset(wxChartsDoubleDataset::ptr dataset);

    /// Gets the categories.
    /// @return A vector containing the categories.
    const wxVector<wxString>& GetCategories() const;
    const wxVector<wxChartsDoubleDataset::ptr>& GetDatasets() const;

    void AddObserver();

private:
    wxVector<wxString> m_categories;
    wxVector<wxChartsDoubleDataset::ptr> m_datasets;
    wxVector<wxChartsCategoricalDataObserver*> m_observers;
};

#endif
