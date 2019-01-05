FROM fedora:24
LABEL maintainer="developers@moneymanagerex.org"
RUN dnf -y install --setopt=install_weak_deps=False \
      ccache \
      cmake \
      gcc-c++ \
      gettext \
      git \
      libcurl-devel \
      lua-devel \
      make \
      redhat-lsb-core \
      rpm-build \
      wxGTK3-devel \
 && dnf clean all

ENV MMEX_INST_CMD dnf -y install ./mmex-*.rpm
