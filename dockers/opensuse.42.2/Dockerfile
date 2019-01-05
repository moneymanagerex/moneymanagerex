FROM opensuse:42.2
LABEL maintainer="developers@moneymanagerex.org"
RUN zypper install -y --no-recommends \
      ccache \
      cmake \
      gcc-c++ \
      gettext-tools \
      git \
      libcurl-devel \
      lsb-release \
      lua-devel \
      make \
      rpm-build \
      wxWidgets-3_0-devel \
 && zypper clean --all

ENV MMEX_INST_CMD zypper --no-refresh --no-gpg-checks install -y ./mmex-*.rpm
