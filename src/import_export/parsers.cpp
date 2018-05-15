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
#include <wx/textfile.h>
#include <wx/tokenzr.h>

// ---------------------------- CSV Parser --------------------------------
FileCSV::FileCSV(wxWindow *pParentWindow, wxConvAuto encoding, wxString delimiter):
    TableBasedFile(pParentWindow), encoding_(encoding), delimiter_(delimiter)
{
}

bool FileCSV::Load(const wxString& fileName, unsigned int itemsInLine)
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
        itemsTable_.push_back(std::vector<ValueAndType>());

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

bool FileCSV::Save(const wxString& fileName)
{
    // Make sure file exists
    if (fileName.IsEmpty())
    {
        mmErrorDialogs::InvalidFile(pParentWindow_);
        return false;
    }

    // Open file
    wxTextFile txtFile(fileName);
    if (!txtFile.Create())
    {
        mmErrorDialogs::MessageError(pParentWindow_, _("Unable to create file."), _("Universal CSV Import"));
        return false;
    }

    // Store lines
    for (auto rowIt : itemsTable_)
    {
        wxString line;
        for (auto itemIt : rowIt)
        {
            if (!line.IsEmpty())
                line += delimiter_;
            line += inQuotes(itemIt.value, delimiter_);
        }
        txtFile.AddLine(line);
    }

    // Save the file.
    if (!txtFile.Write(wxTextFileType_None, encoding_))
    {
        mmErrorDialogs::MessageError(pParentWindow_, _("Could not save file."), _("Export error"));
        return false;
    }
    txtFile.Close();
    return true;
}
// ---------------------------- XML Parser --------------------------------
FileXML::FileXML(wxWindow *pParentWindow, wxString encoding):
    TableBasedFile(pParentWindow), encoding_(encoding)
{
}

bool FileXML::Load(const wxString& fileName, unsigned int itemsInLine)
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
        mmErrorDialogs::MessageError(pParentWindow_
            , _("File is not in Excel XML Spreadsheet 2003 format."), _("Parsing error"));
        return false;
    }

    // Workbook
    wxXmlNode *workbookElement = xmlFile.GetRoot();
    if (workbookElement->GetName().Cmp("Workbook") != 0
        || workbookElement->GetAttribute("xmlns").Cmp("urn:schemas-microsoft-com:office:spreadsheet") != 0)
    {
        mmErrorDialogs::MessageError(pParentWindow_, _("File is not in Excel XML Spreadsheet 2003 format."), _("Parsing error"));
        return false;
    }

    // Worksheet
    // TODO: Allow the user to choose the worksheet. This just uses the first.
    wxXmlNode *worksheetElement = workbookElement->GetChildren();
    for (; worksheetElement && worksheetElement->GetName() != "Worksheet"; worksheetElement = worksheetElement->GetNext())
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
    for (wxXmlNode *rowElement = tableElement->GetChildren(); rowElement; rowElement = rowElement->GetNext())
    {
        if (rowElement->GetName() != "Row")
            continue;
        AddNewLine();

        // Cells in row
        for (wxXmlNode *cellElement = rowElement->GetChildren(); cellElement; cellElement = cellElement->GetNext())
        {
            if (cellElement->GetName() != "Cell")
                continue;

            if (itemsTable_.back().size() >= itemsInLine)
                break;

            wxXmlNode *dataElement = cellElement->GetChildren();
            wxString content = dataElement? dataElement->GetNodeContent(): "";
            AddNewItem(content);
        }
    }
    return true;
}

bool FileXML::Save(const wxString& fileName)
{
    // Make sure file exists
    if (fileName.IsEmpty())
    {
        mmErrorDialogs::InvalidFile(pParentWindow_);
        return false;
    }

    // Open file
    wxXmlDocument xmlFile;
 
    // Workbook
    wxXmlNode* workbookElement = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, "Workbook");
    xmlFile.SetRoot(workbookElement);
    workbookElement->AddAttribute("xmlns", "urn:schemas-microsoft-com:office:spreadsheet");
    workbookElement->AddAttribute("xmlns:o", "urn:schemas-microsoft-com:office:office");
    workbookElement->AddAttribute("xmlns:x", "urn:schemas-microsoft-com:office:excel");
    workbookElement->AddAttribute("xmlns:ss", "urn:schemas-microsoft-com:office:spreadsheet");
    workbookElement->AddAttribute("xmlns:html", "http://www.w3.org/TR/REC-html40");

    // Worksheet
    wxXmlNode* worksheetElement = new wxXmlNode(workbookElement, wxXML_ELEMENT_NODE, "Worksheet");
    worksheetElement->AddAttribute("ss:Name", _("Transactions")); //TODO: account name may be used here
    // workbookElement->AddAttribute("ss:RightToLeft", "1");  

     // Table
    wxXmlNode* tableElement = new wxXmlNode(worksheetElement, wxXML_ELEMENT_NODE, "Table");

    // Rows - reverse iterate because wxXmlNode() adds the new child as the first.
    for (auto rowIt = itemsTable_.rbegin(); rowIt != itemsTable_.rend(); rowIt++)
    {
        wxXmlNode* rowElement = new wxXmlNode(tableElement, wxXML_ELEMENT_NODE, "Row");

        // Items - reverse iterate because wxXmlNode() adds the new child as the first.
        for (auto itemIt = rowIt->rbegin(); itemIt != rowIt->rend(); itemIt++)
        {
            wxXmlNode* cellElement = new wxXmlNode(rowElement, wxXML_ELEMENT_NODE, "Cell");
            wxXmlNode* dataElement = new wxXmlNode(cellElement, wxXML_ELEMENT_NODE, "Data");
            switch (itemIt->type)
            {
            case TYPE_NUMBER:
                dataElement->AddAttribute("ss:Type", "Number");
                break;
            default:
                dataElement->AddAttribute("ss:Type", "String");
            }

            dataElement->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", itemIt->value));
        }
    }

    // Save the file.
    if (!xmlFile.Save(fileName))
    {
        mmErrorDialogs::MessageError(pParentWindow_, _("Could not save file."), _("Export error"));
        return false;
    }

    return true;
}
