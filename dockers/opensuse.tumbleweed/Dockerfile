FROM opensuse/tumbleweed:latest
MAINTAINER developers@moneymanagerex.org
RUN zypper refresh && zypper dup && \
    zypper install -y gtk3-devel  pkg-config \
    	   patterns-devel-base-devel_basis git cmake sqlite3-devel \
    	   libssh-devel libedit-devel hunspell-devel clang-devel \
	   xterm webkit2gtk3-devel webkit2gtk3-soup2-devel \
	   ccache gettext-tools git libcurl-devel libcurl4 \
	   lsb-release lua-devel  make rpm-build file \
    	   patterns-devel-base-devel_basis wget unzip gcc-c++ glibc && \
    cd / && \
    wget --no-check-certificate https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.7/wxWidgets-3.1.7.zip && \
    unzip -d wxWidgets wxWidgets-*.zip && \
    rm wxWidgets-*.zip && \
    cd wxWidgets && \
    mkdir build-linux && \
    cd build-linux && \
    ../configure --disable-shared --enable-cxx11 --with-cxx=11 \
    	   --enable-webview --with-gtk=3 --enable-unicode --enable-intl \
    	   --enable-webviewwebkit --with-gtk=3 && \
    make

ENV MMEX_INST_CMD zypper install -y -f --allow-unsigned-rpm ./mmex*.rpm