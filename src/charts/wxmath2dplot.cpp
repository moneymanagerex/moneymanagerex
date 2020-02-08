/*
    Copyright (c) 2016-2019 Xavier Leclercq and the wxCharts contributors.

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

    Copyright (c) 2013-2017 Nick Downie
    Released under the MIT license
    https://github.com/nnnick/Chart.js/blob/master/LICENSE.md
*/

#include "wxmath2dplot.h"
#include "wxchartstheme.h"
#include <wx/dcmemory.h>
#include <sstream>
#include <limits>

wxMath2DPlotDataset::wxMath2DPlotDataset(
    const wxColor &lineColor,
    const wxColor &dotColor,
    const wxColor &dotStrokeColor,
    wxVector<wxPoint2DDouble> &data,
    const wxChartType &chartType,
    const bool &showDots,
    const bool &showLine)
    : m_showDots(showDots), m_dotColor(dotColor),
      m_dotStrokeColor(dotStrokeColor), m_showLine(showLine),
      m_lineColor(lineColor),m_data(data),m_type(chartType)
{
}

bool wxMath2DPlotDataset::ShowDots() const
{
    return m_showDots;
}

const wxColor& wxMath2DPlotDataset::GetDotColor() const
{
    return m_dotColor;
}

const wxColor& wxMath2DPlotDataset::GetDotStrokeColor() const
{
    return m_dotStrokeColor;
}

const wxChartType& wxMath2DPlotDataset::GetType() const
{
    return m_type;
}

bool wxMath2DPlotDataset::ShowLine() const
{
    return m_showLine;
}

const wxColor& wxMath2DPlotDataset::GetLineColor() const
{
    return m_lineColor;
}

const wxVector<wxPoint2DDouble>& wxMath2DPlotDataset::GetData() const
{
    return m_data;
}

wxMath2DPlotData::wxMath2DPlotData()
{
}

void wxMath2DPlotData::AddDataset(wxMath2DPlotDataset::ptr dataset)
{
    m_datasets.push_back(dataset);
}

const wxVector<wxMath2DPlotDataset::ptr>& wxMath2DPlotData::GetDatasets() const
{
    return m_datasets;
}

wxMath2DPlot::Point::Point(
    wxPoint2DDouble value,
    const wxChartTooltipProvider::ptr tooltipProvider,
    wxDouble x,
    wxDouble y,
    wxDouble radius,
    unsigned int strokeWidth,
    const wxColor &strokeColor,
    const wxColor &fillColor,
    wxDouble hitDetectionRange)
    : wxChartsPoint(x, y, radius, tooltipProvider, wxChartsPointOptions(strokeWidth, strokeColor, fillColor)),
      m_value(value), m_hitDetectionRange(hitDetectionRange)
{
}

wxPoint2DDouble wxMath2DPlot::Point::GetTooltipPosition() const
{
    wxPoint2DDouble position = wxChartsPoint::GetTooltipPosition();
    position.m_y -= 10;
    return position;
}

bool wxMath2DPlot::Point::HitTest(const wxPoint &point,const AxisFunc &F) const
{
    wxDouble distance = (F(point.x) - F(GetPosition().m_x));
    if (distance < 0)
    {
        distance = -distance;
    }
    return (distance < m_hitDetectionRange);
}

wxPoint2DDouble wxMath2DPlot::Point::GetValue() const
{
    return m_value;
}

wxMath2DPlot::Dataset::Dataset(bool showDots,
                               bool showLine,
                               const wxColor &lineColor,
                               const wxColor &dotStrokeColor,
                               const wxChartType &chartType)
    : m_showDots(showDots), m_showLine(showLine),
      m_lineColor(lineColor), m_dotStrokeColor(dotStrokeColor),
      m_dotColor(lineColor), m_type(chartType)
{
}

bool wxMath2DPlot::Dataset::ShowDots() const
{
    return m_showDots;
}

bool wxMath2DPlot::Dataset::ShowLine() const
{
    return m_showLine;
}

