FROM ubuntu:zesty
MAINTAINER developers@moneymanagerex.org
RUN sed -i -re 's/(([a-z]{2}\.)?archive|security)\.ubuntu\.com/old-releases.ubuntu.com/g' /etc/apt/sources.list && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
      cmake build-essential gettext git pkg-config lsb-release ccache \
      libwxgtk-webview3.0-dev liblua5.3-dev libcurl4-openssl-dev && \
    apt-get clean

ENV MMEX_INST_CMD apt install -yqV ./mmex_*.deb
