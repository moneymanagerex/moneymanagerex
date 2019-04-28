FROM opensuse/archive:42.1
LABEL maintainer="developers@moneymanagerex.org"
RUN zypper install -y --no-recommends \
      ccache \
      cmake \
      gcc5-c++ \
      gettext-tools \
      git \
      libcurl-devel \
      lsb-release \
      make \
      rpm-build \
      wxWidgets-3_0-devel \
 && zypper clean --all \
 && update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-5 50 \
 && update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-5 50

ENV MMEX_INST_CMD zypper --no-refresh --no-gpg-checks install -y ./mmex-*.rpm
