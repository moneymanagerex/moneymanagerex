[<img src="https://raw.githubusercontent.com/moneymanagerex/moneymanagerex/master/resources/mmexlogo.png" height="32"/>][website]
Money Manager Ex
================

[![stable]][GitHubLatest] [![unstable]][GitHubDL] [![a]][AppVeyor]
[![t]][Travis] [![s]][SourceForgeDL] [![gha]][GitHubDL] [![ghl]][GitHubLatest]
[![p]][packagecloud] [![w]][website] [![i]][GitHubIssues] [![cd]][CodeDocs]
[![c]][contributors] [![l]](LICENSE) [![o]][OpenHub]

Money Manager Ex is a free, open-source, cross-platform, easy-to-use personal
finance software. It primarily helps organize one's finances and keeps track
of where, when and how the money goes.

MMEX includes all the basic features that 90% of users would want to see in a
personal finance application. The design goals are to concentrate on
simplicity and user-friendliness - something one can use everyday.

MMEX is mainly written in C++11 and built on:  
  [wxWidgets] 3.0+
| [wxSqlite3]
| [SQLite3]
| [Mongoose]
| [ChartNew.JS]
| [html-template]

#### Supported platforms ####
- Windows XP, Vista, 7, 8 and 10
- Most Unix variants using the GTK+ toolkit (version 2.6 or newer or 3.x)
- OS X (10.7 or newer) and macOS using Cocoa

#### External public resources ####
  [Slack]
| [Forum]
| [Wiki]
| [Facebook]
| [YouTube]
| [SourceForge]
| [GitHub]
| [Docker images]
| [Twitter] [![tw]][Twitter]

#### Tips ####
* You can open database file (both regular and encrypted) directly
  with [wxsqliteplus]
* You can download dozen of cool reports from [general report] repository
* Star it if you feel it is helpful [![st]][stargazers]

<!-- links -->
[website]:
  http://moneymanagerex.org
  "MMEX website"
[AppVeyor]:
  https://ci.appveyor.com/project/guanlisheng/moneymanagerex
  "AppVeyor CI"
[Travis]:
  http://travis-ci.org/moneymanagerex/moneymanagerex
  "Travis CI"
[OpenHub]:
  https://www.openhub.net/p/moneymanagerex
  "Open Hub report"
[GitHubIssues]:
  https://github.com/moneymanagerex/moneymanagerex/issues
  "open issues on GitHub"
[SourceForgeDL]:
  https://sourceforge.net/projects/moneymanagerex/files/latest
  "SourceForge downloads"
[GitHubDL]:
  https://github.com/moneymanagerex/moneymanagerex/releases
  "GitHub downloads"
[contributors]:
  https://github.com/moneymanagerex/moneymanagerex/graphs/contributors
  "contributors to Git repo"
[GitHubLatest]:
  https://github.com/moneymanagerex/moneymanagerex/releases/latest
  "GitHub latest stable downloads"
[packagecloud]:
  https://packagecloud.io/moneymanagerex/
  "packagecloud DEB & RPM repository"
[CodeDocs]:
  https://codedocs.xyz/slodki/moneymanagerex/
  "doxygen generated source code documentation"
[Slack]: http://slack.moneymanagerex.org/
[Forum]: http://forum.moneymanagerex.org
[Wiki]: https://sourceforge.net/p/moneymanagerex/wiki/mmex
[Facebook]: https://www.facebook.com/MoneyManagerEx
[YouTube]: https://www.youtube.com/channel/UCAqVC0fOt6C5OnGv_DzE0wg
[SourceForge]: https://sourceforge.net/p/moneymanagerex
[GitHub]: https://github.com/moneymanagerex
[Docker images]: https://hub.docker.com/r/moneymanagerex/moneymanagerex/
[Twitter]: https://twitter.com/MoneyManagerEx
[wxWidgets]: http://wxwidgets.org/
[wxSqlite3]: https://github.com/utelle/wxsqlite3
[SQLite3]: http://sqlite.org/
[Mongoose]: https://www.cesanta.com/
[ChartNew.JS]: https://github.com/FVANCOP/ChartNew.js
[html-template]: https://github.com/moneymanagerex/html-template
[wxsqliteplus]: https://github.com/guanlisheng/wxsqliteplus
[general report]: https://github.com/moneymanagerex/general-reports
[stargazers]: https://github.com/moneymanagerex/moneymanagerex/stargazers
<!-- icons -->
[a]: https://img.shields.io/appveyor/ci/moneymanagerex/moneymanagerex/master.svg?label=windows&logoWidth=0.01
[t]: https://img.shields.io/travis/moneymanagerex/moneymanagerex/master.svg?label=linux/mac
[o]: http://www.openhub.net/p/moneymanagerex/widgets/project_thin_badge.gif
[i]: https://img.shields.io/github/issues-raw/moneymanagerex/moneymanagerex.svg?label=gh%20issues
[c]: https://img.shields.io/github/contributors/moneymanagerex/moneymanagerex.svg
[cd]: https://codedocs.xyz/slodki/moneymanagerex.svg
[l]: https://img.shields.io/badge/license-GPL2-blue.svg
[s]: https://img.shields.io/sourceforge/dt/moneymanagerex.svg?label=sf%20🡇
[gha]: https://img.shields.io/github/downloads/moneymanagerex/moneymanagerex/total.svg?label=gh%20🡇
[ghl]: https://img.shields.io/github/downloads/moneymanagerex/moneymanagerex/latest/total.svg?label=gh%20latest%20🡇
[w]: https://img.shields.io/website-up-down-brightgreen-red/http/www.moneymanagerex.org/.svg
[p]: https://img.shields.io/website-up-down-brightgreen-red/https/packagecloud.io/moneymanagerex/.svg?label=deb%20rpm%20repo
[stable]: https://img.shields.io/github/release/moneymanagerex/moneymanagerex.svg?label=stable
[unstable]: https://img.shields.io/github/release/moneymanagerex/moneymanagerex/all.svg?label=unstable
[st]: https://img.shields.io/github/stars/moneymanagerex/moneymanagerex.svg?&label=GitHub&style=social&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABQAAAAUCAYAAACNiR0NAAAABmJLR0QA%2FwD%2FAP%2BgvaeTAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAB3RJTUUH4QUEAzUBgT422QAAASNJREFUOMvN1LFKA0EQxvFfzmaFFBIQUthEAkoqRVtBhBWCnWhvZSE%2BjC%2Fgcxg4LARBEISAIIiCWJnGQgh4jWBzgRCDl4sp%2FLodZv4zy36zFChL7WWpiyx1YAJVCmDzeMUi%2BlgK0cdvNUlBw6McBlWcTj1hlprDExpD4R6WQ%2FQ5zYQHIzCo51NPPmGWaqCJM7TG1LzgBM8hev4BzFI7OM4hG8rrLm9yHqJOJUu9o%2Bbv6mMhwY3Z6DZEXwkOZwDtYh%2BS3AJt3E4Ju8fuwPCVodet4RJrJWCP2A5Rb6xtslQdbyWAqyF6%2FM3Y1ZLXrRZtSqsksPVvgNfTAldGzlfYDNEW1tEpCxx48QH7IdoO0R2EqBuidu7ZhzyveCHy32aSvOa4%2BDfK70RSyb%2Fw%2BgAAAABJRU5ErkJggg%3D%3D
[tw]: https://img.shields.io/twitter/follow/MoneyManagerEx.svg?style=social&label=follow
