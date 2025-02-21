/*******************************************************
 Copyright (C) 2019 - 2020 Nikolay Akimov

 This program is free software; you can redistribute transcation and/or modify
 transcation under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that transcation will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#pragma once
#include "reportbase.h"

const char* bugreport_template = R"(
<!DOCTYPE html>
<html>
<head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8" />
    <title><TMPL_VAR REPORTNAME></title>
    <link href='memory:master.css' rel='stylesheet' />
    <style>
        canvas {min-height: 100px}
        body {font-size: <TMPL_VAR HTMLSCALE>%}
    </style>
</head>
<body>
<div class = "container">
<h2><TMPL_VAR HEADER></h2>
<BR>
<TMPL_VAR CONTENTS>
</div>
</body>
</html>
)";

class mmBugReport: public mmPrintableBase
{
public:
    mmBugReport();
    virtual ~mmBugReport();

    virtual wxString getHTMLText();
private:
    const wxString do_href_wrap(const wxString& www) const;
};

mmBugReport::mmBugReport()
    : mmPrintableBase(_n("Bug Report"))
{
}

mmBugReport::~mmBugReport()
{
}

wxString mmBugReport::getHTMLText()
{
    wxString diag = getProgramDescription(1);

    wxString info = "> " + _t("Replace this text (marked with >) with detailed description of the problem.") + "\n";
    info << "> " + _t("Please do not remove information attached below this text.") + "\n";

    wxURI req = mmex::weblink::BugReport + "/new?body=" + info + "\n<hr>" + diag;

    const wxString texts[] = {
        _tu("Use Help → Check for Updates in MMEX to get the latest version, where the problem might be already fixed."),
        wxString::Format(_t("Search <a href='%s'>a list of known issues</a> for similar problem. If so, update existing issue instead of creating a new one.")
            ,  do_href_wrap(mmex::weblink::BugReport)),
        wxString::Format(_t("As you know, <a href='%s'>a forum</a> for users is available where you can discuss problems and find solutions.") , do_href_wrap(mmex::weblink::Forum)),
        wxString::Format(_t("Please, join the community on <a href='%s'>Slack</a>. Here you can also pose the question and learn the latest news."), do_href_wrap(mmex::weblink::Slack)),
        wxString::Format(_t("Read <a href='%s'>How to Report Bugs Effectively</a> for useful tips."), do_href_wrap(mmex::weblink::Chiark)),
        _t("Provide a descriptive name for the problem."),
        _t("Include steps to reproduce the problem; attach screenshots where appropriate."),
        wxString::Format(_t("Before click the following link, be sure that you have already signed in to the <a href='%s'>GitHub</a>.")
            ,  do_href_wrap(mmex::weblink::GitHubLogin)),
        wxString::Format(_t("Finally, report the bug using the GitHub <a href='%s'>online form</a> opened in a web browser."), do_href_wrap(req.BuildURI()))
    };

    wxString msg = "<ol>";
    for (const auto& string : texts) {
        msg += "<li>" + string + "</li>" + "\n";
    }
    msg += "</ol>\n";

    mm_html_template report(bugreport_template);
    report(L"REPORTNAME") = this->getReportTitle();
    report(L"HEADER") = _t("Please, follow these instructions before submitting a new bug report:");
    report(L"CONTENTS") = msg;
    report(L"HTMLSCALE") = wxString::Format("%d", Option::instance().getHtmlScale() * 3 / 2);

    wxString out = wxEmptyString;
    try
    {
        out = report.Process();
    }
    catch (const syntax_ex & e)
    {
        return e.what();
    }
    catch (...)
    {
        return _t("Caught exception");
    }

    return out;
}

inline const wxString mmBugReport::do_href_wrap(const wxString & www) const
{
    return wxString::Format("%s' target='_blank>", www);
}