const wxColor& wxMath2DPlot::Dataset::GetLineColor() const
{
    return m_lineColor;
}

const wxColor& wxMath2DPlot::Dataset::GetDotColor() const
{
    return m_dotColor;
}

const wxColor& wxMath2DPlot::Dataset::GetDotStrokeColor() const
{
    return m_dotStrokeColor;
}

const wxChartType& wxMath2DPlot::Dataset::GetType() const
{
    return m_type;
}

void wxMath2DPlot::Dataset::SetType(const wxChartType &chartType)
{
    m_type = chartType;
}

const wxVector<wxMath2DPlot::Point::ptr>& wxMath2DPlot::Dataset::GetPoints() const
{
    return m_points;
}

void wxMath2DPlot::Dataset::AppendPoint(Point::ptr point)
{
    m_points.push_back(point);
}

wxMath2DPlot::wxMath2DPlot(const wxMath2DPlotData &data,
                           const wxSize &size)
    : m_options(wxChartsDefaultTheme->GetMath2DPlotOptions()),
    m_grid(
          wxPoint2DDouble(m_options->GetPadding().GetLeft(), m_options->GetPadding().GetRight()),
          size,
          GetMinXValue(data.GetDatasets(),m_options->GetAxisFuncX()),
          GetMaxXValue(data.GetDatasets(),m_options->GetAxisFuncX()),
          GetMinYValue(data.GetDatasets(),m_options->GetAxisFuncY()),
          GetMaxYValue(data.GetDatasets(),m_options->GetAxisFuncY()),
          m_options->GetGridOptions())
{
    Initialize(data);
}

wxMath2DPlot::wxMath2DPlot(const wxMath2DPlotData &data,
                           wxSharedPtr<wxMath2DPlotOptions> &options,
                           const wxSize &size)
    : m_options(options),
      m_grid(
          wxPoint2DDouble(m_options->GetPadding().GetLeft(), m_options->GetPadding().GetRight()),
          size,
          GetMinXValue(data.GetDatasets(),m_options->GetAxisFuncX()),
          GetMaxXValue(data.GetDatasets(),m_options->GetAxisFuncX()),
          GetMinYValue(data.GetDatasets(),m_options->GetAxisFuncY()),
          GetMaxYValue(data.GetDatasets(),m_options->GetAxisFuncY()),
          m_options->GetGridOptions())
{
    Initialize(data);
}

const wxChartCommonOptions& wxMath2DPlot::GetCommonOptions() const
{
    return m_options->GetCommonOptions();
}

const wxChartsGridOptions& wxMath2DPlot::GetGridOptions() const
{
    return m_grid.GetOptions();
}

void wxMath2DPlot::SetGridOptions(const wxChartsGridOptions& opt)
{
    m_grid.SetOptions(opt);
}

const wxMath2DPlotOptions& wxMath2DPlot::GetChartOptions() const
{
    return *m_options;
}

void wxMath2DPlot::SetChartOptions(const wxMath2DPlotOptions& opt)
{
    *m_options = opt;
    Update();
}

bool wxMath2DPlot::SetChartType(std::size_t index,const wxChartType &type)
{
    if(index >= m_datasets.size() || type == m_datasets[index]->GetType())
        return false;

    m_datasets[index]->SetType(type);
    return true;
}

void wxMath2DPlot::Save(const wxString &filename,
                        const wxBitmapType &type,
                        const wxSize &size,
                        const wxColor &backgroundColor)
{
    wxBitmap bmp(size.GetWidth(), size.GetHeight());
    wxMemoryDC mdc(bmp);
    mdc.SetBackground(wxBrush(backgroundColor));
    mdc.Clear();
    wxGraphicsContext* gc = wxGraphicsContext::Create(mdc);
    if (gc)
    {
        DoDraw(*gc, true);
        bmp.SaveFile(filename, type);
        delete gc;
    }
}

bool wxMath2DPlot::Scale(int coeff)
{
    return m_grid.Scale(coeff);
}

void wxMath2DPlot::Shift(double dx,double dy)
{
    m_grid.Shift(dx,-dy);
}

