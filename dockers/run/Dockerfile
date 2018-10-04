FROM ubuntu:bionic

# Install dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        bash \
        cmake \
        build-essential \
        gettext \
        git \
        pkg-config \
        lsb-release \
        ca-certificates \
        file \
        ccache \
        libwxgtk-webview3.0-gtk3-dev \
        liblua5.3-dev \
        libcurl4-openssl-dev \
        rapidjson-dev \
        gosu && \
    apt-get clean

RUN git clone --recursive https://github.com/moneymanagerex/moneymanagerex.git /opt/moneymanagerex && \
    cd /opt/moneymanagerex && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make

RUN useradd mm &&\
    mkdir -p /home/mm && \
    mv /opt/moneymanagerex /home/mm && \
    chown -R mm /home/mm && \
    rm -rf /opt/moneymanagerex

WORKDIR /home/mm
ADD entrypoint.sh /root/entrypoint.sh
RUN chmod +x /root/entrypoint.sh
CMD ["/root/entrypoint.sh"]
