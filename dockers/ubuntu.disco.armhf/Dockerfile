FROM ubuntu:disco
LABEL maintainer="developers@moneymanagerex.org"
RUN dpkg --add-architecture armhf \
 && sed -ri 's#deb( http://([a-z]{2}\.)?archive|security)#deb [arch=i386,amd64]\1#' /etc/apt/sources.list \
 && echo 'deb [arch=armhf] http://ports.ubuntu.com/ disco main universe' >> /etc/apt/sources.list \
 && apt-get update && apt-get install -y --no-install-recommends \
      ccache \
      cmake \
      crossbuild-essential-armhf \
      file \
      gettext \
      git \
      libcurl4-openssl-dev:armhf \
      liblua5.3-dev:armhf \
      libwxgtk-webview3.0-gtk3-dev:armhf \
      lsb-release \
      pkg-config \
      rapidjson-dev \
 && rm -rf /var/lib/apt/lists/*

ENV MMEX_INST_CMD apt-get update && apt install -yqV ./mmex_*.deb
