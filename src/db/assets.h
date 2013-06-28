/*******************************************************
 Copyright (C) 2013 Stefano Giorgio

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

#pragma once
#include "entry_base.h"

const wxString ASSET_RATE_DEF[] =
{
    wxTRANSLATE("None"),
    wxTRANSLATE("Appreciates"),
    wxTRANSLATE("Depreciates")
};

const wxString ASSET_TYPE_DEF[] =
{
    wxTRANSLATE("Property"),
    wxTRANSLATE("Automobile"),
    wxTRANSLATE("Household Object"),
    wxTRANSLATE("Art"),
    wxTRANSLATE("Jewellery"),
    wxTRANSLATE("Cash"),
    wxTRANSLATE("Other")
};

/************************************************************************************
 Class TAssetEntry
 ***********************************************************************************/
class TAssetEntry : public TEntryBase
{
private:
    friend class TAssetList;    // Allows TAssetList access to private members

    int Add(wxSQLite3Database* db);
    void Delete(wxSQLite3Database* db);
    void SetDatabaseValues(wxSQLite3Statement& st, int& db_index);

    double PeriodValueAtRate(const wxDateTime startDate, double value, double rate);
    double GetAppreciatedValue(const wxDateTime& startDate, double value, double rate);
    double GetDepreciatedValue(const wxDateTime& startDate, double value, double rate);

public:
    bool operator <(const TAssetEntry& asset) const ;
    int to_json(json::Object& o) const;
public:
    enum ASSET_RATE
    {
        RATE_NONE,
        RATE_APPRECIATE,
        RATE_DEPRECIATE
    };
    enum ASSET_TYPE
    {
        TYPE_PROPERTY,
        TYPE_AUTO,
        TYPE_HOUSE,
        TYPE_ART,
        TYPE_JEWELLERY,
        TYPE_CASH,
        TYPE_OTHER
    };

    wxString name_;         // "ASSETNAME"
    wxString type_;         // "ASSETTYPE"
    wxDateTime date_;       // "STARTDATE"
    wxString notes_;        // "NOTES"
    double value_;          // "VALUE"
    wxString rate_type_;    // "VALUECHANGE"
    double rate_value_;     // "VALUECHANGERATE"

    /// Constructor used when loading assets from the database.
    TAssetEntry(wxSQLite3ResultSet& q1);
    /// Copy constructor using a pointer
    TAssetEntry(TAssetEntry* pEntry);
    /// Constructor for creating a new asset entry.
    TAssetEntry();

    void Update(wxSQLite3Database* db);
    double GetValue();
    wxString GetValueCurrencyEditFormat(bool initial_value = false);
    wxString DisplayDate();
    void destroy()
    {
        delete this;
    }
};

/************************************************************************************
 Class TAssetList
 ***********************************************************************************/
class TAssetList : public TListBase
{
private:
    void LoadEntries(bool load_entries = true);
    // delete all the objects in the list and clear the list.
    void DestroyEntryList();

public:
    std::vector<TAssetEntry*> entrylist_;

    TAssetList(wxSQLite3Database* db, bool load_entries = true);
    ~TAssetList();

    /// Allows specialised loads by providing the required SQL statement
    void LoadEntriesUsing(const wxString& sql_statement);

    int AddEntry(TAssetEntry* pAssetEntry);
    void DeleteEntry(int asset_id);

    TAssetEntry* GetEntryPtr(int asset_id);
    TAssetEntry* GetIndexedEntryPtr(unsigned int list_index);

    int CurrentListSize();
    double GetAssetBalance(bool value_today = true);
    wxString GetAssetBalanceCurrencyFormat(bool value_today = true);
    wxString GetAssetBalanceCurrencyEditFormat(bool value_today = true);
};
