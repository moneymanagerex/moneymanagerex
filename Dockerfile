FROM ubuntu:14.04
MAINTAINER Lisheng Guan<guanlisheng@gmail.com>

RUN apt-get update && apt-get install -y build-essential git libgtk-3-dev 

RUN git clone https://github.com/wxWidgets/wxWidgets.git
RUN cd wxWidgets && git checkout tags/v3.1.0 && ./configure --enable-webview --disable-shared && make -j10 && make install && cd ..

# copy MMEX source code to the container:
ADD . /moneymanagerex

# Define working directory:
WORKDIR /moneymanagerex

RUN ./configure && make -j10
