[<img src="https://raw.githubusercontent.com/moneymanagerex/moneymanagerex/master/resources/mmexlogo.png" height="32"/>][website]
# Money Manager Ex (MMEX)

[![stable]][GitHubLatest] [![unstable]][GitHubDL] [![a]][AppVeyor]
[![ci]][CircleCI] [![s]][SourceForgeDL] [![gha]][GitHubDL] [![ghl]][GitHubLatest]
[![p]][packagecloud] [![w]][website] [![i]][GitHubIssues] [![cd]][CodeDocs]
[![c]][contributors] [![l]](LICENSE) [![o]][OpenHub]

[Money Manager Ex (MMEX)](https://moneymanagerex.org/) is a free/libre, open-source, cross-platform, easy-to-use personal
finance software. It helps organize finances and track cash flow.

MMEX includes all the basic features that 90% of users would want to see in a
personal finance application. The design goals are to concentrate on
simplicity and user-friendliness – an application that can be used everyday.


## Features

- Fast, intuitive and functional
- Checking, credit card, savings, stock investment and asset accounts
- Unlimited nested multi-level category support
- Multiple tags per split transaction support
- Reminders for scheduled bills and deposits
- Budgeting and cash flow forecasting
- Simple one-click reporting with graphs and charts
- Import data from [Comma-Separated Values (CSV)](https://en.wikipedia.org/wiki/Comma-separated_values) and [Quicken Interchange Format (QIF)](https://en.wikipedia.org/wiki/Quicken_Interchange_Format) files
- [Custom Reports](https://github.com/moneymanagerex/general-reports#general-reports)
- Installation not required: portable versions run from a USB or a flash drive
- Nonproprietary [SQLite](https://www.sqlite.org/) database with [Advanced Encryption Standard (AES)](https://en.wikipedia.org/wiki/Advanced_Encryption_Standard)
- Cross-platform, available for [desktop](https://moneymanagerex.org/docs/downloads/compatibility/) ([GNU/Linux](https://moneymanagerex.org/docs/downloads/compatibility/#linux), [macOS](https://moneymanagerex.org/docs/downloads/compatibility/#macos--osx), [Windows](https://moneymanagerex.org/docs/downloads/compatibility/#windows)) and mobile ([Android](https://android.moneymanagerex.org/)) operating systems
- International language support: [Crowdin](https://crowdin.com/project/moneymanagerex), [Desktop icon](/resources/dist/linux/share/applications/org.moneymanagerex.MMEX.desktop), [AppData](/resources/dist/linux/share/metainfo/org.moneymanagerex.MMEX.metainfo.xml.in), [Filetype associations](/resources/dist/linux/share/mime/packages/org.moneymanagerex.MMEX.mime.xml), [User interface language](/po/common.po)


## Download

<a href="https://flathub.org/apps/org.moneymanagerex.MMEX">
    <img src="https://dl.flathub.org/assets/badges/flathub-badge-en.svg"
        alt="Download on Flathub"
        title="Download on Flathub"
        style="height: 68px;"
        align="center"
        hspace="15"
    /></a>

<a href="https://apps.apple.com/us/app/money-manager-ex/id6683300571">
    <img src="https://toolbox.marketingtools.apple.com/api/v2/badges/download-on-the-app-store/black/en-us?releaseDate=1732665600"
        alt="Download on the App Store"
        title="Download on the App Store"
        style="height: 68px;"
        align="center"
        hspace="15"
    /></a>

<a href="https://f-droid.org/packages/com.money.manager.ex/">
    <img src="https://f-droid.org/badge/get-it-on.png"
        alt="Get it on F-Droid"
        title="Get it on F-Droid"
        style="height: 100px;"
        align="center"
    /></a>

<a href="https://play.google.com/store/apps/details?id=com.money.manager.ex.android">
    <img src="https://play.google.com/intl/en_us/badges/static/images/badges/en_badge_web_generic.png"
        alt="Get it on Google Play"
        title="Get it on Google Play"
        style="height: 100px;"
        align="center"
    /></a>

### Desktop
* [Download stable release](https://github.com/moneymanagerex/moneymanagerex/releases/) for GNU/Linux, macOS, or Windows
* Download developmental release for [GNU/Linux](https://github.com/moneymanagerex/moneymanagerex/issues/6246#issuecomment-1771746959), [macOS](https://link.ipx.co.uk/mmex-mac-betas), or [Windows](https://ci.appveyor.com/project/whalley/moneymanagerex)

### iPad/iPhone
* [Download on the App Store](https://apps.apple.com/us/app/money-manager-ex/id6683300571)

### Android
* [Get it on F-Droid](https://f-droid.org/packages/com.money.manager.ex/)
* [Get it on Google Play](https://play.google.com/store/apps/details?id=com.money.manager.ex.android)

[![Github All Releases](https://img.shields.io/github/downloads/moneymanagerex/moneymanagerex/total.svg)](https://tooomm.github.io/github-release-stats/?username=moneymanagerex&repository=moneymanagerex)


## Screenshots

<table border="0">
    <tr>
        <td>
            <img src="https://github.com/moneymanagerex/moneymanagerex/assets/17465872/01cc2202-9d5a-4bba-a816-5868512196cb" 
                 alt="Dashboard (MMEX 1.7.1-RC.1 64-bit on Linux Mint 21.3 Cinnamon)" 
                 title="Dashboard (MMEX 1.7.1-RC.1 64-bit on Linux Mint 21.3 Cinnamon)">
        </td>
        <td>
            <img src="https://github.com/moneymanagerex/moneymanagerex/assets/17465872/9ba8d274-7af8-496c-9ab6-2c49235ec735" 
                 alt="Payee Report (MMEX 1.7.1-RC.1 64-bit on Linux Mint 21.3 Cinnamon)" 
                 title="Payee Report (MMEX 1.7.1-RC.1 64-bit on Linux Mint 21.3 Cinnamon)">
        </td>
    </tr>
    <tr>
        <td>
            <img src="https://github.com/moneymanagerex/moneymanagerex/assets/17465872/ef2d2269-a973-4957-a10b-d13651390ad0" 
                 alt="Budget Planner (MMEX 1.7.1-RC.1 64-bit on Linux Mint 21.3 Cinnamon)" 
                 title="Budget Planner (MMEX 1.7.1-RC.1 64-bit on Linux Mint 21.3 Cinnamon)">
        </td>
        <td>
            <img src="https://github.com/moneymanagerex/moneymanagerex/assets/17465872/4edfc0c8-7bfc-4052-bf2c-4b833b8b1547" 
                 alt="Multiple User Interface Language Support – Hungarian (magyar) (MMEX 1.7.1-RC.1 64-bit on Linux Mint 21.3 Cinnamon)" 
                 title="Multiple User Interface Language Support – Hungarian (magyar) (MMEX 1.7.1-RC.1 64-bit on Linux Mint 21.3 Cinnamon)">
        </td>
    </tr>
    <tr>
        <td>
            <img src="https://github.com/moneymanagerex/moneymanagerex/assets/17465872/be52a7f3-9dea-46dd-a04e-f198dd413b08" 
                 alt="Category Manager – unlimited nested multi-level category support (MMEX 1.7.1-RC.1 64-bit on Linux Mint 21.3 Cinnamon)" 
                 title="Category Manager – unlimited nested multi-level category support (MMEX 1.7.1-RC.1 64-bit on Linux Mint 21.3 Cinnamon)">
        </td>
        <td>
            <img src="https://github.com/moneymanagerex/moneymanagerex/assets/17465872/d9dcdcd8-abda-4ae5-b6e4-4d137ef99613" 
                 alt="Customise Transaction Report – English (British) (MMEX 1.8.1-Beta.1 64-bit on Linux Mint 21.3 Cinnamon)" 
                 title="Customise Transaction Report – English (British) (MMEX 1.8.1-Beta.1 64-bit on Linux Mint 21.3 Cinnamon)">
        </td>
    </tr>
    <tr>
        <td>
            <img src="https://user-images.githubusercontent.com/17465872/233931375-051e7c9f-d4c4-4baa-9586-7ad10ef48aba.png" 
                 alt="Import from CSV (Comma-Separated Values) file (MMEX 1.6.4-Beta.4 64-bit on Linux Mint 21.1 Cinnamon)" 
                 title="Import from CSV (Comma-Separated Values) file (MMEX 1.6.4-Beta.4 64-bit on Linux Mint 21.1 Cinnamon)">
        </td>
        <td>
            <img src="https://user-images.githubusercontent.com/17465872/233931411-d7a92fed-272f-446d-b86a-89f99b66683a.png" 
                 alt="Import from QIF (Quicken Interchange Format) file (MMEX 1.6.4-Beta.4 64-bit on Linux Mint 21.1 Cinnamon)" 
                 title="Import from QIF (Quicken Interchange Format) file (MMEX 1.6.4-Beta.4 64-bit on Linux Mint 21.1 Cinnamon)">
        </td>
    <tr>
        <td>
            <img src="https://user-images.githubusercontent.com/17465872/233931435-825fa069-f182-4b50-970d-21755c90f1da.png" 
                 alt="Export as XML (eXtensible Markup Language) file (MMEX 1.6.4-Beta.4 64-bit on Linux Mint 21.1 Cinnamon)" 
                 title="Export as XML (eXtensible Markup Language) file (MMEX 1.6.4-Beta.4 64-bit on Linux Mint 21.1 Cinnamon)">
        </td>
        <td>
            <img src="https://github.com/moneymanagerex/moneymanagerex/assets/17465872/4e6e9a83-a242-43d5-9676-e65163f32fed" 
                 alt="Currency Manager – multi currency support (MMEX 1.7.1-RC.1 64-bit on Linux Mint 21.3 Cinnamon)" 
                 title="Currency Manager – multi currency support (MMEX 1.7.1-RC.1 64-bit on Linux Mint 21.3 Cinnamon)">
        </td>
    </tr>
</table>


## Credits

MMEX is mainly written in C++11 and uses the following open-source packages:
* [wxWidgets] 3.0+ 
* [wxSqlite3] 
* [SQLite3] 
* [RapidJSON]
* [ChartNew.JS] (outdated)
* [ApexCharts.js]
* [html-template] 
* [libcurl]


## External public resources

[Slack] | [Forum] | [Wiki] | [Facebook] | [YouTube] | [Crowdin] | [SourceForge] | [GitHub]
| [Docker images] | [Twitter] [![tw]][Twitter]


## Tips

* Database files (both regular and encrypted) can be opened directly
  with [wxsqliteplus]
* Download dozens of useful reports from the [general reports] repository
* Star this repository if you feel it is helpful [![st]][stargazers]

[website]: https://moneymanagerex.org "MMEX website"
[AppVeyor]: https://ci.appveyor.com/project/whalley/moneymanagerex "AppVeyor CI"
[CircleCI]: https://app.circleci.com/pipelines/github/moneymanagerex/moneymanagerex?branch=master "Circle CI"
[OpenHub]: https://www.openhub.net/p/moneymanagerex "Open Hub report"
[GitHubIssues]: https://github.com/moneymanagerex/moneymanagerex/issues "open issues on GitHub"
[SourceForgeDL]: https://sourceforge.net/projects/moneymanagerex/files/latest "SourceForge downloads"
[GitHubDL]: https://ci.appveyor.com/project/whalley/moneymanagerex/history "appveyor"
[contributors]: https://github.com/moneymanagerex/moneymanagerex/graphs/contributors "contributors to Git repo"
[GitHubLatest]: https://github.com/moneymanagerex/moneymanagerex/releases/latest "GitHub latest stable downloads"
[packagecloud]: https://packagecloud.io/moneymanagerex/ "packagecloud DEB & RPM repository"
[CodeDocs]: https://codedocs.xyz/moneymanagerex/moneymanagerex/ "doxygen generated source code documentation"
[Slack]: http://slack.moneymanagerex.org/
[Forum]: https://forum.moneymanagerex.org
[Wiki]: https://sourceforge.net/p/moneymanagerex/wiki/mmex
[Facebook]: https://www.facebook.com/MoneyManagerEx
[YouTube]: https://www.youtube.com/channel/UCAqVC0fOt6C5OnGv_DzE0wg
[SourceForge]: https://sourceforge.net/p/moneymanagerex
[GitHub]: https://github.com/moneymanagerex
[Docker images]: https://hub.docker.com/r/moneymanagerex/
[Twitter]: https://twitter.com/MoneyManagerEx
[wxWidgets]: http://wxwidgets.org/
[wxSqlite3]: https://github.com/utelle/wxsqlite3
[SQLite3]: http://sqlite.org/
[Mongoose]: https://github.com/cesanta/mongoose
[RapidJSON]: http://rapidjson.org/
[ChartNew.JS]: https://github.com/FVANCOP/ChartNew.js
[ApexCharts.js]: https://apexcharts.com/
[html-template]: https://github.com/moneymanagerex/html-template
[libcurl]: https://curl.haxx.se/libcurl/
[wxsqliteplus]: https://github.com/guanlisheng/wxsqliteplus
[general reports]: https://github.com/moneymanagerex/general-reports
[stargazers]: https://github.com/moneymanagerex/moneymanagerex/stargazers
[a]: https://img.shields.io/appveyor/ci/whalley/moneymanagerex/master.svg?label=windows&logoWidth=0.01
[ci]: https://img.shields.io/circleci/build/github/moneymanagerex/moneymanagerex/master?label=linux/mac
[o]: http://www.openhub.net/p/moneymanagerex/widgets/project_thin_badge.gif
[i]: https://img.shields.io/github/issues-raw/moneymanagerex/moneymanagerex.svg?label=gh%20issues
[c]: https://img.shields.io/github/contributors/moneymanagerex/moneymanagerex.svg
[cd]: https://codedocs.xyz/moneymanagerex/moneymanagerex.svg
[l]: https://img.shields.io/badge/license-GPL2-blue.svg
[s]: https://img.shields.io/sourceforge/dt/moneymanagerex.svg?label=%20sf&logoWidth=8&logo=data%3Aimage%2Fpng%3Bbase64%2CiVBORw0KGgoAAAANSUhEUgAAAC0AAAA4CAMAAABe34GAAAAACVBMVEUAAAAAAAD%2F%2F%2F%2BD3c%2FSAAAAAnRSTlMATX7%2B8BUAAABeSURBVHgB7coxCoBAAAOw0%2F8%2FWtA1tLs0c85Et21vb%2F9pe1jsyP7OrSOzI5eOfL1K%2FlLuyO7O7s7oLbs7uzu7O7s7uzv37uzu7O7sjly6szty6s7uzoZcurMhJ87nAdpRCKWC0tGVAAAAAElFTkSuQmCC
[gha]: https://img.shields.io/github/downloads/moneymanagerex/moneymanagerex/total.svg?label=%20gh&logoWidth=8&logo=data%3Aimage%2Fpng%3Bbase64%2CiVBORw0KGgoAAAANSUhEUgAAAC0AAAA4CAMAAABe34GAAAAACVBMVEUAAAAAAAD%2F%2F%2F%2BD3c%2FSAAAAAnRSTlMATX7%2B8BUAAABeSURBVHgB7coxCoBAAAOw0%2F8%2FWtA1tLs0c85Et21vb%2F9pe1jsyP7OrSOzI5eOfL1K%2FlLuyO7O7s7oLbs7uzu7O7s7uzv37uzu7O7sjly6szty6s7uzoZcurMhJ87nAdpRCKWC0tGVAAAAAElFTkSuQmCC
[ghl]: https://img.shields.io/github/downloads/moneymanagerex/moneymanagerex/latest/total.svg?label=%20gh%20latest&logoWidth=8&logo=data%3Aimage%2Fpng%3Bbase64%2CiVBORw0KGgoAAAANSUhEUgAAAC0AAAA4CAMAAABe34GAAAAACVBMVEUAAAAAAAD%2F%2F%2F%2BD3c%2FSAAAAAnRSTlMATX7%2B8BUAAABeSURBVHgB7coxCoBAAAOw0%2F8%2FWtA1tLs0c85Et21vb%2F9pe1jsyP7OrSOzI5eOfL1K%2FlLuyO7O7s7oLbs7uzu7O7s7uzv37uzu7O7sjly6szty6s7uzoZcurMhJ87nAdpRCKWC0tGVAAAAAElFTkSuQmCC
[w]: https://img.shields.io/website-up-down-brightgreen-red/https/moneymanagerex.org/.svg
[p]: https://img.shields.io/website-up-down-brightgreen-red/https/packagecloud.io/moneymanagerex/.svg?label=deb%20rpm%20repo
[stable]: https://img.shields.io/github/release/moneymanagerex/moneymanagerex.svg?label=stable

[unstable]: https://img.shields.io/badge/unstable-beta-yellow
[st]: https://img.shields.io/github/stars/moneymanagerex/moneymanagerex.svg?&label=GitHub&style=social&logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAA4AAAAOCAYAAAAfSC3RAAAA50lEQVQoz5XSr0oFQRzF8c%2FuBd0iiCYviHi1GXwABYMugiaLmKw3iF18AetF8B1MNouDb6AICgbfwCqGKf4JrjIse132lGHOnC%2FMnN%2FQoBjMxyDEYN8Y5WP8IbYwisFMF%2FCkWuew2QrGYCIGI%2FQS%2BzQGs3Uwi8EOjrFReZMNN4nIcI9bnGUxeMOUblrJsYevDtAlXrLqbUc4%2F6esX90Upe2%2FcorSBQ5boGvsNrX63AK%2B4rMJXG8Bl9IxpeBaLfhY2y%2Bm%2Bbx2AA84KEqrWMYVPtBP81nya%2FpYwFNRek%2F8HgaYxl1R%2FozuGyXRMhoIJiaoAAAAAElFTkSuQmCC
[tw]: https://img.shields.io/twitter/follow/MoneyManagerEx.svg?style=social&label=follow
[Buy us a Cofee]: https://cash.me/$guanlisheng/1
[Donate via PayPal]: https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=moneymanagerex%40gmail%2ecom&lc=US&item_name=MoneyManagerEx&no_note=0&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHostedGuest
[Crowdin]: https://crowdin.com/project/moneymanagerex


## Support

[![Donate via PayPal](https://www.paypalobjects.com/webstatic/mktg/Logo/pp-logo-100px.png)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=moneymanagerex%40gmail%2ecom&lc=US&item_name=MoneyManagerEx&no_note=0&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHostedGuest";)

## License

[GPL](LICENSE)


## Translations

[![CrowdIn](https://badges.awesome-crowdin.com/translation-12682419-342055.png)](https://crowdin.com/project/moneymanagerex)


## Stargazers over time

[![Stargazers over time](https://starchart.cc/moneymanagerex/moneymanagerex.svg)](https://starchart.cc/moneymanagerex/moneymanagerex)
