FROM debian:stretch-slim
MAINTAINER developers@moneymanagerex.org
RUN dpkg --add-architecture i386 && apt-get update && \
 && apt-get install -y --no-install-recommends \
      build-essential \
      ccache \
      cmake \
      g++-multilib \
      gettext \
      git \
      libcurl4-openssl-dev:i386 \
      liblua5.3-dev:i386 \
      libwxgtk-webview3.0-dev:i386 \
      lsb-release \
      pkg-config \
 && apt-get clean

ENV MMEX_INST_CMD apt install -yqV ./mmex_*.deb
