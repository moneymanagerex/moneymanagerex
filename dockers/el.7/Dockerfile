FROM centos:7
LABEL maintainer="developers@moneymanagerex.org"
RUN yum -y --enablerepo=extras install epel-release \
 && yum -y install --setopt=tsflags=nodocs \
      ccache \
      cmake3 \
      gcc-c++ \
      gettext \
      git \
      libcurl-devel \
      make \
      rapidjson-devel \
      redhat-lsb-core \
      rpm-build \
      webkitgtk3-devel \
      wxGTK3-devel \
 && yum clean all

# cmake3 installed from EPEL
RUN for f in cmake make pack test; do \
      ln -sv /usr/bin/c${f}3 /usr/bin/c${f}; done

ENV MMEX_INST_CMD yum -y install ./mmex-*.rpm
