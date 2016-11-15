FROM ubuntu:14.04
MAINTAINER Lisheng Guan<guanlisheng@gmail.com>

RUN apt-get update && apt-get install -y libwxgtk3.0-dev build-essential

# copy MMEX source code to the container:
ADD . /

CMD ./configure && make -j4
