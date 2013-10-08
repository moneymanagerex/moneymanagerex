/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2009 VaDiM

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

#include "mmgraphgenerator.h"
#include "util.h"
#include "singleton.h"
//----------------------------------------------------------------------------

class Eraser
{
public:
    Eraser() : files(2 * mmGraphGenerator::TYPE_MAX) {}
    ~Eraser();

    wxString getFilePath(mmGraphGenerator::EType type);
    static Eraser& instance();

private:
    std::vector<wxString> files;
    void clear();
};
//----------------------------------------------------------------------------

Eraser::~Eraser()
{
    try {
        clear();
    } catch (...) {
        wxASSERT(false);
    }
}
//----------------------------------------------------------------------------

void Eraser::clear()
{
//    for_each(files.begin(), files.end(), wxRemoveFile);
    files.clear();
}
//----------------------------------------------------------------------------

/*
    FIXME: do not create temp files in public temporary directory.
*/
wxString Eraser::getFilePath(mmGraphGenerator::EType type)
{
    wxASSERT(type < mmGraphGenerator::TYPE_MAX);

    wxString &path = files[type];

    if (path.empty()) 
        path = wxFileName::CreateTempFileName(wxGetEmptyString());
    else
    {
        path = files[2 * type];
        if (path.empty())
            path = wxFileName::CreateTempFileName(wxGetEmptyString());
    }

    return path;
}
//----------------------------------------------------------------------------

Eraser& Eraser::instance()
{
    return Singleton<Eraser>::instance();
}
//----------------------------------------------------------------------------

mmGraphGenerator::mmGraphGenerator(EType type) :
    m_path(Eraser::instance().getFilePath(type))
{
}
//----------------------------------------------------------------------------
