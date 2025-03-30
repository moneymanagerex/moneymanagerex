/*******************************************************
Payee Match and Merge Module for Money Manager Ex
Copyright (C) 2025 joshuammex
xAI's GROK was leveraged in the creation of this module.

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

#include "payeematchandmerge.h"
#include "rapidjson/document.h"
#include <algorithm>
#include <wx/regex.h>

PayeeMatchAndMerge::PayeeMatchAndMerge()
{
}

PayeeMatchAndMerge::~PayeeMatchAndMerge()
{
}

bool PayeeMatchAndMerge::MatchPayee(const wxString& payeeName, PayeeMatchMode mode, std::vector<PayeeMatchResult>& results, int maxResults)
{
    if (payeeName.IsEmpty())
        return false;

    results.clear();

    ExactMatch(payeeName, results);

    if (results.size() == 1 && results[0].PayeeID > 0) // Exact match found
    {
        return true;
    }
    if (results.size() < 1) // No exact match
    {
        RegexMatch(payeeName, results);
        FuzzyMatch(payeeName, results);
    }

    if (results.empty())
        return false;

    SortAndTrimResults(results, (mode == PayeeMatchMode::BEST_MATCH) ? 1 : maxResults);
    return true;
}


void PayeeMatchAndMerge::ExactMatch(const wxString& payeeName, std::vector<PayeeMatchResult>& results)
{
    Model_Payee::Data_Set payees = Model_Payee::instance().all(Model_Payee::COL_PAYEENAME);
    wxLogDebug("ExactMatch: Checking payeeName='%s' against %zu payees", payeeName, payees.size());
    for (const auto& payee : payees)
    {
        wxLogDebug("ExactMatch: Comparing '%s' with '%s'", payeeName, payee.PAYEENAME);
        if (payee.PAYEENAME.IsSameAs(payeeName, false)) // Case-insensitive
        {
            PayeeMatchResult result;
            result.PayeeID = payee.PAYEEID.GetValue();
            result.Name = payee.PAYEENAME;
            result.LastUsedCategoryID = payee.CATEGID.GetValue();
            result.MatchConfidence = 100.0;
            result.matchMethod = "Exact";
            results.push_back(result);
            wxLogDebug("ExactMatch: Found exact match '%s'", payee.PAYEENAME);
            return;
        }
    }
    wxLogDebug("ExactMatch: No exact match found for '%s'", payeeName);
}




void PayeeMatchAndMerge::RegexMatch(const wxString& payeeName, std::vector<PayeeMatchResult>& results)
{
    Model_Payee::Data_Set payees = Model_Payee::instance().all();
    for (const auto& payee : payees)
    {
        if (payee.PATTERN.IsEmpty())
            continue;

        std::vector<wxString> patterns;
        LoadRegexPatterns(payee, patterns);

        for (const auto& pattern : patterns)
        {
            wxRegEx re(pattern, wxRE_ADVANCED | wxRE_ICASE);
            if (re.IsValid() && re.Matches(payeeName))
            {
                PayeeMatchResult result;
                result.PayeeID = payee.PAYEEID.GetValue();
                result.Name = payee.PAYEENAME;
                result.LastUsedCategoryID = payee.CATEGID.GetValue();
                result.MatchConfidence = 90.0;
                result.matchMethod = "Regex"; // Set match method
                results.push_back(result);
                break; // Move to next payee
            }
        }
    }
}


void PayeeMatchAndMerge::FuzzyMatch(const wxString& payeeName, std::vector<PayeeMatchResult>& results)
{
    Model_Payee::Data_Set payees = Model_Payee::instance().all(Model_Payee::COL_PAYEENAME);
    for (const auto& payee : payees)
    {
        int distance = CalculateLevenshteinDistance(payeeName, payee.PAYEENAME);
        int maxLen = std::max(payeeName.Length(), payee.PAYEENAME.Length());
        if (maxLen == 0)
            continue;

        double similarity = 1.0 - static_cast<double>(distance) / maxLen;
        double confidence = similarity * 100.0;

        // No hardcoded threshold here; let ImportTransactions handle it
        PayeeMatchResult result;
        result.PayeeID = payee.PAYEEID.GetValue();
        result.Name = payee.PAYEENAME;
        result.LastUsedCategoryID = payee.CATEGID.GetValue();
        result.MatchConfidence = confidence;
        result.matchMethod = "Fuzzy";
        results.push_back(result);
    }
}


int PayeeMatchAndMerge::CalculateLevenshteinDistance(const wxString& s1, const wxString& s2)
{
    const size_t len1 = s1.length(), len2 = s2.length();
    std::vector<std::vector<int>> d(len1 + 1, std::vector<int>(len2 + 1));

    d[0][0] = 0;
    for (size_t i = 1; i <= len1; ++i)
        d[i][0] = i;
    for (size_t j = 1; j <= len2; ++j)
        d[0][j] = j;

    for (size_t i = 1; i <= len1; ++i)
    {
        for (size_t j = 1; j <= len2; ++j)
        {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            d[i][j] = std::min({ d[i - 1][j] + 1,           // Deletion
                                 d[i][j - 1] + 1,           // Insertion
                                 d[i - 1][j - 1] + cost }); // Substitution
        }
    }
    return d[len1][len2];
}

void PayeeMatchAndMerge::LoadRegexPatterns(const Model_Payee::Data& payee, std::vector<wxString>& patterns)
{
    rapidjson::Document j_doc;
    j_doc.Parse(payee.PATTERN.mb_str());
    if (!j_doc.HasParseError() && j_doc.IsObject())
    {
        for (rapidjson::Value::ConstMemberIterator itr = j_doc.MemberBegin(); itr != j_doc.MemberEnd(); ++itr)
        {
            if (itr->value.IsString())
            {
                wxString pattern = wxString::FromUTF8(itr->value.GetString());
                if (!pattern.IsEmpty())
                {
                    if (pattern.Contains("*"))
                    {
                        pattern.Replace("*", ".*", true);
                        pattern.Replace("..*", ".*", true);
                    }
                    patterns.push_back(pattern);
                }
            }
        }
    }
}

void PayeeMatchAndMerge::SortAndTrimResults(std::vector<PayeeMatchResult>& results, int maxResults)
{
    // Sort by confidence (descending)
    std::sort(results.begin(), results.end(), [](const PayeeMatchResult& a, const PayeeMatchResult& b) { return a.MatchConfidence > b.MatchConfidence; });

    // Remove duplicates by PayeeID, keeping the highest confidence
    std::vector<PayeeMatchResult> uniqueResults;
    std::set<long long> seenIds;
    for (const auto& result : results)
    {
        if (seenIds.find(result.PayeeID) == seenIds.end())
        {
            uniqueResults.push_back(result);
            seenIds.insert(result.PayeeID);
        }
    }
    results = std::move(uniqueResults);

    // Trim to maxResults
    if (static_cast<int>(results.size()) > maxResults)
        results.resize(maxResults);
}