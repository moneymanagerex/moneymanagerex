FROM opensuse:tumbleweed
MAINTAINER developers@moneymanagerex.org
RUN zypper refresh && zypper install -y --no-recommends \
      cmake gettext-tools gcc-c++ git make rpm-build lsb-release \
      wxWidgets-3_0-devel \
      automake libtool \
 && zypper clean --all
