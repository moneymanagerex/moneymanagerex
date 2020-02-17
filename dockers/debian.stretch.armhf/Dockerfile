FROM debian:stretch-slim
LABEL maintainer="developers@moneymanagerex.org"
RUN echo deb http://ftp.debian.org/debian stretch-backports main \
      > /etc/apt/sources.list.d/backports.list \
 && dpkg --add-architecture armhf && apt-get update \
 && apt-get install -y --no-install-recommends \
      cmake \
      crossbuild-essential-armhf \
      file ccache \
      gettext \
      git \
      libcurl4-openssl-dev:armhf \
      liblua5.3-dev:armhf \
      lsb-release \
      pkg-config \
      rapidjson-dev \
 && apt-get install -t stretch-backports -y --no-install-recommends \
      libwxgtk-webview3.0-dev:armhf \
 && rm -rf /var/lib/apt/lists/*

ENV MMEX_INST_CMD apt-get update && apt install -t stretch-backports -yqV ./mmex_*.deb
