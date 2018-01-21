FROM ubuntu:zesty
MAINTAINER developers@moneymanagerex.org
RUN sed -i -re 's/(([a-z]{2}\.)?archive|security)\.ubuntu\.com/old-releases.ubuntu.com/g' /etc/apt/sources.list && \
    dpkg --add-architecture i386 && apt-get update && \
    apt-get install -y --no-install-recommends \
      cmake build-essential g++-multilib gettext git pkg-config lsb-release \
      libwxgtk3.0-dev:i386 libwxgtk-webview3.0-dev:i386 \
      libwxsqlite3-3.0-dev:i386 liblua5.2-dev:i386 libcurl4-openssl-dev:i386\
      automake libtool ccache && \
    apt-get clean

ENV MMEX_INST_CMD apt install -yqV ./mmex_*.deb
