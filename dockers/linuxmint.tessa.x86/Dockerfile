FROM linuxmintd/mint19.1-i386:latest
LABEL maintainer="developers@moneymanagerex.org"
RUN dpkg --add-architecture i386 && apt-get update \
 && apt-get install -y --no-install-recommends \
      build-essential \
      ccache \
      cmake \
      gettext \
      git \
      libcurl4-openssl-dev:i386 \
      liblua5.3-dev:i386 \
      libwxgtk-webview3.0-gtk3-dev:i386 \
      lsb-release \
      pkg-config \
      rapidjson-dev \
 && apt-get clean && rm -rf /var/lib/apt/lists/*

ENV MMEX_INST_CMD apt-get update && apt install -yqV ./mmex_*.deb
