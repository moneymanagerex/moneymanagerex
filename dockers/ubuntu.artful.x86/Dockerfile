FROM ubuntu:artful
LABEL maintainer="developers@moneymanagerex.org"
ADD https://repos.codelite.org/CodeLite.asc /etc/apt/trusted.gpg.d/
RUN chown _apt:root /etc/apt/trusted.gpg.d/CodeLite.asc \
 && echo 'deb http://repos.codelite.org/wx3.1/ubuntu/ artful universe' \
      >/etc/apt/sources.list.d/CodeLite.list \
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
      libwxgtk-webview3.1-dev:i386 \
      lsb-release \
      pkg-config \
      rapidjson-dev \
 && rm -rf /var/lib/apt/lists/*

# requires wx3.0-i18n 3.0.4+dfsg-1 or later
# ENV MMEX_INST_CMD apt-get update && apt install -yqV ./mmex_*.deb
