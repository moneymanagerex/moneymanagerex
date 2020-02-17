FROM opensuse/leap:15.0
LABEL maintainer="developers@moneymanagerex.org"
RUN zypper install -y --no-recommends \
      ccache \
      cmake \
      gcc-c++ \
      gettext-tools \
      git \
      libcurl-devel \
      lsb-release \
      make \
      rpm-build \
      wxWidgets-3_2-devel \
 && zypper clean --all

ENV MMEX_INST_CMD zypper --no-refresh --no-gpg-checks install -y ./mmex-*.rpm
