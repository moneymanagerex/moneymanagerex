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
<h3><TMPL_VAR HEADER></h3>
<div class = "col-xs-8">
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
};

mmBugReport::mmBugReport()
    : mmPrintableBase(_("Bug Report"))
{
}

mmBugReport::~mmBugReport()
{
}

wxString mmBugReport::getHTMLText()
{
    wxString diag = getProgramDescription(1);

    wxString info = "> " + _("Replace this text (marked with >) with detailed description of your problem.") + "\n";
    info << "> " + _("Please do not remove information attached below this text.") + "\n";

    wxURI req = mmex::weblink::BugReport + "/new?body=" + info + "\n<hr>" + diag;

    const wxString texts[] = {
        _("Use Help->Check for Updates in MMEX to get latest version, where your problem might be already fixed."),
        wxString::Format(_("Search <a href='%s'>a list of known issues</a> for similar problem. If so, update existing issue instead of creating a new one.")
            ,  mmex::weblink::BugReport),
        wxString::Format(_("As you know, <a href='%s'>a forum</a> for users is available where you can discuss problems and find solutions."), mmex::weblink::Forum),
        wxString::Format(_("Please, join our community in <a href='%s'>Slack</a>. Here you can also pose your question and learn latest news."), mmex::weblink::Slack),
        wxString::Format(_("Read <a href='%s'>How to Report Bugs Effectively</a> for useful tips."), mmex::weblink::Chiark),
        _("Come up with a descriptive name for your problem."),
        _("Include steps to reproduce your problem, attach screenshots where appropriate."),
        wxString::Format(_("Before click the following link, be sure that you have already signed in to the <a href='%s'>GitHub</a>.")
            ,  mmex::weblink::GitHubLogin),
        wxString::Format(_("Finally, report a bug using GitHub <a href='%s'>online form</a> opened in your web browser."), req.BuildURI())
    };

    wxString msg = "<ol>";
    for (const auto& string : texts) {
        msg += "<li>" + string + "</li>" + "\n";
    }
    msg += "</ol>\n";

    mm_html_template report(bugreport_template);
    report(L"REPORTNAME") = this->getReportTitle();
    report(L"HEADER") = _("Please, follow these instructions before submitting a new bug report:");
    report(L"CONTENTS") = msg;
    report(L"HTMLSCALE") = wxString::Format("%d", Option::instance().getHtmlFontSize() * 3 / 2);

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
        return _("Caught exception");
    }

    return out;
}
