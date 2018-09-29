Installing Money Manager Ex
===========================

This document describes installation procedures from pre-compiled binary
packages for different scenarios. Please see [BUILD.md] If you are
interested in compiling and installing from sources.

Jump to [Windows] | [Linux] | [macOS] instructions.

[![stable]][GitHubLatest] [![unstable]][GitHubDL]

Microsoft Windows
-----------------

Microsoft Windows XP, 2003, Vista, 7, 8, 10 are supported, both 32 and 64-bit
versions. Download installer or ZIP portable package from
[GitHub Releases][GitHubDL].

> Use unstable pre-release version (*alpha*, *beta*, *rc*) at your risk
> (work on your database copies!) to check or test newly added functions.

To use portable package simply extract files with directory structure
preserved to selected directory on your hard disk, external drive, pen-drive
etc. and start executable `mmex.exe`. This package not use any files outside
its directory.

Required *Visual C++ Runtime* and *Universal CRT* DLLs are included in MMEX
packages. `msvc*.dll` and `vcruntime*.dll` can be safely deleted if the same
version of *Visual C++ Redistributable Package* or from Microsoft is installed
at system level. `api-ms-win-*.dll` and `ucrtbase.dll` can be deleted under
Windows 10 as they are a component of the Windows operating system.

> Don't delete `wx*.dll` files as they are required to start MMEX!

Linux
-----

### From Snap Store

If you are using one of the following Linux distributions: Arch Linux, Debian,
elementary OS, Fedora, Gentoo, Linux Mint, Manjaro, OpenEmbedded/Yocto,
openSUSE, OpenWrt, Solus, Raspbian, Ubuntu then you can use snapd to install
and run MMEX snap package available from [Snap Store].

> Warning: only unstable versions of MMEX are available now.

snapd is the service you need to install to run and manage snaps. Please
follow [snapd installation instructions] for your distribution.

To install MMEX open terminal window and run `sudo snap install mmex` command.
Then you can run MMEX from your desktop menu or using `mmex` command.
Snaps are updated automatically in the background to the latest version,
every day.

If you want to test unstable version of MMEX you can install with
`sudo snap install --beta mmex` or upgrade with
`sudo snap refresh mmex --beta` commands. You can list available MMEX versions
with `snap info mmex` command.

> Use unstable pre-release version (from *beta* channel) at your risk
> (work on your database copies!) to check or test newly added functions.

### From distribution repository

> Important: There are no MMEX packages in main repositories of popular Linux
> distributions so far. Please use different methods described below.

You must have permissions to use `sudo` command or have admin permissions to
install MMEX in Linux system.

Open terminal window and run command from table below to install binary
package with dependencies.

| Distribution         | Shell command to install MMEX package                |
|----------------------|------------------------------------------------------|
| Debian, Ubuntu, Mint | `sudo apt update; sudo apt install mmex`             |
| openSUSE             | `sudo zypper refresh; sudo zypper install mmex`      |
| Fedora               | `sudo dnf --refresh install mmex`                    |
| CentOS               | `sudo yum clean expire-cache; sudo yum install mmex` |
| Arch                 | `sudo pacman -Syyu mmex`                             |

MMEX can be started from desktop menu or from terminal window using `mmex`
command.

### From MMEX repository

You can configure your Linux distribution to use additional unofficial
repository for additional packages.

> Please remember: they are not supported by your distribution support
> channels!

MMEX developers created dedicated repository for MMEX binary packages for
multiple Linux distributions at [packagecloud.io] hosting platform.
New versions are published there automatically (using CI) when they arrive.
Debian 8/9/10, CentOS 7, Fedora 24-29, Mint 18/19, openSUSE Leap 42.2/42.3,
Ubuntu 16/17/18 are supported for now.

> Warning: only unstable versions of MMEX are available now.

You can [add MMEX repository] to your system configuration using following
commands from terminal window:

    curl -s https://packagecloud.io/install/repositories/moneymanagerex/moneymanagerex/script.deb.sh | sudo bash

