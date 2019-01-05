FROM ubuntu:xenial
LABEL maintainer="developers@moneymanagerex.org"
RUN dpkg --add-architecture i386 && apt-get update \
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
      libwxgtk-webview3.0-dev:i386 \
      lsb-release \
      pkg-config \
      rapidjson-dev \
 && rm -rf /var/lib/apt/lists/*

# requires wx3.0-i18n 3.0.4+dfsg-1 or later
# ENV MMEX_INST_CMD apt-get update && apt install -yqV ./mmex_*.deb
