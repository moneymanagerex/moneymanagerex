FROM debian:jessie-slim
LABEL maintainer="developers@moneymanagerex.org"
RUN echo deb http://ftp.debian.org/debian jessie-backports main \
      > /etc/apt/sources.list.d/backports.list && \
    dpkg --add-architecture i386 && apt-get update && \
    apt-get install -y --no-install-recommends \
      build-essential g++-multilib gettext git pkg-config lsb-release file ccache \
      libwxgtk-webview3.0-dev:i386 liblua5.2-dev:i386 \
      libcurl4-openssl-dev:i386 && \
    apt-get install -t jessie-backports -y --no-install-recommends cmake && \
    apt-get clean