for DEB based Linux distributions or:

    curl -s https://packagecloud.io/install/repositories/moneymanagerex/moneymanagerex/script.rpm.sh | sudo bash

for RPM based Linux distributions.

There are instructions to [manually add MMEX repo] to system configuration
available too.

> Important: in Ubuntu Artful (17.10) you need to manually add CodeLite
> wxWidgets repo with following commands
>
>     sudo apt-key adv --fetch-keys http://repos.codelite.org/CodeLite.asc
>     sudo apt-add-repository 'deb http://repos.codelite.org/wx3.1/ubuntu/ artful universe'

You can use package manager from your distribution to install MMEX as
described in previous section above **after** adding MMEX repository to your
configuration.

### From manually downloaded binaries

Download package for your distribution from [GitHub Releases][GitHubDL].
Arch, Debian 8/9/10, CentOS 7, Fedora 24-29/rawhide, Mint 18/19, openSUSE
Leap 42.2/42.3/tumbleweed, Ubuntu 16/17/18, Slackware 14.2 are supported for now.

> Use unstable pre-release version (*alpha*, *beta*, *rc*) at your risk
> (work on your database copies!) to check or test newly added functions.

To install downloaded package file run following command from terminal window:

| Distribution         | Install package from local file              |
|----------------------|----------------------------------------------|
| Debian, Ubuntu, Mint | `sudo apt install ./mmex-<version>.deb`      |
| openSUSE             | `sudo zypper install ./mmex-<version>.rpm`   |
| Fedora               | `sudo dnf install ./mmex-<version>.rpm`      |
| CentOS               | `sudo yum install ./mmex-<version>.rpm`      |
| Arch                 | `sudo pacman -U ./mmex-<version>.pkg.tar.xz` |
| Slackware            | `sudo installpkg ./mmex-<version>.txz`       |

Slackware requires additional dependency packages to be installed: libwebp,
webkitgtk and wxGTK3 (with webview support). They can be build from
[slackbuilds.org] repo.

OS X / macOS
------------

Download Drag'N'Drop package (.dmg file) from [GitHub Releases][GitHubDL].
OS X / macOS versions starting from 10.7 (Lion) are supported.

> Use unstable pre-release version (*alpha*, *beta*, *rc*) at your risk
> (work on your database copies!) to check or test newly added functions.

Double click the downloaded file to mount it - name will show up in the Finder
sidebar and a new Finder window will open showing MMEX icon, arrow and
Applications folder shortcut.

Drag the application icon to Applications to install (may need an
administrator password to do this).

Eject the mounted .dmg package by clicking the eject button in the Sidebar
when the copy process is finished. You can delete the .dmg file after that.

[BUILD.md]: BUILD.md
[Windows]: #microsoft-windows
[Linux]: #linux
[macOS]: #os-x--macos
[stable]: https://img.shields.io/github/release/moneymanagerex/moneymanagerex.svg?label=stable
[unstable]: https://img.shields.io/github/tag-pre/moneymanagerex/moneymanagerex.svg?label=unstable
[GitHubDL]:
  https://github.com/moneymanagerex/moneymanagerex/releases
  "GitHub downloads"
[GitHubLatest]:
  https://github.com/moneymanagerex/moneymanagerex/releases/latest
  "GitHub latest stable downloads"
[AppVeyor]: https://ci.appveyor.com/project/moneymanagerex/moneymanagerex
[packagecloud.io]: https://packagecloud.io/moneymanagerex/moneymanagerex
[add MMEX repository]: https://packagecloud.io/moneymanagerex/moneymanagerex/install#bash
[manually add MMEX repo]: https://packagecloud.io/moneymanagerex/moneymanagerex/install#manual
[slackbuilds.org]: https://slackbuilds.org/
[Snap Store]: https://snapcraft.io/mmex
[snapd installation instructions]: https://docs.snapcraft.io/core/install
