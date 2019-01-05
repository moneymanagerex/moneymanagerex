FROM debian:stretch-slim
LABEL maintainer="developers@moneymanagerex.org"
RUN apt-get update && apt-get install -y --no-install-recommends \
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
