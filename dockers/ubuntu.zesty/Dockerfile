FROM ubuntu:zesty
LABEL maintainer="developers@moneymanagerex.org"
RUN sed -ri 's/(([a-z]{2}\.)?archive|security)\.ubuntu\.com/old-releases.ubuntu.com/' /etc/apt/sources.list \
 && apt-get update && apt-get install -y --no-install-recommends \
      build-essential \
      ccache \
      cmake \
      file \
      gettext \
      git \
      libcurl4-openssl-dev \
      liblua5.3-dev \
      libwxgtk-webview3.0-dev \
      lsb-release \
      pkg-config \
      rapidjson-dev \
 && rm -rf /var/lib/apt/lists/*

ENV MMEX_INST_CMD apt-get update && apt install -yqV ./mmex_*.deb
