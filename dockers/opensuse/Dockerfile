FROM opensuse:tumbleweed
MAINTAINER developers@moneymanagerex.org
RUN zypper refresh && zypper install -y --no-recommends \
      cmake gettext-tools gcc-c++ git make rpm-build lsb-release ccache \
      wxWidgets-devel libcurl-devel \
 && zypper clean --all

ENV MMEX_INST_CMD zypper --no-refresh --no-gpg-checks install -y ./mmex-*.rpm
