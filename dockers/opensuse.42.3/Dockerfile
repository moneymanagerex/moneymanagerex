FROM opensuse/leap:42.3
LABEL maintainer="developers@moneymanagerex.org"
RUN zypper install -y --no-recommends \
      ccache \
      cmake \
      gcc8-c++ \
      gettext-tools \
      git \
      libcurl-devel \
      lsb-release \
      make \
      rpm-build \
      wxWidgets-3_0-devel \
 && zypper clean --all \
 && update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-8 50 \
 && update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-8 50

ENV MMEX_INST_CMD zypper --no-refresh --no-gpg-checks install -y ./mmex-*.rpm
