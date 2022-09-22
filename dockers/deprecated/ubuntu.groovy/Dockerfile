FROM ubuntu:groovy
MAINTAINER developers@moneymanagerex.org
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y -f --no-install-recommends \
      tcl \
      build-essential \
      ccache \
      cmake \
      file \
      gettext \
      git \
      libcurl4-openssl-dev \
      liblua5.3-dev \
      wx3.0-i18n \
      libwxgtk-webview3.0-gtk3-dev \
      lsb-release \
      pkg-config \
      rapidjson-dev \
 && rm -rf /var/lib/apt/lists/*
ENV MMEX_INST_CMD apt install -yqV ./mmex_*.deb
