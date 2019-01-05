FROM opensuse:tumbleweed
LABEL maintainer="developers@moneymanagerex.org"
RUN zypper refresh && zypper install -y --no-recommends \
      ccache \
      cmake \
      gcc-c++ \
      gettext-tools \
      git \
      libcurl-devel \
      lsb-release \
      make \
      rpm-build \
      wxWidgets-devel \
 && zypper clean --all

ENV MMEX_INST_CMD zypper --no-refresh --no-gpg-checks install -y ./mmex-*.rpm
