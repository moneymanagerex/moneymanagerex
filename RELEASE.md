Preparing and releasing a new Money Manager Ex release
======================================================

# Prepare

Ensure that all required PRs are merged.

Ensure that [contrib.txt](https://github.com/moneymanagerex/moneymanagerex/blob/master/contrib.txt) is up-to-date and captures all current contributors.

# Create release notes.

Update [NEWS](https://github.com/moneymanagerex/moneymanagerex/blob/master/NEWS) file to document the release number, date of release and all changes. Once done push this file to GitHub.

We use the [GitHub Releases](https://docs.github.com/en/repositories/releasing-projects-on-github/managing-releases-in-a-repository) feature to create a new release. Create a  release description here: [GitHub Releases](https://github.com/moneymanagerex/moneymanagerex/releases) and us the same basic descriptions as used in the NEWS file. At this point only save the release as "DRAFT". 

**NOTE:** In order to generate NEWS file and release note contents it is useful to extract key information directly from the GitHub issues that have been resolved, for example:

E.g.
`gh issue list -m v1.9.5 -s all -l enhancement | awk -F"\t" '{ print "* #"$1,$3 }'`

`gh issue list -m v1.9.5 -s all -l feature | awk -F"\t" '{ print "* #"$1,$3 }'`

`gh issue list -m v1.9.5 -s all -l bug | awk -F"\t" '{ print "* #"$1,$3 }'`

# Buildimng Release Assets

Once the latest automation builds have been completed on the master branch, i.e. those that include the last pushed NEWS file the Windows and Linux builds can be added to the release assets.

* mmex-RELEASENUM-win32-portable.zip
* mmex-RELEASENUM-win32.exe
* mmex-RELEASENUM-win64-portable.zip
* mmex-RELEASENUM-win64.exe
* mmex_RELEASENUM-Ubuntu.22.04.jammy_amd64.deb
* mmex_RELEASENUM-Ubuntu.24.04.noble_amd64.deb

A MacOS build automation is not currently available so it must be manually built using the [MacOS build instructions](https://github.com/moneymanagerex/moneymanagerex/blob/master/BUILD.md#macos-with-homebrew). Once built, it must be signed by a Apple developer. Once signed it can be added to the release assets.

* mmex-RELEASENUM-Darwin.dmg

# Releasing the software

Once all the assets have been added to the release assets, the release can be published by editing the release, creating a new tag, and then publishing the release.

# Announcing the release

The latest release should be announced on the customer site, which can be found at [here](https://github.com/moneymanagerex/moneymanagerex.github.io). The following changes should be made:

* Update https://github.com/moneymanagerex/moneymanagerex.github.io/blob/main/config/_default/params.toml with the latest release number.

* Update https://github.com/moneymanagerex/moneymanagerex.github.io/blob/main/content/en/docs/downloads/older/index.md to include details of the previous release, which can be copied from https://github.com/moneymanagerex/moneymanagerex.github.io/blob/main/content/en/docs/downloads/latest/index.md.

* Update https://github.com/moneymanagerex/moneymanagerex.github.io/blob/main/content/en/docs/downloads/latest/index.md with details of  latest release. This includes the release notes and a link to the download pages.


**Announce the new release on the MMEX forum [Announcements](https://forum.moneymanagerex.org/viewforum.php?f=3).**