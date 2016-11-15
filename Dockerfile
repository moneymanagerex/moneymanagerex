FROM ubuntu:14.04
MAINTAINER Lisheng Guan<guanlisheng@gmail.com>

RUN apt-get update && apt-get install -y libwxgtk3.0-dev build-essential webkitgtk-3.0

# copy MMEX source code to the container:
ADD . /moneymanagerex

# Define working directory:
WORKDIR /moneymanagerex

CMD ./configure && make -j4
