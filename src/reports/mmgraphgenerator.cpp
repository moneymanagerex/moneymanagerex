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
#include <wx/fs_mem.h>
#include <vector>
//----------------------------------------------------------------------------

class Eraser
{
public:
    Eraser();
    ~Eraser();
    void cleanup();

    wxString getFilePath();
    static Eraser& instance();

private:
    unsigned long count;
    std::vector<wxString> files;
};
//----------------------------------------------------------------------------

Eraser::Eraser()
    : count(0)
{
    wxFileSystem::AddHandler(new wxMemoryFSHandler);
}

Eraser::~Eraser()
{
}

void Eraser::cleanup()
{
    for(auto& f: files)
        wxMemoryFSHandler::RemoveFile(f);
    files.clear();
}

//----------------------------------------------------------------------------

wxString Eraser::getFilePath()
{
    wxString path = wxString::Format("chart%ld.png", ++count);
    files.push_back(path);
    return path.Prepend("memory:");
}
//----------------------------------------------------------------------------

Eraser& Eraser::instance()
{
    return Singleton<Eraser>::instance();
}
//----------------------------------------------------------------------------

mmGraphGenerator::mmGraphGenerator() :
    m_path(Eraser::instance().getFilePath())
{
}

void mmGraphGenerator::cleanup()
{
    Eraser::instance().cleanup();
}
//----------------------------------------------------------------------------
