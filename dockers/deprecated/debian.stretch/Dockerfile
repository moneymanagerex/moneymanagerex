FROM debian:stretch-slim
MAINTAINER developers@moneymanagerex.org
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      cmake build-essential gettext git pkg-config lsb-release file ccache \
      libwxgtk-webview3.0-dev liblua5.2-dev libcurl4-openssl-dev && \
    apt-get clean
