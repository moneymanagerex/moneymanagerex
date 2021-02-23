FROM ubuntu:groovy
MAINTAINER developers@moneymanagerex.org
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends tcl \
    cmake build-essential gettext git pkg-config lsb-release ccache \
    libwxgtk-webview3.0-gtk3-dev liblua5.3-dev libcurl4-openssl-dev && \
    apt-get clean

ENV MMEX_INST_CMD apt install -yqV ./mmex_*.deb
