/*******************************************************
Copyright (C) 2015 Yosef

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

#include "parsers.h"
#include "mmSimpleDialogs.h"
#include "util.h"
#include <wx/xml/xml.h>
#include <wx/filename.h>

// ---------------------------- CSV Parser --------------------------------
ImportParserCSV::ImportParserCSV(wxWindow *pParentWindow, wxConvAuto encoding, wxString delimiter):
    TableBasedParser(pParentWindow), encoding_(encoding), delimiter_(delimiter)
{
}

bool ImportParserCSV::Parse(const wxString& fileName, unsigned int itemsInLine)
{
    // Make sure file exists
    if (fileName.IsEmpty() || !wxFileName::FileExists(fileName))
    {
        mmErrorDialogs::InvalidFile(pParentWindow_);
        return false;
    }

    // Open file
    wxTextFile txtFile(fileName);
    if (!txtFile.Open(encoding_))
    {
        mmErrorDialogs::MessageError(pParentWindow_, _("Unable to open file."), _("Universal CSV Import"));
        return false;
    }

    // Parse rows
    wxString line;
    int row = 0;
    for (line = txtFile.GetFirstLine(); !txtFile.Eof(); line = txtFile.GetNextLine())
    {
        csv2tab_separated_values(line, delimiter_);
        wxStringTokenizer tkz(line, "\t", wxTOKEN_RET_EMPTY_ALL);
        itemsTable_.push_back(std::vector<wxString>());

        // Tokens in row
        while (tkz.HasMoreTokens())
        {
            if (itemsTable_[row].size() >= itemsInLine)
                break;

            wxString token = tkz.GetNextToken();
            itemsTable_[row].push_back(token);
        }

        ++row;
    }

    txtFile.Close();
    return true;
}

// ---------------------------- XML Parser --------------------------------
ImportParserXML::ImportParserXML(wxWindow *pParentWindow, wxString encoding):
    TableBasedParser(pParentWindow), encoding_(encoding)
{
}

bool ImportParserXML::Parse(const wxString& fileName, unsigned int itemsInLine)
{
    // Make sure file exists
    if (fileName.IsEmpty() || !wxFileName::FileExists(fileName))
    {
        mmErrorDialogs::InvalidFile(pParentWindow_);
        return false;
    }

    // Open file
    wxXmlDocument xmlFile;
    if (!xmlFile.Load(fileName, encoding_))
    {
        mmErrorDialogs::MessageError(pParentWindow_, _("File is not in Excel XML Spreadsheet 2003 format."), _("Parsing error"));
        return false;
    }

    // Workbook
    wxXmlNode *workbookElement = xmlFile.GetRoot();
    if (workbookElement->GetName() != _("Workbook") || workbookElement->GetAttribute("xmlns") != _("urn:schemas-microsoft-com:office:spreadsheet"))
    {
        mmErrorDialogs::MessageError(pParentWindow_, _("File is not in Excel XML Spreadsheet 2003 format."), _("Parsing error"));
        return false;
    }

    // Worksheet
    // TODO: Allow the user to choose the worksheet. This just uses the first.
    wxXmlNode *worksheetElement = workbookElement->GetChildren();
    for (; worksheetElement && worksheetElement->GetName() != _("Worksheet"); worksheetElement = worksheetElement->GetNext())
    {
    };

    if (!worksheetElement)
    {
        mmErrorDialogs::MessageError(pParentWindow_, _("Could not find Worksheet."), _("Parsing error"));
        return false;
    }

    // Table
    wxXmlNode *tableElement = worksheetElement->GetChildren();
    if (tableElement->GetName() != _("Table"))
    {
        mmErrorDialogs::MessageError(pParentWindow_, _("Could not find Table."), _("Parsing error"));
        return false;
    }
    
    // Rows
    int row = 0;
    for (wxXmlNode *rowElement = tableElement->GetChildren(); rowElement; rowElement = rowElement->GetNext())
    {
        if (rowElement->GetName() != _("Row"))
            continue;

        itemsTable_.push_back(std::vector<wxString>());

        // Cells in row
        for (wxXmlNode *cellElement = rowElement->GetChildren(); cellElement; cellElement = cellElement->GetNext())
        {
            if (cellElement->GetName() != _("Cell"))
                continue;

            if (itemsTable_[row].size() >= itemsInLine)
                break;

            wxXmlNode *dataElement = cellElement->GetChildren();
            wxString content = dataElement->GetNodeContent();
            itemsTable_[row].push_back(content);
        }
        row++;
    }
    return true;
}
