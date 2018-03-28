FROM linuxmintd/mint19-amd64
MAINTAINER developers@moneymanagerex.org
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      cmake build-essential gettext git pkg-config lsb-release \
      libwxgtk-webview3.0-gtk3-dev liblua5.3-dev libcurl4-openssl-dev \
      automake libtool ccache && \
    apt-get clean

ENV MMEX_INST_CMD apt install -yqV ./mmex_*.deb
