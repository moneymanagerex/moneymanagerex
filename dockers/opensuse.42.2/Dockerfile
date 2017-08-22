FROM opensuse:42.2
MAINTAINER developers@moneymanagerex.org
RUN zypper install -y --no-recommends \
      cmake gettext-tools gcc-c++ git make rpm-build lsb-release \
      wxWidgets-3_0-devel \
      automake libtool \
 && zypper clean --all
