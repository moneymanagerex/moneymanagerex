FROM debian:bullseye-slim
LABEL maintainer="developers@moneymanagerex.org"
RUN dpkg --add-architecture i386 && aptitude update \
 && aptitude install -y --no-install-recommends \
      libc6 \
      build-essential \
      ccache \
      cmake \
      file \
      g++-multilib \
      gettext \
      git \
      libcurl4-openssl-dev:i386 \
      liblua5.3-dev:i386 \
      libwxgtk-webview3.0-gtk3-dev:i386 \
      lsb-release \
      pkg-config \
      rapidjson-dev \
 && rm -rf /var/lib/apt/lists/*

ENV MMEX_INST_CMD apt-get update && apt install -yqV ./mmex_*.deb
