/*******************************************************
Copyright (C) 2014 Stefano Giorgio

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
Foundation, Inc., 59 Temple Placeuite 330, Boston, MA  02111-1307  USA
 ********************************************************/

/****************************************************************************
This frame serves as a test base for dialogs.
*****************************************************************************/ 
class wxFrame;

class TestFrameBase : public wxFrame
{
public:
    /** This is the base frame */
    TestFrameBase(int frame_count);
    /**
    Create subsequent frames using parent TestFrameBase
    * Set the required window size. Base size: (400, 250)
    */
    TestFrameBase(wxWindow* parent, int size_x, int size_y);
    ~TestFrameBase();

private:
    wxDECLARE_EVENT_TABLE();
};
