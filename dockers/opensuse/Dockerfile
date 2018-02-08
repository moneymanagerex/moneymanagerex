FROM opensuse:tumbleweed
MAINTAINER developers@moneymanagerex.org
RUN zypper refresh && zypper install -y --no-recommends \
      cmake gettext-tools gcc-c++ git make rpm-build lsb-release \
      wxWidgets-devel libcurl-devel \
      automake libtool ccache \
 && zypper clean --all
