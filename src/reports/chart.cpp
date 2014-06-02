/*******************************************************
Copyright (C) 2006-2012

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

//----------------------------------------------------------------------------
#include "chart.h"
//----------------------------------------------------------------------------
#include <limits>
#include <algorithm> // min, max
#include <math.h>
#include <wx/fs_mem.h>
//----------------------------------------------------------------------------

namespace
{

int ROUND( double x )
{
        double res = floor( x + 0.5 );
        return static_cast<int>( res ); // (int)(x + 0.5)
}
//----------------------------------------------------------------------------

float getFrequency(float vmax)
{
    int max_steps = 5;
    float step = vmax/max_steps;

    const float base = 10;
    int y = 0;

    while ( pow(base, ++y) < step );
    float new_step = pow(base, --y);

    if (step > new_step)
    {
        size_t cnt = floor(step/new_step + 0.5f);
        if (cnt > 1)
            new_step *= cnt;
    }

    return new_step;
}
//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

ChartData::ChartData( const wxString &aKey, float aVal ) :
                key( aKey ),
                val( aVal ),
                aval( aVal )
{
}
//----------------------------------------------------------------------------

ChartData::ChartData( const wxString &aKey, const std::vector<float> &aSerie ) :
                key( aKey ),
                val(),
                aval(),
                serie( aSerie )
{
}
//----------------------------------------------------------------------------

AbstractChart::AbstractChart( int aWidth, int aHeight ) :
                width_( aWidth ),
                height_( aHeight )
{
    int i = -1;
    palete[++i] = wxColour( 0x00, 0x79, 0xEA );
    palete[++i] = wxColour( 0xee, 0x2A, 0x00 );
    palete[++i] = wxColour( 0xF7, 0x97, 0x31 );
    palete[++i] = wxColour( 0xBD, 0x7F, 0xAE );
    palete[++i] = wxColour( 0xFF, 0xF3, 0xAB );
    palete[++i] = wxColour( 0x66, 0xAE, 0x3F );
    palete[++i] = wxColour( 0xBB, 0x7F, 0xB8 );
    palete[++i] = wxColour( 0x64, 0x91, 0xAA );
    palete[++i] = wxColour( 0xE8, 0xC1, 0x45 );
    palete[++i] = wxColour( 0x2B, 0x96, 0xE7 );
    palete[++i] = wxColour( 0xD2, 0x9A, 0xf7 );
    palete[++i] = wxColour( 0x8F, 0xEA, 0x7B );
    palete[++i] = wxColour( 0xFF, 0xFF, 0x3B );
    palete[++i] = wxColour( 0x58, 0xCC, 0xCC );
    palete[++i] = wxColour( 0x7A, 0xB3, 0x3E );
    palete[++i] = wxColour( 0x42, 0x44, 0x3F );
    palete[++i] = wxColour( 0xFC, 0xAC, 0x00 );
    palete[++i] = wxColour( 0xA2, 0xE1, 0x4A );
    palete[++i] = wxColour( 0xa8, 0x62, 0x16 );
    palete[++i] = wxColour( 0xC3, 0xD9, 0xFF );
    palete[++i] = wxColour( 0xC7, 0x98, 0x10 );
    palete[++i] = wxColour( 0x6B, 0xBA, 0x70 );
    palete[++i] = wxColour( 0xCD, 0xEB, 0x8B );
    palete[++i] = wxColour( 0xD0, 0x1F, 0x3C );

    wxASSERT( ++i == PAL_MAX );

    image_ = wxBitmap( width_, height_ );
    dc_.SelectObject( image_ );

    ClearFillColour();
    ClearStrokeColour();

    plainFont = wxFont( 8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
    boldFont = wxFont( 8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD );
}
//----------------------------------------------------------------------------

void AbstractChart::SetBackground( const wxColour& colour )
{
    ClearStrokeColour();
    SetFillColour( colour );
    dc_.Clear();
}
//----------------------------------------------------------------------------

void AbstractChart::SetFillColour( const wxColour& colour, int style )
{
    wxBrush brush = wxBrush( colour, style );
    dc_.SetBrush( brush );
}
//----------------------------------------------------------------------------

void AbstractChart::SetStrokeColour( const wxColour& colour, int width, int style )
{
    wxPen pen = wxPen( colour, width, style );
    dc_.SetPen( pen );
}
//----------------------------------------------------------------------------

void AbstractChart::ClearFillColour()
{
    dc_.SetBrush( *wxTRANSPARENT_BRUSH );
}
//----------------------------------------------------------------------------

void AbstractChart::ClearStrokeColour()
{
    dc_.SetPen( *wxTRANSPARENT_PEN );
}
//----------------------------------------------------------------------------

bool AbstractChart::Save( const wxString& file )
{
    wxImage pic = image_.ConvertToImage();
    pic.SetMaskColour(255,255,255);
    wxString f;
    if (file.StartsWith("memory:", &f))
        wxMemoryFSHandler::AddFile(f, pic, wxBITMAP_TYPE_PNG);
    else
        pic.SaveFile( file, wxBITMAP_TYPE_PNG );
    return true;
}

bool AbstractChart::Save(wxMemoryOutputStream& stream)
{
    wxImage pic = image_.ConvertToImage();
    pic.SetMaskColour(255,255,255);
    pic.SaveFile(stream, wxBITMAP_TYPE_PNG );
    return true;
}
//----------------------------------------------------------------------------

void AbstractChart::SetData( const std::vector<ChartData> &aData, bool simpleValue )
{
    data = aData;

    if ( !simpleValue ) return;

    float vtotal = 0;

    for ( size_t i = 0; i < data.size(); ++i )
        vtotal += fabs( data[i].val );

    // normalize 0..100 for percentage charts
    for ( size_t i = 0; i < data.size(); ++i )
        data[i].val = fabs( data[i].val ) * 100.0 / vtotal;
}
//----------------------------------------------------------------------------

void AbstractChart::SetData( const std::vector<ChartData> &aData, const std::vector<wxString> &aSerieLabel )
{
    SetData( aData, false );
    serieLabel = aSerieLabel;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

PieChart::PieChart( int aWidth, int aHeight ) : AbstractChart( aWidth, aHeight )
, csize(0)
, keymode(0)
, mode(0)
{
}
//----------------------------------------------------------------------------

void PieChart::Init( int aChartsize, EChartLegend aLegendMode, EPieChartStyle aPieMode )
{
    csize = aChartsize;
    keymode = aLegendMode;
    mode = aPieMode;

    SetBackground( *wxWHITE );
}
//----------------------------------------------------------------------------

bool PieChart::Render( const wxString& title )
{
    if ( data.empty() ) return false;

    if ( !title.empty() )
    {
        SetStrokeColour( *wxBLACK );
        ClearFillColour();

        dc_.SetFont( boldFont );
        dc_.DrawText( title, 10, 20 - dc_.GetTextExtent( title ).GetHeight() );
    }

    // where to start drawing from
    int originLeft = 0;

    if ( keymode == CHART_LEGEND_FIXED )
        originLeft = 10;
    else
        originLeft = ( width_ - csize ) / 2;

    ClearStrokeColour();
    SetFillColour( *wxBLACK );

    // background
    dc_.DrawEllipse( originLeft - 2, ROUND( ( ( height_ - 12 ) / 2 ) - csize / 2.0 ) + 12, csize + 4, csize + 4 );

    if ( keymode == CHART_LEGEND_FIXED )
    {
        SetStrokeColour( *wxBLACK );
        ClearFillColour();

        dc_.SetFont( boldFont );
        wxString legend = _( "Legend:" );
        dc_.DrawText( legend, csize + 23, 25 - dc_.GetTextExtent( legend ).GetHeight() );

        dc_.SetFont( plainFont );

        for ( size_t i = 0; i < data.size(); ++i )
        {
            if ( mode == PIE_CHART_PERCENT )
                dc_.DrawText(
                        data[i].key + " ("  + wxString::Format( "%.2f" , data[i].val ) + "%)" ,
                        csize + 37,
                        40 + i * 18 - 11 );
            else
                dc_.DrawText(
                        data[i].key, // + " (" + wxString::Format("%.2f", data[i].aval) + ")"
                        csize + 37,
                        40 + i * 18 - 11 );
        }

        dc_.DrawRectangle(
                csize + 18,
                10,
                ( width_ - ( csize + 18 ) - 10 ),
                ( int ) ( 20 + data.size() * 18 ) );
    }
    else
    {
        int l = originLeft + ROUND( csize / 2.0 );
        int i1 = ROUND( ( height_ - 12 ) / 2 ) + 12;

        float f1 = csize / 2.0 + 6.0;  // offset of the label to the pie
        float f2 = 0.0;

        for ( size_t j = 0; j < data.size(); ++j )
        {
            float f3 = ( f2 + data[j].val / 2.0 ) - 25.0;
            int j1 = l + ( int ) ( f1 * cos( f3 * 0.062831799999999993 ) );
            int k1 = i1 + ( int ) ( f1 * sin( f3 * 0.062831799999999993 ) );

            dc_.DrawLine( l, i1, j1, k1 );
            dc_.DrawLine( j1, k1, j1 + 6 * ( ( j1 > l ) ? 1 : -1 ), k1 );

            dc_.SetFont( plainFont );
            wxString s;

            if ( mode == PIE_CHART_PERCENT )
                s = data[j].key + " ("  + wxString::Format( "%.2f" , data[j].val ) + "%)" ;
            else
                s = data[j].key; // + " (" + wxString::Format("%.2f", data[j].aval) + ")"

            int l1;

            if ( j1 > l )
                l1 = j1 + 6 + 1;
            else
                l1 = j1 - 6 - ( dc_.GetTextExtent( s ).GetWidth() + 1 );

            int i2 = k1 + 6;
            dc_.DrawText( s, l1, i2 - dc_.GetTextExtent( s ).GetHeight() );
            f2 += data[j].val;
        }
    }

    float f = 25.0;

    for ( size_t k = 0; k < data.size(); ++k )
    {
        ClearStrokeColour();
        SetFillColour( palete[k % PAL_MAX] );

        if ( keymode == CHART_LEGEND_FIXED )
            dc_.DrawRectangle( csize + 23, 30 + k * 18, 12, 12 );

        dc_.DrawEllipticArc(
                originLeft,
                ROUND( ( ( height_ - 12 ) / 2 ) - csize / 2.0 ) + 12,
                csize,
                csize,
                ROUND( f * 3.6 ) + ceil( data[k].val * 3.6 ) * -1,
                ROUND( f * 3.6 ) );
        f -= data[k].val;

        if ( f < 0.0 )
            f += 100;
    }

    return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

BarChart::BarChart( int aWidth, int aHeight ) : AbstractChart( aWidth, aHeight )
, csize(0)
, keymode(0)
, mode(0)
{
}
//----------------------------------------------------------------------------

void BarChart::Init( int aChartsize, EChartLegend aLegendMode, EBarChartStyle aMode )
{
    csize = aChartsize;
    keymode = aLegendMode;
    mode = aMode;

    SetBackground( *wxWHITE );
}
//----------------------------------------------------------------------------

bool BarChart::Render( const wxString& title )
{
    if ( data.empty() ) return false;

    // calculate min/max value (boundaries)
    float min = 0;
    float max = 0;

    if ( mode == BAR_CHART_SIMPLE )
    {
        // this forces the chart to start from 0 if values are all greater than 0
        min = 0;
        max = data[0].aval;

        for ( size_t i = 0; i < data.size(); ++i )
        {
            min = std::min( min, data[i].aval );
            max = std::max( max, data[i].aval );
        }
    }
    else
    {
        // this forces the chart to start from 0 if values are all greater than 0
        min = 0;
        max = data[0].serie[0];

        for ( size_t i = 0; i < data.size(); ++i )
        {
            for ( size_t j = 0; j < data[i].serie.size(); ++j )
            {
                min = std::min( min, data[i].serie[j] );
                max = std::max( max, data[i].serie[j] );
            }
        }
    }

    // avoid out of scale for empty data sets
    if ( min == max ) max = 1;

    const float freq = getFrequency( fabs(min) + fabs(max) );

    if ( !title.empty() )
    {
        SetStrokeColour( *wxBLACK );
        ClearFillColour();

        dc_.SetFont( boldFont );
        dc_.DrawText( title, 10, 20 - dc_.GetTextExtent( title ).GetHeight() );
    }

    // where to start drawing from
    int originLeft = 1;

    while ( max > pow( 10.0f, originLeft++ ) );

    originLeft *= 7;

    int gap = 5;

    int barwidth;

    if ( mode == BAR_CHART_SIMPLE )
    {
        if ( keymode == CHART_LEGEND_FIXED )
        {
            barwidth = ( int ) ( ( csize - gap * data.size() ) / data.size() );
        }
        else
        {
            barwidth = ( int ) ( ( width_ - originLeft - gap * data.size() - 10 ) / data.size() );
        }
    }
    else
    {
        if ( keymode == CHART_LEGEND_FIXED )
        {
            barwidth = ( int ) ( ( csize - gap * data.size() ) / ( data.size() * serieLabel.size() ) );
        }
        else
        {
            barwidth = ( int ) ( ( width_ - originLeft - gap * data.size() - 10 ) / ( data.size() * serieLabel.size() ) );
        }
    }

    // draw the legend/key of the chart
    if ( keymode == CHART_LEGEND_FIXED )
    {
        SetStrokeColour( *wxBLACK );
        ClearFillColour();

        dc_.SetFont( boldFont );
        wxString legend = _( "Legend:" );
        dc_.DrawText( legend, originLeft + csize + 15, 25 - dc_.GetTextExtent( legend ).GetHeight() );

        dc_.SetFont( plainFont );

        if ( mode == BAR_CHART_SIMPLE )
        {
            for ( size_t i = 0; i < data.size(); ++i )
            {
                dc_.DrawText(
                        data[i].key /* + " (" + wxString::Format("%.2f", data[i].aval) + ")" */,
                        originLeft + csize + 30,
                        43 + i * 18 - dc_.GetTextExtent( legend ).GetHeight() );
            }
        }
        else
        {
            for ( size_t i = 0; i < serieLabel.size(); ++i )
            {
                dc_.DrawText(
                        serieLabel[i] /* + " (" + wxString::Format("%.2f", data[i].aval) + ")" */,
                        originLeft + csize + 30,
                        43 + i * 18 - dc_.GetTextExtent( legend ).GetHeight() );
            }
        }

        if ( mode == BAR_CHART_SIMPLE )
        {
            dc_.DrawRectangle(
                    originLeft + csize + 10,
                    10,
                    width_ - ( originLeft + csize + 15 ),
                    ( int ) ( 20 + data.size() * 18 ) );
        }
        else
        {
            dc_.DrawRectangle(
                    originLeft + csize + 10,
                    10,
                    width_ - ( originLeft + csize + 15 ),
                    ( int ) ( 20 + serieLabel.size() * 18 ) );
        }

        // draw axis
        dc_.DrawLine( originLeft - 3, height_ - 25, originLeft + csize + 3, height_ - 25 );
        dc_.DrawLine( originLeft, height_ - 22, originLeft, 22 );
        // vertical axis labels
        int y = height_ - 25;
        int step = static_cast<int>( ( height_ - 50 ) / ( ( fabs( min ) + fabs( max ) ) / freq ) );

        for ( float fi = min; fi <= max; fi += freq )
        {
            // draw the label
            ClearFillColour();
            SetStrokeColour( *wxBLACK );

            wxString label = wxString::Format( "%.0f" , fi );
            dc_.DrawText( label, 3, y - dc_.GetTextExtent( label ).GetHeight() );

            // 1st entry skip grid, otherwise axis is overriden
            if ( fi != min )
            {
                // draw grid
                ClearFillColour();
                SetStrokeColour( *wxLIGHT_GREY );

                dc_.DrawLine( originLeft + 3, y, originLeft + csize - 3, y );
            }

            y -= step;
        }


        if ( mode == BAR_CHART_SERIES )
        {
            int labelOffset = ( int ) ( ( ( barwidth * serieLabel.size() ) + gap ) / 2 );

            SetStrokeColour( *wxBLACK );
            ClearFillColour();

            for ( int i = 0; i < static_cast<int>( data.size() ); ++i )
            {
                wxString key = data[i].key /* + " (" + wxString::Format("%.2f", data[i].aval) + ")" */;
                wxCoord x = originLeft + i * ( barwidth * static_cast<int>( serieLabel.size() ) ) + ( i + 1 ) * gap + labelOffset - ( dc_.GetTextExtent( key ).GetWidth() / 2 );
                wxCoord y = height_ - 10 - dc_.GetTextExtent( key ).GetHeight();
                dc_.DrawText( key, x, y );
            }
        }
    }
    else
    {
        SetStrokeColour( *wxBLACK );
        ClearFillColour();

        // draw axis
        dc_.DrawLine( originLeft - 3, height_ - 25, width_ - 5, height_ - 25 );
        dc_.DrawLine( originLeft, height_ - 22, originLeft, 22 );

        dc_.SetFont( plainFont );

        int y = height_ - 25;
        int step = static_cast<int>( ( height_ - 50 ) / ( ( fabs( min ) + fabs( max ) ) / freq ) );

        // horiz axis labels
        if ( mode == BAR_CHART_SIMPLE )
        {
            int labelOffset = ( barwidth + gap ) / 2;
            labelOffset -= 20;

            for ( size_t i = 0; i < data.size(); ++i )
            {
                wxString key = data[i].key /* + " (" + wxString::Format("%.2f", data[i].aval) + ")" */;
                dc_.DrawText(
                        key,
                        originLeft + i*barwidth + ( i + 1 )*gap + labelOffset /* - (dc_.GetTextExtent(key).GetLength() / 2) */,
                        height_ - 10 - dc_.GetTextExtent( key ).GetHeight() );
            }
        }

        // vertical axis labels
        for ( float fi = min; fi <= max; fi += freq )
        {
            // draw the label
            ClearFillColour();
            SetStrokeColour( *wxBLACK );

            wxString label = wxString::Format( "%.0f" , fi );
            dc_.DrawText( label, 3, y - dc_.GetTextExtent( label ).GetHeight() );

            // 1st entry skip grid, otherwise axis is overriden
            if ( fi != min )
            {
                    // draw grid
                    ClearFillColour();
                    SetStrokeColour( *wxLIGHT_GREY );

                    dc_.DrawLine( originLeft + 3, y, width_ - 8, y );
            }

            y -= step;
        }

        if ( mode == BAR_CHART_SERIES )
        {
            int labelOffset = ( int ) ( ( ( barwidth * serieLabel.size() ) + gap ) / 2 );

            SetStrokeColour( *wxBLACK );
            ClearFillColour();

            for ( int i = 0; i < static_cast<int>( data.size() ); ++i )
            {
                wxString key = data[i].key /* + " (" + wxString::Format("%.2f", data[i].aval) + ")" */;
                wxCoord x = originLeft + i * ( barwidth * static_cast<int>( serieLabel.size() ) ) + ( i + 1 ) * gap + labelOffset /* - (dc_.GetTextExtent(key).GetLength() / 2) */;
                wxCoord y = height_ - 10 - dc_.GetTextExtent( key ).GetHeight();
                dc_.DrawText( key, x, y );
            }
        }
    }

    // draw the bars
    if ( mode == BAR_CHART_SIMPLE )
    {
        int zero = ( int ) ( min * ( height_ - 50 ) / ( max - min ) );

        for ( size_t k = 0; k < data.size(); ++k )
        {
            ClearStrokeColour();
            SetFillColour( palete[k % PAL_MAX] );

            int h = static_cast<int>( data[k].aval * ( height_ - 50 ) / ( max - min ) );

            if ( h == 0 )
            {
                if ( data[k].aval > 0 )
                {
                    h = 1;
                }
                else if ( data[k].aval < 0 )
                {
                    h = -1;
                }
            }

            if ( data[k].aval >= 0 )
            {
                dc_.DrawRectangle(
                        originLeft + k * barwidth + ( k + 1 ) * gap,
                        ( height_ - 25 ) + zero - h,
                        barwidth,
                        h );
            }
            else
            {
                dc_.DrawRectangle(
                        originLeft + k * barwidth + ( k + 1 ) * gap,
                        ( height_ - 25 ) + zero,
                        barwidth,
                        -h );
            }

            // key tag
            if ( keymode == CHART_LEGEND_FIXED )
            {
                dc_.DrawRectangle(
                        originLeft + csize + 15,
                        33 + k * 18,
                        10,
                        10 );
            }
        }
    }
    else
    {
        int zero = ( int ) ( min * ( height_ - 50 ) / ( max - min ) );

        for ( size_t k = 0; k < data.size(); ++k )
        {
            for ( size_t j = 0; j < serieLabel.size(); ++j )
            {
                ClearStrokeColour();
                SetFillColour( palete[j % PAL_MAX] );

                int h = ( int ) ( data[k].serie[j] * ( height_ - 50 ) / ( max - min ) );

                if ( h == 0 )
                {
                    if ( data[k].aval > 0 )
                    {
                        h = 1;
                    }
                    else if ( data[k].aval < 0 )
                    {
                        h = -1;
                    }
                }

                if ( data[k].serie[j] >= 0 )
                {
                    wxCoord x = originLeft + ( k * static_cast<int>( serieLabel.size() ) + j ) * barwidth + ( k + 1 ) * gap;
                    wxCoord y = ( height_ - 25 ) + zero - h;
                    dc_.DrawRectangle( x, y, barwidth, h );
                }
                else
                {
                    wxCoord x = originLeft + ( k * static_cast<int>( serieLabel.size() ) + j ) * barwidth + ( k + 1 ) * gap;
                    wxCoord y = ( height_ - 25 ) + zero;
                    dc_.DrawRectangle( x, y, barwidth, -h );
                }

                // key tag
                if ( keymode == CHART_LEGEND_FIXED )
                {
                    // only once
                    if ( k == 0 )
                    {
                        dc_.DrawRectangle(
                                originLeft + csize + 15,
                                33 + j * 18,
                                10,
                                10 );
                    }
                }
            }
        }
    }

    return true;
}
//----------------------------------------------------------------------------
