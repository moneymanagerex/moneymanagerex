FROM ubuntu:artful
MAINTAINER developers@moneymanagerex.org
ADD https://repos.codelite.org/CodeLite.asc /etc/apt/trusted.gpg.d/
RUN chown _apt:root /etc/apt/trusted.gpg.d/CodeLite.asc && \
    echo 'deb http://repos.codelite.org/wx3.1/ubuntu/ artful universe' \
      >/etc/apt/sources.list.d/CodeLite.list && \
    dpkg --add-architecture i386 && apt-get update && \
    apt-get install -y --no-install-recommends \
      cmake build-essential g++-multilib gettext git pkg-config lsb-release file ccache \
      libwxgtk-webview3.1-dev:i386 liblua5.3-dev:i386 \
      libcurl4-openssl-dev:i386 && \
    apt-get clean

ENV MMEX_INST_CMD apt install -yqV ./mmex_*.deb
