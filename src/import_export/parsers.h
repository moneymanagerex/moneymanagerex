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

#ifndef MM_PARSERS_H_
#define MM_PARSERS_H_

#include <wx/string.h>
#include <wx/window.h>
#include <wx/convauto.h>
#include <vector>

// Generic interface for importing data from a file.
// Get functions should be called after Parse() was called.
class ITransactionsFile
{
public:
    enum ItemType {/*TYPE_UNKNOWN,*/ TYPE_STRING, TYPE_NUMBER};

    virtual ~ITransactionsFile() {}

 // *********************** Import related methods ***********************
    // Opens and parses the input file in to an internal structure that allows calling the getter functions below.
    virtual bool Load(const wxString& fileName, unsigned int itemsInLine) = 0;

    // Gets the number of lines that can be parsed.
    // Depending on type of file there may be lines that are not transactions.
    virtual unsigned int GetLinesCount() const = 0;

    // Gets the number of items in the specified line.
    virtual unsigned int GetItemsCount(unsigned int line) const = 0;

    // Gets the item or wxEmptyString if there is none.
    virtual wxString GetItem(unsigned int line, unsigned int itemInLine) const = 0;

// *********************** Export related methods ***********************
    // Adds a new empty line to the output file. Use NewItem() to add items to this line.
    virtual void AddNewLine() = 0;

    // Adds a new item to the last line. NewLine() must be called first.
    virtual void AddNewItem(const wxString &stringItem) = 0;

    virtual void AddNewItem(const wxString &stringItem, ItemType itemType) = 0;

    // Exports all item to file.
    virtual bool Save(const wxString& fileName) = 0;
};

// A base class for a parser that reads the file in to a string table in memory.
class TableBasedFile : public ITransactionsFile
{
public:
    TableBasedFile(wxWindow *pParentWindow) : pParentWindow_(pParentWindow) {}
    virtual ~TableBasedFile()
    {
        for (auto line : itemsTable_)
            line.clear();
        itemsTable_.clear();
    }
    virtual unsigned int GetLinesCount() const
    {
        return itemsTable_.size();
    }
    virtual unsigned int GetItemsCount(unsigned int line) const
    {
        if (line >= GetLinesCount())
            return 0;
        return itemsTable_[line].size();
    }
    virtual wxString GetItem(unsigned int line, unsigned int itemInLine) const
    {
        if (line >= GetLinesCount() || itemInLine >= itemsTable_[line].size())
            return wxEmptyString;
        return itemsTable_[line][itemInLine].value;
    }
    virtual void AddNewLine()
    {
        itemsTable_.push_back(std::vector<ValueAndType>());
    }

    virtual void AddNewItem(const wxString &stringItem)
    {
        itemsTable_.back().push_back(stringItem);
    }

    virtual void AddNewItem(const wxString &stringItem, ItemType itemType)
    {
        itemsTable_.back().push_back({ stringItem, itemType });
    }

protected:
    wxWindow *pParentWindow_;
    struct ValueAndType
    {
        ValueAndType() : value(wxEmptyString), type(TYPE_STRING) {};
        ValueAndType(const wxString &setValue) : value(setValue), type(TYPE_STRING) {};
        ValueAndType(const wxString &setValue, ItemType setItemType) : value(setValue), type(setItemType) {};
        wxString value;
        ItemType type;
    };
    typedef std::vector<ValueAndType> RowItemsT;
    std::vector<RowItemsT> itemsTable_;
};

// CSV parser
class FileCSV : public TableBasedFile
{
public:
    FileCSV(wxWindow *pParentWindow, wxConvAuto encoding, wxString delimiter);
    virtual bool Load(const wxString& fileName, unsigned int itemsInLine);
    virtual bool Save(const wxString& fileName);
protected:
    wxConvAuto encoding_;
    wxString delimiter_;
};

// XML parser
class FileXML : public TableBasedFile
{
public:
    FileXML(wxWindow *pParentWindow, wxString encoding);
    virtual bool Load(const wxString& fileName, unsigned int itemsInLine);
    virtual bool Save(const wxString& fileName);
protected:
    wxString encoding_;
};

#endif // MM_PARSERS_H_
