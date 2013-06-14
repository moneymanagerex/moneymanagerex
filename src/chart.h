//----------------------------------------------------------------------------
#ifndef _MM_EX_CHART_H_
#define _MM_EX_CHART_H_
//----------------------------------------------------------------------------
#include "defs.h"
#include <vector>
#include <wx/mstream.h>
//----------------------------------------------------------------------------

class ChartData
{
public:
    ChartData( const wxString &aKey, float aVal = 0 );
    ChartData( const wxString &aKey, const std::vector<float> &aSerie );

    wxString key;

    // normalized val
    float val;
    // real val
    float aval;

    std::vector<float> serie;
};
//----------------------------------------------------------------------------
enum EChartLegend { CHART_LEGEND_FIXED, CHART_LEGEND_FLOAT };
//----------------------------------------------------------------------------

class AbstractChart
{
public:
    AbstractChart( int aWidth, int aHeight );
    virtual ~AbstractChart() {}

    void SetBackground( const wxColour& colour );
    void SetFillColour( const wxColour& colour, int style = wxSOLID );
    void SetStrokeColour( const wxColour& colour, int width = 1, int style = wxSOLID );

    void ClearFillColour();
    void ClearStrokeColour();

    bool Save( const wxString& file );
    bool Save(wxMemoryOutputStream& stream);

    void SetData( const std::vector<ChartData> &aData, bool simpleValue = true );
    void SetData( const std::vector<ChartData> &aData, const std::vector<wxString> &aSerieLabel );

    virtual bool Render( const wxString& title ) = 0;

protected:

    std::vector<ChartData> data;
    std::vector<wxString> serieLabel;

    enum { PAL_MAX = 24 };
    wxColour palete[PAL_MAX];

    wxMemoryDC dc_;
    int width_;
    int height_;

    wxFont plainFont;
    wxFont boldFont;

private:
    wxBitmap image_;
};
//----------------------------------------------------------------------------
enum EPieChartStyle { PIE_CHART_PERCENT, PIE_CHART_ABSOLUT };
//----------------------------------------------------------------------------

class PieChart : public AbstractChart
{
public:
    PieChart( int aWidth, int aHeight );

    void Init( int aChartsize, EChartLegend aLegendMode = CHART_LEGEND_FIXED, EPieChartStyle aPieMode = PIE_CHART_PERCENT );
    bool Render( const wxString& title = "" );

private:
    int csize;
    int keymode;
    int mode;
};
//----------------------------------------------------------------------------
enum EBarChartStyle { BAR_CHART_SIMPLE, BAR_CHART_SERIES };
//----------------------------------------------------------------------------

class BarChart : public AbstractChart
{
public:
    BarChart( int aWidth, int aHeight );

    void Init( int aChartsize, EChartLegend aLegendMode = CHART_LEGEND_FIXED, EBarChartStyle aMode = BAR_CHART_SIMPLE );
    bool Render( const wxString& title = "" );

private:
    int csize;
    int keymode;
    int mode;
};
//----------------------------------------------------------------------------
#endif // _MM_EX_CHART_H_
//----------------------------------------------------------------------------

