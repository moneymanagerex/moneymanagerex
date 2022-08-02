FROM ubuntu:bionic
MAINTAINER developers@moneymanagerex.org
RUN ln -snf /usr/share/zoneinfo/Europe/London /etc/localtime && echo "Europe/London" > /etc/timezone && \
    apt update && \
    apt install -y --no-install-recommends \
      cmake build-essential libssl-dev gettext git pkg-config lsb-release file ccache \
      libgtk-3-dev liblua5.3-dev libcurl4-openssl-dev libwebkit2gtk-4.0-dev wget unzip && \
    apt clean && \
    cd / && \
    wget --no-check-certificate https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.7/wxWidgets-3.1.7.zip && \
    unzip -d wxWidgets wxWidgets-*.zip && \
    rm wxWidgets-*.zip && \
    cd wxWidgets && \
    mkdir build-linux && \
    cd build-linux && \
    ../configure --disable-shared --enable-cxx11 --with-cxx=11 --enable-webview --with-gtk=3 --enable-unicode --enable-intl && \
    make

ENV MMEX_INST_CMD apt install -yqV ./mmex_*.deb
