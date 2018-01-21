FROM ubuntu:xenial
MAINTAINER developers@moneymanagerex.org
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      cmake build-essential gettext git pkg-config lsb-release \
      libwxgtk3.0-dev libwxgtk-webview3.0-dev \
      libwxsqlite3-3.0-dev liblua5.2-dev libcurl4-openssl-dev \
      automake libtool ccache && \
    apt-get clean

ENV MMEX_INST_CMD apt install -yqV ./mmex_*.deb
