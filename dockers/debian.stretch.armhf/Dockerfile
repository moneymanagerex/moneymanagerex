FROM debian:stretch-slim
LABEL maintainer="developers@moneymanagerex.org"
RUN dpkg --add-architecture armhf && apt-get update && \
    apt-get install -y --no-install-recommends \
      cmake crossbuild-essential-armhf gettext git pkg-config lsb-release \
      file ccache \
      libwxgtk-webview3.0-dev:armhf liblua5.3-dev:armhf \
      libcurl4-openssl-dev:armhf rapidjson-dev && \
    apt-get clean

ENV MMEX_INST_CMD apt install -yqV ./mmex_*.deb
