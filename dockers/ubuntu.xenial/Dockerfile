FROM ubuntu:xenial
MAINTAINER developers@moneymanagerex.org
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      cmake build-essential gettext git pkg-config lsb-release \
      libwebkitgtk-dev libwxgtk3.0-dev libwxgtk-webview3.0-dev \
      libwxsqlite3-3.0-dev liblua5.2-dev \
      automake libtool && \
    apt-get clean
