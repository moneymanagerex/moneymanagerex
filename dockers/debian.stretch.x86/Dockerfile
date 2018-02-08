FROM debian:stretch-slim
MAINTAINER developers@moneymanagerex.org
RUN dpkg --add-architecture i386 && apt-get update && \
    apt-get install -y --no-install-recommends \
      cmake build-essential g++-multilib gettext git pkg-config lsb-release \
      libwxgtk3.0-dev:i386 libwxgtk-webview3.0-dev:i386 \
      libwxsqlite3-3.0-dev:i386 liblua5.2-dev:i386 libcurl4-openssl-dev:i386 \
      automake libtool ccache && \
    apt-get clean
