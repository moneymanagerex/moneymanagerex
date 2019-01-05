FROM debian:stretch-slim
LABEL maintainer="developers@moneymanagerex.org"
RUN echo deb http://ftp.debian.org/debian stretch-backports main \
      > /etc/apt/sources.list.d/backports.list \
 && dpkg --add-architecture i386 && apt-get update \
 && apt-get install -y --no-install-recommends \
      build-essential \
      ccache \
      cmake \
      file \
      g++-multilib \
      gettext \
      git \
      libcurl4-openssl-dev:i386 \
      liblua5.3-dev:i386 \
      lsb-release \
      pkg-config \
      rapidjson-dev \
 && apt-get install -t stretch-backports -y --no-install-recommends \
      libwxgtk-webview3.0-dev:i386 \
 && rm -rf /var/lib/apt/lists/*

ENV MMEX_INST_CMD apt-get update && apt install -t stretch-backports -yqV ./mmex_*.deb
