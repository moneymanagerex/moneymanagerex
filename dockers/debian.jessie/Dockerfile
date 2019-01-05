FROM debian:jessie-slim
LABEL maintainer="developers@moneymanagerex.org"
RUN echo deb http://ftp.debian.org/debian jessie-backports main \
      > /etc/apt/sources.list.d/backports.list \
 && apt-get update && apt-get install -y --no-install-recommends \
      build-essential \
      ccache \
      file \
      gettext \
      git \
      libcurl4-openssl-dev \
      liblua5.2-dev \
      libwxgtk-webview3.0-dev \
      lsb-release \
      pkg-config \
 && apt-get install -t jessie-backports -y --no-install-recommends \
      cmake \
 && rm -rf /var/lib/apt/lists/*

ENV MMEX_INST_CMD apt-get update && ( dpkg -i ./mmex_*.deb || apt-get install -y -f )