bool wxMath2DPlot::UpdateData(std::size_t index,const wxVector<wxPoint2DDouble> &points)
{
    if (index >= m_datasets.size())
    {
        return false;
    }

    auto transformX = m_options->GetAxisFuncX();
    auto transformY = m_options->GetAxisFuncY();
    Dataset::ptr newDataset(new Dataset(m_datasets[index]->ShowDots(),
        m_datasets[index]->ShowLine(),m_datasets[index]->GetLineColor(),
        m_datasets[index]->GetDotStrokeColor(),m_datasets[index]->GetType()));

    for (size_t j = 0; j < points.size(); ++j)
    {
        std::stringstream tooltip;
        tooltip << "(" << transformX(points[j].m_x) << "," << transformY(points[j].m_y) << ")";
        wxChartTooltipProvider::ptr tooltipProvider(
            new wxChartTooltipProviderStatic("", tooltip.str(), m_datasets[index]->GetLineColor())
        );

        Point::ptr point(
            new Point(points[j], tooltipProvider, 20 + j * 10, 0,
                m_options->GetDotRadius(), m_options->GetDotStrokeWidth(),
                m_datasets[index]->GetDotStrokeColor(), m_datasets[index]->GetDotColor(),
                m_options->GetHitDetectionRange()));

        newDataset->AppendPoint(point);
    }
    m_datasets[index] = newDataset;
    Update();
    return true;
}

bool wxMath2DPlot::AddData(std::size_t index,const wxVector<wxPoint2DDouble> &points)
{
    if(index >= m_datasets.size())
        return false;

    auto transformX = m_options->GetAxisFuncX();
    auto transformY = m_options->GetAxisFuncY();
    auto shift = m_datasets[index]->GetPoints().size();

    for (size_t j = 0; j < points.size(); ++j)
        {
        std::stringstream tooltip;
        tooltip << "(" << transformX(points[j].m_x) << "," << transformY(points[j].m_y) << ")";
        wxChartTooltipProvider::ptr tooltipProvider(
            new wxChartTooltipProviderStatic("", tooltip.str(), m_datasets[index]->GetLineColor())
            );

        Point::ptr point(
            new Point(points[j], tooltipProvider, 20 + (shift+j)* 10, 0,
            m_options->GetDotRadius(), m_options->GetDotStrokeWidth(),
            m_datasets[index]->GetDotStrokeColor(), m_datasets[index]->GetDotColor(),
            m_options->GetHitDetectionRange()));

        m_datasets[index]->AppendPoint(point);
        }
    Update();
    return true;
}

void wxMath2DPlot::AddDataset(const wxMath2DPlotDataset::ptr &newset,bool is_new)
{
    auto transformX = m_options->GetAxisFuncX();
    auto transformY = m_options->GetAxisFuncY();

    Dataset::ptr newDataset(new Dataset(newset->ShowDots(),newset->ShowLine(),
        newset->GetLineColor(),newset->GetDotStrokeColor(),newset->GetType()));

    const wxVector<wxPoint2DDouble>& datasetData = newset->GetData();
    for (size_t j = 0; j < datasetData.size(); ++j)
        {
        std::stringstream tooltip;
        tooltip << "(" << transformX(datasetData[j].m_x) << "," << transformY(datasetData[j].m_y) << ")";
        wxChartTooltipProvider::ptr tooltipProvider(
            new wxChartTooltipProviderStatic("", tooltip.str(), newset->GetLineColor())
            );

        Point::ptr point(
            new Point(datasetData[j], tooltipProvider, 20 + j * 10, 0,
            m_options->GetDotRadius(), m_options->GetDotStrokeWidth(),
            newset->GetDotStrokeColor(), newset->GetDotColor(),
            m_options->GetHitDetectionRange()));

        newDataset->AppendPoint(point);
        }
    m_datasets.push_back(newDataset);

    if(is_new)
        Update();
}

bool wxMath2DPlot::RemoveDataset(std::size_t index)
{
    if(index >= m_datasets.size())
        return false;
    m_datasets.erase(m_datasets.begin()+index);
    Update();
    return true;
}

