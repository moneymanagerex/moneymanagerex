FROM debian:jessie-slim
LABEL maintainer="developers@moneymanagerex.org"
RUN echo deb http://ftp.debian.org/debian jessie-backports main \
      > /etc/apt/sources.list.d/backports.list && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
      build-essential gettext git pkg-config lsb-release file ccache \
      libwxgtk-webview3.0-dev liblua5.2-dev libcurl4-openssl-dev && \
    apt-get install -t jessie-backports -y --no-install-recommends cmake && \
    apt-get clean
