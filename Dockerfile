FROM ubuntu:14.04
MAINTAINER Lisheng Guan<guanlisheng@gmail.com>

RUN apt-get update && apt-get install -y build-essential

RUN git clone https://github.com/wxWidgets/wxWidgets.git
RUN cd wxWidgets && git checkout tags/v3.1.0 && ./configure && make -j10 && make install && cd ..

# copy MMEX source code to the container:
ADD . /moneymanagerex

# Define working directory:
WORKDIR /moneymanagerex

CMD ./configure && make -j4
