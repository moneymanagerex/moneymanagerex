FROM ubuntu:artful
LABEL maintainer="developers@moneymanagerex.org"
ADD https://repos.codelite.org/CodeLite.asc /etc/apt/trusted.gpg.d/
RUN chown _apt:root /etc/apt/trusted.gpg.d/CodeLite.asc \
 && echo 'deb http://repos.codelite.org/wx3.1/ubuntu/ artful universe' \
      >/etc/apt/sources.list.d/CodeLite.list \
 && apt-get update && apt-get install -y --no-install-recommends \
      build-essential \
      ccache \
      cmake \
      file \
      gettext \
      git \
      libcurl4-openssl-dev \
      liblua5.3-dev \
      libwxgtk-webview3.1-dev \
      lsb-release \
      pkg-config \
      rapidjson-dev \
 && rm -rf /var/lib/apt/lists/*

ENV MMEX_INST_CMD apt-get update && apt install -yqV ./mmex_*.deb