void wxMath2DPlot::Update()
{
    wxDouble minX = std::numeric_limits<wxDouble>::max();
    wxDouble maxX = std::numeric_limits<wxDouble>::min();
    wxDouble minY = minX;
    wxDouble maxY = maxX;
    auto transformX = m_options->GetAxisFuncX();
    auto transformY = m_options->GetAxisFuncY();
    for (size_t i = 0; i < m_datasets.size(); ++i)
        {
        for (const auto &el : m_datasets[i]->GetPoints())
            {
            auto value = el->GetValue();
            auto tX = transformX(value.m_x);
            auto tY = transformY(value.m_y);
            if (maxX < tX)
                maxX = tX;
            if (minX > tX)
                minX = tX;
            if (maxY < tY)
                maxY = tY;
            if (minY > tY)
                minY = tY;
            }
        }
    if (maxX == std::numeric_limits<wxDouble>::min())
            maxX = transformX(0);
    if (minX == std::numeric_limits<wxDouble>::max())
            minX = transformX(0);
    if (maxY == std::numeric_limits<wxDouble>::min())
            maxY = transformY(0);
    if (minY == std::numeric_limits<wxDouble>::max())
            minY = transformY(0);
    m_grid.UpdateAxisLimit("x",minX,maxX);
    m_grid.UpdateAxisLimit("y",minY,maxY);
}

void wxMath2DPlot::Initialize(const wxMath2DPlotData &data)
{
    for (const auto &el : data.GetDatasets())
        AddDataset(el,false);
}

wxDouble wxMath2DPlot::GetMinXValue(const wxVector<wxMath2DPlotDataset::ptr>& datasets,const AxisFunc &F)
{
    wxDouble result = 0;
    bool foundValue = false;
    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxPoint2DDouble>& values = datasets[i]->GetData();
        for (size_t j = 0; j < values.size(); ++j)
        {
            if (!foundValue)
            {
                result = F(values[j].m_x);
                foundValue = true;
            }
            else if (result > F(values[j].m_x))
            {
                result = F(values[j].m_x);
            }
        }
    }

    return result;
}

wxDouble wxMath2DPlot::GetMaxXValue(const wxVector<wxMath2DPlotDataset::ptr>& datasets,const AxisFunc &F)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxPoint2DDouble>& values = datasets[i]->GetData();
        for (size_t j = 0; j < values.size(); ++j)
        {
            if (!foundValue)
            {
                result = F(values[j].m_x);
                foundValue = true;
            }
            else if (result < F(values[j].m_x))
            {
                result = F(values[j].m_x);
            }
        }
    }

    return result;
}

wxDouble wxMath2DPlot::GetMinYValue(const wxVector<wxMath2DPlotDataset::ptr>& datasets,const AxisFunc &F)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxPoint2DDouble>& values = datasets[i]->GetData();
        for (size_t j = 0; j < values.size(); ++j)
        {
            if (!foundValue)
            {
                result = F(values[j].m_y);
                foundValue = true;
            }
            else if (result > F(values[j].m_y))
            {
                result = F(values[j].m_y);
            }
        }
    }

    return result;
}

wxDouble wxMath2DPlot::GetMaxYValue(const wxVector<wxMath2DPlotDataset::ptr>& datasets,const AxisFunc &F)
{
    wxDouble result = 0;
    bool foundValue = false;

    for (size_t i = 0; i < datasets.size(); ++i)
    {
        const wxVector<wxPoint2DDouble>& values = datasets[i]->GetData();
        for (size_t j = 0; j < values.size(); ++j)
        {
            if (!foundValue)
            {
                result = F(values[j].m_y);
                foundValue = true;
            }
            else if (result < F(values[j].m_y))
            {
                result = F(values[j].m_y);
            }
        }
    }

    return result;
}

