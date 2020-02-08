/*
    Copyright (c) 2016-2019 Xavier Leclercq

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

/*
    Part of this file were copied from the Chart.js project (http://chartjs.org/)
    and translated into C++.

    The files of the Chart.js project have the following copyright and license.

    Copyright (c) 2013-2016 Nick Downie
    Released under the MIT license
    https://github.com/nnnick/Chart.js/blob/master/LICENSE.md
*/

/// @file

#ifndef _WX_CHARTS_WXCHARTSAXIS_H_
#define _WX_CHARTS_WXCHARTSAXIS_H_

#include "wxchartselement.h"
#include "wxchartsaxisoptions.h"
#include "wxchartslabelgroup.h"
#include <wx/graphics.h>

/// This class represents an axis.

/// \ingroup elementclasses
class wxChartsAxis : public wxChartsElement
{
public:
    /// Smart pointer typedef.
    typedef wxSharedPtr<wxChartsAxis> ptr;

    virtual void Draw(wxGraphicsContext &gc) const;

    virtual bool HitTest(const wxPoint &point) const;

    virtual wxPoint2DDouble GetTooltipPosition() const;

    /// Updates the size of each label using the 
    /// font details specified in the axis options
    /// and the provided graphics context.
    /// @param gc The graphics context.
    void UpdateLabelSizes(wxGraphicsContext &gc);
    void Fit(wxPoint2DDouble startPoint, wxPoint2DDouble endPoint);
    void UpdateLabelPositions();

    /// Gets the labels.
    /// @return The list of labels.
    const wxChartsLabelGroup& GetLabels() const;
    void SetLabels(const wxVector<wxChartsLabel> &labels);

    wxPoint2DDouble CalculateLabelPosition(size_t index);
    size_t GetNumberOfTickMarks() const;
    wxDouble GetDistanceBetweenTickMarks() const;
    wxPoint2DDouble GetTickMarkPosition(size_t index) const;
    wxPoint2DDouble GetPosition(wxDouble relativeValue) const;

    const std::string& GetId() const;
    const wxChartsAxisOptions& GetOptions() const;

protected:
    /// Construcs a wxChartsAxis element.
    /// @param options The settings to be used for the axis.
    wxChartsAxis(const std::string &id, const wxChartsAxisOptions &options);
    /// Constructs a wxChartsAxis element.
    /// @param labels The labels to display along the axis.
    /// @param options The settings to be used for the axis.
    wxChartsAxis(const std::string &id, const wxVector<wxString> &labels,
        const wxChartsAxisOptions &options);

private:
    void DrawTickMarks(wxGraphicsContext &gc) const;

private:
    wxChartsAxisOptions m_options;
    std::string m_id;
    wxPoint2DDouble m_startPoint;
    wxPoint2DDouble m_endPoint;
    wxChartsLabelGroup m_labels;
};

#endif
