FROM fedora:36
LABEL maintainer="developers@moneymanagerex.org"
RUN ln -snf /usr/share/zoneinfo/Europe/London /etc/localtime && echo "Europe/London" > /etc/timezone && \
    dnf -y install --setopt=install_weak_deps=False \
      ccache \
      cmake \
      gcc-c++ \
      gettext \
      git \
      libcurl-devel \
      lua-devel \
      make \
      rapidjson-devel \
      redhat-lsb-core \
      rpm-build \
      wxGTK3-devel \
      webkit2gtk3-devel \
      wget \
      unzip && \
    dnf clean all && \
    cd / && \
    wget --no-check-certificate https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.7/wxWidgets-3.1.7.zip && \
    unzip -d wxWidgets wxWidgets-*.zip && \
    rm wxWidgets-*.zip && \
    cd wxWidgets && \
    mkdir build-linux && \
    cd build-linux && \
    ../configure --disable-shared --enable-cxx11 --with-cxx=11 --enable-webview --with-gtk=3 --enable-unicode --enable-intl && \
    make

ENV MMEX_INST_CMD dnf -y install ./mmex-*.rpm