void wxMath2DPlot::DoSetSize(const wxSize &size)
{
    wxSize newSize(
        size.GetWidth() - m_options->GetPadding().GetTotalHorizontalPadding(),
        size.GetHeight() - m_options->GetPadding().GetTotalVerticalPadding()
    );
    m_grid.Resize(newSize);
}

void wxMath2DPlot::DoFit()
{
    auto transformX = m_options->GetAxisFuncX();
    auto transformY = m_options->GetAxisFuncY();
    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        const wxVector<Point::ptr>& points = m_datasets[i]->GetPoints();
        for (size_t j = 0; j < points.size(); ++j)
        {
            const Point::ptr& point = points[j];
            point->SetPosition(m_grid.GetMapping().GetWindowPosition(
                transformX(point->GetValue().m_x), transformY(point->GetValue().m_y)));
        }
    }
}

void wxMath2DPlot::DoDraw(wxGraphicsContext &gc,
                          bool suppressTooltips)
{
    m_grid.Fit(gc);
    m_grid.Draw(gc);
    Fit();

    auto transformX = m_options->GetAxisFuncX();
    auto transformY = m_options->GetAxisFuncY();

    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        const wxVector<Point::ptr>& points = m_datasets[i]->GetPoints();

        wxGraphicsPath path = gc.CreatePath();

        if (points.size() > 0)
        {
            const Point::ptr& point = points[0];
            auto value = point->GetValue();

            wxPoint2DDouble firstPosition = m_grid.GetMapping().GetWindowPosition(transformX(value.m_x),transformY(value.m_y));
            path.MoveToPoint(firstPosition);

            wxPoint2DDouble lastPosition;
            for (size_t j = 1; j < points.size(); ++j)
            {
                const Point::ptr& point = points[j];
                value = point->GetValue();
                lastPosition =  m_grid.GetMapping().GetWindowPosition(transformX(value.m_x),transformY(value.m_y));

                if(m_datasets[i]->GetType()==wxCHARTTYPE_STEPPED)
                {
                    wxDouble y = transformY(value.m_y);
                    value = points[j-1]->GetValue();
                    value = m_grid.GetMapping().GetWindowPosition(transformX(value.m_x),y);
                    path.AddLineToPoint(value);
                }

                if(m_datasets[i]->GetType()==wxCHARTTYPE_STEM)
                {
                    value = points[j-1]->GetValue();
                    value = m_grid.GetMapping().GetWindowPosition(transformX(value.m_x),transformY(0));
                    path.AddLineToPoint(value);
                    path.MoveToPoint(lastPosition);
                    if(j+1==points.size())
                    {
                        value = points[j]->GetValue();
                        value = m_grid.GetMapping().GetWindowPosition(transformX(value.m_x),transformY(0));
                        path.AddLineToPoint(value);
                    }
                }
                else
                {
                    path.AddLineToPoint(lastPosition);
                }
            }
            wxPen pen;
            if (m_datasets[i]->ShowLine())
            {
                pen = wxPen(m_datasets[i]->GetLineColor(), m_options->GetLineWidth());
            }
            else
            {
                pen = wxPen(wxColor(255, 255, 255, 0), 0);
            }
            gc.SetPen(pen);
            gc.StrokePath(path);
        }

        if (m_datasets[i]->ShowDots())
        {
            for (size_t j = 0; j < points.size(); ++j)
            {
                points[j]->Draw(gc);
            }
        }
    }

    if (!suppressTooltips)
    {
        DrawTooltips(gc);
    }
}

wxSharedPtr<wxVector<const wxChartsElement*>> wxMath2DPlot::GetActiveElements(const wxPoint &point)
{
    wxSharedPtr<wxVector<const wxChartsElement*>> activeElements(new wxVector<const wxChartsElement*>());
    for (size_t i = 0; i < m_datasets.size(); ++i)
    {
        const wxVector<Point::ptr>& points = m_datasets[i]->GetPoints();
        for (size_t j = 0; j < points.size(); ++j)
        {
            if (points[j]->HitTest(point,m_options->GetAxisFuncX()))
            {
                activeElements->push_back(points[j].get());
            }
        }
    }
    return activeElements;
}
