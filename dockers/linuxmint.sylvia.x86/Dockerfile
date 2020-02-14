FROM linuxmintd/mint18-i386
MAINTAINER developers@moneymanagerex.org
RUN dpkg --add-architecture i386 && apt-get update && \
    apt-get install -y --no-install-recommends \
      cmake build-essential gettext git lsb-release ccache \
      libwxgtk-webview3.0-dev:i386 liblua5.3-dev:i386 \
      libcurl4-openssl-dev:i386 && \
    apt-get clean

ENV MMEX_INST_CMD apt install -yqV ./mmex_*.deb